#pragma once
#include "PropertyTree/PropertyRow.h"
#include "PropertyTree/sigslot.h"
#include "yasli/StringList.h"
#include "ww/ComboBox.h"

class AutoDropComboBox : public ww::ComboBox{
public:
	void show(){
		ComboBox::show();
		ComboBox::setFocus();
		showDropDown();
	}
};

class InplaceWidgetComboBox : public property_tree::InplaceWidget, public sigslot::has_slots{
public:
	InplaceWidgetComboBox(ComboBoxClientRow* client, ww::PropertyTree* tree)
	: InplaceWidget(tree)
	, comboBox_(new AutoDropComboBox())
	, client_(client)
	{
		std::vector<std::string> values;
		int selectedIndex;
		client_->populateComboBox(&values, &selectedIndex, tree_);

		yasli::StringList stringList;
		stringList.assign(values.begin(), values.end());

		yasli::StringListValue value(stringList, selectedIndex);

		comboBox_->_setParent(tree);
		comboBox_->set(value, false);
		comboBox_->signalEdited().connect(this, &InplaceWidgetComboBox::onChange);
	}

	~InplaceWidgetComboBox()
	{
	}

	void onChange(){
		yasli::StringListValue comboList;
		comboBox_->get(comboList);
		if (!client_->onComboBoxSelected(comboList.c_str(), tree()))
			tree_->_cancelWidget();
	}
	void commit() override{}
	void* actualWidget() override { return comboBox_; }
protected:
	ComboBoxClientRow* client_;
	yasli::SharedPtr<ww::ComboBox> comboBox_;
};

