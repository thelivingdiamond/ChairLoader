#pragma once
#include "CGameObjectExtensionHelper.h"
#include "IArkPlayer.h"
#include "Prey/CrySystem/ISystem.h"

class __cppobj ArkPlayer : CGameObjectExtensionHelper<ArkPlayer, CActor, 64>, IArkPlayer, IArkPlayerCombatListener, ISystemEventListener
{
	ArkPlayerMovementFSM m_movementFSM;
	ArkPlayerComponent m_playerComponent;
	ArkStatsComponent m_statsComponent;
	ArkEquipmentModComponent m_suitChipsetComponent;
	ArkEquipmentModComponent m_scopeChipsetComponent;
	ArkPlayerInput m_input;
	ArkExaminationMode m_examinationMode;
	ArkPlayerInteraction m_interaction;
	ArkGroundColliderComponent m_groundColliderComponent;
	ArkPlayerFlashlight m_flashlight;
	ArkPlayerAudio m_audio;
	ArkPlayerCamera m_camera;
	ArkPlayerHelmet m_helmet;
	ArkPlayerWeaponComponent m_weaponComponent;
	std::vector<IPlayerEventListener*> m_playerEventListeners;
	std::vector<std::pair<CryStringT<char>, unsigned int>> m_attachmentFlags;
	std::vector<unsigned __int64> m_learnedNames;
	std::vector<unsigned __int64> m_criticalHitMetaTags;
	std::vector<CryStringT<char>> m_animationTagsSetFromFG;
	std::array<StanceInfo, 18> m_stanceInfos;
	CryStringT<char> m_modelName;
	Vec3_tpl<float> m_serializedVelocity;
	Vec3_tpl<float> m_cachedReticlePos;
	Vec3_tpl<float> m_cachedReticleDir;
	Vec2_tpl<float> m_reticlePos;
	_smart_ptr<IAction> m_pHeadBobbingAction;
	std::unique_ptr<ArkPlayerMovementController> m_pMovementController;
	IAnimatedCharacter* m_pAnimatedCharacter;
	ArkInventory* m_pInventory;
	std::unique_ptr<CFragmentCache> m_pFragmentCache;
	unsigned __int64 m_coralSignalPackageId;
	ArkSimpleTimer m_coralInteractionTimer;
	float m_knockdownLoopDuration;
	float m_idleFragmentSpeedThreshold;
	EStance m_stance;
	ArkPlayerAttentionObject m_attentionObject;
	float m_fearNoiseAmplitude;
	float m_autoSaveHealthThreshold;
	float m_autoSaveOxygenThreshold;
	float m_attackTargetPosHeightPercent;
	float m_lookAtFarRange;
	bool m_bInTrackview;
	bool m_bInAir;
	bool m_bShowArmor;
	bool m_bShowHealth;
	bool m_bAllowIdleBreak;
	bool m_bJustPhysicalized;
	bool m_bShowContinuePrompt;
	bool m_bReceivedStartLevelEvent;
	bool m_bReticleViewPositionValid;
	ArkSimpleTimer m_autoSaveBlockedBySignalTimer;
	ArkPlayerZoomManager m_zoomManager;
	unsigned int m_rotationModifierHandleGenerator;
	std::vector<std::pair<unsigned int, Functor2<Ang3_tpl<float>&, float> >> m_rotationModifiers;
	unsigned int m_mimicGrabSequenceEntityId;
	ArkMaterialAnimationManager m_materialAnimationManager;
};
