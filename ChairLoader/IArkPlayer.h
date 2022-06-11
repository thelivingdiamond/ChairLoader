#pragma once
#include "pch.h"

#include "Prey/CryMath/Cry_Vector3.h"
class IArkPlayer
{
public:
	virtual std::pair<Vec3_tpl<float>, Vec3_tpl<float> >* GetReticleViewPositionAndDir(std::pair<Vec3_tpl<float>, Vec3_tpl<float> >* result) = 0;
	virtual void EnableIdleBreak(const bool) = 0;
	virtual void InhibitRotation(const bool) = 0;
};