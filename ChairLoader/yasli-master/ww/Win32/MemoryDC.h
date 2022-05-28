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

namespace Win32{

class MemoryDC{
public:
	MemoryDC(HDC dc);
	~MemoryDC();

	operator HDC() const{ return dc_; }
protected:
	RECT rect_;
	HDC destinationDC_;
	HDC dc_;
	HBITMAP bitmap_;
	HBITMAP oldBitmap_;
};

}

