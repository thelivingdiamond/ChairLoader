// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#include "StdAfx.h"
#include "RenderView.h"

#include "DriverD3D.h"

#include "GraphicsPipeline/ShadowMap.h"
#include "CompiledRenderObject.h"

//////////////////////////////////////////////////////////////////////////
CRenderView::CRenderView(const char* name, EViewType type, CRenderView* pParentView, ShadowMapFrustum* pShadowFrustumOwner)
	: m_usageMode(eUsageModeUndefined)
	, m_viewType(type)
	, m_name(name)
	, m_frameId(0)
	, m_nSkipRenderingFlags(0)
	, m_nShaderRenderingFlags(0)
	, m_pParentView(pParentView)
	, m_bPostWriteExecuted(false)
	, m_bTrackUncompiledItems(false)
	, m_numUsedClientPolygons(0)
	, m_bDeferrredNormalDecals(false)
	, m_bAddingClientPolys(false)
{
	for (int i = 0; i < EFSLIST_NUM; i++)
	{
		m_renderItems[i].Init();
		m_renderItems[i].SetNoneWorkerThreadID(gEnv->mMainThreadId);
	}

	m_shadows.m_pShadowFrustumOwner = pShadowFrustumOwner;

	m_permanentRenderObjectsToCompile.Init();
	m_permanentRenderObjectsToCompile.SetNoneWorkerThreadID(gEnv->mMainThreadId);

	m_permanentObjects.Init();
	m_permanentObjects.SetNoneWorkerThreadID(gEnv->mMainThreadId);
}

//////////////////////////////////////////////////////////////////////////
CRenderView::~CRenderView()
{
	Clear();
}

void CRenderView::Clear()
{
	m_numUsedClientPolygons = 0;

	for (int i = 0; i < EFSLIST_NUM; i++)
	{
		m_renderItems[i].clear();
		m_BatchFlags[i] = 0;
	}

	for (int i = 0; i < eDLT_NumLightTypes; i++)
	{
		for (auto& light : m_lights[i])
		{
			light.DropResources();
		}
		m_lights[i].clear();
	}

	m_deferredDecals.clear();
	m_bDeferrredNormalDecals = false;
	m_clipVolumes.clear();

	m_shadows.Clear();

	ClearTemporaryCompiledObjects();
	m_permanentRenderObjectsToCompile.clear();

	m_bTrackUncompiledItems = gRenDev->m_nGraphicsPipeline >= 2;
}

// Helper function to allocate new compiled object from pool
CCompiledRenderObject* CRenderView::AllocCompiledObject(CRenderObject* pObj, CRendElementBase* pElem, const SShaderItem& shaderItem)
{
	CCompiledRenderObject* pCompiledObject = CCompiledRenderObject::AllocateFromPool();
	pCompiledObject->Init(shaderItem, pElem);

	// Assign any compiled object to the RenderObject, just to be used as a root reference for constant buffer sharing
	pObj->m_pCompiledObject = pCompiledObject;

	return pCompiledObject;
}

// Helper function
CCompiledRenderObject* CRenderView::AllocCompiledObjectTemporary(CRenderObject* pObj, CRendElementBase* pElem, const SShaderItem& shaderItem)
{
	CCompiledRenderObject* pCompiledObject = CCompiledRenderObject::AllocateFromPool();
	pCompiledObject->Init(shaderItem, pElem);

	// Assign any compiled object to the RenderObject, just to be used as a root reference for constant buffer sharing
	pObj->m_pCompiledObject = pCompiledObject;

	SCompiledPair pair;
	pair.pRenderObject = pObj;
	pair.pCompiledObject = pCompiledObject;
	m_temporaryCompiledObjects.push_back(pair);

	// Temporary object is not linked to the RenderObject owner
	return pCompiledObject;
}

SDeferredDecal* CRenderView::AddDeferredDecal(const SDeferredDecal& source)
{
	m_deferredDecals.push_back(source);
	return &m_deferredDecals.back();
}

std::vector<SDeferredDecal>& CRenderView::GetDeferredDecals()
{
	return m_deferredDecals;
}

void CRenderView::SetFrameId(uint64 frameId)
{
	m_frameId = frameId;
}

//////////////////////////////////////////////////////////////////////////
void CRenderView::SwitchUsageMode(EUsageMode mode)
{
	FUNCTION_PROFILER_RENDERER;

	if (mode == m_usageMode)
		return;

	//assert(mode != m_usageMode);

	if (mode == eUsageModeWriting)
	{
		CRY_ASSERT(m_usageMode == IRenderView::eUsageModeUndefined || m_usageMode == IRenderView::eUsageModeReadingDone);
		CRY_ASSERT(!m_jobstate_Write.IsRunning());
		CRY_ASSERT(!m_jobstate_Sort.IsRunning());

		m_bPostWriteExecuted = false;

		// Post jobs are not executed immediately for Shadow views, they will only start when all shadow views are done writing
		//if (m_viewType != eViewType_Shadow)
		{
			// Job_PostWrite job will be called when all writing to the view will finish (m_jobstate_Write is stopped)
			auto lambda_job_after_write = [this] {
				Job_PostWrite();
			};
			m_jobstate_Write.RegisterPostJobCallback("JobRenderViewPostWrite", lambda_job_after_write, JobManager::eRegularPriority, &m_jobstate_PostWrite);
		}

		// If no items will be written, next SetStopped call will trigger post job
		m_jobstate_Write.SetRunning();
	}

	if (mode == eUsageModeWritingDone)
	{
		CRY_ASSERT(m_usageMode == IRenderView::eUsageModeWriting);

		// Write to view is finished, we can now start processing render items preparing them for rendering.
		// Try stopping writes, this can trigger PostWrite job
		if (!m_jobstate_Write.TryStopping())
		{
			// If still jobs running we cannot switch to WritingDone yet.
			return;
		}
	}

	if (mode == eUsageModeReading)
	{
		{
			PROFILE_FRAME(RenderViewWaitForPostWriteJob);

			// Prepare view for rasterizing (reading)
			// We now need to wait until all prepare for writing jobs are done.
			m_jobstate_Write.Wait();
			m_jobstate_PostWrite.Wait();
			m_jobstate_Sort.Wait();
		}

		//Job_PostWrite();
		assert(m_bPostWriteExecuted);

		CRY_ASSERT(m_usageMode == IRenderView::eUsageModeWritingDone);

		CompileModifiedRenderObjects();
		UpdateModifiedShaderItems();
	}

	if (mode == eUsageModeReadingDone)
	{
		CRY_ASSERT(m_usageMode == IRenderView::eUsageModeReading);
		CRY_ASSERT(!m_jobstate_Write.IsRunning());
		CRY_ASSERT(!m_jobstate_PostWrite.IsRunning());
		CRY_ASSERT(!m_jobstate_Sort.IsRunning());
	}

	{
		CryAutoLock<CryCriticalSectionNonRecursive> lock(m_lock_UsageMode);
		m_usageMode = mode;
	}
}

void CRenderView::PrepareForRendering()
{
	for (int i = 0; i < EFSLIST_NUM; i++)
	{
		//m_renderItems[i].CoalesceMemory();
		//m_RenderListDesc[0].m_nEndRI[i] = m_renderItems[i].size();
	}
	//ClearBatchFlags();
}

//////////////////////////////////////////////////////////////////////////
void CRenderView::PrepareForWriting()
{
}

//////////////////////////////////////////////////////////////////////////
void CRenderView::AddDynamicLight(SRenderLight& light)
{
	AddLight(eDLT_DynamicLight, light);
}

//////////////////////////////////////////////////////////////////////////
int CRenderView::GetDynamicLightsCount() const
{
	return GetLightsCount(eDLT_DynamicLight);
}

//////////////////////////////////////////////////////////////////////////
SRenderLight& CRenderView::GetDynamicLight(int nLightId)
{
	return GetLight(eDLT_DynamicLight, nLightId);
}

//////////////////////////////////////////////////////////////////////////
void CRenderView::AddLight(eDeferredLightType lightType, SRenderLight& light)
{
	assert((light.m_Flags & DLF_LIGHTTYPE_MASK) != 0);
	if (m_lights[lightType].size() >= 32)
	{
		light.m_Id = -1;
		return;
	}
	if (light.m_Flags & DLF_FAKE)
	{
		light.m_Id = -1;
	}
	{
		light.m_Id = int16(m_lights[lightType].size());
		m_lights[lightType].push_back(light);
	}
}

//////////////////////////////////////////////////////////////////////////
SRenderLight* CRenderView::AddLightAtIndex(eDeferredLightType lightType, const SRenderLight& light, int index /*=-1*/)
{
	if (index < 0)
	{
		m_lights[lightType].push_back(light);
		index = m_lights[lightType].size() - 1;
	}
	else
	{
		m_lights[lightType].insert(m_lights[lightType].begin() + index, light);
	}
	m_lights[lightType][index].AcquireResources();
	return &m_lights[lightType][index];
}

int CRenderView::GetLightsCount(eDeferredLightType lightType) const
{
	return (int)m_lights[lightType].size();
}

SRenderLight& CRenderView::GetLight(eDeferredLightType lightType, int nLightId)
{
	assert(nLightId >= 0 && nLightId < m_lights[lightType].size());
	return m_lights[lightType][nLightId];
}

//////////////////////////////////////////////////////////////////////////
RenderLightsArray& CRenderView::GetLightsArray(eDeferredLightType lightType)
{
	assert(lightType >= 0 && lightType < eDLT_NumLightTypes);
	return m_lights[lightType];
}
//////////////////////////////////////////////////////////////////////////
uint8 CRenderView::AddClipVolume(const IClipVolume* pClipVolume)
{
	if (m_clipVolumes.size() < MAX_DEFERRED_CLIP_VOLUMES)
	{
		m_clipVolumes.push_back(SDeferredClipVolume());

		auto& volume = m_clipVolumes.back();
		volume.nStencilRef = uint8(m_clipVolumes.size() + 1); // the first clip volume ID is reserved for outdoors
		volume.nFlags = pClipVolume->GetClipVolumeFlags();
		volume.mAABB = pClipVolume->GetClipVolumeBBox();
		pClipVolume->GetClipVolumeMesh(volume.m_pRenderMesh, volume.mWorldTM);

		return volume.nStencilRef;
	}

	return -1;
}

void CRenderView::SetClipVolumeBlendInfo(const IClipVolume* pClipVolume, int blendInfoCount, IClipVolume** blendVolumes, Plane* blendPlanes)
{
	size_t nClipVolumeIndex = pClipVolume->GetStencilRef() - 1; // 0 is reserved for outdoor
	CRY_ASSERT(blendInfoCount < SDeferredClipVolume::MaxBlendInfoCount);
	CRY_ASSERT(nClipVolumeIndex >= 0 && nClipVolumeIndex < m_clipVolumes.size() && m_clipVolumes[nClipVolumeIndex].nStencilRef == pClipVolume->GetStencilRef());

	SDeferredClipVolume& volume = m_clipVolumes[nClipVolumeIndex];

	SDeferredClipVolume::BlendInfo emptyBlendInfo = { -1, Vec4(ZERO) };
	std::fill(volume.blendInfo, volume.blendInfo + SDeferredClipVolume::MaxBlendInfoCount, emptyBlendInfo);

	for (int i = 0; i < blendInfoCount; ++i)
	{
		volume.blendInfo[i].blendID = blendVolumes[i] ? blendVolumes[i]->GetStencilRef() : 0;
		volume.blendInfo[i].blendPlane = Vec4(blendPlanes[i].n, blendPlanes[i].d);
	}

	volume.nFlags |= IClipVolume::eClipVolumeBlend;
}

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
CREClientPoly* CRenderView::AddClientPoly()
{
	int nIndex = m_numUsedClientPolygons++;
	if (nIndex >= m_polygonsPool.size())
	{
		m_polygonsPool.push_back(new CREClientPoly);
	}
	return m_polygonsPool[nIndex];
}

//////////////////////////////////////////////////////////////////////////
CRenderView::RenderItems& CRenderView::GetRenderItems(int nRenderList)
{
	assert(m_usageMode != eUsageModeWriting || (nRenderList == EFSLIST_PREPROCESS || nRenderList == EFSLIST_HDRPOSTPROCESS || nRenderList == EFSLIST_POSTPROCESS)); // While writing we must not read back items.

	m_renderItems[nRenderList].CoalesceMemory();
	return m_renderItems[nRenderList];
}

uint32 CRenderView::GetBatchFlags(int nRenderList) const
{
	return m_BatchFlags[nRenderList];
}

//////////////////////////////////////////////////////////////////////////
void CRenderView::AddPermanentObjectInline(CPermanentRenderObject* pObject, SRendItemSorter sorter, int shadowFrustumSide)
{
	SPermanentObjectRecord rec;
	rec.pRenderObject = pObject;
	;
	rec.itemSorter = sorter.GetValue();
	rec.shadowFrustumSide = shadowFrustumSide;
	m_permanentObjects.push_back(rec);
}

//////////////////////////////////////////////////////////////////////////
void CRenderView::AddPermanentObject(CRenderObject* pObject, const SRenderingPassInfo& passInfo)
{
	assert(pObject->m_bPermanent);
	AddPermanentObjectInline(static_cast<CPermanentRenderObject*>(pObject), passInfo.GetRendItemSorter(), passInfo.ShadowFrustumSide());
}

//////////////////////////////////////////////////////////////////////////
void CRenderView::AddRenderItem(CRendElementBase* pElem, CRenderObject* RESTRICT_POINTER pObj, const SShaderItem& shaderItem, uint32 nList, uint32 nBatchFlags,
                                SRendItemSorter sorter, bool bShadowPass, bool bForceOpaqueForward)
{
	assert(m_usageMode == eUsageModeWriting || m_bAddingClientPolys || (nList == EFSLIST_PREPROCESS || nList == EFSLIST_HDRPOSTPROCESS || nList == EFSLIST_POSTPROCESS));  // Adding items only in writing mode

	CShader* RESTRICT_POINTER pShader = (CShader*)shaderItem.m_pShader;

	if (!bShadowPass)
	{
		// Special case optimization for new pipeline.
		if (nList == EFSLIST_GENERAL && (!(pShader->m_Flags & EF_SUPPORTSDEFERREDSHADING_FULL) || bForceOpaqueForward))
		{
			// Redirect general list items to the forward opaque list
			nList = EFSLIST_FORWARD_OPAQUE;
		}
	}

	nBatchFlags |= (shaderItem.m_nPreprocessFlags & FSPR_MASK);

	SRendItem ri;

	ri.pObj = pObj;
	ri.pCompiledObject = nullptr;

	if (nList == EFSLIST_TRANSP || nList == EFSLIST_HALFRES_PARTICLES)
		ri.fDist = pObj->m_fDistance + pObj->m_fSort;
	else
		ri.ObjSort = (pObj->m_ObjFlags & 0xffff0000) | pObj->m_nSort;
	ri.nBatchFlags = nBatchFlags;
	//ri.nStencRef = pObj->m_nClipVolumeStencilRef + 1; // + 1, we start at 1. 0 is reserved for MSAAed areas.

	ri.rendItemSorter = sorter;

	uint32 nResID = shaderItem.m_pShaderResources ? ((CShaderResources*)(shaderItem.m_pShaderResources))->m_Id : 0;
	uint32 nShaderId = pShader->mfGetID();
	assert(nResID < CShader::s_ShaderResources_known.size());
	assert(nShaderId != 0);
	ri.SortVal = (nResID << 6) | (nShaderId << 20) | (shaderItem.m_nTechnique & 0x3f);
	ri.pElem = pElem;

	// objects with FOB_NEAREST go to EFSLIST_NEAREST in shadow pass and general
	if ((pObj->m_ObjFlags & FOB_NEAREST) && (bShadowPass || (nList == EFSLIST_GENERAL && CRenderer::CV_r_GraphicsPipeline > 0)))
		nList = EFSLIST_NEAREST_OBJECTS;

	// add flags atomically, if this one is not set already
	CryInterlockedExchangeOr((volatile LONG*)&m_BatchFlags[nList], nBatchFlags);

	//////////////////////////////////////////////////////////////////////////
	// Permanent objects support.
	//////////////////////////////////////////////////////////////////////////
	if (pObj->m_bPermanent)
	{
		CPermanentRenderObject* RESTRICT_POINTER pPermanentRendObj = reinterpret_cast<CPermanentRenderObject*>(pObj);
		WriteLock lock(pPermanentRendObj->m_accessLock); // Block on write access to the render object

		// Must add this render item to the local array.
		// This is not thread safe!!!, code at 3d engine makes sure this can never be called simultaneously on the same RenderObject on the same pass type.
		//  General and shadows can be called simultaneously, they are writing to the separate arrays.
		CPermanentRenderObject::SPermanentRendItem pri;
		pri.m_nBatchFlags = nBatchFlags;
		pri.m_nRenderList = nList;
		pri.m_objSort = ri.ObjSort;
		pri.m_pCompiledObject = ri.pCompiledObject;
		pri.m_pRenderElement = pElem;
		pri.m_sortValue = ri.SortVal;

		auto renderPassType = (bShadowPass) ? CPermanentRenderObject::eRenderPass_Shadows : CPermanentRenderObject::eRenderPass_General;
		pPermanentRendObj->m_permanentRenderItems[renderPassType].push_back(pri);

		pPermanentRendObj->PrepareForUse(pElem, renderPassType);

		return;
	}
	else if (m_bTrackUncompiledItems)
	{
		// This item will need a temporary compiled object
		if (pElem && pElem->mfGetType() == eDATA_Mesh && nList != EFSLIST_TRANSP) // temporary disable for these types
		{
			// Allocate new CompiledRenderObject.
			ri.pCompiledObject = AllocCompiledObjectTemporary(pObj, pElem, shaderItem);
		}
	}
	//////////////////////////////////////////////////////////////////////////
	m_renderItems[nList].push_back(ri);

	//////////////////////////////////////////////////////////////////////////
	// Handle additional passes
	if (!bShadowPass)
	{
		if (nBatchFlags & FB_ZPREPASS)
		{
			m_renderItems[EFSLIST_ZPREPASS].push_back(ri);
			CryInterlockedExchangeOr((volatile LONG*)&m_BatchFlags[EFSLIST_ZPREPASS], nBatchFlags);
		}
		if (shaderItem.m_pShaderResources && shaderItem.m_pShaderResources->IsEmissive())
		{
			m_renderItems[EFSLIST_FORWARD_OPAQUE].push_back(ri);
			CryInterlockedExchangeOr((volatile LONG*)&m_BatchFlags[EFSLIST_FORWARD_OPAQUE], nBatchFlags);
		}
	}

	////////////////////////////////////////////////////////////////////////
	// Check if shader item needs update
	CheckAndScheduleForUpdate(shaderItem);
}

//////////////////////////////////////////////////////////////////////////
void CRenderView::ExpandPermanentRenderObjects()
{
	PROFILE_FRAME(ExpandPermanentRenderObjects);

	m_permanentObjects.CoalesceMemory();

	if (!m_permanentObjects.empty())
	{
		assert(m_usageMode != eUsageModeWriting);
	}

	SShaderItem shaderItem;
	shaderItem.m_nPreprocessFlags = 0;

	int renderPassType = IsShadowGenView() ? CPermanentRenderObject::eRenderPass_Shadows : CPermanentRenderObject::eRenderPass_General;
	uint32 passId = IsShadowGenView() ? 1 : 0;
	uint32 passMask = 1 << passId;

	// Expand normal render items
	for (int obj = 0, numObj = m_permanentObjects.size(); obj < numObj; obj++)
	{
		const SPermanentObjectRecord& RESTRICT_REFERENCE record = m_permanentObjects[obj];
		CPermanentRenderObject* RESTRICT_POINTER pRenderObject = record.pRenderObject;
		assert(pRenderObject->m_bPermanent);

		// Submit all valid objects (skip not ready and helper objects), TODO: release helper objects
		while (pRenderObject)
		{
			ReadLock lock(pRenderObject->m_accessLock); // Block on read access to the render object

			bool bRecompile = (pRenderObject->m_passReadyMask != pRenderObject->m_compiledReadyMask) && (pRenderObject->m_passReadyMask & passMask);

			auto& permanent_items = pRenderObject->m_permanentRenderItems[renderPassType];
			auto& RESTRICT_REFERENCE shadow_items = pRenderObject->m_permanentRenderItems[CPermanentRenderObject::eRenderPass_Shadows];
			int num_shadow_items = shadow_items.size();

			size_t numItems = permanent_items.size();
			assert(numItems < 128); // Sanity check, otherwise too many chunks in the mesh
			for (size_t i = 0; i < numItems; i++)
			{
				auto& RESTRICT_REFERENCE pri = permanent_items[i];

				if (!pri.m_pCompiledObject)
				{
					bool bRequireCompiledRenderObject = false;
					// This item will need a temporary compiled object
					if (pri.m_pRenderElement && pri.m_pRenderElement->mfGetType() == eDATA_Mesh && pri.m_nRenderList != EFSLIST_TRANSP) // temporary disable for these types
					{
						bRequireCompiledRenderObject = true;
					}

					if (bRequireCompiledRenderObject)
					{
						// Optimization code that try to share CompiledRenderObject with the one from the shadow pass.
						// Same CompiledRenderObject can be used in general pass and in shadow gen pass when then both reference the same render element and material (shader item)
						if (renderPassType == CPermanentRenderObject::eRenderPass_General && i < num_shadow_items)
						{
							auto& pri2 = shadow_items[i];
							if (pri2.m_pCompiledObject && pri2.m_pRenderElement == pri.m_pRenderElement && pri2.m_sortValue == pri.m_sortValue)
							{
								pri.m_pCompiledObject = pri2.m_pCompiledObject;
								// Mark this compiled object to be shared between general and shadow pass, to prevent double deletion
								pri.m_pCompiledObject->m_bSharedWithShadow = true;
								pRenderObject->m_bInstanceDataDirty = false; // In this case everything need to be recompiled, not only instance data.
								bRecompile = true;
							}
						}

						if (!pri.m_pCompiledObject)
						{
							SShaderItem shaderItem;
							SRendItem::ExtractShaderItem(pri.m_sortValue, shaderItem);
							pri.m_pCompiledObject = AllocCompiledObject(pRenderObject, pri.m_pRenderElement, shaderItem); // Allocate new CompiledRenderObject.
							pRenderObject->m_bInstanceDataDirty = false;                                                  // In this case everything need to be recompiled, not only instance data.
							bRecompile = true;
						}
					}
				}

				{
					SShaderItem shaderItem;
					SRendItem::ExtractShaderItem(pri.m_sortValue, shaderItem);
					if (!shaderItem.m_pShader || !shaderItem.m_pShaderResources || !shaderItem.m_pShaderResources->IsValid())
					{
						if (pRenderObject->m_pRenderNode)
							pRenderObject->m_pRenderNode->InvalidatePermanentRenderObject();

						continue;
					}

					CheckAndScheduleForUpdate(shaderItem);
				}

				SRendItem ri;
				ri.SortVal = pri.m_sortValue;
				ri.pElem = pri.m_pRenderElement;
				ri.pObj = pRenderObject;
				ri.pCompiledObject = pri.m_pCompiledObject;
				ri.ObjSort = pri.m_objSort;
				ri.nBatchFlags = pri.m_nBatchFlags;
				//ri.nStencRef = pRenderObject->m_nClipVolumeStencilRef + 1; // + 1, we start at 1. 0 is reserved for MSAAed areas.
				ri.rendItemSorter = SRendItemSorter(record.itemSorter);

				int rendeList = pri.m_nRenderList;

				if (renderPassType == CPermanentRenderObject::eRenderPass_Shadows)
				{
					// Shadow pass
					m_renderItems[record.shadowFrustumSide].push_back(ri);
				}
				else
				{
					// General pass
					m_renderItems[pri.m_nRenderList].push_back(ri);
					// Doesn't need interlocked or, this function is called only from one thread
					m_BatchFlags[rendeList] |= pri.m_nBatchFlags;

					//////////////////////////////////////////////////////////////////////////
					// Handle Z-Prepass
					if (pri.m_nBatchFlags & FB_ZPREPASS)
					{
						// Add one more item to the ZPrepass list
						m_renderItems[EFSLIST_ZPREPASS].push_back(ri);
						m_BatchFlags[EFSLIST_ZPREPASS] |= pri.m_nBatchFlags;
					}

					if (pRenderObject->m_ObjFlags & FOB_NEAREST)
					{
						// Add one more item to the nearest objects list
						m_renderItems[EFSLIST_NEAREST_OBJECTS].push_back(ri);
						m_BatchFlags[EFSLIST_NEAREST_OBJECTS] |= pri.m_nBatchFlags;
					}
				}

				if (pri.m_pRenderElement && pri.m_pRenderElement->m_Flags & (FCEF_DIRTY | FCEF_SKINNED | FCEF_UPDATEALWAYS))
				{
					pRenderObject->m_bInstanceDataDirty = false; // In this case everything need to be recompiled, not only instance data.
					bRecompile = true;
				}
			}

			if (bRecompile ||
			    pRenderObject->m_bInstanceDataDirty ||
			    !pRenderObject->m_bAllCompiledValid)
			{
				CPermanentRenderObject* pNonRestrict = pRenderObject;
				m_permanentRenderObjectsToCompile.push_back(pNonRestrict);
				pRenderObject->m_compiledReadyMask &= ~passMask;      // This compiled masks invalid
			}

			pRenderObject = pRenderObject->m_pNextPermanent;
		}
	}

	// Clean permanent objects
	m_permanentObjects.clear();
}

//////////////////////////////////////////////////////////////////////////
void CRenderView::CompileModifiedRenderObjects()
{
	CRY_PROFILE_FUNCTION_ARG(PROFILE_RENDERER, m_name.c_str())

	const float realTime = gRenDev->m_RP.m_TI[SRenderThread::GetLocalThreadCommandBufferId()].m_RealTime;

	assert(gRenDev->m_pRT->IsRenderThread());

	//////////////////////////////////////////////////////////////////////////
	// Compile all modified objects.
	m_permanentRenderObjectsToCompile.CoalesceMemory();

	uint32 passId = IsShadowGenView() ? 1 : 0;
	uint32 passMask = 1 << passId;

	int numObjects = m_permanentRenderObjectsToCompile.size();

	int nFrameId = gEnv->pRenderer->GetFrameID(false);
	//{ char buf[1024]; cry_sprintf(buf, "CRenderView::CompileModifiedRenderObjects: frame(%d) numObjects(%d) \r\n", nFrameId,numObjects); OutputDebugString(buf); }

	for (int obj = 0; obj < numObjects; obj++)
	{
		CPermanentRenderObject* pRenderObject = m_permanentRenderObjectsToCompile[obj];

		//if (pRenderObject->m_compiledReadyMask == pRenderObject->m_passReadyMask &&	pRenderObject->m_lastCompiledFrame == nFrameId)
		if (pRenderObject->m_compiledReadyMask == pRenderObject->m_passReadyMask)
		//if (pRenderObject->m_bAllCompiledValid && pRenderObject->m_lastCompiledFrame == nFrameId)
		{
			// Already compiled in this frame
			//continue;
		}

		ReadLock lock(pRenderObject->m_accessLock); // Block on read access to the render object

		// Do compilation on the chain of the compiled objects
		bool bAllCompiled = true;

		// compile items
		auto& RESTRICT_REFERENCE shadow_items = pRenderObject->m_permanentRenderItems[CPermanentRenderObject::eRenderPass_Shadows];
		auto& RESTRICT_REFERENCE general_items = pRenderObject->m_permanentRenderItems[CPermanentRenderObject::eRenderPass_General];

		for (int i = 0, num = general_items.size(); i < num; i++)
		{
			auto& pri = general_items[i];
			if (!pri.m_pCompiledObject)
				continue;
			if (!pri.m_pCompiledObject->Compile(pRenderObject, realTime))
			{
				bAllCompiled = false;
			}
		}

		for (int i = 0, num = shadow_items.size(); i < num; i++)
		{
			auto& pri = shadow_items[i];
			if (!pri.m_pCompiledObject || pri.m_pCompiledObject->m_bSharedWithShadow) // This compiled object must be already compiled with the general items
				continue;
			if (!pri.m_pCompiledObject->Compile(pRenderObject, realTime))
				bAllCompiled = false;
		}

		if (bAllCompiled)
		{
			pRenderObject->m_compiledReadyMask |= passMask;
		}

		pRenderObject->m_lastCompiledFrame = nFrameId;
		pRenderObject->m_bAllCompiledValid = bAllCompiled;
	}
	m_permanentRenderObjectsToCompile.clear();

	// Compile all temporary compiled objects
	m_temporaryCompiledObjects.CoalesceMemory();
	int numTempObjects = m_temporaryCompiledObjects.size();
	for (int i = 0; i < numTempObjects; i++)
	{
		auto& pair = m_temporaryCompiledObjects[i]; // first=CRenderObject, second=CCompiledObject
		pair.pCompiledObject->Compile(pair.pRenderObject, realTime);
	}

	//////////////////////////////////////////////////////////////////////////

#if defined(ENABLE_PROFILING_CODE)
	CryInterlockedAdd(&gcpRendD3D->m_RP.m_PS[gcpRendD3D->m_RP.m_nProcessThreadID].m_nModifiedCompiledObjects, numObjects);
	CryInterlockedAdd(&gcpRendD3D->m_RP.m_PS[gcpRendD3D->m_RP.m_nProcessThreadID].m_nTempCompiledObjects, numTempObjects);
#endif
}

//////////////////////////////////////////////////////////////////////////
void CRenderView::UpdateModifiedShaderItems()
{
	for (int i = 0; i < m_numUsedClientPolygons; ++i)
	{
		CheckAndScheduleForUpdate(m_polygonsPool[i]->m_Shader);
	}

	for (auto& decal : m_deferredDecals)
	{
		CheckAndScheduleForUpdate(decal.pMaterial->GetShaderItem(0));
	}

	m_shaderItemsToUpdate.CoalesceMemory();

	for (auto& item : m_shaderItemsToUpdate)
	{
		auto pShaderResources = item.first;
		auto pShader = item.second;
		auto pResourceSet = pShaderResources->m_pCompiledResourceSet;

		if (pResourceSet && (pResourceSet->GetFlags() & CDeviceResourceSet::EFlags_DynamicUpdates))
		{
			gcpRendD3D->FX_UpdateDynamicShaderResources(pShaderResources, FB_GENERAL, 0);
		}

		if (pResourceSet && (pResourceSet->GetFlags() & CDeviceResourceSet::EFlags_PendingAllocation))
		{
			pResourceSet->Fill(pShader, pShaderResources, EShaderStage_AllWithoutCompute);
		}

		if (pShaderResources->HasDynamicTexModifiers())
		{
			pShaderResources->RT_UpdateConstants(pShader);
		}
		else if (pResourceSet && pResourceSet->IsDirty())
		{
			pResourceSet->Build();
		}
	}

	m_shaderItemsToUpdate.clear();
}

//////////////////////////////////////////////////////////////////////////
void CRenderView::ClearTemporaryCompiledObjects()
{
	PROFILE_FRAME(ClearTemporaryCompiledObjects);

	/////////////////////////////////////////////////////////////////////////////
	// Clean up non permanent compiled objects
	size_t numObjects = m_temporaryCompiledObjects.size();
	for (size_t i = 0; i < numObjects; i++)
	{
		auto& pair = m_temporaryCompiledObjects[i];
		CCompiledRenderObject::FreeToPool(pair.pCompiledObject);
	}
	m_temporaryCompiledObjects.clear();
	/////////////////////////////////////////////////////////////////////////////
}

CRenderView::ItemsRange CRenderView::GetItemsRange(ERenderListID renderList)
{
	auto& items = GetRenderItems(renderList);
	return ItemsRange(0, items.size());
}

CRenderView::ItemsRange CRenderView::GetShadowItemsRange(ShadowMapFrustum* pFrustum, int nFrustumSide)
{
	if (nFrustumSide < EFSLIST_NUM)
	{
		auto& items = GetShadowItems(pFrustum, nFrustumSide);
		return ItemsRange(0, items.size());
	}
	return ItemsRange(0, 0);
}

CRenderView::RenderItems& CRenderView::GetShadowItems(ShadowMapFrustum* pFrustum, int nFrustumSide)
{
	assert(m_usageMode != eUsageModeWriting); // While writing we must not read back items.
	assert(nFrustumSide >= 0 && nFrustumSide < EFSLIST_NUM);

	CRenderView* pShadowsView = GetShadowsView(pFrustum);
	assert(pShadowsView);
	if (!pShadowsView)
	{
		static CRenderView::RenderItems empty;
		return empty;
	}
	return pShadowsView->GetRenderItems(nFrustumSide);
}

//////////////////////////////////////////////////////////////////////////
void CRenderView::Job_PostWrite()
{
	CRY_PROFILE_FUNCTION_ARG(PROFILE_RENDERER, m_name.c_str());

	// Prevent double entering this
	CryAutoLock<CryCriticalSectionNonRecursive> lock(m_lock_PostWrite);

	// At this point it is really writing done, if was not set yet.
	{
		CryAutoLock<CryCriticalSectionNonRecursive> lock(m_lock_UsageMode);
		if (m_usageMode == eUsageModeWriting)
			m_usageMode = eUsageModeWritingDone;
	}

	if (m_bPostWriteExecuted)
		return;

	ExpandPermanentRenderObjects();

	AddRenderItemsFromClientPolys();

	for (int renderList = 0; renderList < EFSLIST_NUM; renderList++)
	{
		if (renderList == EFSLIST_PREPROCESS && m_viewType != eViewType_Shadow)
		{
			// These lists need not to be sorted here
			continue;
		}

		auto& renderItems = GetRenderItems(renderList);

		if (!renderItems.empty())
		{
			auto lambda_job = [ = ]
			{
				Job_SortRenderItemsInList((ERenderListID)renderList);
			};
			gEnv->pJobManager->AddLambdaJob("SortRenderItems", lambda_job, JobManager::eRegularPriority, &m_jobstate_Sort);
			//lambda_job();
		}
	}
	m_bPostWriteExecuted = true;
}

///////////////////////////////////////////////////////////////////////////////
void CRenderView::Job_SortRenderItemsInList(ERenderListID list)
{
	FUNCTION_PROFILER_RENDERER

	auto& renderItems = GetRenderItems(list);
	if (renderItems.empty())
		return;

	int nStart = 0;
	int n = renderItems.size();

	if (m_viewType == eViewType_Shadow)
	{
		// Sort Shadow render items differently
		//assert(m_shadows.m_frustums.size() == 1);// Should only have one current frustum.
		if (m_shadows.m_pShadowFrustumOwner)
			m_shadows.m_pShadowFrustumOwner->SortRenderItemsForFrustumAsync(list, &renderItems[0], renderItems.size());
		return;
	}

	if (m_viewType != eViewType_Shadow)
	{
		// Reorder all but shadow gen
		//std::sort(&renderItems[nStart],&renderItems[nStart] + n, SCompareByOnlyStableFlagsOctreeID());
	}

	switch (list)
	{
	case EFSLIST_PREPROCESS:
		{
			PROFILE_FRAME(State_SortingPre);
			SRendItem::mfSortPreprocess(&renderItems[nStart], n);
		}
		break;

	case EFSLIST_SHADOW_GEN:
		// No need to sort.
		break;

	case EFSLIST_DEFERRED_PREPROCESS:
	case EFSLIST_HDRPOSTPROCESS:
	case EFSLIST_POSTPROCESS:
	case EFSLIST_FOG_VOLUME:
		break;

	case EFSLIST_WATER_VOLUMES:
	case EFSLIST_TRANSP:
	case EFSLIST_WATER:
	case EFSLIST_HALFRES_PARTICLES:
	case EFSLIST_LENSOPTICS:
	case EFSLIST_EYE_OVERLAY:
		{
			PROFILE_FRAME(State_SortingDist);
			SRendItem::mfSortByDist(&renderItems[nStart], n, false);
		}
		break;
	case EFSLIST_DECAL:
		{
			PROFILE_FRAME(State_SortingDecals);
			SRendItem::mfSortByDist(&renderItems[nStart], n, true);
		}
		break;

	case EFSLIST_ZPREPASS:
		//if (m_bUseGPUFriendlyBatching[nThread])
		{
			PROFILE_FRAME(State_SortingZPass);
			if (CRenderer::CV_r_ZPassDepthSorting == 1)
				SRendItem::mfSortForZPass(&renderItems[nStart], n);
			else if (CRenderer::CV_r_ZPassDepthSorting == 2)
				SRendItem::mfSortByDist(&renderItems[nStart], n, false, true);
		}
		break;

	case EFSLIST_GENERAL:
	case EFSLIST_SKIN:
		{
			{
				PROFILE_FRAME(State_SortingGBuffer);
				SRendItem::mfSortByLight(&renderItems[nStart], n, true, false, false);
			}
		}
		break;

	case EFSLIST_FORWARD_OPAQUE:
		{
			{
				PROFILE_FRAME(State_SortingForwardOpaque);
				SRendItem::mfSortByLight(&renderItems[nStart], n, true, false, false);
			}
		}
		break;

	case EFSLIST_AFTER_POSTPROCESS:
	case EFSLIST_AFTER_HDRPOSTPROCESS:
		{
			PROFILE_FRAME(State_SortingLight);
			SRendItem::mfSortByLight(&renderItems[nStart], n, true, false, list == EFSLIST_DECAL);
		}
		break;
	case EFSLIST_TERRAINLAYER:
		{
			PROFILE_FRAME(State_SortingLight_TerrainLayers);
			SRendItem::mfSortByLight(&renderItems[nStart], n, true, true, false);
		}
		break;

	case EFSLIST_NEAREST_OBJECTS:
		// No need to sort.
		break;

	default:
		assert(0);
	}
}

int CRenderView::FindRenderListSplit(ERenderListID list, uint32 objFlag)
{
	FUNCTION_PROFILER_RENDERER

	// Binary search, assumes that list is sorted by objFlag
	auto& renderItems = GetRenderItems(list);

	int first = 0;
	int last = renderItems.size() - 1;

	while (first <= last)
	{
		int middle = (first + last) / 2;

		if (renderItems[middle].pObj->m_ObjFlags & objFlag)
			first = middle + 1;
		else
			last = middle - 1;
	}

	return last + 1;
}

//////////////////////////////////////////////////////////////////////////
void CRenderView::AddRenderItemsFromClientPolys()
{
	FUNCTION_PROFILER_RENDERER

	uint32 i;
	CREClientPoly* pl;

	// Needed to prevent assert in AddRenderItem
	m_bAddingClientPolys = true;

	int numPolys = GetClientPolyCount();

	bool bShadows = (m_viewType == eViewType_Shadow);
	int defaultList = (bShadows) ? 0 : EFSLIST_GENERAL;

	if (!(m_nShaderRenderingFlags & SHDF_ALLOWPOSTPROCESS))
		return;

	for (i = 0; i < numPolys; i++)
	{
		pl = GetClientPoly(i);

		CShader* pShader = (CShader*)pl->m_Shader.m_pShader;
		CShaderResources* const __restrict pShaderResources = static_cast<CShaderResources*>(pl->m_Shader.m_pShaderResources);
		SShaderTechnique* pTech = pl->m_Shader.GetTechnique();

		uint32 nBatchFlags = FB_GENERAL;
		nBatchFlags |= (pl->m_nCPFlags & CREClientPoly::efAfterWater) ? 0 : FB_BELOW_WATER;

		//if (pTech && pShaderResources)
		//{
		//if (pTech->m_nTechnique[TTYPE_CUSTOMRENDERPASS] > 0 && m_nThermalVisionMode && pShaderResources->HeatAmount())
		//nBatchFlags |= FB_CUSTOM_RENDER;
		//}

		if (pl->m_Shader.m_nPreprocessFlags & FSPR_MASK)
		{
			AddRenderItem(pl, pl->m_pObject, pl->m_Shader, (bShadows) ? 0 : EFSLIST_PREPROCESS, FB_GENERAL, pl->rendItemSorter, bShadows, false);
		}

		if (pShader->GetFlags() & EF_DECAL) // || pl->m_pObject && (pl->m_pObject->m_ObjFlags & FOB_DECAL))
		{
			if (pTech && pTech->m_nTechnique[TTYPE_Z] > 0 && (pShader && (pShader->m_Flags & EF_SUPPORTSDEFERREDSHADING)))
			{
				nBatchFlags |= FB_Z;
			}

			if (!bShadows && !(pl->m_nCPFlags & CREClientPoly::efShadowGen))
				AddRenderItem(pl, pl->m_pObject, pl->m_Shader, EFSLIST_DECAL, nBatchFlags, pl->rendItemSorter, bShadows, false);
			else if (bShadows && (pl->m_nCPFlags & CREClientPoly::efShadowGen))
				AddRenderItem(pl, pl->m_pObject, pl->m_Shader, defaultList, FB_GENERAL, pl->rendItemSorter, bShadows, false);
		}
		else
		{
			uint32 list = defaultList;
			if (!bShadows && (pl->m_pObject->m_fAlpha < 1.0f || (pShaderResources && pShaderResources->IsTransparent())))
			{
				list = EFSLIST_TRANSP;
			}
			nBatchFlags |= FB_TRANSPARENT;
			AddRenderItem(pl, pl->m_pObject, pl->m_Shader, list, nBatchFlags, pl->rendItemSorter, bShadows, false);
		}
	}
	m_numUsedClientPolygons = 0;
	m_bAddingClientPolys = false;
}

//////////////////////////////////////////////////////////////////////////
void CRenderView::AddShadowFrustumToRender(const SShadowFrustumToRender& frustum)
{
	CRY_ASSERT(frustum.pShadowsView->GetFrameId() == GetFrameId());
	m_shadows.m_renderFrustums.push_back(frustum);
}

//////////////////////////////////////////////////////////////////////////
CRenderView* CRenderView::GetShadowsView(ShadowMapFrustum* pFrustum)
{
	for (auto& fr : m_shadows.m_renderFrustums)
	{
		if (fr.pFrustum == pFrustum)
		{
			return (CRenderView*)fr.pShadowsView.get();
		}
	}
	assert(0); // Should not be used like this
	return 0;
}

//////////////////////////////////////////////////////////////////////////
CRenderView::ShadowFrustumsPtr& CRenderView::GetShadowFrustumsForLight(int lightId)
{
	assert(m_usageMode != eUsageModeWriting); // While writing we must not read back frustums per light.

	auto iter = m_shadows.m_frustumsByLight.find(lightId);
	if (iter != m_shadows.m_frustumsByLight.end())
		return iter->second;
	static ShadowFrustumsPtr empty;
	return empty;
}

CRenderView::ShadowFrustumsPtr& CRenderView::GetShadowFrustumsByType(eShadowFrustumRenderType type)
{
	return m_shadows.m_frustumsByType[type];
}

//////////////////////////////////////////////////////////////////////////
void CRenderView::PostWriteShadowViews()
{
	// Rendering of our shadow views finished, but they may still write items in the jobs in the background
}

//////////////////////////////////////////////////////////////////////////
void CRenderView::AddCustomShadowFrustum(const ShadowMapFrustumPtr& pFrustum, const ShadowMapFrustumPtr& pOriginalFrustum)
{
	// Add fake ShadowsToRender item, for with the original frustum render view
	for (auto& fr : m_shadows.m_renderFrustums)
	{
		if (fr.pFrustum == pOriginalFrustum)
		{
			SShadowFrustumToRender copyFr(fr);
			copyFr.nLightID = -1;
			copyFr.pFrustum = pFrustum;
			m_shadows.m_renderFrustums.push_back(copyFr);
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CRenderView::PrepareShadowViews()
{
	FUNCTION_PROFILER_RENDERER
	for (auto & fr : m_shadows.m_renderFrustums)
	{
		if (fr.pFrustum->m_eFrustumType != ShadowMapFrustum::e_Nearest)
		{
			fr.pShadowsView->SwitchUsageMode(CRenderView::eUsageModeReading);
		}
	}

	m_shadows.PrepareNearestShadows();
	m_shadows.CreateFrustumGroups();
}

//////////////////////////////////////////////////////////////////////////
void CRenderView::DrawCompiledRenderItems(const SGraphicsPipelinePassContext& passContext) const
{
	GetDrawer().DrawCompiledRenderItems(passContext);
}

//////////////////////////////////////////////////////////////////////////
void CRenderView::CheckAndScheduleForUpdate(const SShaderItem& shaderItem)
{
	auto* pSR = reinterpret_cast<CShaderResources*>(shaderItem.m_pShaderResources);
	auto* pShader = reinterpret_cast<CShader*>(shaderItem.m_pShader);

	if (pSR && pShader)
	{
		auto& pRS = pSR->m_pCompiledResourceSet;

		if (pSR->HasDynamicTexModifiers() || (pRS.get() && (pRS->IsDirty() || (pRS->GetFlags() & (CDeviceResourceSet::EFlags_DynamicUpdates | CDeviceResourceSet::EFlags_PendingAllocation)))))
		{
			if (CryInterlockedExchange((volatile LONG*)&pSR->m_nUpdateFrameID, (uint32)m_frameId) != (uint32)m_frameId)
			{
				m_shaderItemsToUpdate.push_back(std::make_pair(pSR, pShader));
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// Sort priority: light id, shadow map type (Dynamic, DynamicDistance, Cached), shadow map lod
struct SCompareFrustumsByLightIds
{
	bool operator()(const SShadowFrustumToRender& rA, const SShadowFrustumToRender& rB) const
	{
		if (rA.nLightID != rB.nLightID)
		{
			return rA.nLightID < rB.nLightID;
		}
		else
		{
			if (rA.pFrustum->m_eFrustumType != rB.pFrustum->m_eFrustumType)
			{
				return int(rA.pFrustum->m_eFrustumType) < int(rB.pFrustum->m_eFrustumType);
			}
			else
			{
				return rA.pFrustum->nShadowMapLod < rB.pFrustum->nShadowMapLod;
			}
		}
	}
};

//////////////////////////////////////////////////////////////////////////
void CRenderView::SShadows::CreateFrustumGroups()
{
	m_frustumsByLight.clear();
	for (auto& frustumList : m_frustumsByType)
		frustumList.resize(0);

	// Go thru frustums render views and make sure we can read from them.
	for (auto& fr : m_renderFrustums)
	{
		m_frustumsByLight[fr.nLightID].push_back(&fr);

		switch (fr.pFrustum->m_eFrustumType)
		{
		case ShadowMapFrustum::e_GsmDynamic:
		case ShadowMapFrustum::e_GsmDynamicDistance:
			{
				if (fr.pLight->m_Flags & DLF_SUN)
					m_frustumsByType[eShadowFrustumRenderType_SunDynamic].push_back(&fr);
				else
					m_frustumsByType[eShadowFrustumRenderType_LocalLight].push_back(&fr);
			}
			break;
		case ShadowMapFrustum::e_GsmCached:
			m_frustumsByType[eShadowFrustumRenderType_SunCached].push_back(&fr);
			break;
		case ShadowMapFrustum::e_HeightMapAO:
			m_frustumsByType[eShadowFrustumRenderType_HeightmapAO].push_back(&fr);
			break;
		case ShadowMapFrustum::e_Nearest:
		case ShadowMapFrustum::e_PerObject:
			m_frustumsByType[eShadowFrustumRenderType_Custom].push_back(&fr);
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CRenderView::SShadows::Clear()
{
	// Force clear call on all dependent shadow views.
	for (auto& fr : m_renderFrustums)
	{
		CRenderView* pShadowView = reinterpret_cast<CRenderView*>(fr.pShadowsView.get());
		pShadowView->Clear();
	}
	m_renderFrustums.resize(0);

	m_frustumsByLight.clear();
	for (auto& frustumList : m_frustumsByType)
		frustumList.resize(0);

	m_pShadowFrustumOwner = nullptr;
}

//////////////////////////////////////////////////////////////////////////
void CRenderView::SShadows::PrepareNearestShadows()
{
	FUNCTION_PROFILER_RENDERER
	SShadowFrustumToRender* pNearestFrustum = nullptr;
	for (auto& fr : m_renderFrustums)
	{
		if (fr.pFrustum->m_eFrustumType == ShadowMapFrustum::e_Nearest)
		{
			pNearestFrustum = &fr;
			break;
		}
	}

	if (pNearestFrustum)
	{
		CRenderView* pNearestShadowsView = reinterpret_cast<CRenderView*>(pNearestFrustum->pShadowsView.get());
		RenderItems& nearestRenderItems = pNearestShadowsView->m_renderItems[0]; // NOTE: rend items go in list 0

		pNearestFrustum->pFrustum->aabbCasters.Reset();

		for (auto& fr : m_renderFrustums)
		{
			if (fr.pFrustum->m_eFrustumType == ShadowMapFrustum::e_GsmDynamic && (fr.pLight->m_Flags & DLF_SUN))
			{
				CRenderView* pShadowsView = reinterpret_cast<CRenderView*>(fr.pShadowsView.get());
				CRY_ASSERT(pShadowsView->m_usageMode == IRenderView::eUsageModeReading);

				for (auto& ri : pShadowsView->GetRenderItems(EFSLIST_NEAREST_OBJECTS))
				{
					if (ri.pObj && ri.pObj->m_pRenderNode)
					{
						AABB objBox;
						ri.pObj->m_pRenderNode->GetLocalBounds(objBox);
						objBox.Move(ri.pObj->GetTranslation());

						nearestRenderItems.lockfree_push_back(ri);

						pNearestFrustum->pFrustum->aabbCasters.Add(objBox);
						pNearestFrustum->pFrustum->nShadowGenMask = 1;
					}
				}
			}
		}

		nearestRenderItems.CoalesceMemory();
		pNearestShadowsView->SwitchUsageMode(CRenderView::eUsageModeReading);
	}
}
