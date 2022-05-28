/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once
#include "yasli/BitVectorImpl.h"
#include "PropertyTree/PropertyRowField.h"

namespace ww{

class PropertyRowBitVector : public PropertyRowField{
public:
	bool isLeaf() const{ return true; }
	bool isStatic() const{ return false; }
	PropertyRowBitVector();
	const char* value() const{ return valueText_.c_str(); }
	const char* valueAlt() const{ return valueAlt_.c_str(); }
	void setValueAlt(const char* value);
	int flags() const { return flags_; }
	void serializeValue(yasli::Archive& ar) override;
	bool assignTo(const yasli::Serializer& ser) const override;
	yasli::string valueAsString() const override{ return valueAlt_; }
	void setValueAndContext(const yasli::Serializer& ser, yasli::Archive& ar) override;
	const yasli::EnumDescription* description() { return description_; }
	property_tree::InplaceWidget* createWidget(PropertyTree* tree) override;
	WidgetPlacement widgetPlacement() const override{ return WIDGET_VALUE; }
protected:
	yasli::string valueText_;
	yasli::string valueAlt_;
	int flags_;
	const yasli::EnumDescription* description_;
};

}
