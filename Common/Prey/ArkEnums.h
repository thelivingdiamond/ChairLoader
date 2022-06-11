#pragma once
;
enum class EArkMapObjectiveState {
    none=0,
    side=1,
    roster=2,
    main=3
};

enum class EArkReason {
    userSignOut=0,
    controllerUserInvalid=1,
    controllerDisconnect=2
};

enum class EArkAiTreeCreatePhantomStage {
    TryingToPerformAbility=0,
    CreatingPhantom=1,
    Done=2
};

enum class EArkTestResult {
    untested=0,
    unavailable=1,
    failed=2,
    passed=3
};

enum class EArkLoadState {
    CryModuleCRTFree=0,
    loading=1,
    loadNotice=2,
    loaded=3
};

enum class EArkCurrentPage {
    keypad=0,
    cargoBays=1,
    help=2
};

enum class EArkPryState : int32_t {
    none=0,
    lerp=1,
    in=2,
    loop=3,
    out=4
};

enum class EArkExaminationState : int32_t {
    inactive=0,
    active=1,
    exiting=2
};

enum class EArkExaminationType : int32_t {
    none=0,
    worldUI=1,
    telescope=2,
    other=3
};

enum class EArkInputs : int32_t {
    plan=0
};

enum class EArkOutputs : int32_t {
    output=0
};


/* WARNING! conflicting data type names: /PreyDll.pdb/ArkFlowNodeAudioLogEvent/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/ArkFlowNodeAudioLogEvent/EArkOutputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkOutputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/ArkFlowNodeChipsetInstalled/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/ArkFlowNodeChipsetInstalled/EArkOutputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkOutputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/ArkFlowNodeCreatePhantom/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/ArkFlowNodeCreatePhantom/EArkOutputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkOutputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/ArkFlowNodePlayerAlien/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/ArkFlowNodePlayerAlien/EArkOutputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkOutputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/ArkFlowNodeRemoveLocationAlternateName/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/ArkFlowNodeScanAlienEvent/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/ArkFlowNodeScanAlienEvent/EArkOutputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkOutputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/ArkFlowNodeSetCharacterAlternateName/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/ArkFlowNodeSetLocationAlternateName/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/ArkFlowNodeSetPortalOcclusion/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/ArkFlowSendSignalPackage/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/ArkFlowSendSignalPackageInArea/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/ArkFlowSetEntityCollision/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/ArkFlowSignalAddModifier/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/ArkFlowSignalPackageReceiver/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/ArkFlowSignalReceiver/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/ArkFlowSignalReceiver/EArkOutputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkOutputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/ArkFlowSignalRemoveModifier/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/ArkFlowSignalRemoveModifier/EArkOutputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkOutputs */

enum class EArkConsumeResult {
    empty=0,
    fail=1,
    success=2
};

enum class EArkSafeBoundsResult {
    Safe=0,
    Warning=1,
    Unsafe=2
};



enum class EArkInstigationType {
    _standard=0,
    _thrown=1,
    _collision=2
};

enum class EArkGridSizes {
    tinyExternal=0,
    smallExternal=1,
    mediumExternal=2,
    largeExternal=3,
    chipSet=4,
    player=5,
    overflow=6
};

enum class EArkKeyCardUIState {
    LockedDefault=0,
    LockedNoCard=1,
    LockedFail=2,
    LockedMatch=3,
    UnlockedDefault=4,
    UnlockedNoCard=5,
    UnlockedFail=6,
    UnlockedMatch=7,
    Processing=8,
    Inaccessible=9
};

enum class EArkKeypadState {
    Invalid=0,
    Processing=1,
    Failure=2,
    Locked=3,
    Unlocked=4
};


/* WARNING! conflicting data type names: /PreyDll.pdb/ArkLauncherMenu/EArkLoadState - /PreyDll.pdb/ArkDialogLoader/EArkLoadState */

enum class EArkShellMode {
    disabled=0,
    hidden=1,
    attract=2,
    menu=3,
    loadTransition=4,
    store=5
};

enum class EArkMarkClass {
    Invalid=0,
    Hostile=1,
    NonHostile=2,
    Mimic=3,
    EliteMimic=4,
    Poltergeist=5,
    Count=6
};

enum class EArkMultipleChoiceKioskState {
    Invalid=0,
    StartPage=1,
    QuestionPage=2,
    RorschachPage=3,
    LogoPage=4
};

enum class EArkNoteUIState {
    closed=0,
    open=1,
    closing=2,
    unload=3
};

enum class EArkOptionWidgetType {
    none=0,
    list=1,
    slider=2,
    datalist=3,
    command=4,
    toggle=5,
    dropdown=6,
    controlMap=7
};

enum class EArkPAInput {
    chime=3,
    channel=4,
    chimeType=5
};

enum class EArkPauseMode {
    disabled=0,
    hidden=1,
    menu=2,
    continuePrompt=3,
    scoreCard=4
};

enum class EArkPlayerCharacterSlot {
    model=0,
    shadow=5,
    bleed=6,
    zerog=7
};

enum class EArkPlayerMovementState {
    idle=0,
    movement=1,
    inAir=2,
    last=3
};

enum class EArkBoneList {
    BONE_BIP01=0,
    BONE_SPINE=1,
    BONE_SPINE2=2,
    BONE_SPINE3=3,
    BONE_HEAD=4,
    BONE_EYE_R=5,
    BONE_EYE_L=6,
    BONE_WEAPON=7,
    BONE_WEAPON2=8,
    BONE_FOOT_R=9,
    BONE_FOOT_L=10,
    BONE_ARM_R=11,
    BONE_ARM_L=12,
    BONE_CALF_R=13,
    BONE_CALF_L=14,
    BONE_CAMERA=15,
    BONE_HUD=16,
    BONE_ID_NUM=17
};


/* WARNING! conflicting data type names: /PreyDll.pdb/ArkPlayerMovementAction/EArkPlayerMovementState - /PreyDll.pdb/ArkPlayer/EArkPlayerMovementState */

enum class EArkFlyMode {
    off=0,
    on=1,
    onNoCollision=2
};

enum class EArkRadiationLevel {
    None=0,
    Low=1,
    Medium=2,
    High=3,
    Count=4
};

enum class EArkAwarenessUpdateType {
    none=0,
    visible=1,
    fadeOut=2,
    fullAwareness=3
};

enum class EArkType {
    intT=0,
    uintT=1,
    boolT=2,
    floatT=3,
    stringT=4,
    crynameT=5,
    vec2T=6,
    vec3T=7,
    objectT=8,
    object_ptrT=9,
    unknownT=10
};


/* WARNING! conflicting data type names: /PreyDll.pdb/ArkSaveLoadMenu/EArkLoadState - /PreyDll.pdb/ArkDialogLoader/EArkLoadState */

enum class EArkSignalReceiverType {
    invalid=0,
    lua=1,
    cplusplus=2,
    flowSignal=3,
    flowPackage=4,
    recycle=5
};

enum class EArkSpeakerType {
    prop=0,
    npc=1,
    last=2
};

enum class EArkTimerFlag {
    Real=1,
    Game=2,
    UI=4,
    Player=8
};

enum class EArkTurretState {
    invalid=0,
    good=1,
    broken=2,
    undeployed=3
};

enum class EArkAttackAnim {
    none=0,
    a=1,
    b=2,
    c=3,
    d=4,
    e=5,
    f=6,
    g=7,
    hitA=8,
    hitB=9,
    hitC=10,
    hitD=11,
    hitE=12,
    hitF=13,
    hitG=14,
    hitEnemyA=15,
    hitEnemyB=16,
    hitEnemyC=17,
    hitEnemyD=18,
    hitEnemyE=19,
    hitEnemyF=20,
    hitEnemyG=21,
    fatality=22,
    genericFatality=23
};


/* WARNING! conflicting data type names: /PreyDll.pdb/ArkWeaponWrench/EArkAttackAnim - /PreyDll.pdb/ArkWeaponDoubleWrench/EArkAttackAnim */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNode_IsArchetypeInWorld/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNode_IsArchetypeInWorld/EArkOutputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkOutputs */

enum class EArkInputPorts {
    profile_name=0,
    encounter_name=1
};

enum class EArkOutputPorts {
    out_encounter_name=0
};


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeAddEncounterProfile/EArkInputPorts - /PreyDll.pdb/CArkFlowNodeAddEncounter/EArkInputPorts */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeAddEncounterProfile/EArkOutputPorts - /PreyDll.pdb/CArkFlowNodeAddEncounter/EArkOutputPorts */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeAddEncounterSpawnInfo/EArkInputPorts - /PreyDll.pdb/CArkFlowNodeAddEncounter/EArkInputPorts */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeAddEncounterSpawnInfo/EArkOutputPorts - /PreyDll.pdb/CArkFlowNodeAddEncounter/EArkOutputPorts */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeCharacterPasswordAcquired/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeCheckDownloadHidden/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeCheckDownloadHidden/EArkOutputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkOutputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeCheckUtilityButtonEnabled/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeCheckUtilityButtonEnabled/EArkOutputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkOutputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeCheckUtilityButtonHidden/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeCheckUtilityButtonHidden/EArkOutputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkOutputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeCheckUtilityEnabled/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeCheckUtilityEnabled/EArkOutputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkOutputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeCheckUtilityHidden/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeCheckUtilityHidden/EArkOutputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkOutputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeCompareUtility/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeCompareUtility/EArkOutputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkOutputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeCompareUtilityButton/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeCompareUtilityButton/EArkOutputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkOutputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeConfirmDialog/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeConfirmDialog/EArkOutputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkOutputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeConversationEvent/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeConversationEvent/EArkOutputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkOutputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeConversationStatus/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeConversationStatus/EArkOutputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkOutputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeConversationTrigger/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeConversationTrigger/EArkOutputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkOutputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeDiscoverLocation/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeDownloadAcquired/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeEliminateCurrentPopulation/EArkInputPorts - /PreyDll.pdb/CArkFlowNodeAddEncounter/EArkInputPorts */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeEliminateCurrentPopulation/EArkOutputPorts - /PreyDll.pdb/CArkFlowNodeAddEncounter/EArkOutputPorts */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeEmailAcquired/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeFocusModeEnabled/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeFocusModeEnabled/EArkOutputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkOutputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeGetEquippedItem/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeGetEquippedItem/EArkOutputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkOutputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeGiveAbility/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeGiveCharacterInfo/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeGiveCharacterPassword/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeGiveKeyCard/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeGiveKeyCode/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeGiveNote/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeHasAbility/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeHasAbility/EArkOutputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkOutputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeHasCharacterPassword/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeHasCharacterPassword/EArkOutputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkOutputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeHasDiscoveredLocation/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeHasDiscoveredLocation/EArkOutputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkOutputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeHasDownload/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeHasDownload/EArkOutputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkOutputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeHasEmail/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeHasEmail/EArkOutputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkOutputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeHasKeyCard/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeHasKeyCard/EArkOutputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkOutputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeHasKeyCode/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeHasKeyCode/EArkOutputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkOutputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeHasNote/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeHasNote/EArkOutputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkOutputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeHideEmail/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeIsTrial/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeKeyCardAcquired/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeKeyCodeAcquired/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeLearnResearch/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeMarkEmailUnread/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeNotificationDialog/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeNotificationDialog/EArkOutputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkOutputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodePsiPowerActive/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodePsiPowerActive/EArkOutputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkOutputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodePsiPowerMenuActive/EArkOutputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkOutputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodePsiPowerTargetingActive/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodePsiPowerTargetingActive/EArkOutputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkOutputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodePsiScanning/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodePurchaseRandomAbilities/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeRefreshCurrentPopulation/EArkInputPorts - /PreyDll.pdb/CArkFlowNodeAddEncounter/EArkInputPorts */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeRefreshCurrentPopulation/EArkOutputPorts - /PreyDll.pdb/CArkFlowNodeAddEncounter/EArkOutputPorts */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeSendEmail/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeSetDownloadHidden/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeSetEncounterProfile/EArkInputPorts - /PreyDll.pdb/CArkFlowNodeAddEncounter/EArkInputPorts */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeSetEncounterProfile/EArkOutputPorts - /PreyDll.pdb/CArkFlowNodeAddEncounter/EArkOutputPorts */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeSetEquippedItem/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeSetEquippedItem/EArkOutputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkOutputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeSetRepopulationDataForProfile/EArkInputPorts - /PreyDll.pdb/CArkFlowNodeAddEncounter/EArkInputPorts */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeSetRepopulationDataForProfile/EArkOutputPorts - /PreyDll.pdb/CArkFlowNodeAddEncounter/EArkOutputPorts */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeSetSpawnerActiveForEncounters/EArkInputPorts - /PreyDll.pdb/CArkFlowNodeAddEncounter/EArkInputPorts */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeSetSpawnerActiveForEncounters/EArkOutputPorts - /PreyDll.pdb/CArkFlowNodeAddEncounter/EArkOutputPorts */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeSetUtilityButtonEnabled/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeSetUtilityButtonHidden/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeSetUtilityEnabled/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeSetUtilityHidden/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeSpawnAtTravelDest/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeSpawnAtTravelDest/EArkOutputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkOutputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeSpawnProjectile/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeStatChanged/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeTriggerRepopulation/EArkInputPorts - /PreyDll.pdb/CArkFlowNodeAddEncounter/EArkInputPorts */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeTriggerRepopulation/EArkOutputPorts - /PreyDll.pdb/CArkFlowNodeAddEncounter/EArkOutputPorts */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeUnequipItem/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeUnequipItem/EArkOutputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkOutputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeWorkstationPopup/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeWorkstationPopup/EArkOutputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkOutputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeWorkstationResult/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */


/* WARNING! conflicting data type names: /PreyDll.pdb/CArkFlowNodeWorkstationResult/EArkOutputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkOutputs */

enum class EArkMoveDir {
    right=0,
    left=1,
    up=2,
    down=3
};

enum class EArkPAChannels {
    global=0,
    total=4
};

enum class EArkCombineAction {
    none=0,
    repair=1,
    upgrade=2,
    stack=3
};

enum class EArkFogUpdateState {
    invalid=0,
    calculate=1,
    send=2,
    icons=3
};

enum class EArkAirlockState {
    none=0,
    open=1,
    locked=2
};

enum class EArkProjectileTimer {
    lifetime=272,
    hidetime=273
};

enum class EArkGooState {
    none=0,
    adding=1,
    locked=2
};

enum class EArkRecyclerGrenadeState {
    none=0,
    initialize=1,
    launch=2,
    detonate=3,
    vibrateObjects=4,
    gravityPull=5,
    recycle=6,
    destroyed=7
};

enum class EArkWeaponStatus {
    normal=0,
    damaged=1,
    broken=2,
    invalid=3
};


/* WARNING! conflicting data type names: /PreyDll.pdb/CFlowNode_InventoryAddItem/EArkInputPorts - /PreyDll.pdb/CArkFlowNodeAddEncounter/EArkInputPorts */


/* WARNING! conflicting data type names: /PreyDll.pdb/CFlowNode_InventoryAddItem/EArkOutputPorts - /PreyDll.pdb/CArkFlowNodeAddEncounter/EArkOutputPorts */


/* WARNING! conflicting data type names: /PreyDll.pdb/CFlowNode_InventoryAddItemById/EArkInputPorts - /PreyDll.pdb/CArkFlowNodeAddEncounter/EArkInputPorts */


/* WARNING! conflicting data type names: /PreyDll.pdb/CFlowNode_InventoryAddItemById/EArkOutputPorts - /PreyDll.pdb/CArkFlowNodeAddEncounter/EArkOutputPorts */


/* WARNING! conflicting data type names: /PreyDll.pdb/CFlowNode_InventoryGetItemQuantity/EArkInputPorts - /PreyDll.pdb/CArkFlowNodeAddEncounter/EArkInputPorts */


/* WARNING! conflicting data type names: /PreyDll.pdb/CFlowNode_InventoryGetItemQuantity/EArkOutputPorts - /PreyDll.pdb/CArkFlowNodeAddEncounter/EArkOutputPorts */


/* WARNING! conflicting data type names: /PreyDll.pdb/CFlowNode_InventoryHasItem/EArkInputPorts - /PreyDll.pdb/CArkFlowNodeAddEncounter/EArkInputPorts */


/* WARNING! conflicting data type names: /PreyDll.pdb/CFlowNode_InventoryHasItem/EArkOutputPorts - /PreyDll.pdb/CArkFlowNodeAddEncounter/EArkOutputPorts */


/* WARNING! conflicting data type names: /PreyDll.pdb/CFlowNode_InventoryItemListener/EArkInputPorts - /PreyDll.pdb/CArkFlowNodeAddEncounter/EArkInputPorts */


/* WARNING! conflicting data type names: /PreyDll.pdb/CFlowNode_InventoryItemListener/EArkOutputPorts - /PreyDll.pdb/CArkFlowNodeAddEncounter/EArkOutputPorts */


/* WARNING! conflicting data type names: /PreyDll.pdb/CFlowNode_InventoryRemoveAllItems/EArkInputPorts - /PreyDll.pdb/CArkFlowNodeAddEncounter/EArkInputPorts */


/* WARNING! conflicting data type names: /PreyDll.pdb/CFlowNode_InventoryRemoveAllItems/EArkOutputPorts - /PreyDll.pdb/CArkFlowNodeAddEncounter/EArkOutputPorts */


/* WARNING! conflicting data type names: /PreyDll.pdb/CFlowNode_InventoryRemoveItem/EArkInputPorts - /PreyDll.pdb/CArkFlowNodeAddEncounter/EArkInputPorts */


/* WARNING! conflicting data type names: /PreyDll.pdb/CFlowNode_InventoryRemoveItem/EArkOutputPorts - /PreyDll.pdb/CArkFlowNodeAddEncounter/EArkOutputPorts */


/* WARNING! conflicting data type names: /PreyDll.pdb/CFlowNode_PlayerLookAt/EArkOutputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkOutputs */

enum class EArkInventoryListenerUpdates {
    addItem=0,
    removeItem=1,
    setAmmoCount=2,
    clearInventory=3
};

enum class EArkPlayerAnimationPriority {
    APAP_Lowest=0,
    APAP_Movement=1,
    APAP_PsiPower=3,
    APAP_Normal=4,
    APAP_High=5,
    APAP_PDA=6,
    APAP_Trackview=7
};

enum class EArkInventorySizes {
    smallExternal=1,
    mediumExternal=2,
    largeExternal=3,
    player=4
};

enum class EArkAccessState {
    Available=0,
    Locked=1,
    Hidden=2
};

enum class EArkAiTreeBookendFlags {
    none=0,
    resumedInitialized=1,
    resumedEnabled=2,
    resumedActivated=4,
    resumed=7,
    suspendedInitialized=8,
    initialized=9,
    suspendedEnabled=16,
    enabled=18,
    suspendedActivated=32,
    activated=36,
    suspended=56
};

enum class EArkAiTreeCompletionResult {
    unsuccessful=0,
    successful=1
};

enum class EArkAiTreeCorruptNpcNodeState {
    tryingToCorrupt=0,
    performingCorruption=1,
    complete=2
};

enum class EArkAiTreeSearchReason {
    indirectAnomaly=0,
    indirectDanger=1,
    direct=2
};

enum class EArkAiTreeStatus {
    uninitialized=0,
    resumedInitialized=1,
    resumedEnabled=3,
    resumedActivated=7,
    suspendedInitialized=9,
    suspendedEnabled=11,
    suspendedActivated=15
};

enum class EArkAiTreeStatusFlags : uint8_t {
    uninitialized=0,
    initialized=1,
    enabled=2,
    activated=4,
    suspended=8
};

enum class EArkAiTreeStimulusResult {
    notConsumed=0,
    consumed=1
};

enum class EArkAspect {
    unknown=0,
    fourByThree=1,
    sixteenByNine=2,
    sixteenByTen=3,
    other=4
};

enum class EArkAttentionChangeReason {
    initialized=0,
    cleared=1,
    demoted=2,
    drained=3,
    simpleGain=4,
    clamped=5,
    ambient=6,
    heardAnomaly=7,
    flashlight=8,
    heardDanger=9,
    heardCallForHelp=10,
    hitByProjectile=11,
    remotelyManipulated=12,
    observed=13,
    etherDuplicate=14,
    touched=15
};

enum class EArkAttentionLevel : uint32_t {
    unknown=0,
    noticed=1,
    curious=2,
    interested=3,
    known=4,
    count_=5
};

enum class EArkBinkLoadFlags {
    None=0,
    AutoStart=1,
    Looping=2
};

enum class EArkBinkMovieType {
    Unknown=0,
    Fullscreen=1,
    Background=2,
    Texture=3
};

enum class EArkBinkPlayerFlags {
    None=0,
    Valid=1,
    Playing=2,
    Paused=4,
    QueueStart=8
};

enum class EArkBnetLoggingPlatform {
    INVALID_PLATFORM=0,
    Windows=1,
    MacOS=2,
    AppleAppStore=3,
    GooglePlayStore=4,
    MicrosoftStore=7,
    Playstation=8,
    Web=9,
    Steam=10,
    AmazonAppStore=11,
    GOG=12,
    NintendoNSA=13,
    NoLauncher=14,
    Server=15,
    Twitch=16,
    Xbox=17,
    Stadia=18,
    Epic=21
};

class EArkBnetLoggingProductData {
    unsigned __int64 intLoggingId,
     certLoggingId,
     prodLoggingId,
     loadTestLoggingId;
};

enum class EArkCameraShakePhase {
    Invalid=0,
    RampIn=1,
    Hold=2,
    RampOut=3
};

enum class EArkCameraShakeType {
    Invalid=0,
    Procedural=1,
    Animated=2
};

enum class EArkChimeType {
    none=0,
    chime=1,
    interference=2
};

enum class EArkCombatRole {
    none=0,
    melee=1,
    ranged=2,
    count_=3
};

enum class EArkComparisonType {
    EqualTo=0,
    NotEqual=1,
    GreaterThan=2,
    LessThan=3,
    GreaterThanOrEqualTo=4,
    LessThanOrEqualTo=5,
    Last=6
};

enum class EArkConditionTriggerType {
    Force=0,
    GainedItem=1,
    LostItem=2,
    AcquiredChipset=3,
    HealthAmount=4,
    PsiAmount=5,
    ObjectiveStatusChange=6,
    TaskStatusChange=7,
    InteractedWith=8,
    LookingAtNear=9,
    LookingAtFar=10,
    PlayerStatus=11,
    AbilityAcquired=12,
    TutorialOnCooldown=13,
    ConditionExecuted=14,
    FlashlightOn=15,
    WeaponDurability=16,
    GameMetric=17,
    TraumaActive=18,
    PlayerInCombat=19,
    DifficultyOption=20
};

enum class EArkContractType {
    precondition=0,
    postcondition=1
};

enum class EArkConversationDominance {
    lowest=0,
    normal=0,
    important=1,
    highest=2,
    transcribe=2
};

enum class EArkConversationPriority {
    veryhigh=0,
    high=1,
    medium=2,
    low=3,
    verylow=4,
    none=5,
    last=6
};

enum class EArkConversationStatus {
    not_started=0,
    started=1,
    interrupted=2,
    completed=3,
    last=4
};

enum class EArkConversationType {
    systemic=0,
    trackView=1,
    last=2
};

enum class EArkCraftingIngredientType {
    Organic=0,
    Mineral=1,
    Synthetic=2,
    Exotic=3,
    Count=4
};

enum class EArkCriterionKeyType {
    responseKey=0,
    gameToken=1,
    writeback=2,
    conversation=3,
    objective=4,
    metatag=5,
    task=6
};

enum class EArkCystoidNestState {
    Invalid=0,
    Inactive=1,
    Primed=2,
    Spawning=3,
    Depleted=4
};

enum class EArkCystoidState {
    eInvalid=0,
    eRest=1,
    eFollow=2,
    eAttack=3,
    eDead=4,
    eEnergized=5
};

enum class EArkDeferredActionStatus {
    ready=0,
    pending=1,
    aborted=2,
    completed=3
};

enum class EArkDialogPlayerType {
    invalid=0,
    base=1,
    npc=2,
    pa=3,
    transcribe=4,
    turret=5,
    last=6
};

enum class EArkDifficultyLevel {
    invalid=-1,
    story=0,
    easy=1,
    normal=2,
    hard=3,
    nightmare=4,
    count=5
};

enum class EArkDifficultyOption {
    ironman=0,
    weaponDegradation=1,
    allTraumas=2,
    oxygen=3,
    count=4
};

enum class EArkDispenserError {
    None=0,
    MaxedOut=1,
    LifetimeMaxReached=2,
    Blocked=3
};

enum class EArkDispenserState {
    Invalid=0,
    Locked=1,
    Unlocked=2,
    Processing=3,
    Error=4,
    Overridden=5
};

enum class EArkDisposition {
    none=0,
    friendly=1,
    neutral=2,
    hostile=3,
    count_=4
};

enum class EArkDistractedStateId {
    any=-1,
    null=0,
    movingTo=1,
    movingClose=2,
    usingDistractor=3,
    arrived=4,
    count_=5
};

enum class EArkDockingBay {
    none=0,
    bay1=1,
    bay2=2
};

enum class EArkDockingState {
    undocked=0,
    docking=1,
    docked=2,
    undocking=3
};

enum class EArkElevatorKioskState {
    Default=0,
    Processing=1,
    Fail=2
};

enum class EArkEthericDoppelgangerStage {
    Casting=0,
    MovingApart=1,
    SpawningDoppelgangers=2,
    Done=3
};

enum class EArkExplosiveTankState{
    Invalid=0,
    Intact=1,
    Leaking=2,
    PreExplode=3,
    Destroyed=4
};

enum class EArkFabricatorState {
    Invalid=0,
    PoweredOff=1,
    StandBy=2,
    InUse=3,
    Fabricating=4,
    ItemReady=5
};

enum class EArkFirstPartyEnvironment {
    Invalid=0,
    Integration=1,
    Cert=2,
    Production=3
};

enum class EArkFirstPartyEnvironmentQueryResponse {
    Success=0,
    NotReady=1,
    Failure=2
};

enum class EArkFleeCorneredBehavior {
    none=0,
    cower=1,
    fight=2
};

enum class EArkFsmTransitionPredicateResult {
    disallow=0,
    allow=1,
    unspecified=2
};

enum class EArkGibletType {
    smallGib=0,
    mediumGib=1,
    largeGib=2,
    limbGib=3,
    extraLargeGib=4,
    count_=5
};

enum class EArkHUDTutorialHintChannel {
    Weapon=0,
    Tutorial=1,
    Count=2
};

enum class EArkHumanCombatStateId {
    any=-1,
    null=0,
    shooting=1,
    reloading=2,
    unreachableTargeting=3,
    count_=4
};


/* WARNING! conflicting data type names: /PreyDll.pdb/EArkInputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkInputs */

enum class EArkInteractionMode {
    use=0,
    holdUse=1,
    loot=2,
    special=3,
    last=4,
    remoteManipulation=4
};

enum class EArkInteractionPrompt {
    examine=4,
    tutorial=5
};

enum class EArkInteractionType {
    none=0,
    scriptDefined=1,
    unavailable=2,
    codeDefined=3,
    pickup=4,
    consume=5,
    carry=6,
    hack=7,
    repair=8,
    fortify=9,
    examine=10,
    equip=11,
    hoover=12,
    last=13
};

enum class EArkInteractiveScreenType {
    fabricator=0,
    keycardReader=1,
    keypad=2,
    kiosk=3,
    operatorDispenser=4,
    securityStation=5,
    workstation=6,
    count=7
};

enum class EArkKeycardReaderState {
    Invalid=0,
    Locked=1,
    Unlocked=2,
    Error=3
};

enum class EArkKioskButtonState {
    Disabled=0,
    Default=1,
    Processing=2,
    Error=3
};

enum class EArkLightInterferenceState {
    rampIn=0,
    bright=1,
    rampOut=2,
    dark=3,
    none=4
};

enum class EArkLineOfSightCheckPolicy {
    succeedIfAnyLineClear=0,
    failIfAnyLineObstructed=1
};

enum class EArkLineOfSightCheckStatus {
    ready=0,
    pending=1,
    succeeded=2,
    failed=3
};

enum class EArkLoadDisconnectReason {
    none=0,
    signout=1,
    corruptSave=2
};

enum class EArkLogType {
    Unspecified=0,
    ItemPickup=1,
    SneakAttack=2,
    CriticalAttack=3,
    MimicHit=4,
    Chipset=5,
    EntitlementUnlock=6,
    Airlock=7
};

enum class EArkMimicryReason {
    ambush=0,
    escape=1,
    explore=2,
    count_=3
};

enum class EArkMoveDescriptor {
    None=-1,
    Hover=0,
    LookDisabled=1,
    Shift=2,
    Large=3,
    Small=4,
    MultiDir=5,
    ZeroG=6,
    DispenserSpawn=7,
    DoorAnimation=8,
    DoorCrouch=9,
    EntityMovement=10,
    MeleeFacing=11,
    GravShaft=12,
    HoverZeroGIgnoreObstacles=13,
    Max=14
};

enum class EArkNightmareCombatStateId {
    any=-1,
    null=0,
    largeForm=1,
    meleePosition=2,
    unreachableTargeting=3,
    regainLoS=4,
    avoidDamage=5,
    debug=6,
    count_=7
};

enum class EArkNpcAbilityPerformanceResult {
    failed=0,
    completed=1,
    inProgress=2
};

enum class EArkNpcAbilityPrereqStatus {
    k_unevaluated=-1,
    k_unfulfilled=0,
    k_fulfilled=1
};

enum class EArkNpcAliveStateId {
    any=-1,
    null=0,
    broken=1,
    conscious=2,
    unconscious=3,
    count_=4
};

enum class EArkNpcAmbientSoundId {
    living=0,
    mimicking=1,
    _count=2
};

enum class EArkNpcBlackboardTargetType {
    hostile=0
};

enum class EArkNpcBodyStateId {
    any=-1,
    null=0,
    animated=1,
    animatedAbility=2,
    animatedDistraction=3,
    animatedFall=4,
    animatedStunned=5,
    breakout=6,
    cower=7,
    dead=8,
    dissipate=9,
    dodge=10,
    fatality=11,
    frozen=12,
    gravShaft=13,
    hitReact=14,
    hitReactShift=15,
    hypnotized=16,
    jump=17,
    landing=18,
    lifted=19,
    lurking=20,
    lurkOut=21,
    mantle=22,
    mimicking=23,
    notice=24,
    ragdoll=25,
    raised=26,
    react=27,
    ready=28,
    rebound=29,
    resist=30,
    resistScrunch=31,
    searchExamine=32,
    surprised=33,
    trackview=34,
    unanimatedAbility=35,
    count_=36
};

enum class EArkNpcConsciousStateId {
    any=-1,
    null=0,
    alert=1,
    dormant=2,
    count_=3
};

enum class EArkNpcDamageStateId {
    any=-1,
    null=0,
    damaged=1,
    destroyed=2,
    disabled=3,
    undamaged=4,
    count_=5
};

enum class EArkNpcDesireTargetRefreshResult {
    noChange=0,
    positionChanged=1,
    targetLost=2
};

enum class EArkNpcDesireTargetType {
    none=0,
    position=1,
    entity=2,
    bone=3,
    offset=4,
    boundsCenter=5,
    proxy=6,
    camera=7,
    direction=8,
    attackPos=9
};

enum class EArkNpcFacingDesireCallbackEvent {
    activated=0,
    deactivated=1,
    fulfilled=2,
    failed=3,
    resumed=4,
    targetMoved=5,
    targetLost=6
};

enum class EArkNpcFacingDesirePriority {
    ambient=0,
    movement=1,
    distracted=2,
    following=3,
    attention=4,
    search=5,
    combat=6,
    surprise=7,
    escape=8,
    script=9,
    flowGraph=10,
    trackView=11,
    conversation=12,
    fullbodyAction=13,
    reaction=14,
    ragdollBlend=15,
    debug=16
};

enum class EArkNpcLifecycleStateId {
    any=-1,
    null=0,
    alive=1,
    dead=2,
    count_=3
};

enum class EArkNpcLookDesireCallbackEvent {
    activated=0,
    deactivated=1,
    fulfilled=2,
    resumed=3,
    targetMoved=4,
    targetLost=5
};

enum class EArkNpcLookDesirePriority {
    ambient=0,
    distracted=1,
    following=2,
    attention=3,
    search=4,
    combat=5,
    surprise=6,
    script=7,
    flowGraph=8,
    trackView=9,
    conversation=10,
    fullbodyAction=11,
    reaction=12,
    ragdollBlend=13,
    debug=14
};

enum class EArkNpcMimicryReason {
    none=0,
    ambush=1,
    escape=2,
    timid=3,
    wander=4,
    _count=5
};

enum class EArkNpcMovementDesireCallbackEvent {
    activated=0,
    deactivated=1,
    fulfilled=2,
    failed=3,
    resumed=4,
    destinationMoved=5,
    destinationLost=6
};

enum class EArkNpcMovementDesirePriority {
    ambient=0,
    distracted=1,
    following=2,
    flowgraphAmbient=3,
    attention=4,
    search=5,
    combat=6,
    surprise=7,
    escape=8,
    script=9,
    flowGraph=10,
    trackView=11,
    conversation=12,
    fullbodyAction=13,
    reaction=14,
    gooed=15,
    ragdollBlend=16,
    debug=17
};

enum class EArkNpcNoiseState : uint32_t{
    idle=0,
    notice=1,
    stare=2,
    search=3,
    combat=4,
    count_=5
};

enum class EArkNpcNoiseStateDesirePriority {
    npc=0,
    notice=1,
    search=2,
    combat=3
};

enum class EArkNpcPlayerTimer {
    delayedDeathBodyCharSwap=600,
    delayedHideTimer=601,
    beginDisintegration=602,
    delayOptimizedDeadUpdates=603,
    delayForceUpdateRagdoll=604
};

enum class EArkNpcSpeedDesireCallbackEvent {
    activated=0,
    deactivated=1
};

enum class EArkNpcSpeedDesirePriority {
    ambient=0,
    distracted=1,
    following=2,
    attention=3,
    search=4,
    combat=5,
    escape=6,
    script=7,
    flowGraph=8,
    trackView=9,
    fullbodyAction=10,
    reaction=11,
    debug=12
};

enum class EArkNpcSpeedType : int32_t{
    normal=0,
    fast=1,
    fastest=2
};

enum class EArkNpcVisionDirection : int32_t {
    cameraBone=0,
    lookJoint=1,
    mimic=2,
    cameraBoneForward=3
};

enum class EArkObjectiveState {
    UNASSIGNED=0,
    ASSIGNED=1,
    COMPLETED=2,
    FAILED=3,
    CANCELLED=4
};

enum class EArkObjectiveTaskState {
    ACTIVE=0,
    INACTIVE=1,
    COMPLETED=2,
    FAILED=3
};

enum class EArkObstacleAvoidanceUrgency {
    Low=0,
    High=1
};

enum class EArkOffMeshLinkType {
    Unset=0,
    Door=1,
    Mantle=2,
    GravShaft=3,
    Gloo=4
};

enum class EArkOptionPlatform {
    All=0,
    PC=1,
    Console=2,
    Last=3
};


/* WARNING! conflicting data type names: /PreyDll.pdb/EArkOutputs - /PreyDll.pdb/ArkFlowNode_FabricationPlanAcquired/EArkOutputs */

enum class EArkPatrolType {
    eRandom=0,
    ePingPong=1,
    eLooped=2,
    eOneWay=3,
    eInvalid=4
};

enum class EArkPhantomCombatStateId {
    any=-1,
    null=0,
    rangedAttacking=1,
    rangedShifting=2,
    regainLineOfSight=3,
    debugState=4,
    meleeAttacking=5,
    unreachableTargeting=6,
    reacquireTargetAngle=7,
    meleeShifting=8,
    shiftToRange=9,
    movingBackIntoRange=10,
    count_=11
};

enum class EArkPlayerLightPriority {
    Weapon=0,
    PsiPower=1,
    Flashlight=2,
    Psychoscope=3,
    Count=4
};

enum class EArkPlayerMovementStateId {
    any=-1,
    null=0,
    ground=1,
    death=2,
    deathByRecyclerGrenade=3,
    jump=4,
    fall=5,
    climb=6,
    fly=7,
    smoke=8,
    zerog=9,
    cinematic=10,
    slide=11,
    mimic=12,
    gravShaftG=13,
    gravShaftToG=14,
    gravShaftZeroG=15,
    lift=16,
    shift=17,
    count_=18
};

enum class EArkPlayerStatus {
    Invalid=0,
    Bleeding=1,
    Blind=2,
    Burning=3,
    Concussion=4,
    Crippled=5,
    Disruption=6,
    Exhausted=7,
    Fear=8,
    PsychoShock=9,
    Radiation=10,
    SuitIntegrity=11,
    WellFed=12,
    Drunk=13,
    GoodDrunk=14,
    Stun=15,
    Oxygen=16,
    Last=17
};

enum class EArkPoltergeistCombatStateId {
    any=-1,
    null=0,
    defaultState=1,
    unreachableTargeting=2,
    count_=3
};

enum class EArkPoltergeistVisibilityState {
    Invisible=0,
    Visible=1,
    Appearing=2,
    Disappearing=3,
    PsychoscopeAppearing=4,
    PsychoscopeVisible=5,
    PsychoscopeDisappearing=6
};

enum class EArkPostEffectDataType {
    Any=-1,
    Boolean=0,
    Integer=1,
    Float=2,
    Color=3,
    Vector4=4,
    Texture=5
};

enum class EArkPreGameOptions {
    difficulty=0,
    survival=1,
    next=2,
    survivalOption=3
};

enum class EArkPsiPowerError {
    None=0,
    NoTargets=1
};

enum class EArkPsiPowers : int32_t {
    firstPower=-1,
    electrostaticBurst=0,
    thermalBlast=1,
    kineticShield=2,
    kineticBlast=3,
    lift=4,
    remoteManipulation=5,
    mimic=6,
    smokeForm=7,
    fear=8,
    mindBlast=9,
    hypnosis=10,
    cyberkinesis=11,
    createPhantom=12,
    shift=13,
    combatFocus=14,
    empty=15,
    last=16
};

enum class EArkQueryDataType {
    instance=0,
    memory=1,
    global=2,
    last=3
};

enum class EArkRawMaterialClass {
    Invalid=0,
    Small=1,
    Medium=2,
    Large=3,
    XLarge=4
};

enum class EArkRecyclerState {
    Invalid=0,
    Idle=1,
    Processing=2,
    Producing=3
};

enum class EArkRelativeDirection {
    front=0,
    back=1,
    right=2,
    left=3
};

enum class EArkResponseCompareType {
    equals=0,
    greater_than=1,
    less_than=2,
    greater_eq=3,
    less_eq=4,
    not_equal=5,
    is_a=6,
    num_values=7
};

enum class EArkResponseKey {
    voice=0,
    concept=1,
    random=2,
    talking=3,
    itemName=4,
    entityClassName=5,
    distance=6,
    health=7,
    group=8,
    damage=9,
    character=10,
    location=11,
    playerHealth=12,
    corrupted=13,
    trauma=14,
    traumaCount=15,
    attentionLevel=16,
    playerIsInCombat=17,
    priority=18,
    dispositionToPlayer=19,
    npcIsControlled=20,
    upgrades=21,
    durability=22,
    ammo=23,
    kills=24,
    importantConversation=25,
    isBroken=26,
    isCowering=27,
    isFeared=28,
    isGlooed=29,
    isLurking=30,
    playerAbilityTier=31,
    playerAlternateForm=32,
    playerNeuromods=33,
    playerPsi=34,
    distraction=35,
    targetCharacter=36,
    suitIntegrity=37,
    scansRemaining=38,
    last=39
};

enum class EArkSaveFailure {
    None=0,
    Cinematic=1,
    Dead=2,
    LowHealth=3,
    LowOxygen=4,
    InAir=5,
    HazardSignal=6,
    ImpendingDanger=7,
    InCombat=8,
    Shifting=9,
    MimicGrab=10
};

enum class EArkSaveFileType {
    autosave=0,
    quicksave=1,
    manual=2,
    ironman=3,
    count_=4
};

enum class EArkSaveLoadMode {
    disabled=0,
    prompt=1,
    newGame=2,
    newGamePlus=3,
    load=4,
    save=5,
    gameOptions=6,
    loadTransition=7
};

enum class EArkSearchFsmStateId {
    any=-1,
    null=0,
    debugState=1,
    investigate=2,
    examineAnim=3,
    examineStare=4,
    hunt=5,
    count_=6
};

enum class EArkSilhouetteType {
    MarkedEnemy=1,
    Pickup=2,
    PsiTargeting=4
};

enum class EArkStationWorldUIState {
    Invalid=0,
    Locked=1,
    RootMenu=2,
    Email=3,
    Utility=4,
    ChildDefined=5
};

enum class EArkSurfaceHazardState {
    Invalid=0,
    Inert=1,
    Triggered=2,
    Hazard=3,
    Depleted=4
};

enum class EArkTaskMarkerStyle {
    main=0,
    roster=1,
    side=2,
    _count=3
};

enum class EArkTechnopathCombatStateId {
    any=-1,
    null=0,
    attackingInPlace=1,
    retreatingFromTarget=2,
    movingToTarget=3,
    regainLoS=4,
    debugState=5,
    count_=6
};

enum class EArkTelepathCombatStateId {
    any=-1,
    null=0,
    attackingInPlace=1,
    retreatingFromTarget=2,
    movingToTarget=3,
    regainLoS=4,
    debugState=5,
    count_=6
};

enum class EArkTurretAnimStateId {
    any=-1,
    null=0,
    undeployed=1,
    deploying=2,
    deployed=3,
    undeploying=4,
    destroyed=5,
    broken=6,
    searching=7,
    count_=8
};

enum class EArkTurretDamageStateId {
    any=-1,
    null=0,
    undamaged=1,
    damaged=2,
    disabled=3,
    count_=4
};

enum class EArkTurretSFX {
    eDetect=0,
    eLostTarget=1,
    eFire=2,
    eDestroy=3,
    eGooStuck=4,
    eDamagedStart=5,
    eDamagedStop=6,
    eDisabledStart=7,
    eDisabledStopRepaired=8,
    eDisabledStopDestroyed=9,
    eStunnedStart=10,
    eStunnedStop=11,
    eSize=12
};

enum class EArkTurretStateId {
    any=-1,
    null=0,
    undeployed=1,
    fallenSpaz=2,
    attacking=3,
    searching=4,
    flowgraphControlled=5,
    playerControlled=6,
    playerCarried=7,
    disabled=8,
    count_=9
};

enum class EArkVitalSign {
    NoLifeSigns=0,
    Nominal=1,
    Error=2,
    Safe=3,
    Last=4
};

enum class EArkWeaverEscapeStateId {
    any=-1,
    null=0,
    attacking=1,
    movingToRetreatObject=2,
    spooked=3,
    done=4,
    debugState=5,
    count_=6
};

enum class EArkWritebackType {
    set=0,
    increment=1,
    decrement=2,
    last=3
};

enum class EArkZoomPriority {
    none=0,
    lowest=1,
    low=2,
    normal=3,
    high=4,
    highest=5
};

