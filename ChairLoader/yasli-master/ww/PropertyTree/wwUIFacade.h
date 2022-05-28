#pragma once 
#include "ww/PropertyTree.h"
#include "PropertyTree/IMenu.h"
#include "PropertyTree/IUIFacade.h"
#include "ww/PopupMenu.h"
#include "ww/Win32/Window32.h"
#include <Windows.h>

namespace property_tree {

class wwMenuAction : public IMenuAction, public sigslot::has_slots
{
public:
	wwMenuAction(ww::PopupMenuItem0* item) : item_(item) {
		item_->connect(this, &wwMenuAction::onActivate);
	}

private:
	void onActivate()
	{
		signalTriggered.emit();
	}

	ww::PopupMenuItem0* item_;
};

class wwMenu : public IMenu
{
public:
	wwMenu(ww::Widget* tree)
	: menu_(new ww::PopupMenu())
	, tree_(tree)
	{
		menuItem_ = &menu_->root();
	}
	wwMenu(ww::Widget* tree, ww::PopupMenuItem* item)
	: menuItem_(item)
    , tree_(tree) {}
	~wwMenu() {
		for (size_t i = 0; i < ownedMenus_.size(); ++i)
			delete ownedMenus_[i];
		ownedMenus_.clear();
		for (size_t i = 0; i < ownedActions_.size(); ++i)
			delete ownedActions_[i];
		ownedActions_.clear();
	}

	bool isEmpty() override { return menuItem_->empty(); }
	IMenu* addMenu(const char* text) override {
		wwMenu* result = new wwMenu(tree_, &menuItem_->add(text));
		ownedMenus_.push_back(result);
		return result;
	}
	IMenu* findMenu(const char* text) {
		ww::PopupMenuItem* item = menuItem_->find(text);
		if (!item)
			return 0;
		wwMenu* result = new wwMenu(tree_, (ww::PopupMenuItem0*)item);
		ownedMenus_.push_back(result);
		return result;
	}
	void addSeparator() { menuItem_->addSeparator(); }
	IMenuAction* addAction(const char* text, int flags = 0) {
		ww::PopupMenuItem0& item = menuItem_->add(text);
		if (flags & MENU_DISABLED)
			item.enable(false);
		if (flags & MENU_DEFAULT)
			item.setDefault(true);
		wwMenuAction* result = new wwMenuAction(&item);
		ownedActions_.push_back(result);
		return result;
	}
	void exec(const Point& point) {
		if (menu_.get()) {
			::RECT rt;
			GetWindowRect(tree_->_window()->handle(), &rt);
			menu_->spawn(ww::Vect2(point.x() + rt.left, point.y() + rt.top), tree_);
		}
	}

private:
	ww::Widget* tree_;
	ww::PopupMenuItem* menuItem_;
	std::vector<wwMenu*> ownedMenus_;
	std::vector<wwMenuAction*> ownedActions_;
	std::auto_ptr<ww::PopupMenu> menu_;
};

class wwUIFacade : public IUIFacade
{
public:
	wwUIFacade(ww::PropertyTree* tree) : tree_(tree) {}

	HWND hwnd() override;
	
	IMenu* createMenu() override;
	void setCursor(Cursor cursor) override;
	void unsetCursor() override;
	Point cursorPosition() override;
	int textWidth(const char* text, Font font) override;
	Point screenSize() override;

	InplaceWidget* createComboBox(ComboBoxClientRow* client) override;
	InplaceWidget* createNumberWidget(PropertyRowNumberField* row) override;
	InplaceWidget* createStringWidget(PropertyRowString* row) override;
private:
	ww::PropertyTree* tree_;
};

}
