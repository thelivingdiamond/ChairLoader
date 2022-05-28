/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "Unicode.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace ww{

string fromWideChar(const wchar_t* wstr)
{
#ifdef WW_DISABLE_UTF8
	const unsigned int codepage = CP_ACP;
#else
	const unsigned int codepage = CP_UTF8;
#endif
	int len = WideCharToMultiByte(codepage, 0, wstr, -1, NULL, 0, 0, 0);
	char* buf = (char*)alloca(len);
    if(len > 1){ 
        WideCharToMultiByte(codepage, 0, wstr, -1, buf, len, 0, 0);
		return string(buf, len - 1);
    }
	return string();

}

wstring toWideChar(const char* str)
{
#ifdef WW_DISABLE_UTF8
	const unsigned int codepage = CP_ACP;
#else
	const unsigned int codepage = CP_UTF8;
#endif
    int len = MultiByteToWideChar(codepage, 0, str, -1, NULL, 0);
	wchar_t* buf = (wchar_t*)alloca(len * sizeof(wchar_t));
    if(len > 1){ 
        MultiByteToWideChar(codepage, 0, str, -1, buf, len);
		return wstring(buf, len - 1);
    }
	return wstring();
}

wstring fromANSIToWide(const char* str)
{
	const unsigned int codepage = CP_ACP;
    int len = MultiByteToWideChar(codepage, 0, str, -1, NULL, 0);
	wchar_t* buf = (wchar_t*)alloca(len * sizeof(wchar_t));
    if(len > 1){ 
        MultiByteToWideChar(codepage, 0, str, -1, buf, len);
		return wstring(buf, len - 1);
    }
	return wstring();
}

string toANSIFromWide(const wchar_t* wstr)
{
	const unsigned int codepage = CP_ACP;
	int len = WideCharToMultiByte(codepage, 0, wstr, -1, NULL, 0, 0, 0);
	char* buf = (char*)alloca(len);
    if(len > 1){ 
        WideCharToMultiByte(codepage, 0, wstr, -1, buf, len, 0, 0);
		return string(buf, len - 1);
    }
	return string();

}


FILE* fopen(const char* utf8filename, const char* mode)
{
#ifdef WIN32
    return _wfopen(toWideChar(utf8filename).c_str(), toWideChar(mode).c_str());
#else
    return fopen(utf8filename, mode);
#endif
}

}
