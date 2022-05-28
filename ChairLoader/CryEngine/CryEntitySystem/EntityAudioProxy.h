// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

#include <CryAudio/IAudioSystem.h>
#include <CryMemory/STLPoolAllocator.h>

//////////////////////////////////////////////////////////////////////////
// Description:
//    Handles audio on the entity.
//////////////////////////////////////////////////////////////////////////
struct CEntityAudioProxy : public IEntityAudioProxy
{
public:

	CEntityAudioProxy();
	virtual ~CEntityAudioProxy();

	// IComponent
	virtual void ProcessEvent(SEntityEvent& event);
	virtual void Initialize(SComponentInitializer const& init);
	// ~IComponent

	// IEntityProxy
	virtual EEntityProxy GetType() { return ENTITY_PROXY_AUDIO; }
	virtual void         Release();
	virtual void         Done();
	virtual void         Update(SEntityUpdateContext& ctx)                   {}
	virtual bool         Init(IEntity* pEntity, SEntitySpawnParams& params)  { return true; }
	virtual void         Reload(IEntity* pEntity, SEntitySpawnParams& params);
	virtual void         SerializeXML(XmlNodeRef& entityNode, bool bLoading) {}
	virtual void         Serialize(TSerialize ser);
	virtual bool         NeedSerialize()                                     { return false; }
	virtual bool         GetSignature(TSerialize signature);
	virtual void         GetMemoryUsage(ICrySizer* pSizer) const             { pSizer->AddObject(this, sizeof(*this)); }
	// ~IEntityProxy

	// IEntityAudioProxy
	virtual void               SetFadeDistance(float const fadeDistance)                       { m_fadeDistance = fadeDistance; }
	virtual float              GetFadeDistance() const                                         { return m_fadeDistance; }
	virtual void               SetEnvironmentFadeDistance(float const environmentFadeDistance) { m_environmentFadeDistance = environmentFadeDistance; }
	virtual float              GetEnvironmentFadeDistance() const                              { return m_environmentFadeDistance; }
	virtual void               SetEnvironmentId(AudioEnvironmentId const environmentId)        { m_audioEnvironmentId = environmentId; }
	virtual AudioEnvironmentId GetEnvironmentID() const                                        { return m_audioEnvironmentId; }
	virtual AudioProxyId       CreateAuxAudioProxy();
	virtual bool               RemoveAuxAudioProxy(AudioProxyId const audioProxyId);
	virtual void               SetAuxAudioProxyOffset(Matrix34 const& offset, AudioProxyId const audioProxyId = DEFAULT_AUDIO_PROXY_ID);
	virtual Matrix34 const&    GetAuxAudioProxyOffset(AudioProxyId const audioProxyId = DEFAULT_AUDIO_PROXY_ID);
	virtual void               PlayFile(char const* const _szFile, AudioProxyId const _audioProxyId = DEFAULT_AUDIO_PROXY_ID, SAudioCallBackInfo const& _callBackInfo = SAudioCallBackInfo::GetEmptyObject());
	virtual void               StopFile(char const* const _szFile, AudioProxyId const _audioProxyId = DEFAULT_AUDIO_PROXY_ID);
	virtual bool               ExecuteTrigger(AudioControlId const audioTriggerId, AudioProxyId const audioProxyId = DEFAULT_AUDIO_PROXY_ID, SAudioCallBackInfo const& callBackInfo = SAudioCallBackInfo::GetEmptyObject());
	virtual void               StopTrigger(AudioControlId const audioTriggerId, AudioProxyId const audioProxyId = DEFAULT_AUDIO_PROXY_ID);
	virtual void               SetSwitchState(AudioControlId const audioSwitchId, AudioSwitchStateId const audioStateId, AudioProxyId const audioProxyId = DEFAULT_AUDIO_PROXY_ID);
	virtual void               SetRtpcValue(AudioControlId const audioRtpcId, float const value, AudioProxyId const audioProxyId = DEFAULT_AUDIO_PROXY_ID);
	virtual void               SetObstructionCalcType(EAudioOcclusionType const occlusionType, AudioProxyId const audioProxyId = DEFAULT_AUDIO_PROXY_ID);
	virtual void               SetEnvironmentAmount(AudioEnvironmentId const audioEnvironmentId, float const amount, AudioProxyId const audioProxyId = DEFAULT_AUDIO_PROXY_ID);
	virtual void               SetCurrentEnvironments(AudioProxyId const audioProxyId = DEFAULT_AUDIO_PROXY_ID);
	virtual void               AuxAudioProxiesMoveWithEntity(bool const bCanMoveWithEntity);
	virtual void               AddAsListenerToAuxAudioProxy(AudioProxyId const audioProxyId, void (* func)(SAudioRequestInfo const* const), EAudioRequestType requestType = eAudioRequestType_AudioAllRequests, AudioEnumFlagsType specificRequestMask = ALL_AUDIO_REQUEST_SPECIFIC_TYPE_FLAGS);
	virtual void               RemoveAsListenerFromAuxAudioProxy(AudioProxyId const audioProxyId, void (* func)(SAudioRequestInfo const* const));
	// ~IEntityAudioProxy

private:

	enum EEntityAudioProxyFlags : AudioEnumFlagsType
	{
		eEAPF_NONE                 = 0,
		eEAPF_CAN_MOVE_WITH_ENTITY = BIT(0),
	};

	struct SAudioProxyWrapper
	{
		SAudioProxyWrapper(IAudioProxy* const _pIAudioProxy)
			: pIAudioProxy(_pIAudioProxy)
			, offset(IDENTITY)
		{}

		~SAudioProxyWrapper() {}

		IAudioProxy* const pIAudioProxy;
		Matrix34           offset;

	private:

		SAudioProxyWrapper()
			: pIAudioProxy(NULL)
			, offset(IDENTITY)
		{}
	};

	typedef std::pair<AudioProxyId const, SAudioProxyWrapper>                                                                 TAudioProxyPair;
	typedef std::map<AudioProxyId const, SAudioProxyWrapper, std::less<AudioProxyId>, stl::STLPoolAllocator<TAudioProxyPair>> TAuxAudioProxies;
	static TAudioProxyPair            s_nullAudioProxyPair;
	static CAudioObjectTransformation s_audioListenerLastTransformation;

	void             OnListenerEnter(IEntity const* const pEntity);
	void             OnListenerMoveNear(Vec3 const& closestPointToArea);
	void             OnListenerMoveInside(Vec3 const& listenerPos);
	void             OnListenerExclusiveMoveInside(IEntity const* const __restrict pEntity, IEntity const* const __restrict pAreaHigh, IEntity const* const __restrict pAreaLow, float const fade);
	void             OnMove();
	void             OnEnter(IEntity const* const pIEntity);
	void             OnLeaveNear(IEntity const* const pIEntity);
	void             OnMoveNear(IEntity const* const pIEntity, float const amount);
	void             OnExclusiveMoveInside(IEntity const* const __restrict pEntity, IEntity const* const __restrict pEntityAreaHigh, IEntity const* const __restrict pEntityAreaLow, float const fade);
	void             OnAreaCrossing(IEntity const* const pIEntity);
	TAudioProxyPair& GetAuxAudioProxyPair(AudioProxyId const audioProxyId);
	void             SetEnvironmentAmountInternal(IEntity const* const pIEntity, float const amount) const;

	// Function objects
	struct SReleaseAudioProxy
	{
		SReleaseAudioProxy() {}

		inline void operator()(TAudioProxyPair const& pair)
		{
			pair.second.pIAudioProxy->Release();
		}
	};

	struct SRepositionAudioProxy
	{
		SRepositionAudioProxy(Matrix34 const& _transformation)
			: transformation(_transformation)
		{}

		inline void operator()(TAudioProxyPair const& pair)
		{
			pair.second.pIAudioProxy->SetTransformation(transformation * pair.second.offset);
		}

	private:

		Matrix34 const& transformation;
	};

	struct SPlayFile
	{
		SPlayFile(char const* const _szFile, SAudioCallBackInfo const& _callBackInfo = SAudioCallBackInfo::GetEmptyObject())
			: szFile(_szFile)
			, callBackInfo(_callBackInfo)
		{}

		inline void operator()(TAudioProxyPair const& pair)
		{
			pair.second.pIAudioProxy->PlayFile(szFile, callBackInfo);
		}

	private:

		char const* const         szFile;
		SAudioCallBackInfo const& callBackInfo;
	};

	struct SStopFile
	{
		SStopFile(char const* const _szFile)
			: szFile(_szFile)
		{}

		inline void operator()(TAudioProxyPair const& pair)
		{
			pair.second.pIAudioProxy->StopFile(szFile);
		}

	private:

		char const* const szFile;
	};

	struct SStopTrigger
	{
		SStopTrigger(AudioControlId const _audioTriggerId)
			: audioTriggerId(_audioTriggerId)
		{}

		inline void operator()(TAudioProxyPair const& pair)
		{
			pair.second.pIAudioProxy->StopTrigger(audioTriggerId);
		}

	private:

		AudioControlId const audioTriggerId;
	};

	struct SSetSwitchState
	{
		SSetSwitchState(AudioControlId const _audioSwitchId, AudioSwitchStateId const _audioStateId)
			: audioSwitchId(_audioSwitchId)
			, audioStateId(_audioStateId)
		{}

		inline void operator()(TAudioProxyPair const& pair)
		{
			pair.second.pIAudioProxy->SetSwitchState(audioSwitchId, audioStateId);
		}

	private:

		AudioControlId const     audioSwitchId;
		AudioSwitchStateId const audioStateId;
	};

	struct SSetRtpcValue
	{
		SSetRtpcValue(AudioControlId const _rtpcId, float const _value)
			: rtpcId(_rtpcId)
			, value(_value)
		{}

		inline void operator()(TAudioProxyPair const& pair)
		{
			pair.second.pIAudioProxy->SetRtpcValue(rtpcId, value);
		}

	private:

		AudioControlId const rtpcId;
		float const          value;
	};

	struct SSetOcclusionType
	{
		SSetOcclusionType(EAudioOcclusionType const _occlusionType)
			: occlusionType(_occlusionType)
		{}

		inline void operator()(TAudioProxyPair const& pair)
		{
			pair.second.pIAudioProxy->SetOcclusionType(occlusionType);
		}

	private:

		EAudioOcclusionType const occlusionType;
	};

	struct SSetEnvironmentAmount
	{
		SSetEnvironmentAmount(AudioEnvironmentId const _audioEnvironmentId, float const _amount)
			: audioEnvironmentId(_audioEnvironmentId)
			, amount(_amount)
		{}

		inline void operator()(TAudioProxyPair const& pair)
		{
			pair.second.pIAudioProxy->SetEnvironmentAmount(audioEnvironmentId, amount);
		}

	private:

		AudioEnvironmentId const audioEnvironmentId;
		float const              amount;
	};

	struct SSetCurrentEnvironments
	{
		SSetCurrentEnvironments(EntityId const entityId)
			: m_entityId(entityId)
		{}

		inline void operator()(TAudioProxyPair const& pair)
		{
			pair.second.pIAudioProxy->SetCurrentEnvironments(m_entityId);
		}

	private:

		EntityId const m_entityId;
	};

	struct SSetAuxAudioProxyOffset
	{
		SSetAuxAudioProxyOffset(Matrix34 const& _offset, Matrix34 const& _entityPosition)
			: offset(_offset)
			, entityPosition(_entityPosition)
		{}

		inline void operator()(TAudioProxyPair& pair)
		{
			pair.second.offset = offset;
			(SRepositionAudioProxy(entityPosition))(pair);
		}

	private:

		Matrix34 const& offset;
		Matrix34 const& entityPosition;
	};
	// ~Function objects

	TAuxAudioProxies   m_mapAuxAudioProxies;
	AudioProxyId       m_audioProxyIDCounter;

	AudioEnvironmentId m_audioEnvironmentId;
	CEntity*           m_pEntity;
	AudioEnumFlagsType m_flags;

	float              m_fadeDistance;
	float              m_environmentFadeDistance;
};
