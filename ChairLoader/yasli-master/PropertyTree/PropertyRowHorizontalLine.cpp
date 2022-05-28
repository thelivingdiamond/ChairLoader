#include "PropertyTree/PropertyRowImpl.h"
#include "PropertyTree/IDrawContext.h"
#include "yasli/decorators/HorizontalLine.h"

using yasli::HorizontalLine;

class PropertyRowHorizontalLine : public PropertyRowImpl<HorizontalLine>{
public:
	void redraw(IDrawContext& context) override;
	bool isSelectable() const override{ return false; }
};

void PropertyRowHorizontalLine::redraw(IDrawContext& context)
{
	context.drawHorizontalLine(context.widgetRect);
}

DECLARE_SEGMENT(PropertyRowHorizontalLine)
REGISTER_PROPERTY_ROW(HorizontalLine, PropertyRowHorizontalLine);

