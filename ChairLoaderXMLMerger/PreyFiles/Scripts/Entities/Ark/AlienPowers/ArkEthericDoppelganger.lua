local L0_3083, L1_3084, L2_3085, L3_3086, L4_3087
L0_3083 = {}
L1_3084 = {}
L1_3084.object_GeometryFP = "Objects/ArkEffects/Aliens/Phantom/phantom_ethericdoppelganger_00.cgf"
L2_3085 = {}
L2_3085.sType = "particle"
L2_3085.bPhysicalize = 1
L2_3085.bRigidBody = 1
L2_3085.bPushableByPlayers = 0
L2_3085.bPushableByAI = 0
L2_3085.bRigidBodyActive = 1
L2_3085.Density = -1
L2_3085.Mass = 1
L2_3085.Speed = 18.75
L3_3086 = {}
L3_3086.fSize = 0.5
L4_3087 = {}
L4_3087.x = 0
L4_3087.y = 0
L4_3087.z = -0.1
L3_3086.vGravity = L4_3087
L2_3085.Particle = L3_3086
L1_3084.Physics = L2_3085
L2_3085 = {}
L2_3085.fLifetime = 2000000
L2_3085.fTimeBeforeOwnerCollides = 0
L2_3085.fPlayerCollisionCheckRadius = 1.5
L2_3085.bDestroyOnCollision = 1
L2_3085.signalpackage_SignalPackageId = "10641886185795227403"
L2_3085.hitType = "ArkPhantomKineticProjectile"
L2_3085.fDestroyDelay = 0.5
L3_3086 = {}
L3_3086.particleeffect_ParticleEffect = "Characters.Aliens.Phantom.Etheric.EthericDoppelganger.Trailing_00"
L3_3086.fEffectScale = 1
L3_3086.bShouldSpawnTrailEntities = 0
L3_3086.fTimeBetweenTrailEntitySpawns = 0.08
L3_3086.fSpawnDistanceBehindProjectile = 1
L2_3085.TrailEffect = L3_3086
L3_3086 = {}
L3_3086.particleeffect_ParticleEffect = "Characters.Aliens.Phantom.Etheric.EthericDoppelganger.Spawn_01"
L3_3086.fEffectScale = 1
L2_3085.SpawnEffect = L3_3086
L3_3086 = {}
L3_3086.particleeffect_ParticleEffect = "Characters.Aliens.Phantom.Etheric.EthericDoppelganger.Dissipate_01"
L3_3086.fEffectScale = 1
L2_3085.DestroyedEffect = L3_3086
L1_3084.Projectile = L2_3085
L0_3083.Properties = L1_3084
ArkEthericDoppelganger = L0_3083