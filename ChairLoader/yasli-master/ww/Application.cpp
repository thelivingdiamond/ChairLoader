/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "ww/Application.h"
#include "ww/Win32/Types.h"
#include "ww/Win32/MessageLoop.h"
#include "ww/Win32/Window32.h"

namespace ww{

Application* Application::globalInstance_;

Application* Application::get()
{
	return globalInstance_;
}

Application::Application(void* instance)
: messageLoop_(new Win32::MessageLoop())
{
	globalInstance_ = this;
    messageLoop_->signalIdle().connect(this, &Application::onLoopIdle);
    messageLoop_->signalQuit().connect(this, &Application::onLoopQuit);

#ifndef WW_DLL
    if(instance)
        Win32::_setGlobalInstance((HINSTANCE)instance);
#endif
}

Application::~Application()
{
	globalInstance_ = 0;
}

int Application::run()
{
    YASLI_ESCAPE(messageLoop_, return 0);
    return messageLoop_->run();
}

void Application::quit()
{
    quit(0);
}

void Application::quit(int returnCode)
{
    YASLI_ESCAPE(messageLoop_, return);
    messageLoop_->quit(returnCode);
}

void Application::onLoopIdle()
{
    signalIdle_.emit();
}

void Application::onLoopQuit()
{
    signalQuit_.emit();
}

void Application::processPendingMessages()
{
	Win32::MessageLoop::processPendingMessages();
}

}


#ifdef WW_DLL
extern "C" int APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
   UNREFERENCED_PARAMETER(lpReserved);

   if(dwReason == DLL_PROCESS_ATTACH){
       Win32::_setGlobalInstance(hInstance);
   }
   else if (dwReason == DLL_PROCESS_DETACH){
   }
   return TRUE;
}
#endif
