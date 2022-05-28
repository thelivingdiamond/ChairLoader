// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

/************************************************************************

   This manager handles all dynamic Responses. He is responsible for loading them from file,
   Handling of Signals, Checking the rules, find the correct response

   /************************************************************************/

#pragma once

#include "Response.h"
#include <CryString/CryString.h>

namespace CryDRS
{
class CVariableCollection;
class CResponseActor;
typedef std::shared_ptr<CVariableCollection> VariableCollectionSharedPtr;

struct SSignal
{
	SSignal(const CHashedString& signalName, CResponseActor* pSender, VariableCollectionSharedPtr pSignalContext);

	DRS::SignalId               m_id;
	CHashedString               m_signalName;
	CResponseActor*             m_pSender;
	VariableCollectionSharedPtr m_pSignalContext;
};

typedef std::shared_ptr<CResponse> ResponsePtr;

//--------------------------------------------------------------------------------------------------

class CResponseManager final : public DRS::IResponseManager
{
public:
	static string s_currentSignal;

	enum EUsedFileFormat
	{
		eUFF_JSON = 1,
		eUFF_XML  = 2,
		eUFF_BIN  = 3
	};

	enum ESerializeHint
	{
		eSH_ReplaceAllExisting        = BIT(0), //will remove existing ones
		eSH_CollapsedResponseSegments = BIT(1), //will only display responseSegments hierarchy (editor only)
		eSH_EvaluateResponses         = BIT(2), //will display if the condition is currently met (editor only)
	};

	typedef std::unordered_map<CHashedString, ResponsePtr>                           MappedSignals;
	typedef std::vector<SSignal>                                                     SignalList;
	typedef std::vector<CResponseInstance*>                                          ResponseInstanceList;
	typedef std::vector<std::pair<DRS::IResponseManager::IListener*, DRS::SignalId>> ListenerList;

	CResponseManager();
	~CResponseManager();

	//////////////////////////////////////////////////////////
	// IResponseManager implementation
	virtual bool                  AddListener(DRS::IResponseManager::IListener* pNewListener, DRS::SignalId signalID = DRS::s_InvalidSignalId) override;
	virtual bool                  RemoveListener(DRS::IResponseManager::IListener* pListenerToRemove) override;

	virtual DynArray<const char*> GetRecentSignals(DRS::IResponseManager::eSignalFilter filter = DRS::IResponseManager::eSF_All) override;

	virtual void                  SerializeResponse(Serialization::IArchive& ar, const DynArray<stack_string>& signalNames, DRS::IResponseActor* pActorForEvaluation = nullptr) override;
	virtual void                  SerializeRecentResponse(Serialization::IArchive& ar, const DynArray<stack_string>& signalNames, int maxElemets = -1) override;
	virtual void                  SerializeVariableChanges(Serialization::IArchive& ar, const stack_string& variableName = "ALL", int maxElemets = -1) override;

	virtual bool                  AddResponse(const stack_string& signalName) override;
	virtual bool                  RemoveResponse(const stack_string& signalName) override;
	//////////////////////////////////////////////////////////////////////////

	//will load all DynamicResponse Definitions from the folder
	void               SetFileFormat(EUsedFileFormat format);
	bool               LoadFromFiles(const char* szDataPath);
	bool               SaveToFiles(const char* szDataPath);
	void               SerializeResponseStates(Serialization::IArchive& ar);

	ResponsePtr        GetResponse(const CHashedString& signalName);

	void               QueueSignal(const SSignal& signal);
	void               CancelSignalProcessing(const SSignal& signal);
	void               Update();

	CResponseInstance* CreateInstance(SSignal& signal, CResponse* pResponse);
	void               ReleaseInstance(CResponseInstance* pInstance, bool removeFromRunningInstances = true);
	void               Reset(bool bResetExecutionCounter);
	void               Serialize(Serialization::IArchive& ar);

private:
	bool _LoadFromFiles(const string& dataPath);

	void InformListenerAboutSignalProcessingStarted(const SSignal& signal, DRS::IResponseInstance* pInstance);
	void InformListenerAboutSignalProcessingFinished(const CHashedString& signalName, CResponseActor* pSender, const VariableCollectionSharedPtr& pSignalContext, const DRS::SignalId signalID, DRS::IResponseInstance* pInstance, DRS::IResponseManager::IListener::eProcessingResult outcome);

	EUsedFileFormat      m_UsedFileFormat;

	MappedSignals        m_mappedSignals;

	ResponseInstanceList m_runningResponses;

	bool                 m_bCurrentlyUpdating;

	ListenerList         m_Listener;

	SignalList           m_currentlyQueuedSignals;              //make this a lockfree list so that queuing signals is thread safe
	SignalList           m_currentlyQueuedSignalsDuringUpdate;  //we have a second list where we store signals that gets queued while we are updating the normal list (because we dont want the original list to change, while we use it)
	SignalList           m_currentlySignalsWaitingToBeCanceled; //make this a lockfree list so that queuing signals is thread safe
};
} // namespace CryDRS
