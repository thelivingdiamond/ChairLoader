project("xmath")
cmake_minimum_required(VERSION 2.8)

if(WIN32 AND ${CMAKE_SIZEOF_VOID_P} EQUAL 4)
    set(ERRH_SOURCES XErrHand/BSUFunctions.cpp XErrHand/CrashHandler.cpp XErrHand/DiagAssert.cpp XErrHand/GetLoadedModules.cpp XErrHand/IsNT.cpp XErrHand/NT4ProcessInfo.cpp XErrHand/TLHELPProcessInfo.cpp
	XErrHand/XERRHAND.CPP XErrHand/BugslayerUtil.h XErrHand/CrashHandler.h XErrHand/CriticalSection.h XErrHand/DiagAssert.h XErrHand/Internal.h XErrHand/MemDumperValidator.h XErrHand/MSJDBG.h XErrHand/PCH.h
	XErrHand/PSAPI.H XErrHand/SymbolEngine.h XErrHand/WarningsOff.h XErrHand/WarningsOn.h Profiler.cpp XClock.cpp XClock.h)
    source_group("Errh" FILES ${ERRH_SOURCES})
else()
    set(ERRH_SOURCES)
endif()

set(SOURCES xmath.h XMath.cpp Recti.cpp Rectf.cpp Range.cpp Colors.cpp exception.h ExceptionStub.cpp Profiler.h)
source_group("" FILES ${SOURCES})

include_directories(. ..)
if(WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif(WIN32)
if (CMAKE_CXX_COMPILER MATCHES "clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-invalid-source-encoding")
endif()

if (EMSCRIPTEN)
add_library("xmath" ${SOURCES})
else (EMSCRIPTEN)
add_library("xmath" ${SOURCES} ${ERRH_SOURCES})
endif (EMSCRIPTEN)

set_target_properties("xmath" PROPERTIES DEBUG_POSTFIX "-debug")

#include("Testo")
#TESTO("yasli" "Test.cpp")
