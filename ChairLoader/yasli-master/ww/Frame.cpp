/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"

#include "ww/Frame.h"
#include "ww/Unicode.h"
#include "ww/_WidgetWindow.h"
#include <windowsx.h>

#include "ww/Serialization.h"
#include "yasli/ClassFactory.h"
#include "ExcludeClipRectVisitor.h"


namespace ww{

YASLI_CLASS(Container, Frame, "Frame");

#pragma warning(push)
#pragma warning(disable: 4355) // 'this' : used in base member initializer list

class FrameImpl: public _ContainerWindow{
public:
	FrameImpl(Frame* owner);
	LRESULT onMessage(UINT message, WPARAM wparam, LPARAM lparam);

	HFONT font() { return owner_->emphasis() ? Win32::defaultBoldFont() : Win32::defaultFont(); }
	void setFrameText(const wchar_t* text);
protected:
	ww::Frame* owner_;
//	static WNDPROC controlWindowProc_;
};
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable: 4312) // 'type cast' : onversion from 'LONG' to 'HINSTANCE' of greater size

FrameImpl::FrameImpl(Frame* owner)
: _ContainerWindow(owner)
, owner_(owner)
{
	WW_VERIFY(create(L"", WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, Rect(0, 0, 42, 42), Win32::getDefaultWindowHandle(), WS_EX_CONTROLPARENT));
}

#pragma warning(pop)

void FrameImpl::setFrameText(const wchar_t* text)
{
	YASLI_ASSERT(::IsWindow(handle_));
	WW_VERIFY(::SetWindowTextW(handle_, text));

	HFONT font = GetWindowFont(handle_);
	Vect2 textSize = Win32::calculateTextSize(handle_, font, text);

	owner_->_setMinimalSize(textSize + Vect2(GetSystemMetrics(SM_CXBORDER) * 2 + 6 + 1,
		GetSystemMetrics(SM_CYBORDER) * 2 + 6 + 1) + Vect2(owner_->border(), owner_->border()));
	owner_->_queueRelayout();

}

LRESULT FrameImpl::onMessage(UINT message, WPARAM wparam, LPARAM lparam)
{
	switch(message){
	case WM_PAINT:
		{
			PAINTSTRUCT paintStruct;
			HDC paintDC = ::BeginPaint(handle_, &paintStruct);
			{
				HDC dc = paintDC;
				RECT clientRect;
				::GetClientRect(handle(), &clientRect);
				::FillRect(dc, &clientRect, ::GetSysColorBrush(COLOR_BTNFACE));

				Rect rect(clientRect.left, clientRect.top, clientRect.right, clientRect.bottom);
				rect.setTop(clientRect.top);
				rect.setLeft(clientRect.left);
				rect.setBottom(clientRect.bottom);
				rect.setRight(clientRect.right);
				int spacing = 3;
				int hspacing = 3;
				int vspacing = 3;
				RECT windowRect = { rect.left() + hspacing,
					rect.top() + vspacing*2 + 1,
					rect.right() - hspacing,
					rect.bottom() - vspacing };
				::DrawEdge(dc, &windowRect, EDGE_ETCHED, BF_RECT);
				HFONT oldFont = HFONT(::SelectObject(dc, (HGDIOBJ)font()));
				HBRUSH oldBrush = HBRUSH(::SelectObject(dc, (HGDIOBJ)(::GetStockObject(GRAY_BRUSH))));
				int oldBackMode = ::SetBkMode(dc, OPAQUE);
				COLORREF oldBkColor = ::SetBkColor(dc, ::GetSysColor(COLOR_BTNFACE));
                wstring text = toWideChar(owner_->text());
				HFONT fontInt = font();
				Vect2 textSize = Win32::calculateTextSize(handle_, fontInt, text.c_str());
				RECT fillRect = { rect.left() + hspacing*2 - 1,
					rect.top(),
					rect.left() + hspacing*2 + textSize.x + 2,
					rect.top() + textSize.y };
				::FillRect(dc, &fillRect, ::GetSysColorBrush(COLOR_BTNFACE));
				RECT textRect = { rect.left() + hspacing*2,
					rect.top(),
					rect.right() - hspacing,
					rect.bottom() - vspacing };
				DrawTextW(dc, text.c_str(), int(wcslen(text.c_str())), &textRect, FS_CYRILLIC);
				::SelectObject(dc, oldFont);
				::SelectObject(dc, oldBrush);
				::SetBkMode(dc, oldBackMode);
				::SetBkColor(dc, oldBkColor);
			}
			EndPaint(handle_, &paintStruct);
			
			return __super::onMessage(message, wparam, lparam);
		}
	}
	return __super::onMessage(message, wparam, lparam);
}



// --------------------------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4355) // 'this' : used in base member initializer list

Frame::Frame(const char* text, bool emphasis, int border)
: _ContainerWithWindow(new FrameImpl(this), border)
, space_(7)
, emphasis_(emphasis)
{
	_setMinimalSize(24, 24);
	setText(text);
}

#pragma warning(pop)

Frame::~Frame()
{
	if(child_)
		child_->_setParent(0);
}

void Frame::setEmphasis(bool emphasis)
{
	emphasis_ = emphasis;
}

void Frame::setText(const char* text)
{
	text_ = text;
	safe_cast<FrameImpl*>(_window())->setFrameText(toWideChar(text).c_str());
}

void Frame::serialize(Archive& ar)
{
	ar(text_, "text", "&Текст");
	Widget::serialize(ar);
}

void Frame::add(Widget* widget)
{
	child_ = widget;
	widget->_setParent(this);
	_arrangeChildren();
}

void Frame::_arrangeChildren()
{
	if(child_){
		RECT rect;
		WW_VERIFY(::GetClientRect(_window()->handle(), &rect));
		WW_VERIFY(::InflateRect(&rect, -border_, -border_));
		child_->_setPosition(Rect(rect.left + space_, rect.top + 2 * space_, 
			                       rect.right - space_, rect.bottom - 2 * space_));
	}
}

void Frame::_relayoutParents()
{
	if(child_)
		_setMinimalSize(child_->_minimalSize() + Vect2(border_ * 2 + space_ * 2, border_ * 2 + space_ * 3));
	else
		_setMinimalSize(border_ * 2, border_ * 2);
	
	RECT clientRect;
	::GetClientRect(_window()->handle(), &clientRect);
	bool move = false;
	RECT windowRect;
	::GetWindowRect(_window()->handle(), &windowRect);
	SIZE borderSize = { windowRect.right - windowRect.left - (clientRect.right - clientRect.left),
		windowRect.bottom - windowRect.top - (clientRect.bottom - clientRect.top) };

	if(clientRect.right - clientRect.left < _minimalSize().x){
		windowRect.right = windowRect.left + _minimalSize().x + borderSize.cx;
		move = true;
	}
	if(clientRect.bottom - clientRect.top < _minimalSize().y){
		windowRect.bottom = windowRect.top + _minimalSize().y + borderSize.cy;
		move = true;
	}
	if(move){
		_window()->move(Rect(windowRect.left, windowRect.top, windowRect.right, windowRect.bottom));
		_window()->update();
	}
	else
		::RedrawWindow(_window()->handle(), 0, 0, RDW_INVALIDATE | RDW_ALLCHILDREN);

	Widget::_relayoutParents();
}

void Frame::visitChildren(WidgetVisitor& visitor) const
{
	if(child_){
		visitor(*child_);
	}
}

void Frame::remove()
{
	child_ = 0;
}

};



