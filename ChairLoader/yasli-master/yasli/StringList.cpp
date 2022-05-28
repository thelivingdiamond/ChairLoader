/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"
#include "StringList.h"
#include "yasli/STL.h"
#include "yasli/Archive.h"
#include "yasli/STLImpl.h"

namespace yasli{

StringList StringList::EMPTY;
StringListStatic StringListStatic::EMPTY;

// ---------------------------------------------------------------------------
void splitStringList(StringList* result, const char *str, char delimeter)
{
    result->clear();

    const char* ptr = str;
    for(; *ptr; ++ptr)
	{
        if(*ptr == delimeter){
			result->push_back(std::string(str, ptr));
            str = ptr + 1;
        }
	}
	result->push_back(std::string(str, ptr));
}

void joinStringList(std::string* result, const StringList& stringList, char sep)
{
    YASLI_ESCAPE(result != 0, return);
    result->clear();
    for(StringList::const_iterator it = stringList.begin(); it != stringList.end(); ++it)
    {
        if(!result->empty())
            result += sep;
        result->append(*it);
    }
}

void joinStringList(std::string* result, const StringListStatic& stringList, char sep)
{
    YASLI_ESCAPE(result != 0, return);
    result->clear();
    for(StringListStatic::const_iterator it = stringList.begin(); it != stringList.end(); ++it)
    {
        if(!result->empty())
            (*result) += sep;
        YASLI_ESCAPE(*it != 0, continue);
        result->append(*it);
    }
}

bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, StringList& value, const char* name, const char* label)
{
	return ar(static_cast<std::vector<std::string>&>(value), name, label);
}

bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, StringListValue& value, const char* name, const char* label)
{
    if(ar.isEdit()){
		Serializer ser(value);
        return ar(ser, name, label);
    }
    else{
        std::string str;
        if(ar.isOutput())
            str = value.c_str();
        if(ar(str, name, label) && ar.isInput()){
            value = str.c_str();
            return true;
        }
        return false;
    }
}

bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, StringListStaticValue& value, const char* name, const char* label)
{
    if(ar.isEdit())
        return ar(Serializer(value), name, label);
    else{
        std::string str;
        if(ar.isOutput())
            str = value.c_str();
        if(ar(str, name, label) && ar.isInput()){
            value = str.c_str();
            return true;
        }
        return false;
    }
}

}
