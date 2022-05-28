/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "ww/Dialog.h"
#include "KeyPress.h"

namespace ww{

class HotkeyButton;

class HotkeyDialog : public Dialog{
public:
	HotkeyDialog(Widget* owner, KeyPress key = KeyPress());
	HotkeyDialog(HWND owner, KeyPress key = KeyPress());
	KeyPress& get(){ return key_; }

	void onChanged();

	void onKeyDefault();
	void onKeyCancel();
protected:
	HotkeyButton* button_;
	KeyPress key_;
};

}

