/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once
#include "ww/Widget.h"

namespace ww{

class Container : public Widget{
public:
	// virtuals:
	bool isVisible() const;
	void setBorder(int border);
	// ^^^

	// internal methods:
	virtual void _arrangeChildren() {}
protected:
};

}
