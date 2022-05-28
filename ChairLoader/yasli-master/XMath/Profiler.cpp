#include "stdafx.h"
#include <functional>
#include <algorithm>
#include <list>
#include <math.h>

#include "crtdbg.h"
#include "Profiler.h"
#include "MTSection.h"
#include "ww/PropertyEditor.h"
#include "Macros.h"
#include "yasli/TextOArchive.h"
#include "ww/Decorators.h"
#include "yasli/MemoryWriter.h"
#include "yasli/STLImpl.h"
#include "yasli/PointersImpl.h"

using namespace ww;
using namespace yasli;
using namespace std;

#if !defined(_FINAL_VERSION_) && !defined(_FINAL) && defined(WIN32)

//#pragma warning (disable: 4073) // initializers put in library initialization area
//#pragma init_seg(lib)

// YASLI_CLASS(TimerData, TimerData, "TimerData")
// YASLI_CLASS(StatisticalData, StatisticalData, "StatisticalData")
// YASLI_CLASS(Profiler, Profiler, "Profiler")

struct SerializeAll{
	SerializeAll(Profiler* profiler)
		: profiler_(profiler)
	{}

	void serialize(Archive& ar){
		profiler_->serializeAll(ar);
	}
	Profiler* profiler_;
};

__int64 getRDTSC();
int totalMemoryUsed(); 
int __cdecl allocationTrackingHook( int  nAllocType, void * pvData, size_t nSize, int nBlockUse, long lRequest, const unsigned char * szFileName, int nLine  );

static bool isPressed(int vkKey) 
{
	if(GetAsyncKeyState(vkKey) >> 15)
		return true;
	return false;
}

AllocationData AllocationData::dbgHookData_;

ProfilerMode ProfilerInterface::profilerMode_ = PROFILER_ACCURATE;
__declspec(thread) class Profiler* ProfilerInterface::profiler_;
Profiler* ProfilerInterface::profilerForSerialization_;
MTSection ProfilerInterface::lock_;
bool ProfilerInterface::sortByAvr_ = true;

bool Profiler::started = false;
int Profiler::milliseconds = 0;
__int64 Profiler::counterPrev_;
__int64 Profiler::frequency_;
__int64 Profiler::start_ticks = 0;
__int64 Profiler::ticks = 0;
double Profiler::time_factor = 0;

bool Profiler::autoExit_;
int Profiler::startLogicQuant_;
int Profiler::endLogicQuant_;
string Profiler::title_;
string Profiler::profileFile_;

Profiler::Profilers Profiler::profilers_;

Profiler& ProfilerInterface::profiler() 
{ 
	if(!profiler_) 
		profiler_ = new Profiler();
	return *profiler_;
}

TimerData::TimerData(const char* title) 
{ 
	title_ = title; 
	clear(); 
	profiler().attach(this);
}

void TimerData::start() 
{
	if(profilerMode_){
		profiler().startBuildTree(this);
#ifdef _DEBUG
		if(profilerMode_ == PROFILER_MEMORY)
			last_alloc = AllocationData::dbgHookData_;
#endif
	}
	
	if(!startCounter_)
		t0 = getRDTSC(); 
	startCounter_++;
}

void TimerData::stop() 
{
	if(!--startCounter_){
		__int64 t = getRDTSC();
		__int64 dt = t - t0; 
		dt_sum += dt;

		if(dt_max < dt){
			dt_max = dt;
			t_max = t;
		}
		n++;
	}
	else if(startCounter_ < 0)
		startCounter_ = 0;

	if(profilerMode_){
		profiler().stopBuildTree();
#ifdef _DEBUG
		if(profilerMode_ == PROFILER_MEMORY){
			accumulated_alloc += AllocationData::dbgHookData_;
			accumulated_alloc -= last_alloc;
		}
#endif
	}
}

void TimerData::clear()
{
	t0 = 0;
	dt_sum = 0;
	n = 0;
	dt_max = 0;
	t_max = 0;
	startCounter_ = 0;
	serializing_ = false;
	accumulated_alloc = AllocationData();
	if(profilerMode_)
		children_.clear();
}

struct lessAvr
{																	       
	bool operator()(const TimerData* c1, const TimerData* c2) const
	{
		return c1->dt_sum > c2->dt_sum;
	}
};

struct lessMax
{																	       
	bool operator()(const TimerData* c1, const TimerData* c2) const
	{
		return c1->dt_max > c2->dt_max;
	}
};

struct lessMemory
{																	       
	bool operator()(const TimerData* c1, const TimerData* c2) const
	{
		return c1->accumulated_alloc.size > c2->accumulated_alloc.size;
	}
};


void TimerData::serialize(Archive& ar)
{
	Profiler& profiler = *profilerForSerialization_;
	MemoryWriter buf;
	buf.setDigits(4);
	if(profilerMode_ != PROFILER_MEMORY)
		buf << dt_sum*100./profiler.ticks << " %, " << (n ? (double)dt_sum*profiler.time_factor/n : 0) << " ms, " 
			<< n*1000./profiler.milliseconds << " cps, " << double(n)/profiler.frames << " cpq, max = " << (double)dt_max*profiler.time_factor << " (" << profiler.ticks2time(t_max) << ")";
	else 
		buf << "size = " << accumulated_alloc.size << ", blocks = " << accumulated_alloc.blocks << ", operations = " << accumulated_alloc.operations;

	string str = buf.c_str();
	ar(str, title_, "^!");

	if(!children_.empty()){
		serializing_ = true;
		if(profilerMode_ == PROFILER_MEMORY)
			sort(children_.begin(), children_.end(), lessMemory());
		else if(sortByAvr_)
			sort(children_.begin(), children_.end(), lessAvr());
		else 
			sort(children_.begin(), children_.end(), lessMax());
		Timers::iterator i;
		FOR_EACH(children_, i){
			if((*i)->empty())
				continue;
			if(!(*i)->serializing_)
				ar(**i, (*i)->title_, (*i)->title_);
			else{
				string message = "Recursive";
				ar(message, (*i)->title_, (*i)->title_);
			}
		}
		//ar(HLineDecorator(), "hline", "<");
		serializing_ = false;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//				StatisticalData
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
StatisticalData::StatisticalData(char* title) 
{
	title_ = title; 
	clear();
	profiler().attach(this);
}

void StatisticalData::clear() 
{ 
	n = 0; 
	x_sum = x2_sum = 0; 
	x_max = -1e15; 
	x_min = 1e15; 
	t_max = t_min = 0; 
}

void StatisticalData::serialize(Archive& ar)
{ 
	Profiler& profiler = *profilerForSerialization_;
	MemoryWriter buf;
	buf.setDigits(4);
	buf << avr() << " ± " << (avr() ? sigma()*100/avr() : 0) << " %, max = " << x_max << " (" << profiler.ticks2time(t_max) << "), min = " << x_min << " (" << profiler.ticks2time(t_min) << "), sampling: " << n;
	string str = buf.c_str();
	ar(str, title_, title_);
}

double StatisticalData::sigma() const 
{ 
	double d2 = (x2_sum - x_sum*x_sum/n); 
	return n > 1 && d2 > 0 ? sqrt(d2/((double)n*(n - 1))) : 0; 
}

void StatisticalData::add(double x) 
{
	n++; 
	x_sum += x; 
	if(x_max < x){ 
		x_max = x; 
		t_max = getRDTSC(); 
	} 
	if(x_min > x){ 
		x_min = x; 
		t_min = getRDTSC(); 
	} 
	x2_sum += x*x;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//				Profiler
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
Profiler::Profiler()
{
	MTAutoInternal lock(lock_);

	QueryPerformanceFrequency((LARGE_INTEGER*)&frequency_);

	profileFile_ = "profile";
	name_ = (MemoryWriter() << "Thread" << profilers_.size()).c_str();

	clear();

	profilers_.push_back(this);
}

Profiler::~Profiler()
{
	stack_.clear();
	roots_.clear();

	Timers::iterator i;
	FOR_EACH(timers_, i)
		(*i)->children_.clear();
}

void Profiler::attach(TimerData* timer)
{
	MTAutoInternal lock(lock_);

	timers_.push_back(timer);
}

void Profiler::attach(StatisticalData* stats)
{
	MTAutoInternal lock(lock_);

	statistics_.push_back(stats);
}

void Profiler::clear()
{
	MTAutoInternal lock(lock_);

	stack_.clear();

	if(profilerMode_)
		roots_.clear();

	Timers::iterator i;
	FOR_EACH(timers_, i)
		(*i)->clear();

	Statistics::iterator si;
	FOR_EACH(statistics_, si)
		(*si)->clear();

	frames = 0;
	quantEntered_ = false;
}

void Profiler::start_stop(ProfilerMode profilerMode)
{
	MTAutoInternal lock(lock_);

	if(!started){
		started = 1;
		profilerMode_ = profilerMode;

#ifdef _DEBUG
		if(profilerMode == PROFILER_MEMORY)
			_CrtSetAllocHook( &allocationTrackingHook );			
#endif

		Profilers::iterator i;
		FOR_EACH(profilers_, i)
			(*i)->clear();

		QueryPerformanceCounter((LARGE_INTEGER*)&counterPrev_);
		start_ticks = getRDTSC();
	}
	else{
		__int64 counter;
		QueryPerformanceCounter((LARGE_INTEGER*)&counter);
		milliseconds = (counter - counterPrev_)*1000/frequency_;
		ticks = getRDTSC() - start_ticks;

		if(!endLogicQuant_){
			string stateFileName_ = string(getenv("TEMP")) + "/profiler.tmp";
			if(ww::edit(Serializer(SerializeAll(this)), stateFileName_.c_str())){
				if(!profileFile_.empty()){
					TextOArchive oa;
					serializeAll(oa);
					oa.save(profileFile_.c_str());
				}
			}
		}
		else {
			/*
			XStream ff(0);
			if(ff.open(profileFile_, XS_OUT | XS_APPEND | XS_NOSHARING)){
				const int BUF_CN_SIZE=MAX_COMPUTERNAME_LENGTH + 1;
				DWORD cns = BUF_CN_SIZE;
				char cname[BUF_CN_SIZE];
				GetComputerName(cname, &cns);

				SYSTEMTIME localTime;
				::GetLocalTime(&localTime);

				ff < "\r\n";
				ff < cname < "\t" <= localTime.wYear < "." <= localTime.wMonth < "." <= localTime.wDay < " " <= localTime.wHour < ":" <=  localTime.wMinute < " " <= localTime.wSecond < "\r\n";
				ff < title_.c_str() < "\r\n";

				ff.write(buf.buffer(), buf.tell());
			}
			*/
		}
		started = 0;
		profilerMode_ = PROFILER_ACCURATE;
	}
}

void Profiler::quant(unsigned long curLogicQuant)
{
	if(quantEntered_)
		return;
	quantEntered_ = true;
	frames++;

	if(endLogicQuant_){
		if(!started){
			if(int(curLogicQuant) >= startLogicQuant_)
                start_stop(profilerMode_);
		}
		else if(int(curLogicQuant) >= endLogicQuant_){
			start_stop(profilerMode_);
			//if(autoExit_)
			//	ErrH.Exit();
			startLogicQuant_ = endLogicQuant_ = 0;
			started = 0;
		}
	}

	static bool wasPressed;
	if(isPressed(VK_F6) && (isPressed(VK_CONTROL) || GetFocus())){
		if(!wasPressed){
			wasPressed = true;
			start_stop(isPressed(VK_SHIFT) ? PROFILER_ACCURATE : PROFILER_REBUILD);
		}
	}
	else
		wasPressed = false;

	quantEntered_ = false;
}

void Profiler::setAutoMode(int startLogicQuant, int endLogicQuant, const char* title, const char* profileFile, bool autoExit) 
{
	startLogicQuant_ = startLogicQuant; 
	endLogicQuant_ = endLogicQuant;
	autoExit_ = autoExit;
	title_ = title;
	profileFile_ = profileFile;
}

void Profiler::serializeAll(Archive& ar)
{
	time_factor = (double)milliseconds/ticks;

	string mode;
	switch(profilerMode_){
		case PROFILER_ACCURATE:
			mode = !roots_.empty() ? "Accurate sampling" :
				"Accurate sampling. Tree should be built before!";
			break;
		case PROFILER_REBUILD:
			mode = "Unaccurate sampling, building tree";
			break;
		case PROFILER_MEMORY:
#ifdef _DEBUG
			mode = "Memory profiling";
#else
			mode = "Memory profiling work in DEBUG-mode only!";
#endif
			break;
	}
	ar(mode, "Mode", "!Mode");

	ar(milliseconds, "Time_interval_mS", "!Time interval, mS");
	char total_name[2048];
	string  ticksName = _i64toa(ticks, total_name, 10);
	ar(ticksName, "Ticks", "!Ticks");
	float freq(ticks/(milliseconds*1000.));
	ar(freq, "CPU_MHz", "!CPU, MHz");
	//ar(memory, "Memory start", "Memory start");
	//ar(totalMemoryUsed(), "Memory end", "Memory end");

	//Profilers::iterator end = remove_if(profilers_.begin(), profilers_.end(), mem_fun(&Profiler::empty));
	//profilers_.erase(end, profilers_.end());

	ar(HLineDecorator(), "line1", "<");

	if(profilers_.empty()){
		string noData = "No data";
		ar(noData, "noData", "!<");
	}
	else if(profilers_.size() == 1)
		profilers_.front()->serialize(ar);
	else
		FOR_EACH(profilers_, i, Profilers::iterator)
			ar(**i, (*i)->name_.c_str(), (*i)->name_.c_str());

	ar(HLineDecorator(), "line2", "<");

	ar(sortByAvr_, "sortByAvr", "Sort by average");
	if(ar.isEdit())
		ar(profileFile_, "profileFile", "Save to file");
}

void Profiler::serialize(Archive& ar)
{
	if(!milliseconds)
		milliseconds = 1;
	if(!frames)
		frames = 1;
	if(!ticks)
		ticks = 1;

	profilerForSerialization_ = this;
	if(profilerMode_ != PROFILER_MEMORY){
		ar(frames, "Frames", "!Frames");
		float fps(frames*1000./milliseconds);
		ar(fps, "FPS", "!FPS");
	}

	Timers& list = !roots_.empty() ? roots_ : timers_;
	if(profilerMode_ == PROFILER_MEMORY)
		sort(list.begin(), list.end(), lessMemory());
	else if(sortByAvr_)
		sort(list.begin(), list.end(), lessAvr());
	else
		sort(list.begin(), list.end(), lessMax());

	Timers::iterator i;
	FOR_EACH(list, i)
		ar(**i, (*i)->title_, (*i)->title_);

	if(ar.openBlock("Stat", "!Statistics")){
		Statistics::iterator i;
		FOR_EACH(statistics_, i)
			(*i)->serialize(ar);
		ar.closeBlock();
	}
}

void Profiler::startBuildTree(TimerData* timer)
{
	MTAutoInternal lock(lock_);

	if(!stack_.empty()){
		Timers& timers = stack_.back()->children_;
		if(find(timers.begin(), timers.end(), timer) == timers.end())
			timers.push_back(timer);
	}
	else if(find(roots_.begin(), roots_.end(), timer) == roots_.end())
		roots_.push_back(timer);
	stack_.push_back(timer);
}

void Profiler::stopBuildTree() 
{ 
	if(!stack_.empty())
		stack_.pop_back(); 
	else
		roots_.clear();
}

bool Profiler::empty() const
{
	Timers::const_iterator ti;
	FOR_EACH(timers_, ti)
		if(!(*ti)->empty())
			return false;

	Statistics::const_iterator si;
	FOR_EACH(statistics_, si)
		if(!(*si)->empty())
			return false;

	return true;
}


///////////////////////////////////////////////////////////////////////
//	Memory Hook
///////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#define nNoMansLandSize 4
typedef struct _CrtMemBlockHeader
{
	struct _CrtMemBlockHeader * pBlockHeaderNext;
	struct _CrtMemBlockHeader * pBlockHeaderPrev;
	char *                      szFileName;
	int                         nLine;
	size_t                      nDataSize;
	int                         nBlockUse;
	long                        lRequest;
	unsigned char               gap[nNoMansLandSize];
	/* followed by:
	*  unsigned char           data[nDataSize];
	*  unsigned char           anotherGap[nNoMansLandSize];
	*/
} _CrtMemBlockHeader;

#define pHdr(pbData) (((_CrtMemBlockHeader *)pbData)-1)
int __cdecl allocationTrackingHook(  int  nAllocType,  void   * pvData,  size_t nSize,  int      nBlockUse,  long     lRequest,  const unsigned char * szFileName,  int      nLine  )
{
	AllocationData::dbgHookData_.operations++;
	switch(nAllocType){
		case _HOOK_REALLOC:
			{
				_CrtMemBlockHeader *pHead=pHdr(pvData);
				int dSize=(int)nSize-(int)pHead->nDataSize;
				AllocationData::dbgHookData_.size += dSize;
				break;
			}
		case _HOOK_ALLOC:   
			{
				AllocationData::dbgHookData_.size += int(nSize);
				AllocationData::dbgHookData_.blocks++;
				break;
			}
		case _HOOK_FREE:   
			{
				_CrtMemBlockHeader *pHead = pHdr(pvData);
				nSize = (unsigned int)pHead->nDataSize;
				AllocationData::dbgHookData_.size -= int(nSize);
				AllocationData::dbgHookData_.blocks--;
				break;
			}
		default:{
			_CrtMemBlockHeader *pHead=pHdr(pvData);
			break;
				}

	}

	return 1;         // Allow the memory operation to proceed
}
#endif //_DEBUG

///////////////////////////////////////////////////////////////////////
//	Count by pages
///////////////////////////////////////////////////////////////////////

int totalMemoryUsed()
{
	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);

	int size = 0;
	MEMORY_BASIC_INFORMATION Buffer;
	VirtualQuery(SystemInfo.lpMinimumApplicationAddress, &Buffer,  sizeof(Buffer) );
	while(Buffer.BaseAddress < SystemInfo.lpMaximumApplicationAddress){
		if(Buffer.State == MEM_COMMIT && !(Buffer.Type & MEM_MAPPED) && Buffer.Protect & (PAGE_READWRITE | PAGE_EXECUTE_READ) )
			size += Buffer.RegionSize;
		void* prev_address = Buffer.BaseAddress; 
		VirtualQuery((char*)Buffer.BaseAddress + Buffer.RegionSize, &Buffer,  sizeof(Buffer) );
		if(prev_address == Buffer.BaseAddress)
			break;
	}
	return size;
}

	
#endif