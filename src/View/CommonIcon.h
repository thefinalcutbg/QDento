#pragma once

#include <QPixmap>
class QColor;

namespace CommonIcon
{
	enum Type { NOICON, PATIENT, BDAY, DENTALVISIT, PERIO, INVOICE, CALENDAR, PRINT, ADD, REMOVE, MAX_SIZE };
	const QPixmap& getPixmap(CommonIcon::Type t);
	QPixmap getIndicator(const QColor& color);
}