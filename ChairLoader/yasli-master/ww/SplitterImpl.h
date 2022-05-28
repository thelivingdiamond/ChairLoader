/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once



#include "ww/Win32/Window32.h"
#include "ww/Win32/Handle.h"
namespace ww{
class Splitter;
class SplitterImpl : public Win32::Window32{
public:
	SplitterImpl(ww::Splitter* owner);
protected:
	BOOL	onMessageSetCursor(HWND window, USHORT hitTest, USHORT message);
	void	onMessageLButtonDown(UINT button, int x, int y);
	void	onMessageLButtonUp(UINT button, int x, int y);
	void	onMessageMouseMove(UINT button, int x, int y);

	void	onMessagePaint();

	int splitterByPoint(Vect2 point);
	Rect getSplitterRect(int splitterIndex);

	ww::Splitter* owner_;
	Vect2 lastCursorPosition_;
	int grabbedSplitter_;
};
};

