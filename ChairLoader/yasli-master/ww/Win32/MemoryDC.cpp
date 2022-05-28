/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"

#include "ww/Macros.h"
#include "yasli/Assert.h"
#include "ww/Win32/MemoryDC.h"
#include <windows.h>
#include <windowsx.h>

namespace Win32{

MemoryDC::MemoryDC(HDC dc)
: destinationDC_(dc)
, dc_(0)
{
	WW_VERIFY(GetClipBox(destinationDC_, &rect_) != ERROR);
	WW_VERIFY(dc_ = ::CreateCompatibleDC(destinationDC_));
	//WW_VERIFY(bitmap_ = ::CreateCompatibleBitmap(dc, rect_.right - rect_.left, rect_.bottom - rect_.top));
	WW_VERIFY(bitmap_ = ::CreateCompatibleBitmap(dc, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)));
	oldBitmap_ = (HBITMAP)::SelectObject(dc_, (HGDIOBJ)bitmap_);

/*

	GetBoundsRect(destinationDC_, &rect_, 0);
	WW_VERIFY(dc_ = ::CreateCompatibleDC(destinationDC_));
	WW_VERIFY(bitmap_ = ::CreateCompatibleBitmap(dc, rect_.right - rect_.left, rect_.bottom - rect_.top));
	oldBitmap_ = (HBITMAP)::SelectObject(dc_, (HGDIOBJ)bitmap_);
*/
	//SetViewportOrgEx(dc_, -rect_.left, -rect_.top, 0); 
}

MemoryDC::~MemoryDC()
{
	::BitBlt(destinationDC_, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),dc_, 0, 0, SRCCOPY);
	::SelectObject(dc_, (HGDIOBJ)oldBitmap_);
	::DeleteDC(dc_);
	::DeleteObject(bitmap_);
}

}
