#pragma once
#include <vector>

class ArkEventScheduler
{
  std::vector<ArkEventScheduler::TimeEventInfo> m_events;
  unsigned __int64 m_time;
  unsigned __int64 m_playTime;
};
