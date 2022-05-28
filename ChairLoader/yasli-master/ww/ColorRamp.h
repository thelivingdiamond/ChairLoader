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

class ColorRampImpl;
class ColorRamp : public _WidgetWithWindow{
public:
	ColorRamp(int border = 0);

	signal0& signalChanged(){ return signalChanged_; }

	void set(const Color& color);
	const Color& get() const{ return color_; }
protected:
	ColorRampImpl& impl();
	signal0 signalChanged_;

	Color color_;
	friend class ColorRampImpl;
};

}

