/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "Tabs.h"
#include "ww/SafeCast.h"
#include "ww/Unicode.h"
#include "ww/_WidgetWindow.h"
#include "ww/Win32/Window32.h"
#include "ww/Win32/MemoryDC.h"
#include "ww/Win32/Rectangle.h"
#include "ww/Win32/Handle.h"
#include "ww/Win32/Drawing.h"
#include "ww/PropertyTree/wwDrawContext.h"
#include "gdiplusUtils.h"
#include "yasli/Archive.h"
#include <algorithm>

namespace ww{

struct TabsItem{
	string text;
	Win32::Rect rect;
};

class TabsImpl : public _WidgetWindow{
public:
	TabsImpl(Tabs* tabs);

	void redraw(HDC dc);
	int onMessageGetDlgCode(int keyCode, MSG* msg);
	void onMessagePaint();
	BOOL onMessageSize(UINT type, USHORT width, USHORT height);
	BOOL onMessageEraseBkgnd(HDC dc);
	int onMessageSetFocus(HWND lastFocusedWindow);
	int onMessageKillFocus(HWND focusedWindow);
	void onMessageLButtonDown(UINT button, int x, int y);
	void onMessageMButtonDown(UINT button, int x, int y);
	void onMessageRButtonDown(UINT button, int x, int y);
	int onMessageKeyDown(UINT keyCode, USHORT count, USHORT flags);

	void recalculateRects();
protected:
	typedef std::vector<TabsItem> Items;
	Items items_;
	Tabs* owner_;
	int selectedTab_;
	friend class Tabs;
};

TabsImpl::TabsImpl(Tabs* tabs)
: _WidgetWindow(tabs)
, selectedTab_(0)
, owner_(0)
{
	WW_VERIFY(create(L"", WS_CHILD | WS_TABSTOP, Rect(0, 0, 20, 20), Win32::getDefaultWindowHandle()));
	owner_ = tabs;
}

int TabsImpl::onMessageKeyDown(UINT keyCode, USHORT count, USHORT flags)
{
	if(keyCode == VK_LEFT){
		if(selectedTab_ > 0){
			owner_->setSelectedTab(selectedTab_ - 1, owner_);
		}
	}
	if(keyCode == VK_RIGHT){
		if(size_t(selectedTab_ + 1) < items_.size()){
			owner_->setSelectedTab(selectedTab_ + 1, owner_);
		}
	}
	return __super::onMessageKeyDown(keyCode, count, flags);
}

void TabsImpl::onMessagePaint()
{
	PAINTSTRUCT ps;
	HDC dc = BeginPaint(handle_, &ps);
	{
		Win32::MemoryDC memoryDC(dc);
		redraw(memoryDC);
	}
	EndPaint(handle_, &ps);
}

BOOL TabsImpl::onMessageSize(UINT type, USHORT width, USHORT height)
{
	if(owner_)
		recalculateRects();
	return __super::onMessageSize(type, width, height);
}

int TabsImpl::onMessageSetFocus(HWND lastFocusedWindow)
{
	bool redraw = false;
	if(owner_){
		owner_->_setFocus();
		redraw = true;
	}
	int result = Window32::onMessageSetFocus(lastFocusedWindow);
	if (redraw)
		RedrawWindow(handle_, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
	return result;
}

int TabsImpl::onMessageKillFocus(HWND focusedWindow)
{
	if(owner_)
		RedrawWindow(handle_, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
	return Window32::onMessageKillFocus(focusedWindow);
}

BOOL TabsImpl::onMessageEraseBkgnd(HDC dc)
{
	return FALSE;
}

int TabsImpl::onMessageGetDlgCode(int keyCode, MSG* msg)
{
	if (msg == 0)
		return DLGC_WANTMESSAGE;
	if (msg->message == WM_KEYDOWN && (keyCode == VK_LEFT || keyCode == VK_RIGHT))
		return DLGC_WANTMESSAGE;
	
	return 0;
}

void TabsImpl::recalculateRects()
{
	Win32::Rect rect;
	GetClientRect(handle(), &rect);
	int offset = 0;
	Items::iterator it;

	const int PADDING_X = 8;
	const int PADDING_Y = 4;

	int height = 20;
	int index = 0;

	for( it = items_.begin(); it != items_.end(); ++it ){
		TabsItem& item = *it;

		HFONT font = selectedTab_ == index ? Win32::defaultBoldFont() : Win32::defaultFont();
		Vect2 size = Win32::calculateTextSize(handle(), font, toWideChar(item.text.c_str()).c_str());

		size.x += PADDING_X * 2;
		size.y += PADDING_Y * 2;
		height = std::max(size.y, height);

		item.rect = Win32::Rect(offset, 0, size.x + offset, size.y);
		offset += size.x;
		++index;
	}
	for( it = items_.begin(); it != items_.end(); ++it ){
		TabsItem& item = *it;
		item.rect.bottom = rect.height();//item.rect.top + height;
		if(offset > rect.width()){
			float scale =  rect.width() / float(offset);
			item.rect.left = xround(item.rect.left * scale);
			item.rect.right = xround(item.rect.right * scale);
		}
	}
	owner_->_setMinimalSize(20, height);
	::RedrawWindow(handle(), 0, 0, RDW_INVALIDATE);
}


void TabsImpl::redraw(HDC dc)
{
	Win32::Rect rect;
	GetClientRect(handle(), &rect);
    using namespace Gdiplus;
		using Gdiplus::Color;

    Graphics gr(dc);
    gr.FillRectangle( &SolidBrush(gdiplusSysColor(COLOR_BTNFACE)), gdiplusRect(rect) );
	gr.SetSmoothingMode(SmoothingModeAntiAlias);

	int index = 0;
	Items::iterator it;
    Pen pen(gdiplusSysColor(COLOR_3DHIGHLIGHT), 1);

	Gdiplus::Point lineStart;
    for( it = items_.begin(); it != items_.end(); ++it ){
		TabsItem& item = *it;
		bool selected = selectedTab_ == index;
		bool focused = selected && GetFocus() == handle();

		const int roundness = 8;
		if(selected){
			Gdiplus::Rect rect(gdiplusRect(item.rect));
			LinearGradientBrush brush(Gdiplus::Rect(rect.X, rect.Y, rect.Width, rect.Height), Gdiplus::Color(), Gdiplus::Color(), LinearGradientModeVertical);

            Gdiplus::Color colors[3] = {
                gdiplusSysColor(COLOR_WINDOW),
                gdiplusSysColor(COLOR_WINDOW),
                gdiplusSysColor(COLOR_BTNFACE)
            };
            Gdiplus::REAL positions[3] = { 0.0f, 0.4f, 1.0f };
            brush.SetInterpolationColors(colors, positions, 3);

			Gdiplus::SolidBrush shadowBrush(gdiplusSysColor(COLOR_3DSHADOW));
            rect.Height += roundness;
            fillRoundRectangle(&gr, &brush, 
							   Gdiplus::Rect(rect.X, rect.Y, rect.Width - 1, rect.Height),
                               gdiplusSysColor(COLOR_3DSHADOW), roundness);

			lineStart = Gdiplus::Point(item.rect.right - 1, item.rect.bottom - 1);
		}
		else{
			Gdiplus::Rect rect(gdiplusRect(item.rect));
			rect.Width -= 1;
            rect.Height += roundness;
            fillRoundRectangle(&gr, &SolidBrush(gdiplusSysColor(COLOR_BTNFACE)), rect, gdiplusSysColor(COLOR_3DSHADOW), roundness);

			gr.DrawLine(&Pen(gdiplusSysColor(COLOR_3DHIGHLIGHT)), item.rect.left, item.rect.bottom - 1, item.rect.right, item.rect.bottom - 1);
			lineStart = Gdiplus::Point(item.rect.right - 1, item.rect.bottom - 1);
		}		

		wstring text(toWideChar(item.text.c_str()));

		Win32::Rect textRect = item.rect;
		textRect.left += 3;
		textRect.right -= 3;
		if(!selected){
			textRect.top += 1;
			textRect.bottom += 1;
		}

		Gdiplus::Font* font = selected ? property_tree::propertyTreeDefaultBoldFont() : property_tree::propertyTreeDefaultFont();
		StringFormat format;
		format.SetAlignment(Gdiplus::StringAlignmentCenter);
		format.SetLineAlignment(Gdiplus::StringAlignmentCenter);
		format.SetTrimming(Gdiplus::StringTrimmingNone);
		format.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap);
		gr.DrawString(text.c_str(), int(wcslen(text.c_str())), font, gdiplusRectF(textRect), &format, &SolidBrush(gdiplusSysColor(COLOR_WINDOWTEXT)));

		if(focused){
			Win32::Rect focusRect = item.rect;
			focusRect.left += 3;
			focusRect.top += 4;
			focusRect.right -= 4;
			focusRect.bottom -= 2;
			::DrawFocusRect(dc, &focusRect);
		}

		++index;
	}
	if(!items_.empty())
		gr.DrawLine(&Pen(gdiplusSysColor(COLOR_3DHIGHLIGHT)), lineStart.X, lineStart.Y, rect.right, rect.bottom - 1);
}

void TabsImpl::onMessageMButtonDown(UINT button, int x, int y)
{
	int index = 0;
	Items::iterator it;
    for( it = items_.begin(); it != items_.end(); ++it ){
		TabsItem& item = *it;
		if(item.rect.pointIn(Vect2(x, y))){
			owner_->signalMouseButtonDown().emit(MOUSE_BUTTON_MIDDLE, index);
			break;
		}
		++index;
	}
	__super::onMessageMButtonDown(button, x, y);
}

void TabsImpl::onMessageRButtonDown(UINT button, int x, int y)
{
	int index = 0;
	Items::iterator it;
    for( it = items_.begin(); it != items_.end(); ++it ){
		TabsItem& item = *it;
		if(item.rect.pointIn(Vect2(x, y))){
			owner_->signalMouseButtonDown().emit(MOUSE_BUTTON_RIGHT, index);
			break;
		}
		++index;
	}

	__super::onMessageRButtonDown(button, x, y);
}

void TabsImpl::onMessageLButtonDown(UINT button, int x, int y)
{
	::SetFocus(handle());
	int index = 0;
	Items::iterator it;
    for( it = items_.begin(); it != items_.end(); ++it ){
		TabsItem& item = *it;
		if(item.rect.pointIn(Vect2(x, y))){
			if(index != selectedTab_){
				owner_->setSelectedTab(index);
			}
		}
		++index;
	}
	RedrawWindow(handle(), 0, 0, RDW_INVALIDATE);
	__super::onMessageLButtonDown(button, x, y);
}


// ---------------------------------------------------------------------------

#pragma warning(push)
#pragma warning(disable: 4355) // warning C4355: 'this' : used in base member initializer list
Tabs::Tabs(int border)
: _WidgetWithWindow(new TabsImpl(this), border)
{
	_setMinimalSize(20, 20);
	impl().recalculateRects();
}
#pragma warning(pop)

TabsImpl& Tabs::impl()
{
	return *safe_cast<TabsImpl*>(_window());
}

int Tabs::selectedTab()
{
	return impl().selectedTab_;
}

void Tabs::setSelectedTab(int index, const TabChanger* changer)
{
	int oldTab = impl().selectedTab_;
	impl().selectedTab_ = index;
	if(oldTab != index){
		impl().recalculateRects();
		signalChanged_.emit(changer);
	}
	RedrawWindow(impl().handle(), 0, 0, RDW_INVALIDATE);
}

size_t Tabs::add(const char* tabTitle, int before)
{
	TabsItem item;
	item.text = tabTitle;
	item.rect = Win32::Rect(0, 0, 0, 0);
	impl().items_.push_back(item);
	impl().recalculateRects(); 
	return impl().items_.size() - 1;
}

void Tabs::remove(int index)
{
	YASLI_ASSERT(size_t(index) < impl().items_.size());
	if(size_t(index) >= impl().items_.size())
		return;

	impl().items_.erase(impl().items_.begin());
	impl().recalculateRects();
}

void Tabs::clear()
{
	impl().selectedTab_ = -1;
	impl().items_.clear();
}

void Tabs::serialize(Archive& ar)
{
	if(ar.filter(SERIALIZE_STATE)){
		int index = selectedTab();
		ar(index, "selectedTab", "selectedTab");
		if (size_t(index) < impl().items_.size())
			setSelectedTab(index);
	}
}

// ---------------------------------------------------------------------------

TabPages::TabPages(int border)
: VBox(2, border)
{
	tabs_ = new Tabs();
	tabs_->signalChanged().connect(this, &TabPages::onTabChange);
	VBox::add(tabs_);
}


int TabPages::add(const char* title, Widget* widget, int before)
{
	YASLI_ASSERT(widget);
	tabs_->add(title, before);

	if(before > -1)
		widgets_.insert(widgets_.begin() + before, widget);
	else
		widgets_.push_back(widget);

	if(widgets_.size() == 1){
		VBox::add(widget, PACK_FILL);
	}
	return int(widgets_.size() - 1);
}

void TabPages::onTabChange(const TabChanger* changer)
{
	if(changer == this)
		return;
	setSelectedTab(tabs_->selectedTab());
}

void TabPages::remove(int index)
{
	if(index == tabs_->selectedTab() && VBox::size() > 1){
		VBox::remove(&*widgets_[index]);
	}
	tabs_->remove(index);
	widgets_.erase(widgets_.begin() + index);
	setSelectedTab(index);
}

Widget* TabPages::widgetByIndex(int index)
{
	return widgets_.at(index);
}

size_t TabPages::size() const
{
	return widgets_.size();
}

void TabPages::setSelectedTab(int index)
{
	if(VBox::size() > 1)
		VBox::remove(1);
	YASLI_ESCAPE(size_t(index) < widgets_.size(), return);
	Widget* widget = widgets_.at(index);
	VBox::add(widget, PACK_FILL);
	tabs_->setSelectedTab(index);
	showAll();
}

void TabPages::serialize(yasli::Archive& ar)
{
	__super::serialize(ar);

	if(ar.filter(SERIALIZE_STATE)){
        for(size_t i = 0; i < widgets_.size(); ++i){
			ar(*widgets_[i], "widget", "Widget");
        }
	}
}

}
