/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include <vector>

#include "ww/API.h"
#include "ww/Win32/Types.h"
#include "ww/Rect.h"
#include "ww/Vect2.h"

#include "yasli/Pointers.h"
#include "yasli/Assert.h"


struct tagDRAWITEMSTRUCT;
typedef tagDRAWITEMSTRUCT DRAWITEMSTRUCT;

struct tagMEASUREITEMSTRUCT;
typedef tagMEASUREITEMSTRUCT MEASUREITEMSTRUCT;

namespace Win32{
using namespace yasli;

void _setGlobalInstance(HINSTANCE instance);
HINSTANCE _globalInstance();
/// проверяет зарегиистрирован ли класс кна
bool isClassRegistered(const wchar_t* className);
bool isKeyPressed(UINT keyCode);

LRESULT CALLBACK universalWindowProcedure(HWND handle, UINT message, WPARAM wparam, LPARAM lparam);

/// регистрирует класс окна использую обработчик сообщений библиотеки (вызывает метод onMessage)
//bool registerClass(const char* className);

/// приступить к обработки сообщений вплоть до получения WM_QUIT 
int basicMessageLoop(HACCEL acceleratorTable = 0);

ww::Vect2 calculateTextSize(HWND window, HFONT font, const wchar_t* text);
HFONT defaultFont();
HFONT defaultBoldFont();
void initializeCommonControls();

class Window32;

class TimerInterface : public ww::RefCounter{
public:
	TimerInterface()
	: window_(0)
	{}
	virtual ~TimerInterface();

	virtual void onTimer(){}
	void setWindow(Win32::Window32* window){ 
		YASLI_ASSERT(window_ == 0 || window == 0);
		window_ = window;
	}
	Window32* window(){ return window_; }
protected:
	Window32* window_;
};

/// encapsulates BeginDeferWindowPos, DeferWindowPos, EndDeferWindowPos
class WindowPositionDeferer : public ww::RefCounter{
public:
	WindowPositionDeferer(Window32* parent, int numWindows = 1);
	~WindowPositionDeferer();

	void defer(Window32* window, const ww::Rect& position);
protected:
	HDWP handle_;
	Window32* parent_;
};

/// инкапсулирует Windows-окно
class Window32 : public ww::RefCounter{
public:
	Window32(HWND handle = 0);
	virtual ~Window32();
	HWND attach(HWND handle);
	HWND detach();

	virtual const wchar_t* className() const{ return L"ww.Window"; }
	virtual bool registerClass(const wchar_t* className);
		
	virtual LRESULT onMessage(UINT message, WPARAM wparam, LPARAM lparam);
	
	virtual LRESULT defaultWindowProcedure(UINT message, WPARAM wparam, LPARAM lparam);

	virtual BOOL onMessageMeasureItem(UINT id, MEASUREITEMSTRUCT* drawItemStruct);
	virtual BOOL onMessageDrawItem(UINT id, DRAWITEMSTRUCT* drawItemStruct);

	virtual void onMessageClose();
	virtual int onMessageDestroy();
	virtual int onMessageChar(UINT code, USHORT count, USHORT flags);
	virtual int onMessageKeyDown(UINT keyCode, USHORT count, USHORT flags);
	
	virtual int onMessageKeyUp(UINT keyCode, USHORT count, USHORT flags);
	virtual int onMessageSysKeyDown(UINT keyCode, USHORT count, USHORT flags);
	virtual int onMessageSysKeyUp(UINT keyCode, USHORT count, USHORT flags);

	virtual BOOL onMessageEraseBkgnd(HDC dc);
	virtual void onMessageShowWindow(BOOL show, int status);
	virtual BOOL onMessageSize(UINT type, USHORT width, USHORT height);
    virtual BOOL onMessageWindowPosChanged(WINDOWPOS* windowPos);
	virtual int onMessageCommand(USHORT command, USHORT id, HWND wnd);
	virtual int onMessageKillFocus(HWND focusedWindow);
	virtual int onMessageSetFocus(HWND lastFocusedWindow);

	virtual BOOL onMessageSetCursor(HWND window, USHORT hitTest, USHORT message);
	virtual void onMessageMouseMove(UINT button, int x, int y);
	virtual void onMessageMouseWheel(SHORT delta);
	
	virtual void onMessageLButtonDblClk(int x, int y);

	virtual void onMessageLButtonDown(UINT button, int x, int y);
	virtual void onMessageLButtonUp(UINT button, int x, int y);

	virtual void onMessageMButtonDown(UINT button, int x, int y);
	virtual void onMessageMButtonUp(UINT button, int x, int y);

	virtual void onMessageRButtonDown(UINT button, int x, int y);
	virtual void onMessageRButtonUp(UINT button, int x, int y);

	virtual void onMessagePaint();
	virtual void onMessageTimer(int id);
	virtual int onMessageGetDlgCode(int keyCode, MSG* msg);

	bool isWindow() const;

	Window32* parent() const;

	// api functions
	bool create(const wchar_t* windowName, UINT style, const ww::Rect& position, HWND parentWnd, UINT exStyle = 0);
	bool creating() const{ return creating_; }
	void destroy();

	void setWindowText(const wchar_t* windowText);
	void enable(bool enabled);
	void show(int showCommand);
	ww::Rect getRect() const;
	void move(const ww::Rect& position, bool redraw = false);
	void update();
	bool isVisible() const;

	void setStyle(UINT style);
	UINT getStyle();
	void setParent(Window32* newParent);

	LONG_PTR setUserDataLongPtr(LONG_PTR newLongPtr);
	LONG_PTR getUserDataLongPtr();

	LONG_PTR getLongPtr(int index);
	LONG_PTR setLongPtr(int index, LONG_PTR newLongPtr);

	typedef ww::SharedPtr<WindowPositionDeferer> PositionDeferer;
	PositionDeferer positionDeferer(int numWindows = 1);	

	void clientToScreen(RECT& rect);
	void screenToClient(RECT& rect);
	//

	HWND handle() const{ return handle_; }

	void attachTimer(TimerInterface* timer, int interval);
	void detachTimer(TimerInterface* timer);
	typedef std::vector<ww::SharedPtr< TimerInterface> > Timers;
protected:
	Timers timers_;
	friend class WindowPositionDeferer;
	WindowPositionDeferer* positionDeferer_;
	bool handleOwner_;
	bool creating_;
	HWND handle_;
};

// A hidden service window that is used as default parent for
// windows that are not yet composed.
HWND getDefaultWindowHandle();
Win32::Window32* getDefaultWindow();

inline TimerInterface::~TimerInterface()
{
	if(window_)
		window_->detachTimer(this);
}

}

