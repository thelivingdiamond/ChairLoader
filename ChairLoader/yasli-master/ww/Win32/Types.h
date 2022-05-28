/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

// basic types from <windows.h>:
typedef unsigned long       DWORD;
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef float               FLOAT;
typedef long				LONG;

typedef int                 INT;
typedef unsigned int        UINT;
typedef short               SHORT;
typedef unsigned short      USHORT;

#ifdef _WIN64
typedef unsigned __int64 UINT_PTR;
typedef __int64 LONG_PTR;
#else
typedef __w64 unsigned int       UINT_PTR;
typedef __w64 long				LONG_PTR;
#endif

typedef UINT_PTR            WPARAM;
typedef LONG_PTR            LPARAM;
typedef LONG_PTR            LRESULT;

typedef DWORD   COLORREF;

struct HWND__;
typedef HWND__* HWND;
struct HDC__;
typedef HDC__* HDC;
struct HMENU__;
typedef HMENU__* HMENU;
struct HBITMAP__;
typedef HBITMAP__* HBITMAP;
struct HRGN__;
typedef HRGN__* HRGN;
struct HFONT__;
typedef HFONT__* HFONT;
struct HINSTANCE__;
typedef HINSTANCE__* HINSTANCE;
struct HACCEL__;
typedef HACCEL__* HACCEL;
struct _IMAGELIST;
typedef struct _IMAGELIST* HIMAGELIST;
struct tagRECT;
typedef tagRECT RECT;
struct tagMSG;
typedef tagMSG MSG;
typedef void *HANDLE;
typedef HANDLE HDWP;

typedef struct tagWINDOWPOS WINDOWPOS;


#define CALLBACK    __stdcall
#define WINAPI      __stdcall
// ^^^ это все можно заменить на #include <windows.h>



namespace Win32{

}

