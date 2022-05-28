/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"

#include "ww/RadioButton.h"
#include "ww/_WidgetWindow.h"
#include <algorithm>
#include <windowsx.h>

#include "ww/Serialization.h"
#include "ww/Unicode.h"
#include "yasli/ClassFactory.h"


namespace ww{

YASLI_CLASS(Widget, RadioButton, "Radio Button");

class RadioButtonGroup: public RefCounter{
public:
    void add(ww::RadioButton* rBtn);
	void remove(ww::RadioButton* rBtn);
	typedef std::vector<ww::RadioButton*> Group;
	Group group;
};

void RadioButtonGroup::add(ww::RadioButton* rBtn)
{
	group.push_back(rBtn);
}

void RadioButtonGroup::remove(ww::RadioButton* rBtn)
{
	Group::iterator i = std::find(group.begin(), group.end(), rBtn);
	if(i != group.end())
		group.erase(i);
}

#pragma warning(push)
#pragma warning(disable: 4355) // 'this' : used in base member initializer list

class RadioButtonImpl: public _WidgetWindow{
public:
	RadioButtonImpl(RadioButton* owner, RadioButton* groupWith);
	~RadioButtonImpl();
	const wchar_t* className() const{ return L"BUTTON"; }
	
	int onMessageKeyDown(UINT keyCode, USHORT count, USHORT flags);
	int onMessageSysKeyDown(UINT keyCode, USHORT count, USHORT flags);
	int onMessageSetFocus(HWND lastFocusedWindow);
	//BOOL onMessageEraseBkgnd(HDC dc);
	LRESULT defaultWindowProcedure(UINT message, WPARAM wparam, LPARAM lparam);

	void setRadioButtonText(const wchar_t* text);
	void setRadioButtonStatus(bool status);
	RadioButtonGroup* group() const { return group_; }
protected:
	void setFocusNext();
	void setFocusPrev();
	ww::RadioButton* owner_;
	SharedPtr<RadioButtonGroup> group_;
	static WNDPROC controlWindowProc_;
};

WNDPROC RadioButtonImpl::controlWindowProc_ = 0;

#pragma warning(push)
#pragma warning(disable: 4312) // 'type cast' : conversion from 'LONG' to 'HINSTANCE' of greater size

RadioButtonImpl::RadioButtonImpl(RadioButton* owner, RadioButton* groupWith)
: _WidgetWindow(owner)
, owner_(owner)
{
	if(!groupWith)
		group_ = new RadioButtonGroup();
	else
		group_ = groupWith->group();
	group_->add(owner);
	WW_VERIFY(create(L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_RADIOBUTTON | BS_NOTIFY, Rect(0, 0, 42, 42), Win32::getDefaultWindowHandle()));

	controlWindowProc_ = reinterpret_cast<WNDPROC>(::GetWindowLongPtr(handle_, GWLP_WNDPROC));
	::SetWindowLongPtr(handle_, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Win32::universalWindowProcedure));
	::SetWindowLongPtr(handle_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	SetWindowFont(handle_, Win32::defaultFont(), FALSE);
}

#pragma warning(pop)

RadioButtonImpl::~RadioButtonImpl()
{
	group_->remove(owner_);
}

void RadioButtonImpl::setFocusPrev()
{
	RadioButtonGroup::Group::iterator i;
	RadioButtonGroup::Group::iterator j;
	FOR_EACH(group()->group, i){
		if((*i) == owner_){
			if(i != group()->group.begin())
				::SetFocus((*j)->window()->handle());
			break;
		}
		j = i;
	}
}

void RadioButtonImpl::setFocusNext()
{
	RadioButtonGroup::Group::iterator i;
	bool mustBreak = false;
	FOR_EACH(group()->group, i){
		if(mustBreak){
			::SetFocus((*i)->window()->handle());
			break;
		}
		if((*i) == owner_)
			mustBreak = true;
	}
}

void RadioButtonImpl::setRadioButtonStatus(bool status)
{
	if(status){
		RadioButtonGroup::Group::iterator it; 
		FOR_EACH(group_->group, it)
			if(*it != owner_)
				(*it)->setStatus(false);
	}
	YASLI_ASSERT(::IsWindow(handle_));
	::SendMessageW(handle_, BM_SETCHECK, status ? BST_CHECKED : BST_UNCHECKED, 0);
}
void RadioButtonImpl::setRadioButtonText(const wchar_t* text)
{
	YASLI_ASSERT(::IsWindow(handle_));
	WW_VERIFY(::SetWindowTextW(handle_, text));

	HFONT font = GetWindowFont(handle_);
	Vect2 textSize = Win32::calculateTextSize(handle_, font, text);

	owner_->_setMinimalSize(textSize + Vect2(GetSystemMetrics(SM_CXBORDER) * 2 + 6 + 1,
		GetSystemMetrics(SM_CYBORDER) * 2 + 6 + 1) + Vect2(owner_->border(), owner_->border()));
	owner_->_queueRelayout();
}

int RadioButtonImpl::onMessageKeyDown(UINT keyCode, USHORT count, USHORT flags)
{
	if(keyCode == VK_UP || keyCode == VK_LEFT)
		setFocusPrev();
	else if(keyCode == VK_DOWN || keyCode == VK_RIGHT)
		setFocusNext();
	return __super::onMessageKeyDown(keyCode, count, flags);
}

int RadioButtonImpl::onMessageSysKeyDown(UINT keyCode, USHORT count, USHORT flags)
{
	if(keyCode == VK_UP || keyCode == VK_LEFT)
		setFocusPrev();
	else if(keyCode == VK_DOWN || keyCode == VK_RIGHT)
		setFocusNext();
	return __super::onMessageSysKeyDown(keyCode, count, flags);
}

LRESULT RadioButtonImpl::defaultWindowProcedure(UINT message, WPARAM wparam, LPARAM lparam)
{
	return ::CallWindowProcW(controlWindowProc_, handle_, message, wparam, lparam);
}

int RadioButtonImpl::onMessageSetFocus(HWND lastFocusedWindow)
{
    int result = __super::onMessageSetFocus(lastFocusedWindow);
	owner_->_setFocus();
	owner_->onChanged();
	setRadioButtonStatus(owner_->status());
    return result;
}


/*
BOOL RadioButtonImpl::onMessageEraseBkgnd(HDC dc)
{
	return __super::onMessageEraseBkgnd(dc);
}
*/

// --------------------------------------------------------------------------------------------

RadioButton::RadioButton(RadioButton* groupWith, const char* text, int border)
: _WidgetWithWindow(new RadioButtonImpl(this, groupWith), border), status_(groupWith == 0)
{
	_setMinimalSize(24, 24);
	setText(text);
	window()->setRadioButtonStatus(status_);
}

RadioButtonImpl* RadioButton::window() const
{
	return static_cast<RadioButtonImpl*>(_window());
}

RadioButtonGroup* RadioButton::group() const
{ 
	return window()->group(); 
}

void RadioButton::setText(const char* text)
{
	text_ = text;
	window()->setRadioButtonText(toWideChar(text).c_str());
}

void RadioButton::onChanged()
{
	if(!status_){
		status_ = !status_; 
		signalChanged_.emit();
	}	
}

void RadioButton::serialize(Archive& ar)
{
	if(ar.filter(SERIALIZE_DESIGN)){
		ar(text_, "text", "&Текст");
		ar(status_, "status", "&Статус");
	}
	Widget::serialize(ar);
}

void RadioButton::setStatus(bool status)
{
	status_ = status;
	window()->setRadioButtonStatus(status_);
}

};


#pragma warning(pop)

