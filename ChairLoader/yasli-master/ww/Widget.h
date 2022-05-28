/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#pragma warning(disable: 4264) // warning C4264: '...' : no override available for virtual member function from base '...'; function is hidden

#include "ww/API.h"
#include "ww/MouseButton.h"
#include "ww/sigslot.h"
#include "ww/Vect2.h"
#include "ww/Rect.h"

#include "yasli/Pointers.h"

namespace Win32{
	class Window32;
};	

namespace ww{

enum{
	SERIALIZE_DESIGN = 1 << 0,
	SERIALIZE_STATE  = 1 << 1
};


class HotkeyContext;
class Container;
class Tooltip;

class Widget;

struct WidgetVisitor{
	virtual void operator()(Widget& widget) = 0;
};

// IMPORTANT: All methods, that start with underscore('_') are not supposed
// to be used by library users.
// When you override one of these methods in custom widget, please keep them
// protected or private.
	
class Widget : public PolyRefCounter, public yasli::WeakObject, public has_slots{
public:
	Widget();
	virtual ~Widget();
	virtual bool isVisible() const = 0;

	void setVisibility(bool visible);
	virtual void show();
	virtual void showAll();
	virtual void hide();

	virtual void visitChildren(WidgetVisitor& visitor) const{};

	/// sensitive to user input (enabled/active otherwise)
	virtual void setSensitive(bool sensitive){ sensitive_ = sensitive; }
	bool sensitive() const{ return sensitive_; }

	/// invisible border around control in pixels
	virtual void setBorder(int border);
	int border() const{ return border_; }

	/// minimal size requested by user
	Vect2 requestSize() const{ return requestSize_; }
	void setRequestSize(int w, int h);
	virtual void setRequestSize(const Vect2 size);

	virtual void setFocus();
	bool hasFocus() const;

	/// parent container
	Container* parent() const{ return parent_; }

	signal0& signalDelete() { return signalDelete_; };

	virtual void serialize(Archive& ar);

	void setToolTip(Tooltip* toolTip) { toolTip_ = toolTip; }
	Tooltip* toolTip() const { return toolTip_; }

	// All methods that start with underscore(_) may be considered as internal
	// and not supposed to be used by library users.
	
	// internal methods:
    virtual void _setParent(Container* container);
	const Rect& _position() const{ return position_; }
	virtual void _setPosition(const Rect& position);
	void _relayoutParents(bool propagate);
	virtual void _relayoutParents();
	virtual void _queueRelayout();

	const Vect2 _minimalSize() const;
	void _setMinimalSize(int x, int y);
	virtual void _setMinimalSize(const Vect2& size);

	virtual Win32::Window32* _window() const{ return 0; }
    
	virtual Widget* _focusedWidget() const;
	virtual void _setFocusedWidget(Widget* widget);

	virtual void _setFocus();

	virtual bool canBeDefault() const { return false; }
	virtual void setDefaultFrame(bool enable) {}

	virtual void _updateVisibility();
	virtual void _setVisibleInLayout(bool visibleInLayout);
	bool _visibleInLayout() const{ return visibleInLayout_; }
	bool _visible() const{ return visible_; }

	virtual HotkeyContext* _hotkeyContext();
protected:
	/// called from destructor
	signal0 signalDelete_;

	/// location in parent container (in pixels), don't confuse with coordinates in parent windows
	Rect position_;
	/// parent container, that owns widget
    Container* parent_;
	/// minimal adequate size of the control
	Vect2 minimalSize_;
	/// minimal size requested by user
	Vect2 requestSize_;
	/// border around control in pixels
	int border_;

	/// request to relayout before showing widget
	bool relayoutQueued_;
	/// sensitive to user input (enabled in windows terminology)
	bool sensitive_;
	/// visibility flag (doesn't guarantee that control visible, e.g. if parent is hidden)
	bool visible_;
	/// visible in layout - some container may hide children (e.g. splitter)
	bool visibleInLayout_;

	Tooltip* toolTip_;
};

Win32::Window32* _findWindow(const Widget* widget);

};

