#pragma once
#include <array>

#include "ArkButtonPrompt.h"
#include "ArkExclusiveActionFilter.h"
#include "IArkPauseMenuListener.h"
#include "IUIControlSchemeListener.h"

class ArkButtonPromptCollection : IUIControlSchemeListener, IArkPauseMenuListener
{
public:


  std::array<ArkButtonPrompt,8> m_buttonPrompts;
  unsigned __int8 m_maxUsed;
  CryStringT<char> m_formatFunction;
  CryStringT<char> m_setFunctionOrArrayName;
  ArkExclusiveActionFilter m_filter;
  bool m_bInitialized;
};
