/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "ww/ComboBox.h"
#include "ww/Container.h"

#include "ww/_WidgetWindow.h"
#include "ww/Win32/Rectangle.h"
#include "ww/Win32/Drawing.h" // for Win32::isAppThemed

#include "ww/Serialization.h"
#include "ww/Unicode.h"
#include "yasli/ClassFactory.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

namespace ww{

#pragma warning(push)

YASLI_CLASS(Widget, ComboBox, "ComboBox")

class ComboBoxImpl : public _WidgetWindow{
public:
	ComboBoxImpl(ww::ComboBox* owner);
	~ComboBoxImpl();
	
	BOOL onMessageEraseBkgnd(HDC dc);
	BOOL onMessageEnable(BOOL enabled);
	int onMessageSetFocus(HWND lastFocusedWindow);
	BOOL onMessageSize(UINT type, USHORT width, USHORT height);

	LRESULT onMessage(UINT message, WPARAM wparam, LPARAM lparam);

	void showDropDown();
	HWND comboBox(){ return comboBoxHandle_; }
protected:
	HWND comboBoxHandle_;
	ww::ComboBox* owner_;

	friend class ww::ComboBox;
	static UINT WM_WW_FINISH;
};
UINT ComboBoxImpl::WM_WW_FINISH = RegisterWindowMessageW(L"ww.Finish");

ComboBoxImpl::ComboBoxImpl(ww::ComboBox* owner)
: _WidgetWindow(owner)
, owner_(owner)
, comboBoxHandle_(0)
{
	WW_VERIFY(create(L"", WS_CHILD, Rect(0, 0, 10, 10), Win32::getDefaultWindowHandle(), WS_EX_CONTROLPARENT));
	comboBoxHandle_ = ::CreateWindowExW(WS_EX_CLIENTEDGE, L"COMBOBOX", L"",
							   WS_CHILD | WS_TABSTOP | CBS_DROPDOWNLIST | WS_VISIBLE | WS_VSCROLL,
							 0, 0, 42, 42, handle(), 0, (HINSTANCE)(GetWindowLong(handle(), GWLP_HINSTANCE)), 0);
	
	YASLI_ASSERT(comboBoxHandle_);
	SetWindowFont(comboBoxHandle_, Win32::defaultFont(), FALSE);
}

ComboBoxImpl::~ComboBoxImpl()
{
}

LRESULT ComboBoxImpl::onMessage(UINT message, WPARAM wparam, LPARAM lparam)
{
	if(message == WM_WW_FINISH){
		SharedPtr<ww::Widget> ref(owner_->refCount() == 0 ? 0 : owner_);
		owner_->onEdited();
		return 0;
	}
	switch(message){
	case WM_COMMAND:
		if(IsWindowVisible(comboBoxHandle_)){
			switch(HIWORD(wparam)){
				case CBN_SELCHANGE:
					{
					owner_->selectedIndex_ = SendMessageW(comboBoxHandle_, CB_GETCURSEL, wparam, lparam);
					owner_->onSelectionChanged();
					break;
					}
				case CBN_SELENDOK:
				{
					owner_->selectedIndex_ = SendMessageW(comboBoxHandle_, CB_GETCURSEL, wparam, lparam);
					PostMessageW(handle(), WM_WW_FINISH, WPARAM(handle()), 0);
					return 0;
					break;
				}
				case CBN_CLOSEUP:
				{
					PostMessageW(handle(), WM_WW_FINISH, WPARAM(handle()), 0);
					return 0;
					break;
				}
				case CBN_SELENDCANCEL:
				{
					break;
				}
				case CBN_KILLFOCUS:
				{
					owner_->selectedIndex_ = SendMessageW(comboBoxHandle_, CB_GETCURSEL, 0, 0);
					PostMessageW(handle(), WM_WW_FINISH, WPARAM(handle()), 0);
					return 0;
					break;
				}
				case CBN_SETFOCUS:
				{
					break;
				}
			}
		}
		break;
	case WM_SHOWWINDOW:
		UINT msg = message;
		ShowWindow(comboBoxHandle_, msg);
		break;
	}
	return __super::onMessage(message, wparam, lparam);
}

BOOL ComboBoxImpl::onMessageSize(UINT type, USHORT width, USHORT height)
{
	if(comboBoxHandle_)
		::MoveWindow(comboBoxHandle_, 0, 0, width, 800/*height*owner_->items_.size()*/, TRUE);
	return TRUE;
}


BOOL ComboBoxImpl::onMessageEraseBkgnd(HDC dc)
{
	return FALSE;
}

BOOL ComboBoxImpl::onMessageEnable(BOOL enabled)
{
	::EnableWindow(comboBoxHandle_, enabled);
	YASLI_ASSERT(::IsWindowEnabled(comboBoxHandle_) == enabled);
	return FALSE;
}

int ComboBoxImpl::onMessageSetFocus(HWND lastFocusedWindow)
{
	int result = __super::onMessageSetFocus(lastFocusedWindow);
    owner_->_setFocus();
	//if (comboBoxHandle_)
	//	SetFocus(comboBoxHandle_);
    return 0;
}

void ComboBoxImpl::showDropDown()
{
	//SetFocus(comboBoxHandle_);
	ComboBox_ShowDropdown(comboBoxHandle_, TRUE);
}

// --------------------------------------------------- 


#pragma warning(push)
#pragma warning(disable: 4355) //  'this' : used in base member initializer list
ComboBox::ComboBox(bool expandByContent, int border)
: expandByContent_(expandByContent)
, dropDownHeight_(10)
, selectedIndex_(0)
, _WidgetWithWindow(new ComboBoxImpl(this), border)
{
	updateMinimalSize();
}

#pragma warning(pop)

ComboBox::ComboBox(ComboBoxImpl* impl, bool expandByContent, int border)
: expandByContent_(expandByContent)
, dropDownHeight_(10)
, selectedIndex_(0)
, _WidgetWithWindow(impl, border)
{
}

void ComboBox::updateMinimalSize()
{
	Vect2 size = Win32::calculateTextSize(impl()->comboBox(), GetWindowFont(impl()->handle()), L" ");
	if(expandByContent_){
		Items::iterator it;
		FOR_EACH(items_, it){
			Vect2 anotherSize = Win32::calculateTextSize(impl()->handle(), GetWindowFont(impl()->handle()), toWideChar(it->c_str()).c_str());
			size.x = std::max(anotherSize.x, size.x);
			size.y = std::max(anotherSize.y, size.y);
		}
	}
	bool themed = Win32::isAppThemed(); // FIXME: hack
	_setMinimalSize(size + (themed ? Vect2(4, 4) : Vect2(4, 2)) + Vect2(GetSystemMetrics(SM_CXFOCUSBORDER), GetSystemMetrics(SM_CYFOCUSBORDER)) * 2
		            + Vect2(border_, border_) * 2 + Vect2(GetSystemMetrics(SM_CXBORDER), GetSystemMetrics(SM_CYBORDER)));
}


void ComboBox::_setPosition(const Rect& position)
{
	Widget::_setPosition(position);

	Win32::Window32* window = _findWindow(parent());
	YASLI_ASSERT(window);
	Win32::Window32::PositionDeferer deferer = window->positionDeferer();
	Rect pos(position.left() + border_, position.top() + border_,
		      position.right() - border_ * 2, position.bottom() - border_ * 2);
	deferer->defer(_window(), pos);
}

void ComboBox::setExpandByContent(bool expand)
{
	expandByContent_ = expand;
	updateMinimalSize();
	_queueRelayout();
}

void ComboBox::showDropDown()
{
	impl()->showDropDown();
}

void ComboBox::set(const char* comboList, const char* value, bool onlyVisible)
{
	clear();
	StringList strings;
	splitStringList(&strings, comboList, '|');
	StringList::iterator it;
	int index = 0;
	int selectedIndex = 0;

	FOR_EACH(strings, it){
		if(!onlyVisible || it->c_str()[0] != '\0'){
			add(it->c_str());
			if(*it == value)
				selectedIndex = index;
			++index;
		}
	}
	setSelectedIndex(selectedIndex);
}


void ComboBox::set(const StringListValue& value, bool onlyVisible)
{
	//set(value.stringList(), value.c_str(), onlyVisible);
	clear();
	StringList::const_iterator it;
	int index = 0;
	int selectedIndex = 0;

	FOR_EACH(value.stringList(), it){
		if(!onlyVisible || it->c_str()[0] != '\0'){
			add(it->c_str());
			if(*it == value.c_str())
				selectedIndex = index;
			++index;
		}
	}
	setSelectedIndex(selectedIndex);
}

void ComboBox::get(StringListValue& value)
{
	string str;
    if(size_t(selectedIndex_) < items_.size())
      value = StringListValue( items_, items_[selectedIndex_].c_str() );
    else
      value = StringListValue( items_, 0 );
}

void ComboBox::get(StringListStaticValue& value)
{
	string str;
    if(size_t(selectedIndex_) < items_.size())
        value = selectedIndex_;
    else
        value = 0;
}

void ComboBox::setSelectedIndex(int selectedIndex)
{
	selectedIndex_ = selectedIndex;
	::SendMessageW(impl()->comboBox(), CB_SETCURSEL, selectedIndex, 0);
}

int ComboBox::selectedIndex() const
{
    return selectedIndex_;
}

string ComboBox::selectedString() const
{
	YASLI_ASSERT(selectedIndex_ >= 0 && selectedIndex_ < int(items_.size()));
	if(selectedIndex_ >= 0 && selectedIndex_ < int(items_.size())){
		Items::const_iterator it = items_.begin();
		std::advance(it, selectedIndex_);
		return *it;
	}
	return "";
}

void ComboBox::clear()
{
	items_.clear();
	::SendMessageW(impl()->comboBox(), CB_RESETCONTENT, 0, 0);
	updateMinimalSize();
	_queueRelayout();
}

void ComboBox::onEdited()
{
	SharedPtr<ww::Widget> ref(refCount() == 0 ? 0 : this);
	signalEdited_.emit();
}

void ComboBox::add(const char* text)
{
	insert(end(), text);
}

void ComboBox::insert(iterator before, const char* text)
{
	int index = before == items_.end() ? -1 : std::distance(items_.begin(), before);
	items_.insert(before, text);
	::SendMessageW(impl()->comboBox(), CB_INSERTSTRING, index, (LPARAM)toWideChar(text).c_str());
	if(items_.size() == 1)
		::SendMessageW(impl()->comboBox(), CB_SETCURSEL, 0, 0);
	updateMinimalSize();
	_queueRelayout();
}

void ComboBox::setDropDownHeight(int lines)
{
	dropDownHeight_ = lines;
	_setPosition(_position());
}

int ComboBox::dropDownHeight() const
{
	return dropDownHeight_;
}

void ComboBox::setFocus()
{
	if(_window())
		::SetFocus(impl()->comboBoxHandle_);
}

void ComboBox::serialize(Archive& ar)
{
	ar(items_, "items", "Items");
	ar(selectedIndex_, "selectedIndex", "&Selected Item Index");
	ar(dropDownHeight_, "dropDownHeight_", "Drop Down Height");
	ar(expandByContent_, "expandByContent", "Expand by Content");
	Widget::serialize(ar);
}

}
#pragma warning(pop)
