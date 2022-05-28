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
#include "PropertyTree/PropertyTreeModel.h"
#include "PropertyTree/PropertyRowImpl.h"

#include "PropertyTree/PropertyTree.h"
#include "PropertyTree/IDrawContext.h"
#include "PropertyTree/PropertyRow.h"
#include "PropertyTree/Serialization.h"
#include "PropertyTree/IUIFacade.h"
#include "ww/ColorChooserDialog.h"
#include "ww/Win32/Drawing.h"
#include "ww/Win32/Window32.h"
#include "ww/Color.h"	
#include "gdiplusUtils.h"

namespace ww{


template<size_t Size>
void formatColor(char (&str)[Size], const ww::Color& c) { sprintf_s(str, "(%i %i %i %i)", c.r, c.g, c.b, c.a); }

unsigned int toARGB(Color color)
{
	return color.argb();
}

void fromColor(Color* out, Color color)
{
	*out = color;
}

template<class ColorType>
class PropertyRowColor : public PropertyRowImpl<ColorType>{
public:
	PropertyRowColor();
	void redraw(IDrawContext& context) override;

	bool activateOnAdd() const override{ return true; }
	bool onActivate(PropertyTree* tree, bool force) override;
	string valueAsString() const { 
		char buf[64];
		formatColor(buf, value_);
		return string(buf);
	}
	virtual int widgetSizeMin(const PropertyTree* tree) const override{ 
		return userWidgetSize() >= 0 ? userWidgetSize() : tree->_defaultRowHeight(); 
	}
};

template<class ColorType>
bool PropertyRowColor<ColorType>::onActivate(PropertyTree* tree, bool force)
{
	ColorChooserDialog dialog(tree->ui()->hwnd(), Color(toARGB(value())));
    if(dialog.showModal() == ww::RESPONSE_OK){
        tree->model()->rowAboutToBeChanged(this);
		fromColor(&value(), dialog.get());
		tree->model()->rowChanged(this);
		return true;
	}
	return false;
}

template<class ColorType>
void PropertyRowColor<ColorType>::redraw(IDrawContext& context)
{
	using namespace Gdiplus;
	using Gdiplus::Color;

	if(multiValue()){
		__super::redraw(context);
		return;
	}

	context.drawColor(context.widgetRect, property_tree::Color(value().r, value().g, value().b, value().a));
}

template<class ColorType>
PropertyRowColor<ColorType>::PropertyRowColor()
{
}

DECLARE_SEGMENT(PropertyRowColorWW)
typedef PropertyRowColor<Color> PropertyRowWWColor;
REGISTER_PROPERTY_ROW(Color, PropertyRowWWColor); 
}
