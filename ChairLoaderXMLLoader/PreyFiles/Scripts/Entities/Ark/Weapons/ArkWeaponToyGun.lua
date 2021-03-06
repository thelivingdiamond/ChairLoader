Script.ReloadScript("Scripts/Entities/Ark/Pickups/ArkItem.lua")
ArkWeaponToyGun = {
  Properties = {
    object_GeometryFP = "Objects/weapons/Shotgun/1P/Shotgun1P.cdf",
    object_GeometryTP = "Objects/weapons/Shotgun/3P/shotgun3p_01.cgf",
    bEquippable = 1,
    fLerpSpeed = 10,
    Description = {textFocusDescription = ""},
    Weapon = {
      playerImpulse = 0,
      playerImpulseZeroG = 0,
      fAmmoLootImpulse = 0.8,
      recoilPitchMin = 0,
      recoilPitchMax = 0,
      recoilYawMin = 0,
      recoilYawMax = 0,
      recoilDuration = 0,
      recoilDelay = 0,
      anim_Animation = "Animations/Mannequin/ADB/ArkWeaponShotgun1P.adb",
      sAnimTag = "shotgun",
      sAttachmentName = "weapon",
      archetype_ammo = "ArkProjectiles.Bullets.ToygunDart_Default",
      sAmmoSpawnPointName = "muzzle",
      fAimRange = 10,
      fBaseAmmoCost = 1,
      nClipSize = 3,
      fReloadSpeed = 1,
      fRateOfAttack = 1,
      bAutoloadAmmo = 1,
      nNumAmmoReloaded = 1,
      nInitialAmmoMin = 0,
      nInitialAmmoMax = 10,
      fSpawnFromCameraTestDistance = 1.5,
      fSpawnBehindCameraDistance = 0.3,
      fCriticalShotChance = 0,
      bAffectedByDisruption = 0,
      fCombatFocusAmount = 0.5,
      fWalkSpeedMultiplier = 1,
      fCameraSpeedMultiplier = 1,
      fZoomedHFOV = 15,
      fZoomTime = 0,
      fZoomedWalkSpeedMultiplier = 0.25,
      fZoomedCameraSpeedMultiplier = 0.25,
      bUsesCrosshairReticle = 0,
      sUIElementName = "DanielleWeaponUI",
      nWeaponUIType = 1,
      fIdleBreakDelayTime = 15,
      fIronsightsTargetWidth = 0,
      fIronsightsTargetDistance = 0,
      fIronsightsTargetSpeed = 0,
      fIronsightsSuccessDistance = 0,
      fIronsightsMaxAngularVelocity = 0,
      fAimAssistRangeMax = 100,
      sLeverageImpulseScaleBaseName = "Leverage Base",
      sLeverageImpulseScale1Name = "Leverage I",
      sLeverageImpulseScale2Name = "Leverage II",
      sLeverageImpulseScale3Name = "Leverage III",
      fLeverage0_ImpulseScale = 1,
      fLeverage1_ImpulseScale = 0,
      fLeverage2_ImpulseScale = 0,
      fLeverage3_ImpulseScale = 0,
      Degradation = {
        fDegradationRate = 1,
        fMalfunctionThreshold = 20,
        fMalfunctionRate = 12.5,
        sBrokenIcon = "",
        sBrokenHUDIcon = "",
        audioTrigger_StartMalfunction = "",
        audioTrigger_StopMalfunction = "",
        audioTrigger_Break = "",
        fHazardIconTime = 1,
        bTakeHazardDamage = 1,
        fMinStartCondition = 20,
        fMaxStartCondition = 50
      },
      tutorial_firstTimePickup = "",
      tutorial_weaponCard = "",
      Light = {
        sLightAttachmentName = "",
        archetype_LightArchetype = "",
        vector_DefaultLightColor = {
          x = 1,
          y = 1,
          z = 1
        },
        vector_DamagedLightColor = {
          x = 1,
          y = 1,
          z = 0
        },
        vector_BrokenLightColor = {
          x = 1,
          y = 0,
          z = 0
        }
      }
    },
    Shotgun = {
      bShowAmmoOnHUD = 1,
      nNumberOfPelletRows = 1,
      nNumberOfPelletColumns = 1,
      fSpreadConeDegrees = 0,
      fAccurateShotChance = 0,
      fZeroGravityTime = 0.25,
      Dispersion = {
        fMinimum = 1,
        fMaximum = 3,
        fMinimumIronsights = 0.05,
        fMaximumIronsights = 2,
        fMinimumCrawling = 0.06,
        fMaximumCrawling = 2.25,
        fMinimumSneaking = 0.07,
        fMaximumSneaking = 2.75,
        fAmount_Recoil = 0.02,
        fRate_IronSights = -0.01,
        fAmount_Jumping = 0.01,
        fAmount_Sliding = 0.01,
        fRate_Turning = 0.002,
        fRate_NotTurning = -0.005,
        fRate_Moving = 0.002,
        fRate_NotMoving = -0.005,
        fRate_Sprinting = 0.005,
        fRate_Standing = 0,
        fRate_Crawling = 0.003,
        fRate_Sneaking = 0.007,
        fRate_ZeroG = 0.001
      },
      PointBlank = {
        fRange = 0,
        fRadius = 0,
        archetype_ammo = "",
        fHitDistance = 0,
        fImpulse = 0,
        maxImpulseMassScale = 0
      },
      DebugTesting = {fDebugDrawTime = 20, nNumTestPoints = 1000}
    }
  },
  PropertiesInstance = {iInitialAmmo = 10, fStartingConditionOverride = -1}
}
ArkMakeDerivedEntityOverride(ArkWeaponToyGun, ArkItem, true)
