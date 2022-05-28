/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "ww/Label.h"
#include "ww/Container.h"

#include "ww/_WidgetWindow.h"
#include "ww/Unicode.h"
#include <windowsx.h>

#include "ww/Serialization.h"
#include "yasli/ClassFactory.h"

namespace ww{

YASLI_CLASS(Widget, Label, "Label")
class LabelImpl: public _WidgetWindow{
public:
	LabelImpl(Label* owner, bool emphasis);
	~LabelImpl();

	int onMessageDestroy();
	LRESULT onMessage(UINT message, WPARAM wparam, LPARAM lparam);
	void recreateFont(bool emphasis);
	HFONT font() { return owner_->emphasis_ ? Win32::defaultBoldFont() : Win32::defaultFont(); }

	void setText(const wchar_t* text);
protected:
	ww::Label* owner_;
};

LabelImpl::LabelImpl(Label* owner, bool emphasis)
: _WidgetWindow(owner)
, owner_(owner)
{
	WW_VERIFY(create(L"", WS_CHILD , Rect(0, 0, 42, 42), Win32::getDefaultWindowHandle()));
}

LabelImpl::~LabelImpl()
{

}

void LabelImpl::setText(const wchar_t* text)
{
	Vect2 textSize = Win32::calculateTextSize(handle(), font(), text);
	if(owner_->expandByContent_)
		owner_->_setMinimalSize(textSize + Vect2(2 + owner_->border() * 2, 2 + owner_->border() * 2));
	else
		owner_->_setMinimalSize(2 + owner_->border() * 2, 
								textSize.y + 2 + owner_->border() * 2);
	owner_->_queueRelayout();
}

int LabelImpl::onMessageDestroy()
{
	return __super::onMessageDestroy();
}

LRESULT LabelImpl::onMessage(UINT message, WPARAM wparam, LPARAM lparam)
{
	switch(message){
	case WM_ERASEBKGND:
		{
			HDC dc = (HDC)(wparam);
			return FALSE;
		}
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC dc = ::BeginPaint(handle(), &ps); 
			YASLI_ASSERT(dc != 0);
			wstring text = toWideChar(owner_->text_.c_str());
			
			UINT flags = DT_SINGLELINE;
			if(!owner_->expandByContent_)
				flags |= DT_PATH_ELLIPSIS;
			
			switch(owner_->alignHorizontal_){
			case ww::ALIGN_LEFT:
				flags |= DT_LEFT;
				break;
			case ww::ALIGN_CENTER:
				flags |= DT_CENTER;
				break;
			case ww::ALIGN_RIGHT:
				flags |= DT_RIGHT;
				break;
			};
			
			switch(owner_->alignVertical_){
			case ww::ALIGN_TOP:
				flags |= DT_TOP;
				break;
			case ww::ALIGN_MIDDLE:
				flags |= DT_VCENTER;
				break;
			case ww::ALIGN_BOTTOM:
				flags |= DT_BOTTOM;
				break;
			};

			RECT rect;
			WW_VERIFY(GetClientRect(handle(), &rect));
			HFONT oldFont = HFONT(::SelectObject(dc, (HGDIOBJ)font()));
			HBRUSH oldBrush = HBRUSH(::SelectObject(dc, (HGDIOBJ)(::GetStockObject(GRAY_BRUSH))));
			int oldBackMode = ::SetBkMode(dc, OPAQUE);
			COLORREF oldBkColor = ::SetBkColor(dc, ::GetSysColor(COLOR_BTNFACE));
			::FillRect(dc, &rect, ::GetSysColorBrush(COLOR_BTNFACE));
			DrawTextW(dc, text.c_str(), int(wcslen(text.c_str())), &rect, flags);
			::SelectObject(dc, oldFont);
			::SelectObject(dc, oldBrush);
			::SetBkMode(dc, oldBackMode);
			::SetBkColor(dc, oldBkColor);

			::EndPaint(handle(), &ps);
			return TRUE;
		}
	}
	return __super::onMessage(message, wparam, lparam);
}


#pragma warning(push)
#pragma warning(disable: 4355) // 'this' : used in base member initializer list

Label::Label(const char* text, bool emphasis, int border)
: _WidgetWithWindow(new LabelImpl(this, emphasis), border)
, mnemonicWidget_(0)
, alignHorizontal_(ALIGN_LEFT)
, alignVertical_(ALIGN_MIDDLE)
, emphasis_(emphasis)
, expandByContent_(true)
{
    _setMinimalSize(42, 42);
	setText(text);
}
#pragma warning(pop)

void Label::setText(const char* text)
{
	impl()->setText(toWideChar(text).c_str());
	text_ = text;
}

void Label::setAlignment(ww::TextAlignHorizontal alignh, ww::TextAlignVertical alignv)
{
	alignHorizontal_ = alignh;
	alignVertical_ =  alignv;

	::RedrawWindow(impl()->handle(), 0, 0, RDW_INVALIDATE);
}

void Label::setExpandByContent(bool expand)
{
	expandByContent_ = expand;
	setText(text_.c_str());
}

void Label::serialize(Archive& ar)
{
	if(ar.filter(SERIALIZE_DESIGN)){
		ar(text_, "text", "&Текст");
		ar(emphasis_, "emphasis", "Выделить");
		ar(expandByContent_, "expandByContent", "Растягивать под содержимое");
	}
	Widget::serialize(ar);
}

/*
void Label::setMnemonicWidget(Widget* mnemonicWidget)
{

}
*/

};
