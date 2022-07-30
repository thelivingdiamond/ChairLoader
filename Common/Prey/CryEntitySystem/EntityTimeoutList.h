// Header file automatically created from a PDB.

#pragma once
#include <Prey/CryEntitySystem/EntityTimeoutList.h>

struct ITimer;

// Header: FromCpp
// CryEngine/cryentitysystem/entitytimeoutlist.h
class CEntityTimeoutList // Id=800C735 Size=16
{
public:
	class CEntry // Id=800C737 Size=16
	{
	public:
		unsigned m_id;
		float m_time;
		int m_next;
		int m_prev;
	};

	using EntryContainer = DynArray<CEntityTimeoutList::CEntry,int,NArray::SmallDynStorage<NAlloc::AllocCompatible<NAlloc::ModuleAlloc> > >;
	
	DynArray<CEntityTimeoutList::CEntry,int,NArray::SmallDynStorage<NAlloc::AllocCompatible<NAlloc::ModuleAlloc> > > m_entries;
	ITimer *m_pTimer;
	
	CEntityTimeoutList(ITimer *pTimer);
	void ResetTimeout(unsigned id) { FResetTimeout(this,id); }
	unsigned PopTimeoutEntity(float timeout) { return FPopTimeoutEntity(this,timeout); }
	void Clear() { FClear(this); }
	
#if 0
	static CSaltHandle<unsigned short,unsigned short> IdToHandle(const unsigned arg0);
#endif
	
	static inline auto FResetTimeout = PreyFunction<void(CEntityTimeoutList *const _this, unsigned id)>(0x929D00);
	static inline auto FPopTimeoutEntity = PreyFunction<unsigned(CEntityTimeoutList *const _this, float timeout)>(0x929C40);
	static inline auto FClear = PreyFunction<void(CEntityTimeoutList *const _this)>(0x929C20);
};

