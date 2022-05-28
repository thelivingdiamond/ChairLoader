/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "yasli/Config.h"
#include "PropertyRow.h"
#include "Unicode.h"

class PropertyRowBool : public PropertyRow
{
public:
	PropertyRowBool();
	bool assignToPrimitive(void* val, size_t size) const override;
	void setValue(bool value) { value_ = value; }

	void redraw(IDrawContext& context) override;
	bool isLeaf() const override{ return true; }
	bool isStatic() const override{ return false; }

	bool onActivate(PropertyTree* tree, bool force) override;
	DragCheckBegin onMouseDragCheckBegin() override;
	bool onMouseDragCheck(PropertyTree* tree, bool value) override;
	yasli::wstring valueAsWString() const override{ return value_ ? L"true" : L"false"; }
	yasli::string valueAsString() const override{ return value_ ? "true" : "false"; }
	WidgetPlacement widgetPlacement() const override{ return WIDGET_ICON; }
	void serializeValue(yasli::Archive& ar) override;
	int widgetSizeMin(const PropertyTree* tree) const override;
protected:
	bool value_;
};

