#pragma once
#include "pch.h"

#include "EntityUtils.h"
#include <Prey/ArkEntityArchetypeLibrary.h>
#include "../ChairLoader/GUIUtils.h"
#include "../ChairLoader/IChairloaderModule.h"
#include "ChairLoader/IChairloader.h"
#include "ChairLoader/ChairloaderEnv.h"
#include "Prey/CryEntitySystem/EntitySystem.h"


class EntityManager : public IChairloaderModule
{
public:
    //TODO: completely redo entity modify
    EntityManager(ChairloaderGlobalEnvironment* env);
    ~EntityManager();

    void drawEntitySpawner(bool* bShow);
    void drawEntityList(bool* bShow);
    void drawMenuBar();

   
private:
    bool showEntitySpawner = false, showEntityList = false;

    // environment *
    ChairloaderGlobalEnvironment* gCLEnv = nullptr;

    const std::string moduleName = "EntityManager";

    bool refreshDisplayList = true;
    std::string filterText, oldFilterText;

    CEntity* selected = nullptr;

    IEntityArchetype* archetypeToSpawn = nullptr;

    // archetype filter
    std::string archetypeFilterText, oldArchetypeFilterText;

    std::vector<IEntityArchetype*> archetypeFilteredList;
    std::vector<CEntity*> entityDisplayList;

    // std::unordered_map< uint64_t, IEntityArchetype*>* archetypeList = &((IEntityArchetype*)gEnv->pEntitySystem)->m_pEntityArchetypeManager->m_idToArchetypeMap;
    // std::queue<entityModifyRequest> modifyQueue;
    // bool entitySpawningMethod = false;


    inline CEntitySystem* GetEntitySystem() { return (CEntitySystem*)gEnv->pEntitySystem; }

   bool usePlayerPos = true, offsetFromPlayer = true;
   std::string inputId, inputName;
   std::string statusMessage;
   float spawnX, spawnY, spawnZ;
   int spawnCount{ 1 };
   ImVec4 color{ 1,1,1,1 };
   time_t statusTimer;

    void spawnEntity();
    void quickSpawnEntity(uint64_t archetypeId);
public:
    void InitSystem() override;
    void InitGame(IGameFramework* pFramework) override;
    void Draw(ImGuiContext*) override;
    void PreUpdate() override;
    void PostUpdate() override;
    void ShutdownGame() override;
    void ShutdownSystem() override;
    std::string GetModuleName() override { return moduleName; };
private:
    // Currently Mostly Unused
    std::vector <std::string> actions{
        "ActivateWander",
        "AddCollisionObserver",
        "AddFacingDesire",
        "AddInboundModifier",
        "AddLookDesire",
        "AddMovementDesire",
        "AddNoiseStateDesire",
        "AddSpeedDesire",
        "AttachPistol",
        "BecomeActive",
        "BecomeBroken",
        "BecomeInactive",
        "BeginAnimatedDistraction",
        "BeginAnimatedStunned",
        "BeginDamagedAudio",
        "BeginDisabledAudio",
        "BeginLurking",
        "BeginSearchExamine",
        "BeginUndamagedAudio",
        "BreakGlooEntity",
        "BreakNearbyGloo",
        "CalcHasSpaceToMimicSideStep",
        "CallForHelp",
        "ClearAllAttention",
        "ClearAttentionOnPlayer",
        "ClearConversationFacingTarget",
        "ClearConversationLookTarget",
        "ClearFear",
        "ClearPrecache",
        "Corrupt",
        "CorruptNpc",
        "CorruptedMovementGlitch",
        "CreateAiTree",
        "DeactivateWander",
        "DestroyAiTree",
        "Dissipate",
        "DoCorruptExplode",
        "EndAnimatedDistraction",
        "EndAnimatedStunned",
        "EndDormant",
        "EndLurking",
        "EndMimicry",
        "EndPsiSuppressed",
        "EndSearchExamine",
        "FearAOEOnPlayerKill",
        "FindDeathInstigator",
        "FindNearestReachablePoint",
        "FindNearestValidPoint",
        "FinishRaiseFromCorpse",
        "FlowGraphDisableAi",
        "FlowGraphIgnoreDistractions",
        "FlowGraphSetDumbed",
        "FlowgraphDisableCombatIntensity",
        "FlowgraphEnableCombatIntensity",
        "FollowLeaderEntity",
        "ForceDesperationMimicryInPlace",
        "ForceDownToSearchAttentionLevel",
        "ForceMovementReplan",
        "ForceReevaluateMovementAnimAction",
        "ForceStopCharacterEffect",
        "ForceStopDistraction",
        "FullSerialize",
        "LightOnFire",
        "LoadScriptVariables",
        "LockKnownAttentionOnTarget",
        "LockMimicry",
        "MakeConscious",
        "MakePoltergeistAppear",
        "MakePoltergeistDisapear",
        "MimicAndReplaceEntity",
        "MimicCollisionRebound",
        "MimiIEntity",
        "NoticeTargetPosition",
        "NotifyCorruptedOwnerOfUncorruption",
        "OverrideCharacterId",
        "OverrideHitReactAnimation",
        "PerformAnimatedAbility",
        "PerformCombatReaction",
        "PerformFatality",
        "PerformHitReactShift",
        "PerformMimicGlitch",
        "PerformMimicReorientation",
        "PerformMindControlled",
        "PerformPassiveGlooBreak",
        "PerformPatrolIdle",
        "PerformPostSerializeGloodPose",
        "PerformReorientation",
        "PerformScan",
        "PerformStopMindControlled",
        "PerformUnanimatedAbility",
        "PerformWanderIdle",
        "PlayMimicHitEffect",
        "PlayMimicryMorphInEffect",
        "PlayMimicryMorphOutEffect",
        "Ragdoll",
        "RefreshCombatIntensity",
        "RefreshNotice",
        "RegisterOnFleeNodeCallsbacks",
        "Release",
        "ReloadAmmo",
        "RemoveAmmoAmount",
        "RemoveCollisionObserver",
        "RemoveFacingDesire",
        "RemoveHead",
        "RemoveInboundModifier",
        "RemoveLookDesire",
        "RemoveMovementDesire",
        "RemoveNoiseStateDesire",
        "RemoveSpeedDesire",
        "RequestDistraction",
        "RequestLurk",
        "RequestStopLurk",
        "ResetAIAlwaysUpdate",
        "ResetPlayerBumpTimer",
        "Resist",
        "ScaleLookDesireSpeed",
        "Serialize",
        "ShiftBegin",
        "ShiftEnd",
        "ShiftTelegraph",
        "ShouldBecomeRigidOnGloo",
        "ShouldMimicRebound",
        "SpawnLootOnDeath",
        "StartAmbientSound",
        "StartAttackGlow",
        "StartAttackJitter",
        "StartCharacterEffect",
        "StartCowering",
        "StartEnergizedDeath",
        "StartGlooEffects",
        "StartInstantDeath",
        "StartMaterialAnimation",
        "StartOnFireEffect",
        "StartOperatorArmIdle",
        "StartPsiSuppressed",
        "StartRaiseFromCorpse",
        "StartRaisePhantomCast",
        "StartStun",
        "StartXRayVisionOn",
        "StopCharacterEffect",
        "StopCowering",
        "StopGlooEffects",
        "StopMimicGlitch",
        "StopOnFireEffect",
        "StopOperatorArmIdle",
        "StopStun",
        "StopXRayVisionOn",
        "SupportsLookAt",
        "TestInteraction",
        "TryDodge",
        "TryEvaluateAbilityContext",
        "TryEvaluateAndPerformAbilityContext",
        "TryPerformAbilityContext1",
        "TryPerformAbilityContext2",
        "TryPerformAbilityContext3",
        "TryPerformAnyAbility",
        "UnCorrupt",
        "UnlockMimicry",
        "Update",
        "UpdateFactions",
        "UpdateMaterialAnimation",
        "WaitForAnimatedStunned"
    };
    std::vector<std::string> is{
        "IsAlert",
        "IsAsleep",
        "IsAttentionProxyFollowing",
        "IsBreakable",
        "IsBroken",
        "IsCharacterEffectEnabled",
        "IsCorrupted",
        "IsCowering",
        "IsDead",
        "IsDestroyed",
        "IsDissipating",
        "IsDormant",
        "IsEntityMimicable",
        "IsEthericDoppelganger",
        "IsFalling",
        "IsFeared",
        "IsFollowingPlayer",
        "IsGlooSlow",
        "IsGoingToFreezeInGloo",
        "IsHacked",
        "IsIgnoringRangedAbilities",
        "IsInRaiseFromCorpseAnim",
        "IsJumping",
        "IsLureFlareValid",
        "IsLurking",
        "IsMimic",
        "IsMimicking",
        "IsMindControlled",
        "IsMovementAnimated",
        "IsMovementDesireExecuting",
        "IsMovementInterruptible",
        "IsNpcHealthUIDisabled",
        "IsPerformingAbility",
        "IsPlayerControlled",
        "IsPsiLifted",
        "IsPsiSuppressed",
        "IsRagdolled",
        "IsResisting",
        "IsScrunched",
        "IsShifting",
        "IsStandingUp",
        "IsStunned",
        "IsVisible",
        "IsVulnerable",
        "IsWithinCameraViewCone",
        "IsXRayTrackingTarget",
        "CanAttachGloo",
        "CanBeDistracted",
        "CanBeHypnotized",
        "CanDeathReact",
        "CanHitReact",
        "CanInstigate",
        "CanJump",
        "CanMimicSideStepLeft",
        "CanMimicSideStepRight",
        "CanPerformAbilityContext",
        "CanPerformAnimatedAbility",
        "CanPerformFatality",
        "CanPerformHitReactShift",
        "CanPerformUnanimatedAbility",
        "CanRaiseAnotherPhantom",
        "CanSee",
        "CanSpeak"
    };
    std::vector<std::string> get{
        "ActiveEthericDoppelgangerId",
        "AmmoCount",
        "AttachedPistolId",
        "Attachment",
        "AttentionOnPlayer",
        "CachedThrowAtLocation",
        "CachedThrowAtType",
        "CharacterEffectsManager",
        "ComplexHearingGain",
        "ComplexVisionGain",
        "CurrentAbilityContextId",
        "CurrentCombatRole",
        "DefaultFaction",
        "DesiredSpeed",
        "DodgeTimeStamp",
        "EnergizedAmount",
        "EntityPoolSignature",
        "EthericDoppelgangerOwnerId",
        "GlooEffect",
        "HeadDirection",
        "HeadPosition",
        "HitReactKnockDownTimeStamp",
        "HitReactStaggerTimeStamp",
        "IsHitReactingPrereqFlag",
        "IsRegainingLosPrereqFlag",
        "IsShuttingDownHack",
        "IsTakingDamagePrereqFlag",
        "LastPerformedContextId",
        "LeaderId",
        "MaterialAnimationDuration",
        "MaterialAnimationManager",
        "MimicingEntityId",
        "MimicryReason",
        "MovementRequestId",
        "NpcManagerCombatPoints",
        "NumControlledTurrets",
        "NumCorruptedNpcs",
        "PhysicalEntitiesInBox",
        "Properties",
        "RMIBase",
        "SearchReasonFromChangeReason",
        "TopAttentionTargetEntityId"
    };
    std::vector<std::string> set{
        "AnimTag",
        "BreakGlooEntity",
        "CachedThrowAtLocation",
        "ComplexHearingGain",
        "ComplexVisionGain",
        "ConversationFacingTarget",
        "ConversationLookTarget",
        "CurrentCombatRole",
        "EthericDoppelgangerId",
        "EthericDoppengangerOwnerId",
        "FearFromHostileDamage",
        "HasPerformedNotice",
        "HypnotizeStartTimeStamp",
        "IsEthericDoppelganger",
        "IsHitReactingPrereqFlag",
        "IsIgnoringRangedAbilities",
        "IsPlayerControlled",
        "IsRegainingLoSPrereqFlag",
        "PistolHidden",
        "ShouldSkipNextCombatReaction",
        "SpawnedState",
        "Stance",
        "TimeUntilDeath",
        "TransitionStateTag",
        "WanderAreaContainerEntity",
        "WanderFollowEntityId",
    };
    std::vector<std::string> statusPop{
         "AIAlwaysUpdate",
         "AttentionDrainLockOnTarget",
         "DisableAiTree",
         "DisableAllAmbientSounds",
         "DisableAttentionAndSenses",
         "DisableAttentionObjectAndPerceivables",
         "DisableAudible",
         "DisableBreakable",
         "DisableHearing",
         "DisableHitReactions",
         "DisableLifetimeEffect",
         "DisableNpcHealthUI",
         "DisableOperatorLevitatorsEffect",
         "DisableSenses",
         "DisableVisible",
         "DisableVision",
         "Dumbed",
         "EnableAbilities",
         "EnableAiTree",
         "EnableAttentionObject",
         "EnableAttentiveSubject",
         "EnableAudible",
         "EnableGlassBreaking",
         "EnableHearing",
         "EnableLifetimeEffect",
         "EnableOperatorLevitatorsEffect",
         "EnableRoomPerceiver",
         "EnableVisible",
         "EnableVision",
         "EnabledAmbientSound",
         "Fear",
         "ForceRigidOnGloo",
         "IndefiniteRagdoll"
    };
    std::vector<std::string> statusPush{
             "AIAlwaysUpdate",
             "AIAlwaysUpdateForPatrol",
             "AttentionDrainLockOnTarget",
             "DisableAiTree",
             "DisableAllAmbientSounds",
             "DisableAttentionAndSenses",
             "DisableAttentionObjectAndPerceivables",
             "DisableAudible",
             "DisableBreakable",
             "DisableDeathReactions",
             "DisableHearing",
             "DisableHitReactions",
             "DisableLifetimeEffect",
             "DisableNpcHealthUI",
             "DisableOperatorLevitatorsEffect",
             "DisableSenses",
             "DisableVisible",
             "DisableVision",
             "Dumbed",
             "EnableAbilities",
             "EnableAiTree",
             "EnableAttentionObject",
             "EnableAttentiveSubject",
             "EnableAudible",
             "EnableGlassBreaking",
             "EnableHearing",
             "EnableLifetimeEffect",
             "EnableOperatorLevitatorsEffect",
             "EnableRoomPerceiver",
             "EnableVisible",
             "EnableVision",
             "EnabledAmbientSound",
             "Fear",
             "ForceRigidOnGloo",
             "IndefiniteRagdoll"
    };
};

