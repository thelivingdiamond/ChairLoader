/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include <vector>
#include "ww/Strings.h"
#include "ww/_WidgetWithWindow.h"

namespace yasli{
class StringListValue;
class StringList;
}

namespace ww{

class CheckComboBoxImpl;
class CheckComboBox : public _WidgetWithWindow{
public:
	CheckComboBox(bool expandByContent = true, int border = 0);

	typedef std::vector<string> Items;
	typedef Items::iterator iterator;

	void setExpandByContent(bool expand);
	bool expandByContent() const{ return expandByContent_; }

	void showDropDown();
	void setDropDownHeight(int lines);
	int dropDownHeight() const;
	void selectAll(bool select = true);

	void serialize(Archive& ar);

	const char* value(){ return value_.c_str(); }
	void setValue(const char* value);

	void set(const StringList& _stringList, const char* values);

	void clear();	
	iterator begin() { return items_.begin(); }
	iterator end() { return items_.end(); }
	void add(const char* text);
	void insert(iterator before, const char* text);

	virtual void onEdited() { signalEdited_.emit(); }
	signal0& signalEdited() { return signalEdited_; }
	
	virtual void onSelectionChanged(){ signalSelectionChanged_.emit(); }
	signal0& signalSelectionChanged() { return signalSelectionChanged_; }

protected:
	CheckComboBoxImpl* window() const{ return reinterpret_cast<CheckComboBoxImpl*>(_window()); }
	friend class CheckComboBoxImpl;
	friend class CheckComboListBoxImpl;
	void _setPosition(const Rect& position);

	void updateMinimalSize();

	signal0 signalSelectionChanged_;
	signal0 signalEdited_;

	bool expandByContent_;
	Items items_;
	string value_;
	int dropDownHeight_;
};

}

