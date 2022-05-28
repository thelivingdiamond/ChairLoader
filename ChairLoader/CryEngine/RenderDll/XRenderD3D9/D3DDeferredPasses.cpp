// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#include "StdAfx.h"
#include "DriverD3D.h"
#include <Cry3DEngine/I3DEngine.h>
#include "D3DPostProcess.h"

#pragma warning(disable: 4244)

bool CD3D9Renderer::FX_DeferredCaustics()
{
	if (!CRenderer::CV_r_watercaustics || !CRenderer::CV_r_watercausticsdeferred || !CTexture::s_ptexBackBuffer || !CTexture::s_ptexSceneTarget)
		return false;

	const Vec4 causticParams = gEnv->p3DEngine->GetOceanAnimationCausticsParams();
	float fCausticsHeight = causticParams.y;
	float fCausticsDepth = causticParams.z;
	float fCausticsIntensity = causticParams.w;

	N3DEngineCommon::SOceanInfo& OceanInfo = gRenDev->m_p3DEngineCommon.m_OceanInfo;
	bool bOceanVolumeVisible = (OceanInfo.m_nOceanRenderFlags & OCR_OCEANVOLUME_VISIBLE) != 0;
	if (!bOceanVolumeVisible || iszero(fCausticsIntensity))
		return false;

	uint64 nFlagsShaderRTSave = gcpRendD3D->m_RP.m_FlagsShader_RT;

	if (m_LogFile)
		Logv(" +++ Deferred caustics pass begin +++ \n");

	PROFILE_LABEL_SCOPE("OCEAN_CAUSTICS");
	PROFILE_FRAME(DrawShader_DeferredCausticsPass);

	float fWatLevel = OceanInfo.m_fWaterLevel;
	float fCausticsLevel = fWatLevel + fCausticsHeight;
	Vec4 pCausticsParams = Vec4(CRenderer::CV_r_watercausticsdistance, OceanInfo.m_vCausticsParams.z * fCausticsIntensity,
	                            OceanInfo.m_vCausticsParams.w, fCausticsLevel);
	static float fDist = 0.0f;

	fDist = sqrtf((pCausticsParams.x * 5.0f) * 13.333f);  // Hard cut off when caustic would be attenuated to 0.2 (1/5.0f)

	I3DEngine* pEng = gEnv->p3DEngine;

	// Caustics are done with projection from sun - hence they update too fast with regular
	// sun direction. Use a smooth sun direction update instead to workaround this

	SCGParamsPF& PF = gRenDev->m_cEF.m_PF[m_RP.m_nProcessThreadID];
	{
		Vec3 pRealtimeSunDirNormalized = pEng->GetRealtimeSunDirNormalized();

		const float fSnapDot = 0.98f;
		float fDot = fabs(PF.vCausticsCurrSunDir.Dot(pRealtimeSunDirNormalized));
		if (fDot < fSnapDot)
			PF.vCausticsCurrSunDir = pRealtimeSunDirNormalized;

		PF.vCausticsCurrSunDir += (pRealtimeSunDirNormalized - PF.vCausticsCurrSunDir) * 0.005f * gEnv->pTimer->GetFrameTime();
		PF.vCausticsCurrSunDir.Normalize();
	}

	Matrix44 m_pLightView;

	Vec3 up = Vec3(0, 0, 1);
	Vec3 dirZ = -PF.vCausticsCurrSunDir.GetNormalized();
	Vec3 dirX = up.Cross(dirZ).GetNormalized();
	Vec3 dirY = dirZ.Cross(dirX).GetNormalized();

	m_pLightView.SetIdentity();
	m_pLightView.SetRow(0, dirX);
	m_pLightView.SetRow(1, dirY);
	m_pLightView.SetRow(2, dirZ);

	float fTime = 0.125f * gcpRendD3D->m_RP.m_TI[gcpRendD3D->m_RP.m_nProcessThreadID].m_RealTime;

	Vec4 vAnimParams = Vec4(0.06f * fTime, 0.05f * fTime, 0.1f * fTime, -0.11f * fTime);

	// Stencil pre-pass
	if (CRenderer::CV_r_watercausticsdeferred == 2)
	{
		// stencil pre-pass
		CShader* pSH(CShaderMan::s_ShaderShadowMaskGen);

		// make box for stencil passes
		t_arrDeferredMeshIndBuff arrDeferredInds;
		t_arrDeferredMeshVertBuff arrDeferredVerts;
		CreateDeferredUnitBox(arrDeferredInds, arrDeferredVerts);

		Vec3 vCamPos = gRenDev->GetRCamera().vOrigin;
		float fWaterPlaneSize = gRenDev->GetCamera().GetFarPlane();

		m_RP.m_TI[m_RP.m_nProcessThreadID].m_matView->Push();
		Matrix34 mLocal;
		mLocal.SetIdentity();

		float heightAboveWater = max(0.0f, vCamPos.z - fCausticsLevel);
		fDist = sqrtf(max((fDist * fDist) - (heightAboveWater * heightAboveWater), 0.0f));

		//TODO: Adjust Z on fog density

		mLocal.SetScale(Vec3(fDist * 2, fDist * 2, fCausticsHeight + fCausticsDepth));
		mLocal.SetTranslation(Vec3(vCamPos.x - fDist, vCamPos.y - fDist, fWatLevel - fCausticsDepth));

		Matrix44 mLocalTransposed = mLocal.GetTransposed();
		m_RP.m_TI[m_RP.m_nProcessThreadID].m_matView->MultMatrixLocal(&mLocalTransposed);

		uint32 nPasses = 0;
		static CCryNameTSCRC TechName0 = "DeferredShadowPass";
		pSH->FXSetTechnique(TechName0);
		pSH->FXBegin(&nPasses, FEF_DONTSETSTATES);
		pSH->FXBeginPass(DS_SHADOW_CULL_PASS);

		//allocate vertices
		TempDynVB<SVF_P3F_C4B_T2F>::CreateFillAndBind(&arrDeferredVerts[0], arrDeferredVerts.size(), 0);

		//allocate indices
		TempDynIB16::CreateFillAndBind(&arrDeferredInds[0], arrDeferredInds.size());

		if (!FAILED(FX_SetVertexDeclaration(0, eVF_P3F_C4B_T2F)))
			FX_StencilCullPass(-1, arrDeferredVerts.size(), arrDeferredInds.size());

		pSH->FXEndPass();
		pSH->FXEnd();

		m_RP.m_TI[m_RP.m_nProcessThreadID].m_matView->Pop();

		FX_StencilTestCurRef(true, false);

	}

	// Deferred caustic pass
	gcpRendD3D->EF_Scissor(false, 0, 0, 0, 0);

	gRenDev->m_cEF.mfRefreshSystemShader("DeferredCaustics", CShaderMan::s_ShaderDeferredCaustics);

	CShader* pShader = CShaderMan::s_ShaderDeferredCaustics;
	gcpRendD3D->m_RP.m_FlagsShader_RT &= ~g_HWSR_MaskBit[HWSR_SAMPLE0] | g_HWSR_MaskBit[HWSR_SAMPLE1] | g_HWSR_MaskBit[HWSR_SAMPLE2] | g_HWSR_MaskBit[HWSR_SAMPLE3];

	static CCryNameTSCRC pTechName = "General";
	SD3DPostEffectsUtils::ShBeginPass(pShader, pTechName, FEF_DONTSETSTATES);

	int32 nRState = GS_NODEPTHTEST | ((CRenderer::CV_r_watercausticsdeferred == 2) ? GS_STENCIL : 0) | (GS_BLSRC_ONE | GS_BLDST_ONEMINUSSRCALPHA);

	gcpRendD3D->FX_SetState(nRState);

	static CCryNameR m_pParamAnimParams("vAnimParams");
	static CCryNameR m_pCausticParams("vCausticParams");
	static CCryNameR m_pParamLightView("mLightView");
	pShader->FXSetPSFloat(m_pParamAnimParams, &vAnimParams, 1);
	pShader->FXSetPSFloat(m_pCausticParams, &pCausticsParams, 1);
	pShader->FXSetPSFloat(m_pParamLightView, (Vec4*) m_pLightView.GetData(), 4);

	SD3DPostEffectsUtils::DrawFullScreenTriWPOS(CTexture::s_ptexSceneTarget->GetWidth(), CTexture::s_ptexSceneTarget->GetHeight()); // TODO: Use Volume

	SD3DPostEffectsUtils::ShEndPass();

	if (CRenderer::CV_r_watercausticsdeferred == 2)
		FX_StencilTestCurRef(false);

	if (m_LogFile)
		Logv(" +++ Deferred caustics pass end +++ \n");

	gcpRendD3D->m_RP.m_FlagsShader_RT = nFlagsShaderRTSave;
	//m_RP.m_TI[m_RP.m_nProcessThreadID].m_PersFlags2 = nPersFlags2Save;

	FX_ResetPipe();

	return true;
}

bool CD3D9Renderer::FX_DeferredWaterVolumeCaustics(const N3DEngineCommon::SCausticInfo& causticInfo)
{
	if (!CTexture::s_ptexBackBuffer || !CTexture::s_ptexSceneTarget)
		return false;

	//gRenDev->m_cEF.mfRefreshSystemShader("DeferredCaustics", CShaderMan::m_ShaderDeferredCaustics);

	CShader* pShader = CShaderMan::s_ShaderDeferredCaustics;

	if (m_LogFile)
		Logv(" +++ Deferred caustics pass begin +++ \n");

	PROFILE_LABEL_SCOPE("DEFERRED WATERVOLUME CAUSTICS");

	bool bTiledDeferredShading = CRenderer::CV_r_DeferredShadingTiled >= 2;

	if (bTiledDeferredShading)
		gcpRendD3D->FX_PushRenderTarget(0, CTexture::s_ptexSceneTargetR11G11B10F[1], NULL);
	else
		gcpRendD3D->FX_PushRenderTarget(0, CTexture::s_ptexSceneDiffuseAccMap, NULL);

	static CCryNameTSCRC pTechName = "WaterVolumeCaustics";

	SD3DPostEffectsUtils::ShBeginPass(pShader, pTechName, FEF_DONTSETSTATES);

	int32 nRState = GS_NODEPTHTEST;
	if (!bTiledDeferredShading)
		nRState |= GS_BLSRC_ONE | GS_BLDST_ONE;  // Blend directly into light accumulation buffer

	gcpRendD3D->FX_SetState(nRState);

	static CCryNameR m_pParamLightView("mLightView");
	pShader->FXSetPSFloat(m_pParamLightView, (Vec4*) causticInfo.m_mCausticMatr.GetData(), 4);

	SD3DPostEffectsUtils::DrawFullScreenTriWPOS(CTexture::s_ptexSceneTarget->GetWidth(), CTexture::s_ptexSceneTarget->GetHeight());

	SD3DPostEffectsUtils::ShEndPass();

	gcpRendD3D->FX_PopRenderTarget(0);

	if (m_LogFile)
		Logv(" +++ Deferred caustics pass end +++ \n");

	FX_ResetPipe();

	if (bTiledDeferredShading)
		GetTiledShading().NotifyCausticsVisible();

	return true;
}

bool CD3D9Renderer::FX_DeferredRainOcclusionMap(const N3DEngineCommon::ArrOccluders& arrOccluders, const SRainParams& rainVolParams)
{
	PROFILE_LABEL_SCOPE("OCCLUSION_PASS");

	const Matrix44& matOccTrans = rainVolParams.matOccTrans;
	uint64 nFlagsShaderRTSave = m_RP.m_FlagsShader_RT;

	Matrix44 matTrans;
	static const Matrix44 matSs2Ps
	  (2.f, 0.f, 0.f, -1.f,
	  0.f, 2.f, 0.f, -1.f,
	  0.f, 0.f, 1.f, 0.f,
	  0.f, 0.f, 0.f, 1.f);
	matTrans = matSs2Ps * matOccTrans;

	// Create texture if required
	if (!CTexture::IsTextureExist(CTexture::s_ptexRainOcclusion))
	{
		if (!CTexture::s_ptexRainOcclusion->Create2DTexture(RAIN_OCC_MAP_SIZE, RAIN_OCC_MAP_SIZE, 1,
		                                                    FT_DONT_RELEASE | FT_DONT_STREAM | FT_USAGE_RENDERTARGET,
		                                                    0, eTF_R8G8B8A8, eTF_R8G8B8A8))
		{
			return false;
		}
	}
	CTexture::s_ptexRainOcclusion->SetRenderTargetTile(0);

	// Get temp depth buffer
	SDepthTexture* pTmpDepthSurface = FX_GetDepthSurface(RAIN_OCC_MAP_SIZE, RAIN_OCC_MAP_SIZE, false);

	// Render geometry to rain occlusion map
	FX_PushRenderTarget(0, CTexture::s_ptexRainOcclusion, pTmpDepthSurface);

	// Get current viewport
	int iTempX, iTempY, iWidth, iHeight;
	GetViewport(&iTempX, &iTempY, &iWidth, &iHeight);
	RT_SetViewport(0, 0, RAIN_OCC_MAP_SIZE, RAIN_OCC_MAP_SIZE);

	EF_ClearTargetsLater(FRT_CLEAR_COLOR | FRT_CLEAR_DEPTH, Clr_Neutral, Clr_FarPlane.r, 0);

	FX_SetState(GS_DEPTHFUNC_LEQUAL | GS_DEPTHWRITE);
	SetCullMode(R_CULL_NONE);

	CShader* pSH = m_cEF.s_ShaderDeferredRain;
	uint32 nPasses = 0;
	static CCryNameTSCRC TechName("RainOcclusion");
	pSH->FXSetTechnique(TechName);
	pSH->FXBegin(&nPasses, FEF_DONTSETTEXTURES | FEF_DONTSETSTATES);
	pSH->FXBeginPass(0);

	static CCryNameR occTransMatParamName("g_RainOcc_TransMat");
	for (N3DEngineCommon::ArrOccluders::const_iterator it = arrOccluders.begin();
	     it != arrOccluders.end(); ++it)
	{
		if (it->m_RndMesh)
		{
			Matrix44A matWVP(it->m_WorldMat);
			matWVP = matTrans * matWVP;
			pSH->FXSetVSFloat(occTransMatParamName, (Vec4*)matWVP.GetData(), 4);

			FX_Commit();

			static_cast<CRenderMesh*>(it->m_RndMesh.get())->DrawImmediately();
		}
	}

	pSH->FXEndPass();
	pSH->FXEnd();

	FX_PopRenderTarget(0);

	RT_SetViewport(iTempX, iTempY, iWidth, iHeight);

	gcpRendD3D->m_RP.m_FlagsShader_RT = nFlagsShaderRTSave;

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

bool CD3D9Renderer::FX_DeferredRainOcclusion()
{
	SRainParams& rainVolParams = m_p3DEngineCommon.m_RainInfo;

	if (rainVolParams.areaAABB.IsReset())
		return false;

	if (m_p3DEngineCommon.m_RainOccluders.m_bProcessed[RT_GetCurrGpuID()])
		return true;

	PROFILE_LABEL_SCOPE("DEFERRED_RAIN_OCCLUSION");

	bool bRet = true;
	const N3DEngineCommon::ArrOccluders& arrOccluders = m_p3DEngineCommon.m_RainOccluders.m_arrCurrOccluders[m_RP.m_nProcessThreadID];
	if (!arrOccluders.empty())
	{
		// Render occluders to occlusion map
		bRet = FX_DeferredRainOcclusionMap(arrOccluders, rainVolParams);
		m_p3DEngineCommon.m_RainOccluders.m_bProcessed[RT_GetCurrGpuID()] = true;
		if (bRet)
			rainVolParams.matOccTransRender = rainVolParams.matOccTrans;
	}

	return bRet;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

bool CD3D9Renderer::FX_DeferredRainPreprocess()
{
	// Snow also uses the occlusion computation.
	CEffectParam* pRainActive = PostEffectMgr()->GetByName("SceneRain_Active");
	if (pRainActive)
		pRainActive->SetParam(0);

	CEffectParam* pSnowActive = PostEffectMgr()->GetByName("SceneSnow_Active");
	if (pSnowActive)
		pSnowActive->SetParam(0);

	if ((CV_r_rain < 1 && CV_r_snow < 1) || !CV_r_PostProcess || !CTexture::s_ptexBackBuffer || !CTexture::s_ptexSceneTarget)
	{
		return false;
	}

	SRainParams& rainVolParams = m_p3DEngineCommon.m_RainInfo;
	SSnowParams& snowVolParams = m_p3DEngineCommon.m_SnowInfo;

	bool bRenderSnow = ((snowVolParams.m_fSnowAmount > 0.05f || snowVolParams.m_fFrostAmount > 0.05f) && snowVolParams.m_fRadius > 0.05f && CV_r_snow > 0);
	bool bRenderRain = (rainVolParams.fAmount * CRenderer::CV_r_rainamount > 0.05f && rainVolParams.fRadius > 0.05f && CV_r_rain > 0);

	bool bRender = bRenderSnow || bRenderRain;
	if (!bRender)
		return false;

	bool bRet = true;
	if (rainVolParams.bApplyOcclusion && ((CV_r_snow == 2 && bRenderSnow) || (CV_r_rain == 2 && bRenderRain)))
	{
		bRet = FX_DeferredRainOcclusion();
	}

	if (bRet && bRenderRain && pRainActive)
	{
		CSceneRain* pEffRain = (CSceneRain*)PostEffectMgr()->GetEffect(ePFX_SceneRain);
		if (!pEffRain)
			return false;

		pEffRain->m_RainVolParams = rainVolParams;
		pRainActive->SetParam(1);
	}

	if (bRet && bRenderSnow && pSnowActive)
	{
		CSceneSnow* pEffSnow = (CSceneSnow*)PostEffectMgr()->GetEffect(ePFX_SceneSnow);
		if (!pEffSnow)
			return false;

		pEffSnow->m_RainVolParams = rainVolParams;
		pEffSnow->m_SnowVolParams = snowVolParams;
		pSnowActive->SetParam(1);
	}

	return bRet;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

bool CD3D9Renderer::FX_DeferredRainGBuffer()
{
	const SRainParams& rainVolParams = m_p3DEngineCommon.m_RainInfo;
	CEffectParam* pParam = PostEffectMgr()->GetByName("SceneRain_Active");
	if (pParam == 0 || pParam->GetParam() < 0.5f
	    || rainVolParams.fCurrentAmount < 0.05f
	    || rainVolParams.fRadius < 0.05f)
		return false;

	PROFILE_LABEL_SCOPE("DEFERRED_RAIN_GBUFFER");

	static CCryNameTSCRC tech("DeferredRainGBuffer");
	static CCryNameR puddleParamName0("g_RainPuddleParams0");
	static CCryNameR puddleParamName1("g_RainPuddleParams1");
	static CCryNameR volumeParamName("g_RainVolumeParams");
	static CCryNameR colorMulParamName("g_RainColorMultipliers");
	static CCryNameR wvpParamName("g_WorldViewPos");
	static CCryNameR occTransMatParamName("g_RainOcc_TransMat");
	static CCryNameR windParamName("g_RainOcc_WindOffs");

	CShader* pShader = CShaderMan::s_ShaderDeferredRain;
	m_cEF.mfRefreshSystemShader("DeferredRain", pShader);

	const CRenderCamera& cCam = gcpRendD3D->m_RP.m_TI[m_RP.m_nProcessThreadID].m_rcam;

	CTexture* pSceneSpecular = CTexture::s_ptexSceneSpecular;
#if defined(DURANGO_USE_ESRAM)
	pSceneSpecular = CTexture::s_ptexSceneSpecularESRAM;
#endif

	// TODO: Try avoiding the copy by directly accessing UAVs
	PostProcessUtils().StretchRect(CTexture::s_ptexSceneNormalsMap, CTexture::s_ptexStereoL);
	PostProcessUtils().StretchRect(pSceneSpecular, CTexture::s_ptexStereoR);
	PostProcessUtils().StretchRect(CTexture::s_ptexSceneDiffuse, CTexture::s_ptexSceneNormalsBent);

	// Prepare for reading from stencil in shader
	const bool bMSAA = m_RP.m_MSAAData.Type ? true : false;
	SResourceView::KeyType nBindResourceMsaa = gcpRendD3D->m_RP.m_MSAAData.Type ? SResourceView::DefaultViewMS : SResourceView::DefaultView;
	CTexture* pDepthBufferRT = CTexture::s_ptexZTarget;
	D3DShaderResource* pZTargetOrigSRV;
	SDepthTexture sBackup = FX_ReplaceMSAADepthBuffer(pDepthBufferRT, bMSAA, pZTargetOrigSRV);

	FX_PushRenderTarget(0, CTexture::s_ptexSceneNormalsMap, &m_DepthBufferOrigMSAA);
	FX_PushRenderTarget(1, pSceneSpecular, NULL);
	FX_PushRenderTarget(2, CTexture::s_ptexSceneDiffuse, NULL);

	uint64 nFlagsShaderRTSave = m_RP.m_FlagsShader_RT;
	m_RP.m_FlagsShader_RT &= ~(g_HWSR_MaskBit[HWSR_SAMPLE0]);

	if (rainVolParams.bApplyOcclusion)
	{
		m_RP.m_FlagsShader_RT |= g_HWSR_MaskBit[HWSR_SAMPLE0]; // Occlusion
	}
	if (rainVolParams.fSplashesAmount > 0.001f && rainVolParams.fRainDropsAmount > 0.001f)
	{
		m_RP.m_FlagsShader_RT |= g_HWSR_MaskBit[HWSR_SAMPLE1];  // Splashes
	}

	SD3DPostEffectsUtils::ShBeginPass(pShader, tech, FEF_DONTSETSTATES);
	FX_SetState(GS_NODEPTHTEST);

	float fMaxZ = -1.f;
	if (CV_r_rain_maxviewdist_deferred > cCam.fNear)
		fMaxZ = (cCam.fFar - (cCam.fNear * cCam.fFar) / CV_r_rain_maxviewdist_deferred) / (cCam.fFar - cCam.fNear);

	// Global wind params
	Vec3 windVec = gEnv->p3DEngine->GetGlobalWind(false);

	// Animated puddles
	float fTime = m_RP.m_TI[m_RP.m_nProcessThreadID].m_RealTime * 0.333f;
	const float puddleWindScale = -0.15f;
	float puddleOffsX = fTime * puddleWindScale * windVec.x;
	float puddleOffsY = fTime * puddleWindScale * windVec.y;

	Vec4 vPuddleParams0 = Vec4(puddleOffsX, puddleOffsY, rainVolParams.fPuddlesAmount * rainVolParams.fCurrentAmount, rainVolParams.fDiffuseDarkening);
	pShader->FXSetPSFloat(puddleParamName0, &vPuddleParams0, 1);

	float invPuddleMask = clamp_tpl(1.0f - rainVolParams.fPuddlesMaskAmount, 0.0f, 1.0f);
	Vec4 vPuddleParams1 = Vec4(invPuddleMask, rainVolParams.fPuddlesRippleAmount, rainVolParams.fSplashesAmount, 0.0f);
	pShader->FXSetPSFloat(puddleParamName1, &vPuddleParams1, 1);

	// Volume
	Vec4 vRainPosCS = Vec4(rainVolParams.vWorldPos, 1.f / max(rainVolParams.fRadius, 1e-3f));
	pShader->FXSetPSFloat(volumeParamName, &vRainPosCS, 1);

	// Global colour multiplier
	float fAmount = rainVolParams.fCurrentAmount * CV_r_rainamount;
	Vec4 vRainColorMultipliers = Vec4(rainVolParams.vColor, 1.f) * fAmount;
	vRainColorMultipliers.w = fMaxZ > 0.f ? CV_r_rain_maxviewdist_deferred / cCam.fFar : 1.f;
	vRainColorMultipliers.w = -10.f / vRainColorMultipliers.w;
	pShader->FXSetPSFloat(colorMulParamName, &vRainColorMultipliers, 1);

	// Camera position
	const Vec3& vCamPos = cCam.vOrigin;
	Vec4 pCamPosParam = Vec4(vCamPos, 0.f);
	pShader->FXSetPSFloat(wvpParamName, &pCamPosParam, 1);

	if (rainVolParams.bApplyOcclusion)
	{
		// Occlusion buffer matrix
		pShader->FXSetPSFloat(occTransMatParamName, (Vec4*)rainVolParams.matOccTransRender.GetData(), 4);

		// Pre-calculate wind-driven occlusion sample offset
		const float windOffsetScale = 15.f / (float)RAIN_OCC_MAP_SIZE;
		windVec = rainVolParams.matOccTransRender.TransformVector(windVec);
		windVec.x *= windOffsetScale;
		windVec.y *= windOffsetScale;

		Vec4 pWindParams(windVec.x, windVec.y, 0.f, 0.f);
		pShader->FXSetPSFloat(windParamName, &pWindParams, 1);
	}

	SPostEffectsUtils::SetTexture(CTexture::s_ptexStereoL, 9, FILTER_POINT, 0);
	SPostEffectsUtils::SetTexture(CTexture::s_ptexStereoR, 10, FILTER_POINT, 0);
	SPostEffectsUtils::SetTexture(CTexture::s_ptexSceneNormalsBent, 11, FILTER_POINT, 0);

	// Bind stencil buffer
	pDepthBufferRT->SetShaderResourceView(gcpRendD3D->m_DepthBufferOrigMSAA.pTexture->GetDeviceStencilReadOnlySRV(0, -1, bMSAA), bMSAA);
	pDepthBufferRT->Apply(12, CTexture::GetTexState(STexState(FILTER_POINT, true)), EFTT_UNKNOWN, -1, nBindResourceMsaa);

	SD3DPostEffectsUtils::DrawFullScreenTriWPOS(CTexture::s_ptexSceneNormalsMap->GetWidth(), CTexture::s_ptexSceneNormalsMap->GetHeight());
	SD3DPostEffectsUtils::ShEndPass();

	// Restore original DSV/SRV
	FX_RestoreMSAADepthBuffer(sBackup, pDepthBufferRT, bMSAA, pZTargetOrigSRV);

	FX_PopRenderTarget(0);
	FX_PopRenderTarget(1);
	FX_PopRenderTarget(2);

	// Set persistent Rain Ripples Flag for Water Volumes and Ocean Ripple effect
	m_RP.m_PersFlags2 |= RBPF2_RAINRIPPLES;

	m_RP.m_FlagsShader_RT = nFlagsShaderRTSave;

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

bool CD3D9Renderer::FX_DeferredSnowLayer()
{
	const SSnowParams& snowVolParams = m_p3DEngineCommon.m_SnowInfo;
	const SRainParams& rainVolParams = m_p3DEngineCommon.m_RainInfo;
	CShader* pShader = CShaderMan::s_ShaderDeferredSnow;
	const CRenderCamera& cCam = gcpRendD3D->m_RP.m_TI[m_RP.m_nProcessThreadID].m_rcam;

	if ((CRenderer::CV_r_snow < 1) || (snowVolParams.m_fSnowAmount < 0.05f && snowVolParams.m_fFrostAmount < 0.05f && snowVolParams.m_fSurfaceFreezing < 0.05f) || snowVolParams.m_fRadius < 0.05f)
		return false;

	PROFILE_LABEL_SCOPE("DEFERRED_SNOW_ACCUMULATION");

	// TODO: Try avoiding the copy by directly accessing UAVs
	PostProcessUtils().StretchRect(CTexture::s_ptexSceneDiffuse, CTexture::s_ptexStereoL);
	PostProcessUtils().StretchRect(CTexture::s_ptexSceneNormalsMap, CTexture::s_ptexBackBuffer);
	PostProcessUtils().StretchRect(CTexture::s_ptexSceneSpecular, CTexture::s_ptexSceneNormalsBent);

	gcpRendD3D->FX_PushRenderTarget(0, CTexture::s_ptexSceneDiffuse, &gcpRendD3D->m_DepthBufferOrigMSAA);
	gcpRendD3D->FX_PushRenderTarget(1, CTexture::s_ptexSceneNormalsMap, NULL);
	gcpRendD3D->FX_PushRenderTarget(2, CTexture::s_ptexSceneSpecular, NULL);

	if (CRenderer::CV_r_snow_displacement)
		gcpRendD3D->FX_PushRenderTarget(3, CTexture::s_ptexStereoR, NULL);

	uint64 nFlagsShaderRTSave = m_RP.m_FlagsShader_RT;

	m_RP.m_FlagsShader_RT &= ~g_HWSR_MaskBit[HWSR_SAMPLE0] | g_HWSR_MaskBit[HWSR_SAMPLE1] | g_HWSR_MaskBit[HWSR_SAMPLE2] | g_HWSR_MaskBit[HWSR_SAMPLE3];
	if (IsHDRModeEnabled())
		m_RP.m_FlagsShader_RT |= g_HWSR_MaskBit[HWSR_HDR_MODE];
	if (rainVolParams.bApplyOcclusion)
		m_RP.m_FlagsShader_RT |= g_HWSR_MaskBit[HWSR_SAMPLE0];

	static CCryNameTSCRC pTechName = "Snow";
	SD3DPostEffectsUtils::ShBeginPass(pShader, pTechName, FEF_DONTSETSTATES);
	FX_SetState(GS_NODEPTHTEST);

	// Textures
	STexState sPointTexState = STexState(FILTER_POINT, true);
	const int pointTexState = CTexture::GetTexState(sPointTexState);

	CTexture::s_ptexStereoL->Apply(0, pointTexState);
	CTexture::s_ptexBackBuffer->Apply(1, pointTexState);
	CTexture::s_ptexSceneNormalsBent->Apply(2, pointTexState);

	static CCryNameR paramName("g_SnowVolumeParams");
	const Vec3& vCamPos = cCam.vOrigin;
	const Vec4 vSnowPosCS = Vec4(snowVolParams.m_vWorldPos, 1.f / max(snowVolParams.m_fRadius, 1e-3f));
	pShader->FXSetPSFloat(paramName, &vSnowPosCS, 1);

	static CCryNameR param1Name("g_SnowMultipliers");
	float fSnowAmount = snowVolParams.m_fSnowAmount;
	float fFrostAmount = snowVolParams.m_fFrostAmount;
	float fSurfaceFreezing = snowVolParams.m_fSurfaceFreezing;
	Vec4 vSnowMultipliers(fSnowAmount, fFrostAmount, clamp_tpl(fSurfaceFreezing, 0.0f, 1.0f), 0);
	pShader->FXSetPSFloat(param1Name, &vSnowMultipliers, 1);

	static CCryNameR param2Name("g_WorldViewPos");
	Vec4 pCamPosParam = Vec4(vCamPos, 1);
	pShader->FXSetPSFloat(param2Name, &pCamPosParam, 1);

	// Sample wind at camera position
	AABB box;
	box.min = box.max = gcpRendD3D->m_RP.m_TI[gcpRendD3D->m_RP.m_nProcessThreadID].m_rcam.vOrigin;
	Vec3 windVec = gEnv->p3DEngine->GetWind(box, false);

	Vec3 windVecOcc = gEnv->p3DEngine->GetGlobalWind(false);
	if (rainVolParams.bApplyOcclusion)
	{
		static CCryNameR param3Name("g_SnowOcc_TransMat");
		pShader->FXSetPSFloat(param3Name, (Vec4*)rainVolParams.matOccTransRender.GetData(), 3);

		// Pre-calculate wind-driven occlusion sample offset
		const float windOffsetScale = 15.f / (float)RAIN_OCC_MAP_SIZE;
		windVecOcc = rainVolParams.matOccTransRender.TransformVector(windVec);
		windVecOcc.x *= windOffsetScale;
		windVecOcc.y *= windOffsetScale;

		static CCryNameR param4Name("g_SnowOcc_WindOffs");
		Vec4 pWindParamsOcc(windVecOcc.x, windVecOcc.y, 0, 0);
		pShader->FXSetPSFloat(param4Name, &pWindParamsOcc, 1);
	}

	static CCryNameR param4Name("g_WindDirection");
	Vec4 pWindParams(windVec.x, windVec.y, windVecOcc.x, windVecOcc.y);
	pShader->FXSetPSFloat(param4Name, &pWindParams, 1);

	short sX, sY, sWidth, sHeight;
	CDeferredShading::Instance().GetScissors(snowVolParams.m_vWorldPos, snowVolParams.m_fRadius, sX, sY, sWidth, sHeight);
	gcpRendD3D->EF_Scissor(true, sX, sY, sWidth, sHeight);

	// Render state
	uint32 renderState = GS_STENCIL;

	gcpRendD3D->FX_SetStencilState(STENC_FUNC(FSS_STENCFUNC_EQUAL) |
	                               STENCOP_FAIL(FSS_STENCOP_KEEP) |
	                               STENCOP_ZFAIL(FSS_STENCOP_KEEP) |
	                               STENCOP_PASS(FSS_STENCOP_KEEP),
	                               BIT_STENCIL_RESERVED, BIT_STENCIL_RESERVED, 0xFFFFFFFF, true);

	gcpRendD3D->FX_SetState(renderState);
	gcpRendD3D->FX_Commit();

	SD3DPostEffectsUtils::DrawFullScreenTriWPOS(CTexture::s_ptexBackBuffer->GetWidth(), CTexture::s_ptexBackBuffer->GetHeight());
	SD3DPostEffectsUtils::ShEndPass();

	// Restore targets
	gcpRendD3D->FX_PopRenderTarget(0);
	gcpRendD3D->FX_PopRenderTarget(1);
	gcpRendD3D->FX_PopRenderTarget(2);

	if (CRenderer::CV_r_snow_displacement)
		gcpRendD3D->FX_PopRenderTarget(3);

	// Restore state
	EF_Scissor(false, 0, 0, 0, 0);
	gcpRendD3D->m_RP.m_FlagsShader_RT = nFlagsShaderRTSave;
	gcpRendD3D->FX_Commit();

	return true;
}

bool CD3D9Renderer::FX_DeferredSnowDisplacement()
{
	const SSnowParams& snowVolParams = m_p3DEngineCommon.m_SnowInfo;
	const SRainParams& rainVolParams = m_p3DEngineCommon.m_RainInfo;
	CShader* pShader = CShaderMan::s_ShaderDeferredSnow;

	if ((CRenderer::CV_r_snow < 1 || CRenderer::CV_r_snow_displacement < 1) || snowVolParams.m_fSnowAmount < 0.05f || snowVolParams.m_fRadius < 0.05f)
		return false;

	PROFILE_LABEL_SCOPE("DEFERRED_SNOW_DISPLACEMENT");

	static CCryNameR param5Name("g_CameraMatrix");
	Matrix44A matView;
	matView = m_RP.m_TI[m_RP.m_nProcessThreadID].m_cam.GetViewMatrix();

	// Adjust the camera matrix so that the camera space will be: +y = down, +z - towards, +x - right
	Vec3 zAxis = matView.GetRow(1);
	matView.SetRow(1, -matView.GetRow(2));
	matView.SetRow(2, zAxis);
	float z = matView.m13;
	matView.m13 = -matView.m23;
	matView.m23 = z;

	short sX, sY, sWidth, sHeight;
	CDeferredShading::Instance().GetScissors(snowVolParams.m_vWorldPos, snowVolParams.m_fRadius, sX, sY, sWidth, sHeight);
	EF_Scissor(true, sX, sY, sWidth, sHeight);

	gcpRendD3D->Set2DMode(true, 1, 1);

	// Render state
	uint32 renderState = GS_NODEPTHTEST | GS_STENCIL;

	gcpRendD3D->FX_SetStencilState(STENC_FUNC(FSS_STENCFUNC_EQUAL) |
	                               STENCOP_FAIL(FSS_STENCOP_KEEP) |
	                               STENCOP_ZFAIL(FSS_STENCOP_KEEP) |
	                               STENCOP_PASS(FSS_STENCOP_KEEP),
	                               BIT_STENCIL_RESERVED, BIT_STENCIL_RESERVED, 0xFFFFFFFF, true);

	gcpRendD3D->FX_SetState(renderState);
	gcpRendD3D->FX_Commit();

	{
		PROFILE_LABEL_SCOPE("GENERATE_HEIGHT_MAP");
		static CCryNameTSCRC pTechNamePrepass = "ParallaxMapPrepass";
		SD3DPostEffectsUtils::ShBeginPass(pShader, pTechNamePrepass, FEF_DONTSETSTATES);

		FX_PushRenderTarget(0, CTexture::s_ptexBackBuffer, NULL);

		pShader->FXSetPSFloat(param5Name, (Vec4*)matView.GetData(), 3);

		PostProcessUtils().SetTexture(CTexture::s_ptexStereoR, 0, FILTER_POINT);

		SD3DPostEffectsUtils::DrawFullScreenTri(CTexture::s_ptexBackBuffer->GetWidth(), CTexture::s_ptexBackBuffer->GetHeight());

		SD3DPostEffectsUtils::ShEndPass();

		FX_PopRenderTarget(0);
	}

	{
		static CCryNameTSCRC pTechNameMin = "ParallaxMapMin";
		SD3DPostEffectsUtils::ShBeginPass(pShader, pTechNameMin, FEF_DONTSETSTATES | FEF_DONTSETTEXTURES);

		FX_PushRenderTarget(0, CTexture::s_ptexSceneDiffuseAccMap, NULL);

		PostProcessUtils().SetTexture(CTexture::s_ptexBackBuffer, 0, FILTER_POINT);

		SD3DPostEffectsUtils::DrawFullScreenTri(CTexture::s_ptexSceneDiffuseAccMap->GetWidth(), CTexture::s_ptexSceneDiffuseAccMap->GetHeight());

		SD3DPostEffectsUtils::ShEndPass();

		FX_PopRenderTarget(0);
	}

	// Copy screen to texture for displacement.
	FX_ScreenStretchRect(CTexture::s_ptexHDRTarget);

	// Iteratively apply displacement to maximize quality and minimize sample count.
	{
		PROFILE_LABEL_SCOPE("APPLY_DISPLACEMENT");
		static CCryNameTSCRC pTechNameApply = "ParallaxMapApply";
		static CCryNameR pPassParamsName("g_DisplacementParams");
		Vec4 pPassParams(0, 0, 0, 0);

		uint64 nFlagsShaderRTSave = m_RP.m_FlagsShader_RT;
		m_RP.m_FlagsShader_RT &= ~g_HWSR_MaskBit[HWSR_SAMPLE0];

		// First pass.
		FX_PushRenderTarget(0, CTexture::s_ptexSceneTarget, NULL);
		FX_PushRenderTarget(1, CTexture::s_ptexSceneSpecularAccMap, NULL);

		SD3DPostEffectsUtils::ShBeginPass(pShader, pTechNameApply, FEF_DONTSETSTATES);

		PostProcessUtils().SetTexture(CTexture::s_ptexHDRTarget, 0, FILTER_LINEAR);
		PostProcessUtils().SetTexture(CTexture::s_ptexSceneDiffuseAccMap, 1, FILTER_LINEAR);
		pPassParams.x = 1.0f;
		pShader->FXSetPSFloat(pPassParamsName, &pPassParams, 1);

		SD3DPostEffectsUtils::DrawFullScreenTri(CTexture::s_ptexSceneTarget->GetWidth(), CTexture::s_ptexSceneTarget->GetHeight());
		SD3DPostEffectsUtils::ShEndPass();

		FX_PopRenderTarget(0);
		FX_PopRenderTarget(1);

		// Second pass.
		FX_PushRenderTarget(0, CTexture::s_ptexHDRTarget, NULL);
		FX_PushRenderTarget(1, CTexture::s_ptexSceneDiffuseAccMap, NULL);

		SD3DPostEffectsUtils::ShBeginPass(pShader, pTechNameApply, FEF_DONTSETSTATES);

		PostProcessUtils().SetTexture(CTexture::s_ptexSceneTarget, 0, FILTER_LINEAR);
		PostProcessUtils().SetTexture(CTexture::s_ptexSceneSpecularAccMap, 1, FILTER_LINEAR);
		pPassParams.x = 0.5f;
		pShader->FXSetPSFloat(pPassParamsName, &pPassParams, 1);

		SD3DPostEffectsUtils::DrawFullScreenTri(CTexture::s_ptexSceneTarget->GetWidth(), CTexture::s_ptexSceneTarget->GetHeight());
		SD3DPostEffectsUtils::ShEndPass();

		FX_PopRenderTarget(0);
		FX_PopRenderTarget(1);

		// Third pass.
		m_RP.m_FlagsShader_RT |= g_HWSR_MaskBit[HWSR_SAMPLE0];

		FX_PushRenderTarget(0, CTexture::s_ptexSceneTarget, NULL);
		FX_PushRenderTarget(1, CTexture::s_ptexZTarget, NULL);

		SD3DPostEffectsUtils::ShBeginPass(pShader, pTechNameApply, FEF_DONTSETSTATES);
		FX_SetState(GS_NODEPTHTEST);

		PostProcessUtils().SetTexture(CTexture::s_ptexHDRTarget, 0, FILTER_LINEAR);
		PostProcessUtils().SetTexture(CTexture::s_ptexSceneDiffuseAccMap, 1, FILTER_LINEAR);
		pPassParams.x = 0.25f;
		pShader->FXSetPSFloat(pPassParamsName, &pPassParams, 1);

		SD3DPostEffectsUtils::DrawFullScreenTri(CTexture::s_ptexSceneTarget->GetWidth(), CTexture::s_ptexSceneTarget->GetHeight());
		SD3DPostEffectsUtils::ShEndPass();

		FX_PopRenderTarget(0);
		FX_PopRenderTarget(1);

		gcpRendD3D->m_RP.m_FlagsShader_RT = nFlagsShaderRTSave;
	}

	PostProcessUtils().CopyTextureToScreen(CTexture::s_ptexSceneTarget);

	EF_Scissor(false, 0, 0, 0, 0);

	FX_Commit();

	FX_ResetPipe();

	gcpRendD3D->Set2DMode(false, 1, 1);

	return true;
}
