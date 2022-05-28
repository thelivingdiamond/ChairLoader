/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include <windows.h>
#include <windowsx.h>

#include "ww/CheckComboBox.h"

#include "ww/Container.h"

#include "ww/_WidgetWindow.h"
#include "ww/Unicode.h"
#include "ww/Win32/Window32.h"

#include "ww/Serialization.h"
#include "yasli/ClassFactory.h"
#include "yasli/StringList.h"

namespace ww{
YASLI_CLASS(Widget, CheckComboBox, "CheckComboBox")

class CheckComboBoxImpl : public _WidgetWindow{
public:
	CheckComboBoxImpl(ww::CheckComboBox* owner);
	const wchar_t* className() const{ return L"COMBOBOX"; }

	BOOL onMessageMeasureItem(UINT id, MEASUREITEMSTRUCT* drawItemStruct);
	BOOL onMessageDrawItem(UINT id, DRAWITEMSTRUCT* drawItemStruct);
	BOOL onMessageEnable(BOOL enabled);
	int onMessageSetFocus(HWND lastFocusedWindow);
	int onMessageKillFocus(HWND focusedWindow);
	int onMessageCommand(USHORT command, USHORT code, HWND wnd);
	LRESULT onMessage(UINT message, WPARAM wparam, LPARAM lparam);

	LRESULT defaultWindowProcedure(UINT message, WPARAM wparam, LPARAM lparam);
	void onEdited();
	void selectAll() {}
	void showDropDown();
	HWND comboBox(){ return handle_; }
	bool getCheck(size_t index) const;
	void setCheck(size_t index, bool checked);
	void updateValue();
protected:
	friend class CheckComboListBoxImpl;
	ww::CheckComboBox* owner_;
	SharedPtr<CheckComboListBoxImpl> listBox_;
	bool itemHeightSet_;
	static WNDPROC controlWindowProc_;
};


// ---------------------------------------------------------------------------

WNDPROC CheckComboBoxImpl::controlWindowProc_ = 0;

class CheckComboListBoxImpl : public Win32::Window32{
public:
	CheckComboListBoxImpl(HWND handle, CheckComboBoxImpl* owner);
	~CheckComboListBoxImpl();

	void onMessageRButtonDown(UINT button, int x, int y);
	void onMessageLButtonDown(UINT button, int x, int y);
	void onMessageLButtonUp(UINT button, int x, int y);
	int onMessageDestroy();
	int onMessageChar(UINT code, USHORT count, USHORT flags);
	LRESULT onMessage(UINT message, WPARAM wparam, LPARAM lparam);
	LRESULT defaultWindowProcedure(UINT message, WPARAM wparam, LPARAM lparam);
protected:
	CheckComboBoxImpl* owner_;
	static WNDPROC controlWindowProc_;
};

WNDPROC CheckComboListBoxImpl::controlWindowProc_ = 0;

CheckComboListBoxImpl::CheckComboListBoxImpl(HWND handle, CheckComboBoxImpl* owner)
: Window32(handle)
, owner_(owner)
{
	controlWindowProc_ = reinterpret_cast<WNDPROC>(::GetWindowLongPtr(handle, GWLP_WNDPROC));
	::SetWindowLongPtr(handle, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Win32::universalWindowProcedure));
	LONG_PTR userData = ::GetWindowLongPtr(handle, GWLP_USERDATA);
	::SetWindowLongPtr(handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
}

LRESULT CheckComboListBoxImpl::onMessage(UINT message, WPARAM wparam, LPARAM lparam)
{
	switch(message)
	case LB_GETCURSEL: {
		return -1;
	default:
		return Win32::Window32::onMessage(message, wparam, lparam);
	}
}

void CheckComboListBoxImpl::onMessageLButtonUp(UINT button, int x, int y)
{
	__super::onMessageLButtonUp(button, x, y);
}

CheckComboListBoxImpl::~CheckComboListBoxImpl()
{
	::SetWindowLongPtr(handle_, GWLP_USERDATA, 0);
}

int CheckComboListBoxImpl::onMessageDestroy()
{
	YASLI_ASSERT(::IsWindow(owner_->handle()));
	owner_->listBox_ = 0;
	return 0;
}

LRESULT CheckComboListBoxImpl::defaultWindowProcedure(UINT message, WPARAM wparam, LPARAM lparam)
{
	return CallWindowProcW(controlWindowProc_, handle_, message, wparam, lparam);
}

void CheckComboListBoxImpl::onMessageRButtonDown(UINT button, int x, int y)
{
	size_t count = owner_->owner_->items_.size();
	int selectedCount = 0;

	for(size_t i = 0; i < count; ++i) {
		if(owner_->getCheck(i))
			selectedCount++;
	}
	owner_->owner_->selectAll(count != selectedCount);
	SendMessageW(owner_->handle(), WM_COMMAND, MAKELONG(GetWindowLong(handle_, GWL_ID), CBN_SELCHANGE), LPARAM(handle_));
	InvalidateRect(handle_, 0, FALSE);
}

void CheckComboListBoxImpl::onMessageLButtonDown(UINT button, int x, int y)
{
	RECT clientRect;
	GetClientRect(handle_, &clientRect);

	POINT pt = { x, y };

	if(PtInRect(&clientRect, pt)) {
		INT nItemHeight = SendMessageW(handle_, LB_GETITEMHEIGHT, 0, 0);
		INT nTopIndex   = SendMessageW(handle_, LB_GETTOPINDEX, 0, 0);

		INT nIndex = nItemHeight == 0 ? -1 : nTopIndex + pt.y / nItemHeight;

		RECT rcItem;
		SendMessageW(handle_, LB_GETITEMRECT, nIndex, reinterpret_cast<LONG_PTR>(&rcItem));

		if(PtInRect(&rcItem, pt)) {
			owner_->setCheck(nIndex, !owner_->getCheck(nIndex));
			SendMessageW(owner_->handle(), WM_COMMAND, MAKELONG(GetWindowLong(handle_, GWL_ID), CBN_SELCHANGE), (LPARAM)handle_);
			owner_->updateValue();
			InvalidateRect(handle_, &rcItem, FALSE);
		}
	}
	__super::onMessageLButtonDown(button, x, y);
}

int CheckComboListBoxImpl::onMessageChar(UINT code, USHORT count, USHORT flags)
{
	if(code == VK_SPACE){
		int index = ::CallWindowProcW(controlWindowProc_, handle_, LB_GETCURSEL, WPARAM(code), MAKELPARAM(count, flags));
		RECT itemRect;
		SendMessageW(handle_, LB_GETITEMRECT, index, reinterpret_cast<LONG_PTR>(&itemRect));
		InvalidateRect(handle_, &itemRect, FALSE);

		owner_->setCheck(index, !owner_->getCheck(index));
		owner_->updateValue();
		SendMessageW(owner_->handle(), WM_COMMAND, MAKELONG(GetWindowLong(handle_, GWL_ID), CBN_SELCHANGE), LPARAM(handle_));
	}
	return 0;
}

// ---------------------------------------------------------------------------


CheckComboBoxImpl::CheckComboBoxImpl(ww::CheckComboBox* owner)
: _WidgetWindow(owner)
, owner_(owner)
, itemHeightSet_(false)
{
	bool created = false;
	WW_VERIFY(create(L"", WS_CHILD | WS_VSCROLL | WS_TABSTOP | CBS_DROPDOWNLIST | CBS_OWNERDRAWVARIABLE | CBS_HASSTRINGS, Rect(0, 0, 30, 30), Win32::getDefaultWindowHandle()));

	if(!created){
		DWORD error = GetLastError();
		unsigned int c = error;
	}

	controlWindowProc_ = reinterpret_cast<WNDPROC>(::GetWindowLongPtr(handle_, GWLP_WNDPROC));
	::SetWindowLongPtr(handle_, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Win32::universalWindowProcedure));
	::SetWindowLongPtr(handle_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
	SetWindowFont(handle_, Win32::defaultFont(), FALSE);
	//ComboBox_SetText(handle_, toWideChar(owner->value()).c_str());
}

void CheckComboBoxImpl::onEdited()
{
	updateValue();
	owner_->onEdited();
}

LRESULT CheckComboBoxImpl::defaultWindowProcedure(UINT message, WPARAM wparam, LPARAM lparam)
{
	return ::CallWindowProcW(controlWindowProc_, handle_, message, wparam, lparam);
}

void CheckComboBoxImpl::setCheck(size_t index, bool flag)
{
	WW_VERIFY(::SendMessageW(handle_, CB_SETITEMDATA, index, flag ? TRUE : FALSE) != -1);
	YASLI_ASSERT(::SendMessageW(handle_, CB_GETITEMDATA, index, 0) != -1);
	YASLI_ASSERT((flag ? TRUE : FALSE) == ::SendMessageW(handle_, CB_GETITEMDATA, index, 0));

	InvalidateRect(handle_, 0, FALSE);
}

void CheckComboBoxImpl::updateValue()
{
	string& value = owner_->value_;
	ww::CheckComboBox::Items& items = owner_->items_;
	ww::CheckComboBox::Items::iterator it;
	int index = 0;
	value = "";
	bool first = true;
	for(it = items.begin(); it != items.end(); ++it){
		if(getCheck(index)){
			if(!first)
				value += "|";
			first = false;
			value += *it;
		}
		++index;
	}
	SetWindowTextW(handle_, toWideChar(value.c_str()).c_str());
}

bool CheckComboBoxImpl::getCheck(size_t index) const
{
	return bool(::SendMessageW(handle_, CB_GETITEMDATA, index, 0) == FALSE ? false : true);
}



LRESULT CheckComboBoxImpl::onMessage(UINT message, WPARAM wparam, LPARAM lparam)
{
	SharedPtr<ww::Widget> ownerReference(owner_->refCount() == 0 ? 0 : owner_);
	switch(message){
	case WM_CTLCOLORLISTBOX:
		HWND wnd = HWND(lparam);
		if(!listBox_ && wnd != 0 && wnd != handle_ ){
			listBox_ = new CheckComboListBoxImpl(wnd, this);
		}
		return TRUE;
	}
	return Window32::onMessage(message, wparam, lparam);
}


int CheckComboBoxImpl::onMessageCommand(USHORT command, USHORT code, HWND wnd)
{
	SharedPtr<ww::Widget> ownerReference(owner_->refCount() == 0 ? 0 : owner_);
	switch(command){
		case CBN_SELCHANGE:
		{
			//owner_->selectedIndex_ = SendMessageW(comboBox_, CB_GETCURSEL, wparam, lparam);
			//owner_->onSelectionChanged();
			return 0;
		}
		case CBN_SELENDOK:
		{
			//owner_->selectedIndex_ = SendMessageW(comboBox_, CB_GETCURSEL, wparam, lparam);
			onEdited();
			return 0;
		}
		case CBN_CLOSEUP:
		{
			onEdited();
			return 0;
		}
		case CBN_SELENDCANCEL:
		{
			//PostMessageW(comboBox_, WM_KEYDOWN, VK_ESCAPE, 1);
			onEdited();
			return 0;
		}
		case CBN_KILLFOCUS:
		{
			// owner_->selectedIndex_ = SendMessageW(comboBox_, CB_GETCURSEL, 0, 0);
			onEdited();
			return 0;
		}
		case CBN_SETFOCUS:
		{
			return 0;
		}
		case CBN_DROPDOWN:
		{
			return 0;
		}
		default:
			break;
	}
	return Window32::onMessageCommand(command, code, wnd);
}

BOOL CheckComboBoxImpl::onMessageMeasureItem(UINT id, MEASUREITEMSTRUCT* measureItemStruct)
{
	HDC dc = GetDC(handle());
	HFONT oldFont = (HFONT)::SelectObject(dc, GetWindowFont(handle_));
	if(oldFont != 0){
		TEXTMETRIC metrics;
		GetTextMetrics(dc, &metrics);

		measureItemStruct->itemHeight = metrics.tmHeight + metrics.tmExternalLeading;

		measureItemStruct->itemHeight += 2;

		if(!itemHeightSet_){
			itemHeightSet_ = TRUE;
			//SetItemHeight(-1, measureItemStruct->itemHeight);
			SendMessageW(handle_, LB_SETITEMHEIGHT, -1, measureItemStruct->itemHeight);
		}
		SelectObject(dc, oldFont);
	}
	ReleaseDC(handle_, dc);
	return TRUE;
}

BOOL CheckComboBoxImpl::onMessageDrawItem(UINT id, DRAWITEMSTRUCT* drawItemStruct)
{
	HDC dc = drawItemStruct->hDC;

	RECT rect = drawItemStruct->rcItem;
	RECT text   = drawItemStruct->rcItem;

	// 0 - No check, 1 - Empty check, 2 - Checked
	INT nCheck = 0;

	wstring str ;

	if((LONG)(drawItemStruct->itemID) < 0){
		updateValue();
		str = toWideChar(owner_->value_.c_str());
		nCheck = 0;
	}
	else{
		int len = CallWindowProcW(controlWindowProc_, handle_, CB_GETLBTEXTLEN, (WPARAM)drawItemStruct->itemID, 0);
		if(len > 0){
			wchar_t* buffer = new wchar_t[len + 1];
			memset((void*)buffer, 0, sizeof(wchar_t) * len + 1);
			YASLI_ASSERT(buffer);
			WW_VERIFY(CallWindowProcW(controlWindowProc_, handle_, CB_GETLBTEXT, (WPARAM)drawItemStruct->itemID, (LPARAM)buffer) != CB_ERR);
			str = buffer;
			delete[] buffer;
		}

		nCheck = 1 + (CallWindowProcW(controlWindowProc_, handle_, CB_GETITEMDATA, drawItemStruct->itemID, 0) != 0);

		TEXTMETRIC metrics;
		GetTextMetrics(dc, &metrics);

		rect.left    = 0;
		rect.right   = rect.left + metrics.tmHeight + metrics.tmExternalLeading + 6;
		rect.top    += 1;
		rect.bottom -= 1;

		text.left = rect.right;
	}
	
	if (nCheck > 0) {
		SetBkColor(dc, GetSysColor(COLOR_WINDOW));
		SetTextColor(dc, GetSysColor(COLOR_WINDOWTEXT));

		UINT nState = DFCS_BUTTONCHECK;

		if (nCheck > 1)
			nState |= DFCS_CHECKED;

		// Draw the checkmark using DrawFrameControl
		DrawFrameControl(dc, &rect, DFC_BUTTON, nState);
	}

	if(drawItemStruct->itemState & ODS_SELECTED) {
		SetBkColor(dc, GetSysColor(COLOR_HIGHLIGHT));
		SetTextColor(dc, GetSysColor(COLOR_HIGHLIGHTTEXT));
	}
	else{
		SetBkColor(dc, GetSysColor(COLOR_WINDOW));
		SetTextColor(dc, GetSysColor(COLOR_WINDOWTEXT));
	}

	// Erase and draw
	ExtTextOutW(dc, 0, 0, ETO_OPAQUE, &text, 0, 0, 0);
	DrawTextW(dc, str.c_str(), int(str.length()), &text, DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS);

	if((drawItemStruct->itemState & (ODS_FOCUS|ODS_SELECTED)) == (ODS_FOCUS|ODS_SELECTED))
		DrawFocusRect(dc, &text);

	return TRUE;
}

BOOL CheckComboBoxImpl::onMessageEnable(BOOL enabled)
{
	return FALSE;
}

int CheckComboBoxImpl::onMessageSetFocus(HWND lastFocusedWindow)
{
	return Window32::onMessageSetFocus(lastFocusedWindow);
}

int CheckComboBoxImpl::onMessageKillFocus(HWND focusedWindow)
{
	return Window32::onMessageKillFocus(focusedWindow);
}

void CheckComboBoxImpl::showDropDown()
{
	ComboBox_ShowDropdown(handle_, TRUE);
}

// --------------------------------------------------- 


#pragma warning(push)
#pragma warning(disable: 4355) //  'this' : used in base member initializer list
CheckComboBox::CheckComboBox(bool expandByContent, int border)
: expandByContent_(expandByContent)
, dropDownHeight_(10)
, _WidgetWithWindow(new CheckComboBoxImpl(this), border)
{
	updateMinimalSize();
}
#pragma warning(pop)

void CheckComboBox::updateMinimalSize()
{
	Vect2 size = Win32::calculateTextSize(window()->comboBox(), GetWindowFont(window()->handle()), L" ");
	if(expandByContent_){
		Items::iterator it;
		FOR_EACH(items_, it){
			Vect2 anotherSize = Win32::calculateTextSize(window()->handle(), GetWindowFont(window()->handle()), toWideChar(it->c_str()).c_str());
			size.x = std::max(anotherSize.x, size.x);
			size.y = std::max(anotherSize.y, size.y);
		}
	}
	_setMinimalSize(size + Vect2(4, 4) + Vect2(GetSystemMetrics(SM_CXFOCUSBORDER), GetSystemMetrics(SM_CYFOCUSBORDER)) * 2
		            + Vect2(border_, border_) * 2 + Vect2(GetSystemMetrics(SM_CXBORDER), GetSystemMetrics(SM_CYBORDER)));
}


void CheckComboBox::_setPosition(const Rect& position)
{
	Widget::_setPosition(position);

	Win32::Window32* window = _findWindow(parent());
	YASLI_ASSERT(window);
	Win32::Window32::PositionDeferer deferer = window->positionDeferer();
	Rect pos(position.left() + border_, position.top() + border_,
		      position.right() - border_ * 2, position.bottom() - border_ * 2 + _minimalSize().y * dropDownHeight_);
	deferer->defer(_window(), pos);
}

void CheckComboBox::setExpandByContent(bool expand)
{
	expandByContent_ = expand;
	updateMinimalSize();
	_queueRelayout();
}

void CheckComboBox::showDropDown()
{
	window()->showDropDown();
}

void CheckComboBox::set(const StringList& stringList, const char *value)
{
	clear();
	value_ = "";
	StringList vals;
	splitStringList(&vals, value, '|');

	StringList::const_iterator it;
	int index = 0;
	FOR_EACH(stringList, it){
		StringList::const_iterator vit;
		add(it->c_str());
		FOR_EACH(vals, vit){
			if(*vit == *it){
				if(vit != vals.begin())
					value_ += "|";
				value_ += *it;
				window()->setCheck(index, true);
				break;
			}
		}
		++index;
	}
}

void CheckComboBox::clear()
{
	items_.clear();
	::SendMessageW(window()->comboBox(), CB_RESETCONTENT, 0, 0);
	updateMinimalSize();
	_queueRelayout();
}


void CheckComboBox::add(const char* text)
{
	insert(end(), text);
}

void CheckComboBox::insert(iterator before, const char* text)
{
	int index = before == items_.end() ? -1 : std::distance(items_.begin(), before);
	items_.insert(before, text);
	::SendMessageW(window()->comboBox(), CB_INSERTSTRING, index, (LPARAM)toWideChar(text).c_str());
	if(items_.size() == 1)
		::SendMessageW(window()->comboBox(), CB_SETCURSEL, 0, 0);
	updateMinimalSize();
	_queueRelayout();
}

void CheckComboBox::setDropDownHeight(int lines)
{
	dropDownHeight_ = lines;
	_setPosition(_position());
}

void CheckComboBox::selectAll(bool select)
{
	for(int i = 0; i < int(items_.size()); ++i)
		window()->setCheck(i, select);
}

int CheckComboBox::dropDownHeight() const
{
	return dropDownHeight_;
}

void CheckComboBox::serialize(Archive& ar)
{
	if(ar.filter(SERIALIZE_DESIGN)){
		ar(items_, "items", "Элементы");
		ar(value_, "value", "&Выбранные элементы");
		ar(dropDownHeight_, "dropDownHeight_", "Высота списка");
		ar(expandByContent_, "expandByContent", "Расширять по содержимому");
	}
	Widget::serialize(ar);
}

}
