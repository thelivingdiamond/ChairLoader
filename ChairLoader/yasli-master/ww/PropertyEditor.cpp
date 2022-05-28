/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "ww/PropertyEditor.h"
#include "ww/PropertyTree.h"
#include "ww/Win32/Window32.h"
#include "EditorDialog.h"

namespace ww{

bool edit(const Serializer& ser, const char* stateFileName, int flags, Widget* parent, const char* title)
{
	HWND parentHandle = 0;
	if (parent)	{
		Win32::Window32* parentWindow = _findWindow(parent);
		parentHandle = parentWindow->handle();
	}
	return edit(ser, stateFileName, flags, parentHandle, title);
}

bool edit(const Serializer& ser, const char* stateFileName, int flags, HWND parent, const char* title)
{
	bool result = false;
	const char* typeName = ser.type().name();
	EditorDialog dialog(ser, title, stateFileName, flags, parent);
	if(title)
		dialog.setTitle(title);
	result = dialog.showModal() == ww::RESPONSE_OK;
	return result;
}
}
