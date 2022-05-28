/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "ww/Win32/Window32.h"

namespace ww{

class _WidgetWithWindow;
class _ContainerWithWindow;


class _WidgetWindow : public Win32::Window32{
public:
	_WidgetWindow(_WidgetWithWindow* owner, HWND handle = 0);

	LRESULT onMessage(UINT message, WPARAM wparam, LPARAM lparam);
	int onMessageKeyDown(UINT keyCode, USHORT count, USHORT flags);
	int onMessageSysKeyDown(UINT keyCode, USHORT count, USHORT flags);
	int onMessageSetFocus(HWND lastFocusedWindow);
	ww::_WidgetWithWindow* owner(){ return owner_; }
protected:
	ww::_WidgetWithWindow* owner_;
};


class _ContainerWindow : public Win32::Window32{
public:
	_ContainerWindow(_ContainerWithWindow* owner);

	int onMessageKeyDown(UINT keyCode, USHORT count, USHORT flags);
	ww::_ContainerWithWindow* owner(){ return owner_; }
protected:
	ww::_ContainerWithWindow* owner_;
};

}
