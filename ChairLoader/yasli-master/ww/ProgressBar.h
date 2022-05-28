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
#include "ww/Strings.h"


namespace ww{
	class ProgressBarImpl;
	class ProgressBar : public _WidgetWithWindow{
	public:
		ProgressBar(int border = 0);
		
		void setPosition(float pos);
		/// [0..1]
		float position() const { return pos_; }
		
		void serialize(Archive& ar);
	protected:
		// внутренние функции
		ProgressBarImpl* window() const{ return reinterpret_cast<ProgressBarImpl*>(_window()); }

		float pos_;
	};

}

