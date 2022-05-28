/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "ww/Window.h"
#include "ww/Widget.h"

#include "ww/Application.h"
#include "ww/Win32/Window32.h"
#include "ww/Win32/MessageLoop.h"
#include "ww/Win32/Rectangle.h"
#include "ww/Win32/Handle.h"
#include "ww/PopupMenu.h"
#include "ww/HotkeyContext.h"


#include "ww/Serialization.h"
#include "ww/Unicode.h"
#include "yasli/ClassFactory.h"
#include "ww/Win32/Drawing.h"
#include "ww/PropertyTree/wwDrawContext.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace ww{

class WindowMessageFilter : public Win32::MessageFilter
{
public:
	WindowMessageFilter(HWND wnd)
	: wnd_(wnd)
	{
		YASLI_ASSERT(wnd_ != 0);
	}

	bool filter(MSG* msg)
	{
		if (!msg->hwnd)
			return false;

		if (!::IsChild(wnd_, msg->hwnd))
			return false;

		// We are using IsDialogMessage as convenient implementation of
		// focus navigation.
		//
		// Children with WS_TABSTOP style can receive focus.
		// WS_EX_CONTROLCONTAINER style is used to tell when control is a
		// container itself, so its children can gain focus.
		//
		// IsDialogMessage sends WM_GETDLGCODE to a focused control to
		// allow it to override defaults keys behaviour.
		//
		// This call may be replaced in future to implement more customized
		// Navigation logic.
		return IsDialogMessage(wnd_, msg) != FALSE;
	}
private:
	HWND wnd_;
};

// ---------------------------------------------------------------------------

class WindowImpl : public Win32::Window32{
public:
	WindowImpl(ww::Window* window);
	~WindowImpl();
	// virtuals:
	LRESULT onMessage(UINT message, WPARAM wparam, LPARAM lparam);
	int onMessageCommand(USHORT command, USHORT id, HWND wnd);
	int onMessageKeyDown(UINT keyCode, USHORT count, USHORT flags);
	// ^^^
	
	void setMenu(ww::PopupMenu* root);
protected:
	HMENU menu_;

	ww::Window* owner_;
	SharedPtr<WindowMessageFilter> filter_;
};

WindowImpl::WindowImpl(ww::Window* owner)
: Win32::Window32()
, owner_(owner)
, menu_(0)
{
}


WindowImpl::~WindowImpl()
{
	if (Application::get()) {
		Application::get()->_messageLoop()->uninstallFilter(filter_);
		filter_ = 0;
	}

	if (menu_) {
		::DestroyMenu(menu_);
	}
}

void WindowImpl::setMenu(PopupMenu* popupMenu)
{
	if (menu_) {
		DestroyMenu(menu_);
		menu_ = 0;
	}

	if (!popupMenu)
		return;

	menu_ = (HMENU)popupMenu->_generateMenuBar();
	::SetMenu(handle(), menu_);
}

int WindowImpl::onMessageCommand(USHORT command, USHORT id, HWND wnd)
{
	if (command == 0 && wnd == 0) {
		// menu command
		if (menu_ != 0) {
			if (owner_->menu_) {
				ww::PopupMenuItem* item = owner_->menu_->root().findById(id);
				if (item) {
					item->_call();
				}
				// else {
				//	YASLI_ASSERT(item != 0 && "Missing menu item for command");
				//}
			}
			else {
				YASLI_ASSERT(owner_->menu_ != 0);
			}
		}
	}
	else {
		owner_->_onWMCommand(command);
	}
	return __super::onMessageCommand(command, id, wnd);
}


LRESULT WindowImpl::onMessage(UINT message, WPARAM wparam, LPARAM lparam)
{
	switch(message){
	case WM_SYSCOLORCHANGE:
		{
			DrawingCache::get()->flush();
			break;
		}
	case WM_CREATE:
		{
			BOOL result = __super::onMessage(message, wparam, lparam);
			filter_ = new WindowMessageFilter(handle());

			if (Application::get()) {
				Application::get()->_messageLoop()->installFilter(filter_);
			}
			return result;
		}
	case WM_CLOSE:
		owner_->onClose();
		return 0;
    case WM_ACTIVATE:
        {
            UINT activationMethod = LOWORD(wparam);
            if(activationMethod != WA_INACTIVE)
                owner_->signalActivate_.emit();
        }
        return 0;
	case WM_SETFOCUS:
		if (Widget* focusedWidget = owner_->_focusedWidget())
			focusedWidget->setFocus();
		owner_->onSetFocus();
		break;
	case WM_WINDOWPOSCHANGED:
		{
			WINDOWPOS* windowPos = (WINDOWPOS*)(lparam);
			if(!(windowPos->flags & SWP_NOSIZE)){
				UINT mode = UINT(wparam);
				UINT width = LOWORD(lparam);
				UINT height = HIWORD(lparam);
				if(mode != SIZE_MINIMIZED){
					owner_->_arrangeChildren();

					RedrawWindow(handle(), 0, 0, RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_ERASE);
				}
			}
			return TRUE;
		}
	case WM_SIZING: 
		{
			// here we are limiting minimal size of the window
			UINT edge = (UINT)wparam;   // edge or corner which is moved by user
			RECT* rect = (RECT*)lparam; // non-client window rectangle
			RECT windowRect;
			RECT clientRect;
			::GetWindowRect(handle(), &windowRect);
			::GetClientRect(handle(), &clientRect);

			POINT borderSize; // difference between non-client and client size
			borderSize.x = windowRect.right - windowRect.left - (clientRect.right - clientRect.left);
			borderSize.y = windowRect.bottom - windowRect.top - (clientRect.bottom - clientRect.top);

			if(rect->right - rect->left < owner_->_minimalSize().x + borderSize.x){
				if(edge == WMSZ_BOTTOMLEFT || edge == WMSZ_TOPLEFT || edge == WMSZ_LEFT)
					rect->left = rect->right - owner_->_minimalSize().x - borderSize.x;
				else if(edge == WMSZ_BOTTOMRIGHT || edge == WMSZ_TOPRIGHT || edge == WMSZ_RIGHT)
					rect->right = rect->left + owner_->_minimalSize().x + borderSize.x;
			}
			if(rect->bottom - rect->top < owner_->_minimalSize().y + borderSize.y){
				if(edge == WMSZ_BOTTOMLEFT || edge == WMSZ_BOTTOMRIGHT || edge == WMSZ_BOTTOM)
					rect->bottom = rect->top + owner_->_minimalSize().y + borderSize.y;
				else if(edge == WMSZ_TOPLEFT || edge == WMSZ_TOPRIGHT || edge == WMSZ_TOP)
					rect->top = rect->bottom - owner_->_minimalSize().y - borderSize.y;
			}
			return TRUE;
		}
	}
	return Win32::Window32::onMessage(message, wparam, lparam);
}

int WindowImpl::onMessageKeyDown(UINT keyCode, USHORT count, USHORT flags)
{
	HotkeyContext* hotkeyContext = owner_->_hotkeyContext();
	if(hotkeyContext && !((flags >> 14) & 1))
		hotkeyContext->injectPress(KeyPress::addModifiers(Key(keyCode)));
	return __super::onMessageKeyDown(keyCode, count, flags);
}

// -------------------------------------------------------------------------------------
Window::Window(Application* app, int border)
{
    style_ = 0;
    init(0, border, app);
}

Window::Window(int border, int style)
{
	style_ = style;
	init(0, border, 0);
}

Window::Window(HWND parent, int border)
{
	style_ = 0;
	init(parent, border, 0);
}

Window::~Window()
{
	if(app_){
		if(hotkeyContext_ )
			hotkeyContext_->uninstallFilter(app_);
	}
	defWidget_ = 0;
	if(child_)
		child_->_setParent(0);
	child_ = 0;
	delete window_;
	window_ = 0;
}

void Window::init(HWND owner, int border, Application* app)
{
	resizeable_ = true;
	minimizeable_ = true;
	showTitleBar_ = true;
	windowPosition_ = POSITION_DEFAULT;
	positioned_ = false;
	defaultSize_.set(0, 0);
	defWidget_ = 0;
	focusedWidget_ = 0;
	app_ = app;
	hotkeyContext_ = new HotkeyContext;
	if(app_)
		hotkeyContext_->installFilter(app_);

	window_ = new WindowImpl(this);
	WW_VERIFY(window_->create(toWideChar(title_.c_str()).c_str(), calculateStyle(), Rect(0, 0, 400, 400), owner));
	setBorder(border);
}

unsigned int Window::calculateStyle()
{
	if(style_)
		return style_;  

	int style = WS_CLIPCHILDREN/* | WS_SYSMENU*/;
	if(showTitleBar_)
		style |= WS_CAPTION | WS_SYSMENU;
	else
		if(resizeable_)
			style |= WS_DLGFRAME;

	if(resizeable_)
		style |= WS_MAXIMIZEBOX | WS_THICKFRAME; 
	else{
		style |= WS_POPUP;
		if(!showTitleBar_)
			style |= WS_DLGFRAME;
	}
	if(minimizeable_)
		style |= WS_MINIMIZEBOX;

	return style;
}

void Window::setResizeable(bool allowResize)
{
	resizeable_ = allowResize;
	window_->setStyle(calculateStyle()); 
}

void Window::setRestoredPosition(const Rect& position)
{
	HWND handle = _window()->handle();
	WINDOWPLACEMENT placement;
	placement.length = sizeof(placement);
	WW_VERIFY(GetWindowPlacement(handle, &placement));
	placement.rcNormalPosition = Win32::Rect(position);
	placement.length = sizeof(placement);
	WW_VERIFY(SetWindowPlacement(handle, &placement));
}

Rect Window::restoredPosition() const
{
	WINDOWPLACEMENT placement;
	placement.length = sizeof(placement);
	WW_VERIFY(GetWindowPlacement(_window()->handle(), &placement));
	return Win32::Rect(placement.rcNormalPosition).recti();
}

void Window::setMinimizeable(bool allowMinimize)
{
	minimizeable_ = allowMinimize;
	window_->setStyle(calculateStyle()); 
}

void Window::setMaximized(bool maximized)
{
	if(this->maximized() != maximized){
		ShowWindow(_window()->handle(), maximized ? SW_MAXIMIZE : SW_RESTORE);
		RedrawWindow(_window()->handle(), 0, 0, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_ERASE);
	}	
}

bool Window::maximized() const
{
	return (_window()->getStyle() & WS_MAXIMIZE) != 0;
}

void Window::setMenu(PopupMenu* menu)
{
	menu_ = menu;
	updateMenu();
}

PopupMenu* Window::menu()
{
	return menu_;
}

void Window::updateMenu()
{
	((WindowImpl*)_window())->setMenu(menu_);
}

bool Window::isVisible() const
{
	return window_->isVisible();
}

void Window::_updateVisibility()
{
	Widget::_updateVisibility();
	if(_visible()){
		if(!positioned_){
			reposition();
		}
		_arrangeChildren(); // FIXME: hack
		window_->show(SW_SHOW);
	}
	else
		window_->show(SW_HIDE);
}

void Window::setDefaultWidget(Widget* widget)
{
	if(widget && !widget->canBeDefault())
		return;

	if(defWidget_ && defWidget_ != widget)
		defWidget_->setDefaultFrame(false);

	if(widget)
		widget->setDefaultFrame(true);

	defWidget_ = widget;
}

void Window::showAll()
{
	Widget::showAll();	
	if(!focusedWidget_)
		_setFocus();
}


void Window::setTitle(const char* title)
{
	title_ = title;
	window_->setWindowText(toWideChar(title).c_str());
}

void Window::setShowTitleBar(bool showTitleBar)
{
	showTitleBar_ = showTitleBar;
	window_->setStyle(calculateStyle()); 
}


void Window::setIconFromResource(const char* resourceName)
{
	HICON icon = LoadIconW(Win32::_globalInstance(), toWideChar(resourceName).c_str());
	YASLI_ASSERT(icon);

	SetClassLongPtr(window_->handle(), GCLP_HICON, (LONG_PTR)icon);
}

void Window::setIconFromFile(const char* resourceName)
{
	HANDLE icon = LoadImageW(0, toWideChar(resourceName).c_str(), IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
	YASLI_ASSERT(icon);

	SetClassLongPtr(window_->handle(), GCLP_HICON, (LONG_PTR)icon);
}

void Window::add(Widget* widget)
{
	if(child_)
		remove();
	child_ = widget;
	widget->_setParent(this);
	_arrangeChildren();
}

void Window::remove()
{
	if(child_)
		child_->_setParent(0);
	child_ = 0;
}

void Window::reposition()
{
	int screenWidth = int(GetSystemMetrics(SM_CXSCREEN));
	int screenHeight = int(GetSystemMetrics(SM_CYSCREEN));
	
	int width = std::max(std::max(GetSystemMetrics(SM_CXMINSPACING), _minimalSize().x), defaultSize_.x);
	int height = std::max(_minimalSize().y, defaultSize_.y);
	if(resizeable_){
		width += GetSystemMetrics(SM_CXBORDER);
		height += GetSystemMetrics(SM_CYBORDER);
	}
	if(showTitleBar_){
		height += GetSystemMetrics(SM_CYCAPTION);
	}

    int left = 0;
    int top = 0;
    switch(windowPosition_) {
    case POSITION_LEFT_TOP:
        left = 0;
        top = 0;
        break;
    case POSITION_LEFT:
        left = 0;
        top = (screenHeight - height) / 2;
        break;
    case POSITION_LEFT_BOTTOM:
        left = 0;
        top = screenHeight - height;
        break;
    case POSITION_RIGHT_TOP:
        left = screenWidth - width;
        top = 0;
        break;
    case POSITION_RIGHT:
        left = screenWidth - width;
        top = (screenHeight - height) / 2;
        break;
    case POSITION_RIGHT_BOTTOM:
        left = screenWidth - width;
        top = screenHeight - height;
        break;
    case POSITION_TOP:
        left = (screenWidth - width) / 2;
        top = 0;
        break;
    case POSITION_BOTTOM:
        left = (screenWidth - width) / 2;
        top = screenHeight - height;
        break;
    case POSITION_CENTER:
        left = (screenWidth - width) / 2;
        top = (screenHeight - height) / 2;
        break;
    case POSITION_DEFAULT:
        left = 0;
        top = 0;
        break;
    case POSITION_CURSOR:
    {
		POINT point;
		GetCursorPos(&point);
		left = point.x;
		top = point.y;
		break;
    } 
    }
    Rect rect(left, top, left + width, top + height);

	::SetWindowPos(window_->handle(), 0, rect.left(), rect.top(), rect.width(), rect.height(), SWP_NOZORDER | SWP_NOACTIVATE | (visible_ ? SWP_SHOWWINDOW : SWP_HIDEWINDOW));
	positioned_ = true;
}

void Window::setDefaultPosition(WindowPosition position)
{
	windowPosition_ = position;
	positioned_ = false;
	reposition();
}

void Window::setDefaultSize(Vect2 size)
{
	defaultSize_ = size;
	positioned_ = false;
	reposition();
}

void Window::setDefaultSize(int w, int h)
{
	setDefaultSize(Vect2(w, h));
}

void Window::visitChildren(WidgetVisitor& visitor) const
{
	if(child_)
		visitor(*child_);
}

void Window::_arrangeChildren()
{
	if(child_){
		RECT rect = { 0, 0, 1, 1 };
		WW_VERIFY(::GetClientRect(window_->handle(), &rect));
		WW_VERIFY(::InflateRect(&rect, -border_, -border_));
		child_->_setPosition(Rect(rect.left, rect.top, rect.right, rect.bottom));
	}
}

void Window::_relayoutParents()
{
	if(child_)
		_setMinimalSize(child_->_minimalSize() + Vect2(border_ * 2, border_ * 2));
	else
		_setMinimalSize(border_ * 2, border_ * 2);

	RECT clientRect;
	::GetClientRect(window_->handle(), &clientRect);
	bool move = false;
	RECT windowRect;
	::GetWindowRect(window_->handle(), &windowRect);
	SIZE borderSize = { windowRect.right - windowRect.left - (clientRect.right - clientRect.left),
						windowRect.bottom - windowRect.top - (clientRect.bottom - clientRect.top) };

	if(clientRect.right - clientRect.left < _minimalSize().x){
		windowRect.right = windowRect.left + _minimalSize().x + borderSize.cx;
		move = true;
	}
	if(clientRect.bottom - clientRect.top < _minimalSize().y){
		windowRect.bottom = windowRect.top + _minimalSize().y + borderSize.cy;
		move = true;
	}
	if(move){
		window_->move(Rect(windowRect.left, windowRect.top, windowRect.right, windowRect.bottom));
		window_->update();
	}
	else
		::RedrawWindow(window_->handle(), 0, 0, RDW_INVALIDATE | RDW_ALLCHILDREN);

	__super::_relayoutParents();
}

void Window::onClose()
{
	hide();
	signalClose().emit();
}

void Window::serialize(Archive& ar)
{
	if(ar.filter(SERIALIZE_DESIGN)){
		//ar(_property(title_, this, &Window::setTitle), "title", "Title");
		ar(_property(defaultSize_, this, &Window::setDefaultSize), "defaultSize", "Default Size");
		ar(resizeable_, "resizeable", "Resizeable");
		if(resizeable_){
			ar(minimizeable_, "resizeable", "Minimizeable");
			ar(showTitleBar_, "showTitleBar", "Show Title Bar");
		}
		ar(child_, "widget", "Widget");
	}
    else if(ar.filter(SERIALIZE_STATE)){
		Rect position = restoredPosition();
		bool maximized = this->maximized();

		ar(position, "restoredPosition", "Restored Position");
		ar(maximized, "maximized", "Maximized");

		if(ar.isInput()){
			setRestoredPosition(position);
			setMaximized(maximized);
		}
        if(child_)
            ar(*child_, "widget", "Widget");
	}
}

void Window::_setFocusedWidget(Widget* widget)
{
	focusedWidget_ = widget;
	setDefaultWidget(widget);
}

}
