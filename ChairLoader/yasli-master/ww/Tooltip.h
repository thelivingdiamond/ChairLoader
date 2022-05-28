/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "ww/Strings.h"

namespace ww{

class Widget;

class Tooltip{
public:
	Tooltip(const char* text = "", bool baloon = false);
	void attach(Widget* widget);

	void setText(const char* text);
	const char* text() const { return text_.c_str(); }
	void show();
	void hide();

	void setBaloon(bool baloon);
	void setOffset(const Vect2& offset) { offset_ = offset; }

protected:
	string text_;
	bool baloon_;
	Widget* widget_;
	HWND toolTipWindow_;
	Vect2 offset_;
};

}
