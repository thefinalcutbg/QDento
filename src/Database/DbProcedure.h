#pragma once

#include "Model/TableStructs.h"
#include "Model/Dental/Procedure.h"
#include "Model/Dental/ProcedureListElement.h"
#include <vector>

class Date;

class Db;

namespace DbProcedure
{
    std::vector<Procedure> getProcedures(long long dental_visit_rowid, Db& db);
    std::vector<Procedure>  getProcedures(long long dental_visit_rowid);
    void saveProcedures(long long dental_visit_rowid, const std::vector<Procedure>& p, Db& db);

    std::vector<Procedure> getToothProcedures(long long patientRowId, int tooth);
    std::vector<Procedure> getPatientProcedures(long long patientRowid);

    std::vector<ProcedureListElement> getProcedureList();
    void setProcedureList(const std::vector<ProcedureListElement>& codes);
};

