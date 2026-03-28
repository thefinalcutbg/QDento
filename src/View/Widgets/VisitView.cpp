#include "VisitView.h"

#include "View/SubWidgets/ContextMenu.h"
#include "View/Theme.h"
#include "View/uiComponents/MouseWheelGuard.h"
#include "View/GlobalFunctions.h"
#include "Model/User.h"
#include <QIcon>

VisitView::VisitView(QWidget* parent)
	: ShadowBakeWidget(parent), presenter(nullptr)
{
	ui.setupUi(this);

	setStyleSheet(Theme::getFancyStylesheet());

	setShadowTargets({
		ui.patientInfoTile,
		ui.frame,
		ui.procedureFrame
	});

	ui.procedureFrame->setDynamicFocusBorderChange();
	ui.frame->setDynamicFocusBorderChange();
	ui.frame->addVerticalSeparator(ui.teethView->width());

	teethViewScene = new TeethViewScene(ui.teethView);
	contextMenu = new ContextMenu();
	teethViewScene->setContextMenu(contextMenu);

	ui.teethView->setScene(teethViewScene);
	ui.teethView->setSceneRect(teethViewScene->sceneRect());
	ui.teethView->installEventFilter(this);
	ui.procedureTable->setModel(&model);
	ui.procedureTable->setAmbListLayout();
	ui.procedureTable->hideColumn(1);
	ui.addProcedure->setIcon(QIcon(":/icons/icon_add.png"));
	ui.statusResultButton->setIcon(QIcon(":/icons/icon_apply.png"));
	ui.deleteProcedure->setIcon(QIcon(":/icons/icon_remove.png"));
	ui.editProcedure->setIcon(QIcon(":/icons/icon_edit.png"));
	ui.historyButton->setIcon(QIcon(":/icons/icon_history.png"));
    ui.perioButton->setIcon(QIcon(":/icons/icon_add.png"));
    ui.invoiceButton->setIcon(QIcon(":/icons/icon_add.png"));

	ui.perioButton->setHoverColor(Theme::mainBackgroundColor);
	ui.invoiceButton->setHoverColor(Theme::mainBackgroundColor);
	ui.addProcedure->setHoverColor(Theme::mainBackgroundColor);
	ui.deleteProcedure->setHoverColor(Theme::mainBackgroundColor);
	ui.editProcedure->setHoverColor(Theme::mainBackgroundColor);
	ui.historyButton->setHoverColor(Theme::mainBackgroundColor);
	ui.statusResultButton->setHoverColor(Theme::mainBackgroundColor);

	ui.procedureTable->setMinimumWidth(ui.teethView->width() + ui.controlPanel->width());

	connect(ui.dateEdit, &QDateEdit::dateChanged, this, [=, this]{ if (presenter) presenter->setAmbDate(ui.dateEdit->getDate()); });
    connect(ui.historyButton, &QPushButton::clicked, this, [=, this] { if (presenter) presenter->historyRequested(); });
	connect(ui.addProcedure, &QAbstractButton::clicked, this, [=, this] { if (presenter) presenter->addProcedure(); });
    connect(ui.editProcedure, &QPushButton::clicked, this, [=, this] { if (presenter) presenter->editProcedure(ui.procedureTable->selectedRow()); });
	connect(ui.statusResultButton, &QPushButton::clicked, this, [=, this] { if (presenter) presenter->showAppliedStatus(); });
    connect(ui.invoiceButton, &QPushButton::clicked, this, [=, this] { if (presenter) presenter->createInvoice(); });
    connect(ui.perioButton, &QPushButton::clicked, this, [=, this] { if (presenter) presenter->createPerioMeasurment(); });
	connect(ui.numberSpin, &QSpinBox::valueChanged, this, [=, this] (int value){ if (presenter) presenter->setAmbNumber(value); });
    connect(ui.deleteProcedure, &QAbstractButton::clicked, this, [=, this]
        {

			if (!presenter) return;

			int currentIdx = ui.procedureTable->selectedRow();
			int lastIdx = ui.procedureTable->verticalHeader()->count()-1;

//			if (currentIdx == -1) return;

			presenter->deleteProcedure(currentIdx);

			if (currentIdx == lastIdx)
			{
				ui.procedureTable->selectRow(currentIdx - 1);
			}
			else ui.procedureTable->selectRow(currentIdx);
		});

    connect(ui.procedureTable, &TableView::deletePressed, this, [=, this](int row) { ui.deleteProcedure->clicked(); });
    connect(ui.procedureTable, &TableView::editPressed, this, [=, this](int row) { if (presenter) presenter->editProcedure(row); });
    connect(ui.procedureTable, &TableView::rowDragged, this, [=, this] {

		int from = ui.procedureTable->selectedRow();
		int to = model.lastDroppedRowIndex();

		if (from == to) to--;

		if(presenter) presenter->moveProcedure(from, to);
	}
	);

	ui.controlPanel->hide();
	ui.surfacePanel->hide();
	ui.procedureTable->enableContextMenu(true);

	ui.procedureLabel->setStyleSheet(
		"color : " + Theme::colorToString(Theme::fontTurquoise) + "; "
		"font-weight: bold; font-size: 12px;"
	);

	ui.otherDocsLabel->setStyleSheet(
		"color : " + Theme::colorToString(Theme::fontTurquoise) + "; "
		"font-weight: bold; font-size: 12px;"
	);

}

void VisitView::setPresenter(VisitPresenter* presenter)
{
	this->presenter = presenter;
	teethViewScene->setPresenter(presenter);
	ui.controlPanel->setPresenter(presenter);
	contextMenu->setPresenter(presenter);
}

bool VisitView::eventFilter(QObject* obj, QEvent* event)
{
	if (obj != ui.teethView) return false;

	if (event->type() == QEvent::FocusOut)
	{
			m_teethViewFocused = false;
			ui.surfacePanel->drawFocused(false);
            teethViewScene->drawFocused(false);
			repaint();

	}
	else if (event->type() == QEvent::FocusIn)
	{
		m_teethViewFocused = true;
		ui.surfacePanel->drawFocused(true);
        teethViewScene->drawFocused(true);
		repaint();
	}


	return false;
}

void VisitView::focusTeethView()
{
	ui.teethView->setFocus();
}

void VisitView::setDate(const Date& date)
{
	QSignalBlocker b(ui.dateEdit);
	ui.dateEdit->set_Date(date);
}

void VisitView::setVisitNumber(int num)
{
	QSignalBlocker b(ui.numberSpin);
	ui.numberSpin->setValue(num);
}

void VisitView::setCheckModel(const CheckModel& checkModel, const CheckModel& dsnCheckModel)
{
	ui.controlPanel->setModel(checkModel, dsnCheckModel);
	contextMenu->setModel(checkModel, dsnCheckModel);
}

void VisitView::hideSurfacePanel(bool hidden)
{
	ui.surfacePanel->hidePanel(hidden);
	ui.controlPanel->hideCommonButtons(!hidden);
}

void VisitView::hideControlPanel(bool hidden)
{
	ui.controlPanel->setHidden(hidden);
}

SurfacePanel* VisitView::surfacePanel()
{
	return ui.surfacePanel;
}

PatientTileInfo* VisitView::tileInfo()
{
	return ui.patientInfoTile;
}

void VisitView::repaintTooth(const ToothPaintHint& tooth)
{
	teethViewScene->display(tooth);
    ui.teethView->setFocus();
}

void VisitView::setNotes(const std::array<std::string, 32>& notes)
{
	teethViewScene->setNotes(notes);
}

void VisitView::setSelectedTeeth(const std::vector<int>& selectedIndexes)
{
	teethViewScene->setSelectedTeeth(selectedIndexes);

	ui.teethView->update(); //the only way to update qgraphicsview without most of the bugs

}


void VisitView::setProcedures(const std::vector<Procedure>& m)
{
	model.setProcedures(m);

	double totalSum = 0;

	std::vector<int> proc_teeth;
	proc_teeth.reserve(32);

	for (auto& t : m) {
		for (auto i : t.getArrayIndexes()) {
			proc_teeth.push_back(i);
		}

		totalSum += t.price;
	}

	ui.totoalSumLabel->setText(tr("Total Price: ") + priceToString(totalSum));

	teethViewScene->setProcedures(proc_teeth);
}

VisitView::~VisitView()
{
	delete teethViewScene;
	delete contextMenu;
}
