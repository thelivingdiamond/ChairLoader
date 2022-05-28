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

namespace ww{
class SliderImpl;

class Slider : public _WidgetWithWindow{
public:
	Slider(int border = 0);

	void setValue(float value);
	float value() const{ return value_; }
	signal0& signalChanged(){ return signalChanged_; }
	void setStepsCount(int stepsCount);
protected:
	signal0 signalChanged_;
	float value_;
	int stepsCount_;

	SliderImpl& impl();
	friend class SliderImpl;
};
}

