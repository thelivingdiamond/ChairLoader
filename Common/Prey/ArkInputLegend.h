#pragma once
#include "ArkBasicTypes.h"
#include "ArkInputLegendItem.h"
#include "ArkReflectedObject.h"

class ArkInputLegend : ArkReflectedObject
{
  CryStringT<char> m_ActionMap;
  std::vector<ArkInputLegendItem> m_Items;
};
