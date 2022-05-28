// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

#include <CryCore/Project/CryModuleDefs.h>
#define eCryModule eCryM_AudioImplementation
#include <CryCore/Platform/platform.h>
#include <CryCore/StlUtils.h>
#include <CryCore/Project/ProjectDefines.h>
#include <SoundAllocator.h>

#if !defined(_RELEASE)
// Define this to enable logging via CAudioLogger.
// We disable logging for Release builds
	#define ENABLE_AUDIO_LOGGING
#endif // _RELEASE

#include <AudioLogger.h>

extern CSoundAllocator g_audioImplMemoryPool_sdlmixer;
extern CAudioLogger g_audioImplLogger_sdlmixer;

#define AUDIO_ALLOCATOR_MEMORY_POOL g_audioImplMemoryPool_sdlmixer
#include <STLSoundAllocator.h>

#if !defined(_RELEASE)
	#define INCLUDE_SDLMIXER_IMPL_PRODUCTION_CODE
#endif

// Windows (32 or 64)
//////////////////////////////////////////////////////////////////////////
#if CRY_PLATFORM_WINDOWS
#endif

// Windows32
//////////////////////////////////////////////////////////////////////////
#if CRY_PLATFORM_WINDOWS && CRY_PLATFORM_32BIT
#endif

// Windows64
//////////////////////////////////////////////////////////////////////////
#if CRY_PLATFORM_WINDOWS && CRY_PLATFORM_64BIT
#endif

// Mac
//////////////////////////////////////////////////////////////////////////
#if CRY_PLATFORM_MAC
#endif

// Android
//////////////////////////////////////////////////////////////////////////
#if CRY_PLATFORM_ANDROID
#endif

// iOS
//////////////////////////////////////////////////////////////////////////
#if CRY_PLATFORM_IOS
#endif

// Linux
//////////////////////////////////////////////////////////////////////////
#if CRY_PLATFORM_LINUX
#endif
