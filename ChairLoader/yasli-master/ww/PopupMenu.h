/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "ww/Strings.h"
#include <vector>
#include "ww/API.h"
#include "ww/sigslot.h"
#include "ww/Vect2.h"
#include "ww/Unicode.h"
#include "yasli/Pointers.h"
#include "yasli/Assert.h"
#include "KeyPress.h"

struct HMENU__;
typedef HMENU__* HMENU;

namespace ww{
using std::vector;

class Widget;
class PopupMenu;
class PopupMenuItem0;
template<class Arg1> class PopupMenuItem1;
template<class Arg1, class Arg2> class PopupMenuItem2;
template<class Arg1, class Arg2, class Arg3> class PopupMenuItem3;

class PopupMenuItem: public RefCounter{
public:
	friend PopupMenu;
    typedef vector<SharedPtr<PopupMenuItem> > Children;

    PopupMenuItem(const wchar_t* text = L"")
    : id_(0)
	, parent_(0)
	, text_(text)
	, checked_(false)
	, enabled_(true)
	, default_(false)
    {}

	virtual ~PopupMenuItem();

	PopupMenuItem& check(bool checked = true){ checked_ = checked; return *this; }
	bool isChecked() const{ return checked_; }

	PopupMenuItem& enable(bool enabled = true){ enabled_ = enabled; return *this; }
	bool isEnabled() const{ return enabled_; }

	PopupMenuItem& setHotkey(KeyPress key);
	KeyPress hotkey() const{ return hotkey_; }

	void setDefault(bool defaultItem = true){ default_ = defaultItem; }
	bool isDefault() const{ return default_; }

    const wchar_t* textW() const { return text_.c_str(); }
    string text() const;
	PopupMenuItem& addSeparator();
    PopupMenuItem0& add(const char* text);
    PopupMenuItem0& add(const wchar_t* text);

    template<class Arg1>
    PopupMenuItem1<Arg1>& add(const char* text, const Arg1& arg1);
    template<class Arg1>
    PopupMenuItem1<Arg1>& add(const wchar_t* text, const Arg1& arg1);

    template<class Arg1, class Arg2>
    PopupMenuItem2<Arg1, Arg2>& add(const char* text, const Arg1& arg1, const Arg2& arg2);
    template<class Arg1, class Arg2>
    PopupMenuItem2<Arg1, Arg2>& add(const wchar_t* text, const Arg1& arg1, const Arg2& arg2);

    template<class Arg1, class Arg2, class Arg3>
    PopupMenuItem3<Arg1, Arg2, Arg3>& add(const char* text, const Arg1& arg1, const Arg2& arg2, const Arg3& arg3);
    template<class Arg1, class Arg2, class Arg3>
    PopupMenuItem3<Arg1, Arg2, Arg3>& add(const wchar_t* text, const Arg1& arg1, const Arg2& arg2, const Arg3& arg3);

	PopupMenuItem* find(const char* text);
	PopupMenuItem* find(const wchar_t* text);
	PopupMenuItem* findById(unsigned int id);

	PopupMenuItem* parent() { return parent_; }
	const PopupMenuItem* parent() const{ return parent_; }
	Children& children() { return children_; };
    const Children& children() const { return children_; }
	bool empty() const { return children_.empty(); }

	// internal methods:
    unsigned int _menuID() const{
        YASLI_ASSERT(children_.empty());
        return id_;
    }
	void _setMenuID(unsigned int id){
        YASLI_ASSERT(children_.empty());
		id_ = unsigned int(id);
	}
    virtual void _call() = 0;
private:
    void addChildren(PopupMenuItem* item){
        children_.push_back(item);
        item->parent_ = this;
    }



    unsigned int id_;

	bool default_;
	bool checked_;
	bool enabled_;
    wstring text_;
    PopupMenuItem* parent_;
    Children children_;
	KeyPress hotkey_;
};

class PopupMenuItem0 : public PopupMenuItem, public signal0{
public:
    PopupMenuItem0(const wchar_t* text = L"")
    : PopupMenuItem(text)
    {}

    void _call(){ emit(); }
	template<class desttype>
	PopupMenuItem0& connect(desttype* pclass, void (desttype::*pmemfun)()){
		signal0::connect(pclass, pmemfun);
		return *this;
	}
protected:
};


template<class Arg1>
class PopupMenuItem1 : public PopupMenuItem, public signal1<Arg1>{
public:
    PopupMenuItem1(const wchar_t* text, Arg1 arg1)
    : PopupMenuItem(text)
    , arg1_(arg1)
    {}

    void _call(){ emit(arg1_); }

	template<class desttype>
	PopupMenuItem1& connect(desttype* pclass, void (desttype::*pmemfun)(Arg1)){
		signal1<Arg1>::connect(pclass, pmemfun);
		return *this;
	}
protected:
    Arg1 arg1_;
};

template<class Arg1, class Arg2>
class PopupMenuItem2 : public PopupMenuItem, public signal2<Arg1, Arg2>{
public:
    PopupMenuItem2(const wchar_t* text, Arg1 arg1, Arg2 arg2)
    : PopupMenuItem(text)
    , arg1_(arg1)
    , arg2_(arg2)
    {}

    void _call(){ emit(arg1_, arg2_); }

	template<class desttype>
	PopupMenuItem2& connect(desttype* pclass, void (desttype::*pmemfun)(Arg1, Arg2)){
		signal2<Arg1, Arg2>::connect(pclass, pmemfun);
		return *this;
	}
protected:
    Arg1 arg1_;
    Arg2 arg2_;
};

template<class Arg1, class Arg2, class Arg3>
class PopupMenuItem3 : public PopupMenuItem, public signal3<Arg1, Arg2, Arg3>{
public:
    PopupMenuItem3(const wchar_t* text, Arg1 arg1, Arg2 arg2, Arg3 arg3)
    : PopupMenuItem(text)
    , arg1_(arg1)
    , arg2_(arg2)
    , arg3_(arg3)
    {}

    void _call(){ emit(arg1_, arg2_, arg3_); }

	template<class desttype>
	PopupMenuItem3& connect(desttype* pclass, void (desttype::*pmemfun)(Arg1, Arg2, Arg3)){
		signal3<Arg1, Arg2, Arg3>::connect(pclass, pmemfun);
		return *this;
	}
protected:
    Arg1 arg1_;
    Arg2 arg2_;
    Arg3 arg3_;
};

template<class Arg1>
PopupMenuItem1<Arg1>& PopupMenuItem::add(const wchar_t* text, const Arg1& arg1){
	PopupMenuItem1<Arg1>* item = new PopupMenuItem1<Arg1>(text, arg1);
	addChildren(item);
	return *item;
}
template<class Arg1>
PopupMenuItem1<Arg1>& PopupMenuItem::add(const char* text, const Arg1& arg1){
	return add(toWideChar(text).c_str(), arg1);
}

template<class Arg1, class Arg2>
PopupMenuItem2<Arg1, Arg2>& PopupMenuItem::add(const wchar_t* text, const Arg1& arg1, const Arg2& arg2){
	PopupMenuItem2<Arg1, Arg2>* item = new PopupMenuItem2<Arg1, Arg2>(text, arg1, arg2);
	addChildren(item);
	return *item;
}
template<class Arg1, class Arg2>
PopupMenuItem2<Arg1, Arg2>& PopupMenuItem::add(const char* text, const Arg1& arg1, const Arg2& arg2){
	return add(toWideChar(text).c_str(), arg1, arg2);
}

template<class Arg1, class Arg2, class Arg3>
PopupMenuItem3<Arg1, Arg2, Arg3>& PopupMenuItem::add(const wchar_t* text, const Arg1& arg1, const Arg2& arg2, const Arg3& arg3){
	PopupMenuItem3<Arg1, Arg2, Arg3>* item = new PopupMenuItem3<Arg1, Arg2, Arg3>(text, arg1, arg2, arg3);
	addChildren(item);
	return *item;
}
template<class Arg1, class Arg2, class Arg3>
PopupMenuItem3<Arg1, Arg2, Arg3>& PopupMenuItem::add(const char* text, const Arg1& arg1, const Arg2& arg2, const Arg3& arg3){
	return add(toWideChar(text).c_str(), arg1, arg2, arg3);
}



class PopupMenu : public RefCounter{
public:
    PopupMenu();
    PopupMenuItem0& root() { return root_; };
    const PopupMenuItem0& root() const { return root_; };
    void spawn(Widget* widget);
    void spawn(Vect2 screenPoint, Widget* widget);
	void clear();

	// internal methods:
	// generates HMENU, used by ww::Window, caller is responsible to call DestroyMenu
	void* _generateMenuBar();
private:
    PopupMenuItem0 root_;

    void assignIDs();

    unsigned int nextId_;
};

}

