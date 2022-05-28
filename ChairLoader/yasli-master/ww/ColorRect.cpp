/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "ww/SafeCast.h"
#include "ww/ColorRect.h"
#include "ww/_WidgetWindow.h"
#include "ww/Win32/MemoryDC.h"

namespace ww{

class ColorRectImpl : public _WidgetWindow{
public:
	ColorRectImpl(ColorRect* owner);

	BOOL onMessageEraseBkgnd(HDC);
	void onMessagePaint();

	void redraw(HDC dc);
protected:
	ColorRect* owner_;
};

ColorRectImpl::ColorRectImpl(ColorRect* owner)
: _WidgetWindow(owner)
, owner_(owner)
{
	WW_VERIFY(create(L"", WS_CHILD, Rect(0, 0, 10, 10), Win32::getDefaultWindowHandle()));
}


void ColorRectImpl::redraw(HDC dc)
{
	RECT rect;
	GetClientRect(handle(), &rect);
	::FillRect(dc, &rect, GetSysColorBrush(COLOR_BTNFACE));

	int roundness = 7;
	::SelectObject(dc, GetStockObject(NULL_PEN));
	::SelectObject(dc, GetSysColorBrush(COLOR_3DSHADOW));
	::RoundRect(dc, rect.left, rect.top, rect.right, rect.bottom, roundness, roundness);
	
	HRGN region = CreateRoundRectRgn(rect.left + 1, rect.top + 1, rect.right - 1, rect.bottom - 1, roundness, roundness - 2);
	::SelectClipRgn(dc, region);

	Color c(owner_->color_);
	HBRUSH brush = CreateSolidBrush(RGB(c.r, c.g, c.b));

	RECT filledRect = { rect.left + 1, rect.top + 1, rect.right - 1, rect.bottom - 1};
	
	Color black = Color(0, 0, 0).interpolate(owner_->get(), owner_->get().a / 255.0f);
	Color white = Color(255, 255, 255).interpolate(owner_->get(), owner_->get().a / 255.0f);
	float w = (filledRect.right - filledRect.left) * 0.25f;
	float h = (filledRect.bottom - filledRect.top) * 0.25f;

	HBRUSH blackBrush = CreateSolidBrush(black.rgb());
	HBRUSH whiteBrush = CreateSolidBrush(white.rgb());

	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 2; ++j) {
			RECT rect = { xround(i * w), xround(j * h + h * 2), xround((i + 1) * w), xround ((j + 1) * h + h * 2) };
			FillRect(dc, &rect, ((i + j) % 2) ? whiteBrush : blackBrush);
		}
	}

	RECT r = { filledRect.left, filledRect.top, filledRect.left + xround(w * 4), filledRect.top + xround(h * 2) };
	::FillRect(dc, &r, brush);

	::SelectClipRgn(dc, 0);
	DeleteObject(blackBrush);
	DeleteObject(whiteBrush);
	DeleteObject(brush);
	DeleteObject(region);
}

void ColorRectImpl::onMessagePaint()
{
	PAINTSTRUCT ps;
	HDC dc = BeginPaint(handle(), &ps);
	{
		Win32::MemoryDC memoryDC(dc);
		redraw(memoryDC);
	}
	EndPaint(handle(), &ps);
}

BOOL ColorRectImpl::onMessageEraseBkgnd(HDC)
{
	return FALSE;
}

// ---------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4355) // 'this' : used in base member initializer list
ColorRect::ColorRect(Color color, int border)
: _WidgetWithWindow(new ColorRectImpl(this), border)
{
	_setMinimalSize(40, 40);
}
#pragma warning(pop)

void ColorRect::set(const Color& color)
{
	color_ = color;
	if(::IsWindowVisible(impl().handle()))
		RedrawWindow(impl().handle(), 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
}


ColorRect::~ColorRect()
{

}

ColorRectImpl& ColorRect::impl()
{
	return *safe_cast<ColorRectImpl*>(_window());
}

}
