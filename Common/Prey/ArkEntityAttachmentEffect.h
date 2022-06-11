#pragma once
#include "ArkBasicTypes.h"
#include "ArkEffectBase.h"

class ArkEntityAttachmentEffect : ArkEffectBase
{
  EntityEffects::CEffectsController *m_controller;
  CryStringT<char> m_attachmentName;
  int m_attachmentSlot;
  unsigned int m_effectId;
};
