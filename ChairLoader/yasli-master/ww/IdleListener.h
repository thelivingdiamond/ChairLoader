/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

namespace ww{

class IdleListenerImpl;
class IdleListener{
public:
	IdleListener();
	virtual void onIdle() = 0;
protected:
	IdleListenerImpl* impl_;
};

}

