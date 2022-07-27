// Header file automatically created from a PDB.

#pragma once

class ArkPlayerLight;

// Header: FromCpp
// Prey/GameDll/ark/player/ArkPlayerLightManager.h
class ArkPlayerLightManager // Id=8017143 Size=40
{
public:
	std::array<ArkPlayerLight *,4> m_lights;
	unsigned m_suppressCount;
	
	ArkPlayerLightManager();
	void Reset() { FReset(this); }
	void RegisterArkPlayerLight(ArkPlayerLight *_pLight) { FRegisterArkPlayerLight(this,_pLight); }
	void UnregisterArkPlayerLight(ArkPlayerLight *_pLight) { FUnregisterArkPlayerLight(this,_pLight); }
	void PostSerialize() { FPostSerialize(this); }
	bool SetLightVisible(ArkPlayerLight &_light, bool _bVisible) const { return FSetLightVisible(this,_light,_bVisible); }
	void SetSuppressed(bool _bSuppress) { FSetSuppressed(this,_bSuppress); }
	void AttachLightsToPlayer() const { FAttachLightsToPlayer(this); }
	void UpdateLightVisibility() const { FUpdateLightVisibility(this); }
	
#if 0
	bool IsSuppressed() const;
	bool CanLightBeVisible(EArkPlayerLightPriority arg0) const;
	EArkPlayerLightPriority GetVisibleLight() const;
#endif
	
	static inline auto FReset = PreyFunction<void(ArkPlayerLightManager *const _this)>(0x12385F0);
	static inline auto FRegisterArkPlayerLight = PreyFunction<void(ArkPlayerLightManager *const _this, ArkPlayerLight *_pLight)>(0x12385E0);
	static inline auto FUnregisterArkPlayerLight = PreyFunction<void(ArkPlayerLightManager *const _this, ArkPlayerLight *_pLight)>(0x1238670);
	static inline auto FPostSerialize = PreyFunction<void(ArkPlayerLightManager *const _this)>(0x1238500);
	static inline auto FSetLightVisible = PreyFunction<bool(ArkPlayerLightManager const *const _this, ArkPlayerLight &_light, bool _bVisible)>(0x1238620);
	static inline auto FSetSuppressed = PreyFunction<void(ArkPlayerLightManager *const _this, bool _bSuppress)>(0x1238650);
	static inline auto FAttachLightsToPlayer = PreyFunction<void(ArkPlayerLightManager const *const _this)>(0x1238480);
	static inline auto FUpdateLightVisibility = PreyFunction<void(ArkPlayerLightManager const *const _this)>(0x1238690);
};

