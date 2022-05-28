/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "ww/PopupMenu.h"
#include <algorithm>
#include <map>
#include "ww/Unicode.h"
#include "ww/Widget.h"
#include "ww/Win32/Window32.h"
#include <windows.h>

namespace ww{

PopupMenu::PopupMenu()
{
	nextId_ = 65535;
}

static PopupMenuItem* nextItem(PopupMenuItem* item)
{
    if(!item->children().empty())
        return item->children().front();
    else{
        PopupMenuItem0::Children& items = item->parent()->children();
        PopupMenuItem0::Children::iterator it = std::find(items.begin(), items.end(), item);
        YASLI_ASSERT(it != items.end());
        while(item->parent() && *it == item->parent()->children().back()){
            item = item->parent();
			if(item->parent())
				it = std::find(item->parent()->children().begin(), item->parent()->children().end(), item);
        }
        if(item->parent()){
            ++it;
            item = *it;
            return item;
        }
        else
            return 0;
    }
}

static const PopupMenuItem* nextItem(const PopupMenuItem* item)
{
	return nextItem(const_cast<PopupMenuItem*>(item));
}

void PopupMenu::assignIDs()
{
    DWORD currentID = nextId_;
	--nextId_;
    PopupMenuItem* current = &root_;
    while(current){
        if(current->children().empty()) {
            current->id_ = nextId_;
			if (nextId_ > 0) {
				--nextId_;
			}
			else {
				YASLI_ASSERT(0 && "Out of Menu IDs");
			}
		}
        else
            current->id_ = 0;

        current = nextItem(current);
    }
}

void PopupMenu::clear()
{
	root_.children().clear();
}

void PopupMenu::spawn(Widget* widget)
{
    POINT pt;
	GetCursorPos(&pt);
	spawn(Vect2(pt.x, pt.y), widget);
}

template<class TKey, class TValue>
TValue find_in_map(const std::map<TKey, TValue>& map, const typename std::map<TKey, TValue>::key_type& key, const typename std::map<TKey, TValue>::mapped_type& defaultValue)
{
	std::map<TKey, TValue>::const_iterator it = map.find(key);
	if (it == map.end())
		return defaultValue;
	else
		return it->second;
}

static HMENU createNativeMenu(const PopupMenuItem& root, bool popupMenu)
{
	HMENU handle = 0;
	if (popupMenu)
		handle = ::CreatePopupMenu();
	else
		handle = ::CreateMenu();

	std::map<const PopupMenuItem*, HMENU> submenuHandles;
	submenuHandles[&root] = handle;

	const PopupMenuItem* current = &root;
	while(current = nextItem(current)){
		if(current->children().empty()){
			UINT_PTR id = current->_menuID();
			wstring text = current->textW();

			HMENU parentHandle = find_in_map(submenuHandles, current->parent(), 0);
			YASLI_ASSERT(parentHandle != 0);

			if(text == L"-")
				AppendMenuW(parentHandle, MF_SEPARATOR, id, L"");
			else{
				if(current->hotkey() != KeyPress()){
					text += L"\t";
					text += toWideChar(current->hotkey().toString(true).c_str());
				}
				AppendMenuW(parentHandle, 
						   MF_STRING | (current->isChecked() ? MF_CHECKED : 0)
						   | (current->isEnabled() ? 0 : MF_GRAYED)
						   | (current->isDefault() ? MF_DEFAULT : 0),
						   id, text.c_str());			
			}
		}
		else{
			HMENU handle = ::CreatePopupMenu();
			submenuHandles[current] = handle;
			HMENU parentHandle = find_in_map(submenuHandles, current->parent(), 0);
			YASLI_ASSERT(parentHandle != 0);
			BOOL result = ::InsertMenuW(parentHandle, -1, MF_BYPOSITION | MF_POPUP, UINT_PTR(handle), current->textW());
			YASLI_ASSERT(result == TRUE);
		}
	}

	return handle;
}

void PopupMenu::spawn(Vect2 point, Widget* widget)
{
	if(root_.children().empty())
		return;

    assignIDs();

	HMENU menu = createNativeMenu(root_, true);

	Win32::Window32* parentWindow = 0;
	if (widget)
		parentWindow = _findWindow(widget);
	HWND parentWindowHandle =  parentWindow ? parentWindow->handle() : 0;
	YASLI_ASSERT(::IsWindow(parentWindowHandle));

	UINT id = ::TrackPopupMenu(menu, TPM_LEFTBUTTON | TPM_LEFTALIGN | TPM_RETURNCMD, point.x, point.y, 0, parentWindowHandle, 0);

	PopupMenuItem* item = root_.findById(id);
	if (item)
		item->_call();

	DestroyMenu(menu);
}

void* PopupMenu::_generateMenuBar()
{
	if (root_.children().empty())
		return 0;

	assignIDs();
	HMENU menu = createNativeMenu(root_, false);
	YASLI_ASSERT(menu != 0);
	return menu;
}

// ---------------------------------------------------------------------------

PopupMenuItem::~PopupMenuItem()
{
}

PopupMenuItem0& PopupMenuItem::add(const wchar_t* text)
{
	PopupMenuItem0* item = new PopupMenuItem0(text);
	addChildren(item);
	return *item;
}

PopupMenuItem0& PopupMenuItem::add(const char* text)
{
	return add(toWideChar(text).c_str());
}


PopupMenuItem& PopupMenuItem::setHotkey(KeyPress key)
{
    hotkey_ = key;
	return *this;
}

PopupMenuItem& PopupMenuItem::addSeparator()
{
	return add("-");
}

string PopupMenuItem::text() const
{ 
	return fromWideChar(text_.c_str()) ;
}

PopupMenuItem* PopupMenuItem::find(const wchar_t* text)
{
    Children::iterator it;
    FOR_EACH(children_, it){
        PopupMenuItem* item = *it;
        if(wcscmp(item->textW(), text) == 0)
            return item;
    };
    return 0;
}

PopupMenuItem* PopupMenuItem::find(const char* text)
{
	return find(toWideChar(text).c_str());
}

PopupMenuItem* PopupMenuItem::findById(unsigned int id)
{
	PopupMenuItem* current = this;
	while(current = nextItem(current)){
		if(current->children().empty()){
			UINT current_id = current->_menuID();
			if(current_id == id){
				return current;
			}
		}
	}
	return 0;
}

}
