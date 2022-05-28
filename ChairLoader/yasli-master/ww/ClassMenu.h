/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once
#include "yasli/StringList.h"

namespace ww{

struct ClassMenuItemAdder
{
	virtual void operator()(PopupMenuItem& root, int index, const char* text){
		root.add(text);
	}

	void generateMenu(PopupMenuItem& createItem, const StringList& comboStrings);
};


}

