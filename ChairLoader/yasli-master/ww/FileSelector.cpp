/**
 *  wWidgets - Lightweight UI Toolkit.
 *  Copyright (C) 2009-2011 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "yasli/STL.h"
#include "yasli/Archive.h"
#include "ww/FileSelector.h"

namespace ww{

void FileSelector::serialize(yasli::Archive& ar)
{
	ar(fileName_, "fileName", "File Name");
}

bool serialize(yasli::Archive& ar, FileSelector& selector, const char* name, const char* label)
{
    if(ar.isEdit())
        return ar(yasli::Serializer(selector), name, label);
    else
        return ar(selector.fileName_, name, label);
}

}
