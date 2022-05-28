/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "ww/Box.h"

namespace ww{

class VBox : public Box{
public:
	VBox(int spacing = 0, int border = 0);
protected:
	void setElementPosition(Element& element, float offset, float length);
	float elementLength(const Element& element) const;
	float elementWidth(const Element& element) const;
	float boxLength() const;
	void setBoxSize(const Vect2& size);
};

}

