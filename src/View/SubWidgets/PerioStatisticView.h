#pragma once

#include <QWidget>
#include "ui_PerioStatisticView.h"
#include "View/uiComponents/RoundedFrame.h"

class HexagonGraphicsItem;
class PerioStatistic;

class PerioStatisticView : public RoundedFrame
{
	Q_OBJECT

	HexagonGraphicsItem* hexagonGraphicsItem;

public:
	PerioStatisticView(QWidget *parent = nullptr);
	void setPerioStatistic(const PerioStatistic& stat, const PerioStatistic* const prev = nullptr);
	~PerioStatisticView();

private:
	Ui::PerioStatisticViewClass ui;
};
