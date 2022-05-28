// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

// -------------------------------------------------------------------------
//  File name:   statobjrend.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: prepare and add render element into renderer
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "StatObj.h"
#include "../RenderDll/Common/Shadow_Renderer.h"

#include "IndexedMesh.h"
#include "VisAreas.h"
#include <CryMath/GeomQuery.h>
#include "DeformableNode.h"

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void CStatObj::RenderInternal(CRenderObject* pRenderObject, hidemask nSubObjectHideMask, const CLodValue& lodValue, const SRenderingPassInfo& passInfo)
{
	FUNCTION_PROFILER_3DENGINE;

	if (m_nFlags & STATIC_OBJECT_HIDDEN)
		return;

	m_nLastDrawMainFrameId = passInfo.GetMainFrameID();
	if (m_pParentObject)
		m_pParentObject->m_nLastDrawMainFrameId = passInfo.GetMainFrameID();

	if (m_nInitialSubObjHideMask != 0 && !nSubObjectHideMask)
	{
		nSubObjectHideMask = m_nInitialSubObjHideMask;
		if ((m_pMergedRenderMesh != NULL) && !(pRenderObject->m_ObjFlags & FOB_MESH_SUBSET_INDICES)) // If not already set by per-instance hide mask.
		{
			SRenderObjData* pOD = pRenderObject->GetObjData();

			// Only pass SubObject hide mask for merged objects, because they have a correct correlation between Hide Mask and Render Chunks.
			pOD->m_nSubObjHideMask = m_nInitialSubObjHideMask;
			pRenderObject->m_ObjFlags |= FOB_MESH_SUBSET_INDICES;
		}
	}

	if (pRenderObject->m_pRenderNode)
	{
		IRenderNode* pRN = (IRenderNode*)pRenderObject->m_pRenderNode;
		if (m_bEditor)
		{
			if (pRN->m_dwRndFlags & ERF_SELECTED)
			{
				m_nSelectedFrameId = passInfo.GetMainFrameID();
				if (m_pParentObject)
					m_pParentObject->m_nSelectedFrameId = passInfo.GetMainFrameID();
				pRenderObject->m_ObjFlags |= FOB_SELECTED;
			}
			else
				pRenderObject->m_ObjFlags &= ~FOB_SELECTED;

			if (!gEnv->IsEditing() && pRN->m_dwRndFlags & ERF_RAYCAST_PROXY)
			{
				return;
			}
		}
		else
		{
			if (pRN->m_dwRndFlags & ERF_RAYCAST_PROXY)
			{
				return;
			}
		}
	}

#ifdef SEG_WORLD
	if (GetISystem()->GetIConsole()->GetCVar("sw_debugInfo")->GetIVal() == 4)
	{
		pRenderObject->m_ObjFlags |= FOB_SELECTED;
	}
#endif //SEG_WORLD

	if ((m_nFlags & STATIC_OBJECT_COMPOUND) && !m_bMerged)
	{
		//////////////////////////////////////////////////////////////////////////
		// Render SubMeshes if present.
		//////////////////////////////////////////////////////////////////////////
		if (m_nSubObjectMeshCount > 0)
		{
			CRenderObject* pRenderObjectB = NULL;

			if (lodValue.DissolveRefB() != 255)
			{
				pRenderObject->m_DissolveRef = lodValue.DissolveRefA();

				if (pRenderObject->m_DissolveRef)
				{
					if (!(pRenderObject->m_ObjFlags & FOB_DISSOLVE))
						pRenderObject->m_ObjFlags &= ~FOB_UPDATED_RTMASK;
					pRenderObject->m_ObjFlags |= FOB_DISSOLVE;

					pRenderObject->m_ObjFlags |= FOB_DISSOLVE_OUT;
				}
				else
				{
					if ((pRenderObject->m_ObjFlags & FOB_DISSOLVE))
						pRenderObject->m_ObjFlags &= ~FOB_UPDATED_RTMASK;
					pRenderObject->m_ObjFlags &= ~FOB_DISSOLVE;
				}

				if (lodValue.LodB() != -1)
				{
					pRenderObjectB = GetRenderer()->EF_DuplicateRO(pRenderObject, passInfo);
					pRenderObjectB->m_ObjFlags &= ~FOB_DISSOLVE_OUT;
				}
			}
			else
			{
				pRenderObject->m_DissolveRef = 0;
				if ((pRenderObject->m_ObjFlags & FOB_DISSOLVE))
					pRenderObject->m_ObjFlags &= ~FOB_UPDATED_RTMASK;
				pRenderObject->m_ObjFlags &= ~(FOB_DISSOLVE | FOB_DISSOLVE_OUT);
			}

			hidemaskOneBit nBitIndex = hidemask1;
			Matrix34A renderTM = pRenderObject->m_II.m_Matrix;
			for (int32 i = 0, subObjectsSize = m_subObjects.size(); i < subObjectsSize; ++i, nBitIndex <<= 1)
			{
				const SSubObject& subObj = m_subObjects[i];
				if (subObj.nType == STATIC_SUB_OBJECT_MESH)  // all the meshes are at the beginning of the array.
				{
					CStatObj* const __restrict pStatObj = (CStatObj*)subObj.pStatObj;

					if (pStatObj &&
					    (pStatObj->m_nRenderTrisCount >= 2) &&
					    !(pStatObj->m_nFlags & STATIC_OBJECT_HIDDEN) &&
					    !(nSubObjectHideMask & nBitIndex)
					    )
					{
						PrefetchLine(pRenderObject, 0);
						RenderSubObject(pRenderObject, lodValue.LodA(), i, renderTM, passInfo);
						if (pRenderObjectB)
						{
							PrefetchLine(pRenderObjectB, 0);
							RenderSubObject(pRenderObjectB, lodValue.LodB(), i, renderTM, passInfo);
						}
					}
				}
				else
				{
					break;
				}
			}

			if (GetCVars()->e_DebugDraw)
			{
				RenderDebugInfo(pRenderObject, passInfo);
			}
		}

		//////////////////////////////////////////////////////////////////////////
	}
	else
	{
		// draw mesh, don't even try to render childs
		RenderObjectInternal(pRenderObject, lodValue.LodA(), lodValue.DissolveRefA(), true, passInfo);
		if (lodValue.DissolveRefB() != 255) // check here since we're passing in A's ref.
		{
			pRenderObject = GetRenderer()->EF_DuplicateRO(pRenderObject, passInfo);
			RenderObjectInternal(pRenderObject, lodValue.LodB(), lodValue.DissolveRefA(), false, passInfo);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
void CStatObj::RenderSubObject(CRenderObject* pRenderObject, int nLod,
                               int nSubObjId, const Matrix34A& renderTM, const SRenderingPassInfo& passInfo)
{
	const SSubObject& subObj = m_subObjects[nSubObjId];

	CStatObj* const __restrict pStatObj = (CStatObj*)subObj.pStatObj;

	if (pStatObj == NULL)
		return;

	SRenderObjData* pOD = 0;
	if (subObj.pFoliage)
	{
		pRenderObject = GetRenderer()->EF_DuplicateRO(pRenderObject, passInfo);
		pOD = pRenderObject->GetObjData();
		pOD->m_pSkinningData = subObj.pFoliage->GetSkinningData(pRenderObject->m_II.m_Matrix, passInfo);
		pOD->m_uniqueObjectId = reinterpret_cast<uintptr_t>(subObj.pFoliage);
		pRenderObject->m_ObjFlags |= FOB_SKINNED | FOB_DYNAMIC_OBJECT;
		((CStatObjFoliage*)subObj.pFoliage)->m_pRenderObject = pRenderObject;
	}

	if (subObj.bIdentityMatrix)
	{
		pStatObj->RenderSubObjectInternal(pRenderObject, nLod, passInfo);
	}
	else
	{
		pRenderObject = GetRenderer()->EF_DuplicateRO(pRenderObject, passInfo);
		pRenderObject->m_II.m_Matrix = renderTM * subObj.tm;
		SRenderObjData* pRenderObjectData = pRenderObject->GetObjData();
		pRenderObjectData->m_uniqueObjectId = pRenderObjectData->m_uniqueObjectId + nSubObjId;

		pStatObj->RenderSubObjectInternal(pRenderObject, nLod, passInfo);
	}
}

///////////////////////////////////////////////////////////////////////////////
void CStatObj::RenderSubObjectInternal(CRenderObject* pRenderObject, int nLod, const SRenderingPassInfo& passInfo)
{
	assert(!(m_nFlags & STATIC_OBJECT_HIDDEN));
	assert(m_nRenderTrisCount);

	m_nLastDrawMainFrameId = passInfo.GetMainFrameID();
	if (m_pParentObject && (m_nFlags & STATIC_OBJECT_MULTIPLE_PARENTS))
		m_pParentObject->m_nLastDrawMainFrameId = passInfo.GetMainFrameID();

	assert(!m_pParentObject || m_pParentObject->m_nLastDrawMainFrameId == passInfo.GetMainFrameID());

	assert(!(m_nFlags & STATIC_OBJECT_COMPOUND));

	nLod = CLAMP(nLod, GetMinUsableLod(), (int)m_nMaxUsableLod);
	assert(nLod < MAX_STATOBJ_LODS_NUM);

	// Skip rendering of this suboject if it is marked as deformable
	if (GetCVars()->e_MergedMeshes == 1 && nLod == 0 && m_isDeformable)
		return;

	// try next lod's if selected one is not ready
	if ((!nLod && m_pRenderMesh && m_pRenderMesh->CanRender()) || !GetCVars()->e_Lods)
	{
		PrefetchLine(pRenderObject, 0);
		RenderRenderMesh(pRenderObject, NULL, passInfo);
	}
	else
	{
		if (m_pLODs && m_pLODs[nLod])
		{
			m_pLODs[nLod]->m_nLastDrawMainFrameId = passInfo.GetMainFrameID();
			if (m_pLODs[nLod]->m_pParentObject)
				m_pLODs[nLod]->m_pParentObject->m_nLastDrawMainFrameId = passInfo.GetMainFrameID();

			if ((nLod + 1) < MAX_STATOBJ_LODS_NUM && m_pLODs[nLod + 1])
			{
				m_pLODs[nLod + 1]->m_nLastDrawMainFrameId = passInfo.GetMainFrameID();
				if (m_pLODs[nLod + 1]->m_pParentObject)
					m_pLODs[nLod + 1]->m_pParentObject->m_nLastDrawMainFrameId = passInfo.GetMainFrameID();
			}
		}

		if (m_pLODs)
			for (; nLod <= (int)m_nMaxUsableLod; nLod++)
			{
				if (m_pLODs[nLod] && m_pLODs[nLod]->m_pRenderMesh && m_pLODs[nLod]->m_pRenderMesh->CanRender())
				{
					PrefetchLine(pRenderObject, 0);
					m_pLODs[nLod]->RenderRenderMesh(pRenderObject, NULL, passInfo);
					break;
				}
			}
	}
}

///////////////////////////////////////////////////////////////////////////////
void CStatObj::RenderObjectInternal(CRenderObject* pRenderObject, int nTargetLod, uint8 uLodDissolveRef, bool dissolveOut, const SRenderingPassInfo& passInfo)
{
	if (nTargetLod == -1 || uLodDissolveRef == 255)
	{
		return;
	}

	int nLod = CLAMP(nTargetLod, GetMinUsableLod(), (int)m_nMaxUsableLod);
	assert(nLod < MAX_STATOBJ_LODS_NUM);

	// Skip rendering of this suboject if it is marked as deformable
	if (GetCVars()->e_MergedMeshes == 1 && nTargetLod == 0 && m_isDeformable)
		return;

	if (passInfo.IsShadowPass() && passInfo.GetShadowMapType() == SRenderingPassInfo::SHADOW_MAP_CACHED && pRenderObject->m_pRenderNode)
	{
		IShadowCaster* pCaster = static_cast<IShadowCaster*>(pRenderObject->m_pRenderNode);
		pCaster->m_cStaticShadowLod = nLod;
	}

	pRenderObject->m_DissolveRef = uLodDissolveRef;

	if (pRenderObject->m_DissolveRef)
	{
		if (!(pRenderObject->m_ObjFlags & FOB_DISSOLVE))
			pRenderObject->m_ObjFlags &= ~FOB_UPDATED_RTMASK;
		pRenderObject->m_ObjFlags |= FOB_DISSOLVE;

		if (dissolveOut)
			pRenderObject->m_ObjFlags |= FOB_DISSOLVE_OUT;
	}
	else
	{
		if ((pRenderObject->m_ObjFlags & FOB_DISSOLVE))
			pRenderObject->m_ObjFlags &= ~FOB_UPDATED_RTMASK;
		pRenderObject->m_ObjFlags &= ~FOB_DISSOLVE;
	}

	// try next lod's if selected one is not ready
	if ((!nLod && m_pRenderMesh && m_pRenderMesh->CanRender()) || !GetCVars()->e_Lods)
	{
		PrefetchLine(pRenderObject, 0);
		RenderRenderMesh(pRenderObject, NULL, passInfo);
	}
	else
	{
		if (m_pLODs && m_pLODs[nLod])
		{
			m_pLODs[nLod]->m_nLastDrawMainFrameId = passInfo.GetMainFrameID();
			if (m_pLODs[nLod]->m_pParentObject)
				m_pLODs[nLod]->m_pParentObject->m_nLastDrawMainFrameId = passInfo.GetMainFrameID();

			if ((nLod + 1) < MAX_STATOBJ_LODS_NUM && m_pLODs[nLod + 1])
			{
				m_pLODs[nLod + 1]->m_nLastDrawMainFrameId = passInfo.GetMainFrameID();
				if (m_pLODs[nLod + 1]->m_pParentObject)
					m_pLODs[nLod + 1]->m_pParentObject->m_nLastDrawMainFrameId = passInfo.GetMainFrameID();
			}
		}

		if (m_pLODs)
			for (; nLod <= (int)m_nMaxUsableLod; nLod++)
			{
				if (m_pLODs[nLod] && m_pLODs[nLod]->m_pRenderMesh && m_pLODs[nLod]->m_pRenderMesh->CanRender())
				{
					PrefetchLine(pRenderObject, 0);
					m_pLODs[nLod]->RenderRenderMesh(pRenderObject, NULL, passInfo);
					break;
				}
			}
	}
}

///////////////////////////////////////////////////////////////////////////////
void CStatObj::RenderRenderMesh(CRenderObject* pRenderObject, SInstancingInfo* pInstInfo, const SRenderingPassInfo& passInfo)
{
#if !defined(_RELEASE)
	//DEBUG - filter which stat objs are rendered
	if (GetCVars()->e_statObjRenderFilterMode && GetCVars()->e_pStatObjRenderFilterStr && GetCVars()->e_pStatObjRenderFilterStr[0])
	{
		if (GetCVars()->e_statObjRenderFilterMode == 1)
		{
			//only render elems containing str
			if (!strstr(m_szFileName.c_str(), GetCVars()->e_pStatObjRenderFilterStr))
			{
				return;
			}
		}
		else if (GetCVars()->e_statObjRenderFilterMode == 2)
		{
			//exclude elems containing str
			if (strstr(m_szFileName.c_str(), GetCVars()->e_pStatObjRenderFilterStr))
			{
				return;
			}
		}
	}

	if (GetCVars()->e_DebugDraw && (!GetCVars()->e_DebugDrawShowOnlyCompound || (m_bSubObject || m_pParentObject)))
	{
		int nLod = 0;
		if (m_pLod0 && m_pLod0->m_pLODs)
			for (; nLod < MAX_STATOBJ_LODS_NUM; nLod++)
			{
				if (m_pLod0->m_pLODs[nLod] == this)
				{
					m_pRenderMesh->SetMeshLod(nLod);
					break;
				}
			}

		if (GetCVars()->e_DebugDrawShowOnlyLod >= 0)
			if (GetCVars()->e_DebugDrawShowOnlyLod != nLod)
				return;

		if (RenderDebugInfo(pRenderObject, passInfo))
			return;

		if (m_bSubObject)
			pRenderObject = gEnv->pRenderer->EF_DuplicateRO(pRenderObject, passInfo);
	}

	if (!passInfo.IsShadowPass())
	{
		if (GetCVars()->e_StreamCgfDebug == 1)
		{
			RenderStreamingDebugInfo(pRenderObject);
		}

		if (GetCVars()->e_CoverCgfDebug == 1)
		{
			RenderCoverInfo(pRenderObject);
		}
	}
#endif

	if (m_pRenderMesh)
	{
		CRenderObject* pObj = pRenderObject;
#if !defined(_RELEASE)
		if (m_isProxyTooBig)
		{
			pObj = GetRenderer()->EF_DuplicateRO(pRenderObject, passInfo);
			pObj->m_pCurrMaterial = m_pMaterial;
		}
#endif
		m_pRenderMesh->Render(pObj, passInfo);
	}
}

///////////////////////////////////////////////////////////////////////////////
int CStatObj::GetMaxUsableLod()
{
	int maxUsable = m_pLod0 ? max((int)m_nMaxUsableLod, (int)m_pLod0->m_nMaxUsableLod) : (int)m_nMaxUsableLod;
	return min(maxUsable, GetCVars()->e_LodMax);
}

///////////////////////////////////////////////////////////////////////////////
int CStatObj::GetMinUsableLod()
{
	int minUsable = m_pLod0 ? max((int)m_nMinUsableLod0, (int)m_pLod0->m_nMinUsableLod0) : (int)m_nMinUsableLod0;
	return max(minUsable, GetCVars()->e_LodMin);
}

///////////////////////////////////////////////////////////////////////////////
int CStatObj::FindNearesLoadedLOD(int nLodIn, bool bSearchUp)
{
	// make sure requested lod is loaded
	/*  if(CStatObj * pObjForStreamIn = nLodIn ? m_pLODs[nLodIn] : this)
	   {
	    bool bRenderNodeValid(rParams.pRenderNode && ((int)(void*)(rParams.pRenderNode)>0) && rParams.pRenderNode->m_fWSMaxViewDist);
	    float fImportance = bRenderNodeValid ? (1.f - (rParams.fDistance / rParams.pRenderNode->m_fWSMaxViewDist)) : 0.5f;
	    pObjForStreamIn->UpdateStreamingPrioriryInternal(fImportance);
	   }*/

	// if requested lod is not ready - find nearest ready one
	int nLod = nLodIn;

	if (nLod == 0 && !GetRenderMesh())
		nLod++;

	while (nLod && nLod < MAX_STATOBJ_LODS_NUM && (!m_pLODs || !m_pLODs[nLod] || !m_pLODs[nLod]->GetRenderMesh()))
		nLod++;

	if (nLod > (int)m_nMaxUsableLod)
	{
		if (bSearchUp)
		{
			nLod = min((int)m_nMaxUsableLod, nLodIn);

			while (nLod && (!m_pLODs || !m_pLODs[nLod] || !m_pLODs[nLod]->GetRenderMesh()))
				nLod--;

			if (nLod == 0 && !GetRenderMesh())
				nLod--;
		}
		else
		{
			nLod = -1;
		}
	}

	return nLod;
}

//////////////////////////////////////////////////////////////////////////
int CStatObj::AddRef()
{
	return CryInterlockedIncrement(&m_nUsers);
}
