#pragma once
#include "StdAfx.h"
#include <stdio.h>
#include <tchar.h>
#include <vector>
#include <windows.h>
#include <TlHelp32.h>


DWORD GetProcId(const wchar_t* procName);

uintptr_t GetModuleBaseAddress(DWORD procID, const wchar_t* modName);

