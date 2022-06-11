#pragma once
#include <vector>

#include "ArkNpcCombatRoleManager.h"

class ArkCombatManager
{
  std::vector<unsigned int> m_combatantsTargetingPlayer;
  ArkNpcCombatRoleManager m_combatRoleManager;
};
