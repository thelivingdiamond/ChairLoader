#pragma once
#include <unordered_map>

#include "ArkBasicTypes.h"
#include "ArkSimpleTimer.h"
#include "ICVar.h"
#include "SubtitleInfo.h"
#include "SLocalizedInfoGame.h"

class ArkDialogSubtitleManager
{
  std::unordered_map<unsigned __int64,SubtitleInfo> m_subtitles;
  std::vector<std::pair<unsigned int,float>> m_subtitleParts;
  SLocalizedInfoGame m_localizedSubtitle;
  ArkSimpleTimer m_lastSpeakerTimer;
  ArkSimpleTimer m_subtitleTimer;
  CryStringT<char> m_lastSpeaker;
  unsigned __int64 m_currentSubtitle;
  unsigned int m_currentSubtitlePart;
  unsigned int m_maxSubtitleCharacters;
  unsigned int m_lookbackCharacters;
  float m_subtitleTimeout;
  ICVar *m_pDebugCvar;
};
