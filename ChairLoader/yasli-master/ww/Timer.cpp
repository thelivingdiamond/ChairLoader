/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "Timer.h"
#include "ww/Win32/Window32.h"
#include "ww/Widget.h"

namespace ww{

class TimerImpl : public Win32::TimerInterface{
public:
	TimerImpl(Timer* owner)
	: owner_(owner)
	{}
	void onTimer(){
		owner_->signalTimer().emit();
	}
protected:
	Timer* owner_;
};

Timer::Timer(Widget* owner, int intervalMS)
{
	owner_ = owner;
	impl_ = new TimerImpl(this);
	owner->_window()->attachTimer(impl_, intervalMS);
}

Timer::~Timer()
{
	owner_->_window()->detachTimer(impl_);
	impl_ = 0;
}

}
