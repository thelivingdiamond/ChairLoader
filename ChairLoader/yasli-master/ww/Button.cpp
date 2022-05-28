/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"

#include "ww/Button.h"
#include "ww/_WidgetWindow.h"
#include "ww/Serialization.h"
#include "ww/Unicode.h"
#include "yasli/ClassFactory.h"

#define WIN32_LEAN_AND_MEAN
#include <windowsx.h>

#pragma warning(push)
#pragma warning(disable: 4355) // 'this' : used in base member initializer list

namespace ww{
YASLI_CLASS(Widget, Button, "Button");

class ButtonImpl: public _WidgetWindow{
public:
	ButtonImpl(ww::Button* owner);
	
	BOOL onMessageWindowPosChanged(WINDOWPOS *windowPos);
	LRESULT onMessage(UINT message, WPARAM wparam, LPARAM lparam);
	int onMessageGetDlgCode(int keyCode, MSG* msg);

	void setButtonText(const wchar_t* text);
	void setDefaultButton(bool defaultBtn);
protected:
	ww::Button* owner_;
	HWND button_;
};

#pragma warning(push)
#pragma warning(disable: 4312) // 'type cast' : conversion from 'LONG' to 'HINSTANCE' of greater size

static WNDPROC buttonWindowProc_ = 0;

LRESULT CALLBACK buttonWindowProcedure(HWND handle, UINT message, WPARAM wparam, LPARAM lparam)
{

	if(message == WM_GETDLGCODE)
	{
		int keyCode = int(wparam);
		MSG* msg = (MSG*)lparam;
		if (!msg)
			return DLGC_WANTMESSAGE;

		if (msg->message == WM_KEYDOWN && msg->wParam == VK_RETURN)
			return DLGC_WANTMESSAGE;

		return 0;
	}
	if(message == WM_KEYDOWN)
	{
		SendMessageW(::GetParent(handle), WM_KEYDOWN, wparam, lparam);
		/*
		UINT key = UINT(wparam);
		if(key == VK_RETURN)
			SendMessageA(::GetParent(handle), WM_COMMAND, MAKEWPARAM(0, BN_CLICKED), (LONG) handle);
			*/
	}

	return ::CallWindowProcW(buttonWindowProc_, handle, message, wparam, lparam);
}

ButtonImpl::ButtonImpl(ww::Button* owner)
: _WidgetWindow(owner)
, owner_(owner)
, button_(0)
{
	HINSTANCE instance = (HINSTANCE)GetWindowLong(handle(), GWLP_HINSTANCE);
	WW_VERIFY(create(L"", WS_CHILD | WS_TABSTOP | WS_CLIPCHILDREN , Rect(0, 0, 42, 42), Win32::getDefaultWindowHandle()));
	button_ = ::CreateWindowW( L"BUTTON", L"Unnamed", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_NOTIFY, 0, 0, 42, 42, handle(), 0, instance, 0);
	YASLI_ASSERT(button_);
	buttonWindowProc_ = reinterpret_cast<WNDPROC>(::GetWindowLongPtrW(button_, GWLP_WNDPROC));
	::SetWindowLongPtrW(button_, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&buttonWindowProcedure));
	::SetWindowLongPtrW(button_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
	SetWindowFont(button_, Win32::defaultFont(), FALSE);
}

#pragma warning(pop)

void ButtonImpl::setDefaultButton(bool defaultBtn)
{
	if(defaultBtn)
		SendMessageW(button_, BM_SETSTYLE, BS_DEFPUSHBUTTON, TRUE); 
	else
		SendMessageW(button_, BM_SETSTYLE, BS_PUSHBUTTON, TRUE); 
}

void ButtonImpl::setButtonText(const wchar_t* text)
{
	YASLI_ESCAPE(::IsWindow(button_), return);
	WW_VERIFY(::SetWindowTextW(button_, text));

	HFONT font = GetWindowFont(button_);
	Vect2 textSize = Win32::calculateTextSize(button_, font, text);

	Vect2 extraSize(8, 8);
	Vect2 minimalSize = textSize + Vect2(owner_->border(), owner_->border()) * 2 + Vect2(GetSystemMetrics(SM_CXBORDER), GetSystemMetrics(SM_CYBORDER)) * 2 + extraSize;
	owner_->_setMinimalSize(minimalSize);
	owner_->_queueRelayout();
}


BOOL ButtonImpl::onMessageWindowPosChanged(WINDOWPOS *windowPos)
{
	if(button_)
	{
		int cx = windowPos->cx;
		int cy = windowPos->cy;
		::SetWindowPos(button_, 0, 0, 0, cx, cy, 0);
	}
	return FALSE;
}

int ButtonImpl::onMessageGetDlgCode(int keyCode, MSG* msg)
{
	if (!msg)
		return DLGC_WANTMESSAGE;

	if (msg->message == WM_KEYDOWN && keyCode == VK_RETURN)
		return DLGC_WANTMESSAGE;

	return 0;
}

LRESULT ButtonImpl::onMessage(UINT message, WPARAM wparam, LPARAM lparam)
{
	switch(message){
	case WM_SIZE:
		{
			if(button_){
				UINT width = LOWORD(lparam);
				UINT height = HIWORD(lparam);
				::MoveWindow(button_, 0, 0, width, height, TRUE);
			}
			return TRUE;
		}
	case WM_ENABLE:
		{
			BOOL enabled = BOOL(wparam);
			if(button_){
				::EnableWindow(button_, enabled);
				YASLI_ASSERT(::IsWindowEnabled(button_) == enabled);
			}
			return TRUE;
		}
	case WM_SETFOCUS:
		{
			::SetFocus(button_);
			break;
		}
		/*
	case WM_ERASEBKGND:
		{
			HDC dc = (HDC)(wparam);
			//::ExcludeClipRect(dc, 0, 0, 
			return FALSE;
		}*/

	case WM_KEYDOWN:
		{
			if (wparam == VK_RETURN)
				owner_->onPressed();
			break;
		}
	case WM_COMMAND:
		if(HWND(lparam) == button_){
			switch(HIWORD(wparam)){
			case BN_CLICKED:
			case BN_PUSHED:
				owner_->onPressed();
				return TRUE;
			case BN_SETFOCUS:
                owner_->_setFocus();
				return TRUE;
			}
		}
	}
	return __super::onMessage(message, wparam, lparam);
}


// --------------------------------------------------------------------------------------------

Button::Button(const char* text, int border)
: _WidgetWithWindow(new ButtonImpl(this), border)
{
	_setMinimalSize(24, 24);
	setText(text);
	defaultBtn_ = false;
}

void Button::setDefaultFrame(bool enable)
{
	defaultBtn_ = enable;
	window()->setDefaultButton(defaultBtn_);
}

void Button::setText(const char* text)
{
	text_ = text;
	window()->setButtonText(toWideChar(text).c_str());
}

void Button::onPressed()
{
	signalPressed_.emit();
}

void Button::serialize(Archive& ar)
{
	if(ar.filter(SERIALIZE_DESIGN)){
		ar(text_, "text", "&Текст");
	}
	Widget::serialize(ar);
}

};


#pragma warning(pop)

