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
#include "ww/_WidgetWithWindow.h"
#include "ww/VBox.h"

namespace ww{
using std::vector;

class TabsImpl;

class TabChanger{};

class Tabs : public _WidgetWithWindow, public TabChanger{
public:
	Tabs(int border = 0);

	size_t add(const char* tabTitle, int before = -1);
	void remove(int index);
	void clear();

	int selectedTab();
	void setSelectedTab(int index, const TabChanger* changer = 0);

	typedef signal1<const TabChanger*> SignalChanged;
	SignalChanged& signalChanged(){ return signalChanged_; }
	
	typedef signal2<MouseButton, int> SignalMouseButtonDown;
	SignalMouseButtonDown& signalMouseButtonDown(){ return signalMouseButtonDown_; }

	void serialize(yasli::Archive& ar);

protected:
	SignalChanged signalChanged_;
	SignalMouseButtonDown signalMouseButtonDown_;

	TabsImpl& impl();
	friend class TabsImpl;
};

class TabPages : public VBox, public TabChanger{
public:
	TabPages(int border = 0);

	int add(const char* title, Widget* widget, int before = -1);
	void remove(int index);
	Widget* widgetByIndex(int index);
	size_t size() const;

	int selectedTab() { return tabs_->selectedTab(); }
	void setSelectedTab(int index);

	void serialize(yasli::Archive& ar);

	typedef signal1<const TabChanger*> SignalChanged;
    SignalChanged &signalChanged(){ return tabs_->signalChanged(); }
protected:
	void onTabChange(const TabChanger* changer);
	yasli::SharedPtr<Tabs> tabs_;
	typedef vector<yasli::SharedPtr<Widget> > Widgets;
	Widgets widgets_;
};

}

