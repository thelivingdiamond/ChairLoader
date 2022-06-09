// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <string>
#include "mem.h"
#include "PreyFunctions.h"
#include "proc.h"
#include "preyDllObjects.h"
#include "PreyFunctions.h"
#include "ChairloaderUtils.h"
#include "ChairloaderGui.h"
#include "ChairLoader.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH: {
        gCL = new ChairLoader();
        break;
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
