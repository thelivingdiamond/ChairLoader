#include "wwUIFacade.h"
#include "ww/Win32/Window32.h"
#include <Windows.h>
#include "wwDrawContext.h"
#include "ww/Unicode.h"
#include "gdiplusUtils.h"
#include "InplaceWidgetString.h"
#include "InplaceWidgetComboBox.h"
#include "InplaceWidgetNumber.h"

namespace property_tree {

HWND wwUIFacade::hwnd()
{
	return tree_->_window()->handle();
}


IMenu* wwUIFacade::createMenu()
{
	return new wwMenu(tree_);
}

void wwUIFacade::setCursor(Cursor cursor)
{
}

void wwUIFacade::unsetCursor()
{
}

Point wwUIFacade::cursorPosition()
{
	POINT pt = { 0, 0 };
	GetCursorPos(&pt);
	return Point(pt.x, pt.y);
}

int wwUIFacade::textWidth(const char* text, Font _font)
{
	Gdiplus::Font* font = _font == FONT_NORMAL ? propertyTreeDefaultFont() : propertyTreeDefaultBoldFont();
	std::wstring wstr(ww::toWideChar(text));
	Gdiplus::StringFormat format;
	Gdiplus::RectF bound;
	tree_->_graphics()->MeasureString(wstr.c_str(), (int)wstr.size(), font, Gdiplus::RectF(0.0f, 0.0f, 0.0f, 0.0f), &format, &bound, 0);
	return bound.Width;
}

Point wwUIFacade::screenSize()
{
	return Point(1920, 1080);
}


InplaceWidget* wwUIFacade::createComboBox(ComboBoxClientRow* client)
{
	return new InplaceWidgetComboBox(client, tree_);
}

InplaceWidget* wwUIFacade::createNumberWidget(PropertyRowNumberField* row)
{
	return new InplaceWidgetNumber(row, tree_);
}

InplaceWidget* wwUIFacade::createStringWidget(PropertyRowString* row)
{
	return new InplaceWidgetString(row, tree_);
}

}
