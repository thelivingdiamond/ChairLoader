#pragma once
#include "ArkBasicTypes.h"
#include "ArkSimpleTimer.h"

class ArkButtonPrompt
{
  CCryName m_actionId;
  CryStringT<char> m_actionMap;
  ArkSimpleTimer m_holdTimer;
  float m_baseHoldDuration;
  bool m_bUseHoldTriggerDelay;
  CryStringT<wchar_t> m_inputPromptString;
  CryStringT<wchar_t> m_promptText;
  bool m_bEnabled;
  bool m_bDirty;
};
