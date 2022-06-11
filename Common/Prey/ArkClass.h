#pragma once
#include <unordered_map>

#include "ArkBasicTypes.h"
#include "ArkReflectedObject.h"
#include "ArkProperty.h"

class ArkClass
{
  std::unordered_map<CryStringT<char>,ArkProperty *,std::hash<CryStringT<char> >,std::equal_to<CryStringT<char> >,std::allocator<std::pair<CryStringT<char> const ,ArkProperty *> > > m_Properties;
  const ArkClass *const m_baseClass;
  const char *m_name;
  ArkReflectedObject *(__fastcall *m_ObjectFactory)();
};
