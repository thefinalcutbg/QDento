#include "QDento.h"

#include <QAction>
#include <QMenu>

#include <QPixmap>
#include <QFileDialog>
#include <QFontDatabase>
#include <QShortcut>
#include <QStatusBar>
#include <QTimer>
#include <QDesktopServices>

#include "Presenter/MainPresenter.h"

#include "Model/User.h"
#include "View/Theme.h"
#include "View/Widgets/GlobalWidgets.h"
#include "View/Widgets/AboutDialog.h"
#include "View/Widgets/SplashScreen.h"
#include "View/Widgets/NotificationListDialog.h"

#include "Version.h"

#include "Database/DbNotification.h"

#ifdef Q_OS_WIN
#include <QWindow>
#include <windows.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#endif

QDento::QDento(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    setWindowState(Qt::WindowMaximized);

#ifdef Q_OS_WIN
//change the default titlebar (windows 11 only)

/*
    auto& tbClr = Theme::mainBackgroundColor;
    auto tbTxt = QColor(Qt::black);

    const COLORREF rgb = RGB(tbClr.red(), tbClr.green(), tbClr.blue());

    const DWORD dwmCaptionAttr = 35;
    const DWORD dwmTextAttr = 36;

	auto hwnd = reinterpret_cast<HWND>(windowHandle()->winId());

    DwmSetWindowAttribute(hwnd, dwmCaptionAttr, &rgb, sizeof(rgb));

    const COLORREF white = RGB(tbTxt.red(), tbTxt.green(), tbTxt.blue());
    DwmSetWindowAttribute(hwnd, dwmTextAttr, &white, sizeof(white));
 */
#endif

    GlobalWidgets::mainWindow = this;
    GlobalWidgets::statusBar = statusBar();

    statusBar()->setStyleSheet("font-weight: bold; color:" + Theme::colorToString(Theme::fontTurquoiseClicked));

    QAction* settingsAction = new QAction(tr("Settings"));
    settingsAction->setIcon(QIcon(":/icons/icon_settings.png"));
    QAction* exitAction = new QAction(tr("Exit"));
    exitAction->setIcon(QIcon(":/icons/icon_remove.png"));
    QMenu* userMenu = new QMenu(this);
    userMenu->addAction(settingsAction);
    userMenu->addAction(exitAction);
    userMenu->setStyleSheet(Theme::getPopupMenuStylesheet());

    //setting global shortcuts
    auto shortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_S), this);
    shortcut->setContext(Qt::WidgetWithChildrenShortcut);
    QObject::connect(shortcut, &QShortcut::activated, [&] { MainPresenter::get().save(); });

    shortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_O), this);
    shortcut->setContext(Qt::WidgetWithChildrenShortcut);
    QObject::connect(shortcut, &QShortcut::activated, [&] { MainPresenter::get().showBrowser(); });

    shortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_N), this);
    shortcut->setContext(Qt::WidgetWithChildrenShortcut);
    QObject::connect(shortcut, &QShortcut::activated, [&] { MainPresenter::get().newAmbPressed(); });

    shortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_M), this);
    shortcut->setContext(Qt::WidgetWithChildrenShortcut);
    QObject::connect(shortcut, &QShortcut::activated, [&] { MainPresenter::get().newPerioPressed(); });

    shortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_F), this);
    shortcut->setContext(Qt::WidgetWithChildrenShortcut);
    QObject::connect(shortcut, &QShortcut::activated, [&] { MainPresenter::get().newInvoicePressed(); });

    //setting buttons
    ui.newButton->setIcon(QIcon(":/icons/icon_sheet.png"));
    ui.perioButton->setIcon(QIcon(":/icons/icon_periosheet.png"));
    ui.saveButton->setIcon(QIcon(":/icons/icon_save.png"));
    ui.browserButton->setIcon(QIcon(":/icons/icon_open.png"));
    ui.settingsButton->setIcon(QIcon(":/icons/icon_settings.png"));
    ui.calendarButton->setIcon(QIcon(":/icons/icon_calendar.png"));
    ui.invoiceButton->setIcon(QIcon(":/icons/icon_invoice.png"));
    ui.aboutButton->setIcon(QIcon(":/icons/icon_question.png"));
    ui.notifButton->setIcon(QIcon(":/icons/icon_bell.png"));
    ui.donateButton->setIcon(QIcon(":/icons/icon_donate.png"));
    ui.notifButton->setMonochrome(true);
     
    connect(ui.donateButton, &QPushButton::clicked, [&] { QDesktopServices::openUrl(QUrl("https://www.paypal.com/donate/?hosted_button_id=WJBJECQ247WN6", QUrl::TolerantMode)); });
    connect(ui.newButton, &QPushButton::clicked, [&] { MainPresenter::get().newAmbPressed(); });
    connect(ui.saveButton, &QPushButton::clicked, [&] { MainPresenter::get().save(); });
    connect(ui.browserButton, &QPushButton::clicked, [&] { MainPresenter::get().showBrowser(); });
    connect(ui.perioButton, &QPushButton::clicked, [&] { MainPresenter::get().newPerioPressed(); });
    connect(ui.calendarButton, &QPushButton::clicked, [&] { MainPresenter::get().openCalendar(); });
    connect(settingsAction, &QAction::triggered, [&] { MainPresenter::get().userSettingsPressed();});
    connect(ui.settingsButton, &QPushButton::clicked, [&] { MainPresenter::get().settingsPressed();});
    connect(ui.invoiceButton, &QPushButton::clicked, [&] { MainPresenter::get().newInvoicePressed(); });
    connect(ui.aboutButton, &QPushButton::clicked, this, [&] { AboutDialog d; d.exec(); });
	connect(ui.userButton, &QPushButton::clicked, [&] { ui.userButton->showMenu(); });
    connect(exitAction, &QAction::triggered, [&] { MainPresenter::get().logOut(); });

    connect(ui.notifButton, &QPushButton::clicked, this, [&]{ MainPresenter::get().notificationPressed();});

    ui.userButton->setMenu(userMenu);
    ui.userButton->setIconSize(QSize(25, 25));

    ui.userButton->setIcon(QIcon{":/icons/icon_user.png"});

    SplashScreen::hideAndDestroy();

    ui.tabView->showWelcomeScreen();

    MainPresenter::get().setView(this);

}

TabView* QDento::tabView()
{
    return ui.tabView;
}

void QDento::setUserLabel(const std::string& doctorName, const std::string& practiceName)
{
    ui.userButton->setText(QString::fromStdString(doctorName));

    QString title = "QDento v";
    title += Version::current().toString().c_str();
    title += " ";
    title += practiceName.c_str();

    setWindowTitle(title);
}

void QDento::exitProgram()
{
    QApplication::quit();
}

bool QDento::initialized()
{
    return m_loggedIn;
}

void QDento::disableButtons(bool saveDisabled)
{
    ui.saveButton->setDisabled(saveDisabled);
}

void QDento::paintEvent(QPaintEvent*)
{
    QPainter painter;
    painter.begin(this);
    painter.fillRect(rect(), Theme::mainBackgroundColor);
    painter.fillRect(0, height() - 21, width(), 21, QColor(240, 240, 240));
    painter.end();
}

void QDento::closeEvent(QCloseEvent* event)
{
    if (!MainPresenter::get().closeAllTabs())
        event->ignore();

    foreach(QWidget * widget, QApplication::topLevelWidgets()) 
    {
        if (widget == this) continue;
        widget->close();
    }
}

void QDento::setNotificationIcon(int activeNotifCount)
{
    ui.notifButton->setMonochrome(!activeNotifCount);
    ui.notifButton->setIcon(activeNotifCount ? QIcon(":/icons/icon_bell_notify.png") : QIcon(":/icons/icon_bell.png"));

    switch(activeNotifCount){
        case 0: ui.notifButton->setToolTip(tr("No active reminders")); break;
        case 1: ui.notifButton->setToolTip(tr("1 active reminder")); break;
        default: ui.notifButton->setToolTip(QString::number(activeNotifCount) + tr(" active reminders"));
    }
}

QDento::~QDento()
{
    Theme::cleanUpFusionStyle();
}
