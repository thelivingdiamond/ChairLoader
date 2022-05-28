/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"

#include "ww/Splitter.h"
#include "ww/HSplitter.h"
#include "ww/VSplitter.h"

#include "ww/Win32/MemoryDC.h"
#include "ww/Win32/Rectangle.h"

#include "ww/Serialization.h"
#include "yasli/ClassFactory.h"

#include "SplitterImpl.h"

#include <float.h>

#pragma warning(push)
#pragma warning(disable: 4355) // 'this' : used in base member initializer list

namespace ww{

SplitterImpl::SplitterImpl(ww::Splitter* owner)
: owner_(owner)
, grabbedSplitter_(-1)
, lastCursorPosition_(0, 0)
{
	create(L"", WS_CHILD | WS_CLIPCHILDREN, Rect(0, 0, 100, 100), Win32::getDefaultWindowHandle(), WS_EX_CONTROLPARENT);
}

void SplitterImpl::onMessagePaint()
{
	PAINTSTRUCT paintStruct;
	HDC paintDC = ::BeginPaint(handle(), &paintStruct);
    {
        //Win32::MemoryDC memoryDC(paintDC);
        HDC dc = paintDC;//memoryDC;

        //InflateRect(&windowRect, -1, -1);
        ::FillRect(dc, &paintStruct.rcPaint, GetSysColorBrush(COLOR_BTNFACE));

        if(owner_ && !owner_->flat_){
            int count = owner_->splittersCount();
            for(int i = 0; i < count; ++i){
                Rect rect = owner_->getSplitterRect(i);
                int spacing = owner_->splitterSpacing();

                int hspacing = owner_->positionFromPoint(Vect2(spacing, 0));
                int vspacing = owner_->positionFromPoint(Vect2(0, spacing));
                RECT windowRect = { rect.left() + hspacing,
                    rect.top() + vspacing,
                    rect.right() - hspacing,
                    rect.bottom() - vspacing };
                ::DrawEdge(dc, &windowRect, BDR_RAISEDINNER, BF_RECT);
            }
        }
    }
	EndPaint(handle(), &paintStruct);
	return Win32::Window32::onMessagePaint();
}

BOOL SplitterImpl::onMessageSetCursor(HWND window, USHORT hitTest, USHORT message)
{
	int count = owner_->splittersCount();

	POINT cursorPosition;
	WW_VERIFY(::GetCursorPos(&cursorPosition));
	WW_VERIFY(::ScreenToClient(handle_, &cursorPosition));

	for(int i = 0; i < count; ++i){
		Rect rect = owner_->getSplitterRect(i);

		if(rect.pointInside(Vect2(cursorPosition.x, cursorPosition.y))){
			if(owner_->positionFromPoint(Vect2(0xffff,  0)))
				SetCursor(LoadCursor(0, MAKEINTRESOURCE(IDC_SIZEWE)));
			else
				SetCursor(LoadCursor(0, MAKEINTRESOURCE(IDC_SIZENS)));
			return TRUE;						
		}
	}
	return Win32::Window32::onMessageSetCursor(window, hitTest, message);
}

void SplitterImpl::onMessageMouseMove(UINT button, int x, int y)
{
	Vect2 cursorPosition;
	::GetCursorPos((POINT*)(&cursorPosition));
	::ScreenToClient(handle_, (POINT*)&cursorPosition);
	cursorPosition -= Vect2(owner_->border(), owner_->border());
	if(cursorPosition != lastCursorPosition_){
		bool buttonPressed = button & MK_LBUTTON;
		if(grabbedSplitter_ >= 0 && buttonPressed && ::GetCapture() == handle()){
			owner_->setPanePosition(grabbedSplitter_, owner_->positionFromPoint(cursorPosition));
		}
	}
	else{

	}
	lastCursorPosition_ = cursorPosition;
	Win32::Window32::onMessageMouseMove(button, x, y);
}


int SplitterImpl::splitterByPoint(Vect2 point)
{
	int count = owner_->splittersCount();
	for(int i = 0; i < count; ++i){
		Rect rect = owner_->getSplitterRect(i);
		if(rect.pointInside(point)){
			return i;
		}
	}
	return -1;
}

Rect SplitterImpl::getSplitterRect(int splitterIndex)
{
	return owner_->getSplitterRect(splitterIndex);
}

void SplitterImpl::onMessageLButtonDown(UINT button, int x, int y)
{
	Vect2 cursorPosition(x, y);

	grabbedSplitter_ = splitterByPoint(Vect2(x, y));
	if(grabbedSplitter_ >= 0){
		::SetCapture(handle());
		return;
	}
	Win32::Window32::onMessageLButtonDown(button, x, y);
}

void SplitterImpl::onMessageLButtonUp(UINT button, int x, int y)
{
	if(::GetCapture() == handle()){
		::ReleaseCapture();
	}
	Win32::Window32::onMessageLButtonUp(button, x, y);
}

// ----------------------------------------------------------------------


Splitter::Splitter(int splitterSpacing, int border, SplitterImpl* impl)
: window_(impl)
, flat_(false)
{
	border_ = border;
	splitterSpacing_ = splitterSpacing;
}

Splitter::~Splitter()
{
	clear();
	delete window_;
	window_ = 0;
}

void Splitter::clear()
{
	Elements::iterator it;
	FOR_EACH(elements_, it){
		Element& element = *it;
		if(element.widget)
			element.widget->_setParent(0);
	}

	Elements temp;
	temp.swap(elements_);
	temp.clear();
}

void Splitter::resize(int newSize)
{

}

void Splitter::add(Widget* widget, float position, bool fold, int beforeIndex)
{
	YASLI_ASSERT(widget);
	Element element;
	element.widget = widget;
	element.position = position;
	element.snappedPosition = 0.f;
	element.fold = fold;
	if(beforeIndex < 0 || beforeIndex >= int(elements_.size()))
		elements_.push_back(element);
	else{
		YASLI_ASSERT(beforeIndex >= 0 && beforeIndex < int (elements_.size()));
		Elements::iterator it = elements_.begin();
		Element& currentElement = *it;

		std::advance(it, beforeIndex);
		if(it != elements_.end()){
			Element& nextElement = *it;
			float pos = nextElement.position;
		}
		elements_.insert(it, element);
	}
	_arrangeChildren();
	widget->_setParent(this);
	_queueRelayout();
}

void Splitter::_setPosition(const Rect& position)
{
	Container::_setPosition(position);
	window_->move(position);
	_arrangeChildren();
}

void Splitter::_setParent(Container* container)
{
	Widget::_setParent(container);

	Win32::Window32* window = _findWindow(container);
	if (!window)
		return;
	
	window_->setParent(window);

	Elements::iterator it;
	FOR_EACH(elements_, it){
		if(it->widget)
			it->widget->_setParent(this);
	}
	Container::_setParent(container);
}

void Splitter::_arrangeChildren()
{
	YASLI_ASSERT(this);
	if(!elements_.empty()){
		Win32::Window32* window = _findWindow(this);
		YASLI_ASSERT(window);
		Win32::Window32::PositionDeferer deferer = window->positionDeferer(int(elements_.size()));

		int totalLength = this->boxLength() - (int(elements_.size()) - 1) * splitterWidth();

		int pixelsLeft = totalLength;
		Elements::iterator it;

		FOR_EACH(elements_, it){
			Widget* widget = it->widget;
			YASLI_ESCAPE(widget, continue);
			Win32::Window32* window = widget->_window();
			YASLI_ASSERT(!window || ::IsWindow(window->handle()));
		}

		float lastPosition = 0.0f;
		bool nextVisible = true;
		int offset = 0;
		Elements::iterator lastSplitterIt = elements_.end();
		if(elements_.size() > 1)
			std::advance(lastSplitterIt, -2);
		FOR_EACH(elements_, it){
			bool lastSplitter = it == lastSplitterIt;
			bool lastElement = &*it == &elements_.back();
			Widget* widget = it->widget;
			if(widget){
				float position = lastElement ? 1.0f : it->position;

				int start = xround(totalLength * lastPosition) + offset;
				int end = xround(totalLength * position) + offset;
				int len = end - start;

				int snapOffset = 0;
				bool visibleInLayout = nextVisible;
				
				if(lastSplitter){
					Elements::iterator nextIt = it;
					++nextIt;
					int len = xround((1.0f - position) * totalLength);
					int nextElementSize = nextIt->widget ? elementSize(nextIt->widget).x : 0;
					if(nextElementSize > len){
						if(nextElementSize > 2 * (len || totalLength < len + nextElementSize + offset + splitterWidth())){
							snapOffset = len;
							nextVisible = false;
						}
						else
							snapOffset = len - nextElementSize;
					}
				}
				int size = elementSize(widget).x;
				if(size > len + snapOffset){
					if(size / 2 > len || totalLength < size){
						snapOffset = -len;
						visibleInLayout = false;
					}
					else
						snapOffset = size - len;
				}

				Rect rect = rectByPosition(start, start + len + snapOffset);
				widget->_setVisibleInLayout(visibleInLayout);
				widget->_setPosition(rect);

				it->splitterRect = rectByPosition(end + snapOffset, end  + snapOffset + splitterWidth());
				it->snappedPosition = position + float(snapOffset) / totalLength;
				pixelsLeft -= len;
				offset += splitterWidth();
				lastPosition = it->snappedPosition;
			}
		}
	}
}

int Splitter::splitterWidth() const
{
	return SPLITTER_WIDTH + splitterSpacing_ * 2;
}

void Splitter::_relayoutParents()
{
	Vect2 oldMinimalSize = _minimalSize();
	if(elements_.empty()){
		setSplitterMinimalSize(Vect2(border_ * 2, border_ * 2));
	}
	else{
		Elements::iterator it;
		int width = 0;
		int minimalLength = (int(elements_.size()) - 1) * splitterWidth();
		FOR_EACH(elements_, it){
			Vect2 size = elementSize(it->widget);
			if(!it->fold)
				minimalLength += size.x;
			width = std::max(width, size.y);
		}
		_arrangeChildren();
		setSplitterMinimalSize(Vect2(minimalLength + border_ * 2, width + border_ * 2));
	}
	Widget::_relayoutParents(oldMinimalSize != _minimalSize() ? true : false);
}

void Splitter::_updateVisibility()
{
	Widget::_updateVisibility();
	if(_visible() && _visibleInLayout())
		window_->show(SW_SHOWNOACTIVATE);
	else
		window_->show(SW_HIDE);
}

void Splitter::visitChildren(WidgetVisitor& visitor) const
{
	Elements::const_iterator it;
	FOR_EACH(elements_, it){
		if(it->widget)
			visitor(*it->widget);
	}
}

float Splitter::widgetPosition(Widget* widget) const
{
	Elements::const_iterator it;
	FOR_EACH(elements_, it){
		if(it->widget == widget)
			return it->position;
	}
	YASLI_ASSERT(0 && "Unable to find widget!");
	return 0.0f;
}

Widget* Splitter::widgetByPosition(float position)
{
	Elements::iterator it;
	float lastPosition = 0.0f;
	FOR_EACH(elements_, it){
		Element& element = *it;
		if(position > lastPosition && position < element.position)
			return element.widget;
		lastPosition = element.position;
	}
	return 0;
}


Widget* Splitter::widgetByIndex(int index)
{
	YASLI_ASSERT(size_t(index) < elements_.size());
	Elements::iterator it = elements_.begin();
	std::advance(it, index);
	return it->widget;
}

Widget* Splitter::widgetByScreenPoint(Vect2 point)
{
	Elements::iterator it;
	FOR_EACH(elements_, it){
		Element& element = *it;
		if(Widget* widget = element.widget){
			Win32::Window32* window = _findWindow(widget);
			YASLI_ASSERT(window);
			Win32::Rect rect(widget->_position());
			window->clientToScreen(rect);
			if(rect.pointIn(point))
				return widget;
		}
	}
	return 0;
}

int Splitter::splittersCount()
{
	return std::max(0, int(elements_.size()) - 1);
}

Rect Splitter::getSplitterRect(int splitterIndex)
{
	YASLI_ASSERT(splitterIndex < int(elements_.size()) - 1);
	Elements::iterator it = elements_.begin();
	std::advance(it, splitterIndex);
	return it->splitterRect;
}

void Splitter::remove(int index, bool inFavourOfPrevious)
{
	YASLI_ASSERT(index >= 0 && index <= splittersCount());
	Elements::iterator it = elements_.begin();
	std::advance(it, index);
	float endPosition = it->position;
	elements_.erase(it);
	if(inFavourOfPrevious && elements_.size() && index > 0){
		Elements::iterator it = elements_.begin();
		std::advance(it, index - 1);
		it->position = endPosition;
	}
	_arrangeChildren();
	_queueRelayout();
	if(::IsWindowVisible(_window()->handle()))
		::RedrawWindow(_window()->handle(), 0, 0, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_ERASE);
}

void Splitter::replace(Widget* oldWidget, Widget* newWidget)
{
	YASLI_ASSERT(newWidget);
	Elements::iterator it;
	for(it = elements_.begin(); it != elements_.end(); ++it){
		Element& element = *it;
		if(element.widget == oldWidget){
			oldWidget->_setParent(0);
			element.widget = newWidget;
			_arrangeChildren();
			newWidget->_setParent(this);
			_queueRelayout();
			return;
		}
	}
	YASLI_ASSERT(0 && "Unable to find oldWidget!");
}

void Splitter::setSplitterPosition(float position, int splitterIndex)
{
	YASLI_ASSERT(splitterIndex >= 0 && splitterIndex < int(elements_.size()));
	Elements::iterator it = elements_.begin();
	if(!elements_.empty()){
		int length = boxLength() - (int(elements_.size()) - 1) * splitterWidth();
		int offset = splitterIndex * splitterWidth();
		float limitMin = 0.0f;
		float limitMax = 1.0f;
		if(splitterIndex > 0){
			std::advance(it, splitterIndex - 1);
			limitMin = it->position;
			++it;
		}
		else
			std::advance(it, splitterIndex);
		if(splitterIndex + 2 < int(elements_.size())){
			++it;
			limitMax = it->position;
			--it;
		}

		it->position = max(limitMin, std::min(position, limitMax));
		_arrangeChildren();
		::RedrawWindow(window_->handle(), 0, 0, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN | RDW_UPDATENOW);
	}
}

void Splitter::setPanePosition(int index, int positionInPixels)
{
	if(!elements_.empty()){
		int length = boxLength() - (int(elements_.size()) - 1) * splitterWidth();
		int offset = index * splitterWidth();
		positionInPixels -= offset;
		positionInPixels = max(0, std::min(positionInPixels, length));
		float position = length > FLT_EPSILON ? float(positionInPixels) / length : 0.0f;
		setSplitterPosition(position, index);
	}
}

void Splitter::Element::serialize(Archive& ar)
{
	ar(position, "position", "&Position");
}

void Splitter::serialize(Archive& ar)
{
	if(ar.filter(SERIALIZE_STATE)){
		Elements::iterator i;
		FOR_EACH(elements_, i){
			ar(*i->widget, "widget", "Widget");
			ar(i->position, "position", "&Position");
		}
		if(ar.isInput()){
			_arrangeChildren();
			::RedrawWindow(window_->handle(), 0, 0, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN | RDW_UPDATENOW);
		}
	}
}

// ----------------------------------------------------------------------

HSplitter::HSplitter(int splitterSpacing, int border, SplitterImpl* impl)
: Splitter(splitterSpacing, border, impl)
{
}

HSplitter::HSplitter(int splitterSpacing, int border)
: Splitter(splitterSpacing, border, new SplitterImpl(this))
{

}

HSplitter::~HSplitter()
{
}

int HSplitter::positionFromPoint(const Vect2 point) const
{
	return point.x;
}

int HSplitter::boxLength() const
{
	return position_.width() - border_ * 2;
}

void HSplitter::setSplitterMinimalSize(const Vect2& size)
{
	minimalSize_ = size;
}

Rect HSplitter::rectByPosition(int start, int end) 
{
	Rect rect(border_ + start, border_, border_ + end, border_ + _position().height() - 2 * border_);
	return rect;
}

Vect2 HSplitter::elementSize(Widget* widget) const
{
	return widget->_minimalSize();
}

// ---------------------------------------------------------------------------

VSplitter::VSplitter(int splitterSpacing, int border, SplitterImpl* impl)
: Splitter(splitterSpacing, border, impl)
{
}

VSplitter::VSplitter(int splitterSpacing, int border)
: Splitter(splitterSpacing, border, new SplitterImpl(this))
{

}

VSplitter::~VSplitter()
{
}

int VSplitter::positionFromPoint(const Vect2 point) const
{
	return point.y;
}

int VSplitter::boxLength() const
{
	return position_.height() - border_ * 2;
}

void VSplitter::setSplitterMinimalSize(const Vect2& size)
{
	minimalSize_.x = size.y;
	minimalSize_.y = size.x;
}

Rect VSplitter::rectByPosition(int start, int end) 
{
	Rect rect(border_, border_ + start, _position().width() - border_, border_ + end);
	return rect;
}

Vect2 VSplitter::elementSize(Widget* widget) const
{
	Vect2 size = widget->_minimalSize();
	return Vect2(size.y, size.x);
}

}

#pragma warning(pop)
