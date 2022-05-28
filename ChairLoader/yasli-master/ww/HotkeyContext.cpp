/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "HotkeyContext.h"
#include "Serialization.h"
#include "sigslot.h"
#include "Win32/MessageLoop.h"
#include "KeyPress.h"
#include "Application.h"

namespace ww{

class HotkeyFilter : public Win32::MessageFilter
{
public:
    HotkeyFilter(HotkeyContext* context)
    : context_(context)
    {
    }
    bool filter(MSG* msg)
    {
        if(msg->message == WM_KEYDOWN){
            UINT code = (UINT)msg->wParam;
            USHORT count = LOWORD(msg->lParam);
            USHORT flags = HIWORD(msg->lParam);
            KeyPress key = KeyPress::addModifiers(Key(code));
            if(context_->injectPress(key))
                return true;
        }
        return false;
    }
private:
    SharedPtr<HotkeyContext> context_;
};

// ---------------------------------------------------------------------------

HotkeyContext::HotkeyContext()
: filter_(0)
{
}

HotkeyContext::~HotkeyContext()
{
}

bool HotkeyContext::injectPress(KeyPress key)
{
    bool filtered = false;
    signalPressedAny_.emit(key, filtered);
    if(filtered)
        return true;
    HotkeySignals::iterator it = signalsPressed_.find(key);
    if(it != signalsPressed_.end()){
        it->second.emit();
        return true;
    }
    return false;
}

void HotkeyContext::installFilter(Application* app)
{
    filter_ = new HotkeyFilter(this);
    app->_messageLoop()->installFilter(filter_);
}

void HotkeyContext::uninstallFilter(Application* app)
{
    if(filter_){
        app->_messageLoop()->uninstallFilter(filter_);
        filter_ = 0;
    }
}

}
