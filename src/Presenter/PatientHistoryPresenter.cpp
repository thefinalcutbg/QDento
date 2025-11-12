#include "PatientHistoryPresenter.h"
#include "Database/DbProcedure.h"
#include "Database/DbInvoice.h"
#include "Database/DbBrowser.h"
#include "Database/DbPatientSummary.h"
#include "View/ModalDialogBuilder.h"
#include "Model/User.h"
#include "Presenter/DetailedStatusPresenter.h"
#include "Presenter/TabPresenter.h"
#include <QObject>

PatientHistoryPresenter::PatientHistoryPresenter(Patient& patient) :
	patient(patient),

	local_history(DbProcedure::getPatientProcedures(patient.rowid)),
	view(*this)

{
	PlainTable docTable;

	std::tie(this->documents, docTable) = DbBrowser::getPatientDocuments(patient.rowid);
	
	view.setProcedures(local_history);

	std::vector<PlainTable> doc_details;

	for (size_t i = 0; i < documents.size(); i++)
	{
		auto docRowid = documents[i].rowID;

		switch (documents[i].type)
		{
			case TabType::DentalVisit: doc_details.emplace_back(DbProcedure::getProcedures(docRowid)); break;
			case TabType::Financial: doc_details.emplace_back(DbInvoice::getInvoice(docRowid).businessOperations); break;
			case TabType::PerioStatus: doc_details.emplace_back(PlainTable{}); break;
		}
	}

	view.setDocuments(docTable, doc_details);

	//init shapshots:

	ToothContainer lastToothStatus;
	
	std::vector<PerioSnapshot> perioSnapshots;

	for (auto& frame : DbPatientSummary::getFrames(patient.rowid)) {
		
		if (frame.type == TimeFrameType::Perio) {

			perioSnapshots.push_back(PerioSnapshot{
				.perioStatus = frame.perioData,
				.toothStatus = lastToothStatus,
				.perioStatistic = PerioStatistic(frame.perioData, patient.getAge(frame.perioData.date))
			});

			continue;
		}

		if (frame.type == TimeFrameType::InitialAmb) {
			local_snapshots.emplace_back(frame.teeth, frame.date);
			local_snapshots.back().procedure_name = QObject::tr("Dental visit ").toStdString() + frame.number + QObject::tr(" (initial oral status)").toStdString();
			lastToothStatus = local_snapshots.back().teeth;
			continue;
		}

		//TimeFrameType::Procedures
		for (auto& p : frame.procedures) {

			local_snapshots.emplace_back(p, local_snapshots.back().teeth);

			if (local_snapshots.back().affected_teeth.empty()) {
				local_snapshots.pop_back(); //because it changes nothing
				lastToothStatus = local_snapshots.back().teeth;
			}

		}
	}
	view.setSnapshots(local_snapshots);
	view.setPerioSnapshots(perioSnapshots);

}

void PatientHistoryPresenter::openDocuments(const std::vector<int>& selectedDocIdx)
{
	if (selectedDocIdx.empty()) return;

	bool someNotOpened = false;

	for (int i = 0; i < selectedDocIdx.size(); i++) {

		auto idx = selectedDocIdx[i];

		if (!TabPresenter::get().open(
			documents[idx],
			i == selectedDocIdx.size() - 1)
			)
		{
			someNotOpened = true;
		};
	}

	if (someNotOpened) {
		ModalDialogBuilder::showMessage(QObject::tr("The document could not be opened because it is not created by the current user.").toStdString());
		return;
	}

	view.close();
}

void PatientHistoryPresenter::toothHistoryRequested(int toothIdx)
{
	if (toothIdx < 1 || toothIdx > 32) return;

	DetailedStatusPresenter d(
		toothIdx,
		patient.rowid,
		DbProcedure::getToothProcedures(patient.rowid, toothIdx)
	);

	d.open();

	patient.teethNotes[toothIdx] = d.getNote();

	view.setPatientNoteFlags(patient.teethNotes);
}

void PatientHistoryPresenter::openDialog()
{
	view.setWindowTitle(
		view.windowTitle() + " - " +
		patient.firstLastName().c_str()
	);
	
	view.exec();
}



