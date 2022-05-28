/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include <float.h>
#include <algorithm>
#include "ww/Box.h"
#include "ww/HBox.h"
#include "ww/VBox.h"

#include "ww/Win32/Window32.h"

#include "ww/Serialization.h"
#include "yasli/Enum.h"
#include "yasli/ClassFactory.h"


namespace ww{

YASLI_CLASS(Widget, VBox, "Layout\\Box (Vertical)");
YASLI_CLASS(Widget, HBox, "Layout\\Box (Horizontal)");
YASLI_CLASS(Container, VBox, "Box (Vertical)");
YASLI_CLASS(Container, HBox, "Box (Horizontal)");

YASLI_ENUM_BEGIN(PackMode, "PackMode")
YASLI_ENUM_VALUE(PACK_COMPACT, "Compact")
YASLI_ENUM_VALUE(PACK_FILL, "Fill")
YASLI_ENUM_VALUE(PACK_BEGIN, "Left")
YASLI_ENUM_VALUE(PACK_CENTER, "Center")
YASLI_ENUM_VALUE(PACK_END, "Right")
YASLI_ENUM_END()

//////////////////////////////////////////////////////////////////////////////

Box::Box(int spacing, int border)
: spacing_(spacing)
, focusedElementIndex_(0)
, clipChildren_(false)
{
	border_ = border;
	YASLI_ASSERT(parent_ == 0);
}

Box::~Box()
{
	clear(false);
}

void Box::remove(Widget* widget)
{
	int index = 0;
	Elements::iterator it;
	FOR_EACH(elements_, it){
		if(it->widget == widget){
			Box::remove(index);
			return;
		}
		++index;
	}
 	YASLI_ASSERT(0);
}

void Box::remove(int index)
{
	YASLI_ASSERT(size_t(index) < elements_.size());
	Widget* widget = elements_[index].widget;
	widget->_setParent(0);
	elements_.erase(elements_.begin() + index);
	_queueRelayout();
}

void Box::add(Widget* widget, PackMode packMode, int padding)
{
	YASLI_ASSERT(widget);

	elements_.push_back(Element(widget, packMode, padding));

	widget->_setParent(this);
	_queueRelayout();
}

void Box::insert(Widget* widget, int beforeIndex, PackMode packMode, int padding)
{
	YASLI_ASSERT(widget);

	if(beforeIndex == -1)
		elements_.push_back(Element(widget, packMode, padding));
	else
		elements_.insert(elements_.begin() + beforeIndex, Element(widget, packMode, padding));
	
	widget->_setParent(this);
	_queueRelayout();
}

void Box::_relayoutParents()
{
	_arrangeChildren();
	Widget::_relayoutParents(updateMinimalSize());
}

void Box::visitChildren(WidgetVisitor& visitor) const
{
	Elements::const_iterator it;
	FOR_EACH(elements_, it)
		if(it->widget)
			visitor(*it->widget);
}

void Box::_updateVisibility()
{
	Widget::_updateVisibility();
	Elements::iterator it;
	FOR_EACH(elements_, it)
		if(it->widget)
			it->widget->_setVisibleInLayout(_visibleInLayout());
}

void Box::_arrangeChildren()
{
	float length = boxLength();
	float fixed_length = 0;
	int sizeable_elements = 0; 

	if(elements_.size() > 1){
		for(int i = 0; i < (int)elements_.size() - 1; i++)
			elements_[i].packModeReal = elements_[i].packMode == PACK_BEGIN && elements_[i].packMode == PACK_BEGIN ? PACK_COMPACT : elements_[i].packMode;
		elements_.back().packModeReal = elements_.back().packMode;
		for(int i = int(elements_.size()) - 1; i > 0; i--)
			if(elements_[i].packMode == PACK_END && elements_[i - 1].packMode == PACK_END)
				elements_[i].packModeReal = PACK_COMPACT;
	}

	Elements::iterator it;
	FOR_EACH(elements_, it){
		fixed_length += it->padding * 2.0f;
		fixed_length += elementLength(*it);
		if(it->expand())
			++sizeable_elements;
	}

	fixed_length += (float)max(0, int(elements_.size()) - 1) * spacing_;
	float sizeable_length = length - fixed_length;

	float element_length = sizeable_elements > 0 && sizeable_length > FLT_EPSILON ? sizeable_length/float(sizeable_elements) : 0;

	float offset = 0;
	Win32::Window32* window = _findWindow(this);
	if (!window)
		return;
	if(!elements_.empty()){
		Win32::Window32::PositionDeferer deferer = window->positionDeferer((int)elements_.size());
		FOR_EACH(elements_, it){
			if(it->widget){
				float len = it->expand() ? element_length + elementLength(*it) : elementLength(*it);

				offset += it->padding;
				setElementPosition(*it, offset, std::min(length - offset, len));

				offset += it->padding + len;
				offset += float(spacing_);
				offset = min(offset, length);
			}
		}
	}
	::RedrawWindow(window->handle(), 0, 0, RDW_ALLCHILDREN);
}

void Box::_setParent(Container* container)
{
	Container::_setParent(container);
	Elements::iterator it;
	FOR_EACH(elements_, it){
		it->widget->_setParent(this);
	}
	_setPosition(position_);
}

void Box::_setPosition(const Rect& position)
{
	Container::_setPosition(position);
	_arrangeChildren();
}

void Box::clear(bool relayout)
{
	Elements::iterator it;
	FOR_EACH(elements_, it){
		Element& element = *it;
		if(Widget* widget = element.widget)
			widget->_setParent(0);
	}

	// просто так вызвать clear у списков нельзя, т.к. при удаление контролов вызывется код апдейта раскладки
	// и кто-нибудь обязательно проитерируется по удаленным children-ам этого объекта

	Elements temp;
	temp.swap(elements_);
	
	if(relayout)
		_queueRelayout();
}	

void Box::_setFocus()
{
	__super::_setFocus();
}


void Box::Element::serialize(Archive& ar)
{
    if(ar.filter(SERIALIZE_DESIGN)){
	    ar(packMode, "packMode", "Pack mode");
	    ar(padding, "padding", "Padding");
	    ar(widget, "widget", "&Widget");
    }
}

void Box::serialize(Archive& ar)
{
    if(ar.filter(SERIALIZE_DESIGN)){
	    ar(_property(spacing_, this, &Box::setSpacing), "spacing", "Spacing");
	    Container::serialize(ar);

	    ar(elements_, "elements", "Elements");
	    Elements::iterator it;
	    FOR_EACH(elements_, it)
		    _ensureChildren(this, it->widget);
    }
	else if(ar.filter(SERIALIZE_STATE)){
		for(size_t i = 0; i < elements_.size(); ++i)
			ar(*elements_[i].widget, "widget", "Widget");
	}
}

void Box::setSpacing(int spacing)
{
	spacing_ = spacing;
	_arrangeChildren();
	_relayoutParents();
}

void Box::setClipChildren(bool clipChildren)
{
	clipChildren_ = clipChildren;
}

bool Box::updateMinimalSize()
{
	Win32::Window32* window = _findWindow(this);
	if (!window)
		return false;

    Vect2 oldMinimalSize = _minimalSize();

    float length = 0;
    float width = 0;
	Elements::iterator it;
    FOR_EACH(elements_, it){
        length += elementLength(*it);
        width = std::max(elementWidth(*it), width);
        length += it->padding * 2.0f;
    }
    length += float(spacing_)*float(std::max(0, int(elements_.size()) + 1)) + border_ * 2.0f;
    width += border_ * 2.0f;
    if(clipChildren_)
        setBoxSize(Vect2(0, xround(width)));
    else
        setBoxSize(Vect2(xround(length), xround(width)));
    ::RedrawWindow(window->handle(), 0, 0, RDW_INVALIDATE | RDW_ALLCHILDREN);
    return oldMinimalSize != _minimalSize();

}
//////////////////////////////////////////////////////////////////////////////
VBox::VBox(int spacing, int border)
: Box(spacing, border)
{
}

float VBox::elementLength(const Element& element) const
{
	return float(element.widget->_minimalSize().y);
}

float VBox::elementWidth(const Element& element) const
{
	return float(element.widget->_minimalSize().x);
}

float VBox::boxLength() const
{
	return float(_position().height() - border_ * 2);
}

void VBox::setElementPosition(Element& element, float offset, float length)
{
	int border = this->border();
	Rect rect(border, border + xround(offset), _position().width() - border, border + xround(offset) + xround(length));
	if(element.fill()){
		element.widget->_setPosition(rect + position_.leftTop());
	}
	else{
		if(length > element.widget->_minimalSize().y){
            rect.setTop(rect.top() + (rect.height() - element.widget->_minimalSize().y)*element.offsetFactor()/2);
            rect.setHeight(element.widget->_minimalSize().y);
            element.widget->_setPosition(rect + position_.leftTop());
        }
        else
            element.widget->_setPosition(rect + position_.leftTop());
	}
}

void VBox::setBoxSize(const Vect2& size)
{
	minimalSize_.x = size.y;
	minimalSize_.y = size.x;
}

//////////////////////////////////////////////////////////////////////////////
HBox::HBox(int spacing, int border)
: Box(spacing, border)
{
}

float HBox::elementLength(const Element& element) const
{
	return float(element.widget->_minimalSize().x);
}

float HBox::elementWidth(const Element& element) const
{
	return float(element.widget->_minimalSize().y);
}

float HBox::boxLength() const
{
	return float(_position().width() - border_ * 2);
}

void HBox::setElementPosition(Element& element, float offset, float length)
{
	int border = this->border();
	Rect rect(border + xround(offset), border, border + xround(offset) + xround(length), _position().height() - border);
	if(element.fill()){
		element.widget->_setPosition(rect + position_.leftTop());
	}
	else{
		if(length > element.widget->_minimalSize().x){
			rect.setLeft(rect.left() + (rect.width() - element.widget->_minimalSize().x)*element.offsetFactor()/2);
			rect.setWidth(element.widget->_minimalSize().x);
			element.widget->_setPosition(rect + position_.leftTop());
		}
		else
			element.widget->_setPosition(rect + position_.leftTop());
	}
}

void HBox::setBoxSize(const Vect2& size)
{
	minimalSize_.x = size.x;
	minimalSize_.y = size.y;
}

//////////////////////////////////////////////////////////////////////////////
}
