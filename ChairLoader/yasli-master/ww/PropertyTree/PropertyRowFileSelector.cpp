/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "ww/Serialization.h"
#include "ww/PropertyTree.h"
#include "PropertyTree/PropertyTreeModel.h"
#include "PropertyTree/IDrawContext.h"
#include "PropertyTree/IUIFacade.h"
#include "PropertyTree/IMenu.h"
#include "PropertyTree/PropertyRowImpl.h"
#include "ww/FileDialog.h"
#include "ww/PopupMenu.h"
#include "ww/Files.h"
#include "yasli/ClassFactory.h"
#include "ww/FileSelector.h"
#include "ww/Win32/Drawing.h"
#include "ww/Win32/Window32.h"
#include "ww/Unicode.h"
#include "gdiplusUtils.h"

namespace ww{

class PropertyRowFileSelector : public PropertyRowImpl<FileSelector>, public has_slots{
	bool locked_;
public:
	PropertyRowFileSelector() : locked_(false) {}
	bool activateOnAdd() const{ return true; }
	bool onActivate(::PropertyTree* tree, bool force) override;
	bool onContextMenu(property_tree::IMenu& root, PropertyTree* tree);
	void onMenuClear(PropertyTreeModel* model);

	void redraw(IDrawContext& context) override
	{
		if(multiValue())
			context.drawEntry(context.widgetRect, " ... ", false, true, 0);
		else if(userReadOnly())
			context.drawValueText(pulledSelected(), valueAsString().c_str());
		else
			context.drawEntry(context.widgetRect, valueAsString().c_str(), true, false, 0);
	}


	yasli::string valueAsString() const override{ return value().c_str(); }

	void serializeValue(yasli::Archive& ar) override{
		string fileName = value_.c_str();
		ar(fileName, "value", "Value");
		if(ar.isInput())
			value_ = fileName.c_str();
	}
};

bool PropertyRowFileSelector::onActivate(::PropertyTree* tree, bool force)
{
	if(locked_)
		return false;
	locked_ = true;

	FileSelector& selector = value();
	const FileSelector::Options* options = selector.options();
	string title = string("(") + options->filter + ")";
	const char* masks[] = { title.c_str(), selector.options()->filter.c_str(), 0 };

	string root = Files::fixSlashes(options->rootDirectory.c_str());
	string fileName = selector.c_str();
    if(root == ".")
        root = Files::currentDirectory();
	if(!root.empty() && fileName.find('\\') == string::npos)
    {
        if(!fileName.empty())
		    fileName = root[root.size() - 1] == '\\' ? root + fileName : root + '\\' + fileName;
    }

	ww::PropertyTree* wwTree = safe_cast<ww::PropertyTree*>(tree);
	ww::FileDialog fileDialog(wwTree, options->save, masks, root.c_str(), fileName.c_str());
	if(fileDialog.showModal()){
        tree->model()->rowAboutToBeChanged(this);
		if(!root.empty()){
			string relativePath = Files::relativePath(fileDialog.fileName(), root.c_str());;
			if(!relativePath.empty())
				selector = relativePath.c_str();
			else
				selector = fileDialog.fileName();
		}
		else
			selector = fileDialog.fileName();
		tree->model()->rowChanged(this);
		locked_ = false;
		return true;
	}
	else{
		locked_ = false;
		return false;
	}
}

struct FileSelectorMenuHandler : PropertyRowMenuHandler
{
	FileSelectorMenuHandler(PropertyRowFileSelector* row, PropertyTreeModel* model) : row(row), model(model) {}
	
	void onMenuClear()
	{
		row->onMenuClear(model);
	}

	PropertyRowFileSelector* row;
	PropertyTreeModel* model;
};

bool PropertyRowFileSelector::onContextMenu(property_tree::IMenu& root, PropertyTree* tree)
{
	if(!root.isEmpty())
		root.addSeparator();

	FileSelectorMenuHandler* handler = new FileSelectorMenuHandler(this, tree->model());
	root.addAction(TRANSLATE("Clear"))->signalTriggered.connect(handler, &FileSelectorMenuHandler::onMenuClear);
	tree->addMenuHandler(handler);
	return __super::onContextMenu(root, tree);
}

void PropertyRowFileSelector::onMenuClear(PropertyTreeModel* model)
{
    model->rowAboutToBeChanged(this);
	value() = "";
	model->rowChanged(this);
}

DECLARE_SEGMENT(PropertyRowFileSelector)
REGISTER_PROPERTY_ROW(FileSelector, PropertyRowFileSelector);

}
