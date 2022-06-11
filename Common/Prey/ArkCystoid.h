#include "IArkPlayerCarryListener.h"
#include "IGameObjectExtension.h"

class ArkCystoid : CGameObjectExtensionHelper<ArkCystoid,IGameObjectExtension,64>, IArkPlayerCarryListener, IArkPlayerCombatListener, IArkEtherDuplicateListener
{
public:


  ArkCystoidSignalReceiver m_signalReceiver;
  ArkEntityEffect m_stateVfx;
  ArkEntityEffect m_spawnedByWeaverVfx;
  Vec3_tpl<float> m_flockingStartPoint;
  EArkCystoidState m_state;
  unsigned int m_targetEntityID;
  unsigned int m_swarmLeader;
  bool m_bScripted;
  float m_updateRate;
  float m_movementPeriod;
  float m_movementPeriodPos;
  float m_nullWaveAccumulation;
  bool m_bIsInZeroG;
  bool m_bDamagedByPlayer;
  bool m_bPostSerializeExplicit;
  const ArkCystoidProperties *m_pProperties;
  ArkSimpleTimer m_invalidateTargetTimer;
  ArkSimpleTimer m_explodeTimer;
  ArkSimpleTimer m_energizeTimer;
  ArkAutoResetTimer m_lureTimer;
  ArkSimpleTimer m_targetResetTimer;
  ArkSimpleTimer m_stunnedTimer;
  ArkSimpleTimer m_timeOffGround;
  ArkTimeRemaining m_randomImpulseTimer;
  ArkGenericAttentionObject<ArkCystoid> m_attentionObject;
  int m_explodeInAnimId;
  int m_explodeLoopAnimId;
  std::vector<ArkCystoidPotentialTarget> m_potentialTargets;
};
