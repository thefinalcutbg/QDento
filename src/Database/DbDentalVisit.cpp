#include "DbDentalVisit.h"
#include "Database/Database.h"
#include "Model/User.h"
#include "Model/Patient.h"
#include "Model/Dental/DentalVisit.h"
#include "Model/Date.h"
#include "Model/Parser.h"
#include "DbProcedure.h"
#include "Model/FreeFunctions.h"

long long DbDentalVisit::insert(const DentalVisit& sheet, long long patientRowId)
{

    Db db("INSERT INTO dental_visit "
        "(date, num, status, patient_rowid, dentist_rowid) "
        "VALUES (?,?,?,?,?)");

    db.bind(1, sheet.date.to8601());
    db.bind(2, sheet.number);
    db.bind(3, Parser::write(sheet.teeth));
    db.bind(4, patientRowId);
    db.bind(5, User::dentist().rowID);

    db.execute();

    auto rowID = db.lastInsertedRowID();

    DbProcedure::saveProcedures(rowID, sheet.procedures.list(), db);

    return rowID;
}

void DbDentalVisit::update(const DentalVisit& sheet)
{
    std::string query = "UPDATE dental_visit SET "
        "num=?,"
        "date=?,"
        "status=?"
        "WHERE rowid=?";
    
    Db db(query);

    db.bind(1, sheet.number);
    db.bind(2, sheet.date.to8601());
    db.bind(3, Parser::write(sheet.teeth));
    db.bind(4, sheet.rowid);

    db.execute();

    DbProcedure::saveProcedures(sheet.rowid, sheet.procedures.list(), db);
}

DentalVisit DbDentalVisit::getNewDoc(long long patientRowId)
{

    DentalVisit visit;
    visit.dentist_rowid = User::dentist().rowID;
    std::string status;

    Db db;

    std::string query = "SELECT rowid, num, status, date FROM dental_visit WHERE "
        "patient_rowid=? AND dentist_rowid=? AND "
        "date(dental_visit.date) = date('now')";

    db.newStatement(query);

    db.bind(1, patientRowId);
    db.bind(2, User::dentist().rowID);

    while(db.hasRows())
    {
        visit.patient_rowid = patientRowId;
        visit.rowid = db.asRowId(0);
        visit.number = db.asInt(1);
        status = db.asString(2);
        visit.date = db.asString(3);
    }

    if (!visit.rowid)
    {
        //getting the last recorded status

        db.newStatement(
            "SELECT rowid, status, date FROM dental_visit WHERE "
            "patient_rowid = ? "
            "ORDER BY date DESC LIMIT 1"
        );

        db.bind(1, patientRowId);

        long long oldId = 0;
        std::string basedOnNrn;
        Date amblistDate;

        while(db.hasRows()){
            
            oldId = db.asRowId(0);
            status = db.asString(1);
            amblistDate = db.asString(2);
        }
  
        if (!oldId) return visit; //no data is found for this patient

        Parser::parse(status, visit.teeth);

        //getting all procedures after the last recorded status and applying them

        db.newStatement(
            "SELECT dental_visit.rowid FROM procedure "
            "LEFT JOIN dental_visit ON procedure.dental_visit_rowid = dental_visit.rowid "
            "WHERE dental_visit.date >= ? AND patient_rowid = ? "
            "GROUP BY dental_visit.rowid ORDER BY dental_visit.date ASC"
        );

        db.bind(1, amblistDate.to8601());
        db.bind(2, patientRowId);

        std::vector<long long> amblistRowidProcedures;


        while (db.hasRows()) {
            amblistRowidProcedures.push_back(db.asLongLong(0));
        }

        for (auto& rowid : amblistRowidProcedures) {
            for (auto& p : DbProcedure::getProcedures(rowid, db))
            {
                p.applyProcedure(visit.teeth);

            }
        }

        std::string query =
            "SELECT num FROM dental_visit WHERE "
            "dentist_rowid=? "
            "ORDER BY num DESC LIMIT 1";

        db.newStatement(query);

        db.bind(1, User::dentist().rowID);

        while (db.hasRows()) {
            visit.number = db.asInt(0) + 1;
        };

        return visit;
    }

    Parser::parse(status, visit.teeth);
    visit.procedures.addProcedures(DbProcedure::getProcedures(visit.rowid, db));


    return visit;
}

DentalVisit DbDentalVisit::get(long long rowId)
{

    std::string status;
    DentalVisit dental_visit;

    Db db(
        "SELECT rowid, num, status, patient_rowid, date FROM dental_visit WHERE "
        "rowid = " + std::to_string(rowId)
    );

    while (db.hasRows())
    {
        dental_visit.rowid = db.asRowId(0);
        dental_visit.number = db.asInt(1);
        status = db.asString(2);
        dental_visit.dentist_rowid = User::dentist().rowID;
        dental_visit.patient_rowid = db.asRowId(3);
        dental_visit.date = db.asString(4);
    }

    Parser::parse(status, dental_visit.teeth);
    dental_visit.procedures.addProcedures(DbProcedure::getProcedures(dental_visit.rowid, db));
    return dental_visit;

}

void DbDentalVisit::remove(long long rowid)
{
    Db::crudQuery("DELETE FROM dental_visit WHERE rowid = " + std::to_string(rowid) + ")");
}

int DbDentalVisit::getNewNumber(Date ambDate)
{

    std::string query = 
        "SELECT num FROM dental_visit WHERE " 
        "dentist_rowid=? "
        "ORDER BY num DESC LIMIT 1";

    Db db(query);

    db.bind(1, User::dentist().rowID);

    while (db.hasRows()) {
        return db.asInt(0) + 1;
    };

    return 1;
}