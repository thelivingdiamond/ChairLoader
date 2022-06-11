#pragma once
#include <unordered_map>
#include "ArkAutoResetTimer.h"

class ArkApexVolumeManager
{
  std::unordered_map<unsigned int,float> m_entityTimes;
  ArkAutoResetTimer m_timer;
  unsigned __int64 m_dotSignal;
  unsigned __int64 m_killSignal;
  float m_killTimeThreshold;
};
