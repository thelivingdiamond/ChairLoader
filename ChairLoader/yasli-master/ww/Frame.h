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
#include "ww/Strings.h"

namespace Win32{
	class Window;
}


namespace ww{

class FrameImpl;

class Frame : public _ContainerWithWindow{
public:
	Frame(const char* text = "Frame", bool emphasis = true, int border = 0);
	~Frame();

	const char* text() const { return text_.c_str(); }
	void setText(const char* text);
	void setEmphasis(bool emphasis);
	bool emphasis() const{ return emphasis_; }

	void visitChildren(WidgetVisitor& visitor) const;
	/// добавить контрол в окно
	void add(Widget* widget);
	/// убирает единственный дочерний виджет
	void remove();

	void serialize(Archive& ar);

	void _arrangeChildren();
	void _relayoutParents();
protected:

	SharedPtr<Widget> child_;
	string text_;
	int space_;
	bool emphasis_;
};

}

