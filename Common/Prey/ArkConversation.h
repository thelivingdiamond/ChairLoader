#pragma once
#include "ArkEnums.h"
#include "ArkSpeakerBase.h"
#include "IArkConversation.h"
#include "IArkSpeaker.h"

class ArkConversation : IArkConversation
{
public:
  virtual void AddListener(IArkConversationListener *const) = 0;
  virtual void RemoveListener(IArkConversationListener *const) = 0;
  virtual unsigned __int64 GetConversationId() = 0;


  std::vector<IArkConversationListener *> m_listeners;
  std::vector<ArkSpeakerBase *> m_speakers;
  unsigned __int64 m_conversationId;
  unsigned __int64 m_hookId;
  char m_nPAChannel;
  EArkConversationDominance m_dominance;
  EArkConversationPriority m_priority;
  EArkConversationType m_type;
  bool m_bIgnoreVoiceRequirement;
  bool m_bLiveAudio;
  bool m_bImportant;
};
