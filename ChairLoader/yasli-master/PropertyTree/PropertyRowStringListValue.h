/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */


#pragma once
#include "PropertyRowImpl.h"
#include "PropertyTreeModel.h"
#include "IDrawContext.h"
#include "PropertyTree.h"
#include "IUIFacade.h"
#include "Rect.h"
#include <vector>


using yasli::StringListValue;
class PropertyRowStringListValue : public PropertyRow, public ComboBoxClientRow {
public:
	property_tree::InplaceWidget* createWidget(PropertyTree* tree) override;

	void populateComboBox(std::vector<std::string>* values, int* selectedIndex, PropertyTree* tree) override
	{
		values->assign(stringList_.begin(), stringList_.end());
		*selectedIndex = stringList_.find(value_.c_str());
	}

	bool onComboBoxSelected(const char* newValue, PropertyTree* tree) override
	{
		if (value_ != newValue) {
			tree->model()->rowAboutToBeChanged(this);
			value_ = newValue;
			tree->model()->rowChanged(this);
			return true;
		}
		return false;
	}

	yasli::string valueAsString() const override { return value_.c_str(); }
	bool assignTo(const Serializer& ser) const override {
		*((StringListValue*)ser.pointer()) = value_.c_str();
		return true;
	}
	void setValueAndContext(const yasli::Serializer& ser, yasli::Archive& ar) override {
		YASLI_ESCAPE(ser.size() == sizeof(StringListValue), return);
		const StringListValue& stringListValue = *((StringListValue*)(ser.pointer()));
		stringList_ = stringListValue.stringList();
		value_ = stringListValue.c_str();
	}

	bool isLeaf() const override{ return true; }
	bool isStatic() const override{ return false; }
	int widgetSizeMin(const PropertyTree*) const override { return userWidgetSize() ? userWidgetSize() : 80; }
	WidgetPlacement widgetPlacement() const override{ return WIDGET_VALUE; }

	void redraw(IDrawContext& context) override
	{
		if(multiValue())
			context.drawEntry(context.widgetRect, " ... ", false, true, 0);
		else if(userReadOnly())
			context.drawValueText(pulledSelected(), valueAsString().c_str());
		else
			context.drawComboBox(context.widgetRect, valueAsString().c_str());

	}

	void serializeValue(yasli::Archive& ar) override{
		ar(value_, "value", "Value");
		ar(stringList_, "stringList", "String List");
	}
private:
	yasli::StringList stringList_;
	yasli::string value_;
	friend class InplaceWidgetStringListValue;
};

using yasli::StringListStaticValue;
class PropertyRowStringListStaticValue : public PropertyRowImpl<StringListStaticValue>, public ComboBoxClientRow {
public:
	InplaceWidget* createWidget(PropertyTree* tree) override;

	void populateComboBox(std::vector<std::string>* values, int* selectedIndex, PropertyTree* tree) override
	{
		values->assign(stringList_.begin(), stringList_.end());
		*selectedIndex = stringList_.find(value_.c_str());
	}

	bool onComboBoxSelected(const char* newValue, PropertyTree* tree) override
	{
		if (value_ != newValue) {
			tree->model()->rowAboutToBeChanged(this);
			value_ = newValue;
			tree->model()->rowChanged(this);
			return true;
		}
		return false;
	}
	yasli::string valueAsString() const override { return value_.c_str(); }
	bool assignTo(const Serializer& ser) const override {
		*((StringListStaticValue*)ser.pointer()) = value_.c_str();
		return true;
	}
	void setValueAndContext(const Serializer& ser, yasli::Archive& ar) override {
		YASLI_ESCAPE(ser.size() == sizeof(StringListStaticValue), return);
		const StringListStaticValue& stringListValue = *((StringListStaticValue*)(ser.pointer()));
		stringList_.assign(stringListValue.stringList().begin(), stringListValue.stringList().end());
		value_ = stringListValue.c_str();
	}

	bool isLeaf() const override{ return true; }
	bool isStatic() const override{ return false; }
	int widgetSizeMin(const PropertyTree*) const override { return userWidgetSize() ? userWidgetSize() : 80; }
	WidgetPlacement widgetPlacement() const override{ return WIDGET_VALUE; }

	void redraw(IDrawContext& context) override
	{
		if(multiValue())
			context.drawEntry(context.widgetRect, " ... ", false, true, 0);
		else if(userReadOnly())
			context.drawValueText(pulledSelected(), valueAsString().c_str());
		else
			context.drawComboBox(context.widgetRect, valueAsString().c_str());
	}


	void serializeValue(yasli::Archive& ar) override{
		ar(value_, "value", "Value");
		ar(stringList_, "stringList", "String List");
	}
private:
	yasli::StringList stringList_;
	yasli::string value_;
	friend class InplaceWidgetStringListValue;
};
	
