/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "PropertyRowField.h"
#include "IDrawContext.h"
#include "Rect.h"

void PropertyRowField::redraw(IDrawContext& context)
{
	int buttonCount = this->buttonCount();
	int offset = 0;
	for (int i = 0; i < buttonCount; ++i) {
		int width = 16;
		Rect iconRect(context.widgetRect.right() - offset - width, context.widgetRect.top(), width, context.widgetRect.height());
		context.drawIcon(iconRect, buttonIcon(context.tree, i));
		offset += width;
	}

	int iconSpace = offset ? offset + 2 : 0;
    if(multiValue())
		context.drawEntry(context.widgetRect, " ... ", false, userReadOnly(), iconSpace);
    else if(userReadOnly())
		context.drawValueText(pulledSelected(), valueAsString().c_str());
    else
        context.drawEntry(context.widgetRect, valueAsString().c_str(), usePathEllipsis(), false, iconSpace);

}

