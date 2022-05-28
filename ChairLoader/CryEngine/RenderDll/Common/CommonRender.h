// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#ifndef _BASERESOURCE_H_
#define _BASERESOURCE_H_

#include <CryMath/Cry_Math.h>

#include "Defs.h"
#include <CryMath/Cry_Color.h>
#include <CryMath/Cry_Camera.h>
#include <CryMemory/MemoryAccess.h>

#include <CryMemory/STLGlobalAllocator.h>

#define VSCONST_INSTDATA                0
#define VSCONST_SKINMATRIX              0
#define VSCONST_NOISE_TABLE             0
#define NUM_MAX_BONES_PER_GROUP         (250)
#define NUM_MAX_BONES_PER_GROUP_WITH_MB (125)

//////////////////////////////////////////////////////////////////////
class CRenderer;
extern CRenderer* gRenDev;

class CBaseResource;

//====================================================================

#define CR_LITTLE_ENDIAN

struct SWaveForm;

extern bool gbRgb;

inline DWORD COLCONV(DWORD clr)
{
	return ((clr & 0xff00ff00) | ((clr & 0xff0000) >> 16) | ((clr & 0xff) << 16));
}
inline void COLCONV(ColorF& col)
{
	float v = col[0];
	col[0] = col[2];
	col[2] = v;
}

inline void f2d(double* dst, float* src)
{
	for (int i = 0; i < 16; i++)
	{
		dst[i] = src[i];
	}
}

inline void d2f(float* dst, double* src)
{
	for (int i = 0; i < 16; i++)
	{
		dst[i] = (float)src[i];
	}
}

//=================================================================

typedef std::map<CCryNameTSCRC, CBaseResource*>                              ResourcesMap;

typedef ResourcesMap::iterator                                               ResourcesMapItor;

typedef std::vector<CBaseResource*, stl::STLGlobalAllocator<CBaseResource*>> ResourcesList;
typedef std::vector<int, stl::STLGlobalAllocator<int>>                       ResourceIds;

struct SResourceContainer
{
	ResourcesList m_RList;             // List of objects for acces by Id's
	ResourcesMap  m_RMap;              // Map of objects for fast searching
	ResourceIds   m_AvailableIDs;      // Available object Id's for efficient ID's assigning after deleting

	SResourceContainer()
	{
		m_RList.reserve(512);
	}

	~SResourceContainer();

	void GetMemoryUsage(ICrySizer* pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
		pSizer->AddObject(m_RList);
		pSizer->AddObject(m_RMap);
		pSizer->AddObject(m_AvailableIDs);
	}
};

typedef std::map<CCryNameTSCRC, SResourceContainer*> ResourceClassMap;

typedef ResourceClassMap::iterator                   ResourceClassMapItor;

class CBaseResource
{
private:
	// Per resource variables
	volatile int32          m_nRefCount;
	int                     m_nID;
	CCryNameTSCRC           m_ClassName;
	CCryNameTSCRC           m_NameCRC;

	static ResourceClassMap m_sResources;

public:
	static CryCriticalSection s_cResLock;

public:
	//! Dirty flags will indicate what kind of data was invalidated
	enum EDirtyFlags
	{
		eDeviceResourceDirty     = BIT(0),
		eDeviceResourceViewDirty = BIT(1),
	};

public:
	// CCryUnknown interface
	inline void   SetRefCounter(int nRefCounter) { m_nRefCount = nRefCounter; }
	virtual int32 AddRef()
	{
		int32 nRef = CryInterlockedIncrement(&m_nRefCount);
		return nRef;
	}
	virtual int32 Release();
	virtual int   GetRefCounter() const { return m_nRefCount; }

	// Increment ref count, if not already scheduled for destruction.
	int32 TryAddRef()
	{
		volatile int nOldRef, nNewRef;
		do
		{
			nOldRef = m_nRefCount;
			if (nOldRef == 0)
				return 0;
			nNewRef = nOldRef + 1;
		}
		while (CryInterlockedCompareExchange(alias_cast<volatile LONG*>(&m_nRefCount), nNewRef, nOldRef) != nOldRef);
		return nNewRef;
	}

	// Constructors.
	CBaseResource() : m_nRefCount(1), m_nID(0) {}
	CBaseResource(const CBaseResource& Src);
	CBaseResource& operator=(const CBaseResource& Src);

	// Destructor.
	virtual ~CBaseResource() {};

	CCryNameTSCRC GetNameCRC() { return m_NameCRC; }
	//inline const char *GetName() const { return m_Name.c_str(); }
	//inline const char *GetClassName() const { return m_ClassName.c_str(); }
	inline int                 GetID() const  { return m_nID; }
	inline void                SetID(int nID) { m_nID = nID; }

	virtual bool               IsValid();

	static ILINE int           RListIndexFromId(int id)  { return id - 1; }
	static ILINE int           IdFromRListIndex(int idx) { return idx + 1; }

	static ResourceClassMap&   GetMaps()                 { return m_sResources; }
	static CBaseResource*      GetResource(const CCryNameTSCRC& className, int nID, bool bAddRef);
	static CBaseResource*      GetResource(const CCryNameTSCRC& className, const CCryNameTSCRC& Name, bool bAddRef);
	static SResourceContainer* GetResourcesForClass(const CCryNameTSCRC& className);
	static void                ShutDown();

	bool                       Register(const CCryNameTSCRC& resName, const CCryNameTSCRC& Name);
	bool                       UnRegister();

	virtual void               GetMemoryUsage(ICrySizer* pSizer) const = 0;

	// Will notify resource's user that some data of the the resource was invalidated.
	// dirtyFlags - one or more of the EDirtyFlags enum bits
	virtual void InvalidateDeviceResource(uint32 dirtyFlags) {};
};

//=================================================================

#if CAPTURE_REPLAY_LOG && (defined(WIN32) || defined (WIN64))
	#define MEMREPLAY_WRAP_D3D11
	#define MEMREPLAY_WRAP_D3D11_CONTEX
#endif

#if CAPTURE_REPLAY_LOG && defined(DURANGO) && defined(DIRECT3D10)
	#define MEMREPLAY_WRAP_D3D11
	#define MEMREPLAY_WRAP_D3D11_CONTEXT
	#define MEMREPLAY_WRAP_XBOX_PERFORMANCE_DEVICE
	#define MEMREPLAY_INSTRUMENT_TEXTUREPOOL
#endif

#ifdef MEMREPLAY_WRAP_D3D11

class MemReplayD3DAnnotation : public IUnknown
{
public:
	static const GUID s_guid;

public:
	MemReplayD3DAnnotation(ID3D11DeviceChild* pRes, size_t sz);
	~MemReplayD3DAnnotation();

	void                              AddMap(UINT nSubRes, void* pData, size_t sz);
	void                              RemoveMap(UINT nSubRes);

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject);
	virtual ULONG STDMETHODCALLTYPE   AddRef();
	virtual ULONG STDMETHODCALLTYPE   Release();

private:
	struct MapDesc
	{
		UINT  nSubResource;
		void* pData;
	};

private:
	MemReplayD3DAnnotation(const MemReplayD3DAnnotation&);
	MemReplayD3DAnnotation& operator=(const MemReplayD3DAnnotation&);

private:
	ULONG                m_nRefCount;
	ID3D11DeviceChild*   m_pRes;
	std::vector<MapDesc> m_maps;
};

inline void MemReplayAnnotateD3DResource(ID3D11DeviceChild* pResource, size_t resSz)
{
	if (pResource)
	{
		UINT sz = sizeof(MemReplayD3DAnnotation*);
		MemReplayD3DAnnotation* pAnnotation;
		if (FAILED(pResource->GetPrivateData(MemReplayD3DAnnotation::s_guid, &sz, &pAnnotation)))
		{
			pAnnotation = new MemReplayD3DAnnotation(pResource, resSz);
			pResource->SetPrivateDataInterface(MemReplayD3DAnnotation::s_guid, pAnnotation);
		}
	}
}

inline MemReplayD3DAnnotation* MemReplayGetD3DAnnotation(ID3D11DeviceChild* pResource)
{
	if (pResource)
	{
		UINT sz = sizeof(MemReplayD3DAnnotation*);
		MemReplayD3DAnnotation* pAnnotation;
		if (!FAILED(pResource->GetPrivateData(MemReplayD3DAnnotation::s_guid, &sz, &pAnnotation)))
		{
			return pAnnotation;
		}
	}
	return NULL;
}

#endif

#endif
