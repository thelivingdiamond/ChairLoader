#pragma once
#include <vector>

#include "CArkAlienGiblet.h"
#include "ArkTurret.h"

class ArkDynamicCorpseManager
{
  unsigned __int64 m_alienGibletCount;
  unsigned __int64 m_operatorCorpseCount;
  unsigned __int64 m_turretCorpseCount;
  std::vector<CArkAlienGiblet *> m_alienGibletVec;
  std::vector<unsigned int> m_operatorCorpseVec;
  std::vector<ArkTurret *> m_turretCorpseVec;
};
