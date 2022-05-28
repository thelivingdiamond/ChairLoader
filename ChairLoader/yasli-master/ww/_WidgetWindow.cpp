/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "ww/_WidgetWindow.h"
#include "ww/HotkeyContext.h"
#include "ww/_WidgetWithWindow.h"
#include "ww/Tooltip.h"
#include "ww/Unicode.h"
#include "Win32/CommonControls.h"

namespace ww{

_WidgetWindow::_WidgetWindow(_WidgetWithWindow* owner, HWND handle)
: Window32(handle)
, owner_(owner)
{

}

int _WidgetWindow::onMessageSysKeyDown(UINT keyCode, USHORT count, USHORT flags)
{
	HotkeyContext* hotkeyContext = owner_->_hotkeyContext();
	if(hotkeyContext){
		if(hotkeyContext->injectPress(KeyPress::addModifiers(Key(keyCode))))
			return 0;
	}
	return __super::onMessageSysKeyDown(keyCode, count, flags);
}

LRESULT _WidgetWindow::onMessage(UINT message, WPARAM wparam, LPARAM lparam)
{
	if(message == WM_MOUSELEAVE && owner_->toolTip())
		owner_->toolTip()->hide();

	if(message == WM_NOTIFY){
		NMHDR* pNMHDR = (NMHDR*)lparam;
		if(pNMHDR->code == TTN_GETDISPINFO && owner_->toolTip()){
			LPNMTTDISPINFOW pdi = (LPNMTTDISPINFOW)lparam;
			::SendMessageW(pdi->hdr.hwndFrom, TTM_SETMAXTIPWIDTH, 0, GetSystemMetrics(SM_CXSCREEN));
			wstring tooltipText = toWideChar(owner_->toolTip()->text());
			pdi->lpszText = (wchar_t*)tooltipText.c_str();
			pdi->hinst = 0;
		}
	}
	return __super::onMessage(message, wparam, lparam);
}

int _WidgetWindow::onMessageKeyDown(UINT keyCode, USHORT count, USHORT flags)
{
	HotkeyContext* hotkeyContext = owner_->_hotkeyContext();
	if(hotkeyContext && !((flags >> 14) & 1)){
		if(hotkeyContext->injectPress(KeyPress::addModifiers(Key(keyCode))))
			return 0;
	}
	return __super::onMessageKeyDown(keyCode, count, flags);
}

int _WidgetWindow::onMessageSetFocus(HWND lastFocusedWindow)
{
    int result = __super::onMessageSetFocus(lastFocusedWindow);
    owner_->_setFocus();
    return result;
}

// ---------------------------------------------------------------------------

_ContainerWindow::_ContainerWindow(_ContainerWithWindow* owner)
: owner_(owner)
{

}

int _ContainerWindow::onMessageKeyDown(UINT keyCode, USHORT count, USHORT flags)
{
	HotkeyContext* hotkeyContext = owner_->_hotkeyContext();
	if(hotkeyContext && !((flags >> 14) & 1)){
		if(hotkeyContext->injectPress(KeyPress::addModifiers(Key(keyCode))))
			return 0;
	}
	return __super::onMessageKeyDown(keyCode, count, flags);
}

}
