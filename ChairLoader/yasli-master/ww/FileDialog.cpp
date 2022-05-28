/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "ww/FileDialog.h"
#include "ww/Widget.h"
#include "ww/Win32/Window32.h"
#include "ww/Unicode.h"
#include "ww/Files.h"
#define WIN32_LEAN_AND_MEAN
#include "Win32/CommonControls.h"
#include <windows.h>
#include <commdlg.h>
#include <cderr.h>

namespace ww{

// COPY_PASTE
static HWND findOwner(HWND wnd){
	if(!wnd)
		return wnd;
	while(HWND parent = GetParent(wnd))
		wnd = parent;
	return wnd;
}

// COPY_PASTE
static HWND findOwner(Widget* widget){
	if(!widget)
		return 0;
	HWND parentWnd = 0;
	Win32::Window32* window = _findWindow(widget);
	YASLI_ASSERT(window);
	if(window)
		return findOwner(window->handle());
	else
		return 0;
}

FileDialog::FileDialog(ww::Widget* owner, bool save, const char** masks, const char* startDirectory, const char* startFileName, bool allow_multiselect)
: ownerWnd_(findOwner(owner))
, save_(save)
, startDirectory_(startDirectory ? startDirectory : "")
, startFileName_(startFileName ? startFileName : "")
, multiselect_(allow_multiselect)
{
	startFileName_ = Files::fixSlashes(startFileName_.c_str());
	const char** p = masks;
	while(*p){
        masks_.push_back(*p);
		++p;
		YASLI_ESCAPE(*p, return);
        masks_.push_back(*p);
		++p;
	}
}

bool FileDialog::showModal()
{
	wchar_t fileName[_MAX_PATH + _MAX_FNAME * 32];
	ZeroMemory(fileName, sizeof(fileName));
	if(startFileName_ != "")
		wcscpy_s(fileName, toWideChar(startFileName_.c_str()).c_str());
	
	wchar_t filter[512];
	ZeroMemory(filter, sizeof(filter));
	wchar_t* p = filter;
	const wchar_t* filterEnd = filter + ARRAY_LEN(filter) - 2;

	std::vector<string>::iterator it;
	for(it = masks_.begin(); it != masks_.end(); ++it){
		wstring mask(toWideChar(it->c_str()));
		int len = int(wcslen(mask.c_str()));
		YASLI_ASSERT(p + len + 1< filterEnd);
		wcscpy_s(p, filterEnd - p - 1, mask.c_str());
		p += len;
		*p = L'\0';
		++p;
	}
	p[0] = L'\0';

	OPENFILENAMEW openFileName;
	ZeroMemory(&openFileName, sizeof(openFileName));
	openFileName.lStructSize = sizeof(openFileName);
	openFileName.Flags = OFN_NOCHANGEDIR;
	if(!save_){
		openFileName.Flags |= OFN_PATHMUSTEXIST;
		if(multiselect_)
			openFileName.Flags |= OFN_EXPLORER | OFN_ALLOWMULTISELECT;
	}

	openFileName.hwndOwner = ownerWnd_;
	openFileName.hInstance = Win32::_globalInstance();
	openFileName.lpstrTitle = save_ ? L"Save File" : L"Open File";
	openFileName.lpstrFilter = filter;
	openFileName.nFilterIndex = 1;
	openFileName.lpstrCustomFilter = 0;
	openFileName.nMaxCustFilter = ARRAY_LEN(filter) - 1;
	openFileName.lpstrFile = fileName;
	openFileName.nMaxFile = ARRAY_LEN(fileName) - 1;
	wstring startDirectory(toWideChar(startDirectory_.c_str()));
	openFileName.lpstrInitialDir = startDirectory.c_str();
	if(save_){
		if(!GetSaveFileNameW(&openFileName))
			return false;
	}
	else{
		if(!GetOpenFileNameW(&openFileName))
		{
			/* TODO: handle
			CDERR_DIALOGFAILURE
			CDERR_FINDRESFAILURE
			CDERR_NOHINSTANCE
			CDERR_INITIALIZATION
			CDERR_NOHOOK
			CDERR_LOCKRESFAILURE
			CDERR_NOTEMPLATE
			CDERR_LOADRESFAILURE
			CDERR_STRUCTSIZE
			CDERR_LOADSTRFAILURE
			FNERR_BUFFERTOOSMALL
			CDERR_MEMALLOCFAILURE
			FNERR_INVALIDFILENAME
			CDERR_MEMLOCKFAILURE
			FNERR_SUBCLASSFAILURE
			*/
			DWORD errorCode = CommDlgExtendedError();
            switch(errorCode)
            {
            case FNERR_INVALIDFILENAME:
				YASLI_ASSERT(0, "Invalid file name: %s", fromWideChar(openFileName.lpstrFile).c_str());
                break;
            default:
			    YASLI_ASSERT(errorCode == 0);
                break;
                
            }
			return false;
		}
	}

	if(!save_ && multiselect_ && fileName[wcslen(fileName) + 1]){
		wchar_t buf[_MAX_PATH];
		wchar_t* p = fileName + wcslen(fileName) + 1;
		while(wcslen(p)){
			swprintf_s(buf, ARRAY_LEN(buf), L"%s\\%s", fileName, p);
			fileNames_.push_back(fromWideChar(buf));
			p += wcslen(p) + 1;
		}
	}
	else {
		fileNames_.push_back(fromWideChar(fileName));
		fileName_ = fromWideChar(fileName);
	}

	return true;
}

void FileDialog::serialize(Archive& ar)
{
	//__super::serialize(ar);
}

}

