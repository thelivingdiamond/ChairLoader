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
#include "ww/Color.h"

namespace ww{

class ColorRectImpl;
class ColorRect : public _WidgetWithWindow{
public:
	ColorRect(Color color = Color(255, 255, 255), int border = 0);
	~ColorRect();

	void set(const Color& color);
	const Color& get() const{ return color_; }

	signal0& signalActivate(){ return signalActivate_; }
protected:
	ColorRectImpl& impl();
	signal0 signalActivate_;
	Color color_;
	friend class ColorRectImpl;
};

}

