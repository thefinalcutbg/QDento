#pragma once

#include <QWidget>
#include "ui_FinancialView.h"
#include "Presenter/FinancialPresenter.h"
#include "View/uiComponents/ShadowBakeWidget.h"
#include "View/TableModels/BusinessOperationModel.h"

class FinancialView : public ShadowBakeWidget
{
	Q_OBJECT

	FinancialPresenter* presenter{nullptr};

	BusinessOperationModel m_model;

    void showMainDocumentDetails(bool show);

public:
	FinancialView(QWidget *parent = Q_NULLPTR);

	void setPresenter(FinancialPresenter* presenter);
	void setInvoice(const Invoice& inv);
    void setBusinessOperations(const BusinessOperations& businessOp, double amount, int VAT);
    void setMainDocument(const std::optional<MainDocument>& mainDoc);
	void setNumberSpinBox(long long num);

	~FinancialView();

private:
	Ui::FinancialView ui;
};
