#include "StdAfx.h"
#include "mem.h"
// Apply a patch to a region of memory
void mem::PatchEx(BYTE* dst, BYTE* src, unsigned int size, HANDLE hProcess) {
	// the old page permissions
	DWORD oldprotect;
	// give us virtual memory page memory access
	VirtualProtectEx(hProcess, dst, size, PAGE_EXECUTE_READWRITE, &oldprotect);
	// write the memory
	WriteProcessMemory(hProcess, dst, src, size, nullptr);
	// restore previous memory conditions
	VirtualProtectEx(hProcess, dst, size, oldprotect, &oldprotect);
}
// Nop a series of bytes
void mem::NopEx(BYTE* dst, unsigned size, HANDLE hProcess) {
	// create an array of bytes
	BYTE* nopArray = new BYTE[size];
	// set each byte to nop
	memset(nopArray, 0x90, size);
	// patch the nop into memory
	PatchEx(dst, nopArray, size, hProcess);
	// destruct
	delete[] nopArray;
	
}
uintptr_t mem::FindDMAAddy(HANDLE hProc, uintptr_t ptr, std::vector<unsigned int> offsets) {
	uintptr_t addr = ptr;
	for (unsigned int i = 0; i < offsets.size(); ++i) {
		ReadProcessMemory(hProc, (BYTE*)addr, &addr, sizeof(addr), 0);
		addr += offsets[i];
	}
	return addr;
}

void mem::Patch(BYTE* dst, BYTE* src, unsigned size) {
	// the old page permissions
	DWORD oldprotect;
	// give us virtual memory page memory access
	VirtualProtect( dst, size, PAGE_EXECUTE_READWRITE, &oldprotect);
	// write the memory
	memcpy(dst, src, size);
	// restore previous memory conditions
	VirtualProtect( dst, size, oldprotect, &oldprotect);
}
void mem::Nop(BYTE* dst, unsigned size) {
	// the old page permissions
	DWORD oldprotect;
	// give us virtual memory page memory access
	VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldprotect);
	// write the memory
	memset(dst, 0x90, size);
	// restore previous memory conditions
	VirtualProtect(dst, size, oldprotect, &oldprotect);
}

uintptr_t mem::FindDMAAddy(uintptr_t ptr, std::vector<unsigned> offsets) {
	uintptr_t addr = ptr;
	for (unsigned int i = 0; i < offsets.size(); ++i) {
		addr = *(uintptr_t*)addr;
		addr += offsets[i];
	}
	return addr;
}
