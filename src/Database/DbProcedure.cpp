#include "DbProcedure.h"

#include "Model/User.h"
#include "Database.h"
#include "Model/Parser.h"

std::vector<Procedure> DbProcedure::getProcedures(long long dental_visit_rowid, Db& db)
{
	std::vector<Procedure> list;

	std::string query = "SELECT "
						"procedure.code, "					//0
						"procedure.name, "					//1
						"procedure.type, "					//2
						"procedure.price, "					//3
						"dental_visit.dentist_rowid, "		//4
						"dental_visit.date, "				//5
						"procedure.diagnosis, "				//6
						"procedure.notes, "					//7
						"procedure.at_tooth_index, "		//8
						"procedure.temporary, "				//9
						"procedure.supernumeral, "			//10
						"procedure.surface_o, "				//11
						"procedure.surface_m, "				//12
						"procedure.surface_d, "				//13
						"procedure.surface_b, "				//14
						"procedure.surface_l, "				//15
						"procedure.surface_c, "				//16
						"procedure.post, "					//17
						"procedure.from_tooth_index, "		//18
						"procedure.to_tooth_index "			//19
				"FROM procedure LEFT JOIN dental_visit ON procedure.dental_visit_rowid = dental_visit.rowid "
				"WHERE dental_visit.rowid=? "
				"ORDER BY procedure.rowid";

	db.newStatement(query);

	db.bind(1, dental_visit_rowid);

	while(db.hasRows())
	{
		list.emplace_back();
		Procedure& p = list.back();

		p.code = db.asString(0);
		p.name = db.asString(1);
		p.type = static_cast<Procedure::Type>(db.asInt(2));
		p.price = db.asDouble(3);
		p.dentist_rowid = db.asRowId(4);
		p.date = db.asString(5);
		p.diagnosis = db.asString(6);
		p.notes = db.asString(7);
	
		auto scope = p.getScope(p.type);

		if (scope == Procedure::Scope::SingleTooth) {

			p.affectedTeeth = ToothIndex{
				.index = db.asInt(8),
				.temp = db.asBool(9),
				.supernumeral = db.asBool(10)
			};

			if (p.type == Procedure::Type::Restoration || p.type == Procedure::Type::RemoveRestoration) {
				p.param = RestorationData{
					.surfaces = {
						db.asBool(11),
						db.asBool(12),
						db.asBool(13),
						db.asBool(14),
						db.asBool(15),
						db.asBool(16)
					},
					.post = db.asBool(17)
				};
			}
		}
		else if (scope == Procedure::Scope::Range)
		{
			p.affectedTeeth = ConstructionRange{
				db.asInt(18),
				db.asInt(19)
			};
		}
	}

	return list;

}

std::vector<Procedure> DbProcedure::getProcedures(long long dental_visit_rowid)
{
	Db db;
	return getProcedures(dental_visit_rowid, db);
}

void DbProcedure::saveProcedures(long long dental_visit_rowid, const std::vector<Procedure>& pList, Db& db)
{
	std::string query = "DELETE FROM procedure WHERE dental_visit_rowid = " + std::to_string(dental_visit_rowid);

	db.execute(query);

	
    for (auto& p : pList)
	{

		db.newStatement(
			"INSERT INTO procedure "
			"(dental_visit_rowid, code, name, type, price, diagnosis, notes, at_tooth_index, temporary, supernumeral, "
			"surface_o, surface_m, surface_d, surface_b, surface_l, surface_c, post, from_tooth_index, to_tooth_index) "
			"VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

		db.bind(1, dental_visit_rowid);
		db.bind(2, p.code);
		db.bind(3, p.name);
		db.bind(4, static_cast<int>(p.type));
		db.bind(5, p.price);
		db.bind(6, p.diagnosis);
		db.bind(7, p.notes);

		auto& tooth = p.getToothIndex();

		db.bind(8, tooth.index);
		db.bind(9, tooth.temp);
		db.bind(10, tooth.supernumeral);

		switch (Procedure::getScope(p.type))
		{
			case Procedure::Scope::SingleTooth:
			{
				if (std::holds_alternative<RestorationData>(p.param)) {

					auto& [surfaces, post] = std::get<RestorationData>(p.param);

					for (int i = 0; i < 6; i++) {
						db.bind(11 + i, surfaces[i]);
					}

					db.bind(17, post);
				}

			}
			break;

			case Procedure::Scope::Range:
			{
				auto& [from, to] = std::get<ConstructionRange>(p.affectedTeeth);

				db.bind(8, -1);
				db.bind(18, from);
				db.bind(19, to);

				break;
			}
		}

		db.execute();
	}

}

std::vector<Procedure> DbProcedure::getToothProcedures(long long patientRowId, int tooth)
{
	std::string query =
		"SELECT  "
		"dental_visit.date, "
		"procedure.code, "
		"procedure.name, "
		"procedure.price, "
		"dental_visit.dentist_rowid, "
		"procedure.temporary, "
		"procedure.supernumeral, "
		"procedure.diagnosis, "
		"procedure.notes, "
		"procedure.at_tooth_index, "
		"procedure.from_tooth_index, "
		"procedure.to_tooth_index "
		"FROM "
		"procedure LEFT JOIN dental_visit ON procedure.dental_visit_rowid = dental_visit.rowid "
		"WHERE (at_tooth_index = ? "
		"OR (from_tooth_index <= ? AND to_tooth_index >= ? )) "
		"AND patient_rowid = ? " 
		"ORDER BY dental_visit.date DESC, procedure.rowid DESC";

	std::vector<Procedure> procedures;
	
	Db db(query);

	db.bind(1, tooth);
	db.bind(2, tooth);
	db.bind(3, tooth);
	db.bind(4, patientRowId);

	while (db.hasRows())
	{
		procedures.emplace_back();
		auto& p = procedures.back();

		p.date = Date{ db.asString(0) };
		p.code = db.asString(1);
		p.name = db.asString(2);
		p.price = db.asDouble(3);

		p.dentist_rowid = db.asRowId(4);

		p.affectedTeeth = ToothIndex{
			.index = tooth,
			.temp = db.asBool(5),
			.supernumeral = db.asBool(6)
		};

		p.diagnosis = db.asString(7);
		p.notes = db.asString(8);

		if (db.asInt(9) != tooth) //a.k.a. is -1
		{
			p.affectedTeeth = ConstructionRange{
				db.asInt(10),
				db.asInt(11)
			};

		}
	}

	return procedures;
}

std::vector<Procedure> DbProcedure::getPatientProcedures(long long patientRowid)
{
	std::vector<Procedure> mList;

	std::string query = "SELECT "
				"dental_visit.dentist_rowid, "		//0
				"dental_visit.date, "				//1
				"procedure.code, "					//2
				"procedure.name, "					//3
				"procedure.price, "					//4
				"procedure.type, "					//5
				"procedure.diagnosis, "				//6
				"procedure.notes, "					//7
				"procedure.at_tooth_index, "		//8
				"procedure.temporary, "				//9
				"procedure.supernumeral, "			//10
				"procedure.surface_o, "				//11
				"procedure.surface_m, "				//12
				"procedure.surface_d, "				//13
				"procedure.surface_b, "				//14
				"procedure.surface_l, "				//15
				"procedure.surface_c, "				//16
				"procedure.post, "					//17
				"procedure.from_tooth_index, "		//18
				"procedure.to_tooth_index "			//19
				"FROM procedure "				
				"LEFT JOIN dental_visit ON procedure.dental_visit_rowid = dental_visit.rowid "
				"LEFT JOIN patient ON dental_visit.patient_rowid = patient.rowid "
				"WHERE patient.rowid=? "
				"ORDER BY dental_visit.date DESC";

	Db db;

	db.newStatement(query);

	db.bind(1, patientRowid);

	while (db.hasRows())
	{
		mList.emplace_back();
		Procedure& p = mList.back();

		p.dentist_rowid = db.asRowId(0);
		p.date = db.asString(1);
		p.code = db.asString(2);
		p.name = db.asString(3);
		p.price = db.asDouble(4);
		p.type = static_cast<Procedure::Type>(db.asInt(5));
		p.diagnosis = db.asString(6);
		p.notes = db.asString(7);

		auto scope = Procedure::getScope(p.type);

		if (scope == Procedure::Scope::SingleTooth) {

			p.affectedTeeth = ToothIndex{
				.index = db.asInt(8),
				.temp = db.asBool(9),
				.supernumeral = db.asBool(10)
			};

			if (p.hasSurfaceData()) {

				p.param = RestorationData{
					.surfaces = {
						db.asBool(11),
						db.asBool(12),
						db.asBool(13),
						db.asBool(14),
						db.asBool(15),
						db.asBool(16)
					},
					.post = db.asBool(17)
				};
			}
		}

		if (scope == Procedure::Scope::Range)
		{
			if (!p.getToothIndex().isValid()) {

				p.affectedTeeth = ConstructionRange{
					db.asInt(18),
					db.asInt(19)
				};
			}
		}

	}

	return mList;

}

std::vector<ProcedureListElement> DbProcedure::getProcedureList()
{
	Db db;

	db.newStatement("SELECT  type, code, name, price, diagnosis FROM procedure_list");

	std::vector<ProcedureListElement> result;

	while (db.hasRows()) {
		result.push_back(
			ProcedureListElement{
				.type = static_cast<Procedure::Type>(db.asInt(0)),
				.code = db.asString(1),
				.name = db.asString(2),
				.price = db.asDouble(3),
				.diagnosis = db.asString(4)
			});
	}

	return result;
}

void DbProcedure::setProcedureList(const std::vector<ProcedureListElement>& codes)
{
	Db db;

	db.execute("DELETE FROM procedure_list");

	for(auto& c : codes){
		db.newStatement("INSERT INTO procedure_list (code,name,type,price,diagnosis) VALUES (?,?,?,?,?)");
		
		db.bind(1, c.code);
		db.bind(2, c.name);
		db.bind(3, static_cast<int>(c.type));
		db.bind(4, c.price);
		db.bind(5, c.diagnosis);

		db.execute();
	};
}