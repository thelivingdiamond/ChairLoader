#include "QUIFacade.h"
#include <QFont>
#include <QFontMetrics>
#include <QStyleOption>
#include <QDesktopWidget>
#include <QApplication>
#include "QPropertyTree.h"
#include "QDrawContext.h"

#include "InplaceWidgetComboBox.h"
#include "InplaceWidgetNumber.h"
#include "InplaceWidgetString.h"

namespace property_tree {


// ---------------------------------------------------------------------------

int QUIFacade::textWidth(const char* text, Font font)
{
	const QFont* qfont = font == FONT_BOLD ? &tree_->boldFont() : &tree_->font();

	QFontMetrics fm(*qfont);
	return fm.width(text);
}

Point QUIFacade::screenSize()
{
	QSize s = QApplication::desktop()->screenGeometry(tree_).size();
	return Point(s.width(), s.height());
}

IMenu* QUIFacade::createMenu()
{
	return new QtMenu(new QMenu(), tree_);
}

void QUIFacade::setCursor(Cursor cursor)
{
	switch (cursor)
	{
	case CURSOR_SLIDE:
	tree_->setCursor(QCursor(Qt::SizeHorCursor));
	return;
	}

	tree_->setCursor(QCursor());
}

void QUIFacade::unsetCursor()
{
	tree_->unsetCursor();
}

Point QUIFacade::cursorPosition()
{
	return fromQPoint(tree_->mapFromGlobal(QCursor::pos()));
}

QWidget* QUIFacade::qwidget()
{
	return tree_;
}

HWND QUIFacade::hwnd()
{
	return (HWND)tree_->winId();
}

InplaceWidget* QUIFacade::createComboBox(ComboBoxClientRow* client) { return new InplaceWidgetComboBox(client, tree_); }
InplaceWidget* QUIFacade::createNumberWidget(PropertyRowNumberField* row) { return new InplaceWidgetNumber(row, tree_); }
InplaceWidget* QUIFacade::createStringWidget(PropertyRowString* row) { return new InplaceWidgetString(row, tree_); }

}
