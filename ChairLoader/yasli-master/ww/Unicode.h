/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once
#include "ww/Strings.h"
#include <stdio.h>

namespace ww{

	string fromWideChar(const wchar_t* wideCharString);
	wstring toWideChar(const char* multiByteString);
	wstring fromANSIToWide(const char* ansiString);
    string toANSIFromWide(const wchar_t* wstr);

    FILE* fopen(const char* utf8filename, const char* mode);
}
