/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#include "StdAfx.h"

#include "yasli/STL.h"
#include "ClassFactory.h"
#include "yasli/Archive.h"
#include "MemoryWriter.h"

#include "yasli/Config.h"

namespace yasli{

const char* TypeID::name() const{
#if YASLI_NO_RTTI
	if (typeInfo_)
		return typeInfo_->name;
	else
		return "";
#else
    if(typeInfo_)
        return typeInfo_->name();
	else
		return name_.c_str();
#endif
}

size_t TypeID::sizeOf() const{
#if YASLI_NO_RTTI
	if (typeInfo_)
		return typeInfo_->size;
	else
		return 0;
#else
	return 0;
#endif
}

bool YASLI_SERIALIZE_OVERRIDE(Archive& ar, TypeIDWithFactory& value, const char* name, const char* label)
{
	std::string typeName;
	if(ar.isOutput()){
		typeName = value.factory->descriptionByType(value.type)->name();
		return ar(typeName, name);
	}
	else{
		if(ar(typeName, name)){
			if(!typeName.empty())
				value.type = value.factory->findTypeByName(typeName.c_str());
			else
				value.type = TypeID();
			if(!value.type){
				char msg[128];
#ifdef _MSC_VER
				_snprintf_s(msg, sizeof(msg), _TRUNCATE, "Unable to read TypeID: unregistered type name: \'%s\'", typeName.c_str());
#else
                snprintf(msg, sizeof(msg), "Unable to read TypeID: unregistered type name: \'%s\'", typeName.c_str());
                msg[sizeof(msg)-1] = '\0';
#endif
				ar.warning(msg);
				return false;
			}
			else
				return true;
		}
		else
			return false;
	}
}

}
