#pragma once
#include "Prey/CryCore/StdAfx.h"

enum class EStance : int32_t {
	STANCE_UNLOCKED = -2,
	STANCE_NULL = -1,
	STANCE_STAND = 0,
	STANCE_SNEAK = 1,
	STANCE_CRAWL = 2,
	STANCE_SMOKE = 3,
	STANCE_PRONE = 4,
	STANCE_RELAXED = 5,
	STANCE_STEALTH = 6,
	STANCE_ALERTED = 7,
	STANCE_SWIM = 8,
	STANCE_ZEROG = 9,
	STANCE_HOVER = 10,
	STANCE_CORRUPTED = 11,
	STANCE_INVESTIGATE_ANOMALY = 12,
	STANCE_INVESTIGATE_DANGER = 13,
	STANCE_INVESTIGATE_VISION = 14,
	STANCE_HUNT = 15,
	STANCE_GUNUP = 16,
	STANCE_CHARGE = 17,
	STANCE_LAST = 18
};