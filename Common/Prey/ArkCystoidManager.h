#pragma once
#include <unordered_map>
#include <vector>

#include "ArkAutoResetTimer.h"
#include "ArkCystoid.h"
#include "ArkCystoidProperties.h"

class ArkCystoidManager
{
public:
class CystoidInfo
{
  unsigned int m_cystoidId;
  ArkCystoid *m_pCystoidExtension;
  float m_lastUpdateTime;
};


  std::vector<std::pair<unsigned int,unsigned int>> m_cystoidsToUnregister;
  unsigned int m_lastSwarmUpdated;
  bool m_bUpdating;
  bool m_bEnabled;
  ArkAutoResetTimer m_swarmBalanceTimer;
  int m_maxExpectedCystoidsInArea;
  int m_nestZeroGUpdateSlice;
  std::vector<unsigned int> m_pendingExplosions;
  std::unordered_map<unsigned __int64,ArkCystoidProperties> m_properties;
  std::unordered_map<unsigned int,std::vector<ArkCystoidManager::CystoidInfo>> m_swarms;
};
