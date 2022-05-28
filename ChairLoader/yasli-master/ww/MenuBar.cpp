/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "ww/MenuBar.h"
#include "ww/PopupMenu.h"
#include "ww/_WidgetWindow.h"
#include "ww/Win32/Rectangle.h"
#include "ww/Win32/Window32.h"
#include "ww/Win32/Drawing.h"
#include "ww/Unicode.h"
#include "ww/HotkeyContext.h"
#include "ww/Window.h"
#include <algorithm>

namespace ww{

MenuItem& MenuItem::add(const char* text)
{
    const char* end = text + strlen(text);
    const char* p = text;

    while(p != end){
        if(*p == '\\'){
            break;
        }
        ++p;
    }

    string name(text, p);
    MenuItem* subItem = findSubItem(name.c_str());
    if(!subItem || name == "-"){
        subItems_.push_back(new MenuItem(name.c_str()));
        subItem = subItems_.back().get();
    }

    if(p == end)
        return *subItem;
    else{
        ++p;
        return subItem->add(p);
    }
}

void MenuItem::addSeparator()
{
	add("-");
}

MenuItem* MenuItem::findSubItem(const char* name)
{
    Items::iterator it;
    for(it = subItems_.begin(); it != subItems_.end(); ++it){
        if(strcmp(it->get()->text(), name) == 0)
            return it->get();
    }
    return 0;
}

void MenuItem::registerHotkeys(HotkeyContext* context)
{
    YASLI_ESCAPE(context, return);
    context->signalPressed(hotkey()).connect(this, &MenuItem::activate);

    for(Items::iterator it = subItems_.begin(); it != subItems_.end(); ++it){
        it->get()->registerHotkeys(context);
    }
}
// ---------------------------------------------------------------------------

class MenuBarItem{
public:
    MenuBarItem()
    {
    }

    MenuBarItem(MenuItem* item, const Rect& rect)
    : item_(item)	
    , rect_(rect)
    {	

    }

    void setRect(Rect& rect){
        rect_ = rect;
    }
    const Rect& rect() const{ return rect_; }
    MenuItem* item(){ return item_; }
protected:
    SharedPtr<MenuItem> item_;
    Rect rect_;
};

class MenuBarImpl : public _WidgetWindow, public has_slots
{
public:
    MenuBarImpl(ww::MenuBar* owner);
    virtual ~MenuBarImpl();
    void redraw(HDC dc);

    LRESULT onMessage(UINT message, WPARAM wparam, LPARAM lparam);
    void onMessagePaint();
    void onMessageMouseMove(UINT button, int x, int y);
    void onMessageLButtonDown(UINT button, int x, int y);
    void onMessageLButtonUp(UINT button, int x, int y);
    void onMessageRButtonDown(UINT button, int x, int y);

    void onItemActivate(MenuItem* item);
    void generateMenu(PopupMenuItem* root, MenuItem& rootItem);    
    void updateRootItems();
    typedef std::vector<MenuBarItem> MenuBarItems;

protected:
    bool updateActiveItem();
    MenuItem root_;
    MenuBarItems rootItems_;
    MenuBar* owner_;
    int activeItem_;
    bool showMenu_;
    bool inMenu_;
    friend class MenuBar;
};

MenuBarImpl::MenuBarImpl(ww::MenuBar* owner)
: _WidgetWindow(owner)
, owner_(owner)
, showMenu_(false)
, inMenu_(false)
, activeItem_(-1)
, root_("")
{
    create(L"", WS_CHILD, Rect(0, 0, 24, 24), Win32::getDefaultWindowHandle());
}

MenuBarImpl::~MenuBarImpl()
{

}

void MenuBarImpl::redraw(HDC dc)
{
    RECT rect;
    GetClientRect(handle(), &rect);
    FillRect(dc, &rect, GetSysColorBrush(COLOR_BTNFACE));

    MenuBarItems::iterator it;
    int index = 0;
    for(it = rootItems_.begin(); it != rootItems_.end(); ++it){
        MenuBarItem& item = *it;
        HFONT font = Win32::defaultFont();
        HFONT oldFont = (HFONT)::SelectObject(dc, font);
        int oldBkMode = ::SetBkMode(dc, TRANSPARENT);
        Win32::Rect rect(item.rect());
        wstring text = toWideChar(item.item()->text());
        if(activeItem_ == index){
            ::FillRect(dc, &rect, GetSysColorBrush(COLOR_HIGHLIGHT));
            COLORREF oldTextColor = ::SetTextColor(dc, GetSysColor(COLOR_HIGHLIGHTTEXT));
            ::DrawTextW(dc, text.c_str(), (int)wcslen(text.c_str()), &rect, DT_VCENTER | DT_SINGLELINE | DT_CENTER);
            ::SetTextColor(dc, oldTextColor);
        }
        else{
            COLORREF oldTextColor = ::SetTextColor(dc, GetSysColor(COLOR_MENUTEXT));
            ::DrawTextW(dc, text.c_str(), (int)wcslen(text.c_str()), &rect, DT_VCENTER | DT_SINGLELINE | DT_CENTER);
            ::SetTextColor(dc, oldTextColor);
        }
        ::SetBkMode(dc, oldBkMode);
        ::SelectObject(dc, oldFont);
        ++index;
    }
}

LRESULT MenuBarImpl::onMessage(UINT message, WPARAM wparam, LPARAM lparam)
{
    LRESULT result = __super::onMessage(message, wparam, lparam);
    if(message == WM_MOUSELEAVE){
        if(updateActiveItem())
            RedrawWindow(handle(), 0, 0, RDW_UPDATENOW | RDW_INVALIDATE);
    }
    else if(message == WM_ENTERIDLE){
        UINT reason = (UINT)wparam;
        HANDLE handle = (HANDLE)lparam;
        if(reason == MSGF_MENU){
            if(!showMenu_)
                return result;
            if(updateActiveItem()){
                RedrawWindow(this->handle(), 0, 0, RDW_UPDATENOW | RDW_INVALIDATE);
                inMenu_ = true;
                SendMessageW(this->handle(), WM_CANCELMODE, 0, 0);
            }
            return 0;
        }
    }
    return result;
}

void MenuBarImpl::onMessagePaint()
{
    PAINTSTRUCT ps;
    HDC dc = BeginPaint(handle(), &ps);
    redraw(dc);
    EndPaint(handle(), &ps);
}

void MenuBarImpl::onMessageMouseMove(UINT button, int x, int y)
{
    if(updateActiveItem())
        RedrawWindow(handle(), 0, 0, RDW_UPDATENOW | RDW_INVALIDATE);
    HWND captureWindow = GetCapture();
    YASLI_ASSERT(captureWindow == 0);
    if(activeItem_ != -1){
        TRACKMOUSEEVENT event;
        ZeroMemory((void*)(&event), sizeof(event));
        event.cbSize = sizeof(event);
        event.dwFlags = TME_LEAVE;
        event.hwndTrack = handle();
        WW_VERIFY(TrackMouseEvent(&event));
    }

    __super::onMessageMouseMove(button, x, y);
}

void MenuBarImpl::onMessageLButtonUp(UINT button, int x, int y)
{
}

void MenuBarImpl::generateMenu(PopupMenuItem* root, MenuItem& rootItem)
{
    MenuItem::Items::iterator it;
	for(it = rootItem.subItems_.begin(); it != rootItem.subItems_.end(); ++it){
        MenuItem &item = *it->get();

        if(item.text_ == "-"){
			root->addSeparator();
            continue;
        }

		string label = item.text_;
		if(item.hotkey_ != KeyPress()){
			label += "\t";
			label += item.hotkey().toString(true);
		}
        item.signalUpdate().emit(&item);
		PopupMenuItem1<MenuItem*>& popupItem = root->add(label.c_str(), &item);
		popupItem.enable(item.isEnabled());
        popupItem.connect(this, &MenuBarImpl::onItemActivate);
		generateMenu(&popupItem, item);
	}
}

void MenuBarImpl::onMessageLButtonDown(UINT button, int x, int y)
{
    updateActiveItem();
    showMenu_ = true;
    do{
        inMenu_ = false;
        if(activeItem_ != -1){
            MenuBarItem& item = rootItems_.at(activeItem_);
            PopupMenu menu;
            generateMenu(&menu.root(), *item.item());
            POINT pt = { item.rect().left(), item.rect().bottom() };
            ClientToScreen(handle(), &pt);
            RedrawWindow(handle(), 0, 0, RDW_UPDATENOW | RDW_INVALIDATE);
            menu.spawn(Vect2(pt.x, pt.y), owner_);
            activeItem_ = -1;
            updateActiveItem();
            RedrawWindow(handle(), 0, 0, RDW_UPDATENOW | RDW_INVALIDATE);
        }
    } while(inMenu_);
    showMenu_ = false;
}

void MenuBarImpl::onMessageRButtonDown(UINT button, int x, int y)
{
    __super::onMessageRButtonDown(button, x, y);
}

void MenuBarImpl::onItemActivate(MenuItem* item)
{
    YASLI_ESCAPE(item, return);

	// get rid of highlighted menu item
	activeItem_ = -1;
	InvalidateRect(handle(), 0, FALSE);

    item->signalActivate().emit();
}

void MenuBarImpl::updateRootItems()
{
    Rect pos(owner_->_position());
	Vect2 size = Win32::calculateTextSize(handle(), Win32::defaultFont(), L" ");
    size.y +=  + GetSystemMetrics(SM_CYEDGE) * 4;
    int height = std::max(size.y, owner_->_position().height());
    rootItems_.clear();
    MenuItem::Items::iterator it;
    int offset = 0;
    for(it = root_.subItems_.begin(); it != root_.subItems_.end(); ++it){
        MenuItem& item = *it->get();
        const wstring text = toWideChar(item.text_.c_str());
        Vect2 textSize = Win32::calculateTextSize(Win32::getDefaultWindowHandle(), Win32::defaultFont(), text.c_str());
        textSize.x += 10;
        Rect rect(offset, (pos.height() - height) / 2, offset + textSize.x, (pos.height() - height) / 2 + height);
        rootItems_.push_back(MenuBarItem(&item, rect));

        offset += textSize.x;
    }
    size.x = offset + GetSystemMetrics(SM_CXEDGE) * 2;
    owner_->_setMinimalSize(size);
}

bool MenuBarImpl::updateActiveItem()
{
    POINT cursorPos;
    GetCursorPos(&cursorPos);
    ScreenToClient(handle(), &cursorPos);

    MenuBarItems::iterator it;
    int index = 0;
    for(it = rootItems_.begin(); it != rootItems_.end(); ++it){
        MenuBarItem& item = *it;
        Win32::Rect rect(item.rect());
        if(::PtInRect(&rect, cursorPos)){
            bool result = activeItem_ != index;
            activeItem_ = index;
            return result;
        }
        ++index;
    }
    if(showMenu_)
        return false;
    else{
        bool result = activeItem_ != -1;
        activeItem_ = -1;
        return result;
    }
}

// -----------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4355) //  'this' : used in base member initializer list
MenuBar::MenuBar(int border)
: _WidgetWithWindow(new MenuBarImpl(this), border)
{
}
#pragma warning(pop)

MenuBar::~MenuBar()
{
}

MenuItem& MenuBar::add(const char* text)
{
    MenuItem& result = impl()->root_.add(text);
    impl()->updateRootItems();
    return result;
}


void MenuBar::registerHotkeys(Window* mainWindow)
{
    impl()->root_.registerHotkeys(mainWindow->_hotkeyContext());
}

void MenuBar::_setPosition(const Rect& position)
{
    __super::_setPosition(position);
    impl()->updateRootItems();
}

void MenuBar::_setParent(Container* container)
{
    __super::_setParent(container);
    impl()->updateRootItems();
}

void MenuBar::_updateVisibility()
{
    __super::_updateVisibility();
    impl()->updateRootItems();
}

MenuBarImpl* MenuBar::impl()
{
    return static_cast<MenuBarImpl*>(_window());
}

};
// vim:ts=4 sw=4:
