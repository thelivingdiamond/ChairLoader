#pragma once

enum ProfilerMode
{
	PROFILER_ACCURATE,
	PROFILER_REBUILD,
	PROFILER_MEMORY
};

#if !defined(_FINAL_VERSION_) && !defined(_FINAL) && defined(WIN32) && !defined(_WIN64)

#include <vector>
#include <string>
#include "XMath/round.h"
#include "yasli/Pointers.h"

#pragma warning(disable : 4512) // assignment operator could not be generated

namespace yasli { class Archive; }
class Profiler;

struct AllocationData
{
	int size;
	int blocks;
	int operations;

	AllocationData() { size = blocks = operations = 0; }
	AllocationData& operator += (const AllocationData& data) { size += data.size; blocks += data.blocks; operations += data.operations; return *this; }
	AllocationData& operator -= (const AllocationData& data) { size -= data.size; blocks -= data.blocks; operations -= data.operations; return *this; }

	static AllocationData dbgHookData_;
};

class ProfilerInterface : public yasli::PolyRefCounter
{
public:
	static Profiler& profiler();

protected:
	typedef std::vector<yasli::SharedPtr<struct TimerData> > Timers;

	static ProfilerMode profilerMode_;
	static __declspec(thread) class Profiler* profiler_;
	static Profiler* profilerForSerialization_;
	static class MTSection lock_;
	static bool sortByAvr_;
};

struct TimerData : public ProfilerInterface
{
	TimerData(const char* title = 0);

	void start();
	void stop();

	void clear();
	void serialize(yasli::Archive& ar);

	bool empty() const { return !n; }

	__int64 t0;
	__int64 dt_sum;
	int n;
	__int64 dt_max, t_max;

	const char* title_;
	int startCounter_;
	bool serializing_;
	
	Timers children_;

	AllocationData accumulated_alloc;
	AllocationData last_alloc;
};

class StatisticalData : public ProfilerInterface
{
	double x_sum, x2_sum, x_max, x_min;
	__int64 t_max, t_min;
	const char* title_;
	int n;

public:
	StatisticalData(char* title = 0);
	void clear();
	void add(double x);
	double avr() const { return n ? x_sum/n : 0; }
	double sigma() const;
	void serialize(yasli::Archive& ar);
	bool empty() const { return !n; }
};

class Profiler : public ProfilerInterface
{
public:
	Profiler();
	~Profiler();

	void attach(TimerData* timer);
	void attach(StatisticalData* timer);
	void clear();

	void startBuildTree(TimerData* timer);
	void stopBuildTree();

	void setAutoMode(int startLogicQuant, int endLogicQuant, const char* title = "", const char* profileFile = "profile", bool autoExit = false);
	void start_stop(ProfilerMode profilerMode);
	void quant(unsigned long curLogicQuant);
	void serialize(yasli::Archive& ar);

	void serializeAll(yasli::Archive& ar);

	int ticks2time(__int64 t) { return t ? xround((t - start_ticks)*time_factor) : 0; }

	static Profiler& instance() { return TimerData::profiler(); }

	bool empty() const;

private:
	Timers timers_;
	Timers stack_;
	Timers roots_;

	typedef std::vector<yasli::SharedPtr<StatisticalData> > Statistics;
	Statistics statistics_;

	int frames;
	bool quantEntered_;
	std::string name_;

	static bool started;
	static int milliseconds;
	static __int64 counterPrev_;
	static __int64 frequency_;
	static __int64 start_ticks;
	static __int64 ticks;
	static double time_factor;

	static bool autoExit_;
	static int startLogicQuant_;
	static int endLogicQuant_;
	static std::string title_;
	static std::string profileFile_;

	typedef std::vector<yasli::SharedPtr<Profiler> > Profilers;
	static Profilers profilers_;

	friend TimerData;
	friend StatisticalData;
};

class AutoStopTimer
{
	TimerData& timer;
public:
	AutoStopTimer(TimerData& timer_) : timer(timer_) {}
	~AutoStopTimer() { timer.stop(); }
};
	
#define start_timer(title) static __declspec(thread) TimerData* __timer_##title; if(!__timer_##title) __timer_##title = new TimerData(__FUNCTION__" "#title); __timer_##title->start(); 
#define stop_timer(title) __timer_##title->stop();
#define start_timer_auto() static __declspec(thread) TimerData* __timer_; if(!__timer_) __timer_ = new TimerData(__FUNCTION__); __timer_->start(); AutoStopTimer autostop_timer_(*__timer_); 
#define start_timer_auto1(title) static __declspec(thread) TimerData* __timer_##title; if(!__timer_##title) __timer_##title = new TimerData(__FUNCTION__" "#title); __timer_##title->start(); AutoStopTimer autostop_timer_##title(*__timer_##title); 
#define statistics_add(title, x) { static __declspec(thread) StatisticalData* stat_##title; if(!stat_##title) stat_##title = new StatisticalData(#title); stat_##title->add(x); }

inline void profiler_start_stop(ProfilerMode mode = PROFILER_REBUILD) { Profiler::instance().start_stop(mode); }
inline void profiler_quant(int curLogicQuant = 0) { Profiler::instance().quant(curLogicQuant); }

#else //_FINAL_VERSION_

#define start_timer(title) 
#define stop_timer(title) 
#define start_timer_auto() 
#define start_timer_auto1(title) 
#define statistics_add(title, x) 

#pragma warning (push)
#pragma warning(disable: 4100)
inline void profiler_start_stop(ProfilerMode mode = PROFILER_REBUILD) {}
inline void profiler_quant(unsigned long curLogicQuant = 0){}
#pragma warning (pop)

#endif //_FINAL_VERSION_



