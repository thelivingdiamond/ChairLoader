/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include <string>
#include "ww/Container.h"
#include "ww/HotkeyContext.h"

struct HWND__;
typedef HWND__* HWND;

namespace Win32{
    class Window;
};

namespace ww{

class WindowImpl;

enum WindowPosition{
	POSITION_DEFAULT,
	POSITION_CENTER,
	POSITION_LEFT_TOP,
	POSITION_LEFT,
	POSITION_LEFT_BOTTOM,
	POSITION_RIGHT_TOP,
	POSITION_RIGHT,
	POSITION_RIGHT_BOTTOM,
	POSITION_TOP,
	POSITION_BOTTOM,
	POSITION_CURSOR
};

class PopupMenu;

class Window : public Container{
public:
	Window(Application* app, int border = 4);

	Window(int border = 4, int style = 0);
	Window(HWND parent, int border);

	// virtuals:
	~Window();

	bool isVisible() const;
	void showAll();

	void visitChildren(WidgetVisitor& visitor) const;
	// ^^^


	/// add widget into window
	void add(Widget* widget);
	/// remove the single child widget
	void remove();

	/// sets window titlebar
	void setTitle(const char* str);

	void setShowTitleBar(bool showTitleBar);
	void setIconFromResource(const char* resourceName);
	void setIconFromFile(const char* resourceName);

	void setDefaultPosition(WindowPosition position);
	void setDefaultSize(int w, int h);
	void setDefaultSize(Vect2 size);

	void setResizeable(bool allowResize);
	bool resizeable() const{ return resizeable_; }

	void setMinimizeable(bool allowMinimize);
	bool minimizeable() const{ return minimizeable_; }

	Rect restoredPosition() const;
	void setRestoredPosition(const Rect& position);

	void setMaximized(bool maximized);
	bool maximized() const;

	void setMenu(PopupMenu* menu);
	PopupMenu* menu();
	// this method should be called after each manual change of menu
	void updateMenu();

	// virtual events:
	virtual void onClose();
	virtual void onResize(int width, int height) {}
	virtual void onSetFocus() {}

	void serialize(Archive& ar);
	
	// signals
	signal0& signalClose(){ return signalClose_; }
	signal0& signalActivate(){ return signalActivate_; }
	sigslot::signal0& signalPressed(KeyPress key) { return hotkeyContext_->signalPressed(key); }
	sigslot::signal2<KeyPress, bool&>& signalPressedAny(){ return hotkeyContext_->signalPressedAny(); }

	// internal methods:
	void setDefaultWidget(Widget* widget);
	Win32::Window32* _window() const{ return window_; }
	HotkeyContext* _hotkeyContext(){ return hotkeyContext_; }
	void _setFocusedWidget(Widget* widget);
	Widget* _focusedWidget() const { return focusedWidget_; }
protected:
	void init(HWND parent, int border, Application* app);

	void _updateVisibility();
	void _arrangeChildren();
	void _relayoutParents();

	// little workarounds for Dialogs:
	virtual void _onWMCommand(int command) {}

	unsigned int calculateStyle();
	void reposition();

	sigslot::signal0 signalClose_;
	sigslot::signal0 signalActivate_;

	bool resizeable_;
	bool minimizeable_;
	bool showTitleBar_;
	bool positioned_;
	WindowPosition windowPosition_;
	Vect2 defaultSize_;
	string title_;
	int style_;

	yasli::SharedPtr<Widget> child_;

	// we need to know focused widget so we can restore it when focus goes away 
	// and then comes back to the window
	yasli::WeakPtr<Widget> focusedWidget_;

	yasli::SharedPtr<PopupMenu> menu_;
	yasli::SharedPtr<HotkeyContext> hotkeyContext_;

	Win32::Window32* window_;

	Widget* defWidget_;
	Application* app_;

	friend class WindowImpl;
};

};

