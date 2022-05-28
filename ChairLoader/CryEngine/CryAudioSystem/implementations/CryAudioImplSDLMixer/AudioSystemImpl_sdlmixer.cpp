// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#include "stdafx.h"
#include "AudioSystemImpl_sdlmixer.h"
#include "AudioSystemImplCVars_sdlmixer.h"
#include <CrySystem/File/CryFile.h>
#include <CryString/CryPath.h>
#include "SDLMixerSoundEngineUtil.h"
#include <CryAudio/IAudioSystem.h>
#include "SDLMixerSoundEngineTypes.h"

// SDL Mixer
#include <SDL_mixer.h>

namespace CryAudio
{
namespace Impl
{

char const* const CAudioSystemImpl_sdlmixer::s_szSDLFileTag = "SDLMixerSample";
char const* const CAudioSystemImpl_sdlmixer::s_szSDLCommonAttribute = "sdl_name";
char const* const CAudioSystemImpl_sdlmixer::s_szSDLEventIdTag = "event_id";
char const* const CAudioSystemImpl_sdlmixer::s_szSDLEventTag = "SDLMixerEvent";
char const* const CAudioSystemImpl_sdlmixer::s_szSDLSoundLibraryPath = CRY_NATIVE_PATH_SEPSTR AUDIO_SYSTEM_DATA_ROOT CRY_NATIVE_PATH_SEPSTR "sdlmixer" CRY_NATIVE_PATH_SEPSTR;
char const* const CAudioSystemImpl_sdlmixer::s_szSDLEventTypeTag = "event_type";
char const* const CAudioSystemImpl_sdlmixer::s_szSDLEventPanningEnabledTag = "enable_panning";
char const* const CAudioSystemImpl_sdlmixer::s_szSDLEventAttenuationEnabledTag = "enable_distance_attenuation";
char const* const CAudioSystemImpl_sdlmixer::s_szSDLEventAttenuationMinDistanceTag = "attenuation_dist_min";
char const* const CAudioSystemImpl_sdlmixer::s_szSDLEventAttenuationMaxDistanceTag = "attenuation_dist_max";
char const* const CAudioSystemImpl_sdlmixer::s_szSDLEventVolumeTag = "volume";
char const* const CAudioSystemImpl_sdlmixer::s_szSDLEventLoopCountTag = "loop_count";

void EndEventCallback(AudioEventId nEventID)
{
	SAudioRequest oRequest;
	SAudioCallbackManagerRequestData<eAudioCallbackManagerRequestType_ReportFinishedEvent> oRequestData(nEventID, true);
	oRequest.flags = eAudioRequestFlags_ThreadSafePush;
	oRequest.pData = &oRequestData;
	gEnv->pAudioSystem->PushRequest(oRequest);
}

CAudioSystemImpl_sdlmixer::CAudioSystemImpl_sdlmixer()
{
	m_sGameFolder = PathUtil::GetGameFolder();
	if (m_sGameFolder.empty())
	{
		CryFatalError("<Audio - SDLMixer>: Needs a valid game folder to proceed!");
	}

#if defined(INCLUDE_SDLMIXER_IMPL_PRODUCTION_CODE)
	m_sFullImplString = "SDL Mixer 2.0.1 (";
	m_sFullImplString += m_sGameFolder + PathUtil::RemoveSlash(s_szSDLSoundLibraryPath) + ")";
#endif // INCLUDE_SDLMIXER_IMPL_PRODUCTION_CODE

#if CRY_PLATFORM_WINDOWS
	m_nMemoryAlignment = 16;
#elif CRY_PLATFORM_MAC
	m_nMemoryAlignment = 16;
#elif CRY_PLATFORM_LINUX
	m_nMemoryAlignment = 16;
#elif CRY_PLATFORM_IOS
	m_nMemoryAlignment = 16;
#elif CRY_PLATFORM_ANDROID
	m_nMemoryAlignment = 16;
#else
	#error "Undefined platform."
#endif
}

CAudioSystemImpl_sdlmixer::~CAudioSystemImpl_sdlmixer()
{
}

void CAudioSystemImpl_sdlmixer::Update(float const deltaTime)
{
	SDLMixer::SoundEngine::Update();
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::Init()
{
	if (SDLMixer::SoundEngine::Init())
	{
		SDLMixer::SoundEngine::RegisterEventFinishedCallback(EndEventCallback);
		return eAudioRequestStatus_Success;
	}

	return eAudioRequestStatus_Failure;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::ShutDown()
{
	return eAudioRequestStatus_Success;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::Release()
{
	SDLMixer::SoundEngine::Release();
	POOL_FREE(this);

	// Freeing Memory Pool Memory again
	uint8* pMemSystem = g_audioImplMemoryPool_sdlmixer.Data();
	g_audioImplMemoryPool_sdlmixer.UnInitMem();

	if (pMemSystem)
	{
		delete[](uint8*)(pMemSystem);
	}

	g_audioImplCVars_sdlmixer.UnregisterVariables();

	return eAudioRequestStatus_Success;
}

void CAudioSystemImpl_sdlmixer::OnAudioSystemRefresh()
{
	SDLMixer::SoundEngine::Refresh();
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::OnLoseFocus()
{
	SDLMixer::SoundEngine::Pause();
	return eAudioRequestStatus_Success;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::OnGetFocus()
{
	SDLMixer::SoundEngine::Resume();
	return eAudioRequestStatus_Success;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::MuteAll()
{
	SDLMixer::SoundEngine::Mute();
	return eAudioRequestStatus_Success;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::UnmuteAll()
{
	SDLMixer::SoundEngine::UnMute();
	return eAudioRequestStatus_Success;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::StopAllSounds()
{
	SDLMixer::SoundEngine::Stop();
	return eAudioRequestStatus_Success;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::RegisterAudioObject(IAudioObject* const pAudioObject, char const* const szAudioObjectName)
{
	SATLAudioObjectData_sdlmixer* const pSDLMixerObject = static_cast<SATLAudioObjectData_sdlmixer* const>(pAudioObject);

	if (pSDLMixerObject)
	{
		SDLMixer::SoundEngine::RegisterAudioObject(pSDLMixerObject);
#if defined(INCLUDE_SDLMIXER_IMPL_PRODUCTION_CODE)
		m_idToName[pSDLMixerObject->nObjectID] = szAudioObjectName;
#endif
		return eAudioRequestStatus_Success;
	}
	return eAudioRequestStatus_Failure;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::RegisterAudioObject(IAudioObject* const pAudioObject)
{
	SATLAudioObjectData_sdlmixer* const pSDLMixerObject = static_cast<SATLAudioObjectData_sdlmixer* const>(pAudioObject);
	if (pSDLMixerObject)
	{
		SDLMixer::SoundEngine::RegisterAudioObject(pSDLMixerObject);
		return eAudioRequestStatus_Success;
	}
	return eAudioRequestStatus_Failure;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::UnregisterAudioObject(IAudioObject* const pAudioObject)
{
	SATLAudioObjectData_sdlmixer* const pSDLMixerObject = static_cast<SATLAudioObjectData_sdlmixer* const>(pAudioObject);
	if (pSDLMixerObject)
	{
		SDLMixer::SoundEngine::UnregisterAudioObject(pSDLMixerObject);
		return eAudioRequestStatus_Success;
	}
	return eAudioRequestStatus_Failure;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::ResetAudioObject(IAudioObject* const pAudioObject)
{
	return eAudioRequestStatus_Success;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::UpdateAudioObject(IAudioObject* const pAudioObject)
{
	return eAudioRequestStatus_Success;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::PlayFile(SAudioStandaloneFileInfo* const _pAudioStandaloneFileInfo)
{
	EAudioRequestStatus result = eAudioRequestStatus_Failure;
	SATLAudioObjectData_sdlmixer const* const pAudioObject = static_cast<SATLAudioObjectData_sdlmixer* const>(_pAudioStandaloneFileInfo->pAudioObject);

	return eAudioRequestStatus_Success;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::StopFile(SAudioStandaloneFileInfo* const _pAudioStandaloneFileInfo)
{
	return eAudioRequestStatus_Success;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::PrepareTriggerSync(IAudioObject* const pAudioObject, IAudioTrigger const* const pAudioTrigger)
{
	return eAudioRequestStatus_Success;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::UnprepareTriggerSync(IAudioObject* const pAudioObject, IAudioTrigger const* const pAudioTrigger)
{
	return eAudioRequestStatus_Success;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::PrepareTriggerAsync(IAudioObject* const pAudioObject, IAudioTrigger const* const pAudioTrigger, IAudioEvent* const pAudioEvent)
{
	return eAudioRequestStatus_Success;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::UnprepareTriggerAsync(IAudioObject* const pAudioObject, IAudioTrigger const* const pAudioTrigger, IAudioEvent* const pAudioEvent)
{
	return eAudioRequestStatus_Success;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::ActivateTrigger(IAudioObject* const pAudioObject, IAudioTrigger const* const pAudioTrigger, IAudioEvent* const pAudioEvent)
{
	EAudioRequestStatus eResult = eAudioRequestStatus_Failure;

	SATLAudioObjectData_sdlmixer* const pSDLAudioObjectData = static_cast<SATLAudioObjectData_sdlmixer* const>(pAudioObject);
	SATLTriggerImplData_sdlmixer const* const pSDLEventStaticData = static_cast<SATLTriggerImplData_sdlmixer const* const>(pAudioTrigger);
	SATLEventData_sdlmixer* const pSDLEventInstanceData = static_cast<SATLEventData_sdlmixer* const>(pAudioEvent);

	if ((pSDLAudioObjectData != nullptr) && (pSDLEventStaticData != nullptr) && (pSDLEventInstanceData != nullptr))
	{
		if (SDLMixer::SoundEngine::ExecuteEvent(pSDLAudioObjectData, pSDLEventStaticData, pSDLEventInstanceData))
		{
			eResult = eAudioRequestStatus_Success;
		}
	}
	return eResult;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::StopEvent(IAudioObject* const pAudioObject, IAudioEvent const* const pAudioEvent)
{
	EAudioRequestStatus eResult = eAudioRequestStatus_Failure;
	SATLEventData_sdlmixer const* const pSDLEventInstanceData = static_cast<SATLEventData_sdlmixer const* const>(pAudioEvent);

	if (pSDLEventInstanceData != nullptr)
	{
		if (SDLMixer::SoundEngine::StopEvent(pSDLEventInstanceData))
		{
			eResult = eAudioRequestStatus_Success;
		}
	}
	return eResult;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::StopAllEvents(IAudioObject* const pAudioObject)
{
	return eAudioRequestStatus_Success;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::Set3DAttributes(IAudioObject* const pAudioObject, SAudioObject3DAttributes const& attributes)
{
	SATLAudioObjectData_sdlmixer* const pSDLMixerObject = static_cast<SATLAudioObjectData_sdlmixer* const>(pAudioObject);
	if (pSDLMixerObject)
	{
		SDLMixer::SoundEngine::SetAudioObjectPosition(pSDLMixerObject, attributes.transformation);
		return eAudioRequestStatus_Success;
	}
	return eAudioRequestStatus_Failure;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::SetRtpc(IAudioObject* const pAudioObject, IAudioRtpc const* const pAudioRtpc, float const value)
{
	return eAudioRequestStatus_Success;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::SetSwitchState(IAudioObject* const pAudioObject, IAudioSwitchState const* const pAudioSwitchState)
{
	return eAudioRequestStatus_Success;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::SetObstructionOcclusion(IAudioObject* const pAudioObject, float const obstruction, float const occlusion)
{
	return eAudioRequestStatus_Success;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::SetEnvironment(IAudioObject* const pAudioObject, IAudioEnvironment const* const pAudioEnvironment, float const amount)
{
	return eAudioRequestStatus_Success;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::SetListener3DAttributes(IAudioListener* const pAudioListener, SAudioObject3DAttributes const& attributes)
{
	SATLListenerData_sdlmixer* const pListener = static_cast<SATLListenerData_sdlmixer* const>(pAudioListener);
	if (pListener)
	{
		SDLMixer::SoundEngine::SetListenerPosition(pListener->nListenerID, attributes.transformation);
		return eAudioRequestStatus_Success;
	}
	return eAudioRequestStatus_Failure;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::RegisterInMemoryFile(SAudioFileEntryInfo* const pAudioFileEntry)
{
	EAudioRequestStatus eResult = eAudioRequestStatus_Failure;

	if (pAudioFileEntry != nullptr)
	{
		SATLAudioFileEntryData_sdlmixer* const pFileData = static_cast<SATLAudioFileEntryData_sdlmixer*>(pAudioFileEntry->pImplData);

		if (pFileData != nullptr)
		{
			pFileData->nSampleID = SDLMixer::SoundEngine::LoadSample(pAudioFileEntry->pFileData, pAudioFileEntry->size, pAudioFileEntry->szFileName);
			eResult = eAudioRequestStatus_Success;
		}
		else
		{
			g_audioImplLogger_sdlmixer.Log(eAudioLogType_Error, "Invalid AudioFileEntryData passed to the SDL Mixer implementation of RegisterInMemoryFile");
		}
	}

	return eResult;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::UnregisterInMemoryFile(SAudioFileEntryInfo* const pAudioFileEntry)
{
	EAudioRequestStatus eResult = eAudioRequestStatus_Failure;

	if (pAudioFileEntry != nullptr)
	{
		SATLAudioFileEntryData_sdlmixer* const pFileData = static_cast<SATLAudioFileEntryData_sdlmixer*>(pAudioFileEntry->pImplData);

		if (pFileData != nullptr)
		{
			SDLMixer::SoundEngine::UnloadSample(pFileData->nSampleID);
			eResult = eAudioRequestStatus_Success;
		}
		else
		{
			g_audioImplLogger_sdlmixer.Log(eAudioLogType_Error, "Invalid AudioFileEntryData passed to the SDL Mixer implementation of UnregisterInMemoryFile");
		}
	}

	return eResult;
}

EAudioRequestStatus CAudioSystemImpl_sdlmixer::ParseAudioFileEntry(XmlNodeRef const pAudioFileEntryNode, SAudioFileEntryInfo* const pFileEntryInfo)
{
	EAudioRequestStatus eResult = eAudioRequestStatus_Failure;

	if ((_stricmp(pAudioFileEntryNode->getTag(), s_szSDLFileTag) == 0) && (pFileEntryInfo != nullptr))
	{
		char const* const sFileName = pAudioFileEntryNode->getAttr(s_szSDLCommonAttribute);

		// Currently the SDLMixer Implementation does not support localized files.
		pFileEntryInfo->bLocalized = false;

		if (sFileName != nullptr && sFileName[0] != '\0')
		{
			pFileEntryInfo->szFileName = sFileName;
			pFileEntryInfo->memoryBlockAlignment = m_nMemoryAlignment;
			POOL_NEW(SATLAudioFileEntryData_sdlmixer, pFileEntryInfo->pImplData);

			eResult = eAudioRequestStatus_Success;
		}
		else
		{
			pFileEntryInfo->szFileName = nullptr;
			pFileEntryInfo->memoryBlockAlignment = 0;
			pFileEntryInfo->pImplData = nullptr;
		}
	}

	return eResult;
}

void CAudioSystemImpl_sdlmixer::DeleteAudioFileEntry(IAudioFileEntry* const pOldAudioFileEntry)
{
	POOL_FREE(pOldAudioFileEntry);
}

char const* const CAudioSystemImpl_sdlmixer::GetAudioFileLocation(SAudioFileEntryInfo* const pFileEntryInfo)
{
	static CryFixedStringT<MAX_AUDIO_FILE_PATH_LENGTH> sPath;
	sPath = m_sGameFolder.c_str();
	sPath += s_szSDLSoundLibraryPath;

	return sPath.c_str();
}

IAudioTrigger const* CAudioSystemImpl_sdlmixer::NewAudioTrigger(XmlNodeRef const pAudioTriggerNode, SAudioTriggerInfo& info)
{
	SATLTriggerImplData_sdlmixer* pNewTriggerImpl = nullptr;
	if (_stricmp(pAudioTriggerNode->getTag(), s_szSDLEventTag) == 0)
	{
		pNewTriggerImpl = SDLMixer::SoundEngine::CreateEventData();
		if (pNewTriggerImpl)
		{
			char const* const szFileName = pAudioTriggerNode->getAttr(s_szSDLCommonAttribute);
			pNewTriggerImpl->nSampleID = SDLMixer::GetIDFromString(szFileName);

			pNewTriggerImpl->bStartEvent = (_stricmp(pAudioTriggerNode->getAttr(s_szSDLEventTypeTag), "stop") != 0);
			if (pNewTriggerImpl->bStartEvent)
			{
				pNewTriggerImpl->bPanningEnabled = (_stricmp(pAudioTriggerNode->getAttr(s_szSDLEventPanningEnabledTag), "true") == 0);
				bool bAttenuationEnabled = (_stricmp(pAudioTriggerNode->getAttr(s_szSDLEventAttenuationEnabledTag), "true") == 0);
				if (bAttenuationEnabled)
				{
					pAudioTriggerNode->getAttr(s_szSDLEventAttenuationMinDistanceTag, pNewTriggerImpl->fAttenuationMinDistance);
					pAudioTriggerNode->getAttr(s_szSDLEventAttenuationMaxDistanceTag, pNewTriggerImpl->fAttenuationMaxDistance);
				}
				else
				{
					pNewTriggerImpl->fAttenuationMinDistance = -1.0f;
					pNewTriggerImpl->fAttenuationMaxDistance = -1.0f;
				}

				info.maxRadius = pNewTriggerImpl->fAttenuationMaxDistance;

				// Translate decibel to normalized value.
				static const int nMaxVolume = 128;
				float fVolume = 0.0f;
				pAudioTriggerNode->getAttr(s_szSDLEventVolumeTag, fVolume);
				pNewTriggerImpl->nVolume = static_cast<int>(pow_tpl(10.0f, fVolume / 20.0f) * nMaxVolume);

				pAudioTriggerNode->getAttr(s_szSDLEventLoopCountTag, pNewTriggerImpl->nLoopCount);
			}
		}
	}

	return pNewTriggerImpl;
}

void CAudioSystemImpl_sdlmixer::DeleteAudioTrigger(IAudioTrigger const* const pOldAudioTrigger)
{
	SATLTriggerImplData_sdlmixer const* const pSDLEventStaticData = static_cast<SATLTriggerImplData_sdlmixer const* const>(pOldAudioTrigger);
	if (pSDLEventStaticData)
	{
		SDLMixer::SoundEngine::StopTrigger(pSDLEventStaticData);
	}
	POOL_FREE_CONST(pOldAudioTrigger);
}

IAudioRtpc const* CAudioSystemImpl_sdlmixer::NewAudioRtpc(XmlNodeRef const pAudioRtpcNode)
{
	SATLRtpcImplData_sdlmixer* pNewRtpcImpl = nullptr;
	POOL_NEW(SATLRtpcImplData_sdlmixer, pNewRtpcImpl)();
	return pNewRtpcImpl;
}

void CAudioSystemImpl_sdlmixer::DeleteAudioRtpc(IAudioRtpc const* const pOldAudioRtpc)
{
	POOL_FREE_CONST(pOldAudioRtpc);
}

IAudioSwitchState const* CAudioSystemImpl_sdlmixer::NewAudioSwitchState(XmlNodeRef const pAudioSwitchStateNode)
{
	SATLSwitchStateImplData_sdlmixer* pNewSwitchImpl = nullptr;
	POOL_NEW(SATLSwitchStateImplData_sdlmixer, pNewSwitchImpl)();
	return pNewSwitchImpl;
}

void CAudioSystemImpl_sdlmixer::DeleteAudioSwitchState(IAudioSwitchState const* const pOldAudioSwitchState)
{
	POOL_FREE_CONST(pOldAudioSwitchState);
}

IAudioEnvironment const* CAudioSystemImpl_sdlmixer::NewAudioEnvironment(XmlNodeRef const pAudioEnvironmentNode)
{
	SATLEnvironmentImplData_sdlmixer* pNewEnvironmentImpl = nullptr;
	POOL_NEW(SATLEnvironmentImplData_sdlmixer, pNewEnvironmentImpl)();
	return pNewEnvironmentImpl;
}

void CAudioSystemImpl_sdlmixer::DeleteAudioEnvironment(IAudioEnvironment const* const pOldAudioEnvironment)
{
	POOL_FREE_CONST(pOldAudioEnvironment);
}

IAudioObject* CAudioSystemImpl_sdlmixer::NewGlobalAudioObject(AudioObjectId const audioObjectID)
{
	POOL_NEW_CREATE(SATLAudioObjectData_sdlmixer, pNewObject)(audioObjectID, true);
	return pNewObject;
}

IAudioObject* CAudioSystemImpl_sdlmixer::NewAudioObject(AudioObjectId const audioObjectID)
{
	POOL_NEW_CREATE(SATLAudioObjectData_sdlmixer, pNewObject)(audioObjectID, false);
	return pNewObject;
}

void CAudioSystemImpl_sdlmixer::DeleteAudioObject(IAudioObject* const pOldObjectData)
{
	POOL_FREE(pOldObjectData);
}

IAudioListener* CAudioSystemImpl_sdlmixer::NewDefaultAudioListener(AudioObjectId const audioObjectId)
{
	POOL_NEW_CREATE(SATLListenerData_sdlmixer, pNewObject)(0);
	return pNewObject;
}

IAudioListener* CAudioSystemImpl_sdlmixer::NewAudioListener(AudioObjectId const audioObjectId)
{
	POOL_NEW_CREATE(SATLListenerData_sdlmixer, pNewObject)(audioObjectId);
	return pNewObject;
}

void CAudioSystemImpl_sdlmixer::DeleteAudioListener(IAudioListener* const pOldAudioListener)
{
	POOL_FREE(pOldAudioListener);
}

IAudioEvent* CAudioSystemImpl_sdlmixer::NewAudioEvent(AudioEventId const audioEventID)
{
	POOL_NEW_CREATE(SATLEventData_sdlmixer, pNewEvent)(audioEventID);
	return pNewEvent;
}

void CAudioSystemImpl_sdlmixer::DeleteAudioEvent(IAudioEvent* const pOldAudioEvent)
{
	POOL_FREE(pOldAudioEvent);
}

void CAudioSystemImpl_sdlmixer::ResetAudioEvent(IAudioEvent* const pAudioEvent)
{
	SATLEventData_sdlmixer* const pEventInstanceData = static_cast<SATLEventData_sdlmixer*>(pAudioEvent);

	if (pEventInstanceData != nullptr)
	{
		pEventInstanceData->Reset();
	}
}

IAudioStandaloneFile* CAudioSystemImpl_sdlmixer::NewAudioStandaloneFile()
{
	POOL_NEW_CREATE(SATLAudioStandaloneFile_sdlmixer, pAudioStandaloneFile);
	return pAudioStandaloneFile;
}

void CAudioSystemImpl_sdlmixer::DeleteAudioStandaloneFile(IAudioStandaloneFile const* const _pOldAudioStandaloneFile)
{
	POOL_FREE_CONST(_pOldAudioStandaloneFile);
}

void CAudioSystemImpl_sdlmixer::ResetAudioStandaloneFile(IAudioStandaloneFile* const _pAudioStandaloneFile)
{
}

void CAudioSystemImpl_sdlmixer::GamepadConnected(TAudioGamepadUniqueID const deviceUniqueID)
{
}

void CAudioSystemImpl_sdlmixer::GamepadDisconnected(TAudioGamepadUniqueID const deviceUniqueID)
{
}

void CAudioSystemImpl_sdlmixer::SetLanguage(char const* const szLanguage)
{
	//TODO
}

char const* const CAudioSystemImpl_sdlmixer::GetImplementationNameString() const
{
#if defined(INCLUDE_SDLMIXER_IMPL_PRODUCTION_CODE)
	return m_sFullImplString.c_str();
#endif // INCLUDE_SDLMIXER_IMPL_PRODUCTION_CODE
	return nullptr;
}

void CAudioSystemImpl_sdlmixer::GetMemoryInfo(SAudioImplMemoryInfo& memoryInfo) const
{
	memoryInfo.primaryPoolSize = g_audioImplMemoryPool_sdlmixer.MemSize();
	memoryInfo.primaryPoolUsedSize = memoryInfo.primaryPoolSize - g_audioImplMemoryPool_sdlmixer.MemFree();
	memoryInfo.primaryPoolAllocations = g_audioImplMemoryPool_sdlmixer.FragmentCount();
	memoryInfo.bucketUsedSize = g_audioImplMemoryPool_sdlmixer.GetSmallAllocsSize();
	memoryInfo.bucketAllocations = g_audioImplMemoryPool_sdlmixer.GetSmallAllocsCount();
	memoryInfo.secondaryPoolSize = 0;
	memoryInfo.secondaryPoolUsedSize = 0;
	memoryInfo.secondaryPoolAllocations = 0;
}

void CAudioSystemImpl_sdlmixer::GetAudioFileData(char const* const szFilename, SAudioFileData& audioFileData) const
{}

}
}
