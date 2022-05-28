/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include <windows.h>
#include "ww/Rect.h"
#include "ww/Vect2.h"

namespace Win32{

using ww::Vect2;

struct Rect : ::RECT{
	Rect()
	{}
	Rect(POINT leftTop, POINT rightBottom)
	{
		left = leftTop.x;
		top = leftTop.y;
		right = rightBottom.x;
		bottom = rightBottom.y;
	}
	Rect(Vect2 leftTop, Vect2 rightBottom)
	{
		left = leftTop.x;
		top = leftTop.y;
		right = rightBottom.x;
		bottom = rightBottom.y;
	}
	Rect(int left, int top, int right, int bottom)
	{
		this->left = left;
		this->top = top;
		this->right = right;
		this->bottom = bottom;
	}
	explicit Rect(const ww::Rect& rect)
	{
		left = rect.left();
		top = rect.top();
		right = rect.right();
		bottom = rect.bottom();
	}
	Rect(const RECT& rect)
	{
		left = rect.left;
		top = rect.top;
		right = rect.right;
		bottom = rect.bottom;
	}

	int width() const{ return right - left; }
	int height() const{ return bottom - top; }

	bool pointIn(Vect2 point) const{
		return point.x >= left && point.x < right &&
			   point.y >= top && point.y < bottom;
	}
	ww::Rect recti(){
		return ww::Rect(left, top, right, bottom);
	}
};


}

