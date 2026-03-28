#include <QFileDialog>

#include "FinancialView.h"
#include "Model/Financial/Invoice.h"
#include "View/Theme.h"
#include "View/GlobalFunctions.h"
#include "View/uiComponents/MouseWheelGuard.h"


FinancialView::FinancialView(QWidget *parent)
	: ShadowBakeWidget(parent)
{
	ui.setupUi(this);

	setShadowTargets({ ui.tableFrame, ui.headerFrame });

	ui.tableFrame->setDynamicFocusBorderChange();
	ui.headerFrame->addVerticalSeparator(ui.recipientButton->width());
	ui.headerFrame->setFrameColor(Theme::border);

	ui.numberSpinBox->setTotalLength(10);

	ui.numberSpinBox->installEventFilter(new MouseWheelGuard(ui.numberSpinBox));

	ui.addButton->setIcon(QIcon(":/icons/icon_add.png"));
	ui.deleteButton->setIcon(QIcon(":/icons/icon_remove.png"));
	ui.editButton->setIcon(QIcon(":/icons/icon_edit.png"));
	ui.printButton->setIcon(QIcon(":/icons/icon_print.png"));

    ui.addButton->setHoverColor(Theme::mainBackgroundColor);
    ui.deleteButton->setHoverColor(Theme::mainBackgroundColor);
    ui.editButton->setHoverColor(Theme::mainBackgroundColor);
	ui.printButton->setHoverColor(Theme::mainBackgroundColor);

	ui.operationsTable->setModel(&m_model);
	ui.operationsTable->setBusinessOperationLayout();
	ui.operationsTable->setStyleSheet(
		"color :" + Theme::colorToString(Theme::fontTurquoise) + "; "
		"selection-color:" + Theme::colorToString(Theme::fontTurquoiseClicked) + "; "
		"selection-background-color: " + Theme::colorToString(Theme::background) + "; "
	);

	setStyleSheet(Theme::getFancyStylesheet());

	ui.operationsLabel->setStyleSheet(
		"color : " + Theme::colorToString(Theme::fontTurquoise) + "; "
		 "font-weight: bold; font-size: 12px;"
	);


	ui.mainDocNumSpin->setTotalLength(10);

    connect(ui.recipientButton, &QPushButton::clicked, this, [=, this] {presenter->editRecipient();});
	connect(ui.issuerButton, &QPushButton::clicked, this, [=, this] {presenter->editIssuer(); });
	connect(ui.printButton, &QPushButton::clicked, this, [=, this] {presenter->print(); });

    connect(ui.dateEdit, &QDateEdit::dateChanged, this,
        [=, this](QDate date){
			if (presenter == nullptr) return;
			presenter->dateChanged(Date(date.day(), date.month(), date.year()));
		});

    connect(ui.taxEventDateEdit, &QDateEdit::dateChanged, this,
        [=, this](QDate date) {
			if (presenter == nullptr) return;
			presenter->taxEventDateChanged(Date(date.day(), date.month(), date.year()));
		});

    connect(ui.paymentTypeCombo, &QComboBox::currentIndexChanged, this,
        [=, this](int index) {
			presenter->paymentTypeChanged(static_cast<PaymentType>(index));
		});

    connect(ui.numberSpinBox, &LeadingZeroSpinBox::valueChanged, this, [=, this](long long num) {if (presenter)presenter->invoiceNumberChanged(num);});

    connect(ui.deleteButton, &QPushButton::clicked, this,
        [=, this] {

			if (!presenter) return;

			int currentIdx = ui.operationsTable->selectedRow();
			int lastIdx = ui.operationsTable->verticalHeader()->count() - 1;

			presenter->removeOperation(currentIdx);
			
			if (currentIdx == lastIdx)
			{
				ui.operationsTable->selectRow(currentIdx - 1);
			}
			else ui.operationsTable->selectRow(currentIdx);
			
		});


    connect(ui.editButton, &QPushButton::clicked, this, [=, this]{ if (presenter) presenter->editOperation(ui.operationsTable->selectedRow());});

    connect(ui.addButton, &QAbstractButton::clicked, this, [=, this] { if (presenter) presenter->addOperation(); });

    connect(ui.docTypeCombo, &QComboBox::currentIndexChanged, this, [=, this](int idx) { presenter->docTypeChanged(idx);});
	
    connect(ui.mainDocDateEdit, &QDateEdit::dateChanged, this, [=, this] (QDate d) {
		presenter->mainDocumentChanged(ui.mainDocNumSpin->value(), Date(d.day(), d.month(), d.year()));
		});
    connect(ui.mainDocNumSpin, &LeadingZeroSpinBox::valueChanged, this, [=, this](long long value) {
		auto d = ui.mainDocDateEdit->date();
		presenter->mainDocumentChanged(value, Date(d.day(), d.month(), d.year()));
		});

    connect(ui.operationsTable, &TableView::deletePressed, this, [=, this](int index) { if (presenter) presenter->removeOperation(index); });
    connect(ui.operationsTable, &TableView::editPressed, this, [=, this](int index) { if (presenter) presenter->editOperation(index); });

    connect(ui.vatCheckBox, &QCheckBox::checkStateChanged, this, [=, this](Qt::CheckState state) {
        if(presenter) presenter->vatChanged(state == Qt::CheckState::Checked);
    });
}

FinancialView::~FinancialView()
{
}

void FinancialView::setPresenter(FinancialPresenter* presenter)
{
	this->presenter = presenter;
}

void FinancialView::setInvoice(const Invoice& inv)
{
    auto issuer = inv.issuer();

    ui.issuerButton->setIssuer(issuer);
	ui.recipientButton->setRecipient(inv.recipient);

    QSignalBlocker blockers[5]{
		QSignalBlocker{ui.dateEdit},
		QSignalBlocker{ui.taxEventDateEdit},
		QSignalBlocker{ui.paymentTypeCombo},
        QSignalBlocker{ui.docTypeCombo},
        QSignalBlocker{ui.vatCheckBox}
	};

	ui.dateEdit->setDate(QDate{ inv.date.year, inv.date.month, inv.date.day });

	auto& d = inv.taxEventDate;
	ui.taxEventDateEdit->setDate(QDate(d.year, d.month, d.day));

	ui.paymentTypeCombo->setCurrentIndex(static_cast<int>(inv.paymentType));



	ui.docTypeCombo->setCurrentIndex(static_cast<int>(inv.type));

    ui.vatCheckBox->setChecked(inv.VAT);

	setMainDocument(inv.mainDocument());

	//centering the label:

	QPushButton* layoutButtons[3]{ ui.addButton, ui.deleteButton, ui.editButton };

	int buttonsSumWidth = 0;

	for (auto button : layoutButtons) {
		if (!button->isHidden())
			buttonsSumWidth += button->width();
	}

	//ui.opLabelSpacer->changeSize(buttonsSumWidth, 0);


    setBusinessOperations(inv.businessOperations, inv.amount(), inv.VAT);

}

void FinancialView::setBusinessOperations(const BusinessOperations& businessOp, double amount, int VAT)
{
	m_model.setBusinessOperations(businessOp);

	ui.priceLabel->setText(priceToString(amount));

	QString vatText = tr("VAT") + " " + QString::number(VAT) + "%:" ;

    ui.vatCheckBox->setText( vatText );

	auto vatTax = amount * VAT / 100;

    ui.vatLabel->setText(priceToString(vatTax));

    if(VAT){
		amount = amount + vatTax;
    }

	ui.sumLabel->setText(priceToString(amount));
	update();
}

void FinancialView::setMainDocument(const std::optional<MainDocument>& mainDoc)
{
	if (mainDoc) {

		QSignalBlocker blockers[2]{
			QSignalBlocker(ui.mainDocDateEdit), 
			QSignalBlocker(ui.mainDocNumSpin) 
		};

		ui.mainDocNumSpin->setValue(mainDoc->number);
		ui.mainDocDateEdit->setDate(QDate(mainDoc->date.year, mainDoc->date.month, mainDoc->date.day));
		showMainDocumentDetails(true);
	}
	else {
		showMainDocumentDetails(false);
	}

}

void FinancialView::setNumberSpinBox(long long num)
{
	QSignalBlocker b(ui.numberSpinBox);
	ui.numberSpinBox->setValue(num);
}

void FinancialView::showMainDocumentDetails(bool show)
{
	QWidget* const mainDocWidgets[4]{

		ui.mainDocNumLabel, 
		ui.mainDocNumSpin, 
		ui.mainDocDateLabel, 
		ui.mainDocDateEdit 
	};

	for (auto& w : mainDocWidgets)
		w->setHidden(!show);
}
