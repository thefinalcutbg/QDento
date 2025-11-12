#include "DbBrowser.h"
#include "Model/FreeFunctions.h"
#include "Model/User.h"
#include "Database.h"
#include "DbProcedure.h"
#include <map>

std::pair<std::vector<RowInstance>, PlainTable> getPatientRows()
{
    std::vector<RowInstance> rows;
    PlainTable tableView;
    
    rows.reserve(50);

    tableView.addColumn({"Identifier",150,PlainColumn::Center});
    tableView.addColumn({"Patient Name",250});
    tableView.indicator_column = 2;
    tableView.addColumn({"Patient Phone",120,PlainColumn::Center});

    std::string query =
        "SELECT rowid, id, fname, lname , phone,  "
        "(strftime('%m-%d', patient.birth) = strftime('%m-%d',date('now', 'localtime')) AND strftime('%Y', patient.birth) != '1900') AS bday, "
        "color FROM patient ORDER BY bday DESC, id ASC";

    for (Db db(query);db.hasRows();)
    {
       rows.emplace_back(TabType::PatientSummary);
       
       rows.back().rowID = db.asRowId(0);
       rows.back().patientRowId = db.asRowId(0);

       tableView.addCell(0, { .data = db.asString(1) });

       tableView.addCell(1, { 
           .data = FreeFn::getPatientName(db.asString(2), db.asString(3)),
           .icon = db.asBool(5) ?
                CommonIcon::BDAY
                :
                CommonIcon::NOICON
       });

       tableView.addCell(2, { 
           .data = db.asString(4)
       });

       tableView.setIndicatorToLastRow(db.asString(6));
    }

    return std::make_pair(rows, tableView);
}

std::pair<std::vector<RowInstance>, PlainTable> getAmbRows(const Date& from, const Date& to)
{
    std::vector<RowInstance> rows;
    PlainTable tableView;

    tableView.addColumn({"Date",120,PlainColumn::Center});
    tableView.addColumn({"Number",110,PlainColumn::Center});
    tableView.addColumn({"Identifier",120,PlainColumn::Center});
    tableView.addColumn({"Patient Name",240});
    tableView.indicator_column = 4;
    tableView.addColumn({"Phone Number",120,PlainColumn::Center});

    rows.reserve(50);

    std::string query =
        "SELECT "
        "dental_visit.rowid, "
        "dental_visit.date, "
        "dental_visit.num, "
        "patient.rowid, patient.id, patient.fname, patient.lname, patient.phone, "
        "(strftime('%m-%d', patient.birth) = strftime('%m-%d',date('now', 'localtime')) AND strftime('%Y', patient.birth) != '1900') AS bday, "
        "patient.color "
        "FROM dental_visit "
        "JOIN patient ON dental_visit.patient_rowid = patient.rowid "
        "LEFT JOIN procedure ON dental_visit.rowid = procedure.dental_visit_rowid "
        "WHERE strftime('%Y-%m-%d', dental_visit.date) BETWEEN '" + from.to8601() + "' AND '" + to.to8601() + "' "
        "AND dental_visit.dentist_rowid = ? "
        "GROUP BY dental_visit.rowid "
        "ORDER BY dental_visit.date ASC, dental_visit.rowid ASC";

    Db db(query);

    db.bind(1, User::dentist().rowID);

    while (db.hasRows())
    {
        rows.emplace_back(TabType::DentalVisit);

        auto& row = rows.back();
        row.rowID = db.asRowId(0);

        tableView.addCell(0, { .data = Date(db.asString(1)).toLocalFormat() });

        tableView.addCell(1, { .data = std::to_string(db.asInt(2)) });

        row.patientRowId = db.asRowId(3);

        //ID
        tableView.addCell(2, { .data = db.asString(4) });

        //Name
        tableView.addCell(3, {
             .data = FreeFn::getPatientName(db.asString(5), db.asString(6)),
             .icon = db.asBool(8) ?
                    CommonIcon::BDAY
                    :
                    CommonIcon::NOICON
        });

        //Phone
        tableView.addCell(4, { .data = db.asString(7)
        });

        //Color
        tableView.setIndicatorToLastRow(db.asString(9));
    }
        
    return std::make_pair(rows, tableView);
}

std::pair<std::vector<RowInstance>, PlainTable> getPerioRows(const Date& from, const Date& to)
{
    std::vector<RowInstance> rows;
    PlainTable tableView;
    rows.reserve(50);

    tableView.addColumn({"Date",120,PlainColumn::Center});
    tableView.addColumn({"Identifier",150,PlainColumn::Center});
    tableView.addColumn({"Patient Name",250,});
    tableView.indicator_column = 3;
    tableView.addColumn({"Phone Number",120,PlainColumn::Center });

    std::string query =
        "SELECT periostatus.rowid, periostatus.date, patient.rowid, patient.id, patient.fname, patient.lname, patient.phone, "
        "(strftime('%m-%d', patient.birth) = strftime('%m-%d',date('now', 'localtime'))) AS bday, patient.color "
        "FROM periostatus INNER JOIN patient ON periostatus.patient_rowid = patient.rowid "
        "WHERE "
        "periostatus.date BETWEEN '" + from.to8601() + "' AND '" + to.to8601() + "' "
        "AND periostatus.dentist_rowid = ? "
        "ORDER BY periostatus.date ASC ";

    Db db(query);

    db.bind(1, User::dentist().rowID);

    while (db.hasRows())
    {
        rows.emplace_back(TabType::PerioStatus);

        auto& row = rows.back();

        row.rowID = db.asRowId(0);
        row.patientRowId = db.asRowId(2);

        //Date
        tableView.addCell(0, {.data = Date(db.asString(1)).toLocalFormat()});

        tableView.addCell(1, { .data = db.asString(3) });

        tableView.addCell(2, {
             .data = FreeFn::getPatientName(db.asString(4), db.asString(5)),
             .icon = db.asBool(7) ?
                    CommonIcon::BDAY
                    :
                    CommonIcon::NOICON
        });

        tableView.addCell(3, { .data = db.asString(6) });

        tableView.setIndicatorToLastRow(db.asString(8));
    }

    return std::make_pair(rows, tableView);
}

#include "Model/Financial/Recipient.h"


std::pair<std::vector<RowInstance>, PlainTable> getFinancialRows(const Date& from, const Date& to)
{
    std::vector<RowInstance> rows;
    PlainTable tableView;

    tableView.addColumn({"Date", 120, PlainColumn::Right});
    tableView.addColumn({"Number", 110, PlainColumn::Center});
    tableView.addColumn({"Identifier", 100, PlainColumn::Center});
    tableView.addColumn({"Recipient Name", 250 });
    tableView.addColumn({"Phone Number", 100, PlainColumn::Center});

    std::string query =
        "SELECT rowid, num, "
        "date, "
        "recipient_id, recipient_name, recipient_phone "
        "FROM financial "
        "WHERE "
        "date BETWEEN '" + from.to8601() + "' AND '" + to.to8601() + "' "
        "ORDER BY date ASC ";

    for (Db db(query); db.hasRows();)
    {
        rows.emplace_back(TabType::Financial);

        auto& row = rows.back();

        row.rowID = db.asRowId(0);

        //Date
        tableView.addCell(0, {.data = Date(db.asString(2)).toLocalFormat()});

        //Number
        tableView.addCell(1, {.data = FreeFn::leadZeroes(db.asLongLong(1), 10)});
        
        tableView.addCell(2, { .data = db.asString(3) });
        tableView.addCell(3, { .data = db.asString(4) });
        tableView.addCell(4, { .data = db.asString(5) });
      
    }

    return std::make_pair(rows, tableView);

}

std::pair<std::vector<RowInstance>, PlainTable> DbBrowser::getPatientDocuments(long long patientRowid)
{
    PlainTable table;

    std::vector<RowInstance> rowidData;

    table.addColumn({"Date",120,PlainColumn::Center});
    table.addColumn({"Document",180});
    table.addColumn({"Number", 150, PlainColumn::Center});
    table.addColumn({"Issued by", 100, PlainColumn::Center });

    Db db;

    db.newStatement(
        "SELECT           rowid, 1 as type, date, num, dentist_rowid as author, (dentist_rowid = ? ) as from_me FROM dental_visit WHERE patient_rowid=? "
        "UNION ALL SELECT rowid, 2 AS type, date, NULL AS num, dentist_rowid as author, (dentist_rowid = ?) as from_me FROM periostatus WHERE patient_rowid=? "
        "UNION ALL SELECT financial.rowid, 3 AS type, date, num, (SELECT name FROM company LIMIT 1) AS author, TRUE as from_me FROM financial LEFT JOIN patient ON financial.recipient_id = patient.id WHERE patient.rowid=? "
        "ORDER BY date DESC"
    );

    auto dentistRowid = User::dentist().rowID;

    db.bind(1, dentistRowid);
    db.bind(2, patientRowid);
    db.bind(3, dentistRowid);
    db.bind(4, patientRowid);
    db.bind(5, patientRowid);

    while (db.hasRows())
    {
        long long rowid = db.asRowId(0);
        int type = db.asInt(1);
        std::string date = Date(db.asString(2)).toLocalFormat();

        std::string docTypeString;
        CommonIcon::Type docTypeIcon = CommonIcon::NOICON;

        switch (type) {
            case 1: 
                docTypeString = "Dental Visit";
                docTypeIcon = CommonIcon::DENTALVISIT;
                break;
            case 2:
                docTypeString = "Periodontal Measurment";
                docTypeIcon = CommonIcon::PERIO;
                break;
            case 3:
                docTypeString = "Financial Document";
                docTypeIcon = CommonIcon::INVOICE;
                break;
        }


        rowidData.push_back(static_cast<TabType>(type));
        rowidData.back().rowID = rowid;
        //financial
        rowidData.back().patientRowId = type == 3 ? 0 : patientRowid;
        rowidData.back().permissionToOpen = db.asBool(5);

        table.addCell(0, { .data = date });
        table.addCell(1, { .data = docTypeString, .icon = docTypeIcon });
        table.addCell(2, { .data = db.asString(3) });
        table.addCell(3, { .data = type == 3 ? db.asString(4) : User::getNameFromRowid(db.asRowId(4))});
        
    }

    return std::make_pair(rowidData, table);
};

std::pair<std::vector<RowInstance>, PlainTable> DbBrowser::getData(TabType type, const Date& from, const Date& to)
{
    switch (type)
    {
        case TabType::DentalVisit: return getAmbRows(from, to);
        case TabType::PerioStatus: return getPerioRows(from, to);
        case TabType::PatientSummary: return getPatientRows();
        case TabType::Financial: return getFinancialRows(from, to);
        default: return {};
    }
}

void DbBrowser::deleteRecord(TabType type, long long rowid)
{
    static const std::map<TabType, std::string_view> tableNames = {
        {TabType::DentalVisit, "dental_visit"},
        {TabType::PerioStatus, "periostatus"},
        {TabType::PatientSummary, "patient"},
        {TabType::Financial, "financial"}
    };

    std::string tableName{ tableNames.at(type) };

    auto query = "DELETE FROM " + tableName + " WHERE rowid = " + std::to_string(rowid);

    Db::crudQuery(query);
}
