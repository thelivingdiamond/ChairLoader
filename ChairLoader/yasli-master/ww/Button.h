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
#include "Strings.h"

namespace ww{

class ButtonImpl;
class Button : public _WidgetWithWindow{
public:
	Button(const char* text = "Button", int border = 0);

	const char* text() const { return text_.c_str(); }
	void setText(const char* text);

	virtual void onPressed();
	signal0& signalPressed() { return signalPressed_; }

	void serialize(Archive& ar);
	
	virtual bool canBeDefault() const { return true; }
	virtual void setDefaultFrame(bool enable);
protected:
	// внутренние функции
	ButtonImpl* window() const{ return reinterpret_cast<ButtonImpl*>(_window()); }

	signal0 signalPressed_;
	string text_;
	bool defaultBtn_;
};

}

