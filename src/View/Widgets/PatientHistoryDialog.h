#pragma once

#include <QDialog>
#include "ui_PatientHistoryDialog.h"
#include "Model/Dental/Snapshot.h"
#include "Model/PlainTable.h"
#include "Model/Dental/Procedure.h"
#include "Model/Dental/PerioStatistic.h"
#include "View/TableModels/PlainTableModel.h"
#include "View/TableModels/ProcedureTableModel.h"


class PatientHistoryPresenter;


class PatientHistoryDialog : public QDialog
{
	Q_OBJECT

	static constexpr int PERIO_TAB_INDEX = 3;

	PatientHistoryPresenter& presenter;

	PlainTableModel doc_model;
	PlainTableModel doc_details_model;
	std::vector<PlainTable> details_data;
	
	ProcedureTableModel procedure_model;

public:
	PatientHistoryDialog(PatientHistoryPresenter& presenter, QWidget *parent = nullptr);
	void setProcedures(const std::vector<Procedure> procedures);
	void setSnapshots(const std::vector<Snapshot>& snapshots);
	void setDocuments(const PlainTable& docList, const std::vector<PlainTable>& contents);
	void setPerioSnapshots(const std::vector<PerioSnapshot>& snapshots);
	void setPatientNoteFlags(const std::array<std::string, 32>& notes);

	~PatientHistoryDialog();

private:
	Ui::PatientHistoryDialogClass ui;
};
