/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "yasli/ClassFactory.h"

#include "PropertyTree/IDrawContext.h"
#include "PropertyTree/PropertyRowImpl.h"
#include "PropertyTree/PropertyTree.h"
#include "PropertyTree/PropertyTreeModel.h"
#include "PropertyTree/Serialization.h"

#include "ww/Win32/Drawing.h"
#include "ww/Color.h"	
#include "ww/Icon.h"
#include "gdiplusUtils.h"
#include "yasli/decorators/IconXPM.h"

namespace ww {
class PropertyRowIcon : public PropertyRow{
public:
	static const bool Custom = true;

	bool assignTo(void* val, size_t size)
	{
		return false;
	}

	void redraw(IDrawContext& context)
	{
		property_tree::Rect rect = context.widgetRect;
		context.drawIcon(rect, icon_);
	}

	bool isLeaf() const override{ return true; }
	bool isStatic() const override{ return false; }
	bool isSelectable() const override{ return false; }

	bool onActivate(::PropertyTree* tree, bool force) override
	{
		return false;
	}
	void setValueAndContext(const Serializer& ser, yasli::Archive& ar) override {
		if (Icon* icon = ser.cast<Icon>()) {
			icon_ = yasli::IconXPM(icon->source(), icon->lineCount());
		}
	}
	yasli::string valueAsString() const override{ return ""; }
	WidgetPlacement widgetPlacement() const{ return WIDGET_ICON; }
	void serializeValue(Archive& ar) override{}
	int widgetSizeMin(const ::PropertyTree* tree) const override{ return 16 + 2; }
	int height() const{ return 16; }
protected:
	yasli::IconXPM icon_;
};

class PropertyRowIconToggle : public PropertyRowImpl<IconToggle>{
public:
	void redraw(IDrawContext& context)
	{
		Icon& icon = value().value_ ? value().iconTrue_ : value().iconFalse_;
		yasli::IconXPM xpmIcon(icon.source(), icon.lineCount());
		context.drawIcon(context.widgetRect, xpmIcon);
	}

	bool isLeaf() const{ return true; }
	bool isStatic() const{ return false; }
	bool isSelectable() const{ return true; }
	bool onActivate(::PropertyTree* tree, bool force)
	{
		tree->model()->rowAboutToBeChanged(this);
		value().value_ = !value().value_;
		tree->model()->rowChanged(this);
		return true;
	}
	yasli::string valueAsString() const override{ return value().value_ ? "true" : "false"; }
	WidgetPlacement widgetPlacement() const override{ return WIDGET_ICON; }

	int widgetSizeMin(const ::PropertyTree* tree) const override{ return value().iconFalse_.width() + 1; }
	int height() const{ return value().iconFalse_.height(); }
};

REGISTER_PROPERTY_ROW(Icon, PropertyRowIcon); 
REGISTER_PROPERTY_ROW(IconToggle, PropertyRowIconToggle); 
DECLARE_SEGMENT(PropertyRowIconWW)
}
