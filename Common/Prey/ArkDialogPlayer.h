#pragma once
#include <array>
#include <vector>

#include "ArkEnums.h"
#include "IArkDialogAudioTriggerOwner.h"
#include "ArkDialogAudioTrigger.h"
#include "SSubtitleAudioData.h"
#include "ArkSpeakerBase.h"
#include "IArkDialogPlayerExtension.h"

class ArkDialogPlayer : IArkDialogAudioTriggerOwner
{
public:
  virtual virtual ~ArkDialogPlayer() = 0;
  virtual EArkDialogPlayerType GetType() = 0;
  virtual std::vector<SSubtitleAudioData> *GetSubtitleAudioData(std::vector<SSubtitleAudioData> *result) = 0;
  virtual void OnDialogComplete(const ArkDialogAudioTrigger *, const bool) = 0;
  virtual bool PlayTriggerInternal(ArkDialogAudioTrigger *) = 0;


  std::array<ArkDialogAudioTrigger,2> m_triggers;
  std::array<int,2> m_nPAPlaybackChannels;
  ArkSpeakerBase *m_pSpeaker;
  IArkDialogPlayerExtension *m_pExtension;
  unsigned int m_nAudioProxy;
  unsigned int m_nVoiceEffect;
  bool m_bMute;
  bool m_bPaused;
};
