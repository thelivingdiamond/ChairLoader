#pragma once
#include <vector>

class ArkAudioSwitchManager
{
public:
class SwitchState
{
  unsigned int m_entityId;
  unsigned int m_switch;
  unsigned int m_state;
};


  std::vector<ArkAudioSwitchManager::SwitchState> m_switchStates;
};
