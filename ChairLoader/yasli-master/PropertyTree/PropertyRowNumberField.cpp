/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "PropertyTree.h"
#include "PropertyTreeModel.h"
#include "PropertyRowNumberField.h"
#include "IDrawContext.h"
#include "PropertyTree.h"
#include "IUIFacade.h"

property_tree::InplaceWidget* PropertyRowNumberField::createWidget(PropertyTree* tree)
{
	return tree->ui()->createNumberWidget(this);
}

void PropertyRowNumberField::redraw(IDrawContext& context)
{
    if(multiValue())
		context.drawEntry(context.widgetRect, " ... ", false, userReadOnly(), 0);
    else 
		context.drawNumberEntry(valueAsString().c_str(), context.widgetRect, context.captured, userReadOnly());
}

bool PropertyRowNumberField::onMouseDown(PropertyTree* tree, Point point, bool& changed)
{
	changed = false;
	if (widgetRect(tree).contains(point) && !userReadOnly()) {
		startIncrement();
		return true;
	}
	return false;
}

void PropertyRowNumberField::onMouseDrag(const PropertyDragEvent& e)
{
	if(abs(e.pos.x() - e.start.x()) < 3)
		return;

	e.tree->ui()->setCursor(CURSOR_SLIDE);

	Point screenSize = e.tree->ui()->screenSize();
	float relativeDelta = float((e.pos - e.start).x()) / screenSize.x();
	incrementLog(relativeDelta);
	setMultiValue(false);
}

void PropertyRowNumberField::onMouseStill(const PropertyDragEvent& e)
{
	e.tree->apply(true);
}

void PropertyRowNumberField::onMouseUp(PropertyTree* tree, Point point) 
{
	tree->ui()->unsetCursor();
	endIncrement(tree);
}

bool PropertyRowNumberField::onActivate(PropertyTree* tree, bool force)
{
	return false;
}

bool PropertyRowNumberField::onActivateRelease(PropertyTree* tree)
{
	return tree->spawnWidget(this, false);
}

// ---------------------------------------------------------------------------

