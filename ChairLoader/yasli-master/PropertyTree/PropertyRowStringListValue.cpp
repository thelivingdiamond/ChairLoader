/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include <math.h>

#include "Factory.h"
#include "PropertyRowStringListValue.h"
#include "PropertyTreeModel.h"
#include "IDrawContext.h"
#include "PropertyTree.h"
#include "ConstStringList.h"

#include "yasli/Archive.h"
#include "yasli/ClassFactory.h"
#include "IMenu.h"

using yasli::StringList;
using yasli::StringListValue;

REGISTER_PROPERTY_ROW(StringListValue, PropertyRowStringListValue)


property_tree::InplaceWidget* PropertyRowStringListValue::createWidget(PropertyTree* tree)
{
	return tree->ui()->createComboBox(this);
}

// ---------------------------------------------------------------------------
REGISTER_PROPERTY_ROW(StringListStaticValue, PropertyRowStringListStaticValue)

property_tree::InplaceWidget* PropertyRowStringListStaticValue::createWidget(PropertyTree* tree)
{
	return tree->ui()->createComboBox(this);
}

DECLARE_SEGMENT(PropertyRowStringList)

// vim:ts=4 sw=4:
