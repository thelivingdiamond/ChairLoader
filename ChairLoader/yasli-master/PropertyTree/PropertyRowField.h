/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once
#include "PropertyRow.h"
class QIcon;

#include "yasli/decorators/IconXPM.h"

class PropertyRowField : public PropertyRow
{
public:
	WidgetPlacement widgetPlacement() const override{ return WIDGET_VALUE; }
	int widgetSizeMin(const PropertyTree* tree) const override{ 
		if (userWidgetSize() >= 0)
			return userWidgetSize();
		else
			return 40;
	}

	virtual int buttonCount() const{ return 0; }
	virtual yasli::IconXPM buttonIcon(const PropertyTree* tree, int index) const{ return yasli::IconXPM(); }
	virtual bool usePathEllipsis() const { return false; }

	void redraw(IDrawContext& context) override;
};

