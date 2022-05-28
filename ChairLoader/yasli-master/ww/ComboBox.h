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
#include "yasli/StringList.h"

namespace yasli{
	class StringListValue;
}

namespace ww{
    
class ComboBoxImpl;
class ComboBox : public _WidgetWithWindow{
public:
	ComboBox(bool expandByContent = false, int border = 0);


	typedef std::vector<std::string> Items;
	typedef Items::iterator iterator;

	void setSelectedIndex(int index);
	int selectedIndex() const;
	std::string selectedString() const;

	void setExpandByContent(bool expand);
	bool expandByContent() const{ return expandByContent_; }

	void showDropDown();
	void setDropDownHeight(int lines);
	int dropDownHeight() const;

	void serialize(Archive& ar);

	void set(const char* comboList, const char* value, bool onlyVisible);
	void set(const StringListValue& value, bool onlyVisible);
	void get(StringListValue& value);
	void get(StringListStaticValue& value);

	void clear();
	iterator begin() { return items_.begin(); }
	iterator end() { return items_.end(); }
	void add(const char* text);
	void insert(iterator before, const char* text);

	virtual void onEdited();
	signal0& signalEdited() { return signalEdited_; }
	
	virtual void onSelectionChanged(){ signalSelectionChanged_.emit(); }
	signal0& signalSelectionChanged() { return signalSelectionChanged_; }

	void setFocus();

protected:
	ComboBox(ComboBoxImpl* impl, bool expandByContent, int border);

	ComboBoxImpl* impl() const{ return reinterpret_cast<ComboBoxImpl*>(_window()); }
	friend class ComboBoxImpl;
	void _setPosition(const Rect& position);

	void updateMinimalSize();

	signal0 signalSelectionChanged_;
	signal0 signalEdited_;

	bool expandByContent_;
	StringList items_;
	int selectedIndex_;
	int dropDownHeight_;
};

}

