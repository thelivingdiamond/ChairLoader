#pragma once
#include <map>

#include "DistractionState.h"
#include "IEntity.h"
#include "CrySystem/ITimer.h"

class ArkDistractionInstance
{
public:
class ObstacleCheckState
{
  bool m_bClear;
  CTimeValue m_lastCheckTime;
};


  IEntity *m_pEntityOwner;
  DistractionState m_state;
  int m_distractionDataIndex;
  float m_cooldownCompleteTime;
  unsigned int m_targetNpcEntityId;
  std::map<TNavigationID<1>,ArkDistractionInstance::ObstacleCheckState,std::less<TNavigationID<1> >,std::allocator<std::pair<TNavigationID<1> const ,ArkDistractionInstance::ObstacleCheckState> > > m_navAgentObstacleChecks;
};
