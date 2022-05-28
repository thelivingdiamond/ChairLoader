/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "ww/_WidgetWithWindow.h"
#include "ww/HotkeyContext.h"
#include "ww/Win32/Types.h"

namespace ww{

class Win32ProxyImpl;
class Win32Proxy : public _ContainerWithWindow{
public:
	Win32Proxy(HWND parent, int border = 0);

	void add(Widget* widget);
	void visitChildren(WidgetVisitor& visitor) const;
	void serialize(yasli::Archive& ar);

	signal0& signalPressed(KeyPress key) { return hotkeyContext_->signalPressed(key); }
	signal2<KeyPress, bool&>& signalPressedAny(){ return hotkeyContext_->signalPressedAny(); }

	HWND hwnd();
protected:
	void _arrangeChildren();
	HotkeyContext* _hotkeyContext(){ return hotkeyContext_; }
	void _setFocusedWidget(Widget* widget) { focusedWidget_ = widget; }
	Widget* _focusedWidget() const{ return focusedWidget_; }

	Win32ProxyImpl& impl();

	yasli::SharedPtr<Widget> child_;
	HWND parentHwnd_;
	yasli::SharedPtr<HotkeyContext> hotkeyContext_;
	Widget* focusedWidget_;

	friend Win32ProxyImpl;
};

}

