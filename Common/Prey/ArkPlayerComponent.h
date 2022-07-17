#pragma once
#include "ArkPsiComponent.h"
#include "ArkSpeaker.h"
#include "ArkPlayerStatus.h"
#include "Prey/CryCore/StdAfx.h"

class ArkPDAComponent;
class ArkPOIComponent;
class ArkStationAccessComponent
{
	std::vector<unsigned __int64> m_lockedPaths;
	std::vector<unsigned __int64> m_lockedAirlocks;
	std::vector<unsigned __int64> m_hiddenAirlocks;
};
class ArkLiveTranscribeComponent;
class ArkFocusModeComponent;
class ArkGameMetricsComponent;
class ArkDialogPlayerTranscribe;
class ArkDialogPlayer;
class ArkPlayerScopeComponent;
class ArkMarkedEnemyComponent;
class ArkAimAssistComponent;
class ArkPlayerSignalReceiver;
class ArkPlayerPropulsionComponent;
class ArkFabricationPlanComponent;
class ArkPlayerUIComponent;
class ArkPlayerFXComponent;
class ArkPlayerLightManager;
class ArkPlayerRadiationComponent;
class ArkPlayerHealthComponent;
class ArkPlayerEntitlementComponent;
class ArkPlayerAwarenessComponent;
class ArkPlayerAchievementComponent;
class ArkPharmaComponent;
class ArkUtilityComponent;
class ArkRosterComponent;
class ArkQuickSelectComponent;
class ArkEmailComponent;
class ArkKeyCardComponent;
class ArkLoreComponent;
class ArkLocationComponent;
class ArkKeyCodeComponent;
class ArkNoteComponent;
class ArkAudioLogComponent;
class ArkAbilityComponent;

class ArkPlayerComponent
{
public:
	std::unique_ptr<CArkPsiComponent> m_pPsiComponent;
	std::unique_ptr<ArkAbilityComponent> m_pAbilityComponent;
	std::unique_ptr<ArkAudioLogComponent> m_pAudioLogComponent;
	std::unique_ptr<ArkNoteComponent> m_pNoteComponent;
	std::unique_ptr<ArkKeyCodeComponent> m_pKeyCodeComponent;
	std::unique_ptr<ArkLocationComponent> m_pLocationComponent;
	std::unique_ptr<ArkLoreComponent> m_pLoreComponent;
	std::unique_ptr<ArkKeyCardComponent> m_pKeyCardComponent;
	std::unique_ptr<ArkEmailComponent> m_pEmailComponent;
	std::unique_ptr<ArkQuickSelectComponent> m_pQuickSelectComponent;
	std::unique_ptr<ArkRosterComponent> m_pRosterComponent;
	std::unique_ptr<ArkUtilityComponent> m_pUtilityComponent;
	std::unique_ptr<ArkPharmaComponent> m_pPharmaComponent;
	std::unique_ptr<ArkPDAComponent> m_pPDAComponent;
	std::unique_ptr<ArkPlayerAchievementComponent> m_pAchievementComponent;
	std::unique_ptr<ArkPlayerAwarenessComponent> m_pAwarenessComponent;
	std::unique_ptr<ArkPlayerEntitlementComponent> m_pEntitlementComponent;
	std::unique_ptr<ArkPlayerFatigueComponent> m_pFatigueComponent;
	std::unique_ptr<ArkPlayerHealthComponent> m_pHealthComponent;
	std::unique_ptr<ArkPlayerRadiationComponent> m_pRadiationComponent;
	std::unique_ptr<ArkPlayerLightManager> m_pLightManager;
	std::unique_ptr<ArkPlayerFXComponent> m_pFXComponent;
	std::unique_ptr<ArkPlayerUIComponent> m_pUIComponent;
	std::unique_ptr<ArkFabricationPlanComponent> m_pFabricationPlanComponent;
	std::unique_ptr<ArkPlayerStatusComponent> m_pStatusComponent;
	std::unique_ptr<ArkPlayerPropulsionComponent> m_pPropulsionComponent;
	std::unique_ptr<ArkPlayerSignalReceiver> m_pSignalReceiver;
	std::unique_ptr<ArkAimAssistComponent> m_pAimAssistComponent;
	std::unique_ptr<ArkMarkedEnemyComponent> m_pMarkedEnemyComponent;
	std::unique_ptr<ArkPlayerScopeComponent> m_pScopeComponent;
	std::unique_ptr<ArkSpeaker<ArkDialogPlayer>> m_pPlayerSpeaker;
	std::unique_ptr<ArkSpeaker<ArkDialogPlayer>> m_pSuitSpeaker;
	std::unique_ptr<ArkSpeaker<ArkDialogPlayer>> m_pDiscRifleSpeaker;
	std::unique_ptr<ArkSpeaker<ArkDialogPlayerTranscribe>> m_pTranscribeSpeaker;
	std::unique_ptr<ArkGameMetricsComponent> m_pGameMetricsComponent;
	std::unique_ptr<ArkFocusModeComponent> m_pFocusModeComponent;
	std::unique_ptr<ArkLiveTranscribeComponent> m_pLiveTranscribeComponent;
	std::unique_ptr<ArkStationAccessComponent> m_pStationAccessComponent;
	std::unique_ptr<ArkPOIComponent> m_pPOIComponent;
	float m_fShadowDissolveBlend;
	float m_fShadowDissolveTarget;
	float m_fShadowDissolveTimeScale;
	bool m_bItemsRestricted;
	bool m_bMarkerTextDisplayed;
};
static_assert(sizeof(ArkPlayerComponent) == 0x148);