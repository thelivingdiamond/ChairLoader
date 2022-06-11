#pragma once
class SubtitleInfo
{
  unsigned __int64 dialogId;
  unsigned __int64 voiceId;
  std::vector<ArkDialogPlayer const *> dialogPlayers;
  const char *pSpeakerName;
  bool bMetaGameMenu;
  bool bMain;
  ArkTimeRemaining removeTimer;
};
