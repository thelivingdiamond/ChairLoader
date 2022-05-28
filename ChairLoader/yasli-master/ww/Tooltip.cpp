/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "ww/Win32/Window32.h"
#include "ww/Widget.h"
#include "ww/Tooltip.h"
#include "Win32/CommonControls.h"

namespace ww{
	

// ---------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4355) // 'this' : used in base member initializer list

Tooltip::Tooltip(const char* text, bool baloon)
: text_(text)
, widget_(0)
, baloon_(baloon)
, toolTipWindow_(0)
, offset_(15, 15)
{
}

#pragma warning(pop)

void Tooltip::setBaloon(bool baloon)
{
	baloon_ = baloon;
}

void Tooltip::attach(Widget* widget)
{
	widget_ = widget;
	widget_->setToolTip(this);

	Win32::Window32* window = _findWindow(widget);
	YASLI_ASSERT(window);
	HWND ownerHandle = window->handle();
	YASLI_ASSERT(::IsWindow(ownerHandle));

	if(!toolTipWindow_){
		toolTipWindow_ = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, 0,
			WS_POPUP | TTS_ALWAYSTIP | (baloon_ ? TTS_BALLOON : 0),
			0, 0, 0, 0,
			ownerHandle, 0, Win32::_globalInstance(), 0);
	}

	TOOLINFO    ti;
	ti.cbSize = 44;//sizeof(TOOLINFO);
	ti.uFlags = TTF_TRACK;
	ti.hwnd   = ownerHandle;
	ti.uId    = 12345;
	ti.hinst  = Win32::_globalInstance();
	ti.lpszText  = LPSTR_TEXTCALLBACK;
	ti.rect.left = ti.rect.top = ti.rect.bottom = ti.rect.right = 0; 

	YASLI_ASSERT(::IsWindow(toolTipWindow_));
	WW_VERIFY(SendMessageW(toolTipWindow_, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti));
	WW_VERIFY(SendMessageW(toolTipWindow_, TTM_TRACKACTIVATE,(WPARAM)TRUE, (LPARAM)&ti));
}

void Tooltip::setText(const char* text)
{
	text_ = text;
	
	if(!widget_)
		return;
	
	attach(widget_);

	if(!text_.empty()){
		TRACKMOUSEEVENT event;
		ZeroMemory((void*)(&event), sizeof(event));
		event.cbSize = sizeof(event);
		event.dwFlags = TME_LEAVE;
		event.hwndTrack = 0;
		if(Win32::Window32* window = _findWindow(widget_))
			event.hwndTrack = window->handle();
		WW_VERIFY(TrackMouseEvent(&event));
	}
}

void Tooltip::show()
{
	POINT point;
	GetCursorPos(&point);

	SendMessageW(toolTipWindow_, TTM_TRACKPOSITION, 0, (LPARAM)MAKELONG(point.x + offset_.x, point.y + offset_.y));
}

void Tooltip::hide()
{
	setText("");

	TOOLINFO    ti;
	ti.cbSize = 44;//sizeof(TOOLINFO);
	ti.uFlags = TTF_TRACK;
	ti.hwnd   = 0;
	if (Win32::Window32* window = _findWindow(widget_))
		ti.hwnd = window->handle();
	ti.uId    = 12345;
	ti.hinst  = Win32::_globalInstance();
	ti.lpszText  = LPSTR_TEXTCALLBACK;
	ti.rect.left = ti.rect.top = ti.rect.bottom = ti.rect.right = 0; 

	WW_VERIFY(SendMessageW(toolTipWindow_, TTM_TRACKACTIVATE,(WPARAM)FALSE, (LPARAM)&ti));
}


}
