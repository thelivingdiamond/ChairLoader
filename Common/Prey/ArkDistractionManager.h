#pragma once
#include <unordered_map>
#include <vector>

#include "ArkDistractionInstance.h"

class ArkDistractionManager
{
  std::vector<ArkDistractionInstance> m_distractionInstances;
  std::vector<unsigned __int64> m_unusedIndices;
  std::unordered_map<unsigned int,std::vector<unsigned __int64>> m_entityToDistractionMap;
  std::unordered_map<unsigned int,ArkDelegate<void __cdecl(unsigned int,bool)>> m_distractionInstanceFGCallbacks;
  float m_fAreaCooldownRadius;
  unsigned __int64 m_iLastUpdateIndex;
  int m_iAmortizedUpdateNumFrames;
  int m_iMaxSearchPerFrame;
};
