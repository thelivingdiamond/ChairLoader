/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"

#include "ww/ProgressBar.h"
#include "ww/_WidgetWindow.h"
#include <windowsx.h>
#include "Win32/CommonControls.h"

#include "ww/Serialization.h"
#include "yasli/ClassFactory.h"

#define MAX_RANGE 10000

#pragma message("Automatically linking with comctl32.lib") 
#pragma comment(lib, "comctl32.lib") 

namespace ww{

YASLI_CLASS(Widget, ProgressBar, "ProgressBar");
#pragma warning(push)
#pragma warning(disable: 4355) // 'this' : used in base member initializer list

class ProgressBarImpl: public _WidgetWindow{
public:
	ProgressBarImpl(ProgressBar* owner);
	const wchar_t* className() const{ return PROGRESS_CLASSW; }
	LRESULT onMessage(UINT message, WPARAM wparam, LPARAM lparam);

	void setProgressBarPosition(float pos);
protected:
	ww::ProgressBar* owner_;
	static WNDPROC controlWindowProc_;
};

WNDPROC ProgressBarImpl::controlWindowProc_ = 0;

#pragma warning(push)
#pragma warning(disable: 4312) // 'type cast' : conversion from 'LONG' to 'HINSTANCE' of greater size

ProgressBarImpl::ProgressBarImpl(ProgressBar* owner)
: _WidgetWindow(owner)
, owner_(owner)
{
	InitCommonControls();
	WW_VERIFY(create(L"", WS_CHILD | PBS_SMOOTH | WS_VISIBLE | WS_CLIPCHILDREN, Rect(0, 0, 42, 42), Win32::getDefaultWindowHandle()));
	controlWindowProc_ = reinterpret_cast<WNDPROC>(::GetWindowLongPtr(handle_, GWLP_WNDPROC));
	::SetWindowLongPtr(handle_, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Win32::universalWindowProcedure));
	::SetWindowLongPtr(handle_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
	SendMessageW(handle_, PBM_SETRANGE, 0, MAKELPARAM(0, MAX_RANGE)); 
	SetWindowFont(handle_, Win32::defaultFont(), FALSE);
}

void ProgressBarImpl::setProgressBarPosition(float pos)
{
	SendMessageW(handle_, PBM_SETPOS, (int)(pos * MAX_RANGE), 0); 	
}

#pragma warning(pop)

LRESULT ProgressBarImpl::onMessage(UINT message, WPARAM wparam, LPARAM lparam)
{
	return ::CallWindowProcW(controlWindowProc_, handle_, message, wparam, lparam);
}

// --------------------------------------------------------------------------------------------

ProgressBar::ProgressBar(int border)
: _WidgetWithWindow(new ProgressBarImpl(this), border)
{
    _setMinimalSize(4, 4);
    setRequestSize(24, 24);
    pos_ = 0.f;
    setPosition(pos_);
}

void ProgressBar::serialize(Archive& ar)
{
    if(ar.filter(SERIALIZE_DESIGN)){
        ar(pos_, "pos", 0);
    }
    Widget::serialize(ar);
}

void ProgressBar::setPosition(float pos)
{
    YASLI_ASSERT((pos >= 0) && (pos <= 1));
    pos_ = pos;
    window()->setProgressBarPosition(pos_);
}
};


#pragma warning(pop)

