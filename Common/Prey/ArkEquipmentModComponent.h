#pragma once
#include <array>
#include <vector>

class ArkEquipmentModComponent
{
  const bool m_bSuitMods;
  std::vector<unsigned int> m_ownedChipsets;
  std::array<unsigned int,8> m_installedChipsets;
};
