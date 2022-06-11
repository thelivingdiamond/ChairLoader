#pragma once
#include <vector>

#include "ArkBasicTypes.h"
#include "ArkEnums.h"
#include "ArkSimpleTimer.h"
#include "IArkActiveUserManagerListener.h"
#include "ISystemEventListener.h"
#include "XmlNodeRef.h"
#include "IArkGameLoadSaveListener.h"
#include "ISaveGameEnumerator.h"

class ArkGame : IArkActiveUserManagerListener, ISystemEventListener
{
public:


  ArkGame::ScreenBlackFadeInfo m_levelLoadFade;
  ArkGame::ScreenBlackFadeInfo m_quickLoadFade;
  std::vector<CryStringT<char>> m_captureVolumeWhiteList;
  std::array<_smart_ptr<ISaveGameEnumerator>,3> m_saveGameEnumerators;
  std::array<std::vector<CryStringT<char>>,3> m_saveGameToUpdate;
  std::array<unsigned __int64,3> m_campaignGuids;
  ArkSimpleTimer m_autoSaveDelay;
  ArkSimpleTimer m_blockAutoSaveWindow;
  ArkSimpleTimer m_screenBlackTimer;
  ArkSimpleTimer m_screenFadeOutTimer;
  CryStringT<char> m_saveFileToLoad;
  CryStringT<char> m_loadLocationOverride;
  IArkGameLoadSaveListener *m_pLoadListener;
  unsigned __int64 m_campaignPlayTime;
  float m_campaignTimerAccumulator;
  unsigned __int64 m_currentLocationId;
  XmlNodeRef m_levelProperties;
  std::unique_ptr<CScriptBind_ArkPlayer> m_pScriptBindArkPlayer;
  std::unique_ptr<ArkCameraShakeManager> m_pArkCameraShakeManager;
  std::unique_ptr<ArkDialogSubtitleManager> m_pArkDialogSubtitleManager;
  std::unique_ptr<ArkDifficultyComponent> m_pArkDifficultyComponent;
  std::unique_ptr<ArkEventScheduler> m_pArkEventScheduler;
  std::unique_ptr<ArkGameStateConditionManager> m_pArkGameStateConditionManager;
  std::unique_ptr<ArkHackingUI> m_pArkHackingUI;
  std::unique_ptr<ArkIndicatorIconManager> m_pArkIndicatorIconManager;
  std::unique_ptr<ArkItemSystem> m_pArkItemSystem;
  std::unique_ptr<ArkLevelMapComponent> m_pArkLevelMapComponent;
  std::unique_ptr<ArkLightManager> m_pArkLightManager;
  std::unique_ptr<ArkLocationManager> m_pArkLocationManager;
  std::unique_ptr<ArkNoiseAttentionParams> m_pArkNoiseAttentionParams;
  std::unique_ptr<ArkObjectiveComponent> m_pArkObjectiveComponent;
  std::unique_ptr<ArkSignalSystem::Manager> m_pArkSignalManager;
  std::unique_ptr<ArkTipComponent> m_pArkTipComponent;
  std::unique_ptr<ArkTutorialSystem> m_pArkTutorialSystem;
  std::unique_ptr<ArkApexVolumeManager> m_pApexVolumeManager;
  std::unique_ptr<ArkGameModeManager> m_pArkGameModeManager;
  std::unique_ptr<ArkNewGamePlus> m_pNewGamePlus;
  AABB m_playerSafeBounds;
  EArkLoadDisconnectReason m_loadDisconnectReason;
  ArkGame::PauseRequestStatus m_pauseRequestStatus;
  float m_playerWarningBuffer;
  float m_timeSinceLastAutoSave;
  bool m_bLevelToLevelTransition;
  bool m_bSaveHasJustBeenDeletedBySystem;
  bool m_bPreTextureStreamerRunning;
  bool m_bNeedsToAutosaveOnMapLoad;
  bool m_bPerformingIronmanSave;
};
