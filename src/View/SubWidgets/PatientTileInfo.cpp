#include "PatientTileInfo.h"

#include <QMenu>

#include "Presenter/PatientInfoPresenter.h"
#include "View/Theme.h"

PatientTileInfo::PatientTileInfo(QWidget *parent)
	: RoundedFrame(parent)
{
	ui.setupUi(this);

	setFrameColor(Theme::border);

    //init context menu
    context_menu = new QMenu(this);

    QAction* action;

    action = (new QAction(tr("Edit"), context_menu));
    connect(action, &QAction::triggered, this, [=, this] { ui.patientTile->click(); });

    action->setIcon(QIcon(":/icons/icon_edit.png"));
    context_menu->addAction(action);

    action = (new QAction(tr("New Dental Visit"), context_menu));
    connect(action, &QAction::triggered, this, [=, this] { presenter->openDocument(TabType::DentalVisit); });
    action->setIcon(QIcon(":/icons/icon_sheet.png"));
    context_menu->addAction(action);

    action = (new QAction(tr("New Periodontal Measurment"), context_menu));
    connect(action, &QAction::triggered, this, [=, this] { presenter->openDocument(TabType::PerioStatus); });
    action->setIcon(QIcon(":/icons/icon_periosheet.png"));
    context_menu->addAction(action);

    action = (new QAction(tr("New Invoice"), context_menu));
    connect(action, &QAction::triggered, this, [=, this] { presenter->openDocument(TabType::Financial); });
    action->setIcon(QIcon(":/icons/icon_invoice.png"));
    context_menu->addAction(action);

    action = (new QAction(tr("Schedule and Appointment"), context_menu));
    connect(action, &QAction::triggered, this, [=, this] { presenter->openDocument(TabType::Calendar); });
    action->setIcon(QIcon(":/icons/icon_calendar.png"));
    context_menu->addAction(action);

    action = (new QAction(tr("Patient History"), context_menu));
    connect(action, &QAction::triggered, this, [=, this] { presenter->openDocument(TabType::PatientSummary); });
    action->setIcon(QIcon(":/icons/icon_history.png"));
    context_menu->addAction(action);

    context_menu->setStyleSheet(Theme::getPopupMenuStylesheet());

    //connect signalsh

    connect(ui.patientTile, &QPushButton::clicked, this, [=, this] {
		if (presenter) presenter->patientTileClicked();
	});

	connect (ui.patientTile->notesButton, &QPushButton::clicked, this, [=, this] {
		if (presenter) presenter->notesRequested();
	});

	connect (ui.patientTile->appointmentButton, &QPushButton::clicked, this, [=, this] {
		if (presenter) presenter->appointmentClicked();
	});

    connect (ui.patientTile->notificationButton, &QPushButton::clicked, this, [=, this]{
        if (presenter) presenter->notificationClicked();
    });

    connect(ui.patientTile, &TileButton::customContextMenuRequested, this, [&](QPoint point) {
        context_menu->popup(point);
    });

}

void PatientTileInfo::setPatient(const Patient& p, int age)
{
	ui.patientTile->setData(p, age);
}

PatientTileInfo::~PatientTileInfo()
{}
