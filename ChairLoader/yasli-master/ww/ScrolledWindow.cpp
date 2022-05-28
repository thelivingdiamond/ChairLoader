/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "ww/ScrolledWindow.h"
#include "ww/_WidgetWindow.h"

#include "ww/Serialization.h"
#include "yasli/ClassFactory.h"


namespace ww{

class ScrolledWindowImpl: public _ContainerWindow{
public:
	ScrolledWindowImpl(ScrolledWindow* owner);
	BOOL onMessageSize(UINT type, USHORT width, USHORT height);
	LRESULT onMessage(UINT message, WPARAM wparam, LPARAM lparam);

	void updateScrollBars();
protected:
	ww::ScrolledWindow* owner_;
};

ScrolledWindowImpl::ScrolledWindowImpl(ScrolledWindow* owner)
: _ContainerWindow(owner)
, owner_(owner)
{
	WW_VERIFY(create(L"scrolledWindow", WS_CHILD | WS_TABSTOP | WS_HSCROLL | WS_VSCROLL | WS_CLIPCHILDREN, Rect(0, 0, 40, 40), Win32::getDefaultWindowHandle()));
}


BOOL ScrolledWindowImpl::onMessageSize(UINT type, USHORT width, USHORT height)
{
	if(!creating())
		updateScrollBars();
	return TRUE;
}

LRESULT ScrolledWindowImpl::onMessage(UINT message, WPARAM wparam, LPARAM lparam)
{
	switch(message){
	case WM_HSCROLL:
	case WM_VSCROLL:
		{
			SCROLLINFO info;
			info.cbSize = sizeof(info);
			info.fMask  = SIF_ALL;
			::GetScrollInfo(handle_, message == WM_VSCROLL ? SB_VERT : SB_HORZ, &info);
			int oldPosition = info.nPos;
			switch(LOWORD(wparam)){
			case SB_TOP:        info.nPos = info.nMin; break;
			case SB_BOTTOM:     info.nPos = info.nMax; break;
			case SB_LINEUP:     info.nPos -= 1; break;
			case SB_LINEDOWN:   info.nPos += 1; break;
			case SB_PAGEUP:     info.nPos -= info.nPage; break;
			case SB_PAGEDOWN:   info.nPos += info.nPage; break;
			case SB_THUMBTRACK: info.nPos = info.nTrackPos; break;
			default:
				break; 
			}

			info.fMask = SIF_POS;

			if(message == WM_VSCROLL){
				::SetScrollInfo(handle_, SB_VERT, &info, TRUE);
				::GetScrollInfo(handle_, SB_VERT, &info);
				owner_->offset_.y = int(info.nPos);
			}
			else{
				::SetScrollInfo(handle_, SB_HORZ, &info, TRUE);
				::GetScrollInfo(handle_, SB_HORZ, &info);
				owner_->offset_.x = int(info.nPos);
			}

			if(info.nPos != oldPosition){                    
				owner_->_arrangeChildren();
				updateScrollBars();
			}
		}
		return 0;
    //case WM_ERASEBKGND:
    //    if(this)
    //        FillRect(GetDC(handle()), 0, GetSysColorBrush(COLOR_BTNSHADOW));
    //    return 0;
	default:
		break;
	}
	return Win32::Window32::onMessage(message, wparam, lparam);
}




void ScrolledWindowImpl::updateScrollBars()
{
	ww::Widget* widget = owner_->child_;
	if(widget){
		SCROLLINFO scrollInfo;

		memset((void*)&scrollInfo, 0, sizeof(scrollInfo));
		scrollInfo.cbSize = sizeof(SCROLLINFO);
		scrollInfo.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
		scrollInfo.nMin = 0;
		if(owner_->policyVertical_ == ww::SCROLL_NEVER){
			scrollInfo.nMax = 0;
			scrollInfo.nPos = 1;
		}
		else{
			scrollInfo.nMax = widget->_minimalSize().y;
			scrollInfo.nPos = owner_->offset_.y;
		}
		scrollInfo.nPage = owner_->clientAreaSize().y;
		SetScrollInfo(handle_, SB_VERT, &scrollInfo, TRUE);

		memset((void*)&scrollInfo, 0, sizeof(scrollInfo));
		scrollInfo.cbSize = sizeof(SCROLLINFO);
		scrollInfo.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
		scrollInfo.nMin = 0;
		if(owner_->policyHorizontal_ == ww::SCROLL_NEVER){
			scrollInfo.nMax = 0;
			scrollInfo.nPos = 1;
		}
		else{
			scrollInfo.nMax = widget->_minimalSize().x;
			scrollInfo.nPos = owner_->offset_.x;
		}
		scrollInfo.nPage = owner_->clientAreaSize().x;
		SetScrollInfo(handle_, SB_HORZ, &scrollInfo, TRUE);
	}
}

// ------------------------------------------------------------------


#pragma warning(push)
#pragma warning(disable: 4355) //  'this' : used in base member initializer list
ScrolledWindow::ScrolledWindow(int border)
: _ContainerWithWindow(new ScrolledWindowImpl(this), border)
, offset_(0, 0)
, policyHorizontal_(SCROLL_AUTOMATIC)
, policyVertical_(SCROLL_AUTOMATIC)
{
	setBorder(border);
	_setMinimalSize(0, 0);
}
#pragma warning(pop)

ScrolledWindow::~ScrolledWindow()
{
	if(child_)
		child_->_setParent(0);
}

void ScrolledWindow::setPolicy(ScrollPolicy horizontalPolicy, ScrollPolicy verticalPolicy)
{
	policyHorizontal_ = horizontalPolicy;
	policyVertical_ = verticalPolicy;
}

void ScrolledWindow::add(Widget* widget, bool fill)
{	
    fill_ = fill;
	child_ = widget;
	widget->_setParent(this);
	_arrangeChildren();
	impl()->updateScrollBars();
}

void ScrolledWindow::remove()
{
	child_ = 0;
}

void ScrolledWindow::visitChildren(WidgetVisitor& visitor) const
{
	if(child_)
		visitor(*child_);
}

void ScrolledWindow::_arrangeChildren()
{
    using std::min;
    using std::max;
    if(child_){
        Vect2 clientSize = clientAreaSize();
        Vect2 minSize = child_->_minimalSize();
        Vect2 size;
        if(fill_)
            size.set(max(minSize.x, clientSize.x), max(minSize.y, clientSize.y));
        else
            size = child_->_minimalSize();

        offset_.x = clamp(offset_.x, 0, minSize.x - clientSize.x);
        offset_.y = clamp(offset_.y, 0, minSize.y - clientSize.y);

        Vect2 pos = -offset_;
        if(size.x <= clientSize.x)
            pos.x = (clientSize.x - size.x) / 2;
        if(size.y <= clientSize.y)
            pos.y = (clientSize.y - size.y) / 2;

        Rect rect(pos, pos + size);
        child_->_setPosition(rect);
    }
}

void ScrolledWindow::_relayoutParents()
{
	Vect2 oldMinimalSize = _minimalSize();
	int h = (policyHorizontal_ == SCROLL_NEVER && child_)
		? child_->_minimalSize().x + GetSystemMetrics(SM_CXVSCROLL)
		: GetSystemMetrics(SM_CXVSCROLL) + GetSystemMetrics(SM_CXHSCROLL) * 2;
	int v = (policyVertical_ == SCROLL_NEVER && child_)
		? child_->_minimalSize().x + GetSystemMetrics(SM_CYHSCROLL)
		: GetSystemMetrics(SM_CYHSCROLL) + GetSystemMetrics(SM_CYVSCROLL) * 2;
	_setMinimalSize(h + border_ * 2, v + border_ * 2);
	impl()->updateScrollBars();
	_arrangeChildren();
	Container::_relayoutParents(oldMinimalSize != _minimalSize());
}

ScrolledWindowImpl* ScrolledWindow::impl()
{
	return static_cast<ScrolledWindowImpl*>(_window());
}

Vect2 ScrolledWindow::clientAreaSize() const
{
	Vect2 result(_position().width() - border() * 2, _position().height() - border() * 2);
	if(policyHorizontal_ == SCROLL_ALWAYS){
		result.y -= GetSystemMetrics(SM_CYHSCROLL);
	}
	else{
		if(policyVertical_ == SCROLL_AUTOMATIC && child_ && child_->_minimalSize().y > result.y){
			result.x -= GetSystemMetrics(SM_CXVSCROLL);
		}
	}
	if(policyVertical_ == SCROLL_ALWAYS){
		result.x -= GetSystemMetrics(SM_CXVSCROLL);
	}
	else{
		if(policyHorizontal_ == SCROLL_AUTOMATIC && child_ && child_->_minimalSize().x > result.x){
			result.y -= GetSystemMetrics(SM_CYVSCROLL);
		}
	}
	return result;	
}

bool ScrolledWindow::horizontalScrollVisible() const
{
	return true;
}

bool ScrolledWindow::verticalScrollVisible() const
{
	return true;
}

void ScrolledWindow::serialize(Archive& ar)
{
    if(ar.filter(SERIALIZE_STATE)){
        ar(offset_, "offset", "Offset");
        if(child_)
            child_->serialize(ar);
    }
}

}
