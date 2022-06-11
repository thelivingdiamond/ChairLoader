#pragma once
#include <vector>

class ArkEthericFogManager
{
public:
class EthericFogInfo
{
  unsigned int m_fogFragmentEntityId;
  bool m_bIsTrackingSmallFragment;
};

  unsigned __int64 m_maxNumFragments;
  float m_minDistanceBetweenFragmentsSq;
  float m_largeFragmentPreventOverlapRadiusSq;
  std::vector<ArkEthericFogManager::EthericFogInfo> m_activeFogFragments;
};
