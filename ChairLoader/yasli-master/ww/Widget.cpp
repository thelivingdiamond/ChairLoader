/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"

#include "ww/Widget.h"
#include "ww/Container.h"
#include "ww/Window.h"

#include "ww/Win32/Window32.h"

#include "ww/Serialization.h"

namespace ww{

Widget::Widget()
: border_(0)
, minimalSize_(0, 0)
, requestSize_(0, 0)
, position_(0, 0, 1, 1)
, parent_(0)
, visible_(false)
, sensitive_(true)
, visibleInLayout_(true)
, relayoutQueued_(true)
, toolTip_(0)
{
}

Widget::~Widget()
{
	signalDelete_.emit();
    if(_focusedWidget() == this)
        _setFocusedWidget(0);
}

void Widget::show()
{
	bool wasVisible = visible_;
	visible_ = true;
	if(!wasVisible)
		_updateVisibility();
}

void Widget::hide()
{
	visible_ = false;
	_updateVisibility();
}

void Widget::setVisibility(bool visible)
{
	if(visible)
		show();
	else
		hide();
}

void Widget::_setVisibleInLayout(bool visibleInLayout)
{
	bool wasVisibleInLayout = visibleInLayout_;
	visibleInLayout_ = visibleInLayout;
	if(wasVisibleInLayout != visibleInLayout_)
		_updateVisibility();
}

void Widget::_setPosition(const ww::Rect& position)
{
	position_ = position;
}

void Widget::setRequestSize(const Vect2 size)
{
	requestSize_ = size;
}

void Widget::setRequestSize(int w, int h)
{
	requestSize_.set(w, h);
}

void Widget::setBorder(int border)
{
	border_ = border;
}

const Vect2 Widget::_minimalSize() const
{ 
	return Vect2(max(minimalSize_.x, requestSize_.x), max(minimalSize_.y, requestSize_.y)); 
}

void Widget::_setMinimalSize(int w, int h)
{
	_setMinimalSize(Vect2(w, h));
}

void Widget::_setMinimalSize(const Vect2& size)
{
	minimalSize_ = size;
}

void Widget::_setParent(Container* parent)
{
	parent_ = parent;
}

void Widget::_queueRelayout()
{
	if(!isVisible()){
		relayoutQueued_ = true;
		if(parent())
			parent()->_queueRelayout();
	}
	else
		_relayoutParents();
}

void Widget::_relayoutParents(bool propagate)
{
	relayoutQueued_ = false;
	if(propagate && parent()){
		if(parent()->isVisible())
			parent()->_relayoutParents();
		else
			parent()->_queueRelayout();
	}
}

void Widget::_relayoutParents()
{
	_relayoutParents(true);
}

void Widget::_updateVisibility()
{
	if(visible_ && relayoutQueued_){
		_relayoutParents();
	}
}

void Widget::setFocus()
{
    if(_window())
        ::SetFocus(_window()->handle());
}

bool Widget::hasFocus() const
{
	return _focusedWidget() == this;
}

HotkeyContext* Widget::_hotkeyContext() 
{ 
	return parent_ ? parent_->_hotkeyContext() : 0; 
}

Widget* Widget::_focusedWidget() const
{ 
	return parent_ ? parent_->_focusedWidget() : 0; 
}

void Widget::_setFocus()
{
    if(_focusedWidget() != this)
        _setFocusedWidget(this);
}

void Widget::_setFocusedWidget(Widget* widget)
{
	if(parent_) 
		parent_->_setFocusedWidget(widget); 
}

struct ShowAllVisitor : public WidgetVisitor{
	void operator()(Widget& widget){
		widget.showAll();
	}
};

void Widget::showAll()
{
	visitChildren(ShowAllVisitor());
	show();
}

void Widget::serialize(Archive& ar)
{
	if(ar.filter(ww::SERIALIZE_DESIGN)){
		/*
		ar(_property(border_, this, &Widget::setBorder), "border", "Отступы");
		ar(requestSize_, "requestSize", "Размер");
		ar(_property(sensitive_, this, &Widget::setSensitive), "sensitive", "Активность");
		ar(_property(visible_, this, &Widget::setVisibility), "visible", "Видимость");
		*/
	}
}

// --------------------------------------------------------------------------------

void Container::setBorder(int border)
{
	Widget::setBorder(border);
	_arrangeChildren();
}

bool Container::isVisible() const
{
	return parent() ? parent()->isVisible() : false;
}

Win32::Window32* _findWindow(const Widget* widget)
{
	while(widget){
		if(Win32::Window32* window = widget->_window())
			return window;
		else
			widget = widget->parent();
	}
	// TODO: review this logic
	return Win32::getDefaultWindow();
}

void _ensureChildren(ww::Container* container, ww::Widget* widget)
{
	if(widget && container && widget->parent() != container){
		container->_arrangeChildren();
		widget->_setParent(container);
		container->_queueRelayout();
	}
}

}
