/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "PropertyRowBool.h"
#include "PropertyTree.h"
#include "PropertyTreeModel.h"
#include "IDrawContext.h"
#include "yasli/ClassFactory.h"
#include "Serialization.h"

YASLI_CLASS(PropertyRow, PropertyRowBool, "bool");

PropertyRowBool::PropertyRowBool()
: value_(false)
{
}

bool PropertyRowBool::assignToPrimitive(void* object, size_t size) const
{
	YASLI_ASSERT(size == sizeof(bool));
	*reinterpret_cast<bool*>(object) = value_;
	return true;
}

void PropertyRowBool::redraw(IDrawContext& context)
{
	context.drawCheck(widgetRect(context.tree), userReadOnly(), multiValue() ? CHECK_IN_BETWEEN : (value_ ? CHECK_SET : CHECK_NOT_SET));
}

bool PropertyRowBool::onActivate(PropertyTree* tree, bool force)
{
	if (!this->userReadOnly()) {
		tree->model()->rowAboutToBeChanged(this);
		value_ = !value_;
		tree->model()->rowChanged(this);
		return true;
	}
	else
		return false;
}

DragCheckBegin PropertyRowBool::onMouseDragCheckBegin() 
{
	if (userReadOnly())
		return DRAG_CHECK_IGNORE;
	return value_ ? DRAG_CHECK_UNSET : DRAG_CHECK_SET;
}

bool PropertyRowBool::onMouseDragCheck(PropertyTree* tree, bool value)
{
	if (value_ != value) {
		tree->model()->rowAboutToBeChanged(this);
		value_ = value;
		tree->model()->rowChanged(this);
		return true;
	}
	return false;
}

void PropertyRowBool::serializeValue(yasli::Archive& ar)
{
    ar(value_, "value", "Value");
}

int PropertyRowBool::widgetSizeMin(const PropertyTree* tree) const
{
	return tree->_defaultRowHeight();
}
