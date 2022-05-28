#include "StdAfx.h"
#include "proc.h"
#include <iostream>

DWORD GetProcId(const wchar_t* procName) {
	// set to 0 for error checking
	DWORD procId = 0;
	// create a snapshot of all processes running at the moment
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	// check to see if the handle is valid
	if (hSnap != INVALID_HANDLE_VALUE) {
		// a process structure
		PROCESSENTRY32 procEntry;
		// set the double word size
		procEntry.dwSize = sizeof(procEntry);
		// grabs the first process from the snapshot
		if(Process32First(hSnap,&procEntry)) {
			// loop through all processes in the snapshot
			do {
				// std::cout << procEntry.szExeFile << "\n";
				// comparing process name to the input value 
				if(!_wcsicmp(procEntry.szExeFile, procName)) {
					// if you have the process you want then bob's your uncle we're done here
					procId = procEntry.th32ProcessID;
					break;
				}
				// if we're at the end of the process list then like fuck bro I guess it's not running
			} while (Process32Next(hSnap, &procEntry));
		}
	}
	// destruct our snapshot variable
	CloseHandle(hSnap);
	// return that dank process ID
	return procId;
}

uintptr_t GetModuleBaseAddress(DWORD procID, const wchar_t* modName) {

	uintptr_t modBaseAddress = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE| TH32CS_SNAPMODULE32, procID);
	if (hSnap != INVALID_HANDLE_VALUE) {
		MODULEENTRY32 modEntry;
		modEntry.dwSize = sizeof(modEntry);
		if(Module32First(hSnap, &modEntry)) {
			// loop through all modules in the snapshot
			do {
				std::cout << modEntry.szModule << "\n";
				// comparing module name to the input value 
				if (!_wcsicmp(modEntry.szModule, modName)) {
					// if you have the module you want then go home fool you done
					modBaseAddress = (uintptr_t)modEntry.modBaseAddr;
					break;
					
				}
				// no more modules, try again next time
			} while (Module32Next(hSnap, &modEntry));
		}
	}
	CloseHandle(hSnap);
	return modBaseAddress;
}


