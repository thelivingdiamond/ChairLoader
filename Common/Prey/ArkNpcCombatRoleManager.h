#pragma once
#include <unordered_map>

#include "ArkMetaTagPairHash.h"
#include "ArkNpcCombatRoleData.h"
#include "ArkSimpleTimer.h"

class ArkNpcCombatRoleManager
{
  ArkSimpleTimer m_assignRolesTimer;
  std::vector<ArkNpcCombatRoleData> m_combatantsRoleData;
  std::unordered_map<std::pair<unsigned __int64,unsigned __int64>,float,ArkMetaTagPairHash,std::equal_to<std::pair<unsigned __int64,unsigned __int64> >,std::allocator<std::pair<std::pair<unsigned __int64,unsigned __int64> const ,float> > > m_meleeDistModHashMap;
};
