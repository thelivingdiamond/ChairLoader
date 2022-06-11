#pragma once
#include "ArkResearchTopicData.h"

class ArkAbilityData
{
  std::vector<ArkAbilityData *> m_prereqs;
  std::vector<ArkAbilityData *> m_children;
  std::vector<ArkResearchTopicData *> m_researchTopicData;
  std::vector<unsigned int> m_modifiers;
  unsigned __int64 m_id;
  bool m_bSeen;
  bool m_bAcquired;
};
