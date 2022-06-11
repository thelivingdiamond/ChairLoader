#pragma once
#include <cstdint>
#include <ctime>
#include <Prey/CryNetwork/ISerialize.h>
#include "Prey/ArkNpc.h"
class CTimeValue {
public:
	uint64_t m_lValue;
};

class ITimer {
public:
	enum ETimer : int8_t
	{
		ETIMER_REAL = 0x0,
		ETIMER_GAME = 0x1,
		ETIMER_UI = 0x2,
		ETIMER_PLAYER = 0x3,ETIMER_LAST = 0x4,
	};

	enum ETimeDilationChannel
	{
		GENERIC = 0x0,
		DEV = 0x1,
		TRACKVIEW = 0x2,
		LAST = 0x3,
	};

	virtual ~ITimer() = 0;
	virtual void ResetTimer() = 0;
	virtual void UpdateOnFrameStart() = 0;
	virtual float GetCurrTime(ITimer::ETimer) = 0;
	virtual const CTimeValue *GetFrameStartTime(ITimer::ETimer) = 0;
	virtual CTimeValue *GetAsyncTime(CTimeValue *result) = 0;
	virtual float GetAsyncCurTime() = 0;
	virtual float GetFrameTime(ITimer::ETimer) = 0;
	virtual float GetRealFrameTime() = 0;
	virtual float GetTimeScale(ITimer::ETimer) = 0;
	virtual void SetTimeScale(ITimer::ETimer, float, ITimer::ETimeDilationChannel) = 0;
	virtual void EnableTimer(const bool) = 0;
	virtual bool IsTimerEnabled() = 0;
	virtual float GetFrameRate() = 0;
	virtual float GetProfileFrameBlending(float *, int *) = 0;
	virtual void Serialize(CSerializeWrapper<ISerialize>) = 0;
	virtual bool PauseTimer(ITimer::ETimer, bool) = 0;
	virtual bool IsTimerPaused(ITimer::ETimer) = 0;
	virtual bool SetTimer(ITimer::ETimer, float) = 0;
	virtual void SecondsToDateUTC(int64_t, tm *) = 0;
	virtual int64_t DateToSecondsUTC(tm *) = 0;
	virtual float TicksToSeconds(int64_t) = 0;
	virtual int64_t GetTicksPerSecond() = 0;
	virtual ITimer *CreateNewTimer() = 0;
};
