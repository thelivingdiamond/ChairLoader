local L0_3419, L1_3420, L2_3421, L3_3422, L4_3423
L0_3419 = {}
L1_3420 = {}
L1_3420.object_GeometryFP = "Objects/Arkeffects/Characters/Robots/Turret/arkturret_tracermesh.cgf"
L2_3421 = {}
L2_3421.sType = "particle"
L2_3421.bPhysicalize = 1
L2_3421.bRigidBody = 1
L2_3421.bPushableByPlayers = 0
L2_3421.bPushableByAI = 0
L2_3421.bRigidBodyActive = 1
L2_3421.signalpackage_collisionDamagePackage = "3149325216948552760"
L2_3421.Density = -1
L2_3421.Mass = 0.25
L2_3421.Speed = 150
L3_3422 = {}
L3_3422.fSize = 0.5
L4_3423 = {}
L4_3423.x = 0
L4_3423.y = 0
L4_3423.z = 0
L3_3422.vGravity = L4_3423
L2_3421.Particle = L3_3422
L1_3420.Physics = L2_3421
L2_3421 = {}
L2_3421.fLifetime = 0.6
L2_3421.fHideTime = 0.05
L2_3421.fTimeBeforeOwnerCollides = 0
L2_3421.bDestroyOnCollision = 1
L2_3421.bAlignWithVelocity = 1
L2_3421.metaTags_SneakAttackTargetTags = "9469288860498988665,9469288860498988629,9469288860517748071"
L2_3421.signalpackage_SignalPackageId = "10641886185795225404"
L2_3421.signalpackage_CriticalSignalPackageId = ""
L2_3421.hitType = "ArkProjectileTurretBullet"
L2_3421.bCollideWithEtherForm = false
L3_3422 = {}
L2_3421.TrailEffect = L3_3422
L1_3420.Projectile = L2_3421
L0_3419.Properties = L1_3420
ArkProjectileTurretBullet = L0_3419
