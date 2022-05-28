/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include <math.h>

#include "PropertyRowString.h"
#include "PropertyTreeModel.h"
#include "IDrawContext.h"
#include "PropertyTree.h"

#include "yasli/STL.h"
#include "yasli/Archive.h"
#include "yasli/ClassFactory.h"
#include "IMenu.h"
#include "IUIFacade.h"
#include "Unicode.h"

// ---------------------------------------------------------------------------
YASLI_CLASS(PropertyRow, PropertyRowString, "string");

bool PropertyRowString::assignTo(yasli::string& str)
{
    str = fromWideChar(value_.c_str());
    return true;
}

bool PropertyRowString::assignTo(yasli::wstring& str)
{
    str = value_;
    return true;
}

property_tree::InplaceWidget* PropertyRowString::createWidget(PropertyTree* tree)
{
	return tree->ui()->createStringWidget(this);
}

yasli::string PropertyRowString::valueAsString() const
{
	return fromWideChar(value_.c_str());
}

void PropertyRowString::setValue(const wchar_t* str)
{
	value_ = str;
}

void PropertyRowString::setValue(const char* str)
{
	value_ = toWideChar(str);
}

void PropertyRowString::serializeValue(yasli::Archive& ar)
{
	ar(value_, "value", "Value");
}

// vim:ts=4 sw=4:
