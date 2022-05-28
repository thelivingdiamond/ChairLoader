/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "ww/Entry.h"
#include "ww/Container.h"
#include "ww/_WidgetWindow.h"
#include "ww/Serialization.h"
#include "ww/Unicode.h"
#include "yasli/ClassFactory.h"
#include "ww/KeyPress.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

namespace ww{
YASLI_CLASS(Widget, Entry, "Entry");

#pragma warning(push)
#pragma warning(disable: 4355) // 'this' : used in base member initializer list

class EntryImpl : public _WidgetWindow{
public:
	EntryImpl(ww::Entry* owner);
	~EntryImpl();
	const wchar_t* className() const{ return L"EDIT"; }
	void setText(const wchar_t* text);
    void updateOwnerText();
	void updateStyle();

	int onMessageChar(UINT code, USHORT count, USHORT flags);
	int onMessageKeyDown(UINT keyCode, USHORT count, USHORT flags);
	int onMessageGetDlgCode(int keyCode, MSG* msg);
	
	BOOL onMessageEraseBkgnd(HDC dc);
	int onMessageCommand(USHORT command, USHORT id, HWND wnd);
	LRESULT onMessage(UINT message, WPARAM wparam, LPARAM lparam);
	LRESULT defaultWindowProcedure(UINT message, WPARAM wparam, LPARAM lparam);

	void commit();
	HWND edit() { return handle(); }
protected:
	unsigned int generateStyle() const;
	unsigned int generateStyleEx() const;

	ww::Entry* owner_;
	bool setting_;
	wstring text_;
	static WNDPROC controlWindowProc_;
};

WNDPROC EntryImpl::controlWindowProc_ = 0;

#pragma warning(disable: 4312) // 'type cast' : conversion from 'LONG' to 'HINSTANCE' of greater size
EntryImpl::EntryImpl(ww::Entry* owner)
: _WidgetWindow(owner)
, owner_(owner)
, setting_(false)
{
	WW_VERIFY(create(L"", generateStyle(), Rect(0, 0, 800, 60), Win32::getDefaultWindowHandle(), generateStyleEx()));

	controlWindowProc_ = reinterpret_cast<WNDPROC>(::GetWindowLongPtr(handle_, GWLP_WNDPROC));
	::SetWindowLongPtr(handle_, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Win32::universalWindowProcedure));
	LONG_PTR userData = ::GetWindowLongPtr(handle_, GWLP_USERDATA);
	::SetWindowLongPtr(handle_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	HFONT font = Win32::defaultFont();
	SetWindowFont(handle(), font, FALSE);

	Vect2 textSize = Win32::calculateTextSize(handle(), font, L" ");
	owner_->_setMinimalSize(textSize + Vect2(GetSystemMetrics(SM_CXBORDER) * 2 + 6 + 1, GetSystemMetrics(SM_CYBORDER) * 2 + 6 + 1));
}

EntryImpl::~EntryImpl()
{

}

unsigned int EntryImpl::generateStyle() const
{
	return WS_CHILD | WS_TABSTOP | (owner_->flat_ ? WS_BORDER : 0) | ES_LEFT | ES_AUTOHSCROLL | ES_AUTOVSCROLL 
			| (owner_->multiline_ ? ES_MULTILINE : 0);
}

unsigned int EntryImpl::generateStyleEx() const
{
	return owner_->flat_ ? 0 : WS_EX_CLIENTEDGE;
}

void EntryImpl::updateStyle()
{
	if(!handle_)
		return;
	
	SetWindowLong(handle_, GWL_STYLE, generateStyle());
	SetWindowLong(handle_, GWL_EXSTYLE, generateStyleEx());
	RedrawWindow(handle_, 0, 0, RDW_INVALIDATE);
}

void EntryImpl::updateOwnerText()
{
    int length = GetWindowTextLength(handle()) + 1;
		wstring text;
    if(length > 0)
    {
        std::vector<wchar_t> buf;
        buf.resize(length);
        GetWindowTextW(handle(), &buf[0], length);

        YASLI_ASSERT(owner_);
        text = &buf[0];
    }
	bool changed = owner_->textW_ != text;
	owner_->text_ = fromWideChar(text.c_str());
	owner_->textW_ = text;
	if(changed)
		owner_->onChanged();
}

void EntryImpl::commit()
{
    updateOwnerText();
	owner_->onEdited();
}

void EntryImpl::setText(const wchar_t* text)
{
	setting_ = true;
	WW_VERIFY(::SetWindowTextW(handle(), text));
	setting_ = false;
	text_ = text;
}

LRESULT EntryImpl::defaultWindowProcedure(UINT message, WPARAM wparam, LPARAM lparam)
{
	return ::CallWindowProc(controlWindowProc_, handle_, message, wparam, lparam);
}

int EntryImpl::onMessageGetDlgCode(int keyCode, MSG* msg)
{
	if (!msg)
		return DLGC_WANTMESSAGE;

	int key = msg->wParam;
	if ((msg->message == WM_KEYDOWN || msg->message == WM_CHAR))
	{
		if (!owner_->multiline_){
			if ((key == VK_UP || key == VK_DOWN) && !owner_->swallowArrows_)
				return 0;
			if ((key == VK_LEFT || key == VK_RIGHT) && owner_->swallowArrows_)
				return DLGC_WANTMESSAGE;
			if (key == VK_ESCAPE && owner_->swallowEscape_)
				return DLGC_WANTMESSAGE;
			if (key == VK_RETURN && !owner_->swallowReturn_)
				return 0;
		}
		if (key == VK_TAB)
			return 0;
	}
	return DLGC_WANTMESSAGE;
}

int EntryImpl::onMessageChar(UINT code, USHORT count, USHORT flags)
{
	if(owner()->hasFocus() && (code == VK_RETURN || code == VK_ESCAPE || code == VK_TAB)){
		if(code == VK_ESCAPE){
			wstring text = text_;
			setText(text.c_str());
		}
		if(owner()->parent())
			owner()->parent()->setFocus();
		if(code != VK_TAB)
			return 0;
	}
	return __super::onMessageChar(code, count, flags);
}

int EntryImpl::onMessageKeyDown(UINT keyCode, USHORT count, USHORT flags)
{
	ww::KeyPress key = ww::KeyPress::addModifiers(ww::Key(keyCode));
    if (owner_->onKeyPress(key))
        return 0;
	if(keyCode == KEY_LEFT || keyCode == KEY_RIGHT)
		return Win32::Window32::onMessageKeyDown(keyCode, count, flags);
	return __super::onMessageKeyDown(keyCode, count, flags);
}

LRESULT EntryImpl::onMessage(UINT message, WPARAM wparam, LPARAM lparam)
{
	SharedPtr<ww::Widget> ref((owner_->refCount() == 0) ? 0 : owner_);

	return __super::onMessage(message, wparam, lparam);
}

int EntryImpl::onMessageCommand(USHORT command, USHORT code, HWND wnd)
{
	switch(command){
	case EN_CHANGE:
		if(!setting_){
            updateOwnerText();
		}
		return TRUE;
	case EN_SETFOCUS:
        {
		int result = __super::onMessageCommand(command, code, wnd);
        owner_->_setFocus();
		//Edit_SetSel(handle(), 0, -1);
        return result;
        }
	case EN_KILLFOCUS:
		if(GetFocus() != handle())
			commit();
		return TRUE;
	}
	return __super::onMessageCommand(command, code, wnd);
}

BOOL EntryImpl::onMessageEraseBkgnd(HDC dc)
{
	return __super::onMessageEraseBkgnd(dc);
	//return FALSE;
}

// ---------------------------------------------------------------------------


Entry::Entry(const char* text, bool multiline, int border)
: _WidgetWithWindow(0, border)
, swallowReturn_(false)
, swallowEscape_(false)
, swallowArrows_(false)
, flat_(false)
, multiline_(false)
{
	setWindow(new EntryImpl(this)); 
	setBorder(border);
	setMultiline(multiline);
	setText(text);
}

Entry::Entry(const wchar_t* text, bool multiline, int border)
: _WidgetWithWindow(0, border)
, swallowReturn_(false)
, swallowEscape_(false)
, swallowArrows_(false)
, multiline_(false)
, flat_(false)
{
	setWindow(new EntryImpl(this)); 
	setBorder(border);
	setMultiline(multiline);
	setText(text);
}

Entry::~Entry()
{
}

EntryImpl* Entry::impl() const
{
	return static_cast<EntryImpl*>(_window());
}

void Entry::setSelection(EntrySelection selection)
{
	Edit_SetSel(impl()->edit(), selection.start(), selection.end());
}

EntrySelection Entry::selection() const
{
	DWORD range = SendMessageW(impl()->handle(), EM_GETSEL, 0, 0);
	WORD start = LOWORD(range);
	WORD end = HIWORD(range);
	return EntrySelection(start, end);
}

void Entry::replace(EntrySelection selection, const char* text)
{
	setSelection(selection);
	WW_VERIFY(SendMessageW(impl()->handle(), EM_REPLACESEL, 0, (LPARAM)text));
}

void Entry::setText(const char* text)
{
	text_ = text;
    textW_ = toWideChar(text);
	impl()->setText(textW_.c_str());
}

void Entry::setText(const wchar_t* text)
{
	text_ = fromWideChar(text);
	textW_ = text;
	impl()->setText(textW_.c_str());
}

void Entry::setFlat(bool _flat)
{
	flat_ = _flat;
	impl()->updateStyle();
}

void Entry::setMultiline(bool multiline)
{
	multiline_ = multiline;
	impl()->updateStyle();
}


void Entry::serialize(Archive& ar)
{
	if(ar.filter(SERIALIZE_DESIGN)){
		ar(textW_, "text", "&Текст");
        if(ar.isInput())
            text_ = fromWideChar(textW_.c_str());
	}
	Widget::serialize(ar);
}

void Entry::commit()
{
	impl()->commit();
}

bool Entry::onKeyPress(const KeyPress& key)
{
	if((key.key == KEY_RETURN && swallowReturn_) || key.key == KEY_ESCAPE/* || KEY_TAB*/)
		return true;
    return false;
}

};

#pragma warning(pop)
