// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#include "StdAfx.h"
#include "MotionBlur.h"
#include "DriverD3D.h"
#include "D3DPostProcess.h"

void CMotionBlurStage::Init()
{
	m_samplerPoint = CTexture::GetTexState(STexState(FILTER_POINT, true));
	m_samplerLinear = CTexture::GetTexState(STexState(FILTER_LINEAR, true));
}

float CMotionBlurStage::ComputeMotionScale()
{
	// The length of the generated motion vectors is proportional to the current time step, so we need
	// to rescale the motion vectors to simulate a constant camera exposure time

	float exposureTime = 1.0f / std::max(CRenderer::CV_r_MotionBlurShutterSpeed, 1e-6f);
	float timeStep = std::max(gEnv->pTimer->GetFrameTime(), 1e-6f);

	exposureTime *= gEnv->pTimer->GetTimeScale();

	return exposureTime / timeStep;
}

void CMotionBlurStage::Execute()
{
	if (!CRenderer::CV_r_MotionBlur || gRenDev->m_nDisableTemporalEffects)
		return;

	PROFILE_LABEL_SCOPE("MB");

	CD3D9Renderer* rd = gcpRendD3D;
	CShader* pShader = CShaderMan::s_shPostMotionBlur;

	int vpX, vpY, vpWidth, vpHeight;
	rd->GetViewport(&vpX, &vpY, &vpWidth, &vpHeight);

	// Check if DOF is enabled
	CDepthOfField* pDofRenderTech = (CDepthOfField*)PostEffectMgr()->GetEffect(ePFX_eDepthOfField);
	SDepthOfFieldParams dofParams = pDofRenderTech->GetParams();
	dofParams.vFocus.w *= 2.0f;
	const bool bGatherDofEnabled = CRenderer::CV_r_dof > 0 && CRenderer::CV_r_DofMode == 1 && dofParams.vFocus.w > 0.0001f;

	Matrix44A mViewProjPrev = CMotionBlur::GetPrevView();
	Matrix44 mViewProj = GetUtils().m_pView;
	mViewProjPrev = mViewProjPrev * GetUtils().m_pProj * GetUtils().m_pScaleBias;
	mViewProjPrev.Transpose();

	float tileCountX = (float)CTexture::s_ptexVelocityTiles[1]->GetWidth();
	float tileCountY = (float)CTexture::s_ptexVelocityTiles[1]->GetHeight();

	static CCryNameR motionBlurParamName("vMotionBlurParams");

	{
		PROFILE_LABEL_SCOPE("PACK VELOCITY");

		CMotionBlur* pMB = (CMotionBlur*)PostEffectMgr()->GetEffect(ePFX_eMotionBlur);
		const float maxRange = 32.0f;
		const float amount = clamp_tpl<float>(pMB->m_pRadBlurAmount->GetParam() / maxRange, 0.0f, 1.0f);
		const float radius = 1.0f / clamp_tpl<float>(pMB->m_pRadBlurRadius->GetParam(), 1e-6f, 2.0f);
		const Vec4 blurDir = pMB->m_pDirectionalBlurVec->GetParamVec4();
		const Vec4 dirBlurParam = Vec4(blurDir.x * (maxRange / (float)vpWidth), blurDir.y * (maxRange / (float)vpHeight), (float)vpWidth / (float)vpHeight, 1.0f);
		const Vec4 radBlurParam = Vec4(pMB->m_pRadBlurScreenPosX->GetParam() * dirBlurParam.z, pMB->m_pRadBlurScreenPosY->GetParam(), radius * amount, amount);

		const bool bRadialBlur = amount + (blurDir.x * blurDir.x) + (blurDir.y * blurDir.y) > 1.0f / (float)vpWidth;

		if (m_passPacking.InputChanged((int)bRadialBlur, GetUtils().GetVelocityObjectRT()->GetTextureID()))
		{
			static CCryNameTSCRC techPackVelocities("PackVelocities");
			m_passPacking.SetTechnique(pShader, techPackVelocities, bRadialBlur ? g_HWSR_MaskBit[HWSR_SAMPLE0] : 0);
			m_passPacking.SetRenderTarget(0, CTexture::s_ptexVelocity);
			m_passPacking.SetState(GS_NODEPTHTEST);
			m_passPacking.SetTextureSamplerPair(0, CTexture::s_ptexZTarget, m_samplerPoint);
			m_passPacking.SetTextureSamplerPair(1, CTexture::s_ptexHDRTarget, m_samplerPoint);
			m_passPacking.SetTextureSamplerPair(2, GetUtils().GetVelocityObjectRT(), m_samplerPoint);
			m_passPacking.SetRequireWorldPos(true);
		}

		static CCryNameR viewProjPrevName("mViewProjPrev");
		static CCryNameR dirBlurName("vDirectionalBlur");
		static CCryNameR radBlurName("vRadBlurParam");

		m_passPacking.BeginConstantUpdate();

		mViewProjPrev.Transpose();
		pShader->FXSetPSFloat(viewProjPrevName, (Vec4*)mViewProjPrev.GetData(), 4);
		pShader->FXSetPSFloat(dirBlurName, &dirBlurParam, 1);
		pShader->FXSetPSFloat(radBlurName, &radBlurParam, 1);
		const Vec4 motionBlurParams = Vec4(ComputeMotionScale(), 1.0f / tileCountX, 1.0f / tileCountX * CRenderer::CV_r_MotionBlurCameraMotionScale, 0);
		pShader->FXSetPSFloat(motionBlurParamName, &motionBlurParams, 1);

		m_passPacking.Execute();

		// Render object velocities (TODO)
		//rd->GetGraphicsPipeline().ResetLegacyRenderState();
		//rd->m_RP.m_pRenderFunc = rd->FX_FlushShader_General;
		//rd->FX_PushRenderTarget(0, pVelocityRT, NULL);
		//pMB->RenderObjectsVelocity();
		//rd->FX_PopRenderTarget(0);
	}

	{
		PROFILE_LABEL_SCOPE("VELOCITY TILES");

		static CCryNameTSCRC techVelocityTileGen("VelocityTileGen");
		static CCryNameTSCRC techTileNeighborhood("VelocityTileNeighborhood");

		// Tile generation first pass
		{
			if (m_passTileGen1.InputChanged())
			{
				m_passTileGen1.SetTechnique(pShader, techVelocityTileGen, 0);
				m_passTileGen1.SetRenderTarget(0, CTexture::s_ptexVelocityTiles[0]);
				m_passTileGen1.SetState(GS_NODEPTHTEST);
				m_passTileGen1.SetTextureSamplerPair(0, CTexture::s_ptexVelocity, m_samplerPoint);
			}

			m_passTileGen1.BeginConstantUpdate();

			Vec4 params = Vec4((float)CTexture::s_ptexVelocity->GetWidth(), (float)CTexture::s_ptexVelocity->GetHeight(), ceilf((float)gcpRendD3D->GetWidth() / tileCountX), 0);
			pShader->FXSetPSFloat(motionBlurParamName, &params, 1);

			m_passTileGen1.Execute();
		}

		// Tile generation second pass
		{
			if (m_passTileGen2.InputChanged())
			{
				m_passTileGen2.SetTechnique(pShader, techVelocityTileGen, 0);
				m_passTileGen2.SetRenderTarget(0, CTexture::s_ptexVelocityTiles[1]);
				m_passTileGen2.SetState(GS_NODEPTHTEST);
				m_passTileGen2.SetTextureSamplerPair(0, CTexture::s_ptexVelocityTiles[0], m_samplerPoint);
			}

			m_passTileGen2.BeginConstantUpdate();

			Vec4 params = Vec4((float)CTexture::s_ptexVelocityTiles[0]->GetWidth(), (float)CTexture::s_ptexVelocityTiles[0]->GetHeight(), ceilf((float)gcpRendD3D->GetHeight() / tileCountY), 1);
			pShader->FXSetPSFloat(motionBlurParamName, &params, 1);

			m_passTileGen2.Execute();
		}

		// Neighborhood max
		{
			if (m_passNeighborMax.InputChanged())
			{
				m_passNeighborMax.SetTechnique(pShader, techTileNeighborhood, 0);
				m_passNeighborMax.SetRenderTarget(0, CTexture::s_ptexVelocityTiles[2]);
				m_passNeighborMax.SetState(GS_NODEPTHTEST);
				m_passNeighborMax.SetTextureSamplerPair(0, CTexture::s_ptexVelocityTiles[1], m_samplerPoint);
			}

			m_passNeighborMax.BeginConstantUpdate();

			Vec4 params = Vec4(1.0f / tileCountX, 1.0f / tileCountY, 0, 0);
			pShader->FXSetPSFloat(motionBlurParamName, &params, 1);

			m_passNeighborMax.Execute();
		}
	}

	{
		PROFILE_LABEL_SCOPE("MOTION VECTOR APPLY");

		if (bGatherDofEnabled)
		{
			m_passCopy.Execute(CTexture::s_ptexHDRTarget, CTexture::s_ptexSceneTargetR11G11B10F[0]);
		}

		if (m_passMotionBlur.InputChanged(CRenderer::CV_r_MotionBlurQuality, (int)bGatherDofEnabled))
		{
			uint64 rtMask = 0;
			rtMask |= (CRenderer::CV_r_MotionBlurQuality >= 2) ? g_HWSR_MaskBit[HWSR_SAMPLE2] : 0;
			rtMask |= (CRenderer::CV_r_MotionBlurQuality == 1) ? g_HWSR_MaskBit[HWSR_SAMPLE1] : 0;

			static CCryNameTSCRC techMotionBlur("MotionBlur");
			m_passMotionBlur.SetTechnique(pShader, techMotionBlur, rtMask);
			m_passMotionBlur.SetRenderTarget(0, CTexture::s_ptexHDRTarget);
			m_passMotionBlur.SetState(GS_NODEPTHTEST | GS_BLSRC_ONE | GS_BLDST_ONEMINUSSRCALPHA);
			m_passMotionBlur.SetTextureSamplerPair(0, bGatherDofEnabled ? CTexture::s_ptexSceneTargetR11G11B10F[0] : CTexture::s_ptexHDRTargetPrev, m_samplerLinear);
			m_passMotionBlur.SetTextureSamplerPair(1, CTexture::s_ptexVelocity, m_samplerPoint);
			m_passMotionBlur.SetTextureSamplerPair(2, CTexture::s_ptexVelocityTiles[2], m_samplerPoint);
		}

		m_passMotionBlur.BeginConstantUpdate();

		Vec4 params = Vec4(1.0f / tileCountX, 1.0f / tileCountY, 0, 0);
		pShader->FXSetPSFloat(motionBlurParamName, &params, 1);

		m_passMotionBlur.Execute();
	}
}
