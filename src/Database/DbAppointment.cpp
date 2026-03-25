#include "DbAppointment.h"
#include "Database/Database.h"

long long DbAppointment::insert(const CalendarEvent& e, long long dentist_rowid)
{
	auto query = "INSERT INTO appointment (dentist_rowid, patient_rowid, start, end, summary, description) VALUES (?,?,?,?,?,?)";

	Db db(query);

	db.bind(1, dentist_rowid);
	e.patient_rowid ? db.bind(2, e.patient_rowid) : db.bindNull(2);
	db.bind(3, e.start.toString(Qt::ISODate).toStdString());
	db.bind(4, e.end.toString(Qt::ISODate).toStdString());
	db.bind(5, e.summary);
	db.bind(6, e.description);

	db.execute();

	return db.lastInsertedRowID();
}

void DbAppointment::update(const CalendarEvent& e)
{
	auto query = "UPDATE appointment SET patient_rowid=?, start=?, end=?, summary=?, description=? WHERE rowid=?";

	Db db(query);

	e.patient_rowid ? db.bind(1, e.patient_rowid) : db.bindNull(1);
	db.bind(2, e.start.toString(Qt::ISODate).toStdString());
	db.bind(3, e.end.toString(Qt::ISODate).toStdString());
	db.bind(4, e.summary);
	db.bind(5, e.description);
	db.bind(6, e.rowid);

	db.execute();

}

void DbAppointment::remove(long long rowid)
{
	Db db("DELETE FROM appointment WHERE rowid=?");

	db.bind(1, rowid);

	db.execute();
}

std::vector<CalendarEvent> DbAppointment::get(const QDate& from, const QDate& to, long long dentist_rowid)
{
	auto fromDate = from.toString(Qt::ISODate).toStdString();
	auto toDate = to.toString(Qt::ISODate).toStdString();

	auto query = "SELECT rowid, patient_rowid, start, end, summary, description FROM appointment WHERE dentist_rowid =? AND strftime('%Y-%m-%d', start) BETWEEN ? AND ? ORDER BY start ASC";

	Db db(query);

	db.bind(1, dentist_rowid);
	db.bind(2, fromDate);
	db.bind(3, toDate);

	std::vector<CalendarEvent> result;

	while (db.hasRows())
	{
		CalendarEvent e;

		e.rowid = db.asRowId(0);
		e.patient_rowid = db.asRowId(1);
		e.start = QDateTime::fromString(db.asString(2).c_str(), Qt::DateFormat::ISODate);
		e.end = QDateTime::fromString(db.asString(3).c_str(), Qt::DateFormat::ISODate);
		e.summary = db.asString(4);
		e.description = db.asString(5);

		result.push_back(e);
	}

	return result;
}
