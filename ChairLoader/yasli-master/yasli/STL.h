/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include <vector>
#include <list>
#include <map>
#include <algorithm>

#include "yasli/Serializer.h"
#include "yasli/KeyValue.h"

namespace yasli{ class Archive; }

namespace std{ // not nice, but needed for argument-dependent lookup to work

template<class T, class Alloc>
bool YASLI_SERIALIZE_OVERRIDE(yasli::Archive& ar, std::vector<T, Alloc>& container, const char* name, const char* label);

template<class T, class Alloc>
bool YASLI_SERIALIZE_OVERRIDE(yasli::Archive& ar, std::list<T, Alloc>& container, const char* name, const char* label);

template<class K, class V, class C, class Alloc>
bool YASLI_SERIALIZE_OVERRIDE(yasli::Archive& ar, std::map<K, V, C, Alloc>& container, const char* name, const char* label);

bool YASLI_SERIALIZE_OVERRIDE(yasli::Archive& ar, std::string& value, const char* name, const char* label);
bool YASLI_SERIALIZE_OVERRIDE(yasli::Archive& ar, std::wstring& value, const char* name, const char* label);

template<class V>
bool YASLI_SERIALIZE_OVERRIDE(yasli::Archive& ar, std::pair<std::string, V>& pair, const char* name, const char* label);
template<class K, class V>
bool YASLI_SERIALIZE_OVERRIDE(yasli::Archive& ar, std::pair<K, V>& pair, const char* name, const char* label);

}

#include "STLImpl.h"
