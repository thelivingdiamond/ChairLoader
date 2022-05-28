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

class HLineImpl;

namespace ww{

	class HLine : public _WidgetWithWindow{
	public:
		HLine(int border = 0);
	protected:
		// внутренние функции
		HLineImpl* window() const{ return reinterpret_cast<HLineImpl*>(_window()); }
	};

}

