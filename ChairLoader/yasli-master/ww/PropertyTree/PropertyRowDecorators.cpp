/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "PropertyTree/PropertyRowImpl.h"
#include "yasli/Archive.h"
#include "yasli/ClassFactory.h"
#include "ww/Decorators.h"
#include "PropertyTree/PropertyTree.h"
#include "PropertyTree/IDrawContext.h"
#include "PropertyTree/PropertyTreeModel.h"
#include "ww/Win32/Window32.h"
#include "ww/Win32/Drawing.h"
#include "ww/Win32/Rectangle.h"
#include "ww/Unicode.h"
#include "ww/PropertyTree/wwDrawContext.h"
#include "ww/SafeCast.h"
#include "gdiplusUtils.h"
#include "yasli/decorators/Button.h"


namespace ww {
	DECLARE_SEGMENT(PropertyRowDecorators)
}

namespace property_tree{

using ww::NotDecorator;


// ------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------

class PropertyRowNot : public PropertyRowImpl<NotDecorator>{
public:
	bool onActivate(PropertyTree* tree, bool force) override;
	void redraw(IDrawContext& context) override;
	WidgetPlacement widgetPlacement() const override{ return WIDGET_ICON; }
	yasli::string valueAsString() const override{ return value_ ? label() : ""; }
	int widgetSizeMin(const PropertyTree* tree) const override{ return 16; }
};

bool PropertyRowNot::onActivate(PropertyTree* tree, bool force)
{
    tree->model()->rowAboutToBeChanged(this);
	value() = !value();
	tree->model()->rowChanged(this);
	return true;
}

void PropertyRowNot::redraw(IDrawContext& context)
{	
	if (property_tree::wwDrawContext* x = ww::safe_cast<property_tree::wwDrawContext*>(&context))
		Win32::drawNotCheck(x->graphics, Gdiplus::Rect(context.widgetRect.x, context.widgetRect.y, context.widgetRect.width(), context.widgetRect.height()), value());
}

REGISTER_PROPERTY_ROW(NotDecorator, PropertyRowNot);

// ---------------------------------------------------------------------------

using ww::RadioDecorator;

class PropertyRowRadio : public PropertyRowImpl<RadioDecorator>{
public:
	bool onActivate(PropertyTree* tree, bool force) override;
	void redraw(IDrawContext& context) override;
	WidgetPlacement widgetPlacement() const override{ return WIDGET_ICON; }
	yasli::string valueAsString() const override{ return value() ? label() : ""; }
	int widgetSizeMin(const PropertyTree*) const override{ return 16; }
};

bool PropertyRowRadio::onActivate(PropertyTree* tree, bool force)
{
    tree->model()->rowAboutToBeChanged(this);
	value() = !value();
	tree->model()->rowChanged(this);
	return true;
}

void PropertyRowRadio::redraw(IDrawContext& context)
{
	if (property_tree::wwDrawContext* x = ww::safe_cast<property_tree::wwDrawContext*>(&context))
		Win32::drawRadio(x->graphics, Gdiplus::Rect(context.widgetRect.x, context.widgetRect.y, context.widgetRect.width(), context.widgetRect.height()), value());
}

REGISTER_PROPERTY_ROW(RadioDecorator, PropertyRowRadio);

}
