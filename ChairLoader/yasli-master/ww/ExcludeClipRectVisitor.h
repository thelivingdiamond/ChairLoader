/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "ww/Widget.h"
#include "ww/Win32/Window32.h"
#include "ww/Win32/Rectangle.h"

struct ExcludeClipRectVisitor : public ww::WidgetVisitor{
	HDC dc_;
	ExcludeClipRectVisitor(HDC dc)
	: dc_(dc)
	{
	}

	void operator()(ww::Widget& widget){
		if(Win32::Window32* window = widget._window()){
			if(widget._visible() && widget._visibleInLayout() && ::IsWindowVisible(window->handle())){
				Win32::Rect rect;
				::GetClientRect(window->handle(), &rect);
				window->clientToScreen(rect);
				window->parent()->screenToClient(rect);
				::ExcludeClipRect(dc_, rect.left, rect.top, rect.right, rect.bottom);
			}
		}
		else
			widget.visitChildren(*this);
	}
};

