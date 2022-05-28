/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "ww/Splitter.h"

namespace ww{
class SplitterImpl;
class HSplitter : public Splitter{
public:
	HSplitter(int splitterSpacing = 1, int border = 0);
	~HSplitter();
protected:
	HSplitter(int splitterSpacing, int border, SplitterImpl* impl);

	bool vertical() { return false; }
	int boxLength() const;
	Vect2 elementSize(Widget* widget) const;
	Rect rectByPosition(int start, int end);
	void setSplitterMinimalSize(const Vect2& size);
	int positionFromPoint(const Vect2 point) const;
};

}

