#pragma once
#include <vector>

#include "ArkAudioTrigger.h"
#include "IArkConversationListener.h"
#include "ArkSimpleTimer.h"
#include "ArkConversation.h"

class ArkAudioLogComponent : IArkConversationListener
{
public:
class UnheardAudioLog
{
  unsigned __int64 m_audioLogId;
  bool m_bAutoPlay;
};

  virtual virtual ~ArkAudioLogComponent() = 0;


  std::vector<unsigned __int64> m_Collected;
  std::vector<unsigned __int64> m_PlayedAudioLogs;
  std::vector<ArkAudioLogComponent::UnheardAudioLog> m_UnheardAudioLogs;
  ArkSimpleTimer m_loadingTimer;
  ArkConversation *m_pCurrentlyPlayingAudioLog;
  unsigned __int64 m_currentlyPlayingConversationId;
  unsigned __int64 m_currentlyPlayingAudioLogId;
  bool m_currentlyPlayingAudioLogAutoPlay;
  ArkAudioTrigger m_loadingTrigger;
  ArkAudioTrigger m_loadedTrigger;
};
