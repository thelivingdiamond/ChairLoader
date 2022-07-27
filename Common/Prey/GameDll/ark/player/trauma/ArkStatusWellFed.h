// Header file automatically created from a PDB.

#pragma once
#include <Prey/GameDll/ark/player/trauma/ArkTraumaBase.h>

// Header: FromCpp
// Prey/GameDll/ark/player/trauma/ArkStatusWellFed.h
class ArkStatusWellFed : public ArkTraumaBase // Id=801B883 Size=296
{
public:
	ArkStatusWellFed();
	virtual void Update(float _frametime);
	virtual bool IsHarmful() const;
	virtual void RepeatEffect();
	virtual ~ArkStatusWellFed();
	
	static inline auto FUpdate = PreyFunction<void(ArkStatusWellFed *const _this, float _frametime)>(0x1327F60);
	static inline auto FIsHarmful = PreyFunction<bool(ArkStatusWellFed const *const _this)>(0xDD23F0);
	static inline auto FRepeatEffect = PreyFunction<void(ArkStatusWellFed *const _this)>(0x1327F20);
};

