#include "WelcomeWidget.h"
#include "View/Theme.h"
#include "Presenter/MainPresenter.h"
#include <QDesktopServices>
#include <QDate>

#include "View/Widgets/AboutDialog.h"

WelcomeWidget::WelcomeWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

    auto date = Date::currentDate();

    ui.cornerLabel->setPixmap(QPixmap(":/icons/qDento.png"));


	setStyleSheet("background-color:" + Theme::colorToString(Theme::background));
    setStyleSheet("color: " + Theme::colorToString(Theme::fontTurquoise));

    ui.ambButton->setIcon(QIcon(":/icons/icon_sheet.png"));
    ui.perioButton->setIcon(QIcon(":/icons/icon_periosheet.png"));
    ui.invoiceButton->setIcon(QIcon(":/icons/icon_invoice.png"));
    ui.browser->setIcon(QIcon(":/icons/icon_open.png"));
    ui.settingsButton->setIcon(QIcon(":/icons/icon_settings.png"));
    ui.calendar->setIcon(QIcon(":/icons/icon_calendar.png"));
    ui.donateButton->setIcon(QIcon(":/icons/icon_donate.png"));
    ui.notifButton->setIcon(QIcon(":/icons/icon_bell.png"));
    ui.aboutButton->setIcon(QIcon(":/icons/icon_question.png"));

    connect(ui.ambButton, &QPushButton::clicked, this, [&] { MainPresenter::get().newAmbPressed(); });
    connect(ui.perioButton, &QPushButton::clicked, this, [&] { MainPresenter::get().newPerioPressed(); });
    connect(ui.invoiceButton, &QPushButton::clicked, this, [&] { MainPresenter::get().newInvoicePressed(); });
    connect(ui.browser, &QPushButton::clicked, this, [&] { MainPresenter::get().showBrowser(); });
    connect(ui.settingsButton, &QPushButton::clicked, this, [&] { MainPresenter::get().settingsPressed(); });
    connect(ui.calendar, &QPushButton::clicked, this, [&] { MainPresenter::get().openCalendar(); });
    connect(ui.donateButton, &QPushButton::clicked, this, [&] { QDesktopServices::openUrl(QUrl("https://www.paypal.com/donate/?hosted_button_id=WJBJECQ247WN6", QUrl::TolerantMode)); });
    connect(ui.notifButton, &QPushButton::clicked, this, [&] { MainPresenter::get().notificationPressed(); });
    connect(ui.aboutButton, &QPushButton::clicked, this, [&] { AboutDialog d; d.exec(); });
}

WelcomeWidget::~WelcomeWidget()
{}
