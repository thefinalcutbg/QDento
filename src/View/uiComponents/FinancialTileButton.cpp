#include "FinancialTileButton.h"
#include "Model/Financial/Recipient.h"
#include "View/Theme.h"
#include "Model/Financial/Issuer.h"

void RecipientTileButton::setRecipient(const Recipient& r)
{
	this->name = QString::fromStdString(r.name);
	this->id = QString::fromStdString(r.identifier);
	this->address = QString::fromStdString(r.address);
	this->phone = QString::fromStdString(r.phone);

	repaint();
}


void RecipientTileButton::paintInfo(QPainter* painter)
{
	constexpr int rowYPos[4]{ 60,80,100,120 };

	painter->setFont(infoLabel);
	painter->drawText(20, rowYPos[0], tr("Name: "));
	painter->drawText(20, rowYPos[1], tr("Identifier: "));
	painter->drawText(20, rowYPos[2], tr("Address: "));
	painter->drawText(20, rowYPos[3], tr("Phone Number: "));

	QFontMetrics metric(infoLabel);

	auto horizontalAdvance = [metric](const QString& label) {
		return metric.horizontalAdvance(label);
	};

	painter->setFont(info);
	painter->drawText(20 + horizontalAdvance(tr("Name: ")), rowYPos[0], name);
	painter->drawText(20 + horizontalAdvance(tr("Identifier: ")), rowYPos[1], id);
	painter->drawText(20 + horizontalAdvance(tr("Address: ")), rowYPos[2], address);
	painter->drawText(20 + horizontalAdvance(tr("Phone Number: ")), rowYPos[3], this->phone);

	painter->setFont(header);
	painter->setPen(QPen(animatedColor(Theme::fontRed, Theme::fontRedClicked)));
	painter->drawText(20, 30, tr("Recipient"));
}

void IssuerTileButton::paintInfo(QPainter* painter)
{

	constexpr int rowYPos[4]{ 60,80,100,120 };

	painter->setFont(infoLabel);
	painter->drawText(20, rowYPos[0], tr("Name: "));
	painter->drawText(20, rowYPos[1], tr("Identifier: "));
	painter->drawText(20, rowYPos[2], tr("Address: "));
	//painter->drawText(20, rowYPos[3], tr("Phone Number: "));

	QFontMetrics metric(infoLabel);

	auto horizontalAdvance = [metric](const QString& label) {
		return metric.horizontalAdvance(label);
	};

	painter->setFont(info);
	painter->drawText(20 + horizontalAdvance(tr("Name: ")), rowYPos[0], name);
	painter->drawText(20 + horizontalAdvance(tr("Identifier: ")), rowYPos[1], id);
	painter->drawText(20 + horizontalAdvance(tr("Address: ")), rowYPos[2], address);
	//painter->drawText(20 + horizontalAdvance(tr("Phone Number: ")), rowYPos[3], this->phone);

	painter->setFont(header);
	painter->setPen(QPen(animatedColor(Theme::fontRed, Theme::fontRedClicked)));
	painter->drawText(20, 30, tr("Issuer"));

}

void IssuerTileButton::setIssuer(const Issuer& r)
{
	this->name = QString::fromStdString(r.company_name);
	this->id = QString::fromStdString(r.identifier);
	this->address = QString::fromStdString(r.address);
	
	repaint();
}





