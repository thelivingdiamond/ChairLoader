// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

#include "SDLMixerSoundEngine.h"
#include <CryCore/CryCrc32.h>
#include <CrySystem/File/CryFile.h>
#include <CryString/CryPath.h>

namespace SDLMixer
{
inline const TSampleID GetIDFromString(const string& sName)
{
	return CCrc32::ComputeLowercase(sName.c_str());
}

inline const TSampleID GetIDFromFilePath(const string& sFilePath)
{
	return GetIDFromString(PathUtil::GetFile(sFilePath));
}

inline void GetDistanceAngleToObject(const CAudioObjectTransformation& listener, const CAudioObjectTransformation& object, float& out_nDistance, float& out_nAngle)
{
	const Vec3 listenerToObject = object.GetPosition() - listener.GetPosition();

	// Distance
	out_nDistance = listenerToObject.len();

	// Angle
	// Project point to plane formed by the listeners position/direction
	Vec3 n = listener.GetUp().GetNormalized();
	Vec3 objectDir = Vec3::CreateProjection(listenerToObject, n).normalized();

	// Get angle between listener position and projected point
	const Vec3 listenerDir = listener.GetForward().GetNormalizedFast();
	out_nAngle = RAD2DEG(asin(objectDir.Cross(listenerDir).Dot(n)));
}
}
