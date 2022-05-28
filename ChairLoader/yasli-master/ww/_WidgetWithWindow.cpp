/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "ww/_WidgetWithWindow.h"
#include "ww/_WidgetWindow.h"
#include "ww/Container.h"
#include "ww/Win32/Window32.h"

namespace ww{

_WidgetWithWindow::_WidgetWithWindow(_WidgetWindow* window, int border)
: window_(0)
{
	setWindow(window);
	setBorder(border);
}

_WidgetWithWindow::~_WidgetWithWindow()
{
	Win32::Window32* window = window_;
	window_ = 0;
	delete window;
}


void _WidgetWithWindow::setWindow(Win32::Window32* window)
{
	window_ = window;
}

void _WidgetWithWindow::_setPosition(const ww::Rect& position)
{
	Widget::_setPosition(position);

	Win32::Window32* window = _findWindow(parent());
	YASLI_ASSERT(window);
	Win32::Window32::PositionDeferer deferer = window->positionDeferer();
	Rect pos(position.left() + border_, position.top() + border_,
		      position.right() - border_, position.bottom() - border_);
	deferer->defer(window_, pos);
}

void _WidgetWithWindow::_setParent(Container* container)
{
	Win32::Window32* window = _findWindow(container);
	if (!window)
		return;
	
	Widget::_setParent(container);
	YASLI_ASSERT(::IsWindow(window_->handle()));
	if(::IsWindow(window_->handle())){
		window_->setParent(window);
		// moves window to the end of tab-focus order
		::SetWindowPos(window_->handle(), HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		_setPosition(position_);
	}
}

bool _WidgetWithWindow::isVisible() const
{
	return window_->isVisible();
}

void _WidgetWithWindow::setSensitive(bool sensitive)
{
	Widget::setSensitive(sensitive);
	window_->enable(sensitive);
}

void _WidgetWithWindow::_updateVisibility()
{
	Widget::_updateVisibility();
	if(_visible() && _visibleInLayout())
		window_->show(SW_SHOWNOACTIVATE);
	else
		window_->show(SW_HIDE);
}

void _WidgetWithWindow::_setFocus()
{
	__super::_setFocus();
}

// ---------------------------------------------------------------------------
// FIXME somehow: Copy & Paste!

_ContainerWithWindow::_ContainerWithWindow(_ContainerWindow* window, int border)
: window_(window)
{
	setBorder(border);
}

_ContainerWithWindow::~_ContainerWithWindow()
{
	Win32::Window32* window = window_;
	window_ = 0;
	delete window;
}

void _ContainerWithWindow::_setPosition(const Rect& position)
{
	Container::_setPosition(position);
	window_->move(Rect(position.left() + border(), position.top() + border(), position.right() - border(), position.bottom() - border()));
	_arrangeChildren();
}

void _ContainerWithWindow::_setParent(Container* container)
{
	Win32::Window32* window = _findWindow(container);
	if (!window)
		return;
	
	Widget::_setParent(container);
	YASLI_ASSERT(window_);
	YASLI_ASSERT(::IsWindow(window_->handle()));
	if(::IsWindow(window_->handle())){
		window_->setParent(window);
		// moves window to the end of tab-focus order
		::SetWindowPos(window_->handle(), HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		_setPosition(position_);
	}
}

bool _ContainerWithWindow::isVisible() const
{
	return window_->isVisible();
}

void _ContainerWithWindow::setSensitive(bool sensitive)
{
	Widget::setSensitive(sensitive);
	window_->enable(sensitive);
}

void _ContainerWithWindow::_updateVisibility()
{
	Widget::_updateVisibility();
	if(_visible() && _visibleInLayout())
		window_->show(SW_SHOWNOACTIVATE);
	else
		window_->show(SW_HIDE);
}

void _ContainerWithWindow::_setFocus()
{
	__super::_setFocus();
}

}
