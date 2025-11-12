#include "BrowserPresenter.h"
#include "View/Widgets/BrowserDialog.h"
#include "View/ModalDialogBuilder.h"
#include "Presenter/TabPresenter.h"
#include "Presenter/PatientDialogPresenter.h"
#include "Database/DbInvoice.h"
#include "Database/DbProcedure.h"
#include "Database/DbBrowser.h"
#include "Database/DbPatient.h"

#include <QObject>

#include <map>

void BrowserPresenter::setView(BrowserDialog* view)
{
	this->view = view;
	
	static bool firstCall = true;

	if (firstCall) {

		firstCall = false;
		//no need to show patient list if it is shown on every new document
		ui_state.model_type =
			TabType::PatientSummary;
	}

	this->view->setUiState(ui_state);

	if (!view) return;

	refreshModel();
	
}

void BrowserPresenter::showProcedureDetails(bool show)
{
	ui_state.showProcedures = show;

	refreshPreview();
}

void BrowserPresenter::setDates(const Date& from, const Date& to)
{
	ui_state.from = from;
	ui_state.to = to;
	refreshModel();
}

void BrowserPresenter::refreshModel()
{
	PlainTable tableView;
	
	std::tie(rowidData, tableView) = DbBrowser::getData(ui_state.model_type, ui_state.from, ui_state.to);

    tableView.data.insert(tableView.data.begin(), PlainColumn());

    for (size_t i = 0; i < rowidData.size(); i++)
	{
		//inserting additional column containing the corresponding index in rowidData
		tableView.addCell(0, { std::to_string(i) });
	}

	m_selectedInstances.clear();

	if (view == nullptr) return;

	int id{ 0 }, name{ 0 }, phone{ 0 };

	switch (ui_state.model_type) {
		case TabType::PatientSummary:
			id = 1; name = 2; phone = 3; break;
		case TabType::DentalVisit:
			id = 3; name = 4; phone = 5; break;
		case TabType::Financial:
			id = 3; name = 4; phone = 5; break;
		case TabType::PerioStatus:
			id = 2; name = 3; phone = 4; break;
	}

	view->setTable(tableView, id, name, phone);
	
	selectionChanged(std::set<int>());
}

void BrowserPresenter::refreshPreview()
{
	patientDocRowid.clear();

	long long rowid = m_selectedInstances.size() == 1 ?
		m_selectedInstances[0]->rowID
		:
		0;

	switch (ui_state.model_type)
	{
		case TabType::DentalVisit: 
			view->setPreview({ DbProcedure::getProcedures(rowid)}); break;
		case TabType::Financial: view->setPreview(DbInvoice::getInvoice(rowid).businessOperations); break;
		case TabType::PerioStatus: view->setPreview(PlainTable{}); break;
		case TabType::PatientSummary: 
			if (ui_state.showProcedures) {
				view->setPreview({ DbProcedure::getPatientProcedures(rowid) });
				return;
			}

			PlainTable temp;

			std::tie(patientDocRowid, temp) = DbBrowser::getPatientDocuments(rowid);

			view->setPreview(temp);

			break;
	}
}


void BrowserPresenter::setListType(TabType type)
{
	ui_state.model_type = type;

	refreshModel();
}

void BrowserPresenter::selectionChanged(const std::set<int>& selectedIndexes)
{ 
	m_selectedInstances.clear();

	for (auto idx : selectedIndexes) {
		m_selectedInstances.push_back(&rowidData[idx]);
	}
	
	refreshPreview();
}

void BrowserPresenter::openNewDocument(TabType type)
{
	if (ui_state.model_type == TabType::Financial) return;

	if (type == TabType::Calendar) {

		CalendarEvent ev(DbPatient::get(m_selectedInstances[0]->patientRowId));

		TabPresenter::get().openCalendar(ev);

		view->close();

		return;
	}

    for (size_t i = 0; i < std::min(m_selectedInstances.size(), size_t(10)); i++) {

		RowInstance row(type);
		row.rowID = 0;
		row.patientRowId = m_selectedInstances[i]->patientRowId;

		TabPresenter::get().open(row, i == m_selectedInstances.size() - 1);
	}

	if (type != TabType::PatientSummary) {
		view->close();
	}
}

void BrowserPresenter::openCurrentSelection()
{
	if (!m_selectedInstances.size()) return;

	if (ui_state.model_type == TabType::PatientSummary) {

		auto result = ModalDialogBuilder::openButtonDialog(
			{
				QObject::tr("New dental visit").toStdString(),
				QObject::tr("New periodontal measurment").toStdString(),
				QObject::tr("New financial document").toStdString(),
				QObject::tr("Schedule new appointment").toStdString(),
				QObject::tr("Patient history").toStdString()
			},
			QObject::tr("Open").toStdString()
		);

		if (result == -1) return;

		static TabType arr[5]{
			TabType::DentalVisit,
			TabType::PerioStatus,
			TabType::Financial,
			TabType::Calendar,
			TabType::PatientSummary
		};

		openNewDocument(arr[result]);

		return;

	}


    std::size_t counter{ 0 };


	for (auto& row : m_selectedInstances) {

		bool isLastTab = ++counter == m_selectedInstances.size();

		TabPresenter::get().open(*row, isLastTab);
	}

	if (view) view->close();
}




void BrowserPresenter::deleteCurrentSelection()
{
	if (m_selectedInstances.empty()) return;
	
	std::string warningMsg = QObject::tr("Are you sure you want to delete selected ").toStdString();

	static std::map<TabType, std::string> endString = {
		{TabType::DentalVisit, QObject::tr("dental visits?").toStdString()},
		{TabType::PerioStatus, QObject::tr("periodontal measurments?").toStdString()},
		{TabType::PatientSummary, QObject::tr("patients? All their medical files will be deleted!").toStdString()},
		{TabType::Financial, QObject::tr("financial documents?").toStdString()},
	};

	warningMsg += endString.at(ui_state.model_type);

	if (!ModalDialogBuilder::askDialog(warningMsg))
		return;

	for (auto& row : m_selectedInstances)
	{
		if (TabPresenter::get().documentTabOpened(row->type, row->rowID))
		{
			ModalDialogBuilder::showMessage
			(QObject::tr("First you have to close all documents marked for deletion!").toStdString());
			return;
		}
	}

	for (auto& row : m_selectedInstances) {
		DbBrowser::deleteRecord(row->type, row->rowID);
	}


	refreshModel();
	
}

void BrowserPresenter::openPatientDocuments(const std::set<int>& selectedIndexes)
{
	bool uiShowsPatientDocs =
		ui_state.model_type == TabType::PatientSummary &&
		ui_state.showProcedures == false
	;

	if (!uiShowsPatientDocs) return;

	if (selectedIndexes.empty()) return;

	int counter = 0;

	bool someNotOpened = false;

	for (auto row : selectedIndexes) {

		bool isLastTab = ++counter == m_selectedInstances.size();

		if (!TabPresenter::get().open(patientDocRowid[row], isLastTab)) {
			someNotOpened = true;
		}
	}

	if (someNotOpened) {
		ModalDialogBuilder::showMessage(QObject::tr("The document could not be opened because it is not created by current user").toStdString());
		return;
	}

	if (view) view->close();
}

void BrowserPresenter::editPatientData()
{
	if (m_selectedInstances.empty()) return;

	auto patient = DbPatient::get(m_selectedInstances[0]->patientRowId);

	PatientDialogPresenter d(patient);
	auto result = d.open();

	if (result) {
		refreshModel();
	}
}
