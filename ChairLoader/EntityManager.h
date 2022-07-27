#pragma once
#include "pch.h"

#include <Prey/ArkBasicTypes.h>
#include "EntityUtils.h"
#include "preyDllObjects.h"
#include <Prey/ArkEntityArchetypeLibrary.h>
#include "GUIUtils.h"
#include "ChairLoader.h"
#include "OverlayLog.h"

class ChairloaderGUIEntityManager
{
public:

    enum class entityModifyType {
        none = -1,
        pos = 0,
        rot = 1,
        scale = 2,
        playerPos = 3,
    };
    struct entityModifyRequest {
        CEntity* entity;
        entityModifyType type;
        Vec3_tpl<float> pos{ 0,0,0 };
        Vec4_tpl<float> rot{ 0,0,0,0 };
        Vec3_tpl<float> scale = { 0,0,0 };
    };
    struct spawnRequest {
        CEntityArchetype* archetype;
        std::string name;
        Vec3_tpl<float> pos = { 0,0,0 };
        Vec4_tpl<float> rot = { 0,0,0,1 };
        int spawnCount = 1;
        bool usePlayerPos = true,
            offsetFromPlayer = true;
    };

    struct archetypeFilterRequest {
        std::string text;
    };
    ChairloaderGUIEntityManager();
    ~ChairloaderGUIEntityManager();

    // void draw(bool* bShow);
    void drawEntitySpawner(bool* bShow);
    void drawEntityList(bool* bShow);
    void drawEntitySpawnerTest(bool* bShow);
    void drawMenuBar(bool* entityListShow, bool* entitySpawnerShow);

    void update(ChairloaderGUILog* log);
private:
    const std::string modName = "EntityManager";
    std::vector<CEntity*> entityDisplayList;
    bool refreshDisplayList = true;
    std::string filterText, oldFilterText;
    CEntity* selected = nullptr;
    std::queue<spawnRequest> archetypeSpawnRequestQueue;
    CEntityArchetype* archetypeToSpawn;
    // archetype filter
    std::string archetypeFilterText, oldArchetypeFilterText;
    std::queue<archetypeFilterRequest> archetypeFilterRequestQueue;
    std::vector<CEntityArchetype*> archetypeFilteredList;
    std::unordered_map< uint64_t, CEntityArchetype*>* archetypeList = &((CEntitySystem *)gEnv->pEntitySystem)->m_pEntityArchetypeManager->m_idToArchetypeMap;
    std::queue<entityModifyRequest> modifyQueue;
    // bool entitySpawningMethod = false;


    inline CEntitySystem *GetEntitySystem() { return static_cast<CEntitySystem*>(gEnv->pEntitySystem); }

	void entityModifyHandler(ChairloaderGUILog* log);
    void filterEntityList(ChairloaderGUILog* log);
    void archetypeFilterRequestHandler(ChairloaderGUILog* log);
   


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
        "MimicEntity",
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
