#pragma once
#include "ArkEnums.h"

class ArkAttentionLevelChange
{
  unsigned int m_attentiveSubjectEntityId;
  unsigned int m_attentionObjectEntityId;
  EArkAttentionLevel m_previousLevel;
  EArkAttentionLevel m_currentLevel;
  EArkAttentionChangeReason m_reason;
  bool m_bChangeDueToDisposition;
  bool m_bSurprising;
  bool m_bSwitchedTarget;
  unsigned int m_oldAttentionObjectEntityId;
};
