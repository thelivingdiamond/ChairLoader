#pragma once

// A hack to fix inclusion order when compiling Win32/Debug
struct _PSP;
typedef struct _PSP * HPROPSHEETPAGE;
#ifndef DECLSPEC_IMPORT
# define DECLSPEC_IMPORT __declspec(dllimport)
#endif
#ifndef WINAPI
# define WINAPI __stdcall
#endif

#include <commctrl.h>
#pragma comment (lib, "comctl32.lib")
