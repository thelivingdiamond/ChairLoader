#include "QDrawContext.h"
#include "PropertyTree/IDrawContext.h"
#include "PropertyTree/Rect.h"

#include "QPropertyTree.h"
#include "PropertyTree/Unicode.h"
#include "PropertyTree/Color.h"
#include <QPainter>
#include <QStyle>
#include <QStyleOption>
#include <QIcon>

namespace property_tree {

void drawRoundRectangle(QPainter& p, const QRect &_r, unsigned int color, int radius, int width)
{
	QRect r = _r;
	int dia = 2*radius;

	p.setPen(QColor(color));
	p.drawRoundedRect(r, dia, dia);
}

void fillRoundRectangle(QPainter& p, const QBrush& brush, const QRect& _r, const QColor& border, int radius)
{
	bool wasAntialisingSet = p.renderHints().testFlag(QPainter::Antialiasing);
	p.setRenderHints(QPainter::Antialiasing, true);

	p.setBrush(brush);
	p.setPen(QPen(border, 1.0));
	QRectF adjustedRect = _r;
	adjustedRect.adjust(0.5f, 0.5f, -0.5f, -0.5f);
	p.drawRoundedRect(adjustedRect, radius, radius);

	p.setRenderHints(QPainter::Antialiasing, wasAntialisingSet);
}

// ---------------------------------------------------------------------------

static QColor interpolate(const QColor& a, const QColor& b, float k)
{
	float mk = 1.0f - k;
	return QColor(a.red() * k  + b.red() * mk,
				  a.green() * k + b.green() * mk,
				  a.blue() * k + b.blue() * mk,
				  a.alpha() * k + b.alpha() * mk);
}

static QColor toQColor(const Color& color)
{
	return QColor(color.r, color.g, color.b, color.a);
}


QRect toQRect(const Rect& r)
{
	return QRect(r.x, r.y, r.w, r.h);
}

Rect fromQRect(const QRect& r)
{
	return Rect(r.left(), r.top(), r.width(), r.height());
}

QPoint toQPoint(const Point& p)
{
	return QPoint(p.x(), p.y());
}

Point fromQPoint(const QPoint& p)
{
	return Point(p.x(), p.y());
}

void QDrawContext::drawColor(const Rect& rect, const Color& color)
{
	painter_->save();
	painter_->setBrush(QBrush(toQColor(color)));
	painter_->setPen(QPen(tree_->palette().color(QPalette::Shadow)));
	QRect rt = toQRect(rect.adjusted(1,1,-1,-3));
	painter_->drawRoundedRect(rt, 2, 2);
	painter_->restore();
}

void QDrawContext::drawComboBox(const Rect& rect, const char* text)
{
	QStyleOptionComboBox option;
	option.editable = false;
	option.frame = true;
	option.currentText = QString(text);
	option.state |= QStyle::State_Enabled;
	option.rect = QRect(0, 0, rect.width(), rect.height());
	// we have to translate painter here to work around bug in some themes
	painter_->translate(rect.left(), rect.top());
	tree_->style()->drawComplexControl(QStyle::CC_ComboBox, &option, painter_);
	painter_->setPen(QPen(tree_->palette().color(QPalette::WindowText)));
	QRect textRect = tree_->style()->subControlRect(QStyle::CC_ComboBox, &option, QStyle::SC_ComboBoxEditField, 0);
	textRect.adjust(1, 0, -1, 0);
	tree_->_drawRowValue(*painter_, text, &tree_->font(), textRect, tree_->palette().color(QPalette::WindowText), false, false);
	painter_->translate(-rect.left(), -rect.top());
}

void QDrawContext::drawSelection(const Rect& rect, bool inlinedRow)
{
	if (inlinedRow) {
		QColor color1(tree_->palette().button().color());
		QColor color2(tree_->palette().highlight().color());
		QColor brushColor = tree_->hasFocusOrInplaceHasFocus() ? interpolate(color1, color2, 0.33f) : tree_->palette().shadow().color();
		fillRoundRectangle(*painter_, QBrush(brushColor), toQRect(rect), brushColor, 6);
	}
	else {
		QBrush brush(tree_->hasFocusOrInplaceHasFocus() ? tree_->palette().highlight() : tree_->palette().shadow());
		QColor brushColor = brush.color();
		QColor borderColor(brushColor.red(), brushColor.green(), brushColor.blue(), brushColor.alpha() / 4);
		fillRoundRectangle(*painter_, brush, toQRect(rect), borderColor, 6);
	}
}

void QDrawContext::drawRowLine(const Rect& rect)
{
	QLinearGradient gradient(rect.left(), rect.top(), rect.right(), rect.top());
	gradient.setColorAt(0.0f, tree_->palette().color(QPalette::Button));
	gradient.setColorAt(0.6f, tree_->palette().color(QPalette::Light));
	gradient.setColorAt(0.95f, tree_->palette().color(QPalette::Light));
	gradient.setColorAt(1.0f, tree_->palette().color(QPalette::Button));
	QBrush brush(gradient);
	painter_->fillRect(toQRect(rect), brush);
}

void QDrawContext::drawHorizontalLine(const Rect& rect)
{
}

void QDrawContext::drawPlus(const Rect& rect, bool expanded, bool selected, bool grayed)
{	
	QStyleOption option;
	option.rect = toQRect(rect);
	option.state = QStyle::State_Enabled | QStyle::State_Children;
	if (expanded)
		option.state |= QStyle::State_Open;
	painter_->setPen(QPen());
	painter_->setBrush(QBrush());
	tree_->style()->drawPrimitive(QStyle::PE_IndicatorBranch, &option, painter_, tree_);
}

void QDrawContext::drawLabel(const char* text, Font font, const Rect& rect, bool selected)
{
	const QFont* qfont = font == FONT_NORMAL ? &tree_->font() : &tree_->boldFont();
	QColor textColor = tree_->palette().buttonText().color();
	if(selected)
		textColor = tree_->palette().highlightedText().color();
	tree_->drawFilteredString(*painter_, text, QPropertyTree::RowFilter::NAME, qfont, toQRect(rect), textColor, false, false);
}

void QDrawContext::drawNumberEntry(const char* text, const Rect& rect, bool selected, bool grayed)
{
	QPainter* painter = painter_;
	const QPropertyTree* tree_ = this->tree_;

	QRect rt = toQRect(rect);
	rt.adjust(0, 0, 0, -1);

	QStyleOptionFrameV2 option;
	option.state = QStyle::State_Sunken;
	option.lineWidth = tree_->style()->pixelMetric(QStyle::PM_DefaultFrameWidth, &option, 0);
	option.midLineWidth = 0;
	option.features = QStyleOptionFrameV2::None;
	if (selected) {
		option.state |= QStyle::State_HasFocus;
		option.state |= QStyle::State_Active;
		option.state |= QStyle::State_MouseOver;
	}
	else if (!grayed) {
		option.state |= QStyle::State_Enabled;
	}
	option.rect = rt; // option.rect is the rectangle to be drawn on.
	option.palette = tree_->palette();
	option.fontMetrics = tree_->fontMetrics();
	QRect textRect = tree_->style()->subElementRect(QStyle::SE_LineEditContents, &option, tree_);
	if (!textRect.isValid()) {
		textRect = rt;
		textRect.adjust(3, 1, -3, -2);
	}
	else {
		textRect.adjust(2, 1, -2, -1);
	}
	// some styles rely on default pens
	painter->setPen(QPen(tree_->palette().color(QPalette::WindowText)));
	painter->setBrush(QBrush(tree_->palette().color(QPalette::Base)));
	tree_->style()->drawPrimitive(QStyle::PE_PanelLineEdit, &option, painter, 0);
	painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, QString(text), 0);
}

void QDrawContext::drawIcon(const Rect& rect, const yasli::IconXPM& icon)
{
	QImage* image = iconCache_->getImageForIcon(icon);
	if (!image)
		return;
	int x = rect.left() + (rect.width() - image->width()) / 2;
	int y = rect.top() + (rect.height() - image->height()) / 2;
	painter_->drawImage(x, y, *image);
}

void QDrawContext::drawCheck(const Rect& rect, bool disabled, CheckState checked)
{
	QStyleOptionButton option;
	if (!disabled)
		option.state |= QStyle::State_Enabled;
	if (checked == CHECK_SET)
		option.state |= QStyle::State_On;
	else if (checked == CHECK_IN_BETWEEN)
		option.state |= QStyle::State_NoChange;
	else
		option.state |= QStyle::State_Off;

	QSize checkboxSize = tree_->style()->subElementRect(QStyle::SE_CheckBoxIndicator, &option, tree_).size();
	option.rect = QRect(toQRect(rect).center().x() - checkboxSize.width() / 2, toQRect(rect).center().y() - checkboxSize.height() / 2, checkboxSize.width(), checkboxSize.height());

	tree_->style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &option, painter_, 0);
}

void QDrawContext::drawControlButton(const Rect& rect, const char* text, int buttonFlags, property_tree::Font font_name)
{
	drawButton(rect, text, buttonFlags, font_name);
}

void QDrawContext::drawButton(const Rect& rect, const char* text, int buttonFlags, property_tree::Font font_name)
{
	bool pressed = (buttonFlags & BUTTON_PRESSED) != 0;
	bool focused = (buttonFlags & BUTTON_FOCUSED) != 0;
	bool center = (buttonFlags & BUTTON_CENTER_TEXT) != 0;
	bool dropDownArrow = (buttonFlags & BUTTON_DROP_DOWN) != 0;
	bool enabled = (buttonFlags & BUTTON_DISABLED) == 0;
	const QFont* font = font_name == property_tree::FONT_NORMAL ? &tree_->font() : &tree_->boldFont();
	QStyleOptionButton option;
	if (enabled)
		option.state |= QStyle::State_Enabled;
	else
		option.state |= QStyle::State_ReadOnly;
	if (pressed) {
		option.state |= QStyle::State_On;
		option.state |= QStyle::State_Sunken;
	}
	else
		option.state |= QStyle::State_Raised;

	if (focused)
		option.state |= QStyle::State_HasFocus;
	option.rect = toQRect(rect.adjusted(0, 0, -1, -1));
	tree_->style()->drawControl(QStyle::CE_PushButton, &option, painter_);
	QRect textRect;
	if (enabled && dropDownArrow)
	{
		QStyleOption arrowOption;
		arrowOption.rect = QRect(rect.right() - 11, rect.top(), 8, rect.height());
		arrowOption.state |= QStyle::State_Enabled;
		tree_->style()->drawPrimitive(QStyle::PE_IndicatorArrowDown, &arrowOption, painter_, 0);
		textRect = toQRect(rect.adjusted(0, 0, -8, 0));
	}
	else
		textRect = toQRect(rect);

	if (pressed)
		textRect = textRect.adjusted(1, 0, 1, 0);
	if (!center)
		textRect.adjust(4, 0, -5, 0);

	QColor textColor = tree_->palette().color(enabled ? QPalette::Active : QPalette::Disabled, QPalette::ButtonText);
	tree_->_drawRowValue(*painter_, text, font, textRect, textColor, false, center);
}


void QDrawContext::drawValueText(bool highlighted, const char* text)
{
	QColor textColor = highlighted ? tree_->palette().highlightedText().color() : tree_->palette().buttonText().color();
	QRect textRect(widgetRect.left() + 3, widgetRect.top() + 2, widgetRect.width() - 6, widgetRect.height() - 4);
	tree_->_drawRowValue(*painter_, text, &tree_->font(), textRect, textColor, false, false);
}

void QDrawContext::drawEntry(const Rect& rect, const char* text, bool pathEllipsis, bool grayBackground, int trailingOffset)
{
	QRect rt = toQRect(rect);
	rt.adjust(0, 0, -trailingOffset, -1);

	QStyleOptionFrameV2 option;
	option.state = QStyle::State_Sunken;
	option.lineWidth = tree_->style()->pixelMetric(QStyle::PM_DefaultFrameWidth, &option, 0);
	option.midLineWidth = 0;
	option.features = QStyleOptionFrameV2::None;
	if (!grayBackground)
		option.state |= QStyle::State_Enabled;
	if (captured)
	  option.state |= QStyle::State_HasFocus;
	option.rect = rt; // option.rect is the rectangle to be drawn on.
	QRect textRect = tree_->style()->subElementRect(QStyle::SE_LineEditContents, &option, tree_);
	if (!textRect.isValid()) {
		textRect = rt;
		textRect.adjust(3, 1, -3, -2);
	}
	else {
		textRect.adjust(2, 1, -2, -1);
	}
	// some styles rely on default pens
	painter_->setPen(QPen(tree_->palette().color(QPalette::WindowText)));
	painter_->setBrush(QBrush(tree_->palette().color(QPalette::Base)));
	tree_->style()->drawPrimitive(QStyle::PE_PanelLineEdit, &option, painter_, 0);
	tree_->_drawRowValue(*painter_, text, &tree_->font(),  textRect,  tree_->palette().color(QPalette::WindowText), pathEllipsis, false);
}

}
