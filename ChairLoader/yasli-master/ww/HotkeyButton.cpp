/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "ww/HotkeyButton.h"
#include "ww/_WidgetWindow.h"
#include "ww/KeyPress.h"


namespace ww{

class HotkeyButtonImpl : public _WidgetWindow{
public:
	HotkeyButtonImpl(HotkeyButton* owner);

	void redraw(HDC dc);
	LRESULT onMessage(UINT message, WPARAM wparam, LPARAM lparam);

	void onMessagePaint();
	void onMessageTimer(int id);
	int onMessageKeyUp(UINT keyCode, USHORT count, USHORT flags);
	int onMessageKeyDown(UINT keyCode, USHORT count, USHORT flags);
	int onMessageSysKeyUp(UINT keyCode, USHORT count, USHORT flags);
	int onMessageSysKeyDown(UINT keyCode, USHORT count, USHORT flags);
	void onMessageLButtonDblClk(int x, int y);
	void onMessageLButtonDown(UINT button, int x, int y);
	void onMessageLButtonUp(UINT button, int x, int y);
	void onMessageMButtonDown(UINT button, int x, int y);
	void onMessageMButtonUp(UINT button, int x, int y);
	void onMessageRButtonDown(UINT button, int x, int y);
	void onMessageRButtonUp(UINT button, int x, int y);
	int onMessageSetFocus(HWND lastFocusedWindow);
	int onMessageKillFocus(HWND focusedWindow);
protected:
	HotkeyButton* owner_;
	bool pressed_;
	int ticksLeft_;
};

static const int TIMER_ID = 100;

HotkeyButtonImpl::HotkeyButtonImpl(HotkeyButton* owner)
: _WidgetWindow(owner)
, owner_(owner)
, pressed_(false)
, ticksLeft_(-1)
{
	WW_VERIFY(create(L"", WS_CHILD | WS_TABSTOP, Rect(0, 0, 42, 42), Win32::getDefaultWindowHandle()));
	::SetTimer(handle(), TIMER_ID, 100, 0);
}

void HotkeyButtonImpl::redraw(HDC dc)
{
	HGDIOBJ oldFont = ::SelectObject(dc, Win32::defaultBoldFont());
	int oldBkMode = ::SetBkMode(dc, TRANSPARENT);
	
	RECT rt;
	GetClientRect(handle(), &rt);
	const wchar_t* str = L"Press a Key, Please...";
	::DrawTextW(dc, str, (int)wcslen(str), &rt, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

	::SelectObject(dc, oldFont);
}

LRESULT HotkeyButtonImpl::onMessage(UINT message, WPARAM wparam, LPARAM lparam)
{
	return __super::onMessage(message, wparam, lparam);
}

void HotkeyButtonImpl::onMessagePaint()
{
	PAINTSTRUCT ps;
	HDC dc = BeginPaint(handle(), &ps);
	redraw(dc);
	EndPaint(handle(), &ps);
}

void HotkeyButtonImpl::onMessageTimer(int id)
{
	if(id == TIMER_ID){
		if(ticksLeft_ > 0){
			--ticksLeft_;
			if(ticksLeft_ == 0 &&
				(owner_->key_.key == VK_LBUTTON ||
				owner_->key_.key == VK_RBUTTON ||
				owner_->key_.key == VK_MBUTTON))
				owner_->signalChanged_.emit();
		}
	}
	__super::onMessageTimer(id);
}

void HotkeyButtonImpl::onMessageLButtonDblClk(int x, int y)
{
//	owner_->key_ = KeyPress(addModifiers(VK_LDBL));
//	owner_->signalChanged_.emit();

	__super::onMessageLButtonDblClk(x, y);
}


int HotkeyButtonImpl::onMessageSetFocus(HWND lastFocusedWindow)
{
	owner_->_setFocus();
    return 0;
}

int HotkeyButtonImpl::onMessageKillFocus(HWND focusedWindow)
{
	return 0;
}

int HotkeyButtonImpl::onMessageKeyDown(UINT keyCode, USHORT count, USHORT flags)
{
	if(((flags >> 14) & 1) == 0){ // кнопка не была нажата
		pressed_ = true;
		owner_->key_ = KeyPress::addModifiers(Key(keyCode));
		if(keyCode != VK_SHIFT &&
		   keyCode != VK_CONTROL &&
		   keyCode != VK_MENU)
			owner_->signalChanged_.emit();
	}
	return __super::onMessageKeyDown(keyCode, count, flags);
}

int HotkeyButtonImpl::onMessageKeyUp(UINT keyCode, USHORT count, USHORT flags)
{
	if(pressed_){
		owner_->key_ = KeyPress::addModifiers(Key(keyCode));
		owner_->signalChanged_.emit();
	}
	return __super::onMessageKeyUp(keyCode, count, flags);
}

int HotkeyButtonImpl::onMessageSysKeyDown(UINT keyCode, USHORT count, USHORT flags)
{
	owner_->key_ = KeyPress::addModifiers(Key(keyCode));
	pressed_ = true;

	if(keyCode == VK_CANCEL)
		owner_->signalChanged_.emit();

	if(keyCode != VK_SHIFT &&
	   keyCode != VK_CONTROL &&
	   keyCode != VK_MENU)
		owner_->signalChanged_.emit();
	return __super::onMessageSysKeyDown(keyCode, count, flags);
}

int HotkeyButtonImpl::onMessageSysKeyUp(UINT keyCode, USHORT count, USHORT flags)
{
	if(pressed_){
		owner_->key_ = KeyPress::addModifiers(Key(keyCode));
		owner_->signalChanged_.emit();
	}
	return __super::onMessageSysKeyUp(keyCode, count, flags);
}

void HotkeyButtonImpl::onMessageLButtonDown(UINT button, int x, int y)
{
	::SetFocus(handle());
	pressed_ = true;
}

void HotkeyButtonImpl::onMessageLButtonUp(UINT button, int x, int y)
{
	if(pressed_){
		owner_->key_ = KeyPress::addModifiers(Key(VK_LBUTTON));
		ticksLeft_ = 30;
	}
}

void HotkeyButtonImpl::onMessageMButtonDown(UINT button, int x, int y)
{
	pressed_ = true;
}

void HotkeyButtonImpl::onMessageMButtonUp(UINT button, int x, int y)
{
	if(pressed_){
		owner_->key_ = KeyPress::addModifiers(Key(VK_MBUTTON));
		ticksLeft_ = 30;
	}
}

void HotkeyButtonImpl::onMessageRButtonDown(UINT button, int x, int y)
{
	pressed_ = true;
}

void HotkeyButtonImpl::onMessageRButtonUp(UINT button, int x, int y)
{
	if(pressed_){
		owner_->key_ = KeyPress::addModifiers(Key(VK_RBUTTON));
		ticksLeft_ = 30;
	}
}

// --------------------------------------------------------------------------- 
#pragma warning(push)
#pragma warning(disable: 4355) // 'this' : used in base member initializer list

HotkeyButton::HotkeyButton(int border)
: _WidgetWithWindow(new HotkeyButtonImpl(this), border)
{
    _setMinimalSize(220, 24);
}

#pragma warning(pop)
HotkeyButtonImpl& HotkeyButton::impl()
{
	return static_cast<HotkeyButtonImpl&>(*__super::_window());
}

}
