/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "yasli/Pointers.h"
#include "ww/Clipboard.h"
#include "ww/Widget.h"
#include "PropertyTree/PropertyRow.h"
#include "PropertyTree/PropertyRowContainer.h"
#include "PropertyTree/PropertyRowPointer.h"
#include "PropertyTree/PropertyTreeModel.h"
#include "PropertyTree/PropertyOArchive.h"
#include "PropertyTree/PropertyIArchive.h"
#include "ww/PropertyEditor.h"
#include "ww/Win32/Window32.h"
#include "ww/SafeCast.h"

#include "yasli/ClassFactory.h"
#include "yasli/Serializer.h"
#include "yasli/BinArchive.h"

#include <windows.h>


namespace ww{

struct PasteSerializerFunc : PasteFunc
{
    PasteSerializerFunc(Serializer& ser, const char* name, const char* label)
    : ser_(ser) , name_(name) , label_(label) {}
    bool operator()(const char* mem, size_t memSize)
    {
        BinIArchive ia;
        if(ia.open(mem, memSize))
            return ia(ser_, name_, label_);
        return false;
    }

    Serializer& ser_;
    const char* name_;
    const char* label_;
};

struct PasteRowFunc : PasteFunc
{
    PasteRowFunc(SharedPtr<PropertyRow>* row)
    : row_(row){}
    bool operator()(const char* mem, size_t memSize)
    {
        BinIArchive ia;
        if(ia.open(mem, memSize))
            return ia(*row_, "row", "Row");
        return false;
    }

    SharedPtr<PropertyRow>* row_;
};

// ---------------------------------------------------------------------------
const DWORD CLIPBOARD_HEADER = 'v000';

Clipboard::Clipboard(Widget* owner, ConstStringList* constStrings, PropertyTreeModel* model, int filter)
: widget_(owner)
, constStrings_(constStrings ? constStrings : &ownConstStrings_)
, model_(model)
, filter_(filter)
{
	clipboardFormat_ = RegisterClipboardFormatW(L"ww.Clipboard0");
	YASLI_ASSERT(clipboardFormat_ != 0);
}

Clipboard::~Clipboard()
{

}

bool Clipboard::copy(yasli::Serializer& se)
{
	PropertyRow::setConstStrings(constStrings_);
	PropertyTreeModel model(0);
	PropertyOArchive oa(model_ ? model_ : &model);
    oa.setFilter(filter_);
	oa(se, "row", "Row");
	bool result = copy(model.root());
	PropertyRow::setConstStrings(0);
	return result;
}

bool Clipboard::paste(yasli::Serializer& se)
{
	ConstStringList constStrings;
	PropertyRow::setConstStrings(constStrings_ ? constStrings_ : &constStrings);
	PropertyTreeModel model(0);
	PropertyOArchive oa(model_ ? model_ : &model);
    oa.setFilter(filter_);
	se(oa);
	bool result = false;
	PropertyRow* dest = model.root();
	if(paste(dest)){
		PropertyIArchive ia(&model);
        ia.setFilter(filter_);
		se(ia);
		result = true;
	}
	PropertyRow::setConstStrings(0);
	return result;
}


bool Clipboard::canBePastedOn(const char* destinationType)
{
	PropertyRow::setConstStrings(constStrings_);

	SharedPtr<PropertyRow> retrievedRow;
	if(!pasteFunc(PasteRowFunc(&retrievedRow)) || !retrievedRow){
		PropertyRow::setConstStrings(0);
		return false;
	}
	bool result = strcmp(retrievedRow->typeName(), destinationType) == 0;
	PropertyRow::setConstStrings(0);
	return result;
	return false;
}


bool Clipboard::empty()
{
	YASLI_ASSERT(widget_);
	Win32::Window32* window = ww::_findWindow(widget_);
	YASLI_ASSERT(window);

	if(!::OpenClipboard(window->handle()))
		return false;

	bool result = false;
	HGLOBAL memoryHandle = (HGLOBAL)(::GetClipboardData(clipboardFormat_));
	if(memoryHandle){
		if(char* mem = (char*)GlobalLock(memoryHandle)){
            result = GlobalSize(memoryHandle) > 0;
			GlobalUnlock(memoryHandle);
		}
	}
	::CloseClipboard();
	return !result;
}

bool Clipboard::paste(PropertyRow* dest, bool onlyCheck)
{
	SharedPtr<PropertyRow> source;
	if(!pasteFunc(PasteRowFunc(&source)) || !source){
		PropertyRow::setConstStrings(0);
		return false;
	}
	
	bool result = false;
	if(strcmp(dest->typeName(), source->typeName()) == 0 && 
		// FIXME: немножко хак, т.к. vector<vector<> > уже может работать 
		// неправильно, нужно сделать нормальные имена типов для контейнеров
		source->isContainer() == dest->isContainer()){
		result = true;
		if(!onlyCheck){
			if(dest->isPointer() && !source->isPointer()){
				PropertyRowPointer* d = safe_cast<PropertyRowPointer*>(dest);

				const char* derivedName = d->typeName();
				const char* derivedNameAlt = d->typeName();
				SharedPtr<PropertyRow> newSourceRoot = d->clone(constStrings_);
				source->swapChildren(newSourceRoot);
				source = newSourceRoot;
			}
			const char* name = dest->name();
			const char* nameAlt = dest->label();
			source->setName(name);
			source->setLabel(nameAlt);
			if(dest->parent())
				dest->parent()->replaceAndPreserveState(dest, source, false);
			else{
				dest->clear();
				dest->swapChildren(source);
			}
		}
	}
	else if(dest->isContainer()){
		if(model_){
			PropertyRowContainer* container = static_cast<PropertyRowContainer*>(dest);
			PropertyRow* elementRow = model_->defaultType(container->elementTypeName());
			YASLI_ESCAPE(elementRow, return false);
			if(strcmp(container->elementTypeName(), source->typeName()) == 0){
				result = true;
				if(!onlyCheck){
					PropertyRow* dest = elementRow;
					if(dest->isPointer() && !source->isPointer()){
						PropertyRowPointer* d = safe_cast<PropertyRowPointer*>(dest);

						const char* derivedName = d->typeName();
						const char* derivedNameAlt = d->typeName();
						SharedPtr<PropertyRow> newSourceRoot = d->clone(constStrings_);
						source->swapChildren(newSourceRoot);
						source = newSourceRoot;
					}

					container->add(source.get());
				}
			}
		}
	}
	return result;
}

bool Clipboard::copy(PropertyRow* row)
{
	YASLI_ASSERT(widget_);
	Win32::Window32* window = ww::_findWindow(widget_);
	YASLI_ASSERT(window);

    SharedPtr<PropertyRow> clonedRow(row->clone(constStrings_));
	BinOArchive oa;
	if(!oa(clonedRow, "row", "Row")){
		PropertyRow::setConstStrings(0);
		return false;
	}

	if(::OpenClipboard(window->handle())){
		HGLOBAL memoryHandle = GlobalAlloc(GPTR, oa.length());
		YASLI_ASSERT(memoryHandle);
		if(!memoryHandle){
			PropertyRow::setConstStrings(0);
			return false;
		}
		
		void* mem = GlobalLock(memoryHandle);
		if(!mem){
			YASLI_ASSERT(0 && "GlobalLock failed!");
			PropertyRow::setConstStrings(0);
			return false;
		}
		CopyMemory(mem, oa.buffer(), oa.length());
		GlobalUnlock(memoryHandle);

		WW_VERIFY(::SetClipboardData(clipboardFormat_, memoryHandle));
		::CloseClipboard();
		return true;
	}
	return false;
}

bool Clipboard::pasteFunc(PasteFunc& func)
{
	YASLI_ASSERT(widget_);
	Win32::Window32* window = ww::_findWindow(widget_);
	YASLI_ASSERT(window);

	if(!::OpenClipboard(window->handle()))
		return false;

    HGLOBAL memoryHandle = (HGLOBAL)(::GetClipboardData(clipboardFormat_));
    if(!memoryHandle)
    {
        ::CloseClipboard();
        return false;
    }

    char* mem = (char*)GlobalLock(memoryHandle);
    if(!mem)
    {
        ::CloseClipboard();
        return false;
    }
    int memSize = GlobalSize(memoryHandle);

    bool result = func(mem, memSize);

    GlobalUnlock(memoryHandle);

	::CloseClipboard();
	return result;
}

int Clipboard::smartPaste(Serializer& se)
{
	YASLI_ASSERT(0 && "Not implemented");
	return paste(se) ? 1 : 0;
}

}
