/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "ww/HotkeyDialog.h"
#include "ww/HotkeyButton.h"

#include "ww/Win32/Window32.h"

namespace ww{

HotkeyDialog::HotkeyDialog(Widget* owner, KeyPress key)
: Dialog(owner, 0)
, button_(0)
{
	setMinimizeable(false);
	setResizeable(false);
	setShowTitleBar(false);
	button_ = new HotkeyButton(0);
	button_->signalChanged().connect(this, &HotkeyDialog::onChanged);
	add(button_, PACK_FILL);
}

HotkeyDialog::HotkeyDialog(HWND owner, KeyPress key)
: Dialog(owner, 0)
, button_(0)
{
	// COPY_PASTE
	setMinimizeable(false);
	setResizeable(false);
	setShowTitleBar(false);
	button_ = new HotkeyButton(0);
	button_->signalChanged().connect(this, &HotkeyDialog::onChanged);
	add(button_, PACK_FILL);
}

void HotkeyDialog::onChanged()
{
	key_ = button_->get();
	Dialog::onResponse(RESPONSE_OK);
}

void HotkeyDialog::onKeyDefault()
{
	// не вызываем __super::onKeyDefault
}

void HotkeyDialog::onKeyCancel()
{
	// не вызываем __super::onKeyCancel
}

}
