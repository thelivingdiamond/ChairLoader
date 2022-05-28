// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#include "StdAfx.h"
#include "DriverD3D.h"
#include <Cry3DEngine/I3DEngine.h>
#include "D3DPostProcess.h"
#include "D3DLightPropagationVolume.h"
#include "../Common/Textures/TextureManager.h"
#include "../Common/Textures/TextureHelpers.h"
#include "../Common/RendElements/FlareSoftOcclusionQuery.h"
#include "../Common/ReverseDepth.h"
#include "D3DTiledShading.h"
#if defined(FEATURE_SVO_GI)
	#include "D3D_SVO.h"
#endif

#include "Common/RenderView.h"

#define MAX_VIS_AREAS 32

// MSAA potential optimizations todo:
//  - long term: port all functionality to compute, including all extra effects passes.

// About MSAA:
// - Please be careful when accessing or rendering into msaa'ed targets. When adding new techniques please make sure to test
// - For post process technique to be MSAA friendly, do either:
//    - Use compute. Single pass and as efficient as gets. Context switches might be problematic, until all lighting pipeline done like this.
//    - For non compute, require 2 passes. One at pixel frequency, other at sub sample frequency.
//				 - Reuse existing sample frequency regions on stencil via stencilread/write mask:
//						- If not possible, tag pixel frequency regions using stencil + m_pMSAAMaskRT
//						- Alternative poor man version, do clip in shader.

#pragma warning(disable: 4244)

CDeferredShading* CDeferredShading::m_pInstance = NULL;

#define RT_LIGHTSMASK                     g_HWSR_MaskBit[HWSR_SAMPLE0] | g_HWSR_MaskBit[HWSR_SAMPLE1] | g_HWSR_MaskBit[HWSR_SAMPLE2] | g_HWSR_MaskBit[HWSR_SAMPLE3] | g_HWSR_MaskBit[HWSR_SAMPLE4] | g_HWSR_MaskBit[HWSR_SAMPLE5] | g_HWSR_MaskBit[HWSR_LIGHT_TEX_PROJ] | g_HWSR_MaskBit[HWSR_CUBEMAP0]
#define RT_LIGHTPASS_RESETMASK            g_HWSR_MaskBit[HWSR_SAMPLE0] | g_HWSR_MaskBit[HWSR_SAMPLE1] | g_HWSR_MaskBit[HWSR_SAMPLE2] | g_HWSR_MaskBit[HWSR_SAMPLE3] | g_HWSR_MaskBit[HWSR_SAMPLE5] | g_HWSR_MaskBit[HWSR_LIGHT_TEX_PROJ] | g_HWSR_MaskBit[HWSR_CUBEMAP0]
#define RT_DEBUGMASK                      g_HWSR_MaskBit[HWSR_DEBUG0] | g_HWSR_MaskBit[HWSR_DEBUG1] | g_HWSR_MaskBit[HWSR_DEBUG2] | g_HWSR_MaskBit[HWSR_DEBUG3]
#define RT_TEX_PROJECT                    g_HWSR_MaskBit[HWSR_SAMPLE0]
#define RT_GLOBAL_CUBEMAP                 g_HWSR_MaskBit[HWSR_SAMPLE0]
#define RT_SPECULAR_CUBEMAP               g_HWSR_MaskBit[HWSR_SAMPLE1]
#define RT_AMBIENT_LIGHT                  g_HWSR_MaskBit[HWSR_SAMPLE5]
#define RT_GLOBAL_CUBEMAP_IGNORE_VISAREAS g_HWSR_MaskBit[HWSR_SAMPLE4]
#define RT_AREALIGHT                      g_HWSR_MaskBit[HWSR_SAMPLE1]
#define RT_OVERDRAW_DEBUG                 g_HWSR_MaskBit[HWSR_DEBUG0]
#define RT_BOX_PROJECTION                 g_HWSR_MaskBit[HWSR_SAMPLE3]
#define RT_CLIPVOLUME_ID                  g_HWSR_MaskBit[HWSR_LIGHTVOLUME0]

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void CTexPoolAtlas::Init(int nSize)
{
	m_nSize = nSize;
	Clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void CTexPoolAtlas::Clear()
{
	memset(&m_arrAllocatedBlocks[0], 0, MAX_BLOCKS * sizeof(m_arrAllocatedBlocks[0]));
#ifdef _DEBUG
	m_nTotalWaste = 0;
	m_arrDebugBlocks.clear();
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void CTexPoolAtlas::FreeMemory()
{
#ifdef _DEBUG
	stl::free_container(m_arrDebugBlocks);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

bool CTexPoolAtlas::AllocateGroup(int32* pOffsetX, int32* pOffsetY, int nSizeX, int nSizeY)
{
	int nBorder = 2;
	nSizeX += nBorder << 1;
	nSizeY += nBorder << 1;

	if (nSizeX > m_nSize || nSizeY > m_nSize)
		return false;

	uint16 nBestX = 0;
	uint16 nBestY = 0;
	uint16 nBestID = 0;
	uint32 nBestWaste = ~0;

	// outer loop over all relevant allocated blocks (Y dimension)
	int nCurrY = 0;
	for (int nCurrBlockID = 0;
	     m_arrAllocatedBlocks[max(nCurrBlockID - 1, 0)] > 0 && nCurrY <= m_nSize - nSizeY && nBestWaste > 0;
	     ++nCurrBlockID)
	{
		const uint32 nCurrBlock = m_arrAllocatedBlocks[nCurrBlockID];
		const uint16 nCurrBlockWidth = LOWORD(nCurrBlock);
		const uint16 nCurrBlockHeight = HIWORD(nCurrBlock);

		// get max X for intersected blocks
		int nCurrX = nCurrBlockWidth;
		int nNextY = nCurrBlockHeight;
		for (uint16 nNextBlockID = nCurrBlockID + 1;
		     m_arrAllocatedBlocks[nNextBlockID] > 0 && nNextY < nSizeY;
		     ++nNextBlockID)
		{
			const uint32 nNextBlock = m_arrAllocatedBlocks[nNextBlockID];
			const uint16 nNextBlockWidth = LOWORD(nNextBlock);
			const uint16 nNextBlockHeight = HIWORD(nNextBlock);
			nCurrX = max(nCurrX, int(nNextBlockWidth));
			nNextY += nNextBlockHeight;
		}

		// can fit next to them?
		if (nSizeX <= m_nSize - nCurrX)
		{
			// compute waste
			uint32 nWaste = 0;
			nNextY = nCurrY;
			for (uint16 nNextBlockID = nCurrBlockID;
			     m_arrAllocatedBlocks[nNextBlockID] > 0 && nNextY < nCurrY + nSizeY;
			     ++nNextBlockID)
			{
				const uint32 nNextBlock = m_arrAllocatedBlocks[nNextBlockID];
				const uint16 nNextBlockWidth = LOWORD(nNextBlock);
				const uint16 nNextBlockHeight = HIWORD(nNextBlock);
				nWaste += (nCurrX - nNextBlockWidth) * (min(nCurrY + nSizeY, nNextY + nNextBlockHeight) - max(nCurrY, nNextY));
				nNextY += nNextBlockHeight;
			}
			nWaste += max(nCurrY + nSizeY - nNextY, 0) * nCurrX;

			// right spot?
			if (nWaste < nBestWaste)
			{
				nBestX = nCurrX;
				nBestY = nCurrY;
				nBestID = nCurrBlockID;
				nBestWaste = nWaste;
			}
		}

		nCurrY += nCurrBlockHeight;
	}

	if ((nBestX | nBestY) || nCurrY <= m_nSize - nSizeY)
	{
		assert(nBestID < MAX_BLOCKS - 1);
		if (nBestID >= MAX_BLOCKS - 1)
			return false;

		*pOffsetX = nBestX + nBorder;
		*pOffsetY = nBestY + nBorder;

		// block to be added, update block info
		uint32 nBlockData = m_arrAllocatedBlocks[nBestID];
		const uint16 nReplacedHeight = HIWORD(nBlockData);
		if (nSizeY < nReplacedHeight)
		{
			nBlockData = MAKELONG(nBlockData, nReplacedHeight - nSizeY);
			// shift by 1
			for (uint16 nID = nBestID + 1; nBlockData > 0; ++nID)
				std::swap(m_arrAllocatedBlocks[nID], nBlockData);
		}
		else if (nSizeY > nReplacedHeight)
		{
			uint16 nCoveredHeight = nReplacedHeight;
			uint16 nBlocksToSkip = 0;
			nBlockData = m_arrAllocatedBlocks[nBestID + 1];
			for (uint16 nID = nBestID + 1; nBlockData > 0; nBlockData = m_arrAllocatedBlocks[++nID])
			{
				const uint16 nCurrHeight = HIWORD(nBlockData);
				nCoveredHeight += nCurrHeight;
				if (nSizeY >= nCoveredHeight)
					nBlocksToSkip++;
				else
				{
					m_arrAllocatedBlocks[nID] = MAKELONG(nBlockData, nCoveredHeight - nSizeY);
					break;
				}
			}
			// shift by nBlocksToSkip
			uint16 nID = nBestID + nBlocksToSkip + 1;
			nBlockData = m_arrAllocatedBlocks[nID];
			for (; nBlockData > 0; nBlockData = m_arrAllocatedBlocks[++nID])
				m_arrAllocatedBlocks[nID - nBlocksToSkip] = nBlockData;
		}
		m_arrAllocatedBlocks[nBestID] = MAKELONG(nBestX + nSizeX, nSizeY);

#ifdef _DEBUG
		if (m_arrDebugBlocks.size())
			m_nTotalWaste += nBestWaste;
		_AddDebugBlock(nBestX, nBestY, nSizeX, nSizeY);
#endif

		return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
void CTexPoolAtlas::_AddDebugBlock(int x, int y, int sizeX, int sizeY)
{
	SShadowMapBlock block;
	block.m_nX1 = x;
	block.m_nX2 = x + sizeX;
	block.m_nY1 = y;
	block.m_nY2 = y + sizeY;
	assert(block.m_nX2 <= m_nSize && block.m_nY2 <= m_nSize);
	for (std::vector<SShadowMapBlock>::const_iterator it = m_arrDebugBlocks.begin();
	     it != m_arrDebugBlocks.end(); it++)
	{
		assert(!block.Intersects(*it));
	}
	m_arrDebugBlocks.push_back(block);
}

float CTexPoolAtlas::_GetDebugUsage() const
{
	uint32 nUsed = 0;
	for (std::vector<SShadowMapBlock>::const_iterator it = m_arrDebugBlocks.begin();
	     it != m_arrDebugBlocks.end(); it++)
	{
		nUsed += (it->m_nX2 - it->m_nX1) * (it->m_nY2 - it->m_nY1);
	}

	return 100.f * nUsed / (m_nSize * m_nSize);
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

uint32 CDeferredShading::AddLight(const CDLight& pDL, float fMult, const SRenderingPassInfo& passInfo)
{
	const uint32 nThreadID = gcpRendD3D->m_RP.m_nFillThreadID;

	bool bSort = true;
	bool bAppend = true;

	eDeferredLightType LightType = eDLT_DeferredLight;
	if (pDL.m_Flags & DLF_DEFERRED_CUBEMAPS)
	{
		LightType = eDLT_DeferredCubemap;
		bSort = false;
	}
	else if (pDL.m_Flags & DLF_AMBIENT)
	{
		LightType = eDLT_DeferredAmbientLight;
		bSort = false;
	}

	if (pDL.GetLensOpticsElement() && !pDL.m_pSoftOccQuery)
	{
		CDLight* pLight = const_cast<CDLight*>(&pDL);

		const uint8 numVisibilityFaders = 2; // For each flare type
		pLight->m_pSoftOccQuery = new CFlareSoftOcclusionQuery(numVisibilityFaders);
	}

	SRenderLight* pAddedLight = nullptr;

	const int32 lightsNum = passInfo.GetRenderView()->GetLightsCount(LightType);
	int idx = -1;

	if (CRenderer::CV_r_DeferredShadingSortLights == 1 && bSort && lightsNum)
	{
		int endPoint = lightsNum - 1;
		int midPoint = 0;
		int startPoint = 0;

		uint32 lightArea = pDL.m_sWidth * pDL.m_sHeight;

		while (startPoint <= endPoint)  //binary search for insertion point
		{
			midPoint = (endPoint + startPoint) / 2;

			SRenderLight& compareLight = passInfo.GetRenderView()->GetLight(LightType, midPoint);
			uint32 compareLightArea = compareLight.m_sWidth * compareLight.m_sHeight;

			if (lightArea < compareLightArea)
			{
				endPoint = midPoint - 1;
			}
			else
			{
				startPoint = midPoint + 1;
			}
		}

		if (startPoint < lightsNum)
		{
			idx = startPoint;
			pAddedLight = passInfo.GetRenderView()->AddLightAtIndex(LightType, pDL, startPoint);
			bAppend = false;
		}
	}

	if (bAppend)
	{
		pAddedLight = passInfo.GetRenderView()->AddLightAtIndex(LightType, pDL, -1);
		idx = passInfo.GetRenderView()->GetLightsCount(LightType) - 1;
	}

	IF_LIKELY ((pDL.m_Flags & (DLF_DEFERRED_CUBEMAPS | DLF_AMBIENT)) == 0)
	{
		pAddedLight->m_Color *= fMult;
		pAddedLight->m_SpecMult *= fMult;
	}
	else if (pDL.m_Flags & DLF_AMBIENT)
	{
		ColorF origCol(pAddedLight->m_Color);
		pAddedLight->m_Color.lerpFloat(Col_White, origCol, fMult);
	}
	else
	{
		pAddedLight->m_Color.a = fMult; // store fade-out constant separately in alpha channel for deferred cubemaps
	}

	gcpRendD3D->EF_CheckLightMaterial(const_cast<CDLight*>(&pDL), idx, passInfo);

	return idx;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDeferredShading::GetClipVolumeParams(const Vec4*& pParams, uint32& nCount)
{
	pParams = m_vClipVolumeParams;
	nCount = RenderView()->GetClipVolumes().size();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void CDeferredShading::ResetLights()
{
	uint32 nThreadID = gcpRendD3D->m_RP.m_nFillThreadID;

	m_vecGIClipVolumes[nThreadID][0].resize(0);

	gcpRendD3D->GetVolumetricFog().Clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void CDeferredShading::ReleaseData()
{
	gcpRendD3D->GetTiledShading().Clear();
	gcpRendD3D->GetVolumetricFog().ClearAll();

	for (uint32 iThread = 0; iThread < 2; ++iThread)
	{
		for (uint32 nRecurseLevel = 0; nRecurseLevel < MAX_REND_RECURSION_LEVELS; ++nRecurseLevel)
		{
			m_vecGIClipVolumes[iThread][nRecurseLevel].clear();
		}
	}

	LPVManager.Cleanup();

	m_shadowPoolAlloc.SetUse(0);
	stl::free_container(m_shadowPoolAlloc);

	m_blockPack.FreeContainers();

	m_nShadowPoolSize = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void CDeferredShading::AddGIClipVolume(IRenderMesh* pClipVolume, const Matrix34& mxTransform)
{
	uint32 nThreadID = gRenDev->m_RP.m_nFillThreadID;

	TArray<SClipShape>& vecGIClipVolumes = m_vecGIClipVolumes[nThreadID][0];
	vecGIClipVolumes.push_back(SClipShape(pClipVolume, mxTransform));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void CDeferredShading::SpecularAccEnableMRT(bool bEnable)
{
	assert(m_pLBufferSpecularRT);

	CD3D9Renderer* const __restrict rd = gcpRendD3D;

	if (bEnable && !m_bSpecularState)
	{
		rd->FX_PushRenderTarget(1, m_pLBufferSpecularRT, NULL, -1, false, 1);
		m_bSpecularState = true;
	}
	else if (!bEnable && m_bSpecularState)
	{
		m_pLBufferSpecularRT->SetResolved(true);
		rd->FX_PopRenderTarget(1);
		m_bSpecularState = false;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void CDeferredShading::SetupPasses()
{
	CreateDeferredMaps();

	m_nThreadID = gcpRendD3D->m_RP.m_nProcessThreadID;

	m_nBindResourceMsaa = gcpRendD3D->m_RP.m_MSAAData.Type ? SResourceView::DefaultViewMS : SResourceView::DefaultView;

	gcpRendD3D->m_RP.m_FlagsShader_RT &= ~(RT_LIGHTSMASK | RT_DEBUGMASK);

	gcpRendD3D->FX_SetMSAAFlagsRT();

	m_pLBufferDiffuseRT = CTexture::s_ptexCurrentSceneDiffuseAccMap;
	m_pLBufferSpecularRT = CTexture::s_ptexSceneSpecularAccMap;
	m_pNormalsRT = CTexture::s_ptexSceneNormalsMap;
	m_pDepthRT = CTexture::s_ptexZTarget;
	m_pResolvedStencilRT = CTexture::s_ptexVelocity;
	m_pDiffuseRT = CTexture::s_ptexSceneDiffuse;
	m_pSpecularRT = CTexture::s_ptexSceneSpecular;
	m_pMSAAMaskRT = CTexture::s_ptexBackBuffer;

	m_nTexStateLinear = CTexture::GetTexState(STexState(FILTER_LINEAR, true));
	m_nTexStatePoint = CTexture::GetTexState(STexState(FILTER_POINT, true));

	CRenderCamera* pCam = &gcpRendD3D->m_RP.m_TI[m_nThreadID].m_rcam;
	m_pCamFront = pCam->vZ;
	m_pCamFront.Normalize();
	m_pCamPos = pCam->vOrigin;

	m_fCamFar = pCam->fFar;
	m_fCamNear = pCam->fNear;

	m_fRatioWidth = (float)m_pLBufferDiffuseRT->GetWidth() / (float)CTexture::s_ptexSceneTarget->GetWidth();
	m_fRatioHeight = (float)m_pLBufferDiffuseRT->GetHeight() / (float)CTexture::s_ptexSceneTarget->GetHeight();

	m_pView = gcpRendD3D->m_CameraMatrix;
	m_pView.Transpose();

	m_mViewProj = gcpRendD3D->m_CameraProjMatrix;
	m_mViewProj.Transpose();

	m_pViewProjI = gcpRendD3D->m_CameraProjMatrix.GetInverted();

	gRenDev->m_cEF.mfRefreshSystemShader("DeferredShading", CShaderMan::s_shDeferredShading);
	m_pShader = CShaderMan::s_shDeferredShading;

	gcpRendD3D->SetCullMode(R_CULL_BACK);

	if (CRenderer::CV_r_DeferredShadingDepthBoundsTest <= 1)
		m_nRenderState |= GS_NODEPTHTEST;
	else
		m_nRenderState &= ~GS_NODEPTHTEST;

	if (CRenderer::CV_r_DeferredShadingDebug == 2)
	{
		gcpRendD3D->m_RP.m_FlagsShader_RT |= RT_OVERDRAW_DEBUG;
	}

	m_nCurTargetWidth = m_pLBufferDiffuseRT->GetWidth();
	m_nCurTargetHeight = m_pLBufferDiffuseRT->GetHeight();

	// Verify if sun present in non-deferred light list (usually fairly small list)
	gcpRendD3D->m_RP.m_pSunLight = NULL;

	if (CRenderView* pView = RenderView())
	{
		for (uint32 i = 0; i < pView->GetDynamicLightsCount(); i++)
		{
			SRenderLight* dl = &pView->GetDynamicLight(i);
			if (dl->m_Flags & DLF_SUN)
			{
				gcpRendD3D->m_RP.m_pSunLight = dl;
				break;
			}
		}
	}

	SetupGlobalConsts();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void CDeferredShading::SetupGlobalConsts()
{
	CD3D9Renderer* const __restrict rd = gcpRendD3D;

	//set world basis
	float maskRTWidth = float(m_nCurTargetWidth);
	float maskRTHeight = float(m_nCurTargetHeight);
	Vec4r vWBasisX, vWBasisY, vWBasisZ, vCamPos;
	Vec4 vParamValue, vMag;
	CShadowUtils::ProjectScreenToWorldExpansionBasis(rd->m_IdentityMatrix, rd->GetCamera(), rd->m_vProjMatrixSubPixoffset, maskRTWidth, maskRTHeight, vWBasisX, vWBasisY, vWBasisZ, vCamPos, true, NULL);

	vWorldBasisX = vWBasisX / rd->m_RP.m_CurDownscaleFactor.x;
	vWorldBasisY = vWBasisY / rd->m_RP.m_CurDownscaleFactor.y;
	vWorldBasisZ = vWBasisZ;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void CDeferredShading::FilterGBuffer()
{
	if (!CRenderer::CV_r_DeferredShadingFilterGBuffer)
	{
#if defined(DURANGO_USE_ESRAM)
		if (gcpRendD3D->m_nGraphicsPipeline == 0 || CRenderer::CV_r_ssdo == 0)
		{
			PostProcessUtils().StretchRect(CTexture::s_ptexSceneSpecularESRAM, CTexture::s_ptexSceneSpecular);
		}
#endif
		return;
	}

	CD3D9Renderer* const __restrict rd = gcpRendD3D;

	PROFILE_LABEL_SCOPE("GBUFFER_FILTER");

	static CCryNameTSCRC tech("FilterGBuffer");

#if defined(DURANGO_USE_ESRAM)
	CTexture* pSceneSpecular = CTexture::s_ptexSceneSpecularESRAM;
#else
	PostProcessUtils().StretchRect(CTexture::s_ptexSceneSpecular, CTexture::s_ptexStereoR);
	CTexture* pSceneSpecular = CTexture::s_ptexStereoR;
#endif

	rd->FX_PushRenderTarget(0, CTexture::s_ptexSceneSpecular, NULL);
	SD3DPostEffectsUtils::ShBeginPass(m_pShader, tech, FEF_DONTSETSTATES);
	rd->FX_SetState(GS_NODEPTHTEST);
	SPostEffectsUtils::SetTexture(CTexture::s_ptexSceneNormalsMap, 0, FILTER_POINT, 0);
	SPostEffectsUtils::SetTexture(pSceneSpecular, 1, FILTER_POINT, 0);
	SPostEffectsUtils::SetTexture(CTexture::s_ptexZTarget, 2, FILTER_POINT, 0);

	// Bind sampler directly so that it works with DX11 style texture objects
	ID3D11SamplerState* pSamplers[1] = { (ID3D11SamplerState*)CTexture::s_TexStates[m_nTexStatePoint].m_pDeviceState };
	rd->m_DevMan.BindSampler(CDeviceManager::TYPE_PS, pSamplers, 15, 1);

	SD3DPostEffectsUtils::DrawFullScreenTri(CTexture::s_ptexSceneSpecular->GetWidth(), CTexture::s_ptexSceneSpecular->GetHeight());
	SD3DPostEffectsUtils::ShEndPass();

	ID3D11SamplerState* pSampNull[1] = { NULL };
	rd->m_DevMan.BindSampler(CDeviceManager::TYPE_PS, pSampNull, 15, 1);
	rd->FX_Commit();

	rd->FX_PopRenderTarget(0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
void CDeferredShading::DrawLightVolume(EShapeMeshType meshType, const Matrix44& mUnitVolumeToWorld, const Vec4& vSphereAdjust)
{
	CD3D9Renderer* const __restrict rd = gcpRendD3D;

	float maskRTWidth = (float) m_nCurTargetWidth;
	float maskRTHeight = (float) m_nCurTargetHeight;

	Vec4 vScreenScale(1.0f / maskRTWidth, 1.0f / maskRTHeight,
	                  0.0f, 0.0f);

	{
		static CCryNameR paramName("g_ScreenScale");
		m_pShader->FXSetPSFloat(paramName, &vScreenScale, 1);
	}

	{
		static CCryNameR paramName("vWBasisX");
		m_pShader->FXSetPSFloat(paramName, &vWorldBasisX, 1);
	}

	{
		static CCryNameR paramName("vWBasisY");
		m_pShader->FXSetPSFloat(paramName, &vWorldBasisY, 1);
	}

	{
		static CCryNameR paramName("vWBasisZ");
		m_pShader->FXSetPSFloat(paramName, &vWorldBasisZ, 1);
	}

	{
		static CCryNameR paramNameVolumeToWorld("g_mUnitLightVolumeToWorld");
		m_pShader->FXSetVSFloat(paramNameVolumeToWorld, (Vec4*) mUnitVolumeToWorld.GetData(), 4);
	}

	{
		static CCryNameR paramNameSphereAdjust("g_vLightVolumeSphereAdjust");
		m_pShader->FXSetVSFloat(paramNameSphereAdjust, &vSphereAdjust, 1);
	}

	{
		static CCryNameR paramName("g_mViewProj");
		Matrix44 mViewProjMatrix = rd->m_ViewMatrix * rd->m_ProjMatrix;
		m_pShader->FXSetVSFloat(paramName, (Vec4*) mViewProjMatrix.GetData(), 4);
	}

	// Vertex/index buffer
	rd->FX_SetVStream(0, rd->m_pUnitFrustumVB[meshType], 0, sizeof(SVF_P3F_C4B_T2F));
	rd->FX_SetIStream(rd->m_pUnitFrustumIB[meshType], 0, (rd->kUnitObjectIndexSizeof == 2 ? Index16 : Index32));

	rd->D3DSetCull(eCULL_Back);
	if (!FAILED(rd->FX_SetVertexDeclaration(0, eVF_P3F_C4B_T2F)))
	{
		rd->FX_Commit();
		rd->FX_DrawIndexedPrimitive(eptTriangleList, 0, 0, rd->m_UnitFrustVBSize[meshType], 0, rd->m_UnitFrustIBSize[meshType]);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void CDeferredShading::DrawDecalVolume(const SDeferredDecal& rDecal, Matrix44A& mDecalLightProj, ECull volumeCull)
{
	CD3D9Renderer* const __restrict rd = gcpRendD3D;

	float maskRTWidth = float(m_nCurTargetWidth);
	float maskRTHeight = float(m_nCurTargetHeight);

	Vec4 vScreenScale(1.0f / maskRTWidth, 1.0f / maskRTHeight,
	                  0.0f, 0.0f);

	{
		static CCryNameR paramName("g_ScreenScale");
		m_pShader->FXSetPSFloat(paramName, &vScreenScale, 1);
	}

	{
		static CCryNameR paramName("vWBasisX");
		m_pShader->FXSetPSFloat(paramName, &vWorldBasisX, 1);
	}

	{
		static CCryNameR paramName("vWBasisY");
		m_pShader->FXSetPSFloat(paramName, &vWorldBasisY, 1);
	}

	{
		static CCryNameR paramName("vWBasisZ");
		m_pShader->FXSetPSFloat(paramName, &vWorldBasisZ, 1);
	}

	//////////////// light sphere processing /////////////////////////////////
	{
		Matrix44 mInvDecalLightProj = mDecalLightProj.GetInverted();
		static CCryNameR paramName("g_mInvLightProj");
		m_pShader->FXSetVSFloat(paramName, alias_cast<Vec4*>(&mInvDecalLightProj), 4);
	}

	{
		static CCryNameR paramName("g_mViewProj");
		m_pShader->FXSetVSFloat(paramName, alias_cast<Vec4*>(&m_mViewProj), 4);
	}

	rd->FX_SetVStream(0, rd->m_pUnitFrustumVB[SHAPE_BOX], 0, sizeof(SVF_P3F_C4B_T2F));
	rd->FX_SetIStream(rd->m_pUnitFrustumIB[SHAPE_BOX], 0, (rd->kUnitObjectIndexSizeof == 2 ? Index16 : Index32));

	rd->D3DSetCull(volumeCull);
	if (!FAILED(rd->FX_SetVertexDeclaration(0, eVF_P3F_C4B_T2F)))
	{
		rd->FX_Commit();
		rd->FX_DrawIndexedPrimitive(eptTriangleList, 0, 0, rd->m_UnitFrustVBSize[SHAPE_BOX], 0, rd->m_UnitFrustIBSize[SHAPE_BOX]);
	}

}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

bool CDeferredShading::DeferredDecalPass(const SDeferredDecal& rDecal, uint32 indDecal)
{
	PROFILE_FRAME(CDeferredShading_DecalPass);

	gcpRendD3D->m_RP.m_FlagsShader_RT &= ~(RT_LIGHTSMASK | g_HWSR_MaskBit[HWSR_SAMPLE4]);

	CD3D9Renderer* const __restrict rd = gcpRendD3D;
	int nThreadID = rd->m_RP.m_nProcessThreadID;

	rd->m_RP.m_nDeferredPrimitiveID = SHAPE_PROJECTOR;

	bool bProj2D = true;
	bool bStencilMask = false;
	bool bUseLightVolumes = true;

	rd->EF_Scissor(false, 0, 0, 1, 1);
	rd->SetDepthBoundTest(0.0f, 1.0f, false); // stencil pre-passes are rop bound, using depth bounds increases even more rop cost

	IMaterial* pDecalMaterial = rDecal.pMaterial;
	if (pDecalMaterial == NULL)
	{
		assert(0);
		return false;
	}

	SShaderItem& sItem = pDecalMaterial->GetShaderItem(0);
	if (sItem.m_pShaderResources == NULL)
	{
		assert(0);
		return false;
	}

	CTexture* pCurTarget = CTexture::s_ptexSceneNormalsMap;

	int nStates = m_nRenderState;

	nStates &= ~GS_BLEND_MASK;
	nStates &= ~GS_COLMASK_NONE;
	nStates |= GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA;

	ITexture* pNormalMap = TextureHelpers::LookupTexDefault(EFTT_NORMALS);
	if (SEfResTexture* pNormalRes = sItem.m_pShaderResources->GetTexture(EFTT_NORMALS))
	{
		if (pNormalRes->m_Sampler.m_pITex)
		{
			pNormalMap = pNormalRes->m_Sampler.m_pITex;
			rd->m_RP.m_FlagsShader_RT |= g_HWSR_MaskBit[HWSR_SAMPLE1];
		}
	}

	ITexture* pSmoothnessMap = TextureHelpers::LookupTexDefault(EFTT_SMOOTHNESS);
	if (SEfResTexture* pSmoothnessRes = sItem.m_pShaderResources->GetTexture(EFTT_SMOOTHNESS))
	{
		if (pSmoothnessRes->m_Sampler.m_pITex)
		{
			pSmoothnessMap = pSmoothnessRes->m_Sampler.m_pITex;
			rd->m_RP.m_FlagsShader_RT |= g_HWSR_MaskBit[HWSR_SAMPLE4];
		}
	}

	ITexture* pOpacityMap = TextureHelpers::LookupTexDefault(EFTT_OPACITY);
	if (SEfResTexture* pOpacityRes = sItem.m_pShaderResources->GetTexture(EFTT_OPACITY))
	{
		if (pOpacityRes->m_Sampler.m_pITex)
		{
			pOpacityMap = pOpacityRes->m_Sampler.m_pITex;
			rd->m_RP.m_FlagsShader_RT |= g_HWSR_MaskBit[HWSR_SAMPLE5];
		}
	}

	if (rDecal.fGrowAlphaRef > 0.0f)
		rd->m_RP.m_FlagsShader_RT |= g_HWSR_MaskBit[HWSR_SAMPLE0];

	nStates &= ~(GS_NODEPTHTEST | GS_DEPTHFUNC_MASK);
	nStates |= GS_DEPTHFUNC_LEQUAL | GS_COLMASK_RGB;

	rd->m_RP.m_PersFlags2 |= RBPF2_READMASK_RESERVED_STENCIL_BIT;

	//////////////////////////////////////////////////////////////////////////
	//apply stencil dynamic masking
	rd->FX_SetStencilState(
	  STENC_FUNC(FSS_STENCFUNC_EQUAL) |
	  STENCOP_FAIL(FSS_STENCOP_KEEP) |
	  STENCOP_ZFAIL(FSS_STENCOP_KEEP) |
	  STENCOP_PASS(FSS_STENCOP_KEEP),
	  BIT_STENCIL_RESERVED, BIT_STENCIL_RESERVED, 0xFFFFFFFF
	  );

	rd->m_RP.m_PersFlags2 &= ~RBPF2_READMASK_RESERVED_STENCIL_BIT;

	nStates |= GS_STENCIL;
	//////////////////////////////////////////////////////////////////////////

	if (bStencilMask)
	{
		rd->FX_StencilTestCurRef(true, false);
	}

	if (CRenderer::CV_r_deferredDecalsDebug == 2)
	{
		nStates &= ~GS_COLMASK_NONE;
		nStates &= ~GS_NODEPTHTEST;
		//newState |= GS_NODEPTHTEST;
		nStates |= GS_DEPTHWRITE;
		nStates |= ((~(0xF)) << GS_COLMASK_SHIFT) & GS_COLMASK_MASK;
		nStates |= GS_WIREFRAME;
	}
	else if (CRenderer::CV_r_deferredDecalsDebug == 1)
	{
		rd->m_RP.m_FlagsShader_RT |= g_HWSR_MaskBit[HWSR_SAMPLE2];
		rd->m_RP.m_FlagsShader_RT &= ~g_HWSR_MaskBit[HWSR_SAMPLE0];
		rd->m_RP.m_FlagsShader_RT &= ~g_HWSR_MaskBit[HWSR_SAMPLE1];
	}

	//////////////////////////////////////////////////////////////////////////
	if (bUseLightVolumes)
	{
		//enable light volumes rendering
		rd->m_RP.m_FlagsShader_RT |= g_HWSR_MaskBit[HWSR_CUBEMAP0];
		static CCryNameTSCRC techName("DeferredDecalVolume");
		SD3DPostEffectsUtils::ShBeginPass(m_pShader, techName, FEF_DONTSETSTATES);
	}
	else
	{
		SD3DPostEffectsUtils::ShBeginPass(m_pShader, m_pDeferredDecalTechName, FEF_DONTSETSTATES);
	}
	//////////////////////////////////////////////////////////////////////////

	m_nCurTargetWidth = pCurTarget->GetWidth();
	m_nCurTargetHeight = pCurTarget->GetHeight();

	ITexture* pDiffuseTex = NULL;
	Matrix44 texMatrix;
	STexState texState;

	texMatrix.SetIdentity();

	texState.SetFilterMode(FILTER_TRILINEAR);

	if (SEfResTexture* pDiffuseRes = sItem.m_pShaderResources->GetTexture(EFTT_DIFFUSE))
	{
		if (pDiffuseRes->m_Sampler.m_pITex)
		{
			pDiffuseTex = pDiffuseRes->m_Sampler.m_pITex;

			if (pDiffuseRes->IsHasModificators())
			{
				pDiffuseRes->UpdateWithModifier(EFTT_MAX);
				SEfTexModificator* mod = pDiffuseRes->m_Ext.m_pTexModifier;
				texMatrix = pDiffuseRes->m_Ext.m_pTexModifier->m_TexMatrix;
			}

			int mode = pDiffuseRes->m_bUTile ? TADDR_WRAP : TADDR_CLAMP;

			texState.SetClampMode(mode, pDiffuseRes->m_bVTile ? TADDR_WRAP : TADDR_CLAMP, mode);
		}
	}

	{
		static CCryNameR paramTextureRect("g_TextureRect");
		assert(rDecal.rectTexture.w * rDecal.rectTexture.h > 0.f);
		Vec4 data[2];
		data[0] = Vec4(rDecal.rectTexture.w * texMatrix.m00, rDecal.rectTexture.h * texMatrix.m10, rDecal.rectTexture.x * texMatrix.m00 + rDecal.rectTexture.y * texMatrix.m10 + texMatrix.m30, 0);
		data[1] = Vec4(rDecal.rectTexture.w * texMatrix.m01, rDecal.rectTexture.h * texMatrix.m11, rDecal.rectTexture.x * texMatrix.m01 + rDecal.rectTexture.y * texMatrix.m11 + texMatrix.m31, 0);
		m_pShader->FXSetPSFloat(paramTextureRect, data, 2);
	}

	assert(pDiffuseTex != NULL);
	if (pDiffuseTex)
	{
		texState.m_bSRGBLookup = true;
		((CTexture*)pDiffuseTex)->Apply(2, CTexture::GetTexState(texState));
	}

	Vec4 vDiff = sItem.m_pShaderResources->GetColorValue(EFTT_DIFFUSE).toVec4();
	vDiff.w = sItem.m_pShaderResources->GetStrengthValue(EFTT_OPACITY) * rDecal.fAlpha;
	m_pShader->FXSetPSFloat(m_pParamDecalDiffuse, &vDiff, 1);

	Vec4 vSpec = sItem.m_pShaderResources->GetColorValue(EFTT_SPECULAR).toVec4();
	vSpec.w = sItem.m_pShaderResources->GetStrengthValue(EFTT_SMOOTHNESS);
	m_pShader->FXSetPSFloat(m_pParamDecalSpecular, &vSpec, 1);

	float decalAlphaMult = 1;
	float decalFalloff = 1;
	float decalDiffuseOpacity = 1;

	DynArrayRef<SShaderParam>& shaderParams = sItem.m_pShaderResources->GetParameters();
	for (uint32 i = 0, si = shaderParams.size(); i < si; ++i)
	{
		const char* name = shaderParams[i].m_Name;
		if (strcmp(name, "DecalAlphaMult") == 0)
			decalAlphaMult = shaderParams[i].m_Value.m_Float;
		else if (strcmp(name, "DecalFalloff") == 0)
			decalFalloff = shaderParams[i].m_Value.m_Float;
		else if (strcmp(name, "DecalDiffuseOpacity") == 0)
			decalDiffuseOpacity = shaderParams[i].m_Value.m_Float;
	}

	Vec4 decalParams(decalAlphaMult, decalFalloff, decalDiffuseOpacity, rDecal.fGrowAlphaRef);
	m_pShader->FXSetPSFloat(m_pGeneralParams, &decalParams, 1);

	if (pDiffuseTex && CRenderer::CV_r_deferredDecalsDebug == 1)
	{
		Vec4 decalParams;
		indDecal = pDiffuseTex->GetTextureID() % 3;
		//indDecal%=3;
		decalParams.x = indDecal == 0 ? 1.0f : 0.0;
		decalParams.y = indDecal == 1 ? 1.0f : 0.0;
		decalParams.z = indDecal == 2 ? 1.0f : 0.0;
		decalParams.w = 0.94f;
		m_pShader->FXSetPSFloat(m_pGeneralParams, &decalParams, 1);
	}

	SD3DPostEffectsUtils::SetTexture((CTexture*)pNormalMap, 3, FILTER_TRILINEAR, 0);
	SD3DPostEffectsUtils::SetTexture((CTexture*)pSmoothnessMap, 4, FILTER_TRILINEAR, 0);
	SD3DPostEffectsUtils::SetTexture((CTexture*)pOpacityMap, 5, FILTER_TRILINEAR, 0);
	SD3DPostEffectsUtils::SetTexture((CTexture*)CTexture::s_ptexBackBuffer, 6, FILTER_POINT, 0);   //contains copy of normals buffer

	//////////////////////////////////////////////////////////////////////////
	// texture and depth transformation matrix
	// fading distance is longer under the decal and shorter in front of the decal

	static const float fZNear = -0.3f;
	static const float fZFar = 0.5f;

	static const Matrix44A mTextureAndDepth(
	  0.5f, 0.0f, 0.0f, 0.5f,
	  0.0f, -0.5f, 0.0f, 0.5f,
	  0.0f, 0.0f, 1.0f / (fZNear - fZFar), fZNear / (fZNear - fZFar),
	  0.0f, 0.0f, 0.0f, 1.0f);

	// transform world coords to decal texture coords
	Matrix44A mDecalLightProj = mTextureAndDepth * rDecal.projMatrix.GetInverted();

	// coord systems conversion (from orientation to shader matrix)
	const Vec3 vBasisX = rDecal.projMatrix.GetColumn0();
	const Vec3 vBasisY = rDecal.projMatrix.GetColumn1();
	const Vec3 vBasisZ = rDecal.projMatrix.GetColumn2();

	const Vec3 vNormX = vBasisX.GetNormalized();
	const Vec3 vNormY = vBasisY.GetNormalized();
	const Vec3 vNormZ = vBasisZ.GetNormalized();

	// decal normal map to world transform
	Matrix44A mDecalTS(
	  vNormX.x, vNormX.y, vNormX.z, 0,
	  -vNormY.x, -vNormY.y, -vNormY.z, 0,
	  vNormZ.x, vNormZ.y, vNormZ.z, 0,
	  0, 0, 0, 1);

	// Manual mip level computation
	//
	//                 tan(fov) * (textureSize * tiling / decalSize) * distance
	// MipLevel = log2 --------------------------------------------------------
	//                 screenResolution * dot(viewVector, decalNormal)

	const float screenRes = (float)rd->GetWidth() * 0.5f + (float)rd->GetHeight() * 0.5f;
	const float decalSize = max(vBasisX.GetLength() * 2.0f, vBasisY.GetLength() * 2.0f);
	const float texScale = max(
	  texMatrix.GetColumn(0).GetLength() * rDecal.rectTexture.w,
	  texMatrix.GetColumn(1).GetLength() * rDecal.rectTexture.h);
	const float mipLevelFactor = (tan(rd->GetCamera().GetFov()) * texScale / decalSize) / screenRes;

	Vec4 vMipLevels;
	vMipLevels.x = pDiffuseTex ? mipLevelFactor * (float)max(pDiffuseTex->GetWidth(), pDiffuseTex->GetHeight()) : 0.f;
	vMipLevels.y = pNormalMap ? mipLevelFactor * (float)max(pNormalMap->GetWidth(), pNormalMap->GetHeight()) : 0.f;
	vMipLevels.z = pSmoothnessMap ? mipLevelFactor * (float)max(pSmoothnessMap->GetWidth(), pSmoothnessMap->GetHeight()) : 0.f;
	vMipLevels.w = pOpacityMap ? mipLevelFactor * (float)max(pOpacityMap->GetWidth(), pOpacityMap->GetHeight()) : 0.f;
	m_pShader->FXSetPSFloat(m_pParamDecalMipLevels, &vMipLevels, 1);

	ECull volumeCull = eCULL_Back;
	nStates &= ~(GS_NODEPTHTEST | GS_DEPTHFUNC_MASK);
	nStates |= GS_DEPTHFUNC_LEQUAL;

	rd->EF_Scissor(false, 0, 0, 1, 1);

	const float r = fabs(vBasisX.dot(m_pCamFront)) + fabs(vBasisY.dot(m_pCamFront)) + fabs(vBasisZ.dot(m_pCamFront));
	const float s = m_pCamFront.dot(rDecal.projMatrix.GetTranslation() - m_pCamPos);
	if (fabs(s) - m_fCamNear <= r) // OBB-Plane via separating axis test, to check if camera near plane intersects decal volume
	{
		nStates &= ~(GS_NODEPTHTEST | GS_DEPTHFUNC_MASK);
		nStates |= GS_DEPTHFUNC_GREAT;
		volumeCull = eCULL_Front;
	}

	if (CRenderer::CV_r_deferredDecalsDebug == 2)
	{
		volumeCull = eCULL_Back;
	}

	rd->FX_SetState(nStates);
	m_pDepthRT->Apply(0, m_nTexStatePoint);

	m_pShader->FXSetPSFloat(m_pParamDecalTS, (Vec4*) mDecalTS.GetData(), 4);
	m_pShader->FXSetPSFloat(m_pParamLightProjMatrix, (Vec4*) mDecalLightProj.GetData(), 4);

	if (bUseLightVolumes)
	{
		DrawDecalVolume(rDecal, mDecalLightProj, volumeCull);
	}
	else
	{
		SD3DPostEffectsUtils::DrawFullScreenTriWPOS(m_nCurTargetWidth, m_nCurTargetHeight);
	}

	SD3DPostEffectsUtils::ShEndPass();

	if (bStencilMask)
		rd->FX_StencilTestCurRef(false);

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void CDeferredShading::GetLightRenderSettings(const SRenderLight* const __restrict pDL, bool& bStencilMask, bool& bUseLightVolumes, EShapeMeshType& meshType)
{
	CD3D9Renderer* const __restrict rd = gcpRendD3D;
	SRenderPipeline& RESTRICT_REFERENCE rRP = rd->m_RP;

	const bool bAreaLight = (pDL->m_Flags & DLF_AREA_LIGHT) && pDL->m_fAreaWidth && pDL->m_fAreaHeight && pDL->m_fLightFrustumAngle;

	if (CRenderer::CV_r_deferredshadingLightVolumes)
	{
		if (bAreaLight)
		{
			// area lights use non-uniform box volume
			// need to do more complex box intersection test
			float fExpensionRadius = pDL->m_fRadius * 1.08f;
			Vec3 vScale(fExpensionRadius, fExpensionRadius, fExpensionRadius);

			Matrix34 mObjInv = CShadowUtils::GetAreaLightMatrix(pDL, vScale);
			mObjInv.Invert();

			// check if volumes bounding box intersects the near clipping plane
			const Plane* pNearPlane(rd->GetCamera().GetFrustumPlane(FR_PLANE_NEAR));
			Vec3 pntOnNearPlane(rd->GetCamera().GetPosition() - pNearPlane->DistFromPlane(rd->GetCamera().GetPosition()) * pNearPlane->n);
			Vec3 pntOnNearPlaneOS(mObjInv.TransformPoint(pntOnNearPlane));

			Vec3 nearPlaneOS_n(mObjInv.TransformVector(pNearPlane->n));
			f32 nearPlaneOS_d(-nearPlaneOS_n.Dot(pntOnNearPlaneOS));

			// get extreme lengths
			float t(fabsf(nearPlaneOS_n.x) + fabsf(nearPlaneOS_n.y) + fabsf(nearPlaneOS_n.z));

			float t0 = t + nearPlaneOS_d;
			float t1 = -t + nearPlaneOS_d;

			if (t0 * t1 > 0.0f)
				bUseLightVolumes = true;
			else
				bStencilMask = true;
		}
		else
		{
			const float kDLRadius = pDL->m_fRadius;
			const float fSmallLightBias = 0.5f;
			// the light mesh tessellation and near clipping plane require some bias when testing if inside sphere
			// higher bias for low radius lights
			float fSqLightRadius = kDLRadius * max(-0.1f * kDLRadius + 1.5f, 1.22f);
			fSqLightRadius = max(kDLRadius + fSmallLightBias, fSqLightRadius); //always add on a minimum bias, for very small light's sake
			fSqLightRadius *= fSqLightRadius;
			if (fSqLightRadius < pDL->m_Origin.GetSquaredDistance(m_pCamPos))
				bUseLightVolumes = true;
			else
				bStencilMask = true;
		}
	}

	Vec4 pLightRect = Vec4(pDL->m_sX, pDL->m_sY, pDL->m_sWidth, pDL->m_sHeight);
	Vec4 scaledLightRect = pLightRect * Vec4(
	  rRP.m_CurDownscaleFactor.x, rRP.m_CurDownscaleFactor.y,
	  rRP.m_CurDownscaleFactor.x, rRP.m_CurDownscaleFactor.y
	  );

	float fCurTargetWidth = (float)(m_nCurTargetWidth);
	float fCurTargetHeight = (float)(m_nCurTargetHeight);

	if (!iszero(CRenderer::CV_r_DeferredShadingLightLodRatio))
	{
		if (CRenderer::CV_r_DeferredShadingLightStencilRatio > 0.01f)
		{
			const float fLightLodRatioScale = CRenderer::CV_r_DeferredShadingLightLodRatio;
			float fLightArea = pLightRect.z * pLightRect.w;
			float fScreenArea = fCurTargetHeight * fCurTargetWidth;
			float fLightRatio = fLightLodRatioScale * (fLightArea / fScreenArea);

			const float fDrawVolumeThres = 0.005f;
			if (fLightRatio < fDrawVolumeThres)
			{
				bUseLightVolumes = false;
			}

			if (fLightRatio > 4 * CRenderer::CV_r_DeferredShadingLightStencilRatio)
				meshType = SHAPE_PROJECTOR2;
			else if (fLightRatio > 2 * CRenderer::CV_r_DeferredShadingLightStencilRatio)
				meshType = SHAPE_PROJECTOR1;
		}
		else
		{
			const float fLightLodRatioScale = CRenderer::CV_r_DeferredShadingLightLodRatio;
			float fLightArea = pLightRect.z * pLightRect.w;
			float fScreenArea = fCurTargetHeight * fCurTargetWidth;
			float fLightRatio = fLightLodRatioScale * (fLightArea / fScreenArea);

			const float fDrawVolumeThres = 0.005f;
			if (fLightRatio < fDrawVolumeThres)
			{
				bUseLightVolumes = false;
			}
		}
	}
}

void CDeferredShading::LightPass(const SRenderLight* const __restrict pDL, bool bForceStencilDisable)
{
	PROFILE_FRAME(CDeferredShading_LightPass);

	PROFILE_LABEL(pDL->m_sName);

	PrefetchLine(&pDL->m_Color, 0);
	PrefetchLine(&pDL->m_sWidth, 0);

	// Skip non-ambient area light if support is disabled
	if ((pDL->m_Flags & DLF_AREA_LIGHT) && !(pDL->m_Flags & DLF_AMBIENT) && !CRenderer::CV_r_DeferredShadingAreaLights)
		return;

	gcpRendD3D->m_RP.m_FlagsShader_RT &= ~(RT_LIGHTPASS_RESETMASK | RT_CLIPVOLUME_ID);

	CD3D9Renderer* const __restrict rd = gcpRendD3D;
	SRenderPipeline& RESTRICT_REFERENCE rRP = rd->m_RP;

#ifdef SUPPORTS_MSAA
	const int32 nNumPassesMSAA = (rd->FX_GetMSAAMode() == 1) ? 2 : 1;
	rd->FX_MSAASampleFreqStencilSetup();
#endif

	const ITexture* pLightTex = pDL->m_pLightImage ? pDL->m_pLightImage : pDL->m_pLightDynTexSource ? pDL->m_pLightDynTexSource->GetTexture() : NULL;
	const bool bProj2D = (pDL->m_Flags & DLF_PROJECT) && pLightTex && !(pLightTex->GetFlags() & FT_REPLICATE_TO_ALL_SIDES);
	const bool bAreaLight = (pDL->m_Flags & DLF_AREA_LIGHT) && pDL->m_fAreaWidth && pDL->m_fAreaHeight && pDL->m_fLightFrustumAngle;

	// Store light properties (color/radius, position relative to camera, rect, zbounds)
	Vec4 pLightDiffuse = Vec4(pDL->m_Color.r, pDL->m_Color.g, pDL->m_Color.b, pDL->m_SpecMult);

	float fInvRadius = (pDL->m_fRadius <= 0) ? 1.0f : 1.0f / pDL->m_fRadius;
	Vec4 pLightPosCS = Vec4(pDL->m_Origin - m_pCamPos, fInvRadius);
	Vec4 pDepthBounds = GetLightDepthBounds(pDL, (rRP.m_TI[rRP.m_nProcessThreadID].m_PersFlags & RBPF_REVERSE_DEPTH) != 0);

	Vec4 scaledLightRect = Vec4(
	  pDL->m_sX * rRP.m_CurDownscaleFactor.x,
	  pDL->m_sY * rRP.m_CurDownscaleFactor.y,
	  pDL->m_sWidth * rRP.m_CurDownscaleFactor.x,
	  pDL->m_sHeight * rRP.m_CurDownscaleFactor.y
	  );

	bool bUseLightVolumes = false;
	bool bStencilMask = (CRenderer::CV_r_DeferredShadingStencilPrepass && (bProj2D || bAreaLight)) || CRenderer::CV_r_DebugLightVolumes || (pDL->m_fProjectorNearPlane < 0);
	rRP.m_nDeferredPrimitiveID = SHAPE_PROJECTOR;

	GetLightRenderSettings(pDL, bStencilMask, bUseLightVolumes, rRP.m_nDeferredPrimitiveID);

	//reset stencil mask
	if (bForceStencilDisable)
	{
		bStencilMask = false;
	}

	if (pDL->m_Flags & DLF_AMBIENT)
		rRP.m_FlagsShader_RT |= RT_AMBIENT_LIGHT;

	if (CRenderer::CV_r_DeferredShadingAreaLights)
		rRP.m_FlagsShader_RT |= g_HWSR_MaskBit[HWSR_SAMPLE3];

	// Apply LBuffers range rescale
	pLightDiffuse.x *= rd->m_fAdaptedSceneScaleLBuffer;
	pLightDiffuse.y *= rd->m_fAdaptedSceneScaleLBuffer;
	pLightDiffuse.z *= rd->m_fAdaptedSceneScaleLBuffer;

	float fAttenuationBulbSize = pDL->m_fAttenuationBulbSize;

	if (bAreaLight)
		fAttenuationBulbSize = (pDL->m_fAreaWidth + pDL->m_fAreaHeight) * 0.25;

	// Adjust light intensity so that the intended brightness is reached 1 meter from the light's surface
	if (!(pDL->m_Flags & DLF_AMBIENT))
	{
		fAttenuationBulbSize = max(fAttenuationBulbSize, 0.001f);

		// Solve I * 1 / (1 + d/lightsize)^2 = 1
		float intensityMul = 1.0f + 1.0f / fAttenuationBulbSize;
		intensityMul *= intensityMul;
		pLightDiffuse.x *= intensityMul;
		pLightDiffuse.y *= intensityMul;
		pLightDiffuse.z *= intensityMul;
	}

	// Enable light pass flags
	if ((pDL->m_Flags & DLF_PROJECT))
	{
		assert(!(pDL->GetDiffuseCubemap() && pDL->GetSpecularCubemap()));
		rRP.m_FlagsShader_RT |= RT_TEX_PROJECT;
		if (bProj2D && !bAreaLight)
		{
			rRP.m_FlagsShader_RT |= g_HWSR_MaskBit[HWSR_LIGHT_TEX_PROJ];
		}
	}

	if (bAreaLight)
	{
		rRP.m_FlagsShader_RT |= RT_AREALIGHT;
	}

	uint16 scaledX = (uint16)(scaledLightRect.x);
	uint16 scaledY = (uint16)(scaledLightRect.y);
	uint16 scaledWidth = (uint16)(scaledLightRect.z) + 1;
	uint16 scaledHeight = (uint16)(scaledLightRect.w) + 1;

	if (CRenderer::CV_r_DeferredShadingScissor)
	{
		SetupScissors(true, scaledX, scaledY, scaledWidth, scaledHeight);
	}

	if (bStencilMask)
	{
		PROFILE_LABEL_SCOPE("STENCIL_VOLUME");

		SpecularAccEnableMRT(false);
		auto prevPersFlags = rd->m_RP.m_TI[m_nThreadID].m_PersFlags;
		rd->m_RP.m_TI[m_nThreadID].m_PersFlags |= RBPF_MIRRORCULL;

		rd->SetDepthBoundTest(0.0f, 1.0f, false); // stencil pre-passes are rop bound, using depth bounds increases even more rop cost
		rd->FX_StencilFrustumCull((pDL->m_Flags & DLF_CASTSHADOW_MAPS) ? -4 : -1, pDL, NULL, 0);

		rd->m_RP.m_TI[m_nThreadID].m_PersFlags = prevPersFlags;
	}
	else if (rd->m_bDeviceSupports_NVDBT && CRenderer::CV_r_DeferredShadingDepthBoundsTest == 1)
		rd->SetDepthBoundTest(pDepthBounds.x, pDepthBounds.z, true);

	// todo: try out on consoles if DBT helps on light pass (on light stencil prepass is actually slower)
	if (rd->m_bDeviceSupports_NVDBT && bStencilMask && CRenderer::CV_r_DeferredShadingDepthBoundsTest && CRenderer::CV_r_deferredshadingDBTstencil)
		rd->SetDepthBoundTest(pDepthBounds.x, pDepthBounds.z, true);

	if (bStencilMask)
	{
		SpecularAccEnableMRT(true);
	}

#ifdef SUPPORTS_MSAA
	rd->FX_MSAARestoreSampleMask(); // If stencil clear occurred, restore sample mask

	for (int32 nPass = 0; nPass < nNumPassesMSAA; ++nPass)
	{
		if (nPass == 1)
		{
			PROFILE_LABEL_PUSH("LIGHT_SAMPLEFREQPASS");
			rd->FX_MSAASampleFreqStencilSetup(MSAA_SAMPLEFREQ_PASS);
		}
#endif
	uint nNumClipVolumes = RenderView()->GetClipVolumes().size();
	if (nNumClipVolumes > 0)
		rRP.m_FlagsShader_RT |= RT_CLIPVOLUME_ID;

	if (bUseLightVolumes)
	{
		rRP.m_FlagsShader_RT |= g_HWSR_MaskBit[HWSR_CUBEMAP0];
		SD3DPostEffectsUtils::ShBeginPass(m_pShader, m_pLightVolumeTechName, FEF_DONTSETTEXTURES | FEF_DONTSETSTATES);
	}
	else
	{
		SD3DPostEffectsUtils::ShBeginPass(m_pShader, m_pTechName, FEF_DONTSETTEXTURES | FEF_DONTSETSTATES);
	}

	int nStates = m_nRenderState;
	nStates &= ~(GS_BLEND_MASK);

	nStates &= ~(GS_NODEPTHTEST | GS_DEPTHFUNC_MASK);     // Ensure zcull used.
	nStates |= GS_DEPTHFUNC_LEQUAL;

	if (!(pDL->m_Flags & DLF_AMBIENT))
		nStates |= (GS_BLSRC_ONE | GS_BLDST_ONE);
	else
		nStates |= (GS_BLSRC_DSTCOL | GS_BLDST_ZERO);

	if (CRenderer::CV_r_DeferredShadingDebug == 2)
	{
		nStates &= ~GS_BLEND_MASK;
		nStates |= (GS_BLSRC_ONE | GS_BLDST_ONE);
	}

	rd->FX_SetState(nStates);

#ifdef SUPPORTS_MSAA
	if (gcpRendD3D->m_RP.m_MSAAData.Type)
	{
		bool bUsesStencil = bForceStencilDisable || bStencilMask;   // shadows/clip volumes, or using light stencil
		rRP.m_PersFlags2 |= RBPF2_READMASK_RESERVED_STENCIL_BIT;
		const int32 nMSAAStState = STENC_FUNC(FSS_STENCFUNC_EQUAL) | STENCOP_FAIL(FSS_STENCOP_KEEP) | STENCOP_ZFAIL(FSS_STENCOP_KEEP) | STENCOP_PASS(FSS_STENCOP_KEEP);
		rd->FX_SetStencilState(nMSAAStState, (bUsesStencil ? rd->m_nStencilMaskRef : 0) | ((rRP.m_PersFlags2 & RBPF2_MSAA_SAMPLEFREQ_PASS) ? BIT_STENCIL_RESERVED : 0), bUsesStencil ? 0xFF : BIT_STENCIL_RESERVED, 0xFF);
		rd->FX_SetState(rRP.m_CurState | GS_STENCIL);
	}
	else
#endif
	if (bStencilMask)
		rd->FX_StencilTestCurRef(true, false);

	if ((pDL->m_Flags & DLF_PROJECT))
	{
		Matrix44A ProjMatrixT;

		if (bProj2D)
		{
			CShadowUtils::GetProjectiveTexGen(pDL, 0, &ProjMatrixT);
		}
		else
		{
			ProjMatrixT = pDL->m_ProjMatrix;
		}

		// translate into camera space
		ProjMatrixT.Transpose();
		const Vec4 vEye(gRenDev->GetRCamera().vOrigin, 0.f);
		Vec4 vecTranslation(vEye.Dot((Vec4&)ProjMatrixT.m00), vEye.Dot((Vec4&)ProjMatrixT.m10), vEye.Dot((Vec4&)ProjMatrixT.m20), vEye.Dot((Vec4&)ProjMatrixT.m30));
		ProjMatrixT.m03 += vecTranslation.x;
		ProjMatrixT.m13 += vecTranslation.y;
		ProjMatrixT.m23 += vecTranslation.z;
		ProjMatrixT.m33 += vecTranslation.w;
		m_pShader->FXSetPSFloat(m_pParamLightProjMatrix, (Vec4*) ProjMatrixT.GetData(), 4);
	}

	{
		Vec2 vLightSize = Vec2(pDL->m_fAreaWidth * 0.5f, pDL->m_fAreaHeight * 0.5f);

		float fAreaFov = pDL->m_fLightFrustumAngle * 2.0f;
		if ((pDL->m_Flags & DLF_CASTSHADOW_MAPS) && bAreaLight)
			fAreaFov = min(fAreaFov, 135.0f);   // Shadow can only cover ~135 degree FOV without looking bad, so we clamp the FOV to hide shadow clipping.

		const float fCosAngle = cosf(fAreaFov * (gf_PI / 360.0f));   // pre-compute on CPU.

		static CCryNameR arealightMatrixName("g_AreaLightMatrix");
		Matrix44 mAreaLightMatrix;
		mAreaLightMatrix.SetRow4(0, Vec4(pDL->m_ObjMatrix.GetColumn0().GetNormalized(), 1.0f));
		mAreaLightMatrix.SetRow4(1, Vec4(pDL->m_ObjMatrix.GetColumn1().GetNormalized(), 1.0f));
		mAreaLightMatrix.SetRow4(2, Vec4(pDL->m_ObjMatrix.GetColumn2().GetNormalized(), 1.0f));
		mAreaLightMatrix.SetRow4(3, Vec4(vLightSize.x, vLightSize.y, 0, fCosAngle));
		m_pShader->FXSetPSFloat(arealightMatrixName, alias_cast<Vec4*>(&mAreaLightMatrix), 4);
	}

	m_pShader->FXSetPSFloat(m_pParamLightPos, &pLightPosCS, 1);
	m_pShader->FXSetPSFloat(m_pParamLightDiffuse, &pLightDiffuse, 1);

	uint32 stencilID = (pDL->m_nStencilRef[1] + 1) << 16 | pDL->m_nStencilRef[0] + 1;
	Vec4 vAttenParams(fAttenuationBulbSize, *alias_cast<float*>(&stencilID), 0, 0);
	m_pShader->FXSetPSFloat(m_pAttenParams, &vAttenParams, 1);

	// Directional occlusion
	Vec4 ssdoParams(CRenderer::CV_r_ssdoAmountDirect, CRenderer::CV_r_ssdoAmountAmbient, CRenderer::CV_r_ssdoAmountReflection, 0);
	Vec4 ssdoParamsNull(0, 0, 0, 0);
	static CCryNameR ssdoParamsName("SSDOParams");
	m_pShader->FXSetPSFloat(ssdoParamsName, CRenderer::CV_r_ssdo ? &ssdoParams : &ssdoParamsNull, 1);

	if (pDL->m_Flags & DLF_CASTSHADOW_MAPS)
	{
		static ICVar* pVar = iConsole->GetCVar("e_ShadowsPoolSize");
		int nShadowAtlasRes = pVar->GetIVal();

		const ShadowMapFrustum& firstFrustum = CShadowUtils::GetFirstFrustum(m_pCurrentRenderView, m_nCurLighID);
		//LRad
		float kernelSize = firstFrustum.bOmniDirectionalShadow ? 2.5f : 1.5f;
		const Vec4 vShadowParams(kernelSize * (float(firstFrustum.nTexSize) / nShadowAtlasRes), 0.0f, 0.0f, firstFrustum.fDepthConstBias);
		m_pShader->FXSetPSFloat(m_pGeneralParams, &vShadowParams, 1);

		// set up shadow matrix
		static CCryNameR paramName("g_mLightShadowProj");
		Matrix44A shadowMat = gRenDev->m_TempMatrices[0][0];
		const Vec4 vEye(gRenDev->GetRCamera().vOrigin, 0.f);
		Vec4 vecTranslation(vEye.Dot((Vec4&)shadowMat.m00), vEye.Dot((Vec4&)shadowMat.m10), vEye.Dot((Vec4&)shadowMat.m20), vEye.Dot((Vec4&)shadowMat.m30));
		shadowMat.m03 += vecTranslation.x;
		shadowMat.m13 += vecTranslation.y;
		shadowMat.m23 += vecTranslation.z;
		shadowMat.m33 += vecTranslation.w;

		// pre-multiply by 1/frustrum_far_plane
		(Vec4&)shadowMat.m20 *= gRenDev->m_cEF.m_TempVecs[2].x;

		//camera matrix
		m_pShader->FXSetPSFloat(paramName, alias_cast<Vec4*>(&shadowMat), 4);
	}

	if (pDL->m_pLightDynTexSource)
	{
		CFlashTextureSourceBase::AddToLightRenderList(pDL->m_pLightDynTexSource);
	}

	CTexture* pTexLightImage = (CTexture*)pLightTex;

	// Note: Shadows use slot 3 and slot 7 for shadow map and jitter map

	m_pDepthRT->Apply(0, m_nTexStatePoint, EFTT_UNKNOWN, -1, (rRP.m_PersFlags2 & RBPF2_MSAA_SAMPLEFREQ_PASS) ? m_nBindResourceMsaa : SResourceView::DefaultView);
	m_pNormalsRT->Apply(1, m_nTexStatePoint, EFTT_UNKNOWN, -1, (rRP.m_PersFlags2 & RBPF2_MSAA_SAMPLEFREQ_PASS) ? m_nBindResourceMsaa : SResourceView::DefaultView);
	m_pDiffuseRT->Apply(2, m_nTexStatePoint, EFTT_UNKNOWN, -1, (rRP.m_PersFlags2 & RBPF2_MSAA_SAMPLEFREQ_PASS) ? m_nBindResourceMsaa : SResourceView::DefaultView);
	m_pSpecularRT->Apply(4, m_nTexStatePoint, EFTT_UNKNOWN, -1, (rRP.m_PersFlags2 & RBPF2_MSAA_SAMPLEFREQ_PASS) ? m_nBindResourceMsaa : SResourceView::DefaultView);

	if ((pDL->m_Flags & DLF_PROJECT) && pTexLightImage)
		SD3DPostEffectsUtils::SetTexture(pTexLightImage, 5, FILTER_TRILINEAR, bProj2D ? 1 : 0);

	CTexture::s_ptexSceneNormalsBent->Apply(8, m_nTexStatePoint);
	m_pMSAAMaskRT->Apply(9, m_nTexStatePoint);

	if (nNumClipVolumes > 0)
	{
		m_pResolvedStencilRT->Apply(11, m_nTexStatePoint);
		m_pShader->FXSetPSFloat(m_pClipVolumeParams, m_vClipVolumeParams, min((uint32)MAX_DEFERRED_CLIP_VOLUMES, nNumClipVolumes + VIS_AREAS_OUTDOOR_STENCIL_OFFSET));
	}

	if (bUseLightVolumes)
	{
		gcpRendD3D->D3DSetCull(eCULL_Back);

		const Vec3 scale(pDL->m_fRadius * 1.08f);
		Matrix34 mUnitVolumeToWorld = bAreaLight ? CShadowUtils::GetAreaLightMatrix(pDL, scale) : Matrix34::CreateScale(scale, pDL->m_Origin);

		DrawLightVolume(bAreaLight ? SHAPE_BOX : SHAPE_SPHERE, mUnitVolumeToWorld.GetTransposed());
	}
	else
	{
		gcpRendD3D->D3DSetCull(eCULL_Back, true);   //fs quads should not revert test..
		SD3DPostEffectsUtils::DrawFullScreenTriWPOS(m_pLBufferDiffuseRT->GetWidth(), m_pLBufferDiffuseRT->GetHeight(), pDepthBounds.x);
	}

	SD3DPostEffectsUtils::ShEndPass();
#ifdef SUPPORTS_MSAA
	if (nPass == 1)
	{
		rd->FX_MSAASampleFreqStencilSetup();
		PROFILE_LABEL_POP("LIGHT_SAMPLEFREQPASS");
	}
}
#endif

	rd->SetDepthBoundTest(0.0f, 1.0f, false);

#ifdef SUPPORTS_MSAA
	if (gcpRendD3D->m_RP.m_MSAAData.Type)
		rRP.m_PersFlags2 &= ~RBPF2_READMASK_RESERVED_STENCIL_BIT;
#endif

	if (bStencilMask)
		rd->FX_StencilTestCurRef(false);

	if (CRenderer::CV_r_DeferredShadingScissor)
		rd->EF_Scissor(false, 0, 0, 0, 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
void CDeferredShading::RenderClipVolumesToStencil(int nClipAreaReservedStencilBit)
{
	const auto& clipVolumes = RenderView()->GetClipVolumes();

	CD3D9Renderer* const __restrict rd = gcpRendD3D;
	const bool bRenderVisAreas = CRenderer::CV_r_VisAreaClipLightsPerPixel > 0;

	for (int nCurrVolume = clipVolumes.size() - 1; nCurrVolume >= 0; --nCurrVolume)
	{
		const SDeferredClipVolume& pVolumeData = clipVolumes[nCurrVolume];
		if (pVolumeData.m_pRenderMesh && pVolumeData.nStencilRef < MAX_DEFERRED_CLIP_VOLUMES)
		{
			if ((pVolumeData.nFlags & IClipVolume::eClipVolumeIsVisArea) != 0 && !bRenderVisAreas)
				continue;

			assert(((pVolumeData.nStencilRef + 1) & (BIT_STENCIL_RESERVED | nClipAreaReservedStencilBit)) == 0);
			const int nStencilRef = ~(pVolumeData.nStencilRef + 1) & ~(BIT_STENCIL_RESERVED | nClipAreaReservedStencilBit);

			rd->FX_StencilCullNonConvex(nStencilRef, pVolumeData.m_pRenderMesh.get(), pVolumeData.mWorldTM);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
void CDeferredShading::RenderPortalBlendValues(int nClipAreaReservedStencilBit)
{
	const auto& clipVolumes = RenderView()->GetClipVolumes();
	CD3D9Renderer* const __restrict rd = gcpRendD3D;

	uint nPrevState = rd->m_RP.m_CurState;
	ECull ePrevCullMode = rd->m_RP.m_eCull;

	uint nNewState = nPrevState;
	nNewState &= ~(GS_COLMASK_NONE | GS_DEPTHWRITE);
	nNewState |= GS_NODEPTHTEST | GS_NOCOLMASK_R | GS_NOCOLMASK_B | GS_NOCOLMASK_A;
	rd->FX_SetState(nNewState);

	static CCryNameTSCRC TechName0 = "PortalBlendVal";
	SD3DPostEffectsUtils::ShBeginPass(m_pShader, TechName0, FEF_DONTSETTEXTURES | FEF_DONTSETSTATES);
	SD3DPostEffectsUtils::SetTexture(CTexture::s_ptexZTarget, 3, FILTER_POINT);

	for (int nCurrVolume = clipVolumes.size() - 1; nCurrVolume >= 0; --nCurrVolume)
	{
		const SDeferredClipVolume& pClipVolumeData = clipVolumes[nCurrVolume];
		if (pClipVolumeData.nStencilRef < MAX_DEFERRED_CLIP_VOLUMES && pClipVolumeData.nFlags & IClipVolume::eClipVolumeBlend)
		{
			const bool bRenderMesh = pClipVolumeData.m_pRenderMesh && CRenderer::CV_r_VisAreaClipLightsPerPixel > 0;

			static CCryNameR blendPlane0Param("BlendPlane0");
			Vec4 plane0ClipSpace = rd->m_InvCameraProjMatrix * pClipVolumeData.blendInfo[0].blendPlane;
			m_pShader->FXSetPSFloat(blendPlane0Param, &plane0ClipSpace, 1);

			static CCryNameR blendPlane1Param("BlendPlane1");
			Vec4 plane1ClipSpace = rd->m_InvCameraProjMatrix * pClipVolumeData.blendInfo[1].blendPlane;
			m_pShader->FXSetPSFloat(blendPlane1Param, &plane1ClipSpace, 1);

			static CCryNameR screenScaleParam("g_ScreenScale");
			Vec4 screenScale = Vec4(1.0f / m_pLBufferDiffuseRT->GetWidth(), 1.0f / m_pLBufferDiffuseRT->GetHeight(), 0, 0);
			m_pShader->FXSetPSFloat(screenScaleParam, &screenScale, 1);

			rd->m_nStencilMaskRef = nClipAreaReservedStencilBit + pClipVolumeData.nStencilRef + 1;
			rd->FX_StencilTestCurRef(true);

			static CCryNameR paramNameVolumeToWorld("g_mUnitLightVolumeToWorld");
			Matrix44 matIdentity(IDENTITY);
			m_pShader->FXSetVSFloat(paramNameVolumeToWorld, (Vec4*) matIdentity.GetData(), 4);

			static CCryNameR paramNameSphereAdjust("g_vLightVolumeSphereAdjust");
			Vec4 vZero(ZERO);
			m_pShader->FXSetVSFloat(paramNameSphereAdjust, &vZero, 1);

			if (bRenderMesh)
			{
				CRenderMesh* pRenderMesh = static_cast<CRenderMesh*>(pClipVolumeData.m_pRenderMesh.get());
				pRenderMesh->CheckUpdate(pRenderMesh->_GetVertexFormat(), 0);

				const buffer_handle_t hVertexStream = pRenderMesh->_GetVBStream(VSF_GENERAL);
				const buffer_handle_t hIndexStream = pRenderMesh->_GetIBStream();

				if (hVertexStream != ~0u && hIndexStream != ~0u)
				{
					size_t nOffsI = 0;
					size_t nOffsV = 0;

					D3DVertexBuffer* pVB = gRenDev->m_DevBufMan.GetD3DVB(hVertexStream, &nOffsV);
					D3DIndexBuffer* pIB = gRenDev->m_DevBufMan.GetD3DIB(hIndexStream, &nOffsI);

					rd->FX_SetVStream(0, pVB, nOffsV, pRenderMesh->GetStreamStride(VSF_GENERAL));
					rd->FX_SetIStream(pIB, nOffsI, (sizeof(vtx_idx) == 2 ? Index16 : Index32));

					if (!FAILED(rd->FX_SetVertexDeclaration(0, pRenderMesh->_GetVertexFormat())))
					{
						static CCryNameR viewProjParam("g_mViewProj");
						m_pShader->FXSetVSFloat(viewProjParam, (Vec4*) rd->m_CameraProjMatrix.GetData(), 4);

						rd->D3DSetCull(eCULL_Front);
						rd->FX_Commit();
						rd->FX_DrawIndexedPrimitive(eptTriangleList, 0, 0, pRenderMesh->_GetNumVerts(), 0, pRenderMesh->_GetNumInds());
					}
				}
			}
			else
			{
				Matrix44 matQuadToClip(IDENTITY);
				matQuadToClip.m00 = 2;
				matQuadToClip.m30 = -1;
				matQuadToClip.m11 = -2;
				matQuadToClip.m31 = 1;

				static CCryNameR viewProjParam("g_mViewProj");
				m_pShader->FXSetVSFloat(viewProjParam, (Vec4*) matQuadToClip.GetData(), 4);

				rd->D3DSetCull(eCULL_Back);
				rd->FX_Commit();
				SD3DPostEffectsUtils::DrawFullScreenTri(m_pLBufferDiffuseRT->GetWidth(), m_pLBufferDiffuseRT->GetHeight());
			}
		}
	}

	SD3DPostEffectsUtils::ShEndPass();

	rd->D3DSetCull(ePrevCullMode);
	rd->FX_SetState(nPrevState);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
void CDeferredShading::PrepareClipVolumeData(bool& bOutdoorVisible)
{
	CD3D9Renderer* const __restrict rd = gcpRendD3D;
	const bool bMSAA = rd->m_RP.m_MSAAData.Type ? true : false;

	// Reserved outdoor fragments
	const auto& clipVolumes = RenderView()->GetClipVolumes();
	for (uint i = 0; i < VIS_AREAS_OUTDOOR_STENCIL_OFFSET; ++i)
	{
		uint32 nFlags = IClipVolume::eClipVolumeConnectedToOutdoor | IClipVolume::eClipVolumeAffectedBySun;
		m_vClipVolumeParams[i] = Vec4(gEnv->p3DEngine->GetSkyColor() * rd->m_fAdaptedSceneScaleLBuffer, *alias_cast<float*>(&nFlags));
	}

	for (const auto& pClipVolumeData : clipVolumes)
	{
		if (pClipVolumeData.nStencilRef + 1 < MAX_DEFERRED_CLIP_VOLUMES)
		{
			uint32 nData = (pClipVolumeData.blendInfo[1].blendID + 1) << 24 | (pClipVolumeData.blendInfo[0].blendID + 1) << 16 | pClipVolumeData.nFlags;
			m_vClipVolumeParams[pClipVolumeData.nStencilRef + 1] = Vec4(0, 0, 0, *alias_cast<float*>(&nData));

			bOutdoorVisible |= pClipVolumeData.nFlags & IClipVolume::eClipVolumeConnectedToOutdoor ? true : false;
		}
	}

	if (rd->m_bVolumetricFogEnabled)
		rd->GetVolumetricFog().ClearVolumeStencil();

	// Render Clip areas to stencil
	const int nClipVolumeReservedStencilBit = BIT_STENCIL_INSIDE_CLIPVOLUME;
	if (!clipVolumes.empty())
	{
		rd->FX_ResetPipe();

		const uint32 nPersFlags2 = rd->m_RP.m_PersFlags2;
		rd->m_RP.m_PersFlags2 |= RBPF2_WRITEMASK_RESERVED_STENCIL_BIT;

		//ClipVolumes
		{
			PROFILE_LABEL_SCOPE("CLIPVOLUMES TO STENCIL");
			rd->FX_PushRenderTarget(0, (CTexture*)NULL, &rd->m_DepthBufferOrigMSAA);
			rd->RT_SetViewport(0, 0, m_pLBufferDiffuseRT->GetWidth(), m_pLBufferDiffuseRT->GetHeight());
			RenderClipVolumesToStencil(nClipVolumeReservedStencilBit);
			rd->FX_PopRenderTarget(0);
		}

		// Portal blend factors
		static ICVar* pPortalsBlendCVar = iConsole->GetCVar("e_PortalsBlend");
		if (pPortalsBlendCVar->GetIVal() > 0)
		{
			rd->FX_PushRenderTarget(0, m_pResolvedStencilRT, &rd->m_DepthBufferOrigMSAA);
			rd->RT_SetViewport(0, 0, m_pLBufferDiffuseRT->GetWidth(), m_pLBufferDiffuseRT->GetHeight());
			RenderPortalBlendValues(nClipVolumeReservedStencilBit);
			rd->FX_PopRenderTarget(0);
		}

		if (rd->m_bVolumetricFogEnabled)
			rd->GetVolumetricFog().RenderClipVolumeToVolumeStencil(nClipVolumeReservedStencilBit);

		rd->m_RP.m_PersFlags2 = nPersFlags2;
	}

	rd->m_nStencilMaskRef = nClipVolumeReservedStencilBit + clipVolumes.size() + 1;

	//  Need to resolve stencil because light volumes and shadow mask overwrite stencil
	{
		// Patch z-target to allow stencil access
		CTexture* pDepthBufferRT = m_pDepthRT;
		D3DShaderResource* pZTargetOrigSRV;
		SDepthTexture sBackup = rd->FX_ReplaceMSAADepthBuffer(pDepthBufferRT, bMSAA, pZTargetOrigSRV);

#ifdef SUPPORTS_MSAA
		if (rd->FX_GetMSAAMode())
			rd->FX_MSAASampleFreqStencilSetup(MSAA_SAMPLEFREQ_PASS);
#endif

		PROFILE_LABEL_SCOPE("RESOLVE STENCIL");
		rd->FX_PushRenderTarget(0, m_pResolvedStencilRT, NULL, -1, false, 1);

		// color mask
		static CCryNameTSCRC pszResolveStencil("ResolveStencil");
		PostProcessUtils().ShBeginPass(CShaderMan::s_shDeferredShading, pszResolveStencil, FEF_DONTSETTEXTURES | FEF_DONTSETSTATES);
		rd->FX_SetState(GS_NODEPTHTEST | GS_NOCOLMASK_G | GS_NOCOLMASK_B | GS_NOCOLMASK_A);
		pDepthBufferRT->SetShaderResourceView(gcpRendD3D->m_DepthBufferOrigMSAA.pTexture->GetDeviceStencilReadOnlySRV(0, -1, bMSAA), bMSAA);
		pDepthBufferRT->Apply(4, m_nTexStatePoint, EFTT_UNKNOWN, -1, m_nBindResourceMsaa);
		GetUtils().DrawQuadFS(CShaderMan::s_shDeferredShading, false, m_pLBufferDiffuseRT->GetWidth(), m_pLBufferDiffuseRT->GetHeight());
		GetUtils().ShEndPass();
		rd->FX_PopRenderTarget(0);

		// Restore DSV/SRV
		rd->FX_RestoreMSAADepthBuffer(sBackup, pDepthBufferRT, bMSAA, pZTargetOrigSRV);

#ifdef SUPPORTS_MSAA
		rd->FX_MSAASampleFreqStencilSetup();
#endif
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

bool CDeferredShading::AmbientPass(SRenderLight* pGlobalCubemap, bool& bOutdoorVisible)
{
	PROFILE_FRAME(CDeferredShading_AmbientPass);
	PROFILE_LABEL_SCOPE("AMBIENT_PASS");

	CD3D9Renderer* const __restrict rd = gcpRendD3D;
	SRenderPipeline& RESTRICT_REFERENCE rRP = rd->m_RP;

	rd->m_RP.m_nDeferredPrimitiveID = SHAPE_PROJECTOR;
	rd->D3DSetCull(eCULL_Back, true); //fs quads should not revert test..

	const bool bMSAA = gcpRendD3D->m_RP.m_MSAAData.Type ? true : false;
#ifdef SUPPORTS_MSAA
	const int32 nNumPassesMSAA = (rd->FX_GetMSAAMode() == 1) ? 2 : 1;
	rd->FX_MSAASampleFreqStencilSetup();
#endif

	const uint64 nFlagsShaderRT = rRP.m_FlagsShader_RT;
	rRP.m_FlagsShader_RT &= ~(RT_LIGHTSMASK);

	SpecularAccEnableMRT(false);
	if (CRenderer::CV_r_DeferredShadingDepthBoundsTest)
		rd->SetDepthBoundTest(0.0f, DBT_SKY_CULL_DEPTH, false); // Disable depth bounds for ambient lookup.

	const uint32 nNumClipVolumes = RenderView()->GetClipVolumes().size();
	if (nNumClipVolumes)
		rRP.m_FlagsShader_RT |= RT_CLIPVOLUME_ID;
	else
		bOutdoorVisible = true;

	// Store global cubemap color
	Vec4 pLightDiffuse;
	if (pGlobalCubemap)
	{
		pLightDiffuse = Vec4(Vec3(pGlobalCubemap->m_Color.r, pGlobalCubemap->m_Color.g, pGlobalCubemap->m_Color.b) * rd->m_fAdaptedSceneScaleLBuffer, pGlobalCubemap->m_SpecMult);

		const float fLuminance = pGlobalCubemap->m_Color.Luminance();
		if (fLuminance > 0.001f)  // too dull => skip
		{
			rRP.m_FlagsShader_RT |= RT_GLOBAL_CUBEMAP;
			// ignore specular if it's too dull
			if (fLuminance * pLightDiffuse.w >= 0.005f)
				rRP.m_FlagsShader_RT |= RT_SPECULAR_CUBEMAP;

			rRP.m_FlagsShader_RT |= (pGlobalCubemap->m_Flags & DLF_IGNORES_VISAREAS) ? RT_GLOBAL_CUBEMAP_IGNORE_VISAREAS : 0;
		}
		else
			pGlobalCubemap = NULL;
	}

	// Patch z-target for all platforms, we need stencil access.
	CTexture* pDepthBufferRT = m_pDepthRT;
	D3DShaderResource* pZTargetOrigSRV;
	SDepthTexture sBackup = rd->FX_ReplaceMSAADepthBuffer(pDepthBufferRT, bMSAA, pZTargetOrigSRV);

	rd->FX_PushRenderTarget(0, m_pLBufferDiffuseRT, &rd->m_DepthBufferOrigMSAA, -1, false, 1);

	SpecularAccEnableMRT(true);
	if (CRenderer::CV_r_DeferredShadingDepthBoundsTest)
		rd->SetDepthBoundTest(0.0f, DBT_SKY_CULL_DEPTH, true); // Enable depth bounds - discard sky

	Vec3 vE3DParam;
	gEnv->p3DEngine->GetGlobalParameter(E3DPARAM_AMBIENT_GROUND_COLOR, vE3DParam);

	const Vec4 cOutdoorAmbient = Vec4(gEnv->p3DEngine->GetSkyColor() * rd->m_fAdaptedSceneScaleLBuffer, 0);
	const Vec4 cAmbGroundColor = Vec4(vE3DParam * rd->m_fAdaptedSceneScaleLBuffer, 0);

	Vec4 cAmbHeightParams = Vec4(gEnv->p3DEngine->GetGlobalParameter(E3DPARAM_AMBIENT_MIN_HEIGHT), gEnv->p3DEngine->GetGlobalParameter(E3DPARAM_AMBIENT_MAX_HEIGHT), 0, 0);
	cAmbHeightParams.z = 1.0 / max(0.0001f, cAmbHeightParams.y);

	rRP.m_FlagsShader_RT &= ~g_HWSR_MaskBit[HWSR_MSAA_SAMPLEFREQ_PASS];

#ifdef SUPPORTS_MSAA
	rd->FX_MSAARestoreSampleMask(); // If stencil clear occurred, restore sample mask
	for (int32 nPass = 0; nPass < nNumPassesMSAA; ++nPass)
	{
		if (nPass == 1)
		{
			PROFILE_LABEL_PUSH("AMBIENT_SAMPLEFREQPASS");
			rd->FX_MSAASampleFreqStencilSetup(MSAA_SAMPLEFREQ_PASS);
		}
#endif

	SD3DPostEffectsUtils::ShBeginPass(m_pShader, m_pAmbientOutdoorTechName, FEF_DONTSETTEXTURES | FEF_DONTSETSTATES);
	rd->FX_SetState(GS_NODEPTHTEST);

#ifdef SUPPORTS_MSAA
	if (gcpRendD3D->m_RP.m_MSAAData.Type)
	{
		rRP.m_PersFlags2 |= RBPF2_READMASK_RESERVED_STENCIL_BIT;
		const int32 nMSAAStState = STENC_FUNC((rRP.m_PersFlags2 & RBPF2_MSAA_SAMPLEFREQ_PASS) ? FSS_STENCFUNC_EQUAL : FSS_STENCFUNC_NOTEQUAL) | STENCOP_FAIL(FSS_STENCOP_KEEP) | STENCOP_ZFAIL(FSS_STENCOP_KEEP) | STENCOP_PASS(FSS_STENCOP_KEEP);
		rd->FX_SetStencilState(nMSAAStState, BIT_STENCIL_RESERVED, BIT_STENCIL_RESERVED, 0xFF);
		rd->FX_SetState(rRP.m_CurState | GS_STENCIL);
	}
#endif

	SD3DPostEffectsUtils::ShSetParamPS(m_pParamAmbient, cOutdoorAmbient);
	SD3DPostEffectsUtils::ShSetParamPS(m_pParamAmbientGround, cAmbGroundColor);
	SD3DPostEffectsUtils::ShSetParamPS(m_pParamAmbientHeight, cAmbHeightParams);

	if (nNumClipVolumes)
		m_pShader->FXSetPSFloat(m_pClipVolumeParams, m_vClipVolumeParams, min((uint32)MAX_DEFERRED_CLIP_VOLUMES, nNumClipVolumes + VIS_AREAS_OUTDOOR_STENCIL_OFFSET));

	m_pNormalsRT->Apply(0, m_nTexStatePoint, EFTT_UNKNOWN, -1, m_nBindResourceMsaa);

	if (pGlobalCubemap)
	{
		CTexture* const texDiffuse = (CTexture*)pGlobalCubemap->GetDiffuseCubemap();
		CTexture* const texSpecular = (CTexture*)pGlobalCubemap->GetSpecularCubemap();

		SD3DPostEffectsUtils::SetTexture(texDiffuse->GetTextureType() < eTT_Cube ? CTexture::s_ptexNoTextureCM : texDiffuse, 1, FILTER_BILINEAR, 1, texDiffuse->IsSRGB());
		SD3DPostEffectsUtils::SetTexture(texSpecular->GetTextureType() < eTT_Cube ? CTexture::s_ptexNoTextureCM : texSpecular, 2, FILTER_TRILINEAR, 1, texSpecular->IsSRGB());

		SD3DPostEffectsUtils::ShSetParamPS(m_pParamLightDiffuse, pLightDiffuse);
		const Vec4 vCubemapParams(IntegerLog2((uint32)texSpecular->GetWidthNonVirtual()) - 2, 0, 0, 0);   // Use 4x4 mip for lowest gloss values
		m_pShader->FXSetPSFloat(m_pGeneralParams, &vCubemapParams, 1);

		// Directional occlusion
		Vec4 ssdoParams(CRenderer::CV_r_ssdoAmountDirect, CRenderer::CV_r_ssdoAmountAmbient, CRenderer::CV_r_ssdoAmountReflection, 0);
		Vec4 ssdoParamsNull(0, 0, 0, 0);
		static CCryNameR ssdoParamsName("SSDOParams");
		m_pShader->FXSetPSFloat(ssdoParamsName, CRenderer::CV_r_ssdo ? &ssdoParams : &ssdoParamsNull, 1);
		CTexture::s_ptexSceneNormalsBent->Apply(8, m_nTexStatePoint);
	}

	// DX11 requires explicitly bind depth then stencil to have access to both depth and stencil read from shader. Formats also must match
	pDepthBufferRT->SetShaderResourceView(gcpRendD3D->m_DepthBufferOrigMSAA.pTexture->GetDeviceDepthReadOnlySRV(0, -1, bMSAA), bMSAA);
	pDepthBufferRT->Apply(3, m_nTexStatePoint, EFTT_UNKNOWN, -1, m_nBindResourceMsaa);
	pDepthBufferRT->SetShaderResourceView(gcpRendD3D->m_DepthBufferOrigMSAA.pTexture->GetDeviceStencilReadOnlySRV(0, -1, bMSAA), bMSAA);
	pDepthBufferRT->Apply(4, m_nTexStatePoint, EFTT_UNKNOWN, -1, m_nBindResourceMsaa);
	m_pMSAAMaskRT->Apply(5, m_nTexStatePoint);
	m_pSpecularRT->Apply(7, m_nTexStatePoint);
	m_pDiffuseRT->Apply(11, m_nTexStatePoint);

	CTexture::s_ptexEnvironmentBRDF->Apply(10, m_nTexStateLinear);

	SD3DPostEffectsUtils::DrawFullScreenTriWPOS(m_pLBufferDiffuseRT->GetWidth(), m_pLBufferDiffuseRT->GetHeight(), 0, &gcpRendD3D->m_FullResRect);
	SD3DPostEffectsUtils::ShEndPass();

#ifdef SUPPORTS_MSAA
	if (nPass == 1)
	{
		PROFILE_LABEL_POP("AMBIENT_SAMPLEFREQPASS");
	}
}
#endif

#ifdef SUPPORTS_MSAA
	if (gcpRendD3D->m_RP.m_MSAAData.Type)
		rRP.m_PersFlags2 &= ~RBPF2_READMASK_RESERVED_STENCIL_BIT;
#endif

	// Restore DSV/SRV
	rd->FX_RestoreMSAADepthBuffer(sBackup, pDepthBufferRT, bMSAA, pZTargetOrigSRV);
	rd->FX_PopRenderTarget(0);

	CTexture::s_ptexBlack->Apply(3, m_nTexStatePoint);
	CTexture::s_ptexBlack->Apply(4, m_nTexStatePoint);
	rd->FX_SetActiveRenderTargets();

	// Follow up with NVidia. Seems driver/pixel quads synchronization? Wrong behavior when reading from native stencil/depth.
	// Luckily can clear stencil since vis areas/decals tag not needed from here on
	if (CRenderer::CV_r_DeferredShadingAmbientSClear)
	{
		rd->EF_ClearTargetsImmediately(FRT_CLEAR_STENCIL, Clr_Unused.r, 1);
#ifdef SUPPORTS_MSAA
		rd->FX_MSAARestoreSampleMask(); // If stencil clear occurred, restore sample mask
#endif
	}

	rRP.m_FlagsShader_RT = nFlagsShaderRT;

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void CDeferredShading::DeferredCubemaps(const RenderLightsArray& rCubemaps, const uint32 nStartIndex /* = 0 */)
{
	if (nStartIndex < rCubemaps.size() && CRenderer::CV_r_DeferredShadingEnvProbes)
	{
		// apply deferred cubemaps first
		PROFILE_LABEL_SCOPE("DEFERRED_CUBEMAPS");

		for (uint32 nCurrentCubemap = nStartIndex; nCurrentCubemap < rCubemaps.size(); ++nCurrentCubemap)
		{
			const SRenderLight& pDL = rCubemaps[nCurrentCubemap];
			if (pDL.m_Flags & (DLF_FAKE | DLF_VOLUMETRIC_FOG_ONLY))
				continue;
			DeferredCubemapPass(&pDL);
			m_nLightsProcessedCount++;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void CDeferredShading::DeferredCubemapPass(const SRenderLight* const __restrict pDL)
{
	PROFILE_FRAME(CDeferredShading_CubemapPass);
	PROFILE_LABEL(pDL->m_sName);

	float scissorInt2Float[] = { (float)(pDL->m_sX), (float)(pDL->m_sY), (float)(pDL->m_sWidth), (float)(pDL->m_sHeight) };

	CD3D9Renderer* const __restrict rd = gcpRendD3D;
	SRenderPipeline& RESTRICT_REFERENCE rRP = rd->m_RP;

	rRP.m_nDeferredPrimitiveID = SHAPE_PROJECTOR;

	bool bStencilMask = CRenderer::CV_r_DeferredShadingStencilPrepass || CRenderer::CV_r_DebugLightVolumes;
	bool bUseLightVolumes = false;
	bool bHasSpecular = false;

#ifdef SUPPORTS_MSAA
	const int32 nNumPassesMSAA = (rd->FX_GetMSAAMode() == 1) ? 2 : 1;
	rd->FX_MSAASampleFreqStencilSetup();
#endif

	const uint64 nOldFlags = rRP.m_FlagsShader_RT;

	rRP.m_FlagsShader_RT &= ~(RT_CLIPVOLUME_ID | RT_LIGHTSMASK | RT_GLOBAL_CUBEMAP | RT_SPECULAR_CUBEMAP | RT_BOX_PROJECTION);

	uint nNumClipVolumes = RenderView()->GetClipVolumes().size();
	rd->m_RP.m_FlagsShader_RT |= (pDL->m_Flags & DLF_BOX_PROJECTED_CM) ? RT_BOX_PROJECTION : 0;
	rd->m_RP.m_FlagsShader_RT |= nNumClipVolumes > 0 ? RT_CLIPVOLUME_ID : 0;

	// Store light properties (color/radius, position relative to camera, rect, z bounds)
	Vec4 pLightDiffuse = Vec4(pDL->m_Color.r, pDL->m_Color.g, pDL->m_Color.b, pDL->m_SpecMult);

	const float fInvRadius = (pDL->m_fRadius <= 0) ? 1.0f : 1.0f / pDL->m_fRadius;
	const Vec4 pLightPosCS = Vec4(pDL->m_Origin - m_pCamPos, fInvRadius);
	const float fAttenFalloffMax = max(pDL->GetFalloffMax(), 1e-3f);

	const bool bReverseDepth = (rRP.m_TI[rRP.m_nProcessThreadID].m_PersFlags & RBPF_REVERSE_DEPTH) != 0;
	Vec4 pDepthBounds = GetLightDepthBounds(pDL, bReverseDepth);

	// avoiding LHS, comment out if we ever use different resolution for light accumulation target
	Vec4 pLightRect = Vec4(scissorInt2Float[0], scissorInt2Float[1], scissorInt2Float[2], scissorInt2Float[3]);
	Vec4 scaledLightRect = Vec4(pLightRect.x * rRP.m_CurDownscaleFactor.x, pLightRect.y * rRP.m_CurDownscaleFactor.y,
	                            pLightRect.z * rRP.m_CurDownscaleFactor.x, pLightRect.w * rRP.m_CurDownscaleFactor.y);

	assert(!(pDL->m_Flags & DLF_PROJECT));

	if (CRenderer::CV_r_DeferredShadingLightLodRatio)
	{
		float fLightArea = pLightRect.z * pLightRect.w;
		float fScreenArea = (float) m_nCurTargetWidth * m_nCurTargetHeight;
		float fLightRatio = fLightArea / fScreenArea;

		const float fMinScreenAreaRatioThreshold = 0.01f;   // 1% of screen by default

		const float fDrawVolumeThres = 0.01f;
		if ((fLightRatio* CRenderer::CV_r_DeferredShadingLightLodRatio) < fDrawVolumeThres)
		{
			//scissor + depthbound test only
			bStencilMask = false;
		}
	}

	uint16 scaledX = (uint16)(scaledLightRect.x);
	uint16 scaledY = (uint16)(scaledLightRect.y);
	uint16 scaledWidth = (uint16)(scaledLightRect.z) + 1;
	uint16 scaledHeight = (uint16)(scaledLightRect.w) + 1;

	if (CRenderer::CV_r_DeferredShadingScissor)
	{
		SetupScissors(true, scaledX, scaledY, scaledWidth, scaledHeight);
	}

	if (bStencilMask)
	{
		SpecularAccEnableMRT(false);
		rd->SetDepthBoundTest(0.0f, 1.0f, false);
		rd->FX_StencilFrustumCull(-1, pDL, NULL, 0);
	}
	else if (rd->m_bDeviceSupports_NVDBT && CRenderer::CV_r_DeferredShadingDepthBoundsTest == 1)
		rd->SetDepthBoundTest(pDepthBounds.x, pDepthBounds.z, true);

	// todo: try out on consoles if DBT helps on light pass (on light stencil prepass is actually slower)
	if (rd->m_bDeviceSupports_NVDBT && bStencilMask && CRenderer::CV_r_DeferredShadingDepthBoundsTest && CRenderer::CV_r_deferredshadingDBTstencil)
		rd->SetDepthBoundTest(pDepthBounds.x, pDepthBounds.z, true);

	const float fFadeout = pDL->m_Color.a;  // fade-out intensity stored in alpha
	const float fLuminance = pDL->m_Color.Luminance() * fFadeout;

	if (fLuminance * pLightDiffuse.w >= 0.03f)  // if specular intensity is too low, skip it
	{
		rRP.m_FlagsShader_RT |= RT_SPECULAR_CUBEMAP;
		bHasSpecular = true;
	}

	SpecularAccEnableMRT(bHasSpecular);

	// Apply LBuffers range rescale
	pLightDiffuse.x *= rd->m_fAdaptedSceneScaleLBuffer;
	pLightDiffuse.y *= rd->m_fAdaptedSceneScaleLBuffer;
	pLightDiffuse.z *= rd->m_fAdaptedSceneScaleLBuffer;

	if (CRenderer::CV_r_deferredshadingLightVolumes)
	{
		const Plane* pNearPlane(rd->GetCamera().GetFrustumPlane(FR_PLANE_NEAR));
		Vec3 n = pNearPlane->n;
		Vec3 e = pDL->m_ProbeExtents;
		Vec3 u0 = pDL->m_ObjMatrix.GetColumn0().GetNormalized();
		Vec3 u1 = pDL->m_ObjMatrix.GetColumn1().GetNormalized();
		Vec3 u2 = pDL->m_ObjMatrix.GetColumn2().GetNormalized();

		// Check if OBB intersects near plane
		float r = e.x * fabs(n.dot(u0)) + e.y * fabs(n.dot(u1)) + e.z * fabs(n.dot(u2));
		float s = pNearPlane->DistFromPlane(pDL->m_Origin);
		bUseLightVolumes = fabs(s) > r;
	}

	int MultplyState = m_nRenderState;
	MultplyState &= ~GS_BLEND_MASK;
	MultplyState |= GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA;

	if (CRenderer::CV_r_DeferredShadingDebug == 2)
	{
		// Debug mode
		MultplyState &= ~GS_BLEND_MASK;
		MultplyState |= (GS_BLSRC_ONE | GS_BLDST_ONE);
	}

	if (bStencilMask)
		MultplyState |= GS_STENCIL;

	MultplyState &= ~(GS_NODEPTHTEST | GS_DEPTHFUNC_MASK);      // Ensure zcull used.
	MultplyState |= GS_DEPTHFUNC_LEQUAL;

#ifdef SUPPORTS_MSAA
	rd->FX_MSAARestoreSampleMask(); // If stencil clear occurred, restore sample mask

	for (int32 nPass = 0; nPass < nNumPassesMSAA; ++nPass)
	{
		if (nPass == 1)
		{
			PROFILE_LABEL_PUSH("DEFERRED_CUBEMAP_SAMPLEFREQPASS");
			rd->FX_MSAASampleFreqStencilSetup(MSAA_SAMPLEFREQ_PASS);
		}
#endif

	// Render..
	if (bUseLightVolumes)
	{
		rRP.m_FlagsShader_RT |= g_HWSR_MaskBit[HWSR_CUBEMAP0];
		SD3DPostEffectsUtils::ShBeginPass(m_pShader, m_pCubemapsVolumeTechName, FEF_DONTSETTEXTURES | FEF_DONTSETSTATES);

	}
	else
	{
		rRP.m_FlagsShader_RT &= ~g_HWSR_MaskBit[HWSR_CUBEMAP0];
		SD3DPostEffectsUtils::ShBeginPass(m_pShader, m_pCubemapsTechName, FEF_DONTSETTEXTURES | FEF_DONTSETSTATES);
	}

	rd->FX_SetState(MultplyState);

#ifdef SUPPORTS_MSAA
	if (gcpRendD3D->m_RP.m_MSAAData.Type)
	{
		rRP.m_PersFlags2 |= RBPF2_READMASK_RESERVED_STENCIL_BIT;
		const int32 nMSAAStState = STENC_FUNC(FSS_STENCFUNC_EQUAL) | STENCOP_FAIL(FSS_STENCOP_KEEP) | STENCOP_ZFAIL(FSS_STENCOP_KEEP) | STENCOP_PASS(FSS_STENCOP_KEEP);
		rd->FX_SetStencilState(nMSAAStState, (bStencilMask ? rd->m_nStencilMaskRef : 0) | ((rRP.m_PersFlags2 & RBPF2_MSAA_SAMPLEFREQ_PASS) ? BIT_STENCIL_RESERVED : 0), bStencilMask ? 0xFF : BIT_STENCIL_RESERVED, 0xFF);
		rd->FX_SetState(rRP.m_CurState | GS_STENCIL);
	}
	else
#endif
	if (bStencilMask)
		rd->FX_StencilTestCurRef(true, false);

	m_pShader->FXSetPSFloat(m_pParamLightPos, &pLightPosCS, 1);
	m_pShader->FXSetPSFloat(m_pParamLightDiffuse, &pLightDiffuse, 1);

	m_pDepthRT->Apply(0, m_nTexStatePoint, EFTT_UNKNOWN, -1, (rRP.m_PersFlags2 & RBPF2_MSAA_SAMPLEFREQ_PASS) ? m_nBindResourceMsaa : SResourceView::DefaultView);
	m_pNormalsRT->Apply(1, m_nTexStatePoint, EFTT_UNKNOWN, -1, (rRP.m_PersFlags2 & RBPF2_MSAA_SAMPLEFREQ_PASS) ? m_nBindResourceMsaa : SResourceView::DefaultView);
	m_pDiffuseRT->Apply(2, m_nTexStatePoint, EFTT_UNKNOWN, -1, (rRP.m_PersFlags2 & RBPF2_MSAA_SAMPLEFREQ_PASS) ? m_nBindResourceMsaa : SResourceView::DefaultView);
	m_pSpecularRT->Apply(3, m_nTexStatePoint, EFTT_UNKNOWN, -1, (rRP.m_PersFlags2 & RBPF2_MSAA_SAMPLEFREQ_PASS) ? m_nBindResourceMsaa : SResourceView::DefaultView);

	static CCryNameR pszProbeOBBParams("g_mProbeOBBParams");
	Vec4 vProbeOBBParams[3];
	vProbeOBBParams[0] = Vec4(pDL->m_ObjMatrix.GetColumn0().GetNormalized(), 1 / pDL->m_ProbeExtents.x);
	vProbeOBBParams[1] = Vec4(pDL->m_ObjMatrix.GetColumn1().GetNormalized(), 1 / pDL->m_ProbeExtents.y);
	vProbeOBBParams[2] = Vec4(pDL->m_ObjMatrix.GetColumn2().GetNormalized(), 1 / pDL->m_ProbeExtents.z);
	m_pShader->FXSetPSFloat(pszProbeOBBParams, vProbeOBBParams, 3);

	if (pDL->m_Flags & DLF_BOX_PROJECTED_CM)
	{
		static CCryNameR pszBoxProjectionMin("g_vBoxProjectionMin");
		static CCryNameR pszBoxProjectionMax("g_vBoxProjectionMax");
		Vec4 vBoxProjectionMin(-pDL->m_fBoxLength * 0.5f, -pDL->m_fBoxWidth * 0.5f, -pDL->m_fBoxHeight * 0.5f, 0);
		Vec4 vBoxProjectionMax(pDL->m_fBoxLength * 0.5f, pDL->m_fBoxWidth * 0.5f, pDL->m_fBoxHeight * 0.5f, 0);

		m_pShader->FXSetPSFloat(pszBoxProjectionMin, &vBoxProjectionMin, 1);
		m_pShader->FXSetPSFloat(pszBoxProjectionMax, &vBoxProjectionMax, 1);
	}

	CTexture* texDiffuse = (CTexture*)pDL->GetDiffuseCubemap();
	CTexture* texSpecular = (CTexture*)pDL->GetSpecularCubemap();

	// Use 4x4 mip for lowest gloss values
	Vec4 vCubemapParams(IntegerLog2((uint32)texSpecular->GetWidthNonVirtual()) - 2, 0, 0, 0);
	m_pShader->FXSetPSFloat(m_pGeneralParams, &vCubemapParams, 1);
	SD3DPostEffectsUtils::SetTexture(texDiffuse, 5, FILTER_BILINEAR, 1, texDiffuse->IsSRGB());
	SD3DPostEffectsUtils::SetTexture(texSpecular, 6, FILTER_TRILINEAR, 1, texSpecular->IsSRGB());

	uint32 stencilID = (pDL->m_nStencilRef[1] + 1) << 16 | pDL->m_nStencilRef[0] + 1;
	const Vec4 vAttenParams(fFadeout, *alias_cast<float*>(&stencilID), 0, fAttenFalloffMax);
	m_pShader->FXSetPSFloat(m_pAttenParams, &vAttenParams, 1);

	m_pMSAAMaskRT->Apply(7, m_nTexStatePoint);

	// Directional occlusion
	Vec4 ssdoParams(CRenderer::CV_r_ssdoAmountDirect, CRenderer::CV_r_ssdoAmountAmbient, CRenderer::CV_r_ssdoAmountReflection, 0);
	Vec4 ssdoParamsNull(0, 0, 0, 0);
	static CCryNameR ssdoParamsName("SSDOParams");
	m_pShader->FXSetPSFloat(ssdoParamsName, CRenderer::CV_r_ssdo ? &ssdoParams : &ssdoParamsNull, 1);
	CTexture::s_ptexSceneNormalsBent->Apply(8, m_nTexStatePoint);

	if (nNumClipVolumes > 0)
	{
		m_pResolvedStencilRT->Apply(9, m_nTexStatePoint, -1, -1, -1);
		m_pShader->FXSetPSFloat(m_pClipVolumeParams, m_vClipVolumeParams, min((uint32)MAX_DEFERRED_CLIP_VOLUMES, nNumClipVolumes + VIS_AREAS_OUTDOOR_STENCIL_OFFSET));
	}

	CTexture::s_ptexEnvironmentBRDF->Apply(10, m_nTexStateLinear);

	if (bUseLightVolumes)
	{
		gcpRendD3D->D3DSetCull(eCULL_Back);

		Matrix33 rotMat(pDL->m_ObjMatrix.GetColumn0().GetNormalized() * pDL->m_ProbeExtents.x,
		                pDL->m_ObjMatrix.GetColumn1().GetNormalized() * pDL->m_ProbeExtents.y,
		                pDL->m_ObjMatrix.GetColumn2().GetNormalized() * pDL->m_ProbeExtents.z);
		Matrix34 mUnitVolumeToWorld = Matrix34::CreateTranslationMat(pDL->m_Origin) * rotMat * Matrix34::CreateScale(Vec3(2, 2, 2), Vec3(-1, -1, -1));

		DrawLightVolume(SHAPE_BOX, mUnitVolumeToWorld.GetTransposed());
	}
	else
	{
		rd->D3DSetCull(eCULL_Back, true);
		SD3DPostEffectsUtils::DrawFullScreenTriWPOS(m_pLBufferDiffuseRT->GetWidth(), m_pLBufferDiffuseRT->GetHeight(), pDepthBounds.x);
	}

	SD3DPostEffectsUtils::ShEndPass();

#ifdef SUPPORTS_MSAA
	if (nPass == 1)
	{
		rd->FX_MSAASampleFreqStencilSetup();
		PROFILE_LABEL_POP("DEFERRED_CUBEMAP_SAMPLEFREQPASS");
	}
}
#endif

#ifdef SUPPORTS_MSAA
	if (gcpRendD3D->m_RP.m_MSAAData.Type)
		rRP.m_PersFlags2 &= ~RBPF2_READMASK_RESERVED_STENCIL_BIT;
#endif

	if (bStencilMask)
	{
		rd->FX_StencilTestCurRef(false);
	}
	else if (rd->m_bDeviceSupports_NVDBT && CRenderer::CV_r_DeferredShadingDepthBoundsTest == 1)
		rd->SetDepthBoundTest(0.f, 1.f, false);

	if (rd->m_bDeviceSupports_NVDBT && bStencilMask && CRenderer::CV_r_DeferredShadingDepthBoundsTest && CRenderer::CV_r_deferredshadingDBTstencil)
		rd->SetDepthBoundTest(0.f, 1.f, false);

	SpecularAccEnableMRT(true);

	if (CRenderer::CV_r_DeferredShadingScissor)
		rd->EF_Scissor(false, 0, 0, 0, 0);

	rRP.m_FlagsShader_RT = nOldFlags;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void CDeferredShading::ScreenSpaceReflectionPass()
{
	if (!CRenderer::CV_r_SSReflections || !CTexture::s_ptexHDRTarget)  // Sketch mode disables HDR rendering
		return;

	CD3D9Renderer* const __restrict rd = gcpRendD3D;
	const uint32 nPrevPersFlags = rd->m_RP.m_TI[rd->m_RP.m_nProcessThreadID].m_PersFlags;

	Matrix44 mViewProj = rd->m_ViewMatrix * rd->m_ProjMatrix;

	if (rd->m_RP.m_TI[rd->m_RP.m_nProcessThreadID].m_PersFlags & RBPF_REVERSE_DEPTH)
	{
		mViewProj = ReverseDepthHelper::Convert(mViewProj);
		rd->m_RP.m_TI[rd->m_RP.m_nProcessThreadID].m_PersFlags &= ~RBPF_REVERSE_DEPTH;
	}

	const int frameID = GetUtils().m_iFrameCounter;

	Matrix44 mViewport(0.5f, 0, 0, 0,
	                   0, -0.5f, 0, 0,
	                   0, 0, 1.0f, 0,
	                   0.5f, 0.5f, 0, 1.0f);
	uint32 numGPUs = rd->GetActiveGPUCount();
	Matrix44 mViewProjPrev = m_prevViewProj[max((frameID - (int)numGPUs) % MAX_GPU_NUM, 0)] * mViewport;

	PROFILE_LABEL_SCOPE("SS_REFLECTIONS");

	const uint64 shaderFlags = rd->m_RP.m_FlagsShader_RT;

	// Get current viewport
	int prevVpX, prevVpY, prevVpWidth, prevVpHeight;
	gRenDev->GetViewport(&prevVpX, &prevVpY, &prevVpWidth, &prevVpHeight);

	{
		PROFILE_LABEL_SCOPE("SSR_RAYTRACE");

		CTexture* dstTex = CRenderer::CV_r_SSReflHalfRes ? CTexture::s_ptexHDRTargetScaled[0] : CTexture::s_ptexHDRTarget;

		rd->FX_PushRenderTarget(0, dstTex, NULL);
		rd->RT_SetViewport(0, 0, dstTex->GetWidth(), dstTex->GetHeight());

		rd->FX_SetState(GS_NODEPTHTEST);
		SD3DPostEffectsUtils::ShBeginPass(m_pShader, m_pReflectionTechName, FEF_DONTSETTEXTURES | FEF_DONTSETSTATES);

		m_pDepthRT->Apply(0, m_nTexStatePoint);
		m_pNormalsRT->Apply(1, m_nTexStateLinear);
		m_pSpecularRT->Apply(2, m_nTexStateLinear);
		CTexture::s_ptexZTargetScaled->Apply(3, m_nTexStatePoint);
		SD3DPostEffectsUtils::SetTexture(CTexture::s_ptexHDRTargetPrev, 4, FILTER_LINEAR, TADDR_BORDER);
		CTexture::s_ptexHDRMeasuredLuminance[rd->RT_GetCurrGpuID()]->Apply(5, m_nTexStatePoint);    // Current luminance

		static CCryNameR param0("g_mViewProj");
		m_pShader->FXSetPSFloat(param0, (Vec4*) mViewProj.GetData(), 4);

		static CCryNameR param1("g_mViewProjPrev");
		m_pShader->FXSetPSFloat(param1, (Vec4*) mViewProjPrev.GetData(), 4);

		SD3DPostEffectsUtils::DrawFullScreenTriWPOS(dstTex->GetWidth(), dstTex->GetHeight());
		SD3DPostEffectsUtils::ShEndPass();

		rd->FX_PopRenderTarget(0);
	}

	if (!CRenderer::CV_r_SSReflHalfRes)
	{
		PostProcessUtils().StretchRect(CTexture::s_ptexHDRTarget, CTexture::s_ptexHDRTargetScaled[0]);
	}

	// Convolve sharp reflections
	PostProcessUtils().StretchRect(CTexture::s_ptexHDRTargetScaled[0], CTexture::s_ptexHDRTargetScaled[1]);
	PostProcessUtils().TexBlurGaussian(CTexture::s_ptexHDRTargetScaled[1], 1, 1.0f, 3.0f, false, 0, false, CTexture::s_ptexHDRTargetScaledTempRT[1]);

	PostProcessUtils().StretchRect(CTexture::s_ptexHDRTargetScaled[1], CTexture::s_ptexHDRTargetScaled[2]);
	PostProcessUtils().TexBlurGaussian(CTexture::s_ptexHDRTargetScaled[2], 1, 1.0f, 3.0f, false, 0, false, CTexture::s_ptexHDRTargetScaledTempRT[2]);

	PostProcessUtils().StretchRect(CTexture::s_ptexHDRTargetScaled[2], CTexture::s_ptexHDRTargetScaled[3]);
	PostProcessUtils().TexBlurGaussian(CTexture::s_ptexHDRTargetScaled[3], 1, 1.0f, 3.0f, false, 0, false, CTexture::s_ptexHDRTargetScaledTempRT[3]);

	{
		PROFILE_LABEL_SCOPE("SSR_COMPOSE");

		static CCryNameTSCRC tech("SSReflection_Comp");

		CTexture* dstTex = CTexture::s_ptexHDRTargetScaledTmp[0];
		dstTex->Unbind();

		rd->FX_SetState(GS_NODEPTHTEST);
		rd->FX_PushRenderTarget(0, dstTex, NULL);
		rd->RT_SetViewport(0, 0, dstTex->GetWidth(), dstTex->GetHeight());

		m_pSpecularRT->Apply(0, m_nTexStateLinear);
		CTexture::s_ptexHDRTargetScaled[0]->Apply(1, m_nTexStateLinear);
		CTexture::s_ptexHDRTargetScaled[1]->Apply(2, m_nTexStateLinear);
		CTexture::s_ptexHDRTargetScaled[2]->Apply(3, m_nTexStateLinear);
		CTexture::s_ptexHDRTargetScaled[3]->Apply(4, m_nTexStateLinear);

		SD3DPostEffectsUtils::ShBeginPass(m_pShader, tech, FEF_DONTSETTEXTURES | FEF_DONTSETSTATES);
		SD3DPostEffectsUtils::DrawFullScreenTri(dstTex->GetWidth(), dstTex->GetHeight());
		SD3DPostEffectsUtils::ShEndPass();
		rd->FX_PopRenderTarget(0);
	}

	// Restore the old flags
	rd->m_RP.m_FlagsShader_RT = shaderFlags;
	rd->m_RP.m_TI[rd->m_RP.m_nProcessThreadID].m_PersFlags = nPrevPersFlags;

	if (rd->m_RP.m_TI[rd->m_RP.m_nProcessThreadID].m_PersFlags & RBPF_REVERSE_DEPTH)
	{
		uint32 depthState = ReverseDepthHelper::ConvertDepthFunc(rd->m_RP.m_CurState);
		rd->FX_SetState(rd->m_RP.m_CurState, rd->m_RP.m_CurAlphaRef, depthState);
	}

	rd->RT_SetViewport(prevVpX, prevVpY, prevVpWidth, prevVpHeight);

	// Array used for MGPU support
	m_prevViewProj[frameID % MAX_GPU_NUM] = mViewProj;
}

void CDeferredShading::ApplySSReflections()
{
	if (!CRenderer::CV_r_SSReflections || !CTexture::s_ptexHDRTarget)  // Sketch mode disables HDR rendering
		return;

	CTexture* pSSRTarget = CTexture::s_ptexHDRTargetScaledTmp[0];

	PROFILE_LABEL_SCOPE("SSR_APPLY");

	SpecularAccEnableMRT(false);
	gcpRendD3D->FX_PushRenderTarget(0, m_pLBufferSpecularRT, NULL);

	static CCryNameTSCRC tech("ApplySSR");
	SD3DPostEffectsUtils::ShBeginPass(m_pShader, tech, FEF_DONTSETTEXTURES | FEF_DONTSETSTATES);
	gcpRendD3D->FX_SetState(GS_NODEPTHTEST | GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA);

	pSSRTarget->Apply(0, m_nTexStateLinear);
	m_pDepthRT->Apply(1, m_nTexStatePoint);
	m_pNormalsRT->Apply(2, m_nTexStatePoint);
	m_pDiffuseRT->Apply(3, m_nTexStatePoint);
	m_pSpecularRT->Apply(4, m_nTexStatePoint);
	CTexture::s_ptexEnvironmentBRDF->Apply(5, m_nTexStateLinear);

	SD3DPostEffectsUtils::DrawFullScreenTriWPOS(pSSRTarget->GetWidth(), pSSRTarget->GetHeight());
	SD3DPostEffectsUtils::ShEndPass();

	gcpRendD3D->FX_PopRenderTarget(0);
	SpecularAccEnableMRT(true);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void CDeferredShading::HeightMapOcclusionPass(ShadowMapFrustum*& pHeightMapFrustum, CTexture*& pHeightMapAOScreenDepth, CTexture*& pHeightmapAO)
{
	CD3D9Renderer* const __restrict rd = gcpRendD3D;
	const int nThreadID = rd->m_RP.m_nProcessThreadID;
	pHeightMapFrustum = NULL;
	pHeightMapAOScreenDepth = pHeightmapAO = NULL;

	if (!CRenderer::CV_r_HeightMapAO || rd->m_RP.m_pSunLight == NULL)
		return;

	// prepare height map ao frustum
	auto arrHeightmapAOFrustums = RenderView()->GetShadowFrustumsByType(CRenderView::eShadowFrustumRenderType_HeightmapAO);
	if (!arrHeightmapAOFrustums.empty())
	{
		ShadowMapFrustum* pFr = arrHeightmapAOFrustums.front()->pFrustum;
		if (pFr->pDepthTex)
		{
			rd->ConfigShadowTexgen(0, pFr, -1, false, false);
			pHeightMapFrustum = pFr;
		}
	}

	if (pHeightMapFrustum)
	{
		PROFILE_LABEL_SCOPE("HEIGHTMAP_OCC");

		const int resolutionIndex = clamp_tpl(CRenderer::CV_r_HeightMapAO - 1, 0, 2);
		CTexture* pDepth[] = { CTexture::s_ptexZTargetScaled2, CTexture::s_ptexZTargetScaled, m_pDepthRT };
		CTexture* pDst = CTexture::s_ptexHeightMapAO[0];

		if (!CTexture::s_ptexHeightMapAODepth[0]->IsResolved())
		{
			PROFILE_LABEL_SCOPE("GENERATE_MIPS");

			rd->GetDeviceContext().CopySubresourceRegion(
			  CTexture::s_ptexHeightMapAODepth[1]->GetDevTexture()->GetBaseTexture(), 0, 0, 0, 0,
			  CTexture::s_ptexHeightMapAODepth[0]->GetDevTexture()->GetBaseTexture(), 0, NULL
			  );

			CTexture::s_ptexHeightMapAODepth[1]->GenerateMipMaps();
			CTexture::s_ptexHeightMapAODepth[0]->SetResolved(true);
		}

		// Generate occlusion
		{
			PROFILE_LABEL_SCOPE("GENERATE_OCCL");

			rd->FX_PushRenderTarget(0, pDst, NULL);

			static CCryNameTSCRC tech("HeightMapAOPass");
			SD3DPostEffectsUtils::ShBeginPass(m_pShader, tech, FEF_DONTSETSTATES);
			rd->FX_SetState(GS_NODEPTHTEST);

			int TsLinearWithBorder = CTexture::GetTexState(STexState(FILTER_TRILINEAR, TADDR_BORDER, TADDR_BORDER, TADDR_BORDER, 0xFFFFFFFF));

			m_pNormalsRT->Apply(0, m_nTexStatePoint);
			pDepth[resolutionIndex]->Apply(1, m_nTexStatePoint);
			CTexture::s_ptexSceneNormalsBent->Apply(10, m_nTexStatePoint);
			CTexture::s_ptexHeightMapAODepth[1]->Apply(11, TsLinearWithBorder);

			Matrix44A matHMAOTransform = gRenDev->m_TempMatrices[0][0];
			Matrix44A texToWorld = matHMAOTransform.GetInverted();

			static CCryNameR paramNameHMAO("HMAO_Params");
			const float texelsPerMeter = CRenderer::CV_r_HeightMapAOResolution / CRenderer::CV_r_HeightMapAORange;
			const bool enableMinMaxSampling = CRenderer::CV_r_HeightMapAO < 3;
			Vec4 paramHMAO(CRenderer::CV_r_HeightMapAOAmount, texelsPerMeter / CTexture::s_ptexHeightMapAODepth[1]->GetWidth(), enableMinMaxSampling ? 1.0 : 0.0, 0.0f);
			m_pShader->FXSetPSFloat(paramNameHMAO, &paramHMAO, 1);

			static CCryNameR paramNameHMAO_TexToWorldT("HMAO_TexToWorldTranslation");
			Vec4 vTranslation = Vec4(texToWorld.m03, texToWorld.m13, texToWorld.m23, 0);
			m_pShader->FXSetPSFloat(paramNameHMAO_TexToWorldT, &vTranslation, 1);

			static CCryNameR paramNameHMAO_TexToWorldS("HMAO_TexToWorldScale");
			Vec4 vScale = Vec4(texToWorld.m00, texToWorld.m11, texToWorld.m22, 1);
			m_pShader->FXSetPSFloat(paramNameHMAO_TexToWorldS, &vScale, 1);

			static CCryNameR paramHMAO_Transform("HMAO_Transform");
			m_pShader->FXSetPSFloat(paramHMAO_Transform, (Vec4*)matHMAOTransform.GetData(), 4);

			SD3DPostEffectsUtils::DrawFullScreenTriWPOS(pDst->GetWidth(), pDst->GetHeight());
			SD3DPostEffectsUtils::ShEndPass();

			rd->FX_PopRenderTarget(0);
		}

		// depth aware blur
		{
			PROFILE_LABEL_SCOPE("BLUR");

			CShader* pSH = rd->m_cEF.s_ShaderShadowBlur;

			CTexture* tpSrc = pDst;
			rd->FX_PushRenderTarget(0, CTexture::s_ptexHeightMapAO[1], NULL);

			const Vec4* pClipVolumeParams = NULL;
			uint32 nClipVolumeCount = 0;
			CDeferredShading::Instance().GetClipVolumeParams(pClipVolumeParams, nClipVolumeCount);

			rd->m_RP.m_FlagsShader_RT &= ~(g_HWSR_MaskBit[HWSR_SAMPLE0]);
			rd->m_RP.m_FlagsShader_RT |= nClipVolumeCount > 0 ? g_HWSR_MaskBit[HWSR_SAMPLE0] : 0;

			static CCryNameTSCRC TechName("HMAO_Blur");
			SD3DPostEffectsUtils::ShBeginPass(pSH, TechName, FEF_DONTSETTEXTURES | FEF_DONTSETSTATES);

			tpSrc->Apply(0, m_nTexStatePoint, -2);
			pDepth[resolutionIndex]->Apply(1, m_nTexStatePoint, -2);

			if (nClipVolumeCount)
			{
				static CCryNameR paramClipVolumeData("HMAO_ClipVolumeData");
				pSH->FXSetPSFloat(paramClipVolumeData, pClipVolumeParams, min((uint32)MAX_DEFERRED_CLIP_VOLUMES, nClipVolumeCount + VIS_AREAS_OUTDOOR_STENCIL_OFFSET));
				SD3DPostEffectsUtils::SetTexture(CDeferredShading::Instance().GetResolvedStencilRT(), 2, FILTER_POINT);
			}

			rd->D3DSetCull(eCULL_Back);
			rd->FX_SetState(0);

			Vec4 v(0, 0, tpSrc->GetWidth(), tpSrc->GetHeight());
			static CCryNameR Param1Name("PixelOffset");
			pSH->FXSetVSFloat(Param1Name, &v, 1);

			SD3DPostEffectsUtils::DrawFullScreenTri(CTexture::s_ptexHeightMapAO[1]->GetWidth(), CTexture::s_ptexHeightMapAO[1]->GetHeight());
			SD3DPostEffectsUtils::ShEndPass();

			rd->FX_PopRenderTarget(0);
		}

		pHeightMapAOScreenDepth = pDepth[resolutionIndex];
		pHeightmapAO = CTexture::s_ptexHeightMapAO[1];
	}
}

//////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

void CDeferredShading::DirectionalOcclusionPass()
{
	CD3D9Renderer* const __restrict rd = gcpRendD3D;
	if (!CRenderer::CV_r_ssdo)
	{
		gcpRendD3D->FX_ClearTarget(CTexture::s_ptexSceneNormalsBent, Clr_Median);
		return;
	}

	// calculate height map AO first
	ShadowMapFrustum* pHeightMapFrustum = NULL;
	CTexture* pHeightMapAODepth, * pHeightMapAO;
	HeightMapOcclusionPass(pHeightMapFrustum, pHeightMapAODepth, pHeightMapAO);

	rd->m_RP.m_FlagsShader_RT &= ~(g_HWSR_MaskBit[HWSR_SAMPLE0] | g_HWSR_MaskBit[HWSR_SAMPLE1] | g_HWSR_MaskBit[HWSR_SAMPLE2]);
	CTexture* pDstSSDO = CTexture::s_ptexStereoR;// re-using stereo buffers (only full resolution 32bit non-multisampled available at this step)
#if defined(DURANGO_USE_ESRAM)
	pDstSSDO = CTexture::s_ptexSceneSpecularESRAM;
#endif

	const bool bLowResOutput = (CRenderer::CV_r_ssdoHalfRes == 3);
	if (bLowResOutput)
		pDstSSDO = CTexture::s_ptexBackBufferScaled[0];

	rd->m_RP.m_FlagsShader_RT |= CRenderer::CV_r_ssdoHalfRes ? g_HWSR_MaskBit[HWSR_SAMPLE0] : 0;
	rd->m_RP.m_FlagsShader_RT |= pHeightMapFrustum ? g_HWSR_MaskBit[HWSR_SAMPLE1] : 0;

	// Extreme magnification as happening with small FOVs will cause banding issues with half-res depth
	if (CRenderer::CV_r_ssdoHalfRes == 2 && RAD2DEG(rd->GetCamera().GetFov()) < 30)
		rd->m_RP.m_FlagsShader_RT &= ~g_HWSR_MaskBit[HWSR_SAMPLE0];

	PROFILE_LABEL_PUSH("DIRECTIONAL_OCC");

	bool allowDepthBounds = !bLowResOutput;
	rd->FX_PushRenderTarget(0, pDstSSDO, allowDepthBounds ? &gcpRendD3D->m_DepthBufferOrig : NULL);

#ifdef SUPPORTS_MSAA
	if (rd->m_RP.m_MSAAData.Type)
	{
		rd->FX_ClearTarget(pDstSSDO, Clr_Median);
	}
#endif

	if (CRenderer::CV_r_DeferredShadingDepthBoundsTest)
		rd->SetDepthBoundTest(0.0f, DBT_SKY_CULL_DEPTH, allowDepthBounds);

	static CCryNameTSCRC tech("DirOccPass");
	SD3DPostEffectsUtils::ShBeginPass(m_pShader, tech, FEF_DONTSETSTATES);

	rd->FX_SetState(GS_NODEPTHTEST);
	m_pNormalsRT->Apply(0, m_nTexStatePoint);
	m_pDepthRT->Apply(1, m_nTexStatePoint);
	SPostEffectsUtils::SetTexture(CTexture::s_ptexAOVOJitter, 3, FILTER_POINT, 0);
	if (bLowResOutput)
		CTexture::s_ptexZTargetScaled2->Apply(5, m_nTexStatePoint);
	else
		CTexture::s_ptexZTargetScaled->Apply(5, m_nTexStatePoint);

	Matrix44A matView;
	matView = rd->m_RP.m_TI[rd->m_RP.m_nProcessThreadID].m_cam.GetViewMatrix();

	// Adjust the camera matrix so that the camera space will be: +y = down, +z - towards, +x - right
	Vec3 zAxis = matView.GetRow(1);
	matView.SetRow(1, -matView.GetRow(2));
	matView.SetRow(2, zAxis);
	float z = matView.m13;
	matView.m13 = -matView.m23;
	matView.m23 = z;

	float radius = CRenderer::CV_r_ssdoRadius / rd->GetRCamera().fFar;
#if defined(FEATURE_SVO_GI)
	if (CSvoRenderer::GetInstance()->IsActive())
		radius *= CSvoRenderer::GetInstance()->GetSsaoAmount();
#endif
	static CCryNameR paramName1("SSDOParams");
	Vec4 param1(radius * 0.5f * rd->m_ProjMatrix.m00, radius * 0.5f * rd->m_ProjMatrix.m11,
	            CRenderer::CV_r_ssdoRadiusMin, CRenderer::CV_r_ssdoRadiusMax);
	m_pShader->FXSetPSFloat(paramName1, &param1, 1);

	static CCryNameR viewspaceParamName("ViewSpaceParams");
	Vec4 vViewSpaceParam(2.0f / rd->m_ProjMatrix.m00, 2.0f / rd->m_ProjMatrix.m11, -1.0f / rd->m_ProjMatrix.m00, -1.0f / rd->m_ProjMatrix.m11);
	m_pShader->FXSetPSFloat(viewspaceParamName, &vViewSpaceParam, 1);

	static CCryNameR paramName2("SSDO_CameraMatrix");
	m_pShader->FXSetPSFloat(paramName2, (Vec4*)matView.GetData(), 3);

	matView.Invert();
	static CCryNameR paramName3("SSDO_CameraMatrixInv");
	m_pShader->FXSetPSFloat(paramName3, (Vec4*)matView.GetData(), 3);

	// set up height map AO
	if (pHeightMapFrustum)
	{
		pHeightMapAODepth->Apply(11, m_nTexStatePoint);
		pHeightMapAO->Apply(12, -2);

		static CCryNameR paramNameHMAO("HMAO_Params");
		Vec4 paramHMAO(CRenderer::CV_r_HeightMapAOAmount, 1.0f / pHeightMapFrustum->nTexSize, 0, 0);
		m_pShader->FXSetPSFloat(paramNameHMAO, &paramHMAO, 1);
	}

	SD3DPostEffectsUtils::DrawFullScreenTri(pDstSSDO->GetWidth(), pDstSSDO->GetHeight(), 0);//, &gcpRendD3D->m_FullResRect);
	SD3DPostEffectsUtils::ShEndPass();

	if (CRenderer::CV_r_DeferredShadingDepthBoundsTest)
		rd->SetDepthBoundTest(0.0f, DBT_SKY_CULL_DEPTH, false);

	rd->FX_PopRenderTarget(0);

	if (CRenderer::CV_r_ssdo != 99)
	{
		CShader* pSH = rd->m_cEF.s_ShaderShadowBlur;
		CTexture* tpSrc = pDstSSDO;
		const int32 nSizeX = CTexture::s_ptexZTarget->GetWidth();
		const int32 nSizeY = CTexture::s_ptexZTarget->GetHeight();
		const int32 nSrcSizeX = tpSrc->GetWidth();
		const int32 nSrcSizeY = tpSrc->GetHeight();

		PROFILE_LABEL_SCOPE("SSDO_BLUR");
		rd->FX_PushRenderTarget(0, CTexture::s_ptexSceneNormalsBent, NULL);

		static CCryNameTSCRC TechName("SSDO_Blur");
		SD3DPostEffectsUtils::ShBeginPass(pSH, TechName, FEF_DONTSETTEXTURES | FEF_DONTSETSTATES);

		tpSrc->Apply(0, m_nTexStateLinear);
		CTexture::s_ptexZTarget->Apply(1, m_nTexStatePoint);

		rd->D3DSetCull(eCULL_Back);
		rd->FX_SetState(0);

		Vec4 v(0, 0, nSrcSizeX, nSrcSizeY);
		static CCryNameR Param1Name("PixelOffset");
		pSH->FXSetVSFloat(Param1Name, &v, 1);

		v = Vec4(0.5f / (float)nSizeX, 0.5f / (float)nSizeY, 1.f / (float)nSrcSizeX, 1.f / (float)nSrcSizeY);
		static CCryNameR Param2Name("BlurOffset");
		pSH->FXSetPSFloat(Param2Name, &v, 1);

		v = Vec4(2.f / nSrcSizeX, 0, 2.f / nSrcSizeY, 10.f); //w = Weight coef
		static CCryNameR Param3Name("SSAO_BlurKernel");
		pSH->FXSetPSFloat(Param3Name, &v, 1);

		SD3DPostEffectsUtils::DrawFullScreenTri(CTexture::s_ptexSceneNormalsBent->GetWidth(), CTexture::s_ptexSceneNormalsBent->GetHeight());
		SD3DPostEffectsUtils::ShEndPass();

		rd->FX_PopRenderTarget(0);
	}
	else  // For debugging
	{
		PostProcessUtils().StretchRect(pDstSSDO, CTexture::s_ptexSceneNormalsBent);
	}

	rd->m_RP.m_FlagsShader_RT &= ~(g_HWSR_MaskBit[HWSR_SAMPLE0] | g_HWSR_MaskBit[HWSR_SAMPLE1]);

	if (CRenderer::CV_r_ssdoColorBleeding)
	{
		// Generate low frequency scene albedo for color bleeding (convolution not gamma correct but acceptable)
		PostProcessUtils().StretchRect(CTexture::s_ptexSceneDiffuse, CTexture::s_ptexBackBufferScaled[0], false, false, false, false);
		PostProcessUtils().StretchRect(CTexture::s_ptexBackBufferScaled[0], CTexture::s_ptexBackBufferScaled[1]);
		PostProcessUtils().StretchRect(CTexture::s_ptexBackBufferScaled[1], CTexture::s_ptexAOColorBleed);
		PostProcessUtils().TexBlurGaussian(CTexture::s_ptexAOColorBleed, 1, 1.0f, 4.0f, false, NULL, false, CTexture::s_ptexBackBufferScaled[2]);
	}

	PROFILE_LABEL_POP("DIRECTIONAL_OCC");
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void CDeferredShading::DeferredSubsurfaceScattering(CTexture* tmpTex)
{
	if (gcpRendD3D->m_nGraphicsPipeline > 0)
	{
		m_pLBufferDiffuseRT->Unbind();
		gcpRendD3D->GetGraphicsPipeline().RenderScreenSpaceSSS(tmpTex);
		return;
	}

	CD3D9Renderer* const __restrict rd = gcpRendD3D;

	if (!CTexture::s_ptexHDRTarget)  // Sketch mode disables HDR rendering
		return;

	PROFILE_LABEL_SCOPE("SSSSS");

	const uint64 nFlagsShaderRT = rd->m_RP.m_FlagsShader_RT;
	rd->m_RP.m_FlagsShader_RT &= ~(g_HWSR_MaskBit[HWSR_SAMPLE0]);

	static CCryNameTSCRC techBlur("SSSSS_Blur");
	static CCryNameR blurParamName("SSSBlurDir");
	static CCryNameR viewspaceParamName("ViewSpaceParams");
	Vec4 vViewSpaceParam(2.0f / rd->m_ProjMatrix.m00, 2.0f / rd->m_ProjMatrix.m11, -1.0f / rd->m_ProjMatrix.m00, -1.0f / rd->m_ProjMatrix.m11);
	Vec4 vBlurParam;

	float fProjScaleX = 0.5f * rd->m_ProjMatrix.m00;
	float fProjScaleY = 0.5f * rd->m_ProjMatrix.m11;

	m_pLBufferDiffuseRT->Unbind();
	m_pDepthRT->Apply(1, m_nTexStatePoint);
	m_pNormalsRT->Apply(2, m_nTexStatePoint);
	m_pDiffuseRT->Apply(3, m_nTexStatePoint);
	m_pSpecularRT->Apply(4, m_nTexStatePoint);

	rd->FX_SetState(GS_NODEPTHTEST);

	// Horizontal pass
	rd->FX_PushRenderTarget(0, CTexture::s_ptexSceneTargetR11G11B10F[1], NULL);
	tmpTex->Apply(0, m_nTexStatePoint);  // Irradiance
	SD3DPostEffectsUtils::ShBeginPass(m_pShader, techBlur, FEF_DONTSETTEXTURES | FEF_DONTSETSTATES);
	m_pShader->FXSetPSFloat(viewspaceParamName, &vViewSpaceParam, 1);
	vBlurParam(fProjScaleX, 0, 0, 0);
	m_pShader->FXSetPSFloat(blurParamName, &vBlurParam, 1);
	SD3DPostEffectsUtils::DrawFullScreenTri(CTexture::s_ptexHDRTarget->GetWidth(), CTexture::s_ptexHDRTarget->GetHeight());
	SD3DPostEffectsUtils::ShEndPass();
	rd->FX_PopRenderTarget(0);

	rd->FX_SetState(GS_NODEPTHTEST | GS_BLSRC_ONE | GS_BLDST_ONE);
	rd->m_RP.m_FlagsShader_RT |= g_HWSR_MaskBit[HWSR_SAMPLE0];

	// Vertical pass
	rd->FX_PushRenderTarget(0, CTexture::s_ptexHDRTarget, NULL);
	CTexture::s_ptexSceneTargetR11G11B10F[1]->Apply(0, m_nTexStatePoint);
	tmpTex->Apply(5, m_nTexStatePoint);  // Original irradiance
	SD3DPostEffectsUtils::ShBeginPass(m_pShader, techBlur, FEF_DONTSETTEXTURES | FEF_DONTSETSTATES);
	m_pShader->FXSetPSFloat(viewspaceParamName, &vViewSpaceParam, 1);
	vBlurParam(0, fProjScaleY, 0, 0);
	m_pShader->FXSetPSFloat(blurParamName, &vBlurParam, 1);
	SD3DPostEffectsUtils::DrawFullScreenTri(CTexture::s_ptexHDRTarget->GetWidth(), CTexture::s_ptexHDRTarget->GetHeight());
	SD3DPostEffectsUtils::ShEndPass();
	rd->FX_PopRenderTarget(0);

	rd->m_RP.m_FlagsShader_RT = nFlagsShaderRT;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void CDeferredShading::DeferredShadingPass()
{
	CD3D9Renderer* const __restrict rd = gcpRendD3D;

	if (rd->IsShadowPassEnabled())
	{
		PROFILE_LABEL_SCOPE("SHADOWMASK");
		rd->FX_DeferredShadowMaskGen(m_pCurrentRenderView, TArray<uint32>());
	}

	rd->D3DSetCull(eCULL_Back, true); //fs quads should not revert test..

	PROFILE_LABEL_PUSH("DEFERRED_SHADING");

	CTexture* tmpTexSSS = CTexture::s_ptexSceneTargetR11G11B10F[0];

	const uint64 nFlagsShaderRT = rd->m_RP.m_FlagsShader_RT;
	rd->m_RP.m_FlagsShader_RT &= ~(g_HWSR_MaskBit[HWSR_SAMPLE0] | g_HWSR_MaskBit[HWSR_SAMPLE1] | g_HWSR_MaskBit[HWSR_SAMPLE2] | g_HWSR_MaskBit[HWSR_SAMPLE4] | RT_CLIPVOLUME_ID);

	const int nResourceViewID = rd->m_RP.m_MSAAData.Type ? m_nBindResourceMsaa : -1;

	if (CRenderer::CV_r_DeferredShadingDepthBoundsTest)
		rd->SetDepthBoundTest(0.0f, DBT_SKY_CULL_DEPTH, true);

	// Deferred subsurface scattering
	bool deferredSSS = CRenderer::CV_r_DeferredShadingSSS && rd->FX_GetMSAAMode() == 0;  // Explicitly disabling deferred SSS has its incompatible with msaa in current stage

	if (deferredSSS)
		rd->FX_PushRenderTarget(1, tmpTexSSS, NULL);

	PROFILE_LABEL_PUSH("COMPOSITION");
	static CCryNameTSCRC techComposition("DeferredShadingPass");

#ifdef SUPPORTS_MSAA
	m_pNormalsRT->SetResolved(true); // normals was updated with shadows on alpha (would require another custom resolve to bind pre-resolved)
	rd->FX_MSAARestoreSampleMask();  // If stencil clear occurred, restore sample mask

	const int32 nNumPassesMSAA = (rd->FX_GetMSAAMode() == 1) ? 2 : 1;
	int32 nPrevStencilMaskRef = rd->m_nStencilMaskRef;
	rd->m_nStencilMaskRef = 0;

	for (int32 nPass = 0; nPass < nNumPassesMSAA; ++nPass)
	{
		if (nPass == 1)
		{
			PROFILE_LABEL_PUSH("SAMPLEFREQPASS");
			rd->FX_MSAASampleFreqStencilSetup(MSAA_SAMPLEFREQ_PASS);
		}
#endif

	if (deferredSSS)
		rd->m_RP.m_FlagsShader_RT |= g_HWSR_MaskBit[HWSR_SAMPLE1];

	if (CRenderer::CV_r_DeferredShadingAreaLights)
		rd->m_RP.m_FlagsShader_RT |= g_HWSR_MaskBit[HWSR_SAMPLE2];

	const uint32 nNumClipVolumes = RenderView()->GetClipVolumes().size();
	if (nNumClipVolumes)
		rd->m_RP.m_FlagsShader_RT |= RT_CLIPVOLUME_ID;

	// Enable sun permutation (eg: when fully inside vis areas and sun not visible/used, skip sun computation)
	if (rd->m_RP.m_pSunLight)
		rd->m_RP.m_FlagsShader_RT |= g_HWSR_MaskBit[HWSR_SAMPLE4];

	SD3DPostEffectsUtils::ShBeginPass(m_pShader, techComposition, FEF_DONTSETTEXTURES | FEF_DONTSETSTATES);

	rd->FX_SetState(GS_NODEPTHTEST);
#ifdef SUPPORTS_MSAA
	if (gcpRendD3D->m_RP.m_MSAAData.Type)
	{
		rd->m_RP.m_PersFlags2 |= RBPF2_READMASK_RESERVED_STENCIL_BIT;
		const int32 nMSAAStState = STENC_FUNC((rd->m_RP.m_PersFlags2 & RBPF2_MSAA_SAMPLEFREQ_PASS) ? FSS_STENCFUNC_EQUAL : FSS_STENCFUNC_NOTEQUAL) | STENCOP_FAIL(FSS_STENCOP_KEEP) | STENCOP_ZFAIL(FSS_STENCOP_KEEP) | STENCOP_PASS(FSS_STENCOP_KEEP);
		rd->FX_SetStencilState(nMSAAStState, BIT_STENCIL_RESERVED, BIT_STENCIL_RESERVED, 0xFF);
		rd->FX_SetState(rd->m_RP.m_CurState | GS_STENCIL);
	}
#endif

	m_pLBufferDiffuseRT->Apply(0, m_nTexStatePoint, EFTT_UNKNOWN, -1, nResourceViewID);
	m_pLBufferSpecularRT->Apply(1, m_nTexStatePoint, EFTT_UNKNOWN, -1, nResourceViewID);
	m_pDiffuseRT->Apply(2, m_nTexStatePoint, EFTT_UNKNOWN, -1, (rd->m_RP.m_PersFlags2 & RBPF2_MSAA_SAMPLEFREQ_PASS) ? nResourceViewID : -1);
	m_pSpecularRT->Apply(3, m_nTexStatePoint, EFTT_UNKNOWN, -1, (rd->m_RP.m_PersFlags2 & RBPF2_MSAA_SAMPLEFREQ_PASS) ? nResourceViewID : -1);
	m_pNormalsRT->Apply(4, m_nTexStatePoint, EFTT_UNKNOWN, -1, nResourceViewID);
	m_pDepthRT->Apply(5, m_nTexStatePoint, EFTT_UNKNOWN, -1, (rd->m_RP.m_PersFlags2 & RBPF2_MSAA_SAMPLEFREQ_PASS) ? nResourceViewID : -1);
	m_pResolvedStencilRT->Apply(6, m_nTexStatePoint, EFTT_UNKNOWN, -1, -1);

	// Directional occlusion
	Vec4 ssdoParams(CRenderer::CV_r_ssdoAmountDirect, CRenderer::CV_r_ssdoAmountAmbient, CRenderer::CV_r_ssdoAmountReflection, 0);
	Vec4 ssdoParamsNull(0, 0, 0, 0);
	static CCryNameR ssdoParamsName("SSDOParams");
	m_pShader->FXSetPSFloat(ssdoParamsName, CRenderer::CV_r_ssdo ? &ssdoParams : &ssdoParamsNull, 1);
	CTexture::s_ptexSceneNormalsBent->Apply(7, m_nTexStatePoint);   // todo: should this be msaaed or upscaled ?

	CTexture::s_ptexShadowMask->Apply(8, m_nTexStatePoint);
	m_pDepthRT->Apply(9, m_nTexStatePoint);

	Vec3 sunColor;
	gEnv->p3DEngine->GetGlobalParameter(E3DPARAM_SUN_COLOR, sunColor);

	// Apply LBuffers range rescale
	sunColor.x *= rd->m_fAdaptedSceneScaleLBuffer;
	sunColor.y *= rd->m_fAdaptedSceneScaleLBuffer;
	sunColor.z *= rd->m_fAdaptedSceneScaleLBuffer;

	static CCryNameR paramNameSunColor("SunColor");
	const Vec4 paramSunColor(sunColor, gEnv->p3DEngine->GetGlobalParameter(E3DPARAM_SUN_SPECULAR_MULTIPLIER));
	m_pShader->FXSetPSFloat(paramNameSunColor, &paramSunColor, 1);

	const Vec4 vParamsHDRRange(0.0f, 0.0f, 0.0f, rd->m_fAdaptedSceneScale / (rd->m_fAdaptedSceneScaleLBuffer + FLT_MIN));  // packed rescale
	m_pShader->FXSetPSFloat(m_pGeneralParams, &vParamsHDRRange, 1);
	if (nNumClipVolumes)
		m_pShader->FXSetPSFloat(m_pClipVolumeParams, m_vClipVolumeParams, min((uint32)MAX_DEFERRED_CLIP_VOLUMES, nNumClipVolumes + VIS_AREAS_OUTDOOR_STENCIL_OFFSET));

	const float SunSourceDiameter = 94.0f;    // atan(AngDiameterSun) * 2 * SunDistance, where AngDiameterSun=0.54deg and SunDistance=10000
	static CCryNameR arealightMatrixName("g_AreaLightMatrix");
	Matrix44 mAreaLightMatrix;
	mAreaLightMatrix.SetIdentity();
	mAreaLightMatrix.SetRow4(3, Vec4(SunSourceDiameter, SunSourceDiameter, 0, 1.0f));
	m_pShader->FXSetPSFloat(arealightMatrixName, alias_cast<Vec4*>(&mAreaLightMatrix), 4);

	SD3DPostEffectsUtils::DrawFullScreenTriWPOS(m_pLBufferDiffuseRT->GetWidth(), m_pLBufferDiffuseRT->GetHeight(), 0, &gcpRendD3D->m_FullResRect);
	SD3DPostEffectsUtils::ShEndPass();

#ifdef SUPPORTS_MSAA
	if (nPass == 1)
	{
		rd->FX_MSAASampleFreqStencilSetup();
		PROFILE_LABEL_POP("SAMPLEFREQPASS");
	}
}

if (gcpRendD3D->m_RP.m_MSAAData.Type)
	rd->m_RP.m_PersFlags2 &= ~RBPF2_READMASK_RESERVED_STENCIL_BIT;

rd->FX_StencilTestCurRef(false);
rd->m_nStencilMaskRef = nPrevStencilMaskRef;
#endif

	PROFILE_LABEL_POP("COMPOSITION");

	if (CRenderer::CV_r_DeferredShadingDepthBoundsTest)
		rd->SetDepthBoundTest(0.0f, DBT_SKY_CULL_DEPTH, false);

	if (deferredSSS)
	{
		rd->FX_PopRenderTarget(1);
		rd->FX_SetActiveRenderTargets();

		DeferredSubsurfaceScattering(tmpTexSSS);
	}

	rd->m_RP.m_FlagsShader_RT = nFlagsShaderRT;

	PROFILE_LABEL_POP("DEFERRED_SHADING");
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
bool CDeferredShading::GIPass()
{
	CD3D9Renderer* const __restrict rd = gcpRendD3D;

	bool bGIIsRendered = false;

	const bool bNeedStencilCull = !m_vecGIClipVolumes[m_nThreadID][m_nRecurseLevel].empty();

	// apply GI
	if (LPVManager.IsGIRenderable() && CRenderer::CV_r_LightPropagationVolumes)
	{
		PROFILE_LABEL_SCOPE("GI");

		if (CRenderer::CV_r_DeferredShadingScissor)
			rd->EF_Scissor(false, 0, 0, 0, 0);
		if (CRenderer::CV_r_DeferredShadingDepthBoundsTest)
			rd->SetDepthBoundTest(0.0f, 1.0f, false); // stencil pre-passes are rop bound, using depth bounds increases even more rop cost

		int nStoredStencilRef = rd->m_nStencilMaskRef;
		// refresh stencil based on visareas and clip volumes
		if (bNeedStencilCull)
		{
			TArray<SClipShape>& vecGIVolumes = m_vecGIClipVolumes[m_nThreadID][m_nRecurseLevel];
			const size_t nVolumes = min((size_t)vecGIVolumes.size(), size_t(STENC_MAX_REF >> 1));

			// reset stencil to value equal to number of volumes as we're going to incr/decr
			const uint8 nRefMaskBase = static_cast<uint8>(nVolumes + 1);  //+ 1, we start at 1. 0 is reserved for MSAAed areas.
			rd->EF_ClearTargetsImmediately(FRT_CLEAR_STENCIL, Clr_Unused.r, nRefMaskBase);
			rd->m_nStencilMaskRef = nRefMaskBase;

			// Render Clip Volumes to cull GI in indoors
			for (size_t iVolume = 0; iVolume < nVolumes; ++iVolume)
			{
				SClipShape& rClipShape = vecGIVolumes[iVolume];

				CRenderMesh* pRenderMesh = (CRenderMesh*)rClipShape.pShape;
				if (!pRenderMesh)
					continue;

				// TODO: add visibility culling
				//if(pRenderMesh->GetDistance(rCamera) > LPVManager.GetDistance)
				//	continue;

				pRenderMesh->CheckUpdate(pRenderMesh->_GetVertexFormat(), 0);
				size_t nOffsV = 0;
				size_t nOffsI = 0;
				D3DVertexBuffer* pCheckArbitraryShapeVB = (D3DVertexBuffer*)pRenderMesh->_GetD3DVB(VSF_GENERAL, &nOffsV);
				D3DIndexBuffer* pCheckArbitraryShapeIB = (D3DIndexBuffer*)pRenderMesh->_GetD3DIB(&nOffsI);
				if (!pCheckArbitraryShapeVB || !pCheckArbitraryShapeIB)
					continue;

				rd->m_RP.m_TI[rd->m_RP.m_nProcessThreadID].m_matView->Push();
				Matrix44 mLocalTransposed;
				mLocalTransposed = rClipShape.mxTransform.GetTransposed();

				rd->m_RP.m_TI[rd->m_RP.m_nProcessThreadID].m_matView->MultMatrixLocal(&mLocalTransposed);

				const int arbitraryShapeVBSize = pRenderMesh->_GetNumVerts();
				const int arbitraryShapeIBSize = pRenderMesh->_GetNumInds();
				const int vdescsize = pRenderMesh->GetStreamStride(VSF_GENERAL);
				const EVertexFormat eVF = pRenderMesh->_GetVertexFormat();

				uint32 nPasses = 0;
				CShader* pSH(CShaderMan::s_ShaderShadowMaskGen);
				static CCryNameTSCRC StencilCullTechName = "DeferredShadowPass";
				pSH->FXSetTechnique(StencilCullTechName);
				pSH->FXBegin(&nPasses, 0);

				pSH->FXBeginPass(CD3D9Renderer::DS_SHADOW_CULL_PASS);

				if (!FAILED(rd->FX_SetVertexDeclaration(0, eVF)))
				{
					rd->FX_SetVStream(0, pCheckArbitraryShapeVB, nOffsV, vdescsize);
					rd->FX_SetIStream(pCheckArbitraryShapeIB, nOffsI, (sizeof(vtx_idx) == 2 ? Index16 : Index32));

					rd->D3DSetCull(eCULL_None);

					rd->FX_SetStencilState(
					  FSS_STENCIL_TWOSIDED | STENC_FUNC(FSS_STENCFUNC_ALWAYS) | STENC_CCW_FUNC(FSS_STENCFUNC_ALWAYS) |
					  STENCOP_FAIL(FSS_STENCOP_KEEP) | STENCOP_CCW_FAIL(FSS_STENCOP_KEEP) |
					  STENCOP_ZFAIL(FSS_STENCOP_INCR) | STENCOP_CCW_ZFAIL(FSS_STENCOP_DECR) |
					  STENCOP_PASS(FSS_STENCOP_KEEP) | STENCOP_CCW_PASS(FSS_STENCOP_KEEP),
					  nRefMaskBase, 0xFF, 0xFF);

					//Disable color writes
					int newState = rd->m_RP.m_CurState;
					newState |= GS_COLMASK_NONE;

					//setup depth test
					newState &= ~GS_NODEPTHTEST;
					newState &= ~GS_DEPTHWRITE;
					newState &= ~GS_DEPTHFUNC_MASK;
					//setup stencil
					newState |= GS_DEPTHFUNC_LEQUAL;
					newState |= GS_STENCIL;

					rd->FX_SetState(newState);

					rd->FX_Commit();

					rd->FX_DrawIndexedPrimitive(eptTriangleList, 0, 0, arbitraryShapeVBSize, 0, arbitraryShapeIBSize);

					++rd->m_nStencilMaskRef;
				}
				pSH->FXEndPass();
				pSH->FXEnd();
				rd->m_RP.m_TI[rd->m_RP.m_nProcessThreadID].m_matView->Pop();
			}

			// set up zero reference
			nStoredStencilRef = rd->m_nStencilMaskRef + 1; //max value currently in stencil + 1
			rd->m_nStencilMaskRef = nRefMaskBase;
			rd->FX_StencilTestCurRef(true, false);
		}
		else
			rd->SetState(rd->m_RP.m_CurState & ~GS_STENCIL);

		// Main rendering of GI
		LPVManager.RenderGI();
		bGIIsRendered = true;

		if (bNeedStencilCull)
		{
			rd->m_nStencilMaskRef = nStoredStencilRef;
			rd->FX_StencilTestCurRef(false);
		}

		rd->SetDepthBoundTest(0.f, 1.f, false);
	}

	return bGIIsRendered;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void CDeferredShading::DeferredLights(RenderLightsArray& rLights, bool bCastShadows)
{
	if (rLights.size())
	{
		PROFILE_LABEL_SCOPE("DEFERRED_LIGHTS");

		if (bCastShadows)
		{
			PackAllShadowFrustums(false);
		}

		for (uint32 nCurrentLight = 0; nCurrentLight < rLights.size(); ++nCurrentLight)
		{
			const SRenderLight& pDL = rLights[nCurrentLight];
			if (pDL.m_Flags & (DLF_FAKE | DLF_VOLUMETRIC_FOG_ONLY))
				continue;

			assert(pDL.GetSpecularCubemap() == NULL);
			if (!(pDL.m_Flags & DLF_CASTSHADOW_MAPS))
			{
				LightPass(&pDL);
			}

			m_nLightsProcessedCount++;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

bool CDeferredShading::PackAllShadowFrustums(bool bPreLoop, bool bRenderShadowMaps)
{
	CD3D9Renderer* const __restrict rd = gcpRendD3D;

	CRY_ASSERT(rd->m_pRT->IsRenderThread());

	const uint64 nPrevFlagsShaderRT = rd->m_RP.m_FlagsShader_RT;

	static ICVar* p_e_ShadowsPoolSize = iConsole->GetCVar("e_ShadowsPoolSize");

	RenderLightsArray& arrLights = m_pCurrentRenderView->GetLightsArray(eDLT_DeferredLight);

#ifndef _RELEASE
	rd->m_RP.m_PS[m_nThreadID].m_NumShadowPoolFrustums = 0;
	rd->m_RP.m_PS[m_nThreadID].m_NumShadowPoolAllocsThisFrame = 0;
	rd->m_RP.m_PS[m_nThreadID].m_NumShadowMaskChannels = 0;
#endif

	int nRequestedPoolSize = p_e_ShadowsPoolSize->GetIVal();
	if (m_nShadowPoolSize != nRequestedPoolSize)
	{
		m_blockPack.UpdateSize(nRequestedPoolSize >> TEX_POOL_BLOCKLOGSIZE, nRequestedPoolSize >> TEX_POOL_BLOCKLOGSIZE);
		m_nShadowPoolSize = nRequestedPoolSize;

		// clear pool and reset allocations
		m_blockPack.Clear();
		m_shadowPoolAlloc.SetUse(0);
	}

	if (bPreLoop)
	{
		m_bClearPool = (CRenderer::CV_r_ShadowPoolMaxFrames > 0) ? false : true;

		m_nCurrentShadowPoolLight = 0;
		m_nFirstCandidateShadowPoolLight = 0;
	}

	bool bShadowRendered = false;

	// light pass here
	if (!bPreLoop)
	{
		for (uint nLightPacked = m_nFirstCandidateShadowPoolLight; nLightPacked < m_nCurrentShadowPoolLight; ++nLightPacked)
		{
			const SRenderLight& rLight = arrLights[nLightPacked];
			if (rLight.m_Flags & DLF_FAKE)
				continue;
			ShadowLightPasses(rLight, nLightPacked);
		}
	}

	// TODO: Sort lights so allocated ones are rendered first to reduce impact of a thrash

	while (m_nCurrentShadowPoolLight < arrLights.size()) //pre-loop to avoid 0.5 ms restore/resolve
	{
		SRenderLight& rLight = arrLights[m_nCurrentShadowPoolLight];
		if (!(rLight.m_Flags & DLF_DIRECTIONAL) && (rLight.m_Flags & DLF_CASTSHADOW_MAPS))
		{
			break;
		}
		m_nCurrentShadowPoolLight++;
	}

	if (bPreLoop && m_nCurrentShadowPoolLight < arrLights.size()) // Shadow allocation tick, free old shadows.
	{
		uint32 nAllocs = m_shadowPoolAlloc.Num();
		for (uint32 i = 0; i < nAllocs; i++)
		{
			SShadowAllocData* pAlloc = &m_shadowPoolAlloc[i];
			uint32 currFrame = gcpRendD3D->GetFrameID(false) & 0xFF;
			if (!pAlloc->isFree() && ((currFrame - pAlloc->m_frameID) > (uint)CRenderer::CV_r_ShadowPoolMaxFrames))
			{
				m_blockPack.RemoveBlock(pAlloc->m_blockID);
				pAlloc->Clear();
				break;  //Max one delete per frame, this should spread updates across more frames
			}
		}
	}

	while (m_nCurrentShadowPoolLight < arrLights.size() && (!bPreLoop || !bShadowRendered))
	{
		m_nFirstCandidateShadowPoolLight = m_nCurrentShadowPoolLight;

		// init before shadowgen
		SetupPasses();
		rd->FX_ResetPipe();
		rd->EF_Scissor(false, 0, 0, 0, 0);
		rd->SetDepthBoundTest(0.0f, 1.0f, false);

		{
			if (bRenderShadowMaps)
			{
				PROFILE_LABEL_PUSH("SHADOWMAP_POOL");
			}

			if (!bPreLoop)
				ResolveCurrentBuffers();

			while (m_nCurrentShadowPoolLight < arrLights.size())
			{
				SRenderLight& rLight = arrLights[m_nCurrentShadowPoolLight];

				if (!(rLight.m_Flags & (DLF_DIRECTIONAL | DLF_FAKE)) && (rLight.m_Flags & DLF_CASTSHADOW_MAPS))
				{

					bool bPacked = PackToPool(&m_blockPack, rLight, m_nCurrentShadowPoolLight, m_nFirstCandidateShadowPoolLight, m_bClearPool, bRenderShadowMaps);
					m_bClearPool = !bPacked;
					if (!bPacked)
					{
						break;
					}
				}
				++m_nCurrentShadowPoolLight;
				bShadowRendered = true;
			}

#ifndef _RELEASE
			{
				uint32 nAllocs = m_shadowPoolAlloc.Num();
				for (uint32 i = 0; i < nAllocs; i++)
				{
					if (!m_shadowPoolAlloc[i].isFree())
					{
						rd->m_RP.m_PS[rd->m_RP.m_nProcessThreadID].m_NumShadowPoolFrustums++;
					}
				}
			}
#endif

			if (bRenderShadowMaps)
			{
				PROFILE_LABEL_POP("SHADOWMAP_POOL");
			}
		}

		if (!bPreLoop && bShadowRendered)
		{
			RestoreCurrentBuffers();

			//insert light pass here
			for (uint nLightPacked = m_nFirstCandidateShadowPoolLight; nLightPacked < m_nCurrentShadowPoolLight; ++nLightPacked)
			{
				const SRenderLight& rLight = arrLights[nLightPacked];
				if (rLight.m_Flags & DLF_FAKE)
					continue;

				ShadowLightPasses(rLight, nLightPacked);
			}
		}
	}

	rd->m_RP.m_FlagsShader_RT = nPrevFlagsShaderRT;

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

bool CDeferredShading::PackToPool(CPowerOf2BlockPacker* pBlockPack, SRenderLight& light, const int nLightID, const int nFirstCandidateLight, bool bClearPool, bool bRenderShadowMaps)
{
	CD3D9Renderer* const __restrict rd = gcpRendD3D;

	CRenderView* pRenderView = RenderView();

	//////////////////////////////////////////////////////////////////////////
	int nDLights = RenderView()->GetDynamicLightsCount();

	//////////////////////////////////////////////////////////////////////////
	int nFrustumIdx = nLightID + nDLights;
	auto& SMFrustums = pRenderView->GetShadowFrustumsForLight(nFrustumIdx);

	int nUpdatedThisFrame = 0;

	assert((unsigned int) nFrustumIdx < (MAX_REND_LIGHTS + MAX_DEFERRED_LIGHTS));
	if ((unsigned int) nFrustumIdx >= (MAX_REND_LIGHTS + MAX_DEFERRED_LIGHTS))
	{
		int nFrameID = gcpRendD3D->GetFrameID(false);
		if (m_nWarningFrame != nFrameID)
		{
			Warning("CDeferredShading::ShadowLightPasses: Too many light sources used ...");
			m_nWarningFrame = nFrameID;
		}
		//reset castshadow flag for further processing
		light.m_Flags &= ~DLF_CASTSHADOW_MAPS;
		return true;
	}
	//////////////////////////////////////////////////////////////////////////

	//no single frustum was allocated for this light
	if (SMFrustums.empty())
	{
		//reset castshadow flag for further processing
		light.m_Flags &= ~DLF_CASTSHADOW_MAPS;
		return true;
	}

	if (pRenderView->IsRecursive())
	{
		//reset castshadow flag for further processing
		light.m_Flags &= ~DLF_CASTSHADOW_MAPS;
		return true;
	}

	ShadowMapFrustum& firstFrustum = *SMFrustums.front()->pFrustum;

	//////////////////////////////////////////////////////////////////////////
	int nBlockW = firstFrustum.nTexSize >> TEX_POOL_BLOCKLOGSIZE;
	int nBlockH = firstFrustum.nTexSize >> TEX_POOL_BLOCKLOGSIZE;
	int nLogBlockW = IntegerLog2((uint32)nBlockW);
	int nLogBlockH = nLogBlockW;

	bool bNeedsUpdate = false;

	if (bClearPool)
	{
		pBlockPack->Clear();
		m_shadowPoolAlloc.SetUse(0);
	}

	uint32 currFrame = gcpRendD3D->GetFrameID(false) & 0xFF;

	uint32 lightID = light.m_nEntityId;

	assert(lightID != (uint32) - 1);

	uint SidesNum = firstFrustum.bUnwrapedOmniDirectional ? OMNI_SIDES_NUM : 1;
	uint nUpdateMask = firstFrustum.bUnwrapedOmniDirectional ? 0x3F : 0x1;

	for (uint nSide = 0; nSide < SidesNum; nSide++)
	{
		bNeedsUpdate = false;
		uint nX1, nX2, nY1, nY2;

		//Find block allocation info (alternative: store in frustum data, but this does not persist)
		bool bFoundAlloc = false;
#if CRY_PLATFORM_WINDOWS
		int nMGPUUpdate = -1;
#endif
		uint32 nAllocs = m_shadowPoolAlloc.Num();
		SShadowAllocData* pAlloc = NULL;
		for (uint32 i = 0; i < nAllocs; i++)
		{
			pAlloc = &m_shadowPoolAlloc[i];

			if (pAlloc->m_lightID == lightID && pAlloc->m_side == nSide)
			{
				bFoundAlloc = true;
				break;
			}
		}

		if (bFoundAlloc)
		{
			uint32 nID = pBlockPack->GetBlockInfo(pAlloc->m_blockID, nX1, nY1, nX2, nY2);

			int nFrameCompare = (currFrame - pAlloc->m_frameID) % 256;

			if (nID == 0xFFFFFFFF)
			{
				bNeedsUpdate = true;
			}
			else
			{
				if (firstFrustum.nShadowPoolUpdateRate == 0)  // forced update, always do this
				{
					bNeedsUpdate = true;
				}
				else if (firstFrustum.nShadowPoolUpdateRate < (uint8) nFrameCompare)
				{
					if (nUpdatedThisFrame < CRenderer::CV_r_ShadowPoolMaxTimeslicedUpdatesPerFrame)
					{
						bNeedsUpdate = true;
						nUpdatedThisFrame++;
					}
				}
#if CRY_PLATFORM_WINDOWS // AFR support
				else if (rd->GetActiveGPUCount() > 1)
				{
					if (gRenDev->GetActiveGPUCount() > nFrameCompare)
					{
						bNeedsUpdate = true;
						nMGPUUpdate = pAlloc->m_frameID;
					}
				}
#endif
			}

			if (!bNeedsUpdate)
			{
				if (nX1 != 0xFFFFFFFF && nBlockW == (nX2 - nX1))   // ignore Y, is square
				{
					pBlockPack->GetBlockInfo(nID, nX1, nY1, nX2, nY2);
					firstFrustum.packX[nSide] = nX1 << TEX_POOL_BLOCKLOGSIZE;
					firstFrustum.packY[nSide] = nY1 << TEX_POOL_BLOCKLOGSIZE;
					firstFrustum.packWidth[nSide] = (nX2 - nX1) << TEX_POOL_BLOCKLOGSIZE;
					firstFrustum.packHeight[nSide] = (nY2 - nY1) << TEX_POOL_BLOCKLOGSIZE;
					firstFrustum.nShadowGenID[m_nThreadID][nSide] = 0xFFFFFFFF; // turn off shadow gen for this side

					nUpdateMask &= ~(1 << nSide);
					continue; // All currently valid, skip
				}
			}

			if (nID != 0xFFFFFFFF && nX1 != 0xFFFFFFFF) // Valid block, realloc
			{
				pBlockPack->RemoveBlock(nID);
				pAlloc->Clear();
			}
		}

		uint32 nID = pBlockPack->AddBlock(nLogBlockW, nLogBlockH);
		bool isAllocated = (nID != 0xFFFFFFFF) ? true : false;

#ifndef _RELEASE
		rd->m_RP.m_PS[rd->m_RP.m_nProcessThreadID].m_NumShadowPoolAllocsThisFrame++;
#endif

		if (isAllocated)
		{
			bNeedsUpdate = true;

			if (!bFoundAlloc)
			{
				pAlloc = NULL;
				nAllocs = m_shadowPoolAlloc.Num();
				for (uint32 i = 0; i < nAllocs; i++)
				{
					if (m_shadowPoolAlloc[i].isFree())
					{
						pAlloc = &m_shadowPoolAlloc[i];
						break;
					}
				}

				if (!pAlloc)
					pAlloc = m_shadowPoolAlloc.AddIndex(1);
			}

			pAlloc->m_blockID = nID;
			pAlloc->m_lightID = lightID;
			pAlloc->m_side = nSide;
#if CRY_PLATFORM_WINDOWS
			pAlloc->m_frameID = (nMGPUUpdate == -1) ? gcpRendD3D->GetFrameID(false) & 0xFF : nMGPUUpdate;
#else
			pAlloc->m_frameID = gcpRendD3D->GetFrameID(false) & 0xFF;
#endif
			bClearPool = true;
		}
		else
		{

#ifndef _RELEASE // failed alloc, will thrash!
			if (CRenderer::CV_r_ShadowPoolMaxFrames != 0 || CRenderer::CV_r_DeferredShadingTiled > 1)
				rd->m_RP.m_PS[rd->m_RP.m_nProcessThreadID].m_NumShadowPoolAllocsThisFrame |= 0x80000000;
#endif

			return false;
		}

		pBlockPack->GetBlockInfo(nID, nX1, nY1, nX2, nY2);
		firstFrustum.packX[nSide] = nX1 << TEX_POOL_BLOCKLOGSIZE;
		firstFrustum.packY[nSide] = nY1 << TEX_POOL_BLOCKLOGSIZE;
		firstFrustum.packWidth[nSide] = (nX2 - nX1) << TEX_POOL_BLOCKLOGSIZE;
		firstFrustum.packHeight[nSide] = (nY2 - nY1) << TEX_POOL_BLOCKLOGSIZE;

	}

	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//  next step is to use these values in shadowgen

	//////////////////////////////////////////////////////////////////////////

	if (bRenderShadowMaps && firstFrustum.bUseShadowsPool && nUpdateMask > 0)
	{
		rd->FX_PrepareDepthMapsForLight(RenderView(), light, nFrustumIdx, bClearPool);
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void CDeferredShading::ResolveCurrentBuffers()
{
	CD3D9Renderer* const __restrict rd = gcpRendD3D;

	PROFILE_LABEL_SCOPE("FLUSH_RESOLVE");
	rd->FX_PopRenderTarget(1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void CDeferredShading::RestoreCurrentBuffers()
{
	CD3D9Renderer* const __restrict rd = gcpRendD3D;

	rd->FX_PushRenderTarget(1, m_pLBufferSpecularRT, NULL, -1, false, 1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

bool CDeferredShading::ShadowLightPasses(const SRenderLight& light, const int nLightID)
{
	CD3D9Renderer* const __restrict rd = gcpRendD3D;

	CRenderView* pRenderView = RenderView();

	float scissorInt2Float[] = { (float)(light.m_sX), (float)(light.m_sY), (float)(light.m_sWidth), (float)(light.m_sHeight) };

	rd->m_RP.m_nDeferredPrimitiveID = SHAPE_PROJECTOR;
	int nDLights = RenderView()->GetDynamicLightsCount();

	//////////////////////////////////////////////////////////////////////////
	int nFrustumIdx = nLightID + nDLights;
	auto& SMFrustums = pRenderView->GetShadowFrustumsForLight(nFrustumIdx);

	assert((unsigned int) nFrustumIdx < (MAX_REND_LIGHTS + MAX_DEFERRED_LIGHTS));
	if ((unsigned int) nFrustumIdx >= (MAX_REND_LIGHTS + MAX_DEFERRED_LIGHTS))
	{
		//Warning("CDeferredShading::ShadowLightPasses: Too many light sources used ..."); // redundant
		return false;
	}

	//no single frustum was allocated for this light
	if (SMFrustums.empty())
		return false;

	if (pRenderView->IsRecursive())
		return false;

	// Area lights are a non-uniform box, not a cone in 1 of 6 directions, so we skip clipping/stencil testing and let the light pass take care of it.
	bool bAreaLight = (light.m_Flags & DLF_AREA_LIGHT) && light.m_fAreaWidth && light.m_fAreaHeight && light.m_fLightFrustumAngle && CRenderer::CV_r_DeferredShadingAreaLights;

	ShadowMapFrustum& firstFrustum = *SMFrustums.front()->pFrustum;

	int nSides = 1;
	if (firstFrustum.bOmniDirectionalShadow && !bAreaLight)
		nSides = 6;

	//enable shadow mapping
	rd->m_RP.m_FlagsShader_RT |= g_HWSR_MaskBit[HWSR_SAMPLE4];

	//enable hw-pcf per frustum
	if (firstFrustum.bHWPCFCompare)
	{
		rd->m_RP.m_FlagsShader_RT |= g_HWSR_MaskBit[HWSR_HW_PCF_COMPARE];
	}

	Vec4 pLightRect = Vec4(scissorInt2Float[0], scissorInt2Float[1], scissorInt2Float[2], scissorInt2Float[3]);
	Vec4 scaledLightRect = Vec4(pLightRect.x * rd->m_RP.m_CurDownscaleFactor.x, pLightRect.y * rd->m_RP.m_CurDownscaleFactor.y,
	                            pLightRect.z * rd->m_RP.m_CurDownscaleFactor.x, pLightRect.w * rd->m_RP.m_CurDownscaleFactor.y);

	if (!bAreaLight)
	{
		rd->m_nStencilMaskRef += (nSides + 2);
		if (rd->m_nStencilMaskRef > STENC_MAX_REF)
		{
			rd->EF_ClearTargetsImmediately(FRT_CLEAR_STENCIL);
			rd->m_nStencilMaskRef = nSides + 1;
		}
	}

	uint16 scaledX = 0, scaledY = 0, scaledWidth = 0, scaledHeight = 0;

	for (int nS = 0; nS < nSides; nS++)
	{
		uint32 nPersFlagsPrev = rd->m_RP.m_TI[m_nThreadID].m_PersFlags;
		rd->m_RP.m_TI[m_nThreadID].m_PersFlags |= RBPF_MIRRORCULL;

		SpecularAccEnableMRT(false);

		if (CRenderer::CV_r_DeferredShadingDepthBoundsTest == 1 && !bAreaLight)
		{
			Vec4 pDepthBounds = GetLightDepthBounds(&light, (rd->m_RP.m_TI[m_nThreadID].m_PersFlags & RBPF_REVERSE_DEPTH) != 0);
			rd->SetDepthBoundTest(pDepthBounds.x, pDepthBounds.z, true);
		}

		if (nS == 0)
		{
			scaledX = (uint16)(scaledLightRect.x);
			scaledY = (uint16)(scaledLightRect.y);
			scaledWidth = (uint16)(scaledLightRect.z) + 1;
			scaledHeight = (uint16)(scaledLightRect.w) + 1;
		}

		if (!bAreaLight)
		{
			//use current WorldProj matrix
			rd->FX_StencilFrustumCull(-2, &light, &firstFrustum, nS);

			rd->SetDepthBoundTest(0, 1, false);
		}

		rd->m_RP.m_TI[m_nThreadID].m_PersFlags = nPersFlagsPrev;
		if (!bAreaLight)
		{
			rd->FX_StencilTestCurRef(true, true);
		}

		SetupPasses();

		if (!bAreaLight)
			m_nRenderState |= GS_STENCIL;

		SpecularAccEnableMRT(true);

		if (firstFrustum.nShadowGenMask & (1 << nS))
		{
			rd->m_RP.m_FlagsShader_RT |= g_HWSR_MaskBit[HWSR_SAMPLE4];
			rd->ConfigShadowTexgen(0, &firstFrustum, nS);

			if (firstFrustum.bUseShadowsPool)
			{
				const int nTexFilter = firstFrustum.bHWPCFCompare ? FILTER_LINEAR : FILTER_POINT;
				STexState TS;
				TS.SetFilterMode(nTexFilter);
				TS.SetClampMode(TADDR_CLAMP, TADDR_CLAMP, TADDR_CLAMP);
				TS.m_bSRGBLookup = false;
				TS.SetComparisonFilter(true);
				CTexture::s_ptexRT_ShadowPool->Apply(3, CTexture::GetTexState(TS));
			}
			else
			{
				SD3DPostEffectsUtils::SetTexture(firstFrustum.pDepthTex, 3, FILTER_POINT, 0);
			}

			SD3DPostEffectsUtils::SetTexture(CTexture::s_ptexShadowJitterMap, 7, FILTER_POINT, 0);
		}
		else
		{
			rd->m_RP.m_FlagsShader_RT &= ~g_HWSR_MaskBit[HWSR_SAMPLE4];
		}

		//////////////////////////////////////////////////////////////////////////

		int nShadowQuality = rd->FX_ApplyShadowQuality();

		m_nCurLighID = nLightID;

		LightPass(&light, true);

		if (!bAreaLight)
			rd->FX_StencilTestCurRef(false);
	}

	//assign range
	if (!bAreaLight)
		rd->m_nStencilMaskRef += nSides;
	//////////////////////////////////////////////////////////////////////////
	rd->m_RP.m_FlagsShader_RT &= ~(g_HWSR_MaskBit[HWSR_SAMPLE4] | g_HWSR_MaskBit[HWSR_HW_PCF_COMPARE]);

	if (!bAreaLight)
		m_nRenderState &= ~GS_STENCIL;
	rd->FX_SetState(m_nRenderState);

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void CDeferredShading::CreateDeferredMaps()
{
	static uint32 nPrevLBuffersFmt = CRenderer::CV_r_DeferredShadingLBuffersFmt;
	if (!CTexture::s_ptexSceneNormalsMap || CTexture::s_ptexSceneNormalsMap->IsMSAAChanged()
	    || CTexture::s_ptexSceneNormalsMap->GetWidth() != gcpRendD3D->m_MainViewport.nWidth
	    || CTexture::s_ptexSceneNormalsMap->GetHeight() != gcpRendD3D->m_MainViewport.nHeight
	    || nPrevLBuffersFmt != CRenderer::CV_r_DeferredShadingLBuffersFmt)
	{
		nPrevLBuffersFmt = CRenderer::CV_r_DeferredShadingLBuffersFmt;

		int nWidth = gcpRendD3D->GetWidth();
		int nHeight = gcpRendD3D->GetHeight();
		int32 nMSAAUsageFlag = ((CRenderer::CV_r_msaa ? FT_USAGE_MSAA : 0));

		SD3DPostEffectsUtils::CreateRenderTarget("$SceneNormalsMap", CTexture::s_ptexSceneNormalsMap, nWidth, nHeight, Clr_Unknown, true, false, eTF_R8G8B8A8, TO_SCENE_NORMALMAP, nMSAAUsageFlag | FT_USAGE_ALLOWREADSRGB);
		SD3DPostEffectsUtils::CreateRenderTarget("$SceneNormalsBent", CTexture::s_ptexSceneNormalsBent, nWidth, nHeight, Clr_Median, true, false, eTF_R8G8B8A8);
		SD3DPostEffectsUtils::CreateRenderTarget("$AOColorBleed", CTexture::s_ptexAOColorBleed, nWidth >> 3, nHeight >> 3, Clr_Unknown, true, false, eTF_R8G8B8A8);

		ETEX_Format fmtZScaled = eTF_R16G16F;
		ETEX_Format nTexFormat = eTF_R16G16B16A16F;
#if CRY_PLATFORM_WINDOWS || CRY_PLATFORM_APPLE || CRY_PLATFORM_LINUX || CRY_PLATFORM_ANDROID
		if (CRenderer::CV_r_DeferredShadingLBuffersFmt == 1)
			nTexFormat = eTF_R11G11B10F;
#endif

		int nFTFlags = nMSAAUsageFlag;

		SD3DPostEffectsUtils::CreateRenderTarget("$SceneDiffuseAcc", CTexture::s_ptexSceneDiffuseAccMap, nWidth, nHeight, Clr_Transparent, true, false, nTexFormat, TO_SCENE_DIFFUSE_ACC, nFTFlags);
		CTexture::s_ptexCurrentSceneDiffuseAccMap = CTexture::s_ptexSceneDiffuseAccMap;
		SD3DPostEffectsUtils::CreateRenderTarget("$SceneSpecularAcc", CTexture::s_ptexSceneSpecularAccMap, nWidth, nHeight, Clr_Transparent, true, false, nTexFormat, TO_SCENE_SPECULAR_ACC, nFTFlags);

		SD3DPostEffectsUtils::CreateRenderTarget("$SceneDiffuse", CTexture::s_ptexSceneDiffuse, nWidth, nHeight, Clr_Empty, true, false, eTF_R8G8B8A8, -1, nFTFlags);
		SD3DPostEffectsUtils::CreateRenderTarget("$SceneSpecular", CTexture::s_ptexSceneSpecular, nWidth, nHeight, Clr_Empty, true, false, eTF_R8G8B8A8, -1, nFTFlags | FT_USAGE_ALLOWREADSRGB);
#if defined(DURANGO_USE_ESRAM)
		SD3DPostEffectsUtils::CreateRenderTarget("$SceneSpecularESRAM", CTexture::s_ptexSceneSpecularESRAM, nWidth, nHeight, Clr_Empty, true, false, eTF_R8G8B8A8, -1, nFTFlags);
#endif

		SD3DPostEffectsUtils::CreateRenderTarget("$VelocityObjects", CTexture::s_ptexVelocityObjects[0], nWidth, nHeight, Clr_Transparent, true, false, eTF_R16G16F, -1, nFTFlags);
		if (gRenDev->m_bDualStereoSupport)
		{
			SD3DPostEffectsUtils::CreateRenderTarget("$VelocityObject_R", CTexture::s_ptexVelocityObjects[1], nWidth, nHeight, Clr_Transparent, true, false, eTF_R16G16F, -1, nFTFlags);
		}

		SD3DPostEffectsUtils::CreateRenderTarget("$ZTargetScaled", CTexture::s_ptexZTargetScaled, nWidth >> 1, nHeight >> 1, Clr_FarPlane, 1, 0, fmtZScaled, TO_DOWNSCALED_ZTARGET_FOR_AO);
		SD3DPostEffectsUtils::CreateRenderTarget("$ZTargetScaled2", CTexture::s_ptexZTargetScaled2, nWidth >> 2, nHeight >> 2, Clr_FarPlane, 1, 0, fmtZScaled, TO_QUARTER_ZTARGET_FOR_AO);

		SD3DPostEffectsUtils::CreateRenderTarget("$DepthBufferQuarter", CTexture::s_ptexDepthBufferQuarter, nWidth >> 2, nHeight >> 2, Clr_FarPlane, false, false, eTF_D32F, -1, FT_USAGE_DEPTHSTENCIL);
	}

	// Pre-create shadow pool
	IF (gcpRendD3D->m_pRT->IsRenderThread() && gEnv->p3DEngine, 1)
	{
		//init shadow pool size
		static ICVar* p_e_ShadowsPoolSize = iConsole->GetCVar("e_ShadowsPoolSize");
		gcpRendD3D->m_nShadowPoolHeight = p_e_ShadowsPoolSize->GetIVal();
		gcpRendD3D->m_nShadowPoolWidth = gcpRendD3D->m_nShadowPoolHeight; //square atlas

		ETEX_Format eShadTF = gcpRendD3D->CV_r_shadowtexformat == 1 ? eTF_D16 : eTF_D32F;
		CTexture::s_ptexRT_ShadowPool->Invalidate(gcpRendD3D->m_nShadowPoolWidth, gcpRendD3D->m_nShadowPoolHeight, eShadTF);
		if (!CTexture::IsTextureExist(CTexture::s_ptexRT_ShadowPool))
		{
			CTexture::s_ptexRT_ShadowPool->CreateRenderTarget(eTF_Unknown, Clr_FarPlane);
		}
	}

	gcpRendD3D->GetTiledShading().CreateResources();

	gcpRendD3D->GetVolumetricFog().CreateResources();

	// shadow mask
	{
		if (CTexture::s_ptexShadowMask)
			CTexture::s_ptexShadowMask->Invalidate(gcpRendD3D->GetWidth(), gcpRendD3D->GetHeight(), eTF_R8G8B8A8);

		if (!CTexture::IsTextureExist(CTexture::s_ptexShadowMask))
		{
			const int nArraySize = (gcpRendD3D->CV_r_ShadowCastingLightsMaxCount + 3) / 4;
			CTexture::s_ptexShadowMask = CTexture::CreateTextureArray("$ShadowMask", eTT_2D, gcpRendD3D->GetWidth(), gcpRendD3D->GetHeight(), nArraySize, 1, FT_DONT_STREAM | FT_USAGE_RENDERTARGET, eTF_R8G8B8A8, TO_SHADOWMASK);
		}
	}

	// height map AO mask
	if (CRenderer::CV_r_HeightMapAO > 0)
	{
		const int nShift = clamp_tpl(3 - CRenderer::CV_r_HeightMapAO, 0, 2);
		const int hmaoWidth = gcpRendD3D->GetWidth() >> nShift;
		const int hmaoHeight = gcpRendD3D->GetHeight() >> nShift;

		for (int i = 0; i < 2; ++i)
		{
			if (CTexture::s_ptexHeightMapAO[i])
				CTexture::s_ptexHeightMapAO[i]->Invalidate(hmaoWidth, hmaoHeight, eTF_R8G8);

			if (!CTexture::IsTextureExist(CTexture::s_ptexHeightMapAO[i]))
			{
				char buf[128];
				cry_sprintf(buf, "$HeightMapAO_%d", i);

				SD3DPostEffectsUtils::CreateRenderTarget(buf, CTexture::s_ptexHeightMapAO[i], hmaoWidth, hmaoHeight, Clr_Neutral, true, false, eTF_R8G8);
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void CDeferredShading::DestroyDeferredMaps()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

struct CubemapsCompare
{
	bool operator()(const SRenderLight& l0, const SRenderLight& l1) const
	{
		// Cubes sort by: Sort priority first, light radius, lastly by entity id (insertion order every frame is not guaranteed)
		if (l0.m_nSortPriority != l1.m_nSortPriority)
			return l0.m_nSortPriority < l1.m_nSortPriority;

		if (fcmp(l0.m_fRadius, l1.m_fRadius))
			return l0.m_nEntityId < l1.m_nEntityId;

		return l0.m_fRadius > l1.m_fRadius;
	}
};

struct CubemapsCompareInv
{
	bool operator()(const SRenderLight& l0, const SRenderLight& l1) const
	{
		// Cubes sort by: Sort priority first, light radius, lastly by entity id (insertion order every frame is not guaranteed)
		if (l0.m_nSortPriority != l1.m_nSortPriority)
			return l0.m_nSortPriority > l1.m_nSortPriority;

		if (fcmp(l0.m_fRadius, l1.m_fRadius))
			return l0.m_nEntityId > l1.m_nEntityId;

		return l0.m_fRadius < l1.m_fRadius;
	}
};

struct LightsCompare
{
	bool operator()(const SRenderLight& l0, const SRenderLight& l1) const
	{
		if (!(l0.m_Flags & DLF_CASTSHADOW_MAPS) && (l1.m_Flags & DLF_CASTSHADOW_MAPS))
		{
			return true;
		}
		else
			return false;
	}
};

struct DeffDecalSort
{
	bool operator()(const SDeferredDecal& decal0, const SDeferredDecal& decal1) const
	{
		uint bBump0 = (decal0.nFlags & DECAL_HAS_NORMAL_MAP);
		uint bBump1 = (decal1.nFlags & DECAL_HAS_NORMAL_MAP);
		//bump-mapped decals first
		if (bBump0 != bBump1)
			return (bBump0 < bBump1);
		return
		  (decal0.nSortOrder < decal1.nSortOrder);
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void CDeferredShading::DebugShadowMaskClear()
{
#ifndef _RELEASE
	CD3D9Renderer* const __restrict rd = gcpRendD3D;

	static const ICVar* pCVarShadows = iConsole->GetCVar("e_Shadows");
	if ((pCVarShadows && pCVarShadows->GetIVal() == 0) || CRenderer::CV_r_ShadowPass == 0)
	{
		if (rd->FX_GetMSAAMode())
		{
			rd->FX_PushRenderTarget(0, CTexture::s_ptexBackBuffer, &gcpRendD3D->m_DepthBufferOrig);
			rd->RT_SetViewport(0, 0, CTexture::s_ptexBackBuffer->GetWidth(), CTexture::s_ptexBackBuffer->GetHeight());

			// MSAA stencil mask is shared on this target, only clear alpha channel
			rd->FX_SetActiveRenderTargets();
			rd->m_pNewTarget[0]->m_ReqColor = Clr_Transparent;
			rd->m_pNewTarget[0]->m_ClearFlags = CLEAR_RTARGET;
			rd->FX_ClearTargetRegion(GS_COLMASK_A);
			rd->m_pNewTarget[0]->m_ClearFlags = 0;

			rd->FX_PopRenderTarget(0);
		}
		else
		{
			rd->FX_ClearTarget(CTexture::s_ptexBackBuffer, Clr_Transparent);
		}
	}
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void CDeferredShading::Render(CRenderView* pRenderView)
{
	m_pCurrentRenderView = pRenderView;

	m_nLightsProcessedCount = 0;

	CD3D9Renderer* const __restrict rd = gcpRendD3D;

	uint64 nFlagsShaderRT = rd->m_RP.m_FlagsShader_RT;

	rd->FX_ResetPipe();

	SetupPasses();

	auto& rDeferredLights = pRenderView->GetLightsArray(eDLT_DeferredLight);
	auto& rDeferredCubemaps = pRenderView->GetLightsArray(eDLT_DeferredCubemap);
	auto& rDeferredAmbientLights = pRenderView->GetLightsArray(eDLT_DeferredAmbientLight);

	const bool bRenderLPVs = LPVManager.IsRenderable();

#ifdef SUPPORTS_MSAA
	if (rd->m_RP.m_MSAAData.Type)
	{
		m_pNormalsRT->SetResolved(true);
		m_pDepthRT->SetResolved(true);
	}
#endif

	if (CRenderer::CV_r_DeferredShadingScissor)
		rd->EF_Scissor(false, 0, 0, 0, 0);

	if (CRenderer::CV_r_DeferredShadingDepthBoundsTest)
		rd->SetDepthBoundTest(0.f, DBT_SKY_CULL_DEPTH, true); // skip sky for ambient and deferred cubemaps

	int iTempX, iTempY, iWidth, iHeight;
	rd->GetViewport(&iTempX, &iTempY, &iWidth, &iHeight);

	bool bOutdoorVisible = false;
	PrepareClipVolumeData(bOutdoorVisible);

	FilterGBuffer();

	if (rd->m_nGraphicsPipeline == 0)
	{
		// Generate directional occlusion information
		DirectionalOcclusionPass();

		// Generate glossy screenspace reflections
		ScreenSpaceReflectionPass();

		PackAllShadowFrustums(true);
	}

	rd->FX_SetState(GS_BLSRC_ONE | GS_BLDST_ZERO);

	// sort lights
	if (rDeferredCubemaps.size())
	{
		if (CRenderer::CV_r_DeferredShadingTiled <= 1)
			std::sort(rDeferredCubemaps.begin(), rDeferredCubemaps.end(), CubemapsCompare());
		else
			std::sort(rDeferredCubemaps.begin(), rDeferredCubemaps.end(), CubemapsCompareInv());
	}

	// Process deferred direct lighting accumulation
	rd->FX_PushRenderTarget(0, m_pLBufferDiffuseRT, &gcpRendD3D->m_DepthBufferOrigMSAA, -1, false, 1);
	rd->FX_PushRenderTarget(1, m_pLBufferSpecularRT, NULL, -1, false, 1);
	m_bSpecularState = true;

	if (gRenDev->GetWireframeMode())
	{
		rd->FX_ClearTarget(m_pLBufferDiffuseRT);
		rd->FX_ClearTarget(m_pLBufferSpecularRT);
		rd->FX_ClearTarget(&gcpRendD3D->m_DepthBufferOrigMSAA, CLEAR_STENCIL, Clr_Unused.r, 1);
		rd->m_nStencilMaskRef = 1;// Stencil initialized to 1 - 0 is reserved for MSAAed samples
	}

	rd->RT_SetViewport(0, 0, gRenDev->GetWidth(), gRenDev->GetHeight());

	uint32 nCurrentDeferredCubemap = 0;

	bool bTiledDeferredShading = CRenderer::CV_r_DeferredShadingTiled >= 2;

	if (CRenderer::CV_r_DeferredShadingDebug == 2)
	{
		gcpRendD3D->m_RP.m_FlagsShader_RT |= RT_OVERDRAW_DEBUG;
		bTiledDeferredShading = false;
	}
	if (CRenderer::CV_r_Unlit)
	{
		bTiledDeferredShading = false;
	}

	// determine if we have a global cubemap in the scene
	SRenderLight* pGlobalCubemap = NULL;
	if (rDeferredCubemaps.size() && CRenderer::CV_r_DeferredShadingEnvProbes)
	{
		SRenderLight& pFirstLight = rDeferredCubemaps[0];
		ITexture* pDiffuseCubeCheck = pFirstLight.GetDiffuseCubemap();
		float fRadius = pFirstLight.m_fRadius;

		if (pDiffuseCubeCheck && fRadius >= 100000.f)
		{
			pGlobalCubemap = &pFirstLight;
			++nCurrentDeferredCubemap;
		}
	}

	if (!bTiledDeferredShading)
	{
		if (CRenderer::CV_r_DeferredShadingAmbient && AmbientPass(pGlobalCubemap, bOutdoorVisible))
			m_nLightsProcessedCount++;

		DeferredCubemaps(rDeferredCubemaps, nCurrentDeferredCubemap);

		if (CRenderer::CV_r_DeferredShadingAmbientLights)
			DeferredLights(rDeferredAmbientLights, false);

		ApplySSReflections();  // TODO: Try to merge with another pass
	}

	SpecularAccEnableMRT(false);

	if (bOutdoorVisible && bRenderLPVs)
	{
		if (GIPass())
			m_nLightsProcessedCount++;
	}

	// apply lighting from LPVs
	if (bRenderLPVs && CRenderer::CV_r_LightPropagationVolumes)
	{
		LPVManager.Render();
		m_nLightsProcessedCount++;
	}

	SpecularAccEnableMRT(true);

	if (CRenderer::CV_r_DeferredShadingDepthBoundsTest)
		rd->SetDepthBoundTest(0.0f, 1.0f, false);

	if (CRenderer::CV_r_DeferredShadingLights && !bTiledDeferredShading)
		DeferredLights(rDeferredLights, true);

	if (CRenderer::CV_r_DeferredShadingScissor)
		rd->EF_Scissor(false, 0, 0, 0, 0);

	if (CRenderer::CV_r_DeferredShadingDepthBoundsTest)
		rd->SetDepthBoundTest(0.0f, 1.0f, false);

	if (CRenderer::CV_r_Unlit)
	{
		rd->FX_ClearTarget(m_pLBufferDiffuseRT, Clr_MedianHalf);
		rd->FX_ClearTarget(m_pLBufferSpecularRT, Clr_Transparent);
		rd->FX_ClearTarget(&gcpRendD3D->m_DepthBufferOrigMSAA, CLEAR_STENCIL, Clr_Unused.r, 0);
	}

	// Commit any potential render target changes - required for x360 resolves, do not remove this plz.
	rd->FX_SetActiveRenderTargets();

	rd->FX_PopRenderTarget(0);
	rd->FX_PopRenderTarget(1);
	m_bSpecularState = false;

#ifdef SUPPORTS_MSAA
	if (rd->m_RP.m_MSAAData.Type)
	{
		// Commit any potential render target changes - required for when shadows disabled
		rd->FX_SetActiveRenderTargets();

		rd->FX_MSAASampleFreqStencilSetup();
		// Ensure no redundant resolves happen for MSAA
		m_pLBufferDiffuseRT->SetResolved(true);
		m_pLBufferSpecularRT->SetResolved(true);
		m_pNormalsRT->SetResolved(true);
	}
#endif

	rd->FX_WaterVolumesCaustics(pRenderView);

	if (bTiledDeferredShading)
	{
		gcpRendD3D->GetTiledShading().Render(pRenderView, m_vClipVolumeParams);

		if (CRenderer::CV_r_DeferredShadingSSS && rd->FX_GetMSAAMode() == 0)  // Explicitly disabling deferred SSS has its incompatible with msaa in current stage
			DeferredSubsurfaceScattering(CTexture::s_ptexSceneTargetR11G11B10F[0]);
	}
	else
	{
		// GPU light culling
		if (CRenderer::CV_r_DeferredShadingTiled == 1)
		{
			// Sort cubemaps in inverse order for tiled forward shading
			std::sort(rDeferredCubemaps.begin(), rDeferredCubemaps.end(), CubemapsCompareInv());

			gcpRendD3D->GetTiledShading().Render(pRenderView, m_vClipVolumeParams);
		}

		DeferredShadingPass();
	}

	rd->RT_SetViewport(iTempX, iTempY, iWidth, iHeight);

#ifndef _RELEASE
	if (CRenderer::CV_r_DeferredShadingDebugGBuffer)
		DebugGBuffer();
#endif

	DebugShadowMaskClear();

	// Commit any potential render target changes - required for when shadows disabled
	rd->FX_SetActiveRenderTargets();

	rd->m_RP.m_FlagsShader_RT = nFlagsShaderRT;
	rd->m_RP.m_PersFlags2 &= ~RBPF2_WRITEMASK_RESERVED_STENCIL_BIT;

	rd->FX_ResetPipe();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void CDeferredShading::Release()
{
	DestroyDeferredMaps();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void CDeferredShading::GetScissors(const Vec3& vCenter, float fRadius, short& sX, short& sY, short& sWidth, short& sHeight) const
{
	Vec3 vViewVec = vCenter - gcpRendD3D->GetCamera().GetPosition();
	float fDistToLS = vViewVec.GetLength();

	bool bInsideLightVolume = fDistToLS <= fRadius;
	if (bInsideLightVolume)
	{
		sX = sY = 0;
		sWidth = gcpRendD3D->GetWidth();
		sHeight = gcpRendD3D->GetHeight();
		return;
	}

	Matrix44 mProj, mView;
	gcpRendD3D->GetProjectionMatrix(mProj.GetData());
	gcpRendD3D->GetModelViewMatrix(mView.GetData());

	Vec3 pBRectVertices[4];
	Vec4 vCenterVS = Vec4(vCenter, 1) * mView;

	{
		// Compute tangent planes
		float r = fRadius;
		float sq_r = r * r;

		Vec3 vLPosVS = Vec3(vCenterVS.x, vCenterVS.y, vCenterVS.z);
		float lx = vLPosVS.x;
		float ly = vLPosVS.y;
		float lz = vLPosVS.z;
		float sq_lx = lx * lx;
		float sq_ly = ly * ly;
		float sq_lz = lz * lz;

		// Compute left and right tangent planes to light sphere
		float sqrt_d = sqrt_tpl(max(sq_r * sq_lx - (sq_lx + sq_lz) * (sq_r - sq_lz), 0.0f));
		float nx = (r * lx + sqrt_d) / (sq_lx + sq_lz);
		float nz = iszero(lz) ? 1.0f : (r - nx * lx) / lz;

		Vec3 vTanLeft = Vec3(nx, 0, nz).normalized();

		nx = (r * lx - sqrt_d) / (sq_lx + sq_lz);
		nz = iszero(lz) ? 1.0f : (r - nx * lx) / lz;
		Vec3 vTanRight = Vec3(nx, 0, nz).normalized();

		pBRectVertices[0] = vLPosVS - r * vTanLeft;
		pBRectVertices[1] = vLPosVS - r * vTanRight;

		// Compute top and bottom tangent planes to light sphere
		sqrt_d = sqrt_tpl(max(sq_r * sq_ly - (sq_ly + sq_lz) * (sq_r - sq_lz), 0.0f));
		float ny = (r * ly - sqrt_d) / (sq_ly + sq_lz);
		nz = iszero(lz) ? 1.0f : (r - ny * ly) / lz;
		Vec3 vTanBottom = Vec3(0, ny, nz).normalized();

		ny = (r * ly + sqrt_d) / (sq_ly + sq_lz);
		nz = iszero(lz) ? 1.0f : (r - ny * ly) / lz;
		Vec3 vTanTop = Vec3(0, ny, nz).normalized();

		pBRectVertices[2] = vLPosVS - r * vTanTop;
		pBRectVertices[3] = vLPosVS - r * vTanBottom;
	}

	Vec2 vPMin = Vec2(1, 1);
	Vec2 vPMax = Vec2(0, 0);
	Vec2 vMin = Vec2(1, 1);
	Vec2 vMax = Vec2(0, 0);

	// Project all vertices
	for (int i = 0; i < 4; i++)
	{
		Vec4 vScreenPoint = Vec4(pBRectVertices[i], 1.0) * mProj;

		//projection space clamping
		vScreenPoint.w = max(vScreenPoint.w, 0.00000000000001f);
		vScreenPoint.x = max(vScreenPoint.x, -(vScreenPoint.w));
		vScreenPoint.x = min(vScreenPoint.x, vScreenPoint.w);
		vScreenPoint.y = max(vScreenPoint.y, -(vScreenPoint.w));
		vScreenPoint.y = min(vScreenPoint.y, vScreenPoint.w);

		//NDC
		vScreenPoint /= vScreenPoint.w;

		//output coords
		//generate viewport (x=0,y=0,height=1,width=1)
		Vec2 vWin;
		vWin.x = (1.0f + vScreenPoint.x) * 0.5f;
		vWin.y = (1.0f + vScreenPoint.y) * 0.5f;   //flip coords for y axis

		assert(vWin.x >= 0.0f && vWin.x <= 1.0f);
		assert(vWin.y >= 0.0f && vWin.y <= 1.0f);

		// Get light sphere screen bounds
		vMin.x = min(vMin.x, vWin.x);
		vMin.y = min(vMin.y, vWin.y);
		vMax.x = max(vMax.x, vWin.x);
		vMax.y = max(vMax.y, vWin.y);
	}

	float fWidth = (float)gcpRendD3D->GetWidth();
	float fHeight = (float)gcpRendD3D->GetHeight();

	sX = (short)(vMin.x * fWidth);
	sY = (short)((1.0f - vMax.y) * fHeight);
	sWidth = (short)ceilf((vMax.x - vMin.x) * fWidth);
	sHeight = (short)ceilf((vMax.y - vMin.y) * fHeight);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void CDeferredShading::SetupScissors(bool bEnable, uint16 x, uint16 y, uint16 w, uint16 h) const
{
	gcpRendD3D->EF_Scissor(bEnable, x, y, w, h);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void CDeferredShading::Debug()
{
	uint64 nFlagsShaderRT = gcpRendD3D->m_RP.m_FlagsShader_RT;

	if (CRenderer::CV_r_DeferredShadingDebug == 2)
	{
#ifdef SUPPORTS_MSAA
		if (gcpRendD3D->m_RP.m_MSAAData.Type)
		{
			// Explicit msaa resolve, so that non-msaa texture bind is valid for debug
			m_pLBufferDiffuseRT->SetResolved(false);
			m_pLBufferDiffuseRT->Resolve();
		}
#endif
		SD3DPostEffectsUtils::ShBeginPass(m_pShader, m_pDebugTechName, FEF_DONTSETTEXTURES | FEF_DONTSETSTATES);
		gcpRendD3D->FX_SetState(GS_NODEPTHTEST);
		m_pLBufferDiffuseRT->Apply(0, m_nTexStatePoint);
		SD3DPostEffectsUtils::SetTexture(CTexture::s_ptexPaletteDebug, 1, FILTER_LINEAR, 1);
		SD3DPostEffectsUtils::DrawFullScreenTriWPOS(m_pLBufferDiffuseRT->GetWidth(), m_pLBufferDiffuseRT->GetHeight());
		SD3DPostEffectsUtils::ShEndPass();
	}

	gcpRendD3D->m_RP.m_FlagsShader_RT = nFlagsShaderRT;
}

void CDeferredShading::DebugGBuffer()
{
	CD3D9Renderer* const __restrict rd = gcpRendD3D;

	static CCryNameTSCRC techShadingDebug("DebugGBuffer");

	CTexture* dstTex = CTexture::s_ptexStereoR;

	m_pDepthRT->Apply(0, m_nTexStatePoint);
	m_pNormalsRT->Apply(1, m_nTexStatePoint);
	m_pDiffuseRT->Apply(2, m_nTexStatePoint);
	m_pSpecularRT->Apply(3, m_nTexStatePoint);

	rd->FX_SetState(GS_NODEPTHTEST);

	rd->FX_PushRenderTarget(0, dstTex, NULL);
	SD3DPostEffectsUtils::ShBeginPass(m_pShader, techShadingDebug, FEF_DONTSETTEXTURES | FEF_DONTSETSTATES);
	static CCryNameR paramName("DebugViewMode");
	Vec4 param(CRenderer::CV_r_DeferredShadingDebugGBuffer, 0, 0, 0);
	m_pShader->FXSetPSFloat(paramName, &param, 1);
	SD3DPostEffectsUtils::DrawFullScreenTri(dstTex->GetWidth(), dstTex->GetHeight());
	SD3DPostEffectsUtils::ShEndPass();
	rd->FX_PopRenderTarget(0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int CRenderer::EF_AddDeferredLight(const CDLight& pLight, float fMult, const SRenderingPassInfo& passInfo)
{
	CDeferredShading& pDS = CDeferredShading::Instance();
	int nLightID = pDS.AddLight(pLight, fMult, passInfo);

	int nThreadID = m_RP.m_nFillThreadID;
	float mipFactor = (m_RP.m_TI[nThreadID].m_cam.GetPosition() - pLight.m_Origin).GetLengthSquared() / max(0.001f, pLight.m_fRadius * pLight.m_fRadius);
	EF_PrecacheResource(const_cast<CDLight*>(&pLight), mipFactor, 0.1f, FPR_STARTLOADING, gRenDev->m_RP.m_TI[nThreadID].m_arrZonesRoundId[1]);
	return nLightID;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void CRenderer::EF_ReleaseDeferredData()
{
	if (CDeferredShading::IsValid())
		CDeferredShading::Instance().ReleaseData();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void CRenderer::Ef_AddDeferredGIClipVolume(const IRenderMesh* pClipVolume, const Matrix34& mxTransform)
{
	if (pClipVolume)
		CDeferredShading::Instance().AddGIClipVolume(const_cast<IRenderMesh*>(pClipVolume), mxTransform);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool CD3D9Renderer::FX_DeferredRendering(CRenderView* pRenderView, bool bDebugPass, bool bUpdateRTOnly)
{
	CDeferredShading& pDS = CDeferredShading::Instance();

	if (!CTexture::s_ptexSceneTarget)
	{
		pDS.Release();
		return false;
	}

	if (bUpdateRTOnly)
	{
		pDS.CreateDeferredMaps();
		return true;
	}

	if (!bDebugPass)
		pDS.Render(pRenderView);
	else
		pDS.Debug();

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

bool CD3D9Renderer::FX_DeferredDecals()
{
	CD3D9Renderer* const __restrict rd = gcpRendD3D;

	if (CV_r_deferredDecals)
	{
		auto& deferredDecals = m_RP.RenderView()->GetDeferredDecals();
#if !CRY_PLATFORM_ORBIS
		// Want the buffer cleared or we'll just get black out
		if (deferredDecals.empty())
			return false;
#endif

		PROFILE_LABEL_SCOPE("DEFERRED_DECALS");

		CDeferredShading& rDS = CDeferredShading::Instance();
		rDS.SetupPasses();

		D3DTexture* pBBRes = CTexture::s_ptexBackBuffer->GetDevTexture()->Get2DTexture();
		D3DTexture* pNMRes = NULL;

		assert(CTexture::s_ptexBackBuffer->m_pPixelFormat->DeviceFormat == CTexture::s_ptexSceneNormalsMap->m_pPixelFormat->DeviceFormat);

		if (rd->m_RP.m_MSAAData.Type > 1) //always copy when deferredDecals is not empty
		{
			pNMRes = CTexture::s_ptexSceneNormalsMap->m_pRenderTargetData->m_pDeviceTextureMSAA->Get2DTexture();
			rd->GetDeviceContext().ResolveSubresource(pBBRes, 0, pNMRes, 0, (DXGI_FORMAT)CTexture::s_ptexBackBuffer->m_pPixelFormat->DeviceFormat);
		}
		else
		{
			pNMRes = CTexture::s_ptexSceneNormalsMap->GetDevTexture()->Get2DTexture();
			rd->GetDeviceContext().CopyResource(pBBRes, pNMRes);
		}

		std::stable_sort(deferredDecals.begin(), deferredDecals.end(), DeffDecalSort());

		//if (CV_r_deferredDecalsDebug == 0)
		//	rd->FX_PushRenderTarget(1, CTexture::s_ptexSceneNormalsMap, NULL);

		const size_t nNumDecals = deferredDecals.size();
		for (size_t d = 0; d < nNumDecals; ++d)
		{
			rDS.DeferredDecalPass(deferredDecals[d], d);
		}

		//if (CV_r_deferredDecalsDebug == 0)
		//	rd->FX_PopRenderTarget(1);

		rd->SetCullMode(R_CULL_BACK);

		// Commit any potential render target changes - required for when shadows disabled
		rd->FX_SetActiveRenderTargets();
		rd->FX_ResetPipe();

		return true;
	}
	return false;

}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

bool CREDeferredShading::mfDraw(CShader* ef, SShaderPass* sfm)
{
	if (gcpRendD3D->m_bDeviceLost)
		return 0;

	gcpRendD3D->FX_DeferredRendering(gRenDev->m_RP.m_pCurrentRenderView);
	return true;
}
