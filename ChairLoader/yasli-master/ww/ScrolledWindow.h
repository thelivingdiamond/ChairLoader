/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "ww/_WidgetWithWindow.h"

namespace Win32{ class Window; }

namespace ww{
class ScrolledWindowImpl;

enum ScrollPolicy{
	SCROLL_AUTOMATIC,
	SCROLL_ALWAYS,
    SCROLL_NEVER
};

class ScrolledWindow: public _ContainerWithWindow{
public:
	ScrolledWindow(int border = 0);
	~ScrolledWindow();

	void add(Widget* child, bool fill = true);
	void remove();
	void visitChildren(WidgetVisitor& visitor) const;

	void setPolicy(ScrollPolicy horizontalPolicy = SCROLL_AUTOMATIC, ScrollPolicy verticalPolicy = SCROLL_AUTOMATIC);
	ScrollPolicy policyHorizontal() const{ return policyHorizontal_; }
	ScrollPolicy policyVertical() const{ return policyVertical_; }

	void serialize(Archive& ar);

	void _arrangeChildren();
	void _relayoutParents();
protected:
	ScrolledWindowImpl* impl();	

	Vect2 clientAreaSize() const;
	bool verticalScrollVisible() const;
	bool horizontalScrollVisible() const;

	yasli::SharedPtr<Widget> child_;
    bool fill_;

	ScrollPolicy policyHorizontal_;
	ScrollPolicy policyVertical_;

	Vect2 offset_;

	friend class ScrolledWindowImpl;
};

}
