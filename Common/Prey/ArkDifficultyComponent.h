#pragma once
#include <array>

#include "IArkPauseMenuListener.h"
#include "ICVar.h"

class ArkDifficultyComponent : IArkPauseMenuListener
{
public:

  int m_currentLevel;
  int m_lowestDifficulty;
  ICVar *m_pDifficultyCVar;
  std::array<bool,4> m_difficultyOptions;
};
