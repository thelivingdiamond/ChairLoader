#pragma once
#include "ArkAbilityScanData.h"
class ArkResearchTopicData
{
  std::vector<ArkAbilityScanData> m_abilityScanData;
  unsigned __int64 m_id;
  int m_scanCount;
  int m_loreScansRequired;
};
