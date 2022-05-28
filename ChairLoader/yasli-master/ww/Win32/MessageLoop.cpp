/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "ww/Win32/MessageLoop.h"

#include <algorithm>
#include <windows.h>
#include <windowsx.h>
#include <CrtDbg.h>

namespace Win32{

signal0 MessageLoop::signalIdle_;

MessageLoop::MessageLoop()
: dialogLoopInterrupted_(0)
{
}

int MessageLoop::run()
{
    MSG msg;
    while(true){
        if(!PeekMessage(&msg, 0, 0, 0, TRUE)){
            signalIdle_.emit();
            Sleep(10);
        }
        else{
            if(msg.message == WM_QUIT)
                return int(msg.wParam);

            bool filtered = false;
            for(size_t i = 0; i < filters_.size(); ++i)
                if(filters_[i]->filter(&msg))
                    filtered = true;

            if(!filtered) {
	            TranslateMessage(&msg); // генерит WM_CHAR из WM_KEYDOWN и т.п.
                DispatchMessage(&msg);
			}
        }
    }
    return 0;
}


int MessageLoop::runDialogLoop(HWND dialog)
{
	dialog_ = dialog;

	MSG msg;
	while(true){
		GetMessage(&msg, 0, 0, 0);

		if(msg.message == WM_QUIT){
			// repost QUIT-message, so main window can get it
			PostQuitMessage(int(msg.wParam));
			return int(msg.wParam);
		}

		if(!IsDialogMessage(dialog, &msg)){
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (dialogLoopInterrupted_) {
			dialogLoopInterrupted_ = false;
			return 0;
		}
	}
	return 0;
}

void MessageLoop::interruptDialogLoop()
{
	dialogLoopInterrupted_ = true;
	::PostMessageW(dialog_, WM_NULL, 0, 0);
}

void MessageLoop::quit()
{
    quit(0);
}

void MessageLoop::quit(int code)
{
    onQuit();
    ::PostQuitMessage(code);
}

void MessageLoop::processPendingMessages()
{
	MSG msg;
	while (PeekMessage(&msg, 0, 0, 0, TRUE))
	{
		if (msg.message == WM_QUIT)
			return;

		TranslateMessage(&msg); // генерит WM_CHAR из WM_KEYDOWN и т.п.
		DispatchMessage(&msg);
	}
}

void MessageLoop::installFilter(MessageFilter* filter)
{
    filters_.push_back(filter);
}

void MessageLoop::uninstallFilter(MessageFilter* filter)
{
    Filters::iterator it;
    for(it = filters_.begin(); it != filters_.end(); )
        if(it->get() == filter)
            it = filters_.erase(it);
        else
            ++it;
}


MessageLoop& MessageLoop::instance()
{
    static MessageLoop messageLoop;
    return messageLoop;
}

}
