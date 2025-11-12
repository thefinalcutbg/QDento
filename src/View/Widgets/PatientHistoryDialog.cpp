#include "PatientHistoryDialog.h"
#include "Presenter/PatientHistoryPresenter.h"
#include "View/GlobalFunctions.h"
#include <set>
#include <array>

PatientHistoryDialog::PatientHistoryDialog(PatientHistoryPresenter& p, QWidget *parent)
	: presenter(p), QDialog(parent)
{
	ui.setupUi(this);

	setWindowTitle("Patient History");
	setWindowIcon(QIcon(":/icons/icon_history.png"));
	setWindowFlag(Qt::WindowMaximizeButtonHint);

	ui.tabWidget->setCurrentIndex(0);

	ui.perioTab->hide();

	ui.procedureTable->setModel(&procedure_model);
	ui.docView->setModel(&doc_model);
	ui.docDetailsView->setModel(&doc_details_model);

	//init procedure table
	ui.procedureTable->setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);

	ui.procedureTable->hideColumn(0);
	ui.procedureTable->hideColumn(7);

	ui.procedureTable->setColumnWidth(1, 100);
	ui.procedureTable->setColumnWidth(2, 70);
	ui.procedureTable->setColumnWidth(3, 200);
	ui.procedureTable->setColumnWidth(4, 65);
	ui.procedureTable->setColumnWidth(5, 200);
	ui.procedureTable->setColumnWidth(6, 100);
	ui.procedureTable->setColumnWidth(8, 200);
	
	setTableViewDefaults(ui.procedureTable);
	setTableViewDefaults(ui.docDetailsView);
	setTableViewDefaults(ui.docView);

	ui.docView->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
	

	connect(ui.docView->selectionModel(), &QItemSelectionModel::selectionChanged, this, [&](const QItemSelection&, const QItemSelection&) {

		auto idx_rows = ui.docView->selectionModel()->selectedRows();

		std::set<int> rows;

		for (auto& idx : idx_rows) {
			rows.insert(idx.row());
		}

		if (rows.size() != 1) {
			doc_details_model.setTableData({});
			return;
		}

		auto currentRow = *rows.begin();

		doc_details_model.setTableData(details_data[currentRow]);

		for (int i = 0; i < details_data[currentRow].size(); i++) {

			ui.docDetailsView->setColumnHidden(i, details_data[currentRow][i].hidden);

			ui.docDetailsView->setColumnWidth(i, details_data[currentRow][i].width);
		}
	});

	connect(ui.snapshotViewer->getTeethScene(), &TeethViewScene::toothDoubleClicked, this,[&](int idx) {

			presenter.toothHistoryRequested(idx);
	});

	connect(ui.openDocButton, &QPushButton::clicked, this, [&] {

		auto idx_rows = ui.docView->selectionModel()->selectedRows();

		std::set<int> rows;

		for (auto& idx : idx_rows) {
			rows.insert(idx.row());
		}

		std::vector<int> docsToOpen;

		for (auto& row : rows) {
			docsToOpen.push_back(row);
		}

		presenter.openDocuments(docsToOpen);

	});

	connect(ui.docView, &QTableView::doubleClicked, this, [&] { ui.openDocButton->click(); });

}

void PatientHistoryDialog::setProcedures(const std::vector<Procedure> procedures)
{
	procedure_model.setProcedures(procedures);
}

void PatientHistoryDialog::setDocuments(const PlainTable& docList, const std::vector<PlainTable>& contents)
{
	doc_model.setTableData(docList);

	for (int i = 0; i < docList.size(); i++) {

		ui.docView->setColumnHidden(i, docList[i].hidden);

		ui.docView->setColumnWidth(i, docList[i].width);
	}

	details_data = contents;

}

void PatientHistoryDialog::setSnapshots(const std::vector<Snapshot>& snapshots)
{
	ui.snapshotViewer->setSnapshots(snapshots);
}

void PatientHistoryDialog::setPerioSnapshots(const std::vector<PerioSnapshot>& snapshots)
{

	if (snapshots.empty()) {
		ui.tabWidget->tabBar()->removeTab(PERIO_TAB_INDEX);
		return;
	}
	ui.perioTab->setSnapshots(snapshots);
}

void PatientHistoryDialog::setPatientNoteFlags(const std::array<std::string, 32>& notes)
{
	ui.snapshotViewer->getTeethScene()->setNotes(notes);
}

PatientHistoryDialog::~PatientHistoryDialog()
{}
