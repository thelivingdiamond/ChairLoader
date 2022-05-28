/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "ww/Container.h"
#include <vector>

namespace ww{

enum PackMode{
	PACK_COMPACT,
	PACK_FILL,
	PACK_BEGIN,
	PACK_CENTER,
	PACK_END,
};

class Box : public Container{
public:
	Box(int spacing, int border);
	~Box();

	void clear(bool relayout = true);
	void add(Widget* widget, PackMode packMode = PACK_COMPACT, int padding = 0);
	void insert(Widget* widget, int beforeIndex, PackMode packMode = PACK_COMPACT, int padding = 0);
	void remove(Widget* widget);
	void remove(int index);
	size_t size() const { return elements_.size(); }

	void setSpacing(int spacing);
	int spacing() const{ return spacing_; }
	void setClipChildren(bool clipChildren);
	
	// virtuals
	void visitChildren(WidgetVisitor& visitor) const;
	void serialize(Archive& ar);
	// ^^

	void _setParent(Container* container);
	void _setPosition(const Rect& position);
	void _relayoutParents();
	void _arrangeChildren();
	void _updateVisibility();
	void _setFocus();

protected:
	struct Element : public RefCounter{
		Element(Widget* _widget = 0, PackMode _packMode = PACK_COMPACT, int _padding = 0) : widget(_widget), packMode(_packMode), packModeReal(_packMode), padding(_padding) {}
		void serialize(Archive& ar);
		bool fill() const { return packModeReal == PACK_FILL; }
		bool expand() const { return packModeReal >= PACK_FILL; }
		int offsetFactor() const { return packModeReal - PACK_BEGIN; }

		yasli::SharedPtr<Widget> widget;
		PackMode packMode;
		PackMode packModeReal; // после каждого изменения списка пробегаемся в обе стороны и compact'им: begin begin -> compact begin, end end -> end compact
		int padding;
	};

	virtual float elementLength(const Element& element) const = 0;
	virtual float elementWidth(const Element& element) const = 0;

	virtual float boxLength() const = 0;
	virtual void setElementPosition(Element& element, float offset, float length) = 0;
	virtual void setBoxSize(const Vect2& size) = 0;
	bool updateMinimalSize();

	typedef std::vector<Element> Elements;
	Elements elements_;
	bool clipChildren_;
	int spacing_;
	int focusedElementIndex_;
};


}

