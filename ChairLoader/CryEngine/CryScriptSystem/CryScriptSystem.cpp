// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#include "StdAfx.h"
#include "ScriptSystem.h"

// Included only once per DLL module.
#include <CryCore/Platform/platform_impl.inl>

#include <CrySystem/IEngineModule.h>
#include <CryExtension/ICryFactory.h>
#include <CryExtension/ClassWeaver.h>

//////////////////////////////////////////////////////////////////////////
struct CSystemEventListner_Script : public ISystemEventListener
{
public:
	virtual void OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam)
	{
		switch (event)
		{
		case ESYSTEM_EVENT_RANDOM_SEED:
			cry_random_seed(gEnv->bNoRandomSeed ? 0 : (uint32)wparam);
			break;
		}
	}
};
static CSystemEventListner_Script g_system_event_listener_script;

//////////////////////////////////////////////////////////////////////////
class CEngineModule_CryScriptSystem : public IEngineModule
{
	CRYINTERFACE_SIMPLE(IEngineModule)
	CRYGENERATE_SINGLETONCLASS(CEngineModule_CryScriptSystem, "EngineModule_CryScriptSystem", 0xd032b16449784f82, 0xa99e7dc6b6338c5c)

	//////////////////////////////////////////////////////////////////////////
	virtual const char* GetName() override { return "CryScriptSystem"; };
	virtual const char* GetCategory() override { return "CryEngine"; };

	//////////////////////////////////////////////////////////////////////////
	virtual bool Initialize(SSystemGlobalEnvironment& env, const SSystemInitParams& initParams) override
	{
		ISystem* pSystem = env.pSystem;

		CScriptSystem* pScriptSystem = new CScriptSystem;

		pSystem->GetISystemEventDispatcher()->RegisterListener(&g_system_event_listener_script);

		bool bStdLibs = true;
		if (!pScriptSystem->Init(pSystem, bStdLibs, 1024))
		{
			pScriptSystem->Release();
			return false;
		}

		env.pScriptSystem = pScriptSystem;
		return true;
	}
};

CRYREGISTER_SINGLETON_CLASS(CEngineModule_CryScriptSystem)

CEngineModule_CryScriptSystem::CEngineModule_CryScriptSystem()
{
};

CEngineModule_CryScriptSystem::~CEngineModule_CryScriptSystem()
{
};

#if CRY_PLATFORM_WINDOWS && !defined(_LIB)
HANDLE gDLLHandle = NULL;
BOOL APIENTRY DllMain(HANDLE hModule,
                      DWORD ul_reason_for_call,
                      LPVOID lpReserved)
{
	gDLLHandle = hModule;
	return TRUE;
}
#endif

#include <CryCore/CrtDebugStats.h>
