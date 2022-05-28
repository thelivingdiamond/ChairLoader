/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "PropertyTree/ConstStringList.h"
namespace yasli{
class Serializer;
}

class PropertyRow;
class PropertyTreeModel;

namespace ww{
struct PasteFunc{
    virtual bool operator()(const char* mem, size_t size) = 0;
};
class Widget;
class Clipboard{
public:
	Clipboard(Widget* owner, ConstStringList* constStrings = 0, PropertyTreeModel* model = 0, int filter = 0);
	~Clipboard();

	bool empty();
	bool copy(PropertyRow* row);
	bool copy(yasli::Serializer& se);
	bool paste(PropertyRow* row, bool onlyCheck = false);
	bool paste(yasli::Serializer& se);
	bool canBePastedOn(const char* destinationType);
	int smartPaste(yasli::Serializer& se);
protected:
	bool pasteFunc(PasteFunc& func);

	Widget* widget_;
	PropertyTreeModel* model_;
	ConstStringList* constStrings_;
	ConstStringList ownConstStrings_;
	unsigned int clipboardFormat_;
    int filter_;
};

};

