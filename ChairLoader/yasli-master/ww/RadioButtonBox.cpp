/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "ww/RadioButtonBox.h"
#include "ww/Serialization.h"
#include "yasli/ClassFactory.h"


namespace ww {

	RadioButtonBox::RadioButtonBox(const char* text, int border)
	{
		box_ = new ww::VBox(4, 6);
		add(box_);
		radioButtons_.clear();
	}

	RadioButtonBox::~RadioButtonBox()
	{
	}

	int RadioButtonBox::selectedIndex() const
	{
		int i = 0;
		for(i; i < int(radioButtons_.size()); ++i)
			if(radioButtons_[i]->status())
				return i;

		return -1;
	}

	void RadioButtonBox::onChangedSelection()
	{
		signalChangedSelection_.emit();
	}

	void RadioButtonBox::setSelectedIndex(int index)
	{
		radioButtons_[index]->setStatus(true);
	}

	void RadioButtonBox::serialize(Archive& ar)
	{
		__super::serialize(ar);
	}

	void RadioButtonBox::addRadioButton(const char * name)
	{
		if(radioButtons_.empty())
			radioButtons_.push_back(new ww::RadioButton(0, name, 0));
		else
			radioButtons_.push_back(new ww::RadioButton(radioButtons_[0], name, 0));
	
		size_t last = radioButtons_.size() - 1;
		box_->add(radioButtons_[last]);
		radioButtons_[last]->signalChanged().connect(this, &RadioButtonBox::onChangedSelection);
	}
}
