#pragma once
class ArkSpeakerBase : IArkSpeaker
{
public:
  bool PlayDialog(IArkSpeaker *this, SDialogParams *) = 0;
  virtual virtual ~ArkSpeakerBase() = 0;
  virtual void Update(float) = 0;
  virtual void OnConversationComplete(const bool) = 0;
  virtual void SetEntity(IEntity *const) = 0;
  virtual bool IsSpeaking() = 0;
  virtual EArkConversationDominance GetDefaultDominance() = 0;
  virtual void PostSerialize() = 0;
  virtual void SetDuckAudio(const bool) = 0;
  virtual EArkDialogPlayerType GetPlayerType() = 0;
  virtual void OnDialogAnimationEvent() = 0;
  virtual void SetMute(const bool) = 0;
  virtual void SetVoiceEffect(unsigned int) = 0;
  virtual void SetVoice(const unsigned __int64) = 0;


  std::vector<ArkSpeakerBase *> m_listeners;
  std::vector<ArkResponseFact> m_memory;
  std::unordered_map<unsigned __int64,float> m_usedResponses;
  std::vector<unsigned __int64> m_voices;
  std::list<std::pair<unsigned __int64,float>> m_timeouts;
  unsigned __int64 m_voiceId;
  const ArkCharacter *m_pCharacter;
  ArkConversation *m_pCurrentConversation;
  const ArkResponse *m_pCurrentResponse;
  IEntity *m_pEntity;
  unsigned __int64 m_conversationId;
  unsigned __int64 m_playbackVoice;
  CryStringT<char> m_playbackName;
  CryStringT<char> m_playbackPortrait;
  bool m_bAudioDucked;
};
