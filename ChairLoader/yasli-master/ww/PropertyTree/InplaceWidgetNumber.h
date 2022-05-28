#pragma once

#include "PropertyTree/IUIFacade.h"
#include "PropertyTree/PropertyRowNumberField.h"
#include "PropertyTree/sigslot.h"
#include "ww/Entry.h"
#include "ww/PropertyTree.h"
#include "yasli/Pointers.h"

class InplaceWidgetNumber : public property_tree::InplaceWidget, public sigslot::has_slots{
public:
	InplaceWidgetNumber(PropertyRowNumberField* row, ww::PropertyTree* tree)
	: InplaceWidget(tree)
	, initialValue_(row->valueAsString())
	, entry_(new ww::Entry(row->valueAsString().c_str()))
	, tree_(tree)
	, row_(row)
	{
		entry_->signalEdited().connect(this, &InplaceWidgetNumber::onChange);
		entry_->setSelection(ww::EntrySelection(0, -1));
		entry_->setSwallowReturn(true);
		entry_->setSwallowArrows(true);
		entry_->setSwallowEscape(true);
		entry_->setFlat(true);
		entry_->_setParent(tree);
	}
	~InplaceWidgetNumber()
	{
		entry_->signalEdited().disconnect_all();
	}

	void onChange(){
		if(initialValue_ != entry_->text() || row_->multiValue()){
			tree_->model()->rowAboutToBeChanged(row_);
			row_->setValueFromString(entry_->text());
			tree_->model()->rowChanged(row_);
		}
		else
			tree_->_cancelWidget();
	}
	void commit() override{
		if(entry_)
			entry_->commit();
	}
	void* actualWidget() override{ return entry_; }
protected:
	ww::PropertyTree* tree_;
	PropertyRowNumberField* row_;
	yasli::SharedPtr<ww::Entry> entry_;
	yasli::string initialValue_;
};

