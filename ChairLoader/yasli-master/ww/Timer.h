/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "ww/API.h"
#include "ww/sigslot.h"
#include "yasli/Pointers.h"

namespace ww{

class Widget;
class TimerImpl;
class Timer : public RefCounter{
public:
	Timer(Widget* owner, int intervalMs);
	virtual ~Timer();
	signal0& signalTimer(){ return signalTimer_; }
protected:
	
	signal0 signalTimer_;
	TimerImpl* impl_;
    Widget* owner_;

private:
	Timer(Widget* owner, float interval = 0.1f) {} // this constructor is obsolete
};

}

