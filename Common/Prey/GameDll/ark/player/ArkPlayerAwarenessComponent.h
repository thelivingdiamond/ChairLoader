// Header file automatically created from a PDB.

#pragma once
#include <Prey/CryNetwork/ISerialize.h>
#include <Prey/GameDll/ark/VectorMap.h>

class XmlNodeRef;

// Header: FromCpp
// Prey/GameDll/ark/player/ArkPlayerAwarenessComponent.h
class ArkPlayerAwarenessComponent // Id=8017062 Size=64
{
public:
	class AwarenessState // Id=8017063 Size=24
	{
	public:
		EArkAttentionLevel m_level;
		bool m_bHidden;
		bool m_bHasLOS;
		bool m_bProxyInRange;
		ArkTimeRemaining m_fullAwarenessTimer;
		ArkTimeRemaining m_timeSinceLOSCheck;
		ArkTimeRemaining m_obscuredTimer;
		ArkTimeRemaining m_shownTimer;
	};

	VectorMap<unsigned int,ArkPlayerAwarenessComponent::AwarenessState,std::less<unsigned int>,std::allocator<std::pair<unsigned int,ArkPlayerAwarenessComponent::AwarenessState> > > m_awarenessStates;
	std::vector<unsigned int> m_blockedSneakAttackTargets;
	float m_fullyAwareDuration;
	float m_minTimeToBeShown;
	float m_obscuredButShownDuration;
	float m_proxyDistanceSq;
	
	ArkPlayerAwarenessComponent();
	void Reset() { FReset(this); }
	void LoadConfig(XmlNodeRef const &_node) { FLoadConfig(this,_node); }
	void Update(const float _dt) { FUpdate(this,_dt); }
	void PreSerialize() { FPreSerialize(this); }
	void Serialize(TSerialize _ser) { FSerialize(this,_ser); }
	bool CanSneakAttackTarget(const unsigned _target) const { return FCanSneakAttackTarget(this,_target); }
	void PerformedSneakAttackOnTarget(const unsigned _target) { FPerformedSneakAttackOnTarget(this,_target); }
	
#if 0
	void AwarenessUpdated(const unsigned arg0, EArkAttentionLevel arg1, EArkAttentionLevel arg2);
	EArkAttentionLevel GetAwarenessLevel(const unsigned arg0) const;
#endif
	
	static inline auto FReset = PreyFunction<void(ArkPlayerAwarenessComponent *const _this)>(0x122CDD0);
	static inline auto FLoadConfig = PreyFunction<void(ArkPlayerAwarenessComponent *const _this, XmlNodeRef const &_node)>(0x122CC00);
	static inline auto FUpdate = PreyFunction<void(ArkPlayerAwarenessComponent *const _this, const float _dt)>(0x122CFB0);
	static inline auto FPreSerialize = PreyFunction<void(ArkPlayerAwarenessComponent *const _this)>(0x122CDD0);
	static inline auto FSerialize = PreyFunction<void(ArkPlayerAwarenessComponent *const _this, TSerialize _ser)>(0x122CE50);
	static inline auto FCanSneakAttackTarget = PreyFunction<bool(ArkPlayerAwarenessComponent const *const _this, const unsigned _target)>(0x122CB50);
	static inline auto FPerformedSneakAttackOnTarget = PreyFunction<void(ArkPlayerAwarenessComponent *const _this, const unsigned _target)>(0x122CD90);
};

