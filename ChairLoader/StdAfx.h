// StdAfx.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include "framework.h"
#include <cstdio>
#include <iostream>
#include <tchar.h>
#include <vector>
// #include <Windows.h>
#include <TlHelp32.h>
#include <vector>
#include <optional>
#include <unordered_map>
#include <map>
#include <DirectXMath.h>
#include <array>
#include <variant>
#include <functional>
#include <memory>
#include <boost/variant.hpp>
#include <deque>
#include "pugixml.hpp"
#include <fstream>
#include <mutex>
#include <queue>
#include <thread>
#include <stack>
#include <algorithm>
#include <string>

#include <d3d11.h>
#include <D3Dcompiler.h>
#pragma comment(lib, "D3dcompiler.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "winmm.lib")


#include <detours/detours.h>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <imgui_stdlib.h>

#include <boost/variant.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>

#endif //PCH_H
