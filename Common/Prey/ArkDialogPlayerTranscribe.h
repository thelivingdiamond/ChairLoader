#pragma once
#include "ArkAudioTrigger.h"
#include "ArkDialogAudioTrigger.h"
#include "ArkDialogPlayer.h"
#include "IArkDialogAudioTriggerOwner.h"

class ArkDialogPlayerTranscribe : ArkDialogPlayer
{
public:
  void OnTriggerLoaded(IArkDialogAudioTriggerOwner *this, const ArkDialogAudioTrigger *, bool) = 0;


  ArkAudioTrigger m_inSoundTrigger;
  ArkAudioTrigger m_outSoundTrigger;
  bool m_bConversationPlaying;
};
