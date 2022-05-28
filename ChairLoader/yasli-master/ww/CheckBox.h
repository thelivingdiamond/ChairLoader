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

namespace ww{

class CheckBoxImpl;
class CheckBox : public _WidgetWithWindow{
public:
	CheckBox(const char* text = "CheckBox", int border = 0);

	const char* text() const { return text_.c_str(); }
	void setText(const char* text);
	void setStatus(bool status);

	void setButtonLike(bool buttonLike);
	bool buttonLike() const{ return buttonLike_; }

	bool status() const { return status_; }

	virtual void onChanged();
	signal0& signalChanged() { return signalChanged_; }

	void serialize(Archive& ar);
protected:
	// внутренние функции
	CheckBoxImpl* window() const{ return reinterpret_cast<CheckBoxImpl*>(_window()); }

	signal0 signalChanged_;
	string text_;
	bool status_;
	bool buttonLike_;

	friend class CheckBoxImpl;
};

}

