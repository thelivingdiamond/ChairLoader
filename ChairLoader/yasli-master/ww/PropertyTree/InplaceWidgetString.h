#pragma once

#include "PropertyTree/IUIFacade.h"
#include "PropertyTree/PropertyRowString.h"
#include "PropertyTree/sigslot.h"
#include "ww/Entry.h"
#include "ww/PropertyTree.h"
#include "yasli/Pointers.h"

class InplaceWidgetString : public property_tree::InplaceWidget, public sigslot::has_slots{
public:
	InplaceWidgetString(PropertyRowString* row, ww::PropertyTree* tree)
	: InplaceWidget(tree)
	, initialValue_(row->value())
	, entry_(new ww::Entry(row->value().c_str()))
	, tree_(tree)
	, row_(row)
	{
		entry_->signalEdited().connect(this, &InplaceWidgetString::onChange);
		entry_->setSelection(ww::EntrySelection(0, -1));
		entry_->setSwallowReturn(true);
		entry_->setSwallowArrows(true);
		entry_->setSwallowEscape(true);
		entry_->setFlat(true);
		entry_->_setParent(tree);
	}
	~InplaceWidgetString()
	{
		entry_->signalEdited().disconnect_all();
	}

	void onChange(){
		if(initialValue_ != entry_->textW() || row_->multiValue()){
			tree_->model()->rowAboutToBeChanged(row_);
			row_->setValue(entry_->textW());
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
	PropertyRowString* row_;
	yasli::SharedPtr<ww::Entry> entry_;
	yasli::wstring initialValue_;
};

