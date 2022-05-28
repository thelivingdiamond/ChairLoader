// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#include "stdafx.h"
#include "AudioImpl_fmod.h"
#include "AudioEventData_fmod.h"
#include "AudioObjectData_fmod.h"
#include "AudioImplCVars_fmod.h"
#include "ATLEntities_fmod.h"
#include <CrySystem/File/ICryPak.h>
#include <CryAudio/IAudioSystem.h>

using namespace CryAudio::Impl;

AudioParameterToIndexMap CryAudio::Impl::g_parameterToIndex;
FmodSwitchToIndexMap CryAudio::Impl::g_switchToIndex;

char const* const CAudioImpl_fmod::s_szFmodEventTag = "FmodEvent";
char const* const CAudioImpl_fmod::s_szFmodSnapshotTag = "FmodSnapshot";
char const* const CAudioImpl_fmod::s_szFmodEventParameterTag = "FmodEventParameter";
char const* const CAudioImpl_fmod::s_szFmodSnapshotParameterTag = "FmodSnapshotParameter";
char const* const CAudioImpl_fmod::s_szFmodFileTag = "FmodFile";
char const* const CAudioImpl_fmod::s_szFmodBusTag = "FmodBus";
char const* const CAudioImpl_fmod::s_szFmodNameAttribute = "fmod_name";
char const* const CAudioImpl_fmod::s_szFmodValueAttribute = "fmod_value";
char const* const CAudioImpl_fmod::s_szFmodMutiplierAttribute = "fmod_value_multiplier";
char const* const CAudioImpl_fmod::s_szFmodShiftAttribute = "fmod_value_shift";
char const* const CAudioImpl_fmod::s_szFmodPathAttribute = "fmod_path";
char const* const CAudioImpl_fmod::s_szFmodLocalizedAttribute = "fmod_localized";
char const* const CAudioImpl_fmod::s_szFmodEventTypeAttribute = "fmod_event_type";
char const* const CAudioImpl_fmod::s_szFmodEventPrefix = "event:/";
char const* const CAudioImpl_fmod::s_szFmodSnapshotPrefix = "snapshot:/";
char const* const CAudioImpl_fmod::s_szFmodBusPrefix = "bus:/";

AudioParameterToIndexMap g_parameterToIndex;
FmodSwitchToIndexMap g_switchToIndex;

//////////////////////////////////////////////////////////////////////////
FMOD_RESULT F_CALLBACK EventCallback(FMOD_STUDIO_EVENT_CALLBACK_TYPE type, FMOD_STUDIO_EVENTINSTANCE* event, void* parameters)
{
	FMOD::Studio::EventInstance* const pEvent = reinterpret_cast<FMOD::Studio::EventInstance*>(event);

	if (pEvent != nullptr)
	{
		CAudioEvent_fmod* pAudioEvent = nullptr;
		FMOD_RESULT const fmodResult = pEvent->getUserData(reinterpret_cast<void**>(&pAudioEvent));
		ASSERT_FMOD_OK;

		if (pAudioEvent != nullptr)
		{
			SAudioRequest request;
			SAudioCallbackManagerRequestData<eAudioCallbackManagerRequestType_ReportFinishedEvent> requestData(pAudioEvent->GetId(), true);
			request.flags = eAudioRequestFlags_ThreadSafePush;
			request.pData = &requestData;

			gEnv->pAudioSystem->PushRequest(request);
		}
	}

	return FMOD_OK;
}

///////////////////////////////////////////////////////////////////////////
CAudioImpl_fmod::CAudioImpl_fmod()
	: m_globalAudioObjectID(INVALID_AUDIO_OBJECT_ID)
	, m_pSystem(nullptr)
	, m_pLowLevelSystem(nullptr)
	, m_pMasterBank(nullptr)
	, m_pStringsBank(nullptr)
{
	m_registeredAudioObjects.reserve(256);
}

///////////////////////////////////////////////////////////////////////////
CAudioImpl_fmod::~CAudioImpl_fmod()
{
}

///////////////////////////////////////////////////////////////////////////
void CAudioImpl_fmod::Update(float const deltaTime)
{
	if (m_pSystem != nullptr)
	{
		FMOD_RESULT fmodResult = FMOD_ERR_UNINITIALIZED;
		fmodResult = m_pSystem->update();
		ASSERT_FMOD_OK;

		if (!m_pendingAudioEvents.empty())
		{
			AudioEvents::iterator iter(m_pendingAudioEvents.begin());
			AudioEvents::const_iterator iterEnd(m_pendingAudioEvents.end());

			while (iter != iterEnd)
			{
				if ((*iter)->GetAudioObjectData()->SetAudioEvent(*iter))
				{
					iter = m_pendingAudioEvents.erase(iter);
					iterEnd = m_pendingAudioEvents.end();
					continue;
				}

				++iter;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioImpl_fmod::Init()
{
	string const gameFolder = PathUtil::GetGameFolder().c_str();

	if (gameFolder.empty())
	{
		CryFatalError("<Audio>: Needs a valid game folder to proceed!");
	}

	m_regularSoundBankFolder = gameFolder.c_str();
	m_regularSoundBankFolder += CRY_NATIVE_PATH_SEPSTR;
	m_regularSoundBankFolder += FMOD_IMPL_DATA_ROOT;
	m_localizedSoundBankFolder = m_regularSoundBankFolder;

	FMOD_RESULT fmodResult = FMOD::Studio::System::create(&m_pSystem);
	ASSERT_FMOD_OK;
	fmodResult = m_pSystem->getLowLevelSystem(&m_pLowLevelSystem);
	ASSERT_FMOD_OK;

#if defined(INCLUDE_FMOD_IMPL_PRODUCTION_CODE)
	uint32 version = 0;
	fmodResult = m_pLowLevelSystem->getVersion(&version);
	ASSERT_FMOD_OK;

	CryFixedStringT<MAX_AUDIO_MISC_STRING_LENGTH> systemVersion;
	systemVersion.Format("%08x", version);
	CryFixedStringT<MAX_AUDIO_MISC_STRING_LENGTH> headerVersion;
	headerVersion.Format("%08x", FMOD_VERSION);
	CreateVersionString(systemVersion);
	CreateVersionString(headerVersion);

	m_fullImplString = FMOD_IMPL_INFO_STRING;
	m_fullImplString += "System: ";
	m_fullImplString += systemVersion;
	m_fullImplString += " Header: ";
	m_fullImplString += headerVersion;
	m_fullImplString += " (";
	m_fullImplString += gameFolder + CRY_NATIVE_PATH_SEPSTR FMOD_IMPL_DATA_ROOT ")";
#endif // INCLUDE_FMOD_IMPL_PRODUCTION_CODE

	int sampleRate = 0;
	int numRawSpeakers = 0;
	FMOD_SPEAKERMODE speakerMode = FMOD_SPEAKERMODE_DEFAULT;
	fmodResult = m_pLowLevelSystem->getSoftwareFormat(&sampleRate, &speakerMode, &numRawSpeakers);
	ASSERT_FMOD_OK;
	fmodResult = m_pLowLevelSystem->setSoftwareFormat(sampleRate, speakerMode, numRawSpeakers);
	ASSERT_FMOD_OK;

	fmodResult = m_pLowLevelSystem->set3DSettings(g_audioImplCVars_fmod.m_dopplerScale, g_audioImplCVars_fmod.m_distanceFactor, g_audioImplCVars_fmod.m_rolloffScale);
	ASSERT_FMOD_OK;

	void* pExtraDriverData = nullptr;
	int initFlags = FMOD_INIT_NORMAL | FMOD_INIT_VOL0_BECOMES_VIRTUAL;
	int studioInitFlags = FMOD_STUDIO_INIT_NORMAL;

	if (g_audioImplCVars_fmod.m_enableLiveUpdate > 0)
	{
		studioInitFlags |= FMOD_STUDIO_INIT_LIVEUPDATE;
	}

	if (g_audioImplCVars_fmod.m_enableSynchronousUpdate > 0)
	{
		studioInitFlags |= FMOD_STUDIO_INIT_SYNCHRONOUS_UPDATE;
	}

	fmodResult = m_pSystem->initialize(g_audioImplCVars_fmod.m_maxChannels, studioInitFlags, initFlags, pExtraDriverData);
	ASSERT_FMOD_OK;

	if (!LoadMasterBanks())
	{
		return eAudioRequestStatus_Failure;
	}

	FMOD_3D_ATTRIBUTES attributes = {
		{ 0 }
	};
	attributes.forward.z = 1.0f;
	attributes.up.y = 1.0f;
	fmodResult = m_pSystem->setListenerAttributes(0, &attributes);
	ASSERT_FMOD_OK;

	return (fmodResult == FMOD_OK) ? eAudioRequestStatus_Success : eAudioRequestStatus_Failure;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioImpl_fmod::ShutDown()
{
	FMOD_RESULT fmodResult = FMOD_OK;

	if (m_pSystem != nullptr)
	{
		UnloadMasterBanks();

		fmodResult = m_pSystem->release();
		ASSERT_FMOD_OK;
	}

	return (fmodResult == FMOD_OK) ? eAudioRequestStatus_Success : eAudioRequestStatus_Failure;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioImpl_fmod::Release()
{
	POOL_FREE(this);

	// Freeing Memory Pool Memory again
	uint8* pMemSystem = g_audioImplMemoryPool_fmod.Data();
	g_audioImplMemoryPool_fmod.UnInitMem();

	if (pMemSystem)
	{
		delete[] (uint8*)(pMemSystem);
	}

	g_audioImplCVars_fmod.UnregisterVariables();

	return eAudioRequestStatus_Success;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioImpl_fmod::OnLoseFocus()
{
	return MuteMasterBus(true);
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioImpl_fmod::OnGetFocus()
{
	return MuteMasterBus(false);
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioImpl_fmod::MuteAll()
{
	return MuteMasterBus(true);
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioImpl_fmod::UnmuteAll()
{
	return MuteMasterBus(false);
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioImpl_fmod::StopAllSounds()
{
	FMOD::Studio::Bus* pMasterBus = nullptr;
	FMOD_RESULT fmodResult = m_pSystem->getBus("bus:/", &pMasterBus);
	ASSERT_FMOD_OK;

	if (pMasterBus != nullptr)
	{
		fmodResult = pMasterBus->stopAllEvents(FMOD_STUDIO_STOP_IMMEDIATE);
		ASSERT_FMOD_OK;
	}

	return (fmodResult == FMOD_OK) ? eAudioRequestStatus_Success : eAudioRequestStatus_Failure;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioImpl_fmod::RegisterAudioObject(IAudioObject* const pAudioObject)
{
	CAudioObject_fmod* const pFmodAudioObject = static_cast<CAudioObject_fmod* const>(pAudioObject);

	if (!stl::push_back_unique(m_registeredAudioObjects, pFmodAudioObject))
	{
		g_audioImplLogger_fmod.Log(eAudioLogType_Warning, "Trying to register an already registered audio object. (%u)", pFmodAudioObject->GetId());
	}

	return eAudioRequestStatus_Success;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioImpl_fmod::RegisterAudioObject(
  IAudioObject* const pAudioObject,
  char const* const szAudioObjectName)
{
	CAudioObject_fmod* const pFmodAudioObject = static_cast<CAudioObject_fmod* const>(pAudioObject);

	if (!stl::push_back_unique(m_registeredAudioObjects, pFmodAudioObject))
	{
		g_audioImplLogger_fmod.Log(eAudioLogType_Warning, "Trying to register an already registered audio object. (%u)", pFmodAudioObject->GetId());
	}

	return eAudioRequestStatus_Success;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioImpl_fmod::UnregisterAudioObject(IAudioObject* const pAudioObject)
{
	CAudioObject_fmod* const pFmodAudioObject = static_cast<CAudioObject_fmod* const>(pAudioObject);

	if (!stl::find_and_erase(m_registeredAudioObjects, pFmodAudioObject))
	{
		g_audioImplLogger_fmod.Log(eAudioLogType_Warning, "Trying to unregister a non-existing audio object. (%u)", pFmodAudioObject->GetId());
	}

	return eAudioRequestStatus_Success;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioImpl_fmod::ResetAudioObject(IAudioObject* const pAudioObject)
{
	CAudioObject_fmod* const pFmodAudioObject = static_cast<CAudioObject_fmod* const>(pAudioObject);
	pFmodAudioObject->Reset();
	return eAudioRequestStatus_Success;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioImpl_fmod::UpdateAudioObject(IAudioObject* const pAudioObject)
{
	CAudioObject_fmod* const pFmodAudioObject = static_cast<CAudioObject_fmod* const>(pAudioObject);
	return pFmodAudioObject ? eAudioRequestStatus_Success : eAudioRequestStatus_Failure;
}

//////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioImpl_fmod::PlayFile(SAudioStandaloneFileInfo* const _pAudioStandaloneFileInfo)
{
	EAudioRequestStatus result = eAudioRequestStatus_Failure;
	CAudioObject_fmod const* const pFmodAudioObject = static_cast<CAudioObject_fmod* const>(_pAudioStandaloneFileInfo->pAudioObject);

	bool bImmediate = true;
	bool bDelayed = false;

	if (bImmediate)
	{
		result = eAudioRequestStatus_Success;
	}
	else if (bDelayed)
	{
		// Pending requests need to report later on using the method below.
		/*SAudioRequest request;
		   SAudioCallbackManagerRequestData<eACMRT_REPORT_STARTED_FILE> requestData(pFmodAudioObject->id, _pAudioStandaloneFileInfo->fileInstanceID, _pAudioStandaloneFileInfo->szFileName);
		   request.nFlags = eARF_THREAD_SAFE_PUSH;
		   request.pData = &requestData;

		   gEnv->pAudioSystem->PushRequest(request);*/
		result = eAudioRequestStatus_Pending;
	}

	return result;
}

//////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioImpl_fmod::StopFile(SAudioStandaloneFileInfo* const _pAudioStandaloneFileInfo)
{
	EAudioRequestStatus result = eAudioRequestStatus_Failure;
	CAudioObject_fmod const* const pFmodAudioObject = static_cast<CAudioObject_fmod* const>(_pAudioStandaloneFileInfo->pAudioObject);

	bool bImmediate = true;
	bool bDelayed = false;

	if (bImmediate)
	{
		result = eAudioRequestStatus_Success;
	}
	else if (bDelayed)
	{
		// Pending requests need to report later on using the method below.
		/*SAudioRequest request;
		   SAudioCallbackManagerRequestData<eACMRT_REPORT_STOPPED_FILE> requestData(pFmodAudioObject->id, _pAudioStandaloneFileInfo->fileInstanceID, _pAudioStandaloneFileInfo->szFileName);
		   request.nFlags = eARF_THREAD_SAFE_PUSH;
		   request.pData = &requestData;

		   gEnv->pAudioSystem->PushRequest(request);*/
		result = eAudioRequestStatus_Pending;
	}

	return result;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioImpl_fmod::PrepareTriggerSync(
  IAudioObject* const pAudioObject,
  IAudioTrigger const* const pAudioTrigger)
{
	return eAudioRequestStatus_Success;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioImpl_fmod::UnprepareTriggerSync(
  IAudioObject* const pAudioObject,
  IAudioTrigger const* const pAudioTrigger)
{
	return eAudioRequestStatus_Success;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioImpl_fmod::PrepareTriggerAsync(
  IAudioObject* const pAudioObject,
  IAudioTrigger const* const pAudioTrigger,
  IAudioEvent* const pAudioEvent)
{
	return eAudioRequestStatus_Success;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioImpl_fmod::UnprepareTriggerAsync(
  IAudioObject* const pAudioObject,
  IAudioTrigger const* const pAudioTrigger,
  IAudioEvent* const pAudioEvent)
{
	return eAudioRequestStatus_Success;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioImpl_fmod::ActivateTrigger(
  IAudioObject* const pAudioObject,
  IAudioTrigger const* const pAudioTrigger,
  IAudioEvent* const pAudioEvent)
{
	EAudioRequestStatus requestResult = eAudioRequestStatus_Failure;
	CAudioObject_fmod* const pFmodAudioObject = static_cast<CAudioObject_fmod* const>(pAudioObject);
	CAudioTrigger_fmod const* const pFmodAudioTrigger = static_cast<CAudioTrigger_fmod const* const>(pAudioTrigger);
	CAudioEvent_fmod* const pFmodAudioEvent = static_cast<CAudioEvent_fmod*>(pAudioEvent);

	if ((pFmodAudioObject != nullptr) && (pFmodAudioTrigger != nullptr) && (pFmodAudioEvent != nullptr))
	{
		if (pFmodAudioTrigger->m_eventType == eFmodEventType_Start)
		{
			FMOD_RESULT fmodResult = FMOD_ERR_UNINITIALIZED;
			FMOD::Studio::EventDescription* pEventDescription = pFmodAudioTrigger->m_pEventDescription;

			if (pEventDescription == nullptr)
			{
				fmodResult = m_pSystem->getEventByID(&pFmodAudioTrigger->m_guid, &pEventDescription);
				ASSERT_FMOD_OK;
			}

			if (pEventDescription != nullptr)
			{
				CRY_ASSERT(pFmodAudioEvent->GetInstance() == nullptr);

				FMOD::Studio::EventInstance* pInstance = nullptr;
				fmodResult = pEventDescription->createInstance(&pInstance);
				ASSERT_FMOD_OK;
				pFmodAudioEvent->SetInstance(pInstance);
				fmodResult = pFmodAudioEvent->GetInstance()->setCallback(EventCallback, FMOD_STUDIO_EVENT_CALLBACK_START_FAILED | FMOD_STUDIO_EVENT_CALLBACK_STOPPED);
				ASSERT_FMOD_OK;
				fmodResult = pFmodAudioEvent->GetInstance()->setUserData(pFmodAudioEvent);
				ASSERT_FMOD_OK;
				fmodResult = pFmodAudioEvent->GetInstance()->set3DAttributes(&pFmodAudioObject->Get3DAttributes());
				ASSERT_FMOD_OK;

				CRY_ASSERT(pFmodAudioEvent->GetEventPathId() == AUDIO_INVALID_CRC32);
				pFmodAudioEvent->SetEventPathId(pFmodAudioTrigger->m_eventPathId);
				pFmodAudioEvent->SetAudioObjectData(pFmodAudioObject);

				stl::push_back_unique(m_pendingAudioEvents, pFmodAudioEvent);
				requestResult = eAudioRequestStatus_Success;
			}
		}
		else
		{
			pFmodAudioObject->StopEvent(pFmodAudioTrigger->m_eventPathId);

			// Return failure here so the ATL does not keep track of this event.
			requestResult = eAudioRequestStatus_Failure;
		}
	}
	else
	{
		g_audioImplLogger_fmod.Log(eAudioLogType_Error, "Invalid AudioObjectData, ATLTriggerData or EventData passed to the Fmod implementation of ActivateTrigger.");
	}

	return requestResult;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioImpl_fmod::StopEvent(
  IAudioObject* const pAudioObject,
  IAudioEvent const* const pAudioEvent)
{
	EAudioRequestStatus requestResult = eAudioRequestStatus_Failure;
	CAudioEvent_fmod const* const pFmodAudioEvent = static_cast<CAudioEvent_fmod const* const>(pAudioEvent);

	if (pFmodAudioEvent != nullptr)
	{
		FMOD_RESULT const fmodResult = pFmodAudioEvent->GetInstance()->stop(FMOD_STUDIO_STOP_IMMEDIATE);
		ASSERT_FMOD_OK;
		requestResult = eAudioRequestStatus_Success;
	}
	else
	{
		g_audioImplLogger_fmod.Log(eAudioLogType_Error, "Invalid EventData passed to the Fmod implementation of StopEvent.");
	}

	return requestResult;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioImpl_fmod::StopAllEvents(IAudioObject* const pAudioObject)
{
	EAudioRequestStatus requestResult = eAudioRequestStatus_Failure;
	CAudioObject_fmod* const pFmodAudioObject = static_cast<CAudioObject_fmod* const>(pAudioObject);

	if (pFmodAudioObject != nullptr)
	{
		pFmodAudioObject->StopAllEvents();
		requestResult = eAudioRequestStatus_Success;
	}
	else
	{
		g_audioImplLogger_fmod.Log(eAudioLogType_Error, "Invalid AudioObjectData passed to the Fmod implementation of StopAllEvents.");
	}

	return requestResult;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioImpl_fmod::Set3DAttributes(
  IAudioObject* const pAudioObject,
  SAudioObject3DAttributes const& attributes)
{
	EAudioRequestStatus requestResult = eAudioRequestStatus_Failure;
	CAudioObject_fmod* const pFmodAudioObject = static_cast<CAudioObject_fmod* const>(pAudioObject);

	if (pFmodAudioObject != nullptr)
	{
		pFmodAudioObject->Set3DAttributes(attributes);
		requestResult = eAudioRequestStatus_Success;
	}
	else
	{
		g_audioImplLogger_fmod.Log(eAudioLogType_Error, "Invalid AudioObjectData passed to the Fmod implementation of SetPosition.");
	}

	return requestResult;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioImpl_fmod::SetEnvironment(
  IAudioObject* const pAudioObject,
  IAudioEnvironment const* const pAudioEnvironment,
  float const amount)
{
	EAudioRequestStatus result = eAudioRequestStatus_Failure;
	CAudioObject_fmod* const pFmodAudioObject = static_cast<CAudioObject_fmod* const>(pAudioObject);
	CAudioEnvironment_fmod const* const pFmodAudioEnvironment = static_cast<CAudioEnvironment_fmod const* const>(pAudioEnvironment);

	if ((pFmodAudioObject != nullptr) && (pFmodAudioEnvironment != nullptr))
	{
		if (pFmodAudioObject->GetId() != m_globalAudioObjectID)
		{
			pFmodAudioObject->SetEnvironment(pFmodAudioEnvironment, amount);
		}
		else
		{
			for (auto const pRegisteredAudioObject : m_registeredAudioObjects)
			{
				pRegisteredAudioObject->SetEnvironment(pFmodAudioEnvironment, amount);
			}
		}

		result = eAudioRequestStatus_Success;
	}
	else
	{
		g_audioImplLogger_fmod.Log(eAudioLogType_Error, "Invalid AudioObjectData or EnvironmentData passed to the Fmod implementation of SetEnvironment");
	}

	return result;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioImpl_fmod::SetRtpc(
  IAudioObject* const pAudioObject,
  IAudioRtpc const* const pAudioRtpc,
  float const value)
{
	EAudioRequestStatus result = eAudioRequestStatus_Failure;
	CAudioObject_fmod* const pFmodAudioObject = static_cast<CAudioObject_fmod* const>(pAudioObject);
	CAudioParameter_fmod const* const pFmodAudioParameter = static_cast<CAudioParameter_fmod const* const>(pAudioRtpc);

	if ((pFmodAudioObject != nullptr) && (pFmodAudioParameter != nullptr))
	{
		if (pFmodAudioObject->GetId() != m_globalAudioObjectID)
		{
			pFmodAudioObject->SetParameter(pFmodAudioParameter, value);
		}
		else
		{
			for (auto const pAudioObject : m_registeredAudioObjects)
			{
				pAudioObject->SetParameter(pFmodAudioParameter, value);
			}
		}

		result = eAudioRequestStatus_Success;
	}
	else
	{
		g_audioImplLogger_fmod.Log(eAudioLogType_Error, "Invalid AudioObjectData or RtpcData passed to the Fmod implementation of SetRtpc");
	}

	return result;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioImpl_fmod::SetSwitchState(
  IAudioObject* const pAudioObject,
  IAudioSwitchState const* const pAudioSwitchState)
{
	EAudioRequestStatus result = eAudioRequestStatus_Failure;
	CAudioObject_fmod* const pFmodAudioObject = static_cast<CAudioObject_fmod* const>(pAudioObject);
	CAudioSwitchState_fmod const* const pFmodAudioSwitchState = static_cast<CAudioSwitchState_fmod const* const>(pAudioSwitchState);

	if ((pFmodAudioObject != nullptr) && (pFmodAudioSwitchState != nullptr))
	{
		if (pFmodAudioObject->GetId() != m_globalAudioObjectID)
		{
			pFmodAudioObject->SetSwitch(pFmodAudioSwitchState);
		}
		else
		{
			for (auto const pAudioObject : m_registeredAudioObjects)
			{
				pAudioObject->SetSwitch(pFmodAudioSwitchState);
			}
		}

		result = eAudioRequestStatus_Success;
	}
	else
	{
		g_audioImplLogger_fmod.Log(eAudioLogType_Error, "Invalid AudioObjectData or RtpcData passed to the Fmod implementation of SetSwitchState");
	}

	return result;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioImpl_fmod::SetObstructionOcclusion(
  IAudioObject* const pAudioObject,
  float const obstruction,
  float const occlusion)
{
	EAudioRequestStatus result = eAudioRequestStatus_Failure;
	CAudioObject_fmod* const pFmodAudioObject = static_cast<CAudioObject_fmod* const>(pAudioObject);

	if (pFmodAudioObject != nullptr)
	{
		if (pFmodAudioObject->GetId() != m_globalAudioObjectID)
		{
			pFmodAudioObject->SetObstructionOcclusion(obstruction, occlusion);
		}
		else
		{
			g_audioImplLogger_fmod.Log(eAudioLogType_Error, "Trying to set occlusion and obstruction values on the global audio object!");
		}

		result = eAudioRequestStatus_Success;
	}
	else
	{
		g_audioImplLogger_fmod.Log(eAudioLogType_Error, "Invalid AudioObjectData passed to the Fmod implementation of SetObjectObstructionAndOcclusion");
	}

	return result;
}

///////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioImpl_fmod::SetListener3DAttributes(
  IAudioListener* const pAudioListener,
  SAudioObject3DAttributes const& attributes)
{
	EAudioRequestStatus requestResult = eAudioRequestStatus_Failure;
	CAudioListener_fmod* const pFmodAudioListener = static_cast<CAudioListener_fmod* const>(pAudioListener);

	if (pFmodAudioListener != nullptr)
	{
		FillFmodObjectPosition(attributes, pFmodAudioListener->Get3DAttributes());
		FMOD_RESULT const fmodResult = m_pSystem->setListenerAttributes(pFmodAudioListener->GetId(), &pFmodAudioListener->Get3DAttributes());
		ASSERT_FMOD_OK;

		requestResult = eAudioRequestStatus_Success;
	}
	else
	{
		g_audioImplLogger_fmod.Log(eAudioLogType_Error, "Invalid ATLListenerData passed to the Fmod implementation of SetListenerPosition");
	}

	return requestResult;
}

//////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioImpl_fmod::RegisterInMemoryFile(SAudioFileEntryInfo* const pFileEntryInfo)
{
	EAudioRequestStatus requestResult = eAudioRequestStatus_Failure;

	if (pFileEntryInfo != nullptr)
	{
		CAudioFileEntry_fmod* const pFmodAudioFileEntry = static_cast<CAudioFileEntry_fmod*>(pFileEntryInfo->pImplData);

		if (pFmodAudioFileEntry != nullptr)
		{
#if defined(INCLUDE_FMOD_IMPL_PRODUCTION_CODE)
			// loadBankMemory requires 32-byte alignment when using FMOD_STUDIO_LOAD_MEMORY_POINT
			if ((reinterpret_cast<uintptr_t>(pFileEntryInfo->pFileData) & (32 - 1)) > 0)
			{
				CryFatalError("<Audio>: allocation not %d byte aligned!", 32);
			}
#endif // INCLUDE_FMOD_IMPL_PRODUCTION_CODE

			FMOD_RESULT const fmodResult = m_pSystem->loadBankMemory(static_cast<char*>(pFileEntryInfo->pFileData), static_cast<int>(pFileEntryInfo->size), FMOD_STUDIO_LOAD_MEMORY_POINT, FMOD_STUDIO_LOAD_BANK_NORMAL, &pFmodAudioFileEntry->pBank);
			ASSERT_FMOD_OK;
			requestResult = (fmodResult == FMOD_OK) ? eAudioRequestStatus_Success : eAudioRequestStatus_Failure;
		}
		else
		{
			g_audioImplLogger_fmod.Log(eAudioLogType_Error, "Invalid AudioFileEntryData passed to the Fmod implementation of RegisterInMemoryFile");
		}
	}

	return requestResult;
}

//////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioImpl_fmod::UnregisterInMemoryFile(SAudioFileEntryInfo* const pFileEntryInfo)
{
	EAudioRequestStatus requestResult = eAudioRequestStatus_Failure;

	if (pFileEntryInfo != nullptr)
	{
		CAudioFileEntry_fmod* const pFmodAudioFileEntry = static_cast<CAudioFileEntry_fmod*>(pFileEntryInfo->pImplData);

		if (pFmodAudioFileEntry != nullptr)
		{
			FMOD_RESULT fmodResult = pFmodAudioFileEntry->pBank->unload();
			ASSERT_FMOD_OK;

			FMOD_STUDIO_LOADING_STATE loadingState;

			do
			{
				fmodResult = m_pSystem->update();
				ASSERT_FMOD_OK;
				fmodResult = pFmodAudioFileEntry->pBank->getLoadingState(&loadingState);
				ASSERT_FMOD_OK_OR_INVALID_HANDLE;
			}
			while (loadingState == FMOD_STUDIO_LOADING_STATE_UNLOADING);

			pFmodAudioFileEntry->pBank = nullptr;
			requestResult = (fmodResult == FMOD_OK) ? eAudioRequestStatus_Success : eAudioRequestStatus_Failure;
		}
		else
		{
			g_audioImplLogger_fmod.Log(eAudioLogType_Error, "Invalid AudioFileEntryData passed to the Fmod implementation of UnregisterInMemoryFile");
		}
	}

	return requestResult;
}

//////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioImpl_fmod::ParseAudioFileEntry(
  XmlNodeRef const pAudioFileEntryNode,
  SAudioFileEntryInfo* const pFileEntryInfo)
{
	EAudioRequestStatus result = eAudioRequestStatus_Failure;

	if ((_stricmp(pAudioFileEntryNode->getTag(), s_szFmodFileTag) == 0) && (pFileEntryInfo != nullptr))
	{
		char const* const szFmodAudioFileEntryName = pAudioFileEntryNode->getAttr(s_szFmodNameAttribute);

		if (szFmodAudioFileEntryName != nullptr && szFmodAudioFileEntryName[0] != '\0')
		{
			char const* const szFmodLocalized = pAudioFileEntryNode->getAttr(s_szFmodLocalizedAttribute);
			pFileEntryInfo->bLocalized = (szFmodLocalized != nullptr) && (_stricmp(szFmodLocalized, "true") == 0);
			pFileEntryInfo->szFileName = szFmodAudioFileEntryName;

			// FMOD Studio always uses 32 byte alignment for preloaded banks regardless of the platform.
			pFileEntryInfo->memoryBlockAlignment = 32;

			POOL_NEW(CAudioFileEntry_fmod, pFileEntryInfo->pImplData);

			result = eAudioRequestStatus_Success;
		}
		else
		{
			pFileEntryInfo->szFileName = nullptr;
			pFileEntryInfo->memoryBlockAlignment = 0;
			pFileEntryInfo->pImplData = nullptr;
		}
	}

	return result;
}

//////////////////////////////////////////////////////////////////////////
void CAudioImpl_fmod::DeleteAudioFileEntry(IAudioFileEntry* const pOldAudioFileEntry)
{
	POOL_FREE(pOldAudioFileEntry);
}

//////////////////////////////////////////////////////////////////////////
char const* const CAudioImpl_fmod::GetAudioFileLocation(SAudioFileEntryInfo* const pFileEntryInfo)
{
	char const* sResult = nullptr;

	if (pFileEntryInfo != nullptr)
	{
		sResult = pFileEntryInfo->bLocalized ? m_localizedSoundBankFolder.c_str() : m_regularSoundBankFolder.c_str();
	}

	return sResult;
}

///////////////////////////////////////////////////////////////////////////
IAudioObject* CAudioImpl_fmod::NewGlobalAudioObject(AudioObjectId const audioObjectID)
{
	POOL_NEW_CREATE(CAudioObject_fmod, pFmodAudioObject)(audioObjectID);
	m_globalAudioObjectID = audioObjectID;
	return pFmodAudioObject;
}

///////////////////////////////////////////////////////////////////////////
IAudioObject* CAudioImpl_fmod::NewAudioObject(AudioObjectId const audioObjectID)
{
	POOL_NEW_CREATE(CAudioObject_fmod, pFmodAudioObject)(audioObjectID);
	return pFmodAudioObject;
}

///////////////////////////////////////////////////////////////////////////
void CAudioImpl_fmod::DeleteAudioObject(IAudioObject* const pOldAudioObject)
{
	POOL_FREE(pOldAudioObject);
}

///////////////////////////////////////////////////////////////////////////
CryAudio::Impl::IAudioListener* CAudioImpl_fmod::NewDefaultAudioListener(AudioObjectId const audioObjectId)
{
	POOL_NEW_CREATE(CAudioListener_fmod, pAudioListener)(0);
	return pAudioListener;
}

///////////////////////////////////////////////////////////////////////////
CryAudio::Impl::IAudioListener* CAudioImpl_fmod::NewAudioListener(AudioObjectId const audioObjectId)
{
	static int id = 0;
	POOL_NEW_CREATE(CAudioListener_fmod, pAudioListener)(++id);
	return pAudioListener;
}

///////////////////////////////////////////////////////////////////////////
void CAudioImpl_fmod::DeleteAudioListener(CryAudio::Impl::IAudioListener* const pOldAudioListener)
{
	POOL_FREE(pOldAudioListener);
}

//////////////////////////////////////////////////////////////////////////
IAudioEvent* CAudioImpl_fmod::NewAudioEvent(AudioEventId const audioEventID)
{
	POOL_NEW_CREATE(CAudioEvent_fmod, pNewEvent)(audioEventID);
	return pNewEvent;
}

///////////////////////////////////////////////////////////////////////////
void CAudioImpl_fmod::DeleteAudioEvent(IAudioEvent* const pOldAudioEvent)
{
	POOL_FREE(pOldAudioEvent);
}

///////////////////////////////////////////////////////////////////////////
void CAudioImpl_fmod::ResetAudioEvent(IAudioEvent* const pAudioEvent)
{
	CAudioEvent_fmod* const pFmodAudioEvent = static_cast<CAudioEvent_fmod*>(pAudioEvent);

	if (pFmodAudioEvent != nullptr)
	{
		pFmodAudioEvent->Reset();
	}
}

//////////////////////////////////////////////////////////////////////////
IAudioStandaloneFile* CAudioImpl_fmod::NewAudioStandaloneFile()
{
	POOL_NEW_CREATE(CAudioStandaloneFile_fmod, pAudioStandaloneFile);
	return pAudioStandaloneFile;
}

//////////////////////////////////////////////////////////////////////////
void CAudioImpl_fmod::DeleteAudioStandaloneFile(IAudioStandaloneFile const* const _pOldAudioStandaloneFile)
{
	POOL_FREE_CONST(_pOldAudioStandaloneFile);
}

//////////////////////////////////////////////////////////////////////////
void CAudioImpl_fmod::ResetAudioStandaloneFile(IAudioStandaloneFile* const _pAudioStandaloneFile)
{
}

//////////////////////////////////////////////////////////////////////////
void CAudioImpl_fmod::GamepadConnected(TAudioGamepadUniqueID const deviceUniqueID)
{
}

//////////////////////////////////////////////////////////////////////////
void CAudioImpl_fmod::GamepadDisconnected(TAudioGamepadUniqueID const deviceUniqueID)
{
}

///////////////////////////////////////////////////////////////////////////
IAudioTrigger const* CAudioImpl_fmod::NewAudioTrigger(XmlNodeRef const pAudioTriggerNode, SAudioTriggerInfo& info)
{
	IAudioTrigger* pAudioTrigger = nullptr;
	char const* const szTag = pAudioTriggerNode->getTag();

	if (_stricmp(szTag, s_szFmodEventTag) == 0)
	{
		stack_string path(s_szFmodEventPrefix);
		path += pAudioTriggerNode->getAttr(s_szFmodNameAttribute);
		FMOD_GUID guid = { 0 };

		if (m_pSystem->lookupID(path.c_str(), &guid) == FMOD_OK)
		{
			EFmodEventType eventType = eFmodEventType_Start;
			char const* const szFmodEventType = pAudioTriggerNode->getAttr(s_szFmodEventTypeAttribute);

			if (szFmodEventType != nullptr &&
			    szFmodEventType[0] != '\0' &&
			    _stricmp(szFmodEventType, "stop") == 0)
			{
				eventType = eFmodEventType_Stop;
			}

#if defined (INCLUDE_FMOD_IMPL_PRODUCTION_CODE)
			POOL_NEW(CAudioTrigger_fmod, pAudioTrigger)(AudioStringToId(path.c_str()), eventType, nullptr, guid, path.c_str());
#else
			POOL_NEW(CAudioTrigger_fmod, pAudioTrigger)(AudioStringToId(path.c_str()), eventType, nullptr, guid);
#endif // INCLUDE_FMOD_IMPL_PRODUCTION_CODE
		}
		else
		{
			g_audioImplLogger_fmod.Log(eAudioLogType_Warning, "Unknown Fmod event: %s", path.c_str());
		}
	}
	else if (_stricmp(szTag, s_szFmodSnapshotTag) == 0)
	{
		stack_string path(s_szFmodSnapshotPrefix);
		path += pAudioTriggerNode->getAttr(s_szFmodNameAttribute);
		FMOD_GUID guid = { 0 };

		if (m_pSystem->lookupID(path.c_str(), &guid) == FMOD_OK)
		{
			FMOD::Studio::EventDescription* pEventDescription = nullptr;
			m_pSystem->getEventByID(&guid, &pEventDescription);

#if defined (INCLUDE_FMOD_IMPL_PRODUCTION_CODE)
			POOL_NEW(CAudioTrigger_fmod, pAudioTrigger)(AudioStringToId(path.c_str()), eFmodEventType_Start, pEventDescription, guid, path.c_str());
#else
			POOL_NEW(CAudioTrigger_fmod, pAudioTrigger)(AudioStringToId(path.c_str()), eFmodEventType_Start, pEventDescription, guid);
#endif // INCLUDE_FMOD_IMPL_PRODUCTION_CODE
		}
		else
		{
			g_audioImplLogger_fmod.Log(eAudioLogType_Warning, "Unknown Fmod snapshot: %s", path.c_str());
		}
	}
	else
	{
		g_audioImplLogger_fmod.Log(eAudioLogType_Warning, "Unknown Fmod tag: %s", szTag);
	}

	info.maxRadius = 0;
	return pAudioTrigger;
}

///////////////////////////////////////////////////////////////////////////
void CAudioImpl_fmod::DeleteAudioTrigger(IAudioTrigger const* const pOldAudioTrigger)
{
	POOL_FREE_CONST(pOldAudioTrigger);
}

///////////////////////////////////////////////////////////////////////////
IAudioRtpc const* CAudioImpl_fmod::NewAudioRtpc(XmlNodeRef const pAudioParameterNode)
{
	CAudioParameter_fmod* pFmodAudioParameter = nullptr;
	char const* const szTag = pAudioParameterNode->getTag();
	stack_string path;

	if (_stricmp(szTag, s_szFmodEventParameterTag) == 0)
	{
		path = s_szFmodEventPrefix;
	}
	else if (_stricmp(szTag, s_szFmodSnapshotParameterTag) == 0)
	{
		path = s_szFmodSnapshotPrefix;
	}

	if (!path.empty())
	{
		char const* const szName = pAudioParameterNode->getAttr(s_szFmodNameAttribute);
		char const* const szPath = pAudioParameterNode->getAttr(s_szFmodPathAttribute);
		path += szPath;
		uint32 const pathId = AudioStringToId(path.c_str());

		float multiplier = 1.0f;
		float shift = 0.0f;
		pAudioParameterNode->getAttr(s_szFmodMutiplierAttribute, multiplier);
		pAudioParameterNode->getAttr(s_szFmodShiftAttribute, shift);

		POOL_NEW(CAudioParameter_fmod, pFmodAudioParameter)(pathId, multiplier, shift, szName);
		g_parameterToIndex.emplace(std::piecewise_construct, std::make_tuple(pFmodAudioParameter), std::make_tuple(FMOD_IMPL_INVALID_INDEX));
	}
	else
	{
		g_audioImplLogger_fmod.Log(eAudioLogType_Warning, "Unknown Fmod tag: %s", szTag);
	}

	return static_cast<IAudioRtpc*>(pFmodAudioParameter);
}

///////////////////////////////////////////////////////////////////////////
void CAudioImpl_fmod::DeleteAudioRtpc(IAudioRtpc const* const pOldAudioRtpc)
{
	CAudioParameter_fmod const* const pFmodAudioParameter = static_cast<CAudioParameter_fmod const* const>(pOldAudioRtpc);

	for (auto const pAudioObject : m_registeredAudioObjects)
	{
		pAudioObject->RemoveParameter(pFmodAudioParameter);
	}

	g_parameterToIndex.erase(pFmodAudioParameter);
	POOL_FREE_CONST(pOldAudioRtpc);
}

///////////////////////////////////////////////////////////////////////////
IAudioSwitchState const* CAudioImpl_fmod::NewAudioSwitchState(XmlNodeRef const pAudioSwitchNode)
{
	CAudioSwitchState_fmod* pFmodAudioSwitchState = nullptr;
	char const* const szTag = pAudioSwitchNode->getTag();
	stack_string path;

	if (_stricmp(szTag, s_szFmodEventParameterTag) == 0)
	{
		path = s_szFmodEventPrefix;
	}
	else if (_stricmp(szTag, s_szFmodSnapshotParameterTag) == 0)
	{
		path = s_szFmodSnapshotPrefix;
	}

	if (!path.empty())
	{
		char const* const szFmodParameterName = pAudioSwitchNode->getAttr(s_szFmodNameAttribute);
		char const* const szFmodPath = pAudioSwitchNode->getAttr(s_szFmodPathAttribute);
		char const* const szFmodParameterValue = pAudioSwitchNode->getAttr(s_szFmodValueAttribute);
		path += szFmodPath;
		uint32 const pathId = AudioStringToId(path.c_str());
		float const value = static_cast<float>(atof(szFmodParameterValue));
		POOL_NEW(CAudioSwitchState_fmod, pFmodAudioSwitchState)(pathId, value, szFmodParameterName);
		g_switchToIndex.emplace(std::piecewise_construct, std::make_tuple(pFmodAudioSwitchState), std::make_tuple(FMOD_IMPL_INVALID_INDEX));
	}
	else
	{
		g_audioImplLogger_fmod.Log(eAudioLogType_Warning, "Unknown Fmod tag: %s", szTag);
	}

	return static_cast<IAudioSwitchState*>(pFmodAudioSwitchState);
}

///////////////////////////////////////////////////////////////////////////
void CAudioImpl_fmod::DeleteAudioSwitchState(IAudioSwitchState const* const pOldAudioSwitchState)
{
	CAudioSwitchState_fmod const* const pFmodAudioSwitchState = static_cast<CAudioSwitchState_fmod const* const>(pOldAudioSwitchState);

	for (auto const pAudioObject : m_registeredAudioObjects)
	{
		pAudioObject->RemoveSwitch(pFmodAudioSwitchState);
	}

	g_switchToIndex.erase(pFmodAudioSwitchState);
	POOL_FREE_CONST(pOldAudioSwitchState);
}

///////////////////////////////////////////////////////////////////////////
IAudioEnvironment const* CAudioImpl_fmod::NewAudioEnvironment(XmlNodeRef const pAudioEnvironmentNode)
{
	IAudioEnvironment* pAudioEnvironment = nullptr;
	char const* const szTag = pAudioEnvironmentNode->getTag();

	if (_stricmp(szTag, s_szFmodBusTag) == 0)
	{
		stack_string path(s_szFmodBusPrefix);
		path += pAudioEnvironmentNode->getAttr(s_szFmodNameAttribute);
		FMOD_GUID guid = { 0 };

		if (m_pSystem->lookupID(path.c_str(), &guid) == FMOD_OK)
		{
			FMOD::Studio::Bus* pBus = nullptr;
			FMOD_RESULT const fmodResult = m_pSystem->getBusByID(&guid, &pBus);
			ASSERT_FMOD_OK;
			POOL_NEW(CAudioEnvironment_fmod, pAudioEnvironment)(nullptr, pBus);
		}
		else
		{
			g_audioImplLogger_fmod.Log(eAudioLogType_Warning, "Unknown Fmod bus: %s", path.c_str());
		}
	}
	else if (_stricmp(szTag, s_szFmodSnapshotTag) == 0)
	{
		stack_string path(s_szFmodSnapshotPrefix);
		path += pAudioEnvironmentNode->getAttr(s_szFmodNameAttribute);
		FMOD_GUID guid = { 0 };

		if (m_pSystem->lookupID(path.c_str(), &guid) == FMOD_OK)
		{
			FMOD::Studio::EventDescription* pEventDescription = nullptr;
			FMOD_RESULT const fmodResult = m_pSystem->getEventByID(&guid, &pEventDescription);
			ASSERT_FMOD_OK;
			POOL_NEW(CAudioEnvironment_fmod, pAudioEnvironment)(pEventDescription, nullptr);
		}
		else
		{
			g_audioImplLogger_fmod.Log(eAudioLogType_Warning, "Unknown Fmod snapshot: %s", path.c_str());
		}
	}

	return pAudioEnvironment;
}

///////////////////////////////////////////////////////////////////////////
void CAudioImpl_fmod::DeleteAudioEnvironment(IAudioEnvironment const* const pOldAudioEnvironment)
{
	CAudioEnvironment_fmod const* const pFmodAudioEnvironment = static_cast<CAudioEnvironment_fmod const* const>(pOldAudioEnvironment);

	for (auto const pAudioObject : m_registeredAudioObjects)
	{
		pAudioObject->RemoveEnvironment(pFmodAudioEnvironment);
	}

	POOL_FREE_CONST(pOldAudioEnvironment);
}

///////////////////////////////////////////////////////////////////////////
char const* const CAudioImpl_fmod::GetImplementationNameString() const
{
#if defined(INCLUDE_FMOD_IMPL_PRODUCTION_CODE)
	return m_fullImplString.c_str();
#endif // INCLUDE_FMOD_IMPL_PRODUCTION_CODE

	return nullptr;
}

///////////////////////////////////////////////////////////////////////////
void CAudioImpl_fmod::GetMemoryInfo(SAudioImplMemoryInfo& memoryInfo) const
{
	memoryInfo.primaryPoolSize = g_audioImplMemoryPool_fmod.MemSize();
	memoryInfo.primaryPoolUsedSize = memoryInfo.primaryPoolSize - g_audioImplMemoryPool_fmod.MemFree();
	memoryInfo.primaryPoolAllocations = g_audioImplMemoryPool_fmod.FragmentCount();

	memoryInfo.bucketUsedSize = g_audioImplMemoryPool_fmod.GetSmallAllocsSize();
	memoryInfo.bucketAllocations = g_audioImplMemoryPool_fmod.GetSmallAllocsCount();

#if defined(PROVIDE_FMOD_IMPL_SECONDARY_POOL)
	memoryInfo.secondaryPoolSize = g_audioImplMemoryPoolSecondary_fmod.MemSize();
	memoryInfo.secondaryPoolUsedSize = memoryInfo.secondaryPoolSize - g_audioImplMemoryPoolSecondary_fmod.MemFree();
	memoryInfo.secondaryPoolAllocations = g_audioImplMemoryPoolSecondary_fmod.FragmentCount();
#else
	memoryInfo.secondaryPoolSize = 0;
	memoryInfo.secondaryPoolUsedSize = 0;
	memoryInfo.secondaryPoolAllocations = 0;
#endif // PROVIDE_AUDIO_IMPL_SECONDARY_POOL
}

//////////////////////////////////////////////////////////////////////////
void CAudioImpl_fmod::OnAudioSystemRefresh()
{
	UnloadMasterBanks();
	LoadMasterBanks();
}

//////////////////////////////////////////////////////////////////////////
void CAudioImpl_fmod::SetLanguage(char const* const szLanguage)
{
	if (szLanguage != nullptr)
	{
		m_localizedSoundBankFolder = PathUtil::GetGameFolder().c_str();
		m_localizedSoundBankFolder += CRY_NATIVE_PATH_SEPSTR;
		m_localizedSoundBankFolder += PathUtil::GetLocalizationFolder();
		m_localizedSoundBankFolder += CRY_NATIVE_PATH_SEPSTR;
		m_localizedSoundBankFolder += szLanguage;
		m_localizedSoundBankFolder += CRY_NATIVE_PATH_SEPSTR;
		m_localizedSoundBankFolder += FMOD_IMPL_DATA_ROOT;
	}
}

//////////////////////////////////////////////////////////////////////////
void CAudioImpl_fmod::CreateVersionString(CryFixedStringT<MAX_AUDIO_MISC_STRING_LENGTH>& stringOut) const
{
	// Remove the leading zeros on the upper 16 bit and inject the 2 dots between the 3 groups
	size_t const stringLength = stringOut.size();

	for (size_t i = 0; i < stringLength; ++i)
	{
		if (stringOut.c_str()[0] == '0')
		{
			stringOut.erase(0, 1);
		}
		else
		{
			if (i < 4)
			{
				stringOut.insert(4 - i, '.'); // First dot
				stringOut.insert(7 - i, '.'); // Second dot
				break;
			}
			else
			{
				// This shouldn't happen therefore clear the string and back out
				stringOut.clear();
				return;
			}
		}
	}
}

struct SFileData
{
	void*        pData;
	int unsigned fileSize;
};

//////////////////////////////////////////////////////////////////////////
FMOD_RESULT F_CALLBACK FmodFileOpenCallback(const char* szName, unsigned int* pFileSize, void** pHandle, void* pUserData)
{
	SFileData* const pFileData = static_cast<SFileData*>(pUserData);
	*pHandle = pFileData->pData;
	*pFileSize = pFileData->fileSize;

	return FMOD_OK;
}

//////////////////////////////////////////////////////////////////////////
FMOD_RESULT F_CALLBACK FmodFileCloseCallback(void* pHandle, void* pUserData)
{
	FMOD_RESULT fmodResult = FMOD_ERR_FILE_NOTFOUND;
	FILE* const pFile = static_cast<FILE*>(pHandle);

	if (gEnv->pCryPak->FClose(pFile) == 0)
	{
		fmodResult = FMOD_OK;
	}

	return fmodResult;
}

//////////////////////////////////////////////////////////////////////////
FMOD_RESULT F_CALLBACK FmodFileReadCallback(void* pHandle, void* pBuffer, unsigned int sizeBytes, unsigned int* pBytesRead, void* pUserData)
{
	FMOD_RESULT fmodResult = FMOD_ERR_FILE_NOTFOUND;
	FILE* const pFile = static_cast<FILE*>(pHandle);
	size_t const bytesRead = gEnv->pCryPak->FReadRaw(pBuffer, 1, static_cast<size_t>(sizeBytes), pFile);
	*pBytesRead = bytesRead;

	if (bytesRead != sizeBytes)
	{
		fmodResult = FMOD_ERR_FILE_EOF;
	}
	else if (bytesRead > 0)
	{
		fmodResult = FMOD_OK;
	}

	return fmodResult;
}

//////////////////////////////////////////////////////////////////////////
FMOD_RESULT F_CALLBACK FmodFileSeekCallback(void* pHandle, unsigned int pos, void* pUserData)
{
	FMOD_RESULT fmodResult = FMOD_ERR_FILE_COULDNOTSEEK;
	FILE* const pFile = static_cast<FILE*>(pHandle);

	if (gEnv->pCryPak->FSeek(pFile, static_cast<long>(pos), 0) == 0)
	{
		fmodResult = FMOD_OK;
	}

	return fmodResult;
}

//////////////////////////////////////////////////////////////////////////
bool CAudioImpl_fmod::LoadMasterBanks()
{
	FMOD_RESULT fmodResult = FMOD_ERR_UNINITIALIZED;
	CryFixedStringT<MAX_AUDIO_FILE_NAME_LENGTH> masterBankPath;
	CryFixedStringT<MAX_AUDIO_FILE_NAME_LENGTH> masterBankStringsPath;
	CryFixedStringT<MAX_AUDIO_FILE_PATH_LENGTH + MAX_AUDIO_FILE_NAME_LENGTH> search(m_regularSoundBankFolder + CRY_NATIVE_PATH_SEPSTR "*.bank");
	_finddata_t fd;
	intptr_t const handle = gEnv->pCryPak->FindFirst(search.c_str(), &fd);

	if (handle != -1)
	{
		do
		{
			masterBankStringsPath = fd.name;
			size_t const substrPos = masterBankStringsPath.find(".strings.bank");

			if (substrPos != masterBankStringsPath.npos)
			{
				masterBankPath = m_regularSoundBankFolder.c_str();
				masterBankPath += CRY_NATIVE_PATH_SEPSTR;
				masterBankPath += masterBankStringsPath.substr(0, substrPos);
				masterBankPath += ".bank";
				masterBankStringsPath.insert(0, CRY_NATIVE_PATH_SEPSTR);
				masterBankStringsPath.insert(0, m_regularSoundBankFolder.c_str());
				break;
			}

		}
		while (gEnv->pCryPak->FindNext(handle, &fd) >= 0);

		gEnv->pCryPak->FindClose(handle);
	}

	if (!masterBankPath.empty() && !masterBankStringsPath.empty())
	{
		size_t const masterBankFileSize = gEnv->pCryPak->FGetSize(masterBankPath.c_str());
		CRY_ASSERT(masterBankFileSize > 0);
		size_t const masterBankStringsFileSize = gEnv->pCryPak->FGetSize(masterBankStringsPath.c_str());
		CRY_ASSERT(masterBankStringsFileSize > 0);

		if (masterBankFileSize > 0 && masterBankStringsFileSize > 0)
		{
			FILE* const pMasterBank = gEnv->pCryPak->FOpen(masterBankPath.c_str(), "rbx", ICryPak::FOPEN_HINT_DIRECT_OPERATION);
			FILE* const pStringsBank = gEnv->pCryPak->FOpen(masterBankStringsPath.c_str(), "rbx", ICryPak::FOPEN_HINT_DIRECT_OPERATION);

			SFileData fileData;
			fileData.pData = static_cast<void*>(pMasterBank);
			fileData.fileSize = static_cast<int>(masterBankFileSize);

			FMOD_STUDIO_BANK_INFO bankInfo;
			ZeroStruct(bankInfo);
			bankInfo.closeCallback = &FmodFileCloseCallback;
			bankInfo.openCallback = &FmodFileOpenCallback;
			bankInfo.readCallback = &FmodFileReadCallback;
			bankInfo.seekCallback = &FmodFileSeekCallback;
			bankInfo.size = sizeof(bankInfo);
			bankInfo.userData = static_cast<void*>(&fileData);
			bankInfo.userDataLength = sizeof(SFileData);

			fmodResult = m_pSystem->loadBankCustom(&bankInfo, FMOD_STUDIO_LOAD_BANK_NORMAL, &m_pMasterBank);
			ASSERT_FMOD_OK;

			fileData.pData = static_cast<void*>(pStringsBank);
			fileData.fileSize = static_cast<int>(masterBankStringsFileSize);
			fmodResult = m_pSystem->loadBankCustom(&bankInfo, FMOD_STUDIO_LOAD_BANK_NORMAL, &m_pStringsBank);
			ASSERT_FMOD_OK;

			if (m_pMasterBank != nullptr)
			{
				int numBuses = 0;
				fmodResult = m_pMasterBank->getBusCount(&numBuses);
				ASSERT_FMOD_OK;

				if (numBuses > 0)
				{
					FMOD::Studio::Bus** pBuses = nullptr;
					POOL_NEW_CUSTOM(FMOD::Studio::Bus*, pBuses, numBuses);
					int numRetrievedBuses = 0;
					fmodResult = m_pMasterBank->getBusList(pBuses, numBuses, &numRetrievedBuses);
					ASSERT_FMOD_OK;
					CRY_ASSERT(numBuses == numRetrievedBuses);

					for (int i = 0; i < numRetrievedBuses; ++i)
					{
						fmodResult = pBuses[i]->lockChannelGroup();
						ASSERT_FMOD_OK;
					}

					POOL_FREE(pBuses);
				}
			}
		}
	}
	else
	{
		// This does not qualify for a fallback to the NULL implementation!
		// Still notify the user about this failure!
		g_audioImplLogger_fmod.Log(eAudioLogType_Error, "Fmod failed to load master banks");
		return true;
	}

	return (fmodResult == FMOD_OK);
}

//////////////////////////////////////////////////////////////////////////
void CAudioImpl_fmod::UnloadMasterBanks()
{
	FMOD_RESULT fmodResult = FMOD_ERR_UNINITIALIZED;

	if (m_pStringsBank != nullptr)
	{
		fmodResult = m_pStringsBank->unload();
		ASSERT_FMOD_OK;
		m_pStringsBank = nullptr;
	}

	if (m_pMasterBank != nullptr)
	{
		fmodResult = m_pMasterBank->unload();
		ASSERT_FMOD_OK;
		m_pMasterBank = nullptr;
	}
}

//////////////////////////////////////////////////////////////////////////
EAudioRequestStatus CAudioImpl_fmod::MuteMasterBus(bool const bMute)
{
	FMOD::Studio::Bus* pMasterBus = nullptr;
	FMOD_RESULT fmodResult = m_pSystem->getBus(s_szFmodBusPrefix, &pMasterBus);
	ASSERT_FMOD_OK;

	if (pMasterBus != nullptr)
	{
		fmodResult = pMasterBus->setMute(bMute);
		ASSERT_FMOD_OK;
	}

	return (fmodResult == FMOD_OK) ? eAudioRequestStatus_Success : eAudioRequestStatus_Failure;
}

//////////////////////////////////////////////////////////////////////////
void CAudioImpl_fmod::GetAudioFileData(char const* const szFilename, SAudioFileData& audioFileData) const
{
	FMOD::Sound* pSound = nullptr;
	FMOD_CREATESOUNDEXINFO info;
	ZeroStruct(info);
	info.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
	info.decodebuffersize = 1;
	FMOD_RESULT const fmodResult = m_pLowLevelSystem->createStream(szFilename, FMOD_OPENONLY, &info, &pSound);
	ASSERT_FMOD_OK;

	if (pSound != nullptr)
	{
		unsigned int length = 0;
		pSound->getLength(&length, FMOD_TIMEUNIT_MS);
		audioFileData.duration = length / 1000.0f; // convert to seconds
	}
}
