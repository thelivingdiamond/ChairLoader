local L0_9113, L1_9114, L2_9115, L3_9116
L0_9113 = {}
L1_9114 = {}
L0_9113.Sounds = L1_9114
L1_9114 = {}
L1_9114.bDisableEngine = 0
L1_9114.Modification = ""
L1_9114.soclasses_SmartObjectClass = ""
L0_9113.Properties = L1_9114
L1_9114 = {}
L0_9113.turretAngles = L1_9114
L1_9114 = {}
L0_9113.difPos = L1_9114
L1_9114 = {}
L0_9113.Client = L1_9114
L1_9114 = {}
L0_9113.Server = L1_9114
Abrams = L0_9113
L0_9113 = Abrams
function L1_9114(A0_9117)
  local L1_9118
  L1_9118 = A0_9117.AIFireProperties
  if L1_9118 then
    L1_9118 = A0_9117.AIFireProperties
    L1_9118[1] = {}
    L1_9118 = A0_9117.AIFireProperties
    L1_9118 = L1_9118[1]
    L1_9118.min_distance = 20
    L1_9118 = A0_9117.AIFireProperties
    L1_9118 = L1_9118[1]
    L1_9118.max_distance = 400
  end
end
L0_9113.GetSpecificFireProperties = L1_9114
L0_9113 = Abrams
L1_9114 = {}
L2_9115 = AIOBJECT_CAR
L1_9114.AIType = L2_9115
L2_9115 = AICombatClasses
L2_9115 = L2_9115.Tank
L1_9114.AICombatClass = L2_9115
L1_9114.AIDefaultSpecies = 1
L2_9115 = {}
L2_9115.aibehavior_behaviour = "TankIdle"
L2_9115.triggerRadius = 90
L1_9114.PropertiesInstance = L2_9115
L2_9115 = {}
L2_9115.aicharacter_character = "Tank"
L2_9115.bHidesPlayer = 0
L3_9116 = {}
L3_9116.FOVPrimary = -1
L3_9116.FOVSecondary = -1
L3_9116.sightrange = 400
L3_9116.persistence = 10
L2_9115.Perception = L3_9116
L1_9114.Properties = L2_9115
L2_9115 = {}
L2_9115.walkSpeed = 7
L2_9115.runSpeed = 11
L2_9115.sprintSpeed = 15
L2_9115.maneuverSpeed = 5
L2_9115.minTurnRadius = 0.2
L2_9115.maxTurnRadius = 10
L2_9115.pathType = "AIPATH_TANK"
L2_9115.pathLookAhead = 8
L2_9115.pathRadius = 3
L2_9115.pathSpeedLookAheadPerSpeed = 1
L2_9115.cornerSlowDown = 0.75
L2_9115.pathFindPrediction = 1
L2_9115.maneuverTrh = 2
L2_9115.passRadius = 5
L2_9115.resolveStickingInTrace = 0
L2_9115.pathRegenIntervalDuringTrace = 4
L2_9115.avoidanceRadius = 10
L1_9114.AIMovementAbility = L2_9115
L1_9114.forgetTimeTarget = 32
L1_9114.forgetTimeSeek = 40
L1_9114.forgetTimeMemory = 40
L0_9113.AIProperties = L1_9114
