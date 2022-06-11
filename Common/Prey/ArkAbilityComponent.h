#pragma once
#include <vector>

#include "ArkAbilityData.h"
#include "IArkPDAListener.h"
#include "ArkResearchTopicData.h"

class ArkAbilityComponent : IArkPDAListener
{
public:
  virtual void OnPDAOpen() = 0;
  virtual virtual ~ArkAbilityComponent() = 0;


  std::vector<ArkAbilityData> m_abilities;
  std::vector<ArkResearchTopicData> m_researchTopics;
  unsigned __int64 m_lastResearchedAbility;
  int m_partialTyphonThreshold;
  int m_fullTyphonThreshold;
};
