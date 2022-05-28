/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include <map>
#include "ww/API.h"
#include "ww/sigslot.h"
#include "KeyPress.h"
#include "yasli/Pointers.h"

namespace ww{

class Application;
class HotkeyFilter;
class HotkeyContext : public RefCounter{
public:
    HotkeyContext();
    ~HotkeyContext();

	bool injectPress(KeyPress key);

	signal0& signalPressed(KeyPress key) { return signalsPressed_[key]; }
	signal2<KeyPress, bool&>& signalPressedAny(){ return signalPressedAny_; }

    void installFilter(Application* app);
    void uninstallFilter(Application* app);
protected:
	typedef std::map<KeyPress, signal0> HotkeySignals;
	HotkeySignals signalsPressed_;

	signal2<KeyPress, bool&> signalPressedAny_;
	yasli::SharedPtr<HotkeyFilter> filter_;
};

}

