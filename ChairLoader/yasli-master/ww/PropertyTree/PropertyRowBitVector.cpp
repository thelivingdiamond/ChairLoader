/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "PropertyRowBitVector.h"
#include "PropertyTree/PropertyRow.h"
#include "PropertyTree/PropertyTree.h"
#include "PropertyTree/PropertyTreeModel.h"
#include "PropertyTree/IUIFacade.h"
#include "ww/CheckComboBox.h"
#include "ww/PropertyTree.h"
#include "ww/SafeCast.h"
#include "yasli/Pointers.h"
#include "yasli/ClassFactory.h"
#include "yasli/STL.h"

namespace ww{
class AutoDropCheckComboBox : public CheckComboBox{
public:
	void show(){
		CheckComboBox::show();
		showDropDown();
	}
};

class InplaceWidgetBitVector : public property_tree::InplaceWidget, public has_slots{
public:
	InplaceWidgetBitVector(PropertyRowBitVector* row, ::PropertyTree* tree)
	: InplaceWidget(tree)
	, comboBox_(new AutoDropCheckComboBox())
	, row_(row)
	{
		comboBox_->_setParent(safe_cast<ww::PropertyTree*>(tree));
		comboBox_->set( row->description()->labels(), row->valueAlt() );
		comboBox_->setDropDownHeight(50);
		comboBox_->signalEdited().connect(this, &InplaceWidgetBitVector::onChange);
	}
	~InplaceWidgetBitVector() {}

	void onChange(){
        tree()->model()->rowAboutToBeChanged(row_);
		row_->setValueAlt(comboBox_->value());
		tree()->model()->rowChanged(row_);
	}
	void commit(){
	}
	void* actualWidget() { return comboBox_; }
protected:
	SharedPtr<CheckComboBox> comboBox_;
	SharedPtr<PropertyRowBitVector> row_;
};

//YASLI_CLASS(PropertyRow, PropertyRowBitVector, "BitVector");

PropertyRowBitVector::PropertyRowBitVector()
: description_(BitVectorWrapper::currentDescription)
, flags_(0)
{
	//YASLI_ASSERT(description_)
}

void PropertyRowBitVector::setValueAndContext(const yasli::Serializer& ser, yasli::Archive& ar) 
{
	BitVectorWrapper* wrapper = ser.cast<BitVectorWrapper>();
	flags_ = wrapper->value;
	if(!description_)
		description_ = wrapper->description;

	if(description_){
		StringListStatic values = description_->nameCombination(flags_);
		joinStringList(&valueText_, values, '|');
		StringListStatic labels = description_->labelCombination(flags_);
		joinStringList(&valueAlt_, labels, '|');
	}
	else{
		YASLI_ASSERT(0);
		typeName_ = "";
		valueText_ = "";
		valueAlt_ = "";
	}
}


bool PropertyRowBitVector::assignTo(const yasli::Serializer& ser) const
{
	if (BitVectorWrapper* wrapper = ser.cast<BitVectorWrapper>()) {
		wrapper->value = flags();
		return true;
	}
	return false;
}

void PropertyRowBitVector::setValueAlt(const char* value)
{
	StringList strings;
	StringList::iterator sit;
	splitStringList(&strings, value, '|');
	int flags = 0;
	for(sit = strings.begin(); sit != strings.end(); ++sit){
		if(!sit->empty())
			flags |= description_->valueByLabel(sit->c_str());
	}
	flags_ = flags;
	valueAlt_ = value;
}


property_tree::InplaceWidget* PropertyRowBitVector::createWidget(::PropertyTree* tree)
{
	return new InplaceWidgetBitVector(this, tree);
}

void PropertyRowBitVector::serializeValue(yasli::Archive& ar)
{
	ar(valueText_, "valueText", "ValueText");
	ar(valueAlt_, "valueAlt", "ValueAlt");
	ar(flags_, "flags", "Flags");
}

REGISTER_PROPERTY_ROW(BitVectorWrapper, PropertyRowBitVector)
DECLARE_SEGMENT(PropertyRowBitVector)

}
