// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#include "stdafx.h"
#include "AudioImplCVars_wwise.h"
#include "AudioImpl_wwise.h"
#include <CryAudio/IAudioSystem.h>
#include <CryCore/Platform/platform_impl.inl>
#include <CrySystem/IEngineModule.h>
#include <CryExtension/ClassWeaver.h>

#if CRY_PLATFORM_DURANGO
	#include <apu.h>
	#include <shapexmacontext.h>
#endif // CRY_PLATFORM_DURANGO

using namespace CryAudio::Impl;

// Define global objects.
namespace CryAudio
{
namespace Impl
{
CAudioImplCVars_wwise g_AudioImplCVars_wwise;
}
}
CSoundAllocator g_AudioImplMemoryPool_wwise;
CAudioLogger g_AudioImplLogger_wwise;

#if defined(PROVIDE_WWISE_IMPL_SECONDARY_POOL)
tMemoryPoolReferenced g_AudioImplMemoryPoolSecondary_wwise;
#endif // PROVIDE_AUDIO_IMPL_SECONDARY_POOL

//////////////////////////////////////////////////////////////////////////
class CEngineModule_CryAudioImplWwise : public IEngineModule
{
	CRYINTERFACE_SIMPLE(IEngineModule)
	CRYGENERATE_SINGLETONCLASS(CEngineModule_CryAudioImplWwise, "CryAudioImplWwise", 0xb4971e5dd02442c5, 0xb34a9ac0b4abfffd)

	//////////////////////////////////////////////////////////////////////////
	virtual const char* GetName() override { return "CryAudioImplWwise"; }
	virtual const char* GetCategory() override { return "CryAudio"; }

	//////////////////////////////////////////////////////////////////////////
	virtual bool Initialize(SSystemGlobalEnvironment& env, const SSystemInitParams& initParams) override
	{
		// initialize memory pools
		MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "Wwise Audio Implementation Memory Pool Primary");
		size_t const poolSize = g_AudioImplCVars_wwise.m_nPrimaryMemoryPoolSize << 10;
		uint8* const pPoolMemory = new uint8[poolSize];
		g_AudioImplMemoryPool_wwise.InitMem(poolSize, pPoolMemory, "Wwise Implementation Audio Pool");

#if defined(PROVIDE_WWISE_IMPL_SECONDARY_POOL)
		size_t secondarySize = 0;
		void* pSecondaryMemory = nullptr;

	#if CRY_PLATFORM_DURANGO
		MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "Wwise Implementation Audio Pool Secondary");
		secondarySize = g_AudioImplCVars_wwise.m_nSecondaryMemoryPoolSize << 10;

		APU_ADDRESS temp;
		HRESULT const result = ApuAlloc(&pSecondaryMemory, &temp, secondarySize, SHAPE_XMA_INPUT_BUFFER_ALIGNMENT);
		CRY_ASSERT(result == S_OK);
	#endif // CRY_PLATFORM_DURANGO

		g_AudioImplMemoryPoolSecondary_wwise.InitMem(secondarySize, (uint8*)pSecondaryMemory);
#endif // PROVIDE_AUDIO_IMPL_SECONDARY_POOL

		POOL_NEW_CREATE(CAudioImpl_wwise, pImpl);

		if (pImpl != nullptr)
		{
			g_AudioImplLogger_wwise.Log(eAudioLogType_Always, "CryAudioImplWwise loaded");

			SAudioRequest request;
			request.flags = eAudioRequestFlags_PriorityHigh | eAudioRequestFlags_ExecuteBlocking;

			SAudioManagerRequestData<eAudioManagerRequestType_SetAudioImpl> requestData(pImpl);
			request.pData = &requestData;

			gEnv->pAudioSystem->AddRequestListener(&CEngineModule_CryAudioImplWwise::OnAudioEvent, nullptr, eAudioRequestType_AudioManagerRequest, eAudioManagerRequestType_SetAudioImpl);
			env.pAudioSystem->PushRequest(request);
			gEnv->pAudioSystem->RemoveRequestListener(&CEngineModule_CryAudioImplWwise::OnAudioEvent, nullptr);
		}
		else
		{
			g_AudioImplLogger_wwise.Log(eAudioLogType_Always, "CryAudioImplWwise failed to load");
		}

		return m_bSuccess;
	}

	//////////////////////////////////////////////////////////////////////////
	static void OnAudioEvent(SAudioRequestInfo const* const pAudioRequestInfo)
	{
		m_bSuccess = pAudioRequestInfo->requestResult == eAudioRequestResult_Success;
	}

	static bool m_bSuccess;
};

CRYREGISTER_SINGLETON_CLASS(CEngineModule_CryAudioImplWwise)
bool CEngineModule_CryAudioImplWwise::m_bSuccess = false;

CEngineModule_CryAudioImplWwise::CEngineModule_CryAudioImplWwise()
{
	g_AudioImplCVars_wwise.RegisterVariables();
}

CEngineModule_CryAudioImplWwise::~CEngineModule_CryAudioImplWwise()
{
}

#include <CryCore/CrtDebugStats.h>
