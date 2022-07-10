#include "pch.h"
#include <Prey/CryMemory/CryMemoryManager.h>
#include "PreyFunctions.h"
#include <ChairLoader/PreyFunction.h>

PreyFunctions *gPreyFuncs = nullptr;

namespace {
constexpr int kCryMMIAT_None = 0x0;

PreyFunction<void* (size_t size, int _module, int _type)> g_pfnCryModuleMalloc(0xA8C70);
PreyFunction<void* (void* ptr, size_t size, int _module, int _type)> g_pfnCryModuleRealloc(0xA8D40);
PreyFunction<void* (void* ptr)> g_pfnCryModuleFree(0xA8C60);
PreyFunction<void* (size_t size, size_t alignment, int _module, int _type)> g_pfnCryModuleMemalign(0xA8CC0);
PreyFunction<void* (void* ptr, size_t size, size_t alignment, int _module, int _type)> g_pfnCryModuleReallocAlign(0xA8DB0);
PreyFunction<void* (void* ptr)> g_pfnCryModuleMemalignFree(0xA8D20);

}

extern "C" void *CryModuleMalloc(size_t size) throw() {
	return g_pfnCryModuleMalloc(size, eCryM_Launcher, kCryMMIAT_None);
}

extern "C" void *CryModuleRealloc(void *memblock, size_t size) throw() {
	
	return g_pfnCryModuleRealloc(memblock, size, eCryM_Launcher, kCryMMIAT_None);
}

extern "C" void  CryModuleFree(void *ptr) throw() {
	g_pfnCryModuleFree(ptr);
}

extern "C" void *CryModuleMemalign(size_t size, size_t alignment) {
	return g_pfnCryModuleMemalign(size, alignment, eCryM_Launcher, kCryMMIAT_None);
}

extern "C" void *CryModuleReallocAlign(void *memblock, size_t size, size_t alignment) {
	return g_pfnCryModuleReallocAlign(memblock, size, alignment, eCryM_Launcher, kCryMMIAT_None);
}

extern "C" void  CryModuleMemalignFree(void *memblock) {
	g_pfnCryModuleMemalignFree(memblock);
}

template <typename t>
t PreyFunctionBaseOld::getFunctionAddr(uintptr_t moduleBaseIn, uintptr_t offset) {
	return (t)(moduleBaseIn + offset);
}
PreyFunctions::ArkPlayerPrivate::ArkPlayerPrivate(uintptr_t moduleBase) {
	// getEntityClassName = getFunctionAddr<_GetEntityClassName>(moduleBase, 0x154f9b0);//154f9b0
	// getViewRotation = getFunctionAddr<_GetViewRotation>(moduleBase, 0x154ff50);

	showThirdPerson = getFunctionAddr<_ShowThirdPerson>(moduleBase, 0x1557e20);
	getPlayerWorldEyePos = getFunctionAddr<_GetPlayerWorldEyePos>(moduleBase, 0x131f920);
	onFlyModePressed = getFunctionAddr<_onFlyModePressed>(moduleBase, 0x15710e0);
	ragdollize = getFunctionAddr<_Ragdollize>(moduleBase, 0x1555000);
	getInstance = getFunctionAddr<_GetPlayerInstance>(moduleBase, 0x154fa60);
	getGodModeInstance = getFunctionAddr<_GetGodModeInstance>(moduleBase, 0x173c050);
}

PreyFunctions::CArkWeaponPrivate::CArkWeaponPrivate(uintptr_t moduleBase) {
	getEquippedWeapon = getFunctionAddr<_GetEquippedWeapon>(moduleBase, 0x1248500);
	fireWeapon = getFunctionAddr<_FireWeapon>(moduleBase, 0x16659b0);
	setWeaponAmmoCount = getFunctionAddr<_SetWeaponAmmoCount>(moduleBase, 0x166d840);
	getClipSize = getFunctionAddr<_GetClipSize>(moduleBase, 0x161665f50);
}
PreyFunctions::CEntitySystemPrivate::CEntitySystemPrivate(uintptr_t moduleBase) {
	findEntityArchetype = getFunctionAddr<_FindEntityArchetype>(moduleBase, 0x0922910);//090fb70
	getArchetype = getFunctionAddr<_GetArchetype>(moduleBase, 0x0922de0);
	getArchetypeNames = getFunctionAddr<_GetArchetypeNames>(moduleBase, 0x0922d00);
	update = getFunctionAddr<_Update>(moduleBase, 0x0927920);
	createNpc = getFunctionAddr<_CreateNpc>(moduleBase, 0x151d6d0);

	generateEntityId = getFunctionAddr<_GenerateEntityId>(moduleBase, 0x0922bb0);//0922bb0
	
}

PreyFunctions::ArkNpcSpawnManagerPrivate::ArkNpcSpawnManagerPrivate(uintptr_t moduleBase) {
	ArkNpcSpawnManagerUpdate = getFunctionAddr<_ArkNpcSpawnManagerUpdate>(moduleBase, 0x151db30);
	requestNpcSpawn = getFunctionAddr<_RequestNpcSpawn>(moduleBase, 0x151dac0);
	createNpc = getFunctionAddr<_CreateNpc>(moduleBase, 0x151d6d0);
}
PreyFunctions::CEntityPrivate::CEntityPrivate(uintptr_t moduleBase) {
	hasAi = getFunctionAddr<_HasAi>(moduleBase, 0x0905b70);//0905b70
	getArkNpcSpawner = getFunctionAddr<_GetArkNpcSpawner>(moduleBase, 0x151c740);
	CEntityConstructor = getFunctionAddr<_CEntity>(moduleBase, 0x09022a0);
	getScriptTable = getFunctionAddr<_GetScriptTable>(moduleBase, 0x03ec540);
	safeGetArkNpcFromEntityPtr = getFunctionAddr<_SafeGetArkNpcFromEntityPtr>(moduleBase, 0x12af240);
	safeGetArkNpcFromEntityId = getFunctionAddr<_SafeGetArkNpcFromEntityId>(moduleBase, 0x12af1f0);
	getSafePropertiesTable = getFunctionAddr<_getSafePropertiesTable>(moduleBase, 0x1480480);
	reloadEntity = getFunctionAddr<_ReloadEntity>(moduleBase, 0x0908650);
}
PreyFunctions::ArkNightmareSpawnManagerPrivate::ArkNightmareSpawnManagerPrivate(uintptr_t moduleBase) {
	spawnNewNightmare = getFunctionAddr<_SpawnNewNightmare>(moduleBase, 0x11f58d0);
	enableNightmareSpawner = getFunctionAddr <_enableNightmareSpawner >(moduleBase, 0x11f47a0);
}

PreyFunctions::CArkNpcSpawnerPrivate::CArkNpcSpawnerPrivate(uintptr_t moduleBase) {
	cArkNpcSpawnerConstructor = getFunctionAddr<_CArkNpcSpawner>(moduleBase, 0x151bed0);
	createNpcSpawner = getFunctionAddr<_CreateNpcSpawner>(moduleBase, 0x1703f00);//1703f00
	requestSpawn = getFunctionAddr<_RequestSpawn>(moduleBase, 0x151cd80);
}

PreyFunctions::CXConsolePrivate::CXConsolePrivate(uintptr_t moduleBase) {
	showConsole = getFunctionAddr<_ShowConsole>(moduleBase, 0x0df85f0);
	displayHelp = getFunctionAddr<_DisplayHelp>(moduleBase, 0x0df0fa0);
	enableActivationKey = getFunctionAddr<_EnableActivationKey>(moduleBase, 0x0df23b0);
	initCXConsole = getFunctionAddr<_Init>(moduleBase, 0x0df5270);
	executeString = getFunctionAddr<_ExecuteString>(moduleBase, 0x0df3640);
	getCvar = getFunctionAddr<_GetCvar>(moduleBase, 0x0df48e0);
	addLine = getFunctionAddr<_AddLine>(moduleBase, 0x0defa00);
}

PreyFunctions::ArkFactionManagerPrivate::ArkFactionManagerPrivate(uintptr_t moduleBase) {
	setEntityFaction = getFunctionAddr<_SetEntityFaction>(moduleBase, 0x14336a0);
}

PreyFunctions::ArkNpcPrivate::ArkNpcPrivate(uintptr_t moduleBase) {
		ActivateWander = getFunctionAddr<_ActivateWander>(moduleBase, 0x14f36c0);
		AddCollisionObserver = getFunctionAddr<_AddCollisionObserver>(moduleBase, 0x14f37a0);
		AddFacingDesire = getFunctionAddr<_AddFacingDesire>(moduleBase, 0x14f3810);
		AddInboundModifier = getFunctionAddr<_AddInboundModifier>(moduleBase, 0x14f3950);
		AddLookDesire = getFunctionAddr<_AddLookDesire>(moduleBase, 0x14f39c0);
		AddMovementDesire = getFunctionAddr<_AddMovementDesire>(moduleBase, 0x14f3b00);
		AddNoiseStateDesire = getFunctionAddr<_AddNoiseStateDesire>(moduleBase, 0x14f3c40);
		AddSpeedDesire = getFunctionAddr<_AddSpeedDesire>(moduleBase, 0x14f3d80);
		AttachPistol = getFunctionAddr<_AttachPistol>(moduleBase, 0x14f3ec0);
		BecomeActive = getFunctionAddr<_BecomeActive>(moduleBase, 0x14f3fe0);
		BecomeBroken = getFunctionAddr<_BecomeBroken>(moduleBase, 0x14f4070);
		BecomeInactive = getFunctionAddr<_BecomeInactive>(moduleBase, 0x14f4080);
		BeginAnimatedDistraction = getFunctionAddr<_BeginAnimatedDistraction>(moduleBase, 0x14f40c0);
		BeginAnimatedStunned = getFunctionAddr<_BeginAnimatedStunned>(moduleBase, 0x14f40d0);
		BeginDamagedAudio = getFunctionAddr<_BeginDamagedAudio>(moduleBase, 0x14f40e0);
		BeginDisabledAudio = getFunctionAddr<_BeginDisabledAudio>(moduleBase, 0x14f40f0);
		BeginLurking = getFunctionAddr<_BeginLurking>(moduleBase, 0x14f4100);
		BeginSearchExamine = getFunctionAddr<_BeginSearchExamine>(moduleBase, 0x14f4150);
		BeginUndamagedAudio = getFunctionAddr<_BeginUndamagedAudio>(moduleBase, 0x14f42a0);
		BreakGlooEntity = getFunctionAddr<_BreakGlooEntity>(moduleBase, 0x14f42b0);
		BreakNearbyGloo = getFunctionAddr<_BreakNearbyGloo>(moduleBase, 0x14f4330);
		CalcHasSpaceToMimicSideStep = getFunctionAddr<_CalcHasSpaceToMimicSideStep>(moduleBase, 0x14f4750);
		CallForHelp = getFunctionAddr<_CallForHelp>(moduleBase, 0x14f4940);
		CanAttachGloo = getFunctionAddr<_CanAttachGloo>(moduleBase, 0x14f4a00);
		CanBeDistracted = getFunctionAddr<_CanBeDistracted>(moduleBase, 0x14f4a40);
		CanBeHypnotized = getFunctionAddr<_CanBeHypnotized>(moduleBase, 0x14f4ac0);
		CanDeathReact = getFunctionAddr<_CanDeathReact>(moduleBase, 0x14f4b10);
		CanHitReact = getFunctionAddr<_CanHitReact>(moduleBase, 0x14f4b20);
		CanInstigate = getFunctionAddr<_CanInstigate>(moduleBase, 0x14f4c30);
		CanJump = getFunctionAddr<_CanJump>(moduleBase, 0x14f4c70);
		CanMimicSideStepLeft = getFunctionAddr<_CanMimicSideStepLeft>(moduleBase, 0x14f4d50);
		CanMimicSideStepRight = getFunctionAddr<_CanMimicSideStepRight>(moduleBase, 0x14f4d60);
		CanPerformAbilityContext = getFunctionAddr<_CanPerformAbilityContext>(moduleBase, 0x14f4d70);
		CanPerformAnimatedAbility = getFunctionAddr<_CanPerformAnimatedAbility>(moduleBase, 0x14f4e00);
		CanPerformFatality = getFunctionAddr<_CanPerformFatality>(moduleBase, 0x14f4e10);
		CanPerformHitReactShift = getFunctionAddr<_CanPerformHitReactShift>(moduleBase, 0x14f4e20);
		CanPerformUnanimatedAbility = getFunctionAddr<_CanPerformUnanimatedAbility>(moduleBase, 0x14f4e30);
		CanRaiseAnotherPhantom = getFunctionAddr<_CanRaiseAnotherPhantom>(moduleBase, 0x14f4e40);
		CanSee = getFunctionAddr<_CanSee>(moduleBase, 0x14f4f40);
		CanSpeak = getFunctionAddr<_CanSpeak>(moduleBase, 0x14f4fe0);
		CancelMovement = getFunctionAddr<_CancelMovement>(moduleBase, 0x14f5020);
		ChangeAiTree = getFunctionAddr<_ChangeAiTree>(moduleBase, 0x14f5030);
		ClearAllAttention = getFunctionAddr<_ClearAllAttention>(moduleBase, 0x14f5080);
		ClearAttentionOnPlayer = getFunctionAddr<_ClearAttentionOnPlayer>(moduleBase, 0x14f5110);
		ClearConversationFacingTarget = getFunctionAddr<_ClearConversationFacingTarget>(moduleBase, 0x14f51c0);
		ClearConversationLookTarget = getFunctionAddr<_ClearConversationLookTarget>(moduleBase, 0x14f5210);
		ClearFear = getFunctionAddr<_ClearFear>(moduleBase, 0x14f5230);
		ClearPrecache = getFunctionAddr<_ClearPrecache>(moduleBase, 0x14f52a0);
		Corrupt = getFunctionAddr<_Corrupt>(moduleBase, 0x14f52c0);
		CorruptNpc = getFunctionAddr<_CorruptNpc>(moduleBase, 0x14f5380);
		CorruptedMovementGlitch = getFunctionAddr<_CorruptedMovementGlitch>(moduleBase, 0x14f53f0);
		CreateAiTree = getFunctionAddr<_CreateAiTree>(moduleBase, 0x14f5430);
		DeactivateWander = getFunctionAddr<_DeactivateWander>(moduleBase, 0x14f5670);
		DestroyAiTree = getFunctionAddr<_DestroyAiTree>(moduleBase, 0x14f5750);
		Dissipate = getFunctionAddr<_Dissipate>(moduleBase, 0x14f58c0);
		DoCorruptExplode = getFunctionAddr<_DoCorruptExplode>(moduleBase, 0x14f58d0);
		EndAnimatedDistraction = getFunctionAddr<_EndAnimatedDistraction>(moduleBase, 0x14f5a10);
		EndAnimatedStunned = getFunctionAddr<_EndAnimatedStunned>(moduleBase, 0x14f5a20);
		EndDormant = getFunctionAddr<_EndDormant>(moduleBase, 0x14f5a30);
		EndLurking = getFunctionAddr<_EndLurking>(moduleBase, 0x14f5a70);
		EndMimicry = getFunctionAddr<_EndMimicry>(moduleBase, 0x14f5ab0);
		EndPsiSuppressed = getFunctionAddr<_EndPsiSuppressed>(moduleBase, 0x14f5ac0);
		EndSearchExamine = getFunctionAddr<_EndSearchExamine>(moduleBase, 0x14f5b10);
		FearAOEOnPlayerKill = getFunctionAddr<_FearAOEOnPlayerKill>(moduleBase, 0x14f5b20);
		FindDeathInstigator = getFunctionAddr<_FindDeathInstigator>(moduleBase, 0x14f5d80);
		FindNearestReachablePoint = getFunctionAddr<_FindNearestReachablePoint>(moduleBase, 0x14f5df0);
		FindNearestValidPoint = getFunctionAddr<_FindNearestValidPoint>(moduleBase, 0x14f5ea0);
		FinishRaiseFromCorpse = getFunctionAddr<_FinishRaiseFromCorpse>(moduleBase, 0x14f5f50);
		FlowGraphDisableAi = getFunctionAddr<_FlowGraphDisableAi>(moduleBase, 0x14f5f80);
		FlowGraphIgnoreDistractions = getFunctionAddr<_FlowGraphIgnoreDistractions>(moduleBase, 0x14f6000);
		FlowGraphSetDumbed = getFunctionAddr<_FlowGraphSetDumbed>(moduleBase, 0x14f60f0);
		FlowgraphDisableCombatIntensity = getFunctionAddr<_FlowgraphDisableCombatIntensity>(moduleBase, 0x14f6160);
		FlowgraphEnableCombatIntensity = getFunctionAddr<_FlowgraphEnableCombatIntensity>(moduleBase, 0x14f61c0);
		FollowLeaderEntity = getFunctionAddr<_FollowLeaderEntity>(moduleBase, 0x14f61e0);
		ForceDesperationMimicryInPlace = getFunctionAddr<_ForceDesperationMimicryInPlace>(moduleBase, 0x14f62b0);
		ForceDownToSearchAttentionLevel = getFunctionAddr<_ForceDownToSearchAttentionLevel>(moduleBase, 0x14f64e0);
		ForceMovementReplan = getFunctionAddr<_ForceMovementReplan>(moduleBase, 0x14f6690);
		ForceReevaluateMovementAnimAction = getFunctionAddr<_ForceReevaluateMovementAnimAction>(moduleBase, 0x14f66a0);
		ForceStopCharacterEffect = getFunctionAddr<_ForceStopCharacterEffect>(moduleBase, 0x14f66d0);
		ForceStopDistraction = getFunctionAddr<_ForceStopDistraction>(moduleBase, 0x14f66e0);
		FullSerialize = getFunctionAddr<_FullSerialize>(moduleBase, 0x14f67b0);
		GetActiveEthericDoppelgangerId = getFunctionAddr<_GetActiveEthericDoppelgangerId>(moduleBase, 0x14f71f0);
		GetAmmoCount = getFunctionAddr<_GetAmmoCount>(moduleBase, 0x14f7200);
		GetAttachedPistolId = getFunctionAddr<_GetAttachedPistolId>(moduleBase, 0x14f7210);
		getAttachment = getFunctionAddr<_GetAttachment>(moduleBase, 0x14f7220);
		GetAttentionOnPlayer = getFunctionAddr<_GetAttentionOnPlayer>(moduleBase, 0x14f7270);
		GetCachedThrowAtLocation = getFunctionAddr<_GetCachedThrowAtLocation>(moduleBase, 0x14f72d0);
		GetCachedThrowAtType = getFunctionAddr<_GetCachedThrowAtType>(moduleBase, 0x14f72e0);
		GetCharacterEffectsManager = getFunctionAddr<_GetCharacterEffectsManager>(moduleBase, 0x14f72f0);
		GetComplexHearingGain = getFunctionAddr<_GetComplexHearingGain>(moduleBase, 0x14f7300);
		GetComplexVisionGain = getFunctionAddr<_GetComplexVisionGain>(moduleBase, 0x14f7310);
		GetCurrentAbilityContextId = getFunctionAddr<_GetCurrentAbilityContextId>(moduleBase, 0x14f7320);
		GetCurrentCombatRole = getFunctionAddr<_GetCurrentCombatRole>(moduleBase, 0x14f7340);
		GetDefaultFaction = getFunctionAddr<_GetDefaultFaction>(moduleBase, 0x14f7350);
		GetDesiredSpeed = getFunctionAddr<_GetDesiredSpeed>(moduleBase, 0x14f73f0);
		GetDodgeTimeStamp = getFunctionAddr<_GetDodgeTimeStamp>(moduleBase, 0x14f7400);
		GetEnergizedAmount = getFunctionAddr<_GetEnergizedAmount>(moduleBase, 0x14f7410);
		GetEntityPoolSignature = getFunctionAddr<_GetEntityPoolSignature>(moduleBase, 0x14f7480);
		GetEthericDoppelgangerOwnerId = getFunctionAddr<_GetEthericDoppelgangerOwnerId>(moduleBase, 0x14f74b0);
		GetGlooEffect = getFunctionAddr<_GetGlooEffect>(moduleBase, 0x14f74c0);
		GetHeadDirection = getFunctionAddr<_GetHeadDirection>(moduleBase, 0x14f7530);
		GetHeadPosition = getFunctionAddr<_GetHeadPosition>(moduleBase, 0x14f7550);
		GetHitReactKnockDownTimeStamp = getFunctionAddr<_GetHitReactKnockDownTimeStamp>(moduleBase, 0x14f7570);
		GetHitReactStaggerTimeStamp = getFunctionAddr<_GetHitReactStaggerTimeStamp>(moduleBase, 0x14f7580);
		GetIsHitReactingPrereqFlag = getFunctionAddr<_GetIsHitReactingPrereqFlag>(moduleBase, 0x14f7590);
		GetIsRegainingLosPrereqFlag = getFunctionAddr<_GetIsRegainingLosPrereqFlag>(moduleBase, 0x14f75a0);
		GetIsShuttingDown_Hack = getFunctionAddr<_GetIsShuttingDown_Hack>(moduleBase, 0x14f75b0);
		GetIsTakingDamagePrereqFlag = getFunctionAddr<_GetIsTakingDamagePrereqFlag>(moduleBase, 0x14f75c0);
		GetLastPerformedContextId = getFunctionAddr<_GetLastPerformedContextId>(moduleBase, 0x14f75d0);
		GetLeaderId = getFunctionAddr<_GetLeaderId>(moduleBase, 0x14f75e0);
		GetMaterialAnimationDuration = getFunctionAddr<_GetMaterialAnimationDuration>(moduleBase, 0x14f75f0);
		GetMaterialAnimationManager = getFunctionAddr<_GetMaterialAnimationManager>(moduleBase, 0x14f7600);
		GetMimicingEntityId = getFunctionAddr<_GetMimicingEntityId>(moduleBase, 0x14f7610);
		GetMimicryReason = getFunctionAddr<_GetMimicryReason>(moduleBase, 0x14f7620);
		GetMovementRequestId = getFunctionAddr<_GetMovementRequestId>(moduleBase, 0x14f7630);
		GetNpcManagerCombatPoints = getFunctionAddr<_GetNpcManagerCombatPoints>(moduleBase, 0x14f7650);
		GetNumControlledTurrets = getFunctionAddr<_GetNumControlledTurrets>(moduleBase, 0x14f7660);
		GetNumCorruptedNpcs = getFunctionAddr<_GetNumCorruptedNpcs>(moduleBase, 0x14f76e0);
		GetPhysicalEntitiesInBox = getFunctionAddr<_GetPhysicalEntitiesInBox>(moduleBase, 0x14f7700);
		GetProperties = getFunctionAddr<_GetProperties>(moduleBase, 0x14f77b0);
		GetRMIBase = getFunctionAddr<_GetRMIBase>(moduleBase, 0x14f77c0);
		GetSearchReasonFromChangeReason = getFunctionAddr<_GetSearchReasonFromChangeReason>(moduleBase, 0x14f77d0);
		GetTopAttentionTargetEntityId = getFunctionAddr<_GetTopAttentionTargetEntityId>(moduleBase, 0x14f77e0);
		GiveUpSearch = getFunctionAddr<_GiveUpSearch>(moduleBase, 0x14f77f0);
		Hack = getFunctionAddr<_Hack>(moduleBase, 0x14f7800);
		HandleEvent = getFunctionAddr<_HandleEvent>(moduleBase, 0x14f7870);
		HasActiveEthericDoppelganger = getFunctionAddr<_HasActiveEthericDoppelganger>(moduleBase, 0x14f79f0);
		HasFacingDesire = getFunctionAddr<_HasFacingDesire>(moduleBase, 0x14f7a30);
		HasHead = getFunctionAddr<_HasHead>(moduleBase, 0x14f7a40);
		HasLookDesire = getFunctionAddr<_HasLookDesire>(moduleBase, 0x14f7aa0);
		HasMaxAmmo = getFunctionAddr<_HasMaxAmmo>(moduleBase, 0x14f7ab0);
		HasMovementDesire = getFunctionAddr<_HasMovementDesire>(moduleBase, 0x14f7ac0);
		HasMovementRequest = getFunctionAddr<_HasMovementRequest>(moduleBase, 0x14f7ad0);
		HasPerformedNotice = getFunctionAddr<_HasPerformedNotice>(moduleBase, 0x14f7ae0);
		HasSpeedDesire = getFunctionAddr<_HasSpeedDesire>(moduleBase, 0x14f7af0);
		HideAttachment = getFunctionAddr<_HideAttachment>(moduleBase, 0x14f7b00);
		HideDisplayName = getFunctionAddr<_HideDisplayName>(moduleBase, 0x14f7bb0);
		Hypnotize = getFunctionAddr<_Hypnotize>(moduleBase, 0x14f7bf0);
		HypnotizeInterrupted = getFunctionAddr<_HypnotizeInterrupted>(moduleBase, 0x14f7c40);
		InTrackviewSequence = getFunctionAddr<_InTrackviewSequence>(moduleBase, 0x14f7c50);
		IncreaseAttentionLevelTowards = getFunctionAddr<_IncreaseAttentionLevelTowards>(moduleBase, 0x14f7c60);
		IncreaseGloo = getFunctionAddr<_IncreaseGloo>(moduleBase, 0x14f7e30);
		Init = getFunctionAddr<_Init>(moduleBase, 0x14f7f20);
		InitLiftVolumeManager = getFunctionAddr<_InitLiftVolumeManager>(moduleBase, 0x14f7fc0);
		InitMimicableClasses = getFunctionAddr<_InitMimicableClasses>(moduleBase, 0x14f8050);
		InitializeNpc = getFunctionAddr<_InitializeNpc>(moduleBase, 0x14f8290);
		InitializeTrackviewTempNpc = getFunctionAddr<_InitializeTrackviewTempNpc>(moduleBase, 0x14f8340);
		InterruptAbility = getFunctionAddr<_InterruptAbility>(moduleBase, 0x14f8360);
		InterruptHitReactShift = getFunctionAddr<_InterruptHitReactShift>(moduleBase, 0x14f83b0);
		InterruptRaiseFromCorpse = getFunctionAddr<_InterruptRaiseFromCorpse>(moduleBase, 0x14f83c0);
		InterruptUnanimatedAbility = getFunctionAddr<_InterruptUnanimatedAbility>(moduleBase, 0x14f83d0);
		IsAlert = getFunctionAddr<_IsAlert>(moduleBase, 0x14f83e0);
		IsAsleep = getFunctionAddr<_IsAsleep>(moduleBase, 0x14f83f0);
		IsAttentionProxyFollowing = getFunctionAddr<_IsAttentionProxyFollowing>(moduleBase, 0x14f8400);
		IsBreakable = getFunctionAddr<_IsBreakable>(moduleBase, 0x14f8470);
		IsBroken = getFunctionAddr<_IsBroken>(moduleBase, 0x14f8490);
		IsCharacterEffectEnabled = getFunctionAddr<_IsCharacterEffectEnabled>(moduleBase, 0x14f84a0);
		IsCorrupted = getFunctionAddr<_IsCorrupted>(moduleBase, 0x14f84b0);
		IsCowering = getFunctionAddr<_IsCowering>(moduleBase, 0x14f84d0);
		IsDead = getFunctionAddr<_IsDead>(moduleBase, 0x14f84e0);
		IsDestroyed = getFunctionAddr<_IsDestroyed>(moduleBase, 0x14f8510);
		IsDissipating = getFunctionAddr<_IsDissipating>(moduleBase, 0x14f8520);
		IsDormant = getFunctionAddr<_IsDormant>(moduleBase, 0x14f8530);
		IsEntityMimicable = getFunctionAddr<_IsEntityMimicable>(moduleBase, 0x14f8540);
		IsEthericDoppelganger = getFunctionAddr<_IsEthericDoppelganger>(moduleBase, 0x14f86c0);
		IsFalling = getFunctionAddr<_IsFalling>(moduleBase, 0x14f86d0);
		IsFeared = getFunctionAddr<_IsFeared>(moduleBase, 0x14f86e0);
		IsFollowingPlayer = getFunctionAddr<_IsFollowingPlayer>(moduleBase, 0x14f8750);
		IsGlooSlow = getFunctionAddr<_IsGlooSlow>(moduleBase, 0x14f8760);
		IsGoingToFreezeInGloo = getFunctionAddr<_IsGoingToFreezeInGloo>(moduleBase, 0x14f8780);
		IsHacked = getFunctionAddr<_IsHacked>(moduleBase, 0x14f8810);
		IsIgnoringRangedAbilities = getFunctionAddr<_IsIgnoringRangedAbilities>(moduleBase, 0x14f8830);
		IsInRaiseFromCorpseAnim = getFunctionAddr<_IsInRaiseFromCorpseAnim>(moduleBase, 0x14f8840);
		IsJumping = getFunctionAddr<_IsJumping>(moduleBase, 0x14f8850);
		IsLureFlareValid = getFunctionAddr<_IsLureFlareValid>(moduleBase, 0x14f8860);
		IsLurking = getFunctionAddr<_IsLurking>(moduleBase, 0x14f8a70);
		IsMimic = getFunctionAddr<_IsMimic>(moduleBase, 0x14f8a80);
		IsMimicking = getFunctionAddr<_IsMimicking>(moduleBase, 0x14f8b90);
		IsMindControlled = getFunctionAddr<_IsMindControlled>(moduleBase, 0x14f8ba0);
		IsMovementAnimated = getFunctionAddr<_IsMovementAnimated>(moduleBase, 0x14f8c10);
		IsMovementDesireExecuting = getFunctionAddr<_IsMovementDesireExecuting>(moduleBase, 0x14f8c20);
		IsMovementInterruptible = getFunctionAddr<_IsMovementInterruptible>(moduleBase, 0x14f8ce0);
		IsNpcHealthUIDisabled = getFunctionAddr<_IsNpcHealthUIDisabled>(moduleBase, 0x14f8d20);
		IsPerformingAbility = getFunctionAddr<_IsPerformingAbility>(moduleBase, 0x14f8d30);
		IsPlayerControlled = getFunctionAddr<_IsPlayerControlled>(moduleBase, 0x14f8d50);
		IsPsiLifted = getFunctionAddr<_IsPsiLifted>(moduleBase, 0x14f8d70);
		IsPsiSuppressed = getFunctionAddr<_IsPsiSuppressed>(moduleBase, 0x14f8d80);
		IsRagdolled = getFunctionAddr<_IsRagdolled>(moduleBase, 0x14f8d90);
		IsResisting = getFunctionAddr<_IsResisting>(moduleBase, 0x14f8da0);
		IsScrunched = getFunctionAddr<_IsScrunched>(moduleBase, 0x14f8db0);
		IsShifting = getFunctionAddr<_IsShifting>(moduleBase, 0x14f8dc0);
		IsStandingUp = getFunctionAddr<_IsStandingUp>(moduleBase, 0x14f8dd0);
		IsStunned = getFunctionAddr<_IsStunned>(moduleBase, 0x14f8de0);
		IsVisible = getFunctionAddr<_IsVisible>(moduleBase, 0x14f8e00);
		IsVulnerable = getFunctionAddr<_IsVulnerable>(moduleBase, 0x14f8e80);
		IsWithinCameraViewCone = getFunctionAddr<_IsWithinCameraViewCone>(moduleBase, 0x14f8ee0);
		IsXRayTrackingTarget = getFunctionAddr<_IsXRayTrackingTarget>(moduleBase, 0x14f9030);
		LightOnFire = getFunctionAddr<_LightOnFire>(moduleBase, 0x14f9060);
		LoadScriptVariables = getFunctionAddr<_LoadScriptVariables>(moduleBase, 0x14f90d0);
		LockKnownAttentionOnTarget = getFunctionAddr<_LockKnownAttentionOnTarget>(moduleBase, 0x14f9670);
		LockMimicry = getFunctionAddr<_LockMimicry>(moduleBase, 0x14f96e0);
		MakeConscious = getFunctionAddr<_MakeConscious>(moduleBase, 0x14f96f0);
		MakePoltergeistAppear = getFunctionAddr<_MakePoltergeistAppear>(moduleBase, 0x14f9700);
		MakePoltergeistDisapear = getFunctionAddr<_MakePoltergeistDisapear>(moduleBase, 0x14f9770);
		MimicAndReplaceEntity = getFunctionAddr<_MimicAndReplaceEntity>(moduleBase, 0x14f97e0);
		MimicCollisionRebound = getFunctionAddr<_MimicCollisionRebound>(moduleBase, 0x14f97f0);
		MimicEntity = getFunctionAddr<_MimicEntity>(moduleBase, 0x14f9b90);
		NoticeTargetPosition = getFunctionAddr<_NoticeTargetPosition>(moduleBase, 0x14f9bc0);
		NotifyCorruptedOwnerOfUncorruption = getFunctionAddr<_NotifyCorruptedOwnerOfUncorruption>(moduleBase, 0x14f9c50);
		OnAnimatedMovement_Start = getFunctionAddr<_OnAnimatedMovement_Start>(moduleBase, 0x14f9d60);
		OnAnimatedMovement_Stop = getFunctionAddr<_OnAnimatedMovement_Stop>(moduleBase, 0x14f9d70);
		OnAttack = getFunctionAddr<_OnAttack>(moduleBase, 0x14f9d80);
		OnAttentionLevelChanged = getFunctionAddr<_OnAttentionLevelChanged>(moduleBase, 0x14f9e20);
		OnAttentionProxyUpdated = getFunctionAddr<_OnAttentionProxyUpdated>(moduleBase, 0x14fa5e0);
		OnBeginDormant = getFunctionAddr<_OnBeginDormant>(moduleBase, 0x14fa6a0);
		OnCallMimicsReceived = getFunctionAddr<_OnCallMimicsReceived>(moduleBase, 0x14fa770);
		OnCallNpcsCallerDead = getFunctionAddr<_OnCallNpcsCallerDead>(moduleBase, 0x14fa830);
		OnCallNpcsEnded = getFunctionAddr<_OnCallNpcsEnded>(moduleBase, 0x14fa920);
		OnCallNpcsReceived = getFunctionAddr<_OnCallNpcsReceived>(moduleBase, 0x14fa9f0);
		OnCancelGuard = getFunctionAddr<_OnCancelGuard>(moduleBase, 0x14faab0);
		OnChargeTarget = getFunctionAddr<_OnChargeTarget>(moduleBase, 0x14fab90);
		OnCollisionSignal = getFunctionAddr<_OnCollisionSignal>(moduleBase, 0x14fac50);
		OnCombatBegin = getFunctionAddr<_OnCombatBegin>(moduleBase, 0x14fac70);
		OnCombatEnd = getFunctionAddr<_OnCombatEnd>(moduleBase, 0x14faca0);
		OnCorruptNpcRequest = getFunctionAddr<_OnCorruptNpcRequest>(moduleBase, 0x14fad10);
		OnCreatePhantomCancel = getFunctionAddr<_OnCreatePhantomCancel>(moduleBase, 0x14fadf0);
		OnCreatePhantomFail = getFunctionAddr<_OnCreatePhantomFail>(moduleBase, 0x14faec0);
		OnCreatePhantomRequest = getFunctionAddr<_OnCreatePhantomRequest>(moduleBase, 0x14faf10);
		OnCreatePhantomSuccess = getFunctionAddr<_OnCreatePhantomSuccess>(moduleBase, 0x14fb030);
		OnDealtDamage = getFunctionAddr<_OnDealtDamage>(moduleBase, 0x14fb0a0);
		OnDesiredRoleChanged = getFunctionAddr<_OnDesiredRoleChanged>(moduleBase, 0x14fb160);
		OnEndDormant = getFunctionAddr<_OnEndDormant>(moduleBase, 0x14fb220);
		OnEnergizedExplosion = getFunctionAddr<_OnEnergizedExplosion>(moduleBase, 0x14fb2f0);
		OnEnteredTrackviewBodyState = getFunctionAddr<_OnEnteredTrackviewBodyState>(moduleBase, 0x14fb360);
		OnEntityEvent = getFunctionAddr<_OnEntityEvent>(moduleBase, 0x14fb430);
		OnFearStopped = getFunctionAddr<_OnFearStopped>(moduleBase, 0x14fb620);
		OnFearedRefreshed = getFunctionAddr<_OnFearedRefreshed>(moduleBase, 0x14fb730);
		OnFearedStarted = getFunctionAddr<_OnFearedStarted>(moduleBase, 0x14fb800);
		OnGlooBroken = getFunctionAddr<_OnGlooBroken>(moduleBase, 0x14fb970);
		OnGlooFrozen = getFunctionAddr<_OnGlooFrozen>(moduleBase, 0x14fb9d0);
		OnGravShaftEnter = getFunctionAddr<_OnGravShaftEnter>(moduleBase, 0x14fbaf0);
		OnGravShaftExit = getFunctionAddr<_OnGravShaftExit>(moduleBase, 0x14fbb40);
		OnGuardInPosition = getFunctionAddr<_OnGuardInPosition>(moduleBase, 0x14fbc30);
		OnHealthChanged = getFunctionAddr<_OnHealthChanged>(moduleBase, 0x14fbc60);
		OnHit = getFunctionAddr<_OnHit>(moduleBase, 0x14fbcc0);
		OnHitFloor = getFunctionAddr<_OnHitFloor>(moduleBase, 0x14fc120);
		OnInteraction = getFunctionAddr<_OnInteraction>(moduleBase, 0x14fc3c0);
		OnJumpAnimActionSequence_Start = getFunctionAddr<_OnJumpAnimActionSequence_Start>(moduleBase, 0x14fc430);
		OnJumpAnimActionSequence_Stop = getFunctionAddr<_OnJumpAnimActionSequence_Stop>(moduleBase, 0x14fc4a0);
		OnJumpAnimAction_Fall = getFunctionAddr<_OnJumpAnimAction_Fall>(moduleBase, 0x14fc510);
		OnKill = getFunctionAddr<_OnKill>(moduleBase, 0x14fc520);
		OnKillHitReaction = getFunctionAddr<_OnKillHitReaction>(moduleBase, 0x14fc8f0);
		OnKnockdownHitReaction = getFunctionAddr<_OnKnockdownHitReaction>(moduleBase, 0x14fc930);
		OnLeaderDamaged = getFunctionAddr<_OnLeaderDamaged>(moduleBase, 0x14fc9a0);
		OnLostAttentionTarget = getFunctionAddr<_OnLostAttentionTarget>(moduleBase, 0x14fc9c0);
		OnMantleAnimAction_Start = getFunctionAddr<_OnMantleAnimAction_Start>(moduleBase, 0x14fcbd0);
		OnMantleAnimAction_Stop = getFunctionAddr<_OnMantleAnimAction_Stop>(moduleBase, 0x14fcbe0);
		OnNewAttentionTarget = getFunctionAddr<_OnNewAttentionTarget>(moduleBase, 0x14fcbf0);
		OnPatrolCancel = getFunctionAddr<_OnPatrolCancel>(moduleBase, 0x14fcd70);
		OnPatrolEnd = getFunctionAddr<_OnPatrolEnd>(moduleBase, 0x14fce60);
		OnPatrolRequest = getFunctionAddr<_OnPatrolRequest>(moduleBase, 0x14fcea0);
		OnPatrolRequestClosest = getFunctionAddr<_OnPatrolRequestClosest>(moduleBase, 0x14fcf50);
		OnPatrolRequestUpdate = getFunctionAddr<_OnPatrolRequestUpdate>(moduleBase, 0x14fcfe0);
		OnPsiLiftedBegin = getFunctionAddr<_OnPsiLiftedBegin>(moduleBase, 0x14fd060);
		OnPsiLiftedEnd = getFunctionAddr<_OnPsiLiftedEnd>(moduleBase, 0x14fd070);
		OnPsiTargetingStart = getFunctionAddr<_OnPsiTargetingStart>(moduleBase, 0x14fd080);
		OnPsiTargetingStop = getFunctionAddr<_OnPsiTargetingStop>(moduleBase, 0x14fd100);
		OnReactionComplete = getFunctionAddr<_OnReactionComplete>(moduleBase, 0x14fd180);
		OnRecycled = getFunctionAddr<_OnRecycled>(moduleBase, 0x14fd190);
		OnRequestGuard = getFunctionAddr<_OnRequestGuard>(moduleBase, 0x14fd250);
		MakeUnconscious = getFunctionAddr<_MakeUnconscious>(moduleBase, 0x14fd340);
		OnScriptWakeFromDormant = getFunctionAddr<_OnScriptWakeFromDormant>(moduleBase, 0x14fd350);
		OnSearchExamineFinished = getFunctionAddr<_OnSearchExamineFinished>(moduleBase, 0x14fd380);
		OnSmallHitReaction = getFunctionAddr<_OnSmallHitReaction>(moduleBase, 0x14fd450);
		OnStaggerHitReaction = getFunctionAddr<_OnStaggerHitReaction>(moduleBase, 0x14fd500);
		OnStartConversation = getFunctionAddr<_OnStartConversation>(moduleBase, 0x14fd570);
		OnStartFlareHypnotize = getFunctionAddr<_OnStartFlareHypnotize>(moduleBase, 0x14fd760);
		OnStartLookingAt = getFunctionAddr<_OnStartLookingAt>(moduleBase, 0x14fd910);
		OnStartScriptMove = getFunctionAddr<_OnStartScriptMove>(moduleBase, 0x14fd970);
		OnStartedMimicking = getFunctionAddr<_OnStartedMimicking>(moduleBase, 0x14fda40);
		OnStartedMusicInactiveBehavior = getFunctionAddr<_OnStartedMusicInactiveBehavior>(moduleBase, 0x14fdd40);
		OnStopConversation = getFunctionAddr<_OnStopConversation>(moduleBase, 0x14fde10);
		OnStopFlareHypnotize = getFunctionAddr<_OnStopFlareHypnotize>(moduleBase, 0x14fde80);
		OnStopScriptMove = getFunctionAddr<_OnStopScriptMove>(moduleBase, 0x14fdf60);
		OnStoppedMimicking = getFunctionAddr<_OnStoppedMimicking>(moduleBase, 0x14fe030);
		OnStoppedMusicInactiveBehavior = getFunctionAddr<_OnStoppedMusicInactiveBehavior>(moduleBase, 0x14fe2b0);
		OnSurprised = getFunctionAddr<_OnSurprised>(moduleBase, 0x14fe380);
		OnTargetDead = getFunctionAddr<_OnTargetDead>(moduleBase, 0x14fe390);
		OnTargetStartedMimicry = getFunctionAddr<_OnTargetStartedMimicry>(moduleBase, 0x14fe4a0);
		OnTargetStoppedMimicry = getFunctionAddr<_OnTargetStoppedMimicry>(moduleBase, 0x14fe5b0);
		OnTrackviewStateExit = getFunctionAddr<_OnTrackviewStateExit>(moduleBase, 0x14fe670);
		OnUnCorrupted = getFunctionAddr<_OnUnCorrupted>(moduleBase, 0x14fe700);
		OnUsePower = getFunctionAddr<_OnUsePower>(moduleBase, 0x14fe730);
		OnWanderBecomeActive = getFunctionAddr<_OnWanderBecomeActive>(moduleBase, 0x14fe750);
		OnWanderBecomeInactive = getFunctionAddr<_OnWanderBecomeInactive>(moduleBase, 0x14fe760);
		OnWanderWaiting = getFunctionAddr<_OnWanderWaiting>(moduleBase, 0x14fe770);
		OnWrenchAttack = getFunctionAddr<_OnWrenchAttack>(moduleBase, 0x14fe840);
		OnZeroDamageHit = getFunctionAddr<_OnZeroDamageHit>(moduleBase, 0x14fe850);
		OutOfAmmo = getFunctionAddr<_OutOfAmmo>(moduleBase, 0x14fe900);
		OverrideCharacterId = getFunctionAddr<_OverrideCharacterId>(moduleBase, 0x14fe910);
		OverrideHitReactAnimation = getFunctionAddr<_OverrideHitReactAnimation>(moduleBase, 0x14feaa0);
		PerformAnimatedAbility = getFunctionAddr<_PerformAnimatedAbility>(moduleBase, 0x14feb00);
		PerformCombatReaction = getFunctionAddr<_PerformCombatReaction>(moduleBase, 0x14Feb10);
		PerformFatality = getFunctionAddr<_PerformFatality>(moduleBase, 0x14feb80);
		PerformHitReactShift = getFunctionAddr<_PerformHitReactShift>(moduleBase, 0x14feb90);
		PerformMimicGlitch = getFunctionAddr<_PerformMimicGlitch>(moduleBase, 0x14feba0);
		PerformMimicReorientation = getFunctionAddr<_PerformMimicReorientation>(moduleBase, 0x14fec20);
		PerformMindControlled = getFunctionAddr<_PerformMindControlled>(moduleBase, 0x14fed40);
		PerformPassiveGlooBreak = getFunctionAddr<_PerformPassiveGlooBreak>(moduleBase, 0x14fedf0);
		PerformPatrolIdle = getFunctionAddr<_PerformPatrolIdle>(moduleBase, 0x14fee20);
		PerformPostSerializeGloodPose = getFunctionAddr<_PerformPostSerializeGloodPose>(moduleBase, 0x14fee30);
		PerformReorientation = getFunctionAddr<_PerformReorientation>(moduleBase, 0x14fee40);
		PerformScan = getFunctionAddr<_PerformScan>(moduleBase, 0x14ff280);
		PerformStopMindControlled = getFunctionAddr<_PerformStopMindControlled>(moduleBase, 0x14ff290);
		PerformUnanimatedAbility = getFunctionAddr<_PerformUnanimatedAbility>(moduleBase, 0x14ff2a0);
		PerformWanderIdle = getFunctionAddr<_PerformWanderIdle>(moduleBase, 0x14ff2b0);
		PlayMimicHitEffect = getFunctionAddr<_PlayMimicHitEffect>(moduleBase, 0x14ff2c0);
		PlayMimicryMorphInEffect = getFunctionAddr<_PlayMimicryMorphInEffect>(moduleBase, 0x14ff460);
		PlayMimicryMorphOutEffect = getFunctionAddr<_PlayMimicryMorphOutEffect>(moduleBase, 0x14ff470);
		PopAIAlwaysUpdate = getFunctionAddr<_PopAIAlwaysUpdate>(moduleBase, 0x14ff4f0);
		PopAttentionDrainLockOnTarget = getFunctionAddr<_PopAttentionDrainLockOnTarget>(moduleBase, 0x14ff560);
		PopDisableAiTree = getFunctionAddr<_PopDisableAiTree>(moduleBase, 0x14ff5d0);
		PopDisableAllAmbientSounds = getFunctionAddr<_PopDisableAllAmbientSounds>(moduleBase, 0x14ff660);
		PopDisableAttentionAndSenses = getFunctionAddr<_PopDisableAttentionAndSenses>(moduleBase, 0x14ff6e0);
		PopDisableAttentionObjectAndPerceivables = getFunctionAddr<_PopDisableAttentionObjectAndPerceivables>(moduleBase, 0x14ff7b0);
		PopDisableAudible = getFunctionAddr<_PopDisableAudible>(moduleBase, 0x14ff860);
		PopDisableBreakable = getFunctionAddr<_PopDisableBreakable>(moduleBase, 0x14ff8a0);
		PopDisableHearing = getFunctionAddr<_PopDisableHearing>(moduleBase, 0x14ff8b0);
		PopDisableHitReactions = getFunctionAddr<_PopDisableHitReactions>(moduleBase, 0x14ff8f0);
		PopDisableLifetimeEffect = getFunctionAddr<_PopDisableLifetimeEffect>(moduleBase, 0x14ff900);
		PopDisableNpcHealthUI = getFunctionAddr<_PopDisableNpcHealthUI>(moduleBase, 0x14ff940);
		PopDisableOperatorLevitatorsEffect = getFunctionAddr<_PopDisableOperatorLevitatorsEffect>(moduleBase, 0x14ff950);
		PopDisableSenses = getFunctionAddr<_PopDisableSenses>(moduleBase, 0x14ff990);
		PopDisableVisible = getFunctionAddr<_PopDisableVisible>(moduleBase, 0x14ffa40);
		PopDisableVision = getFunctionAddr<_PopDisableVision>(moduleBase, 0x14ffa80);
		PopDumbed = getFunctionAddr<_PopDumbed>(moduleBase, 0x14ffac0);
		PopEnableAbilities = getFunctionAddr<_PopEnableAbilities>(moduleBase, 0x14ffad0);
		PopEnableAiTree = getFunctionAddr<_PopEnableAiTree>(moduleBase, 0x14ffae0);
		PopEnableAttentionObject = getFunctionAddr<_PopEnableAttentionObject>(moduleBase, 0x14ffb70);
		PopEnableAttentiveSubject = getFunctionAddr<_PopEnableAttentiveSubject>(moduleBase, 0x14ffbb0);
		PopEnableAudible = getFunctionAddr<_PopEnableAudible>(moduleBase, 0x14ffbf0);
		PopEnableGlassBreaking = getFunctionAddr<_PopEnableGlassBreaking>(moduleBase, 0x14ffc30);
		PopEnableHearing = getFunctionAddr<_PopEnableHearing>(moduleBase, 0x14ffc70);
		PopEnableLifetimeEffect = getFunctionAddr<_PopEnableLifetimeEffect>(moduleBase, 0x14ffcb0);
		PopEnableOperatorLevitatorsEffect = getFunctionAddr<_PopEnableOperatorLevitatorsEffect>(moduleBase, 0x14ffd10);
		PopEnableRoomPerceiver = getFunctionAddr<_PopEnableRoomPerceiver>(moduleBase, 0x14ffd70);
		PopEnableVisible = getFunctionAddr<_PopEnableVisible>(moduleBase, 0x14ffdb0);
		PopEnableVision = getFunctionAddr<_PopEnableVision>(moduleBase, 0x14ffdf0);
		PopEnabledAmbientSound = getFunctionAddr<_PopEnabledAmbientSound>(moduleBase, 0x14ffe50);
		PopFear = getFunctionAddr<_PopFear>(moduleBase, 0x14ffee0);
		PopForceRigidOnGloo = getFunctionAddr<_PopForceRigidOnGloo>(moduleBase, 0x14fff60);
		PopIndefiniteRagdoll = getFunctionAddr<_PopIndefiniteRagdoll>(moduleBase, 0x14fff70);
		PopulateInteractionInfo = getFunctionAddr<_PopulateInteractionInfo>(moduleBase, 0x14fff80);
		PopulateRemoteManipulationInteraction = getFunctionAddr<_PopulateRemoteManipulationInteraction>(moduleBase, 0x14ffff0);
		PostInit = getFunctionAddr<_PostInit>(moduleBase, 0x1500090);
		PostSerialize = getFunctionAddr<_PostSerialize>(moduleBase, 0x1500300);
		Precache = getFunctionAddr<_Precache>(moduleBase, 0x1500700);
		ProcessDoneEvent = getFunctionAddr<_ProcessDoneEvent>(moduleBase, 0x1500a60);
		ProcessEvent = getFunctionAddr<_ProcessEvent>(moduleBase, 0x1500cf0);
		ProcessStartGameEvent = getFunctionAddr<_ProcessStartGameEvent>(moduleBase, 0x1501430);
		PushAIAlwaysUpdate = getFunctionAddr<_PushAIAlwaysUpdate>(moduleBase, 0x1501750);
		PushAIAlwaysUpdateForPatrol = getFunctionAddr<_PushAIAlwaysUpdateForPatrol>(moduleBase, 0x15017b0);
		PushAttentionDrainLockOnTarget = getFunctionAddr<_PushAttentionDrainLockOnTarget>(moduleBase, 0x1501830);
		PushDisableAiTree = getFunctionAddr<_PushDisableAiTree>(moduleBase, 0x1501890);
		PushDisableAllAmbientSounds = getFunctionAddr<_PushDisableAllAmbientSounds>(moduleBase, 0x1501920);
		PushDisableAttentionAndSenses = getFunctionAddr<_PushDisableAttentionAndSenses>(moduleBase, 0x15019c0);
		PushDisableAttentionObjectAndPerceivables = getFunctionAddr<_PushDisableAttentionObjectAndPerceivables>(moduleBase, 0x1501a60);
		PushDisableAudible = getFunctionAddr<_PushDisableAudible>(moduleBase, 0x1501ae0);
		PushDisableBreakable = getFunctionAddr<_PushDisableBreakable>(moduleBase, 0x1501b20);
		PushDisableDeathReactions = getFunctionAddr<_PushDisableDeathReactions>(moduleBase, 0x1501b30);
		PushDisableHearing = getFunctionAddr<_PushDisableHearing>(moduleBase, 0x1501b40);
		PushDisableHitReactions = getFunctionAddr<_PushDisableHitReactions>(moduleBase, 0x1501b80);
		PushDisableLifetimeEffect = getFunctionAddr<_PushDisableLifetimeEffect>(moduleBase, 0x1501b90);
		PushDisableNpcHealthUI = getFunctionAddr<_PushDisableNpcHealthUI>(moduleBase, 0x1501be0);
		PushDisableOperatorLevitatorsEffect = getFunctionAddr<_PushDisableOperatorLevitatorsEffect>(moduleBase, 0x1501bf0);
		PushDisableSenses = getFunctionAddr<_PushDisableSenses>(moduleBase, 0x1501c40);
		PushDisableVisible = getFunctionAddr<_PushDisableVisible>(moduleBase, 0x1501cc0);
		PushDisableVision = getFunctionAddr<_PushDisableVision>(moduleBase, 0x1501d00);
		PushDumbed = getFunctionAddr<_PushDumbed>(moduleBase, 0x1501d40);
		PushEnableAbilities = getFunctionAddr<_PushEnableAbilities>(moduleBase, 0x1501d60);
		PushEnableAiTree = getFunctionAddr<_PushEnableAiTree>(moduleBase, 0x1501d70);
		PushEnableAttentionObject = getFunctionAddr<_PushEnableAttentionObject>(moduleBase, 0x14ffb70);
		PushEnableAttentiveSubject = getFunctionAddr<_PushEnableAttentiveSubject>(moduleBase, 0x14ffbb0);
		PushEnableAudible = getFunctionAddr<_PushEnableAudible>(moduleBase, 0x14ffbf0);
		PushEnableGlassBreaking = getFunctionAddr<_PushEnableGlassBreaking>(moduleBase, 0x1501ec0);
		PushEnableHearing = getFunctionAddr<_PushEnableHearing>(moduleBase, 0x1501f10);
		PushEnableLifetimeEffect = getFunctionAddr<_PushEnableLifetimeEffect>(moduleBase, 0x1501f50);
		PushEnableOperatorLevitatorsEffect = getFunctionAddr<_PushEnableOperatorLevitatorsEffect>(moduleBase, 0x1501fb0);
		PushEnableRoomPerceiver = getFunctionAddr<_PushEnableRoomPerceiver>(moduleBase, 0x1502010);
		PushEnableVisible = getFunctionAddr<_PushEnableVisible>(moduleBase, 0x1502050);
		PushEnableVision = getFunctionAddr<_PushEnableVision>(moduleBase, 0x1502090);
		PushEnabledAmbientSound = getFunctionAddr<_PushEnabledAmbientSound>(moduleBase, 0x15020d0);
		PushFear = getFunctionAddr<_PushFear>(moduleBase, 0x1502140);
		PushForceRigidOnGloo = getFunctionAddr<_PushForceRigidOnGloo>(moduleBase, 0x1502200);
		PushIndefiniteRagdoll = getFunctionAddr<_PushIndefiniteRagdoll>(moduleBase, 0x1502210);
		QueueStimulus_CallForHelp = getFunctionAddr<_QueueStimulus_CallForHelp>(moduleBase, 0x15022e0);
		QueueStimulus_Diagnose = getFunctionAddr<_QueueStimulus_Diagnose>(moduleBase, 0x15023c0);
		QueueStimulus_Escape = getFunctionAddr<_QueueStimulus_Escape>(moduleBase, 0x1502490);
		QueueStimulus_FrightfulNoise = getFunctionAddr<_QueueStimulus_FrightfulNoise>(moduleBase, 0x1502560);
		QueueStimulus_MovementEvent = getFunctionAddr<_QueueStimulus_MovementEvent>(moduleBase, 0x1502640);
		Ragdoll = getFunctionAddr<_Ragdoll>(moduleBase, 0x1502720);
		RefreshCombatIntensity = getFunctionAddr<_RefreshCombatIntensity>(moduleBase, 0x15027f0);
		RefreshNotice = getFunctionAddr<_RefreshNotice>(moduleBase, 0x1502880);
		RegisterOnFleeNodeCallsbacks = getFunctionAddr<_RegisterOnFleeNodeCallsbacks>(moduleBase, 0x1502890);
		Release = getFunctionAddr<_Release>(moduleBase, 0x15028c0);
		ReloadAmmo = getFunctionAddr<_ReloadAmmo>(moduleBase, 0x1502970);
		RemoveAmmoAmount = getFunctionAddr<_RemoveAmmoAmount>(moduleBase, 0x1502980);
		RemoveCollisionObserver = getFunctionAddr<_RemoveCollisionObserver>(moduleBase, 0x1502990);
		RemoveFacingDesire = getFunctionAddr<_RemoveFacingDesire>(moduleBase, 0x15029d0);
		RemoveHead = getFunctionAddr<_RemoveHead>(moduleBase, 0x1502a60);
		RemoveInboundModifier = getFunctionAddr<_RemoveInboundModifier>(moduleBase, 0x1502b40);
		RemoveLookDesire = getFunctionAddr<_RemoveLookDesire>(moduleBase, 0x1502b80);
		RemoveMovementDesire = getFunctionAddr<_RemoveMovementDesire>(moduleBase, 0x1502c10);
		RemoveNoiseStateDesire = getFunctionAddr<_RemoveNoiseStateDesire>(moduleBase, 0x1502ca0);
		RemoveSpeedDesire = getFunctionAddr<_RemoveSpeedDesire>(moduleBase, 0x1502d30);
		RequestDistraction = getFunctionAddr<_RequestDistraction>(moduleBase, 0x1502dc0);
		RequestLurk = getFunctionAddr<_RequestLurk>(moduleBase, 0x1502ea0);
		RequestStopLurk = getFunctionAddr<_RequestStopLurk>(moduleBase, 0x1502f80);
		ResetAIAlwaysUpdate = getFunctionAddr<_ResetAIAlwaysUpdate>(moduleBase, 0x1503050);
		ResetPlayerBumpTimer = getFunctionAddr<_ResetPlayerBumpTimer>(moduleBase, 0x15030b0);
		Resist = getFunctionAddr<_Resist>(moduleBase, 0x15030c0);
		ScaleLookDesireSpeed = getFunctionAddr<_ScaleLookDesireSpeed>(moduleBase, 0x1503140);
		Serialize = getFunctionAddr<_Serialize>(moduleBase, 0x1503150);
		SetAnimTag = getFunctionAddr<_SetAnimTag>(moduleBase, 0x15034d0);
		SetBreakGlooEntity = getFunctionAddr<_SetBreakGlooEntity>(moduleBase, 0x1503620);
		SetCachedThrowAtLocation = getFunctionAddr<_SetCachedThrowAtLocation>(moduleBase, 0x1503630);
		SetComplexHearingGain = getFunctionAddr<_SetComplexHearingGain>(moduleBase, 0x1503650);
		SetComplexVisionGain = getFunctionAddr<_SetComplexVisionGain>(moduleBase, 0x1503660);
		SetConversationFacingTarget = getFunctionAddr<_SetConversationFacingTarget>(moduleBase, 0x1503670);
		SetConversationLookTarget = getFunctionAddr<_SetConversationLookTarget>(moduleBase, 0x1503780);
		SetCurrentCombatRole = getFunctionAddr<_SetCurrentCombatRole>(moduleBase, 0x1503900);
		SetEthericDoppelgangerId = getFunctionAddr<_SetEthericDoppelgangerId>(moduleBase, 0x15039e0);
		SetEthericDoppengangerOwnerId = getFunctionAddr<_SetEthericDoppengangerOwnerId>(moduleBase, 0x15039f0);
		SetFearFromHostileDamage = getFunctionAddr<_SetFearFromHostileDamage>(moduleBase, 0x1503a00);
		SetHasPerformedNotice = getFunctionAddr<_SetHasPerformedNotice>(moduleBase, 0x1503a70);
		SetHypnotizeStartTimeStamp = getFunctionAddr<_SetHypnotizeStartTimeStamp>(moduleBase, 0x1503ab0);
		SetIsEthericDoppelganger = getFunctionAddr<_SetIsEthericDoppelganger>(moduleBase, 0x1503ae0);
		SetIsHitReactingPrereqFlag = getFunctionAddr<_SetIsHitReactingPrereqFlag>(moduleBase, 0x1503af0);
		SetIsIgnoringRangedAbilities = getFunctionAddr<_SetIsIgnoringRangedAbilities>(moduleBase, 0x1503b00);
		SetIsPlayerControlled = getFunctionAddr<_SetIsPlayerControlled>(moduleBase, 0x1503b10);
		SetIsRegainingLoSPrereqFlag = getFunctionAddr<_SetIsRegainingLoSPrereqFlag>(moduleBase, 0x1503b80);
		SetPistolHidden = getFunctionAddr<_SetPistolHidden>(moduleBase, 0x1503b90);
		SetShouldSkipNextCombatReaction = getFunctionAddr<_SetShouldSkipNextCombatReaction>(moduleBase, 0x1503bd0);
		SetSpawnedState = getFunctionAddr<_SetSpawnedState>(moduleBase, 0x1503be0);
		SetStance = getFunctionAddr<_SetStance>(moduleBase, 0x1503c90);
		SetTimeUntilDeath = getFunctionAddr<_SetTimeUntilDeath>(moduleBase, 0x1503cd0);
		SetTransitionStateTag = getFunctionAddr<_SetTransitionStateTag>(moduleBase, 0x1503ce0);
		SetWanderAreaContainerEntity = getFunctionAddr<_SetWanderAreaContainerEntity>(moduleBase, 0x1503d10);
		SetWanderFollowEntityId = getFunctionAddr<_SetWanderFollowEntityId>(moduleBase, 0x1503e10);
		ShiftBegin = getFunctionAddr<_ShiftBegin>(moduleBase, 0x1503ed0);
		ShiftEnd = getFunctionAddr<_ShiftEnd>(moduleBase, 0x1503f90);
		ShiftTelegraph = getFunctionAddr<_ShiftTelegraph>(moduleBase, 0x1504010);
		ShouldBecomeRigidOnGloo = getFunctionAddr<_ShouldBecomeRigidOnGloo>(moduleBase, 0x15040b0);
		ShouldMimicRebound = getFunctionAddr<_ShouldMimicRebound>(moduleBase, 0x15040c0);
		SpawnLootOnDeath = getFunctionAddr<_SpawnLootOnDeath>(moduleBase, 0x15041e0);
		StartAmbientSound = getFunctionAddr<_StartAmbientSound>(moduleBase, 0x15046f0);
		StartAttackGlow = getFunctionAddr<_StartAttackGlow>(moduleBase, 0x15047b0);
		StartAttackJitter = getFunctionAddr<_StartAttackJitter>(moduleBase, 0x15047c0);
		StartCharacterEffect = getFunctionAddr<_StartCharacterEffect>(moduleBase, 0x15047d0);
		StartCowering = getFunctionAddr<_StartCowering>(moduleBase, 0x15047e0);
		StartEnergizedDeath = getFunctionAddr<_StartEnergizedDeath>(moduleBase, 0x1504820);
		StartGlooEffects = getFunctionAddr<_StartGlooEffects>(moduleBase, 0x1504990);
		StartInstantDeath = getFunctionAddr<_StartInstantDeath>(moduleBase, 0x1504a60);
		StartMaterialAnimation = getFunctionAddr<_StartMaterialAnimation>(moduleBase, 0x1504aa0);
		StartOnFireEffect = getFunctionAddr<_StartOnFireEffect>(moduleBase, 0x1504ab0);
		StartOperatorArmIdle = getFunctionAddr<_StartOperatorArmIdle>(moduleBase, 0x1504ac0);
		StartPsiSuppressed = getFunctionAddr<_StartPsiSuppressed>(moduleBase, 0x1504b30);
		StartRaiseFromCorpse = getFunctionAddr<_StartRaiseFromCorpse>(moduleBase, 0x1504bd0);
		StartRaisePhantomCast = getFunctionAddr<_StartRaisePhantomCast>(moduleBase, 0x1504c30);
		StartStun = getFunctionAddr<_StartStun>(moduleBase, 0x1504c40);
		StartXRayVisionOn = getFunctionAddr<_StartXRayVisionOn>(moduleBase, 0x1504d30);
		StopCharacterEffect = getFunctionAddr<_StopCharacterEffect>(moduleBase, 0x1504df0);
		StopCowering = getFunctionAddr<_StopCowering>(moduleBase, 0x1504e00);
		StopGlooEffects = getFunctionAddr<_StopGlooEffects>(moduleBase, 0x1504e10);
		StopMimicGlitch = getFunctionAddr<_StopMimicGlitch>(moduleBase, 0x1504f30);
		StopOnFireEffect = getFunctionAddr<_StopOnFireEffect>(moduleBase, 0x1504f40);
		StopOperatorArmIdle = getFunctionAddr<_StopOperatorArmIdle>(moduleBase, 0x1504f50);
		StopStun = getFunctionAddr<_StopStun>(moduleBase, 0x1504fa0);
		StopXRayVisionOn = getFunctionAddr<_StopXRayVisionOn>(moduleBase, 0x1504ff0);
		SupportsLookAt = getFunctionAddr<_SupportsLookAt>(moduleBase, 0x15050a0);
		TestInteraction = getFunctionAddr<_TestInteraction>(moduleBase, 0x15050b0);
		TryDodge = getFunctionAddr<_TryDodge>(moduleBase, 0x1505210);
		TryEvaluateAbilityContext = getFunctionAddr<_TryEvaluateAbilityContext>(moduleBase, 0x1505490);
		TryEvaluateAndPerformAbilityContext = getFunctionAddr<_TryEvaluateAndPerformAbilityContext>(moduleBase, 0x1505510);
		TryPerformAbilityContext1 = getFunctionAddr<_TryPerformAbilityContext1>(moduleBase, 0x15055b0);
		TryPerformAbilityContext2 = getFunctionAddr<_TryPerformAbilityContext2>(moduleBase, 0x15056b0);
		// TryPerformAbilityContext = getFunctionAddr<_TryPerformAbilityContext>(moduleBase, 0x1505)	;
		TryPerformAbilityContext3 = getFunctionAddr<_TryPerformAbilityContext3>(moduleBase, 0x15056d0);
		TryPerformAnyAbility = getFunctionAddr<_TryPerformAnyAbility>(moduleBase, 0x15056e0);
		UnCorrupt = getFunctionAddr<_UnCorrupt>(moduleBase, 0x1505780);
		UnlockMimicry = getFunctionAddr<_UnlockMimicry>(moduleBase, 0x15057d0);
		// Update = getFunctionAddr<_Update>(moduleBase, 0x)	;
		Update = getFunctionAddr<_Update>(moduleBase, 0x1505d70);
		UpdateFactions = getFunctionAddr<_UpdateFactions>(moduleBase, 0x15068b0);
		UpdateMaterialAnimation = getFunctionAddr<_UpdateMaterialAnimation>(moduleBase, 0x1506f50);
		WaitForAnimatedStunned = getFunctionAddr<_WaitForAnimatedStunned>(moduleBase, 0x1506f60);
}

PreyFunctions::CArkNpcSpawnCystoidPrivate::CArkNpcSpawnCystoidPrivate(uintptr_t moduleBase) {
	getEntityArchetype = getFunctionAddr<_GetEntityArchetype>(moduleBase, 0x151c800);
}

PreyFunctions::ArkSafeScriptTablePrivate::ArkSafeScriptTablePrivate(uintptr_t moduleBase) {
	getArkSafeScriptFromScriptTable = getFunctionAddr<_GetArkSafeScriptTableFromScriptTable>(moduleBase, 0x1480240);
	getArkSafeScriptFromEntity = getFunctionAddr<_GetArkSafeScriptTableFromEntity>(moduleBase, 0x14801c0);
	getCryStringTValue = getFunctionAddr<_GetCryStringTValue>(moduleBase, 0x1160740);
}

PreyFunctions::ArkLocationManagerPrivate::ArkLocationManagerPrivate(uintptr_t moduleBase) {
	GetAlternateNameInfo =	getFunctionAddr<_GetAlternateNameInfo>(moduleBase,			0x1197ed0);
	GetCurrentLocation = getFunctionAddr<_GetCurrentLocation>(moduleBase, 0x1197fd0);
	GetCurrentLocationLabel = getFunctionAddr<_GetCurrentLocationLabel>(moduleBase, 0x1198080);
	GetCurrentLocationScreenshotPath = getFunctionAddr<_GetCurrentLocationScreenshotPath>(moduleBase, 0x1198120);
	GetLocationLabel = getFunctionAddr<_GetLocationLabel>(moduleBase, 0x11981c0);
	HasLoaded = getFunctionAddr<_HasLoaded>(moduleBase, 0x1198230);
	RemoveAlternateName = getFunctionAddr<_RemoveAlternateName>(moduleBase, 0x1198290);
	Reset = getFunctionAddr<_Reset>(moduleBase, 0x11983d0);
	Serialize = getFunctionAddr<_Serialize>(moduleBase, 0x11983e0);
	SetAlternateName = getFunctionAddr<_SetAlternateName>(moduleBase, 0x1198550);
	SetLoaded = getFunctionAddr<_SetLoaded>(moduleBase, 0x1198750);
}

PreyFunctions::ArkAbilityComponentPrivate::ArkAbilityComponentPrivate(uintptr_t moduleBase) {
		ArkAbilityComponentConst1= getFunctionAddr<_ArkAbilityComponentConst>(moduleBase,0x153a390);
		ArkAbilityComponentConst2 = getFunctionAddr < _ArkAbilityComponentConst2>(moduleBase, 0x153a410);
		GetAbilityState = getFunctionAddr < _GetAbilityState >(moduleBase, 0x153a480);
		GetAcquiredAbilities = getFunctionAddr < _GetAcquiredAbilities >(moduleBase, 0x153a5d0);
		GetCurrencyArchetypeId = getFunctionAddr < _GetCurrencyArchetypeId >(moduleBase, 0x153a640);
		GetNumNeuromodsUsed = getFunctionAddr < _GetNumNeuromodsUsed >(moduleBase, 0x153a6c0);
		GetPlayerResearchFraction = getFunctionAddr < _GetPlayerResearchFraction >(moduleBase, 0x153a740);
		GetResearchTopicScanCount = getFunctionAddr < _GetResearchTopicScanCount >(moduleBase, 0x153a780);
		GetTyphonStatus = getFunctionAddr < _GetTyphonStatus >(moduleBase, 0x153a7b0);
		GrantAbility = getFunctionAddr < _GrantAbility >(moduleBase, 0x153a800);
		HasAbility = getFunctionAddr < _HasAbility >(moduleBase, 0x153ad80);
		IncrementResearchTopicScanCount = getFunctionAddr < _IncrementResearchTopicScanCount >(moduleBase, 0x153adc0);
		InitAbilityResearchTopicData = getFunctionAddr < _InitAbilityResearchTopicData >(moduleBase, 0x153b110);
		IsAbilityNameRevealedByAbilityData = getFunctionAddr < _IsAbilityNameRevealedByAbilityData >(moduleBase, 0x153b360);
		IsAbilityNameRevealedByID = getFunctionAddr < _IsAbilityNameRevealedByID >(moduleBase, 0x153b430);
		IsAbilityNew = getFunctionAddr < _IsAbilityNew >(moduleBase, 0x153b480);
		IsAbilityResearched = getFunctionAddr < _IsAbilityResearched >(moduleBase, 0x153b4d0);
		IsAbilityVisible = getFunctionAddr < _IsAbilityVisible >(moduleBase, 0x153b560);
		LoadConfig = getFunctionAddr < _LoadConfig >(moduleBase, 0x153b5d0);
		MarkAbilitySeen = getFunctionAddr < _MarkAbilitySeen >(moduleBase, 0x153b630);
		OnGrantAbilityCheat = getFunctionAddr < _OnGrantAbilityCheat >(moduleBase, 0x153b670);
		OnPDAOpenComplete = getFunctionAddr < _OnPDAOpenComplete >(moduleBase, 0x153b960);
		PostSerialize = getFunctionAddr < _PostSerialize >(moduleBase, 0x153b9b0);
		PurchaseAbility = getFunctionAddr < _PurchaseAbility >(moduleBase, 0x153ba80);
		Reset = getFunctionAddr < _Reset >(moduleBase, 0x153bd00);
		Serialize = getFunctionAddr < _Serialize >(moduleBase, 0x153c2d0);
		ShowLastResearchedAbility = getFunctionAddr < _ShowLastResearchedAbility >(moduleBase, 0x153c340);
		UpdatePlayerMetrics = getFunctionAddr < _UpdatePlayerMetrics >(moduleBase, 0x153c3b0);
}

PreyFunctions::CKeyboardPrivate::CKeyboardPrivate(uintptr_t moduleBase) {
	Update = getFunctionAddr<_Update>(moduleBase, 0x09D2DA0);
}

PreyFunctions::CBaseInputPrivate::CBaseInputPrivate(uintptr_t moduleBase) {
	PostInputEvent = getFunctionAddr<_PostInputEvent>(moduleBase, 0x09D58D0);
}

PreyFunctions::PreyFunctions(uintptr_t moduleBase) {
	ArkPlayerF = new  ArkPlayerPrivate(moduleBase);
	CArkWeaponF = new CArkWeaponPrivate(moduleBase);
	CEntitySystemF = new CEntitySystemPrivate(moduleBase);
	ArkNpcSpawnManagerF = new ArkNpcSpawnManagerPrivate(moduleBase);
	CEntity = new CEntityPrivate(moduleBase);
	ArkNightmareSpawnManagerF = new ArkNightmareSpawnManagerPrivate(moduleBase);
	CArkNpcSpawnerF = new CArkNpcSpawnerPrivate(moduleBase);
	CXConsoleF = new CXConsolePrivate(moduleBase);
	ArkFactionManagerF = new ArkFactionManagerPrivate(moduleBase);
	ArkNpcF = new ArkNpcPrivate(moduleBase);
	CArkNpcSpawnCystoidF = new CArkNpcSpawnCystoidPrivate(moduleBase);
	ArkSafeScriptTablef = new ArkSafeScriptTablePrivate(moduleBase);
	ArkLocationManagerF = new ArkLocationManagerPrivate(moduleBase);
	ArkAbilityComponentF = new ArkAbilityComponentPrivate(moduleBase);
	CryGretCurrentThreadId = getFunctionAddr<_CryGetCurrentThreadID>(moduleBase, 0x0099910);
	beginDraw = getFunctionAddr<_BeginDraw>(moduleBase, 0x0de9710);
	CKeyboardF = new CKeyboardPrivate(moduleBase);
	CBaseInputF = new CBaseInputPrivate(moduleBase);
}
