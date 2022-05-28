/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "yasli/ClassFactory.h"

#include "PropertyRowFileOpen.h"
#include "PropertyTree/IDrawContext.h"
#include "PropertyTree/PropertyRowImpl.h"
#include "PropertyTree/PropertyTreeModel.h"
#include "PropertyTree/Serialization.h"
#include "PropertyTree/PropertyTree.h"
#include "PropertyTree/IUIFacade.h"
#include "PropertyTree/IMenu.h"
#include "yasli/decorators/FileOpen.h"
#include "yasli/decorators/IconXPM.h"
#include <QFileDialog>
#include <QIcon>

using std::string;

static const char* getExtension(const char* path)
{
	const char* ext = strrchr(path, '.');
	if (!ext)
		return path + strlen(path);
	return ext;
}

static string removeExtension(const char* filename)
{
	const char* ext = getExtension(filename);
	return string(filename, ext);
}

string extractExtensionFromFilter(const char* mask)
{
	const char* start = strchr(mask, '(');
	const char* end = strchr(mask, ')');
	if (!start || !end)
		return string();
	++start;
	const char* ext = strchr(mask, '.');
	if (!ext)
		return string();
	if (ext > end)
		return string();
	++ext;
	const char* ext_end = ext;
	while (*ext_end && *ext_end != ' ' && *ext_end != ')' && ext_end < end) {
		if (*ext_end == '*' || *ext_end == '?')
			return string();
		++ext_end;
	}
	if (ext_end != end)
		++ext_end;
	return string(ext, ext_end);
}

using yasli::FileOpen;

class PropertyRowFileOpen : public PropertyRowImpl<FileOpen>{
public:

	bool isLeaf() const override{ return true; }

	bool onActivate(PropertyTree* tree, bool force) override
	{
		QFileDialog dialog(tree->ui()->qwidget());
		dialog.setAcceptMode(QFileDialog::AcceptOpen);
		dialog.setFileMode(QFileDialog::ExistingFile);
		if (labelUndecorated())
			dialog.setWindowTitle(QString("Choose file for '") + labelUndecorated() + "'");		
		
		QDir directory(value().relativeToFolder.c_str());
		dialog.setNameFilter(value().filter.c_str());
		
		QString existingFile = value().path.c_str();
        if (!QDir::isAbsolutePath(existingFile)) {
            existingFile = directory.currentPath();
            if (!value().relativeToFolder.empty()) {
                existingFile += QDir::separator();
                existingFile += value().relativeToFolder.c_str();
            }
            existingFile += QDir::separator();
            existingFile += value().path.c_str();
        }
        if (value().flags & FileOpen::STRIP_EXTENSION)
            existingFile += QString(extractExtensionFromFilter(value().filter.c_str()).c_str());

		if (!QFile::exists(existingFile))
			dialog.setDirectory(directory);
		else
            dialog.selectFile(existingFile);

		if (dialog.exec() && !dialog.selectedFiles().isEmpty()) {
			tree->model()->rowAboutToBeChanged(this);
			QString filename = dialog.selectedFiles()[0];
			QString relativeFilename = directory.relativeFilePath(filename);
			value().path = relativeFilename.toLocal8Bit().data();
			if (value().flags & FileOpen::STRIP_EXTENSION)
				value().path = removeExtension(value().path.c_str());
			tree->model()->rowChanged(this);
		}
		return true;
	}

	void clear(PropertyTree* tree)
	{
		tree->model()->rowAboutToBeChanged(this);
		value().path.clear();
		tree->model()->rowChanged(this);
	}

	bool onContextMenu(IMenu &menu, PropertyTree* tree) override
	{
		FileOpenMenuHandler* handler = new FileOpenMenuHandler(this, tree);
		tree->addMenuHandler(handler);

		menu.addAction("Choose File...", MENU_DEFAULT, handler, &FileOpenMenuHandler::onMenuActivate);
		menu.addAction("Clear", 0, handler, &FileOpenMenuHandler::onMenuClear);

		return PropertyRow::onContextMenu(menu, tree);
	}

	int buttonCount() const override{ return 1; }
	virtual yasli::IconXPM buttonIcon(const PropertyTree* tree, int index) const override{ 
		#include "PropertyTree/file_open.xpm"
		return yasli::IconXPM(file_open_xpm);
	}
	virtual bool usePathEllipsis() const override{ return true; }

	yasli::string valueAsString() const override{ return value().path; }
};

REGISTER_PROPERTY_ROW(FileOpen, PropertyRowFileOpen); 
DECLARE_SEGMENT(PropertyRowFileOpen)

// ---------------------------------------------------------------------------

void FileOpenMenuHandler::onMenuActivate()
{
	self->onActivate(tree, false);
}

void FileOpenMenuHandler::onMenuClear()
{
	self->clear(tree);
}

