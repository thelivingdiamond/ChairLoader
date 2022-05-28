/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "ww/Widget.h"
#include "ww/Frame.h"
#include "ww/VBox.h"
#include "ww/RadioButton.h"
#include "ww/Strings.h"
#include <vector>


namespace ww{

using std::vector;

class RadioButtonBox : public ww::Frame 
{
public:
	RadioButtonBox(const char* text = "RadioButtonBox", int border = 0);
	~RadioButtonBox();

	void addRadioButton(const char * name);

	void setSelectedIndex(int index);
	int selectedIndex() const;

	virtual void onChangedSelection();
	signal0& signalChangedSelection() { return signalChangedSelection_; }

	void serialize(Archive& ar);
	
protected:
	signal0 signalChangedSelection_;

	vector<RadioButton*> radioButtons_;
	VBox* box_;
};

}
