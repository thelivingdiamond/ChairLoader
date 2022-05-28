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
#include "yasli/Serializer.h"
#include "Win32/Types.h"

namespace ww{
class Widget;

enum EditFlags{
	ONLY_TRANSLATED   = 1 << 0,
	IMMEDIATE_UPDATE  = 1 << 1,
	COMPACT		      = 1 << 2,
	EXPAND_ALL		  = 1 << 3
};

bool edit(const Serializer& ser, const char* stateFileName,
		  int flags = IMMEDIATE_UPDATE | ONLY_TRANSLATED,
		  Widget* parent = 0, const char* title = 0);
bool edit(const Serializer& ser, const char* stateFileName,
		  int flags, HWND parent, const char* title = 0);

}

