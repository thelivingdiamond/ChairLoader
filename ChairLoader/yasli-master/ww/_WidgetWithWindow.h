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
#include "ww/Container.h"

namespace ww{

class _WidgetWindow;
class _ContainerWindow;


class _WidgetWithWindow : public Widget{
public:
	void setSensitive(bool sensitive);
	bool isVisible() const;

	_WidgetWithWindow(_WidgetWindow* window, int border);
	~_WidgetWithWindow();

	void setWindow(Win32::Window32* window);

	void _setPosition(const Rect& position);
	void _setParent(Container* container);
	void _setFocus();
	void _updateVisibility();

	Win32::Window32* _window() const{ return window_; }
private:
	Win32::Window32* window_;
};


class _ContainerWithWindow : public Container{
public:
	void setSensitive(bool sensitive);
	bool isVisible() const;

	_ContainerWithWindow(_ContainerWindow* window, int border);
	~_ContainerWithWindow();

	void _setPosition(const Rect& position);
	void _setParent(Container* container);
	void _setFocus();
	void _updateVisibility();

	Win32::Window32* _window() const{ return window_; }
private:
	Win32::Window32* window_;
};

}

