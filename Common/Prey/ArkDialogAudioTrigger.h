#pragma once
#include "ArkBasicTypes.h"
#include "EState.h"
#include "IArkDialogAudioTriggerOwner.h"
#include "IArkDialogLoadListener.h"

class ArkDialogAudioTrigger : IArkDialogLoadListener, ArkAudioTrigger
{
public:
  virtual virtual ~ArkDialogAudioTrigger() = 0;
  IArkDialogAudioTriggerOwner *m_pOwner;
	//possible issue here with ArkDialogAudioTrigger::EState
  EState m_state;
  unsigned __int64 m_dialogId;
  unsigned __int64 m_voiceId;
  unsigned __int64 m_conversationId;
  unsigned __int64 m_playbackHandle;
  float m_fDelay;
  float m_fPlayTime;
  float m_fDuration;
  float m_fEndEarly;
  CryStringT<char> m_speakerName;
  CryStringT<char> m_speakerPortrait;
  int m_nPASpeakerChannel;
  bool m_bCutoffEvent;
};
