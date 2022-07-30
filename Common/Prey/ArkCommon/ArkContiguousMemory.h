// Header file automatically created from a PDB.
#pragma once
#include <vector>

#include "ChairLoader/PreyFunction.h"

// Header: FromCpp
// Prey/ArkCommon/ArkContiguousMemory.h
class ArkContiguousMemory // Id=801346B Size=24
{
public:
	std::vector<unsigned char> m_memory;
	
	ArkContiguousMemory();
	ArkContiguousMemory(uint64_t _reservedByteCount);
	ArkContiguousMemory(ArkContiguousMemory *_other);
	uint64_t GetAllocatedByteCount() const { return FGetAllocatedByteCount(this); }
	void *GetData() { return FGetDataOv1(this); }
	const void *GetData() const { return FGetDataOv0(this); }
	void *AllocateAligned(uint64_t _byteCount, uint64_t _alignment) { return FAllocateAligned(this,_byteCount,_alignment); }
	
#if 0
	bool IsEmpty() const;
	uint64_t GetAvailableByteCount() const;
	uint64_t GetReservedByteCount() const;
	bool CanAllocate(uint64_t arg0) const;
	bool CanAllocateAligned(uint64_t arg0, uint64_t arg1) const;
	void *Allocate(uint64_t arg0);
#endif
	
	static inline auto FGetAllocatedByteCount = PreyFunction<uint64_t(ArkContiguousMemory const *const _this)>(0x1056380);
	static inline auto FGetDataOv1 = PreyFunction<void *(ArkContiguousMemory *const _this)>(0x12E5E70);
	static inline auto FGetDataOv0 = PreyFunction<const void *(ArkContiguousMemory const *const _this)>(0x12E5E70);
	static inline auto FAllocateAligned = PreyFunction<void *(ArkContiguousMemory *const _this, uint64_t _byteCount, uint64_t _alignment)>(0x1056320);
};

