/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "ww/Slider.h"
#include "ww/_WidgetWindow.h"
#include "ww/Win32/MemoryDC.h"
#include "ww/Win32/Rectangle.h"
#include "ww/Win32/Drawing.h"

namespace ww{

class SliderImpl : public _WidgetWindow{
public:
	SliderImpl(Slider* owner);

	void redraw(HDC dc);

	int onMessageGetDlgCode(int keyCode, MSG* msg)
	{
		if (!msg)
			return DLGC_WANTMESSAGE;
		if (msg->message == WM_KEYDOWN && (keyCode == VK_LEFT || keyCode == VK_RIGHT))
			return DLGC_WANTMESSAGE;
		return 0;
	}
	int onMessageSetFocus(HWND lastFocusedWindow);
	int onMessageKillFocus(HWND focusedWindow);
	int onMessageKeyDown(UINT keyCode, USHORT count, USHORT flags);
	void onMessageLButtonDown(UINT button, int x, int y);
	void onMessageLButtonUp(UINT button, int x, int y);
	void onMessageMouseMove(UINT button, int x, int y);
	void onMessagePaint();
	BOOL onMessageEraseBkgnd(HDC dc);
protected:
    float mousePositionToValue(int x, int y) const;

	Slider* owner_;
	bool dragging_;
};

SliderImpl::SliderImpl(Slider* owner)
: _WidgetWindow(owner)
, owner_(owner)
, dragging_(false)
{
	create(L"", WS_CHILD | WS_TABSTOP, Rect(0, 0, 20, 20), Win32::getDefaultWindowHandle());
}

void SliderImpl::redraw(HDC dc)
{
	Win32::Rect rect;
	GetClientRect(handle(), &rect);

	FillRect(dc, &rect, GetSysColorBrush(COLOR_BTNFACE));
	bool focused = GetFocus() == handle();
	if (focused)
		DrawFocusRect(dc, &rect);
	rect.left += 1;	rect.top += 1;
	rect.right -= 1; rect.bottom -= 1;
	drawSlider(dc, rect, owner_->value(), focused);
}

void SliderImpl::onMessagePaint()
{
	PAINTSTRUCT paintStruct;
	HDC dc = BeginPaint(handle(), &paintStruct);
	{
		Win32::MemoryDC memoryDC(dc);
		redraw(memoryDC);
	}
	EndPaint(handle(), &paintStruct);
}

BOOL SliderImpl::onMessageEraseBkgnd(HDC dc)
{
	return false;
}


int SliderImpl::onMessageSetFocus(HWND lastFocusedWindow)
{
	::RedrawWindow(handle(), 0, 0, RDW_INVALIDATE);
	return __super::onMessageSetFocus(lastFocusedWindow);
}

int SliderImpl::onMessageKillFocus(HWND focusedWindow)
{
	::RedrawWindow(handle(), 0, 0, RDW_INVALIDATE);
	return __super::onMessageKillFocus(focusedWindow);
}

int SliderImpl::onMessageKeyDown(UINT keyCode, USHORT count, USHORT flags)
{
	float value = owner_->value_;
	if (keyCode == VK_LEFT){
		if (GetKeyState(VK_CONTROL) >> 15)
			value = value - 0.1f * count;
		else
			value = value - 0.01f * count;
	}
	else if (keyCode == VK_RIGHT){
		if (GetKeyState(VK_CONTROL) >> 15)
			value = value + 0.1f * count;
		else
			value = value + 0.01f * count;
	}
	else if (keyCode == VK_HOME){
		value = 0.0f;
	}
	else if (keyCode == VK_END){
		value = 1.0f;
	}

	if (value != owner_->value_)
	{
		owner_->value_ = clamp(value, 0.0f, 1.0f);
		owner_->signalChanged_.emit();
		InvalidateRect(handle(), 0, FALSE);
	}
	return __super::onMessageKeyDown(keyCode, count, flags);
}

float SliderImpl::mousePositionToValue(int x, int y) const
{
    Win32::Rect rect;
    GetClientRect(handle(), &rect);
    const int handleWidth = 4;
    return clamp(float(x - handleWidth / 2) / float(rect.width() - handleWidth - 2), 0.0f, 1.0f);
}


void SliderImpl::onMessageMouseMove(UINT button, int x, int y)
{
	if((button & MK_LBUTTON) && dragging_){
		float value = mousePositionToValue(x, y);
		owner_->value_ = value;
		owner_->signalChanged_.emit();
		::RedrawWindow(handle(), 0, 0, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW | RDW_NOERASE);
	}
	else
		dragging_ = false;
	__super::onMessageMouseMove(button, x ,y);
}

void SliderImpl::onMessageLButtonDown(UINT button, int x, int y)
{
	Win32::Rect rect;
	GetClientRect(handle(), &rect);
	float value = mousePositionToValue(x, y);
	owner_->value_ = value;
	owner_->signalChanged_.emit();
	::RedrawWindow(handle(), 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);

	dragging_ = true;
	SetFocus(handle());
	owner_->_setFocus();
	SetCapture(handle());
	__super::onMessageLButtonDown(button, x ,y);
}

void SliderImpl::onMessageLButtonUp(UINT button, int x, int y)
{
	dragging_ = false;
	if(GetCapture() == handle())
		ReleaseCapture();
}

// ---------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4355) // 'this' : used in base member initializer list
Slider::Slider(int border)
: _WidgetWithWindow(new SliderImpl(this), border)
, stepsCount_(0)
{
    _setMinimalSize(20, 20);
}
#pragma warning(pop)

SliderImpl& Slider::impl()
{
	return static_cast<SliderImpl&>(*__super::_window());
}

void Slider::setValue(float value)
{
	value_ = value;
	RedrawWindow(impl().handle(), 0, 0, RDW_INVALIDATE);
}

void Slider::setStepsCount(int stepsCount)
{
	stepsCount_ = stepsCount;
}

}
