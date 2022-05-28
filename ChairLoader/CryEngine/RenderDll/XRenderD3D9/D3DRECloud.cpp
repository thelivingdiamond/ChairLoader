// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

/*=============================================================================
   D3DRECloud.cpp : D3D specific volumetric clouds rendering.

   Revision history:
* Created by Honich Andrey

   =============================================================================*/

#include "StdAfx.h"

#include "DriverD3D.h"
#include <Cry3DEngine/I3DEngine.h>

//=======================================================================

extern CTexture* gTexture;

void CRECloud::IlluminateCloud(Vec3 vLightPos, Vec3 vObjPos, ColorF cLightColor, ColorF cAmbColor, bool bReset)
{
	CryFatalError("Not implemented on D3D11+");

	int iOldVP[4];

	CD3D9Renderer* rd = gcpRendD3D;
	int nShadeRes = 256;

	rd->GetViewport(&iOldVP[0], &iOldVP[1], &iOldVP[2], &iOldVP[3]);

	rd->EF_PushMatrix();
	rd->m_RP.m_TI[rd->m_RP.m_nProcessThreadID].m_matProj->Push();
	rd->RT_SetViewport(0, 0, nShadeRes, nShadeRes);

	Vec3 vDir = vLightPos;
	vDir.Normalize();

	if (bReset)
		m_lightDirections.clear();
	m_lightDirections.push_back(vDir);

	vLightPos *= (1.1f * m_boundingBox.GetRadius());
	vLightPos += m_boundingBox.GetCenter();

	CRenderCamera cam;

	Vec3 vUp(0, 0, 1);
	cam.LookAt(vLightPos, m_boundingBox.GetCenter(), vUp);

	SortParticles(cam.ViewDir(), vLightPos, eSort_AWAY);

	float DistToCntr = (m_boundingBox.GetCenter() - vLightPos) * cam.ViewDir();

	float fNearDist = DistToCntr - m_boundingBox.GetRadius();
	float fFarDist = DistToCntr + m_boundingBox.GetRadius();

	Matrix44A* m = rd->m_RP.m_TI[rd->m_RP.m_nProcessThreadID].m_matView->GetTop();
	cam.GetModelviewMatrix((float*)m);

	mathMatrixOrthoOffCenter((Matrix44A*)rd->m_RP.m_TI[rd->m_RP.m_nProcessThreadID].m_matProj->GetTop(), -m_boundingBox.GetRadius(), m_boundingBox.GetRadius(), -m_boundingBox.GetRadius(), m_boundingBox.GetRadius(), fNearDist, fFarDist);
	rd->EF_DirtyMatrix();

	rd->SetColorOp(eCO_MODULATE, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0);
	rd->FX_SetState(GS_BLSRC_ONE | GS_BLDST_ONEMINUSSRCALPHA | GS_NODEPTHTEST | GS_ALPHATEST_GREATER, 0);
	rd->D3DSetCull(eCULL_None);

	CRenderObject* pObj = rd->m_RP.m_pCurObject;
	CShader* pSH = rd->m_RP.m_pShader;
	SShaderTechnique* pSHT = rd->m_RP.m_pCurTechnique;
	SShaderPass* pPass = rd->m_RP.m_pCurPass;
	CShaderResources* pShRes = rd->m_RP.m_pShaderResources;

	if (pShRes && pShRes->m_Textures[EFTT_DIFFUSE])
	{
		m_pTexParticle = pShRes->m_Textures[EFTT_DIFFUSE]->m_Sampler.m_pTex;
	}

	m_pTexParticle->Apply(0);

	rd->FX_SetFPMode();
	HRESULT h = S_OK;

	assert(0);

	rd->EF_ClearTargetsLater(FRT_CLEAR, Clr_White, Clr_FarPlane.r, 0);
	rd->FX_Commit();

	float fPixelsPerLength = (float)nShadeRes / (2.0f * m_boundingBox.GetRadius());

	// the solid angle over which we will sample forward-scattered light.
	float fSolidAngle = 0.09f;
	int i;
	int iNumFailed = 0;
	int nParts = m_particles.size();
	for (i = 0; i < nParts; i++)
	{
		SCloudParticle* p = m_particles[i];
		Vec3 vParticlePos = p->GetPosition();

		Vec3 vOffset = vLightPos - vParticlePos;

		float fDistance = fabs(cam.ViewDir() * vOffset) - fNearDist;

		float fArea = fSolidAngle * fDistance * fDistance;
		int iPixelDim = (int)(sqrtf(fArea) * fPixelsPerLength);
		//iPixelDim = 1;
		int iNumPixels = iPixelDim * iPixelDim;
		if (iNumPixels < 1)
		{
			iNumPixels = 1;
			iPixelDim = 1;
		}

		// the scale factor to convert the read back pixel colors to an average illumination of the area.
		float fColorScaleFactor = fSolidAngle / (iNumPixels * 255.0f);

		unsigned char* ds = new unsigned char[4 * iNumPixels];
		unsigned char* c = ds;

		Vec3 vWinPos;

		// find the position in the buffer to which the particle position projects.
		rd->ProjectToScreen(vParticlePos.x, vParticlePos.y, vParticlePos.z, &vWinPos.x, &vWinPos.y, &vWinPos.z);
		vWinPos.x /= 100.0f / rd->m_NewViewport.nWidth;
		vWinPos.y /= 100.0f / rd->m_NewViewport.nHeight;

		// offset the projected window position by half the size of the readback region.
		vWinPos.x -= 0.5f * iPixelDim;
		if (vWinPos.x < 0)
			vWinPos.x = 0;
		vWinPos.y = (vWinPos.y - 0.5f * iPixelDim);
		if (vWinPos.y < 0)
			vWinPos.y = 0;

		assert(0);

		// scattering coefficient vector.
		//m_sfScatterFactor = m_sfAlbedo * 80 * (1.0f/(4.0f*(float)M_PI));
		ColorF vScatter = ColorF(m_sfScatterFactor, m_sfScatterFactor, m_sfScatterFactor, 1);

		// add up the read back pixels (only need one component -- its grayscale)
		int iSum = 0;
		int nTest = 0;
		for (int k = 0; k < 4 * iNumPixels; k += 4)
		{
			PREFAST_SUPPRESS_WARNING(6001) // ds/c is unintialized, all this code is unused, unmaintained and should be entirely removed
			iSum += c[k];
			nTest += 255;
		}
		delete[] c;

		ColorF vScatteredAmount = ColorF(iSum * fColorScaleFactor, iSum * fColorScaleFactor, iSum * fColorScaleFactor, 1 - m_sfTransparency);
		vScatteredAmount *= vScatter;

		ColorF vColor = vScatteredAmount;
		float fScat = (float)iSum / (float)nTest;
		fScat = powf(fScat, 3);
		//vColor = ColorF(fScat);

		vColor *= cLightColor;
		vColor.a = 1 - m_sfTransparency;

		//ColorF Amb = ColorF(0.1f, 0.1f, 0.1f, 1.0f);
		if (bReset)
		{
			p->SetBaseColor(cAmbColor);
			p->ClearLitColors();
			p->AddLitColor(vColor);
		}
		else
		{
			p->AddLitColor(vColor);
		}

		vScatteredAmount *= 1.5f;

		// clamp the color
		vScatteredAmount.clamp();
		vScatteredAmount.a = 1 - m_sfTransparency;

		Vec3 vPos = vParticlePos;
		Vec3 x = cam.vX * p->GetRadiusX();
		Vec3 y = cam.vY * p->GetRadiusY();
		rd->DrawQuad3D(vPos - y - x, vPos - y + x, vPos + y + x, vPos + y - x, vScatteredAmount, p->m_vUV[0].x, p->m_vUV[0].y, p->m_vUV[1].x, p->m_vUV[1].y);
	}

	rd->FX_PopRenderTarget(0);
	assert(0);
	rd->m_RP.m_pCurObject = pObj;
	rd->m_RP.m_pShader = pSH;
	rd->m_RP.m_pCurTechnique = pSHT;
	rd->m_RP.m_pCurPass = pPass;

	rd->PopMatrix();
	rd->m_RP.m_TI[rd->m_RP.m_nProcessThreadID].m_matProj->Pop();
	rd->EF_DirtyMatrix();
	rd->RT_SetViewport(iOldVP[0], iOldVP[1], iOldVP[2], iOldVP[3]);
}

void CRECloud::DisplayWithoutImpostor(const CRenderCamera& camera)
{
	CD3D9Renderer* rd = gcpRendD3D;
	assert(rd->m_pRT->IsRenderThread());

	int nThreadID = rd->m_RP.m_nProcessThreadID;

	// copy the current camera
	CRenderCamera cam(camera);

	Vec3 vUp(0, 0, 1);

	Vec3 vParticlePlane = cam.vX % cam.vY;
	Vec3 vParticleX = (vUp % vParticlePlane).GetNormalized();
	Vec3 vParticleY = (vParticleX % vParticlePlane).GetNormalized();

	// texture is rotated to get minimum texture occupation so we need compensate that
	//	Matrix33 mCamInv = Matrix33(cam.vX.GetNormalized(),cam.vY.GetNormalized(),vParticlePlane.GetNormalized());		mCamInv.Invert();

	//	Vec3 vParticleX = mCamInv.TransformVector(vParticleWSX);
	//	Vec3 vParticleY = mCamInv.TransformVector(vParticleWSY);

	//	Vec3 vParticleX = cam.vX;
	//	Vec3 vParticleY = cam.vY;

	float fCosAngleSinceLastSort = m_vLastSortViewDir * rd->GetRCamera().ViewDir();
	Vec3 vSkyColor = gEnv->p3DEngine->GetSkyColor();
	ColorF cSkyColor = ColorF(vSkyColor.x, vSkyColor.y, vSkyColor.z, 1);

	float fSquareDistanceSinceLastSort = (rd->GetRCamera().vOrigin - m_vLastSortCamPos).GetLengthSquared();

	if (fCosAngleSinceLastSort < m_sfSortAngleErrorTolerance || fSquareDistanceSinceLastSort > m_sfSortSquareDistanceTolerance)
	{
		Vec3 vSortPos = -cam.ViewDir();
		vSortPos *= (1.1f * m_boundingBox.GetRadius());

		// sort the particles from back to front wrt the camera position.
		SortParticles(cam.ViewDir(), vSortPos, eSort_TOWARD);

		m_vLastSortViewDir = rd->GetRCamera().ViewDir();
		m_vLastSortCamPos = rd->GetRCamera().vOrigin;
	}

	ColorF color;
	Vec3 eyeDir;

	int nParts = m_particles.size();
	int nStartPart = 0;

	if (nParts == 0)
		return;

	int nCurParts;

	CRenderObject* pObj = rd->m_RP.m_pCurObject;
	CShader* pSH = rd->m_RP.m_pShader;
	SShaderTechnique* pSHT = rd->m_RP.m_pCurTechnique;
	SShaderPass* pPass = rd->m_RP.m_pCurPass;
	CShaderResources* pShRes = rd->m_RP.m_pShaderResources;
	CREImposter* pRE = (CREImposter*)pObj->GetRE();
	Vec3 vPos = pRE->GetPosition();

	uint32 nPasses;
	if (IsRecursiveRenderView())
	{
		static CCryNameTSCRC techName("Cloud_Recursive");
		pSH->FXSetTechnique(techName);
	}
	else
	{
		static CCryNameTSCRC techName("Cloud");
		pSH->FXSetTechnique(techName);
	}
	pSH->FXBegin(&nPasses, FEF_DONTSETTEXTURES | FEF_DONTSETSTATES);

	if (pShRes && pShRes->m_Textures[EFTT_DIFFUSE])
	{
		m_pTexParticle = pShRes->m_Textures[EFTT_DIFFUSE]->m_Sampler.m_pTex;
	}
	pSH->FXBeginPass(0);
	m_pTexParticle->Apply(0);
	/*
	   // set depth texture for soft clipping of cloud particle against scene geometry
	   if(0 != CTexture::s_ptexZTarget)
	   {
	    STexState depthTextState( FILTER_POINT, true );
	    CTexture::s_ptexZTarget->Apply( 1, CTexture::GetTexState(depthTextState) );
	   }
	 */
	rd->FX_SetState(GS_BLSRC_ONE | GS_BLDST_ONEMINUSSRCALPHA | GS_NODEPTHTEST | GS_ALPHATEST_GREATER, 0);
	//rd->FX_SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA | GS_NODEPTHTEST | GS_ALPHATEST_GREATER, 0);
	rd->D3DSetCull(eCULL_None);

	if (IsRecursiveRenderView())
	{
		rd->m_cEF.m_RTRect = Vec4(0, 0, 1, 1);

		Vec4 vCloudColorScale(m_fCloudColorScale, 0, 0, 0);
		static CCryNameR g_CloudColorScaleName("g_CloudColorScale");
		pSH->FXSetPSFloat(g_CloudColorScaleName, &vCloudColorScale, 1);
	}

	rd->FX_Commit();

	if (!FAILED(rd->FX_SetVertexDeclaration(0, eVF_P3F_C4B_T2F)))
	{
		while (nStartPart < nParts)
		{
			nCurParts = nParts - nStartPart;
			if (nCurParts > 32768)
				nCurParts = 32768;

			TempDynVB<SVF_P3F_C4B_T2F> vb;
			vb.Allocate(nCurParts * 4);
			SVF_P3F_C4B_T2F* pDst = vb.Lock();

			TempDynIB16 ib;
			ib.Allocate(nCurParts * 6);
			uint16* pDstInds = ib.Lock();

			// get various run-time parameters to determine cloud shading
			Vec3 sunDir(gEnv->p3DEngine->GetSunDir().GetNormalized());

			float minHeight(m_boundingBox.GetMin().z);
			float totalHeight(m_boundingBox.GetMax().z - minHeight);

			ColorF cloudSpec, cloudDiff;
			GetIllumParams(cloudSpec, cloudDiff);

			I3DEngine* p3DEngine(gEnv->p3DEngine);
			assert(0 != p3DEngine);

			Vec3 cloudShadingMultipliers;
			p3DEngine->GetGlobalParameter(E3DPARAM_CLOUDSHADING_MULTIPLIERS, cloudShadingMultipliers);

			Vec3 brightColor(cloudShadingMultipliers.x * p3DEngine->GetSunColor().CompMul(Vec3(cloudSpec.r, cloudSpec.g, cloudSpec.b)));
			Vec3 darkColor(cloudShadingMultipliers.y * p3DEngine->GetSkyColor().CompMul(Vec3(cloudDiff.r, cloudDiff.g, cloudDiff.b)));

			Vec3 negCamFrontDir(-cam.ViewDir());

			m_fCloudColorScale = 1.0f;

			// compute m_fCloudColorScale for HDR rendering
			{
				if (brightColor.x > m_fCloudColorScale) m_fCloudColorScale = brightColor.x;
				if (brightColor.y > m_fCloudColorScale) m_fCloudColorScale = brightColor.y;
				if (brightColor.z > m_fCloudColorScale) m_fCloudColorScale = brightColor.z;

				if (darkColor.x > m_fCloudColorScale) m_fCloudColorScale = darkColor.x;
				if (darkColor.y > m_fCloudColorScale) m_fCloudColorScale = darkColor.y;
				if (darkColor.z > m_fCloudColorScale) m_fCloudColorScale = darkColor.z;

				// normalize color
				brightColor /= m_fCloudColorScale;
				darkColor /= m_fCloudColorScale;
			}

			// render cloud particles
			for (int i = 0; i < nCurParts; i++)
			{
				SCloudParticle* p = m_particles[i + nStartPart];

				// draw the particle as a textured billboard.
				int nInd = i * 4;
				SVF_P3F_C4B_T2F* pQuad = &pDst[nInd];
				Vec3 pos = p->GetPosition() * m_fScale + vPos;
				Vec3 x = vParticleX * p->GetRadiusX() * m_fScale;
				Vec3 y = vParticleY * p->GetRadiusY() * m_fScale;
				//ColorB cb = color;
				//uint32 cd = cb.pack_argb8888();

				// determine shade for each vertex of the billboard
				float f0 = sunDir.Dot(Vec3(-y - x).GetNormalized()) * 0.5f + 0.5f;
				float f1 = sunDir.Dot(Vec3(-y + x).GetNormalized()) * 0.5f + 0.5f;
				float f2 = sunDir.Dot(Vec3(y + x).GetNormalized()) * 0.5f + 0.5f;
				float f3 = sunDir.Dot(Vec3(y - x).GetNormalized()) * 0.5f + 0.5f;

				Vec3 eye0(cam.vOrigin - (pos - y - x));
				eye0 = (eye0.GetLengthSquared() < 1e-4f) ? negCamFrontDir : eye0.GetNormalized();
				Vec3 eye1(cam.vOrigin - (pos - y + x));
				eye1 = (eye1.GetLengthSquared() < 1e-4f) ? negCamFrontDir : eye1.GetNormalized();
				Vec3 eye2(cam.vOrigin - (pos + y + x));
				eye2 = (eye2.GetLengthSquared() < 1e-4f) ? negCamFrontDir : eye2.GetNormalized();
				Vec3 eye3(cam.vOrigin - (pos + y - x));
				eye3 = (eye3.GetLengthSquared() < 1e-4f) ? negCamFrontDir : eye3.GetNormalized();
				f0 *= sunDir.Dot(eye0) * 0.25f + 0.75f;
				f1 *= sunDir.Dot(eye1) * 0.25f + 0.75f;
				f2 *= sunDir.Dot(eye2) * 0.25f + 0.75f;
				f3 *= sunDir.Dot(eye3) * 0.25f + 0.75f;

				//float heightScaleTop( ( p->GetPosition().z + x.z + y.z - minHeight ) / totalHeight );
				//float heightScaleBottom( ( p->GetPosition().z - x.z - y.z - minHeight ) / totalHeight );
				float heightScaleTop(1);
				float heightScaleBottom(1);

				// compute finale shading values
				f0 = clamp_tpl(f0 * heightScaleBottom, 0.0f, 1.0f);
				f1 = clamp_tpl(f1 * heightScaleBottom, 0.0f, 1.0f);
				f2 = clamp_tpl(f2 * heightScaleTop, 0.0f, 1.0f);
				f3 = clamp_tpl(f3 * heightScaleTop, 0.0f, 1.0f);

				// blend between dark and bright cloud color based on shading value
				Vec3 c0(darkColor + f0 * (brightColor - darkColor));
				Vec3 c1(darkColor + f1 * (brightColor - darkColor));
				Vec3 c2(darkColor + f2 * (brightColor - darkColor));
				Vec3 c3(darkColor + f3 * (brightColor - darkColor));
				float transp(pRE->m_fCurTransparency);

				// write billboard vertices
				ColorF col0(c0.x, c0.y, c0.z, transp);
				col0.clamp();
				pQuad[0].xyz = pos - y - x;
				pQuad[0].color.dcolor = ColorB(col0).pack_argb8888();
				pQuad[0].st = Vec2(p->m_vUV[0].x, p->m_vUV[0].y);

				ColorF col1(c1.x, c1.y, c1.z, transp);
				col1.clamp();
				pQuad[1].xyz = pos - y + x;
				pQuad[1].color.dcolor = ColorB(col1).pack_argb8888();
				pQuad[1].st = Vec2(p->m_vUV[1].x, p->m_vUV[0].y);

				ColorF col2(c2.x, c2.y, c2.z, transp);
				col2.clamp();
				pQuad[2].xyz = pos + y + x;
				pQuad[2].color.dcolor = ColorB(col2).pack_argb8888();
				pQuad[2].st = Vec2(p->m_vUV[1].x, p->m_vUV[1].y);

				ColorF col3(c3.x, c3.y, c3.z, transp);
				col3.clamp();
				pQuad[3].xyz = pos + y - x;
				pQuad[3].color.dcolor = ColorB(col3).pack_argb8888();
				pQuad[3].st = Vec2(p->m_vUV[0].x, p->m_vUV[1].y);

				uint16* pInds = &pDstInds[i * 6];
				pInds[0] = nInd;
				pInds[1] = nInd + 1;
				pInds[2] = nInd + 2;

				pInds[3] = nInd;
				pInds[4] = nInd + 2;
				pInds[5] = nInd + 3;
			}

			vb.Unlock();
			vb.Bind(0);
			vb.Release();

			ib.Unlock();
			ib.Bind();
			ib.Release();

			rd->FX_DrawIndexedPrimitive(eptTriangleList, 0, 0, nCurParts * 4, 0, nCurParts * 6);

			nStartPart += nCurParts;
		}
	}

	rd->m_RP.m_pCurObject = pObj;
	rd->m_RP.m_pShader = pSH;
	rd->m_RP.m_pCurTechnique = pSHT;
	rd->m_RP.m_pCurPass = pPass;
}

bool CRECloud::GenerateCloudImposter(CShader* pShader, CShaderResources* pRes, CRenderObject* pObject)
{
	CD3D9Renderer* r = gcpRendD3D;
	r->FX_PreRender(1);
	r->m_RP.m_TI[r->m_RP.m_nProcessThreadID].m_PersFlags |= RBPF_DRAWTOTEXTURE;
	r->m_RP.m_pRE = NULL;
	r->m_RP.m_pShader = pShader;
	r->m_RP.m_pShaderResources = pRes;
	r->m_RP.m_pCurObject = pObject;
	r->m_RP.m_RendNumVerts = 0;
	r->m_RP.m_RendNumIndices = 0;
	mfPrepare(false);
	r->m_RP.m_TI[r->m_RP.m_nProcessThreadID].m_PersFlags &= ~RBPF_DRAWTOTEXTURE;
	r->FX_PostRender();

	return true;
}

bool CRECloud::UpdateImposter(CRenderObject* pObj)
{
	CD3D9Renderer* rd = gcpRendD3D;
	CREImposter* pRE = (CREImposter*)pObj->GetRE();

	if (!pRE->PrepareForUpdate())
	{
		if (!CRenderer::CV_r_cloudsupdatealways && pRE->m_nFrameReset == rd->m_nFrameReset)
			return true;
	}

	PROFILE_FRAME(Imposter_CloudUpdate);

	pRE->m_nFrameReset = rd->m_nFrameReset;
	int nLogX = pRE->m_nLogResolutionX;
	int nLogY = pRE->m_nLogResolutionY;
	int iResX = 1 << nLogX;
	int iResY = 1 << nLogY;
	while (iResX > CRenderer::CV_r_texatlassize)
	{
		nLogX--;
		iResX = 1 << nLogX;
	}
	while (iResY > CRenderer::CV_r_texatlassize)
	{
		nLogY--;
		iResY = 1 << nLogY;
	}

	int iOldVP[4];
	rd->GetViewport(&iOldVP[0], &iOldVP[1], &iOldVP[2], &iOldVP[3]);

	rd->EF_PushMatrix();
	rd->m_RP.m_TI[rd->m_RP.m_nProcessThreadID].m_matProj->Push();
	rd->RT_SetViewport(0, 0, iResX, iResY);
#ifndef _RELEASE
	rd->m_RP.m_PS[rd->m_RP.m_nProcessThreadID].m_NumCloudImpostersUpdates++;
#endif
	Matrix44A* m = rd->m_RP.m_TI[rd->m_RP.m_nProcessThreadID].m_matView->GetTop();
	pRE->m_LastCamera.GetModelviewMatrix((float*)m);
	m = rd->m_RP.m_TI[rd->m_RP.m_nProcessThreadID].m_matProj->GetTop();
	//pRE->m_LastCamera.GetProjectionMatrix(*m);
	mathMatrixPerspectiveOffCenter((Matrix44A*)m, pRE->m_LastCamera.fWL, pRE->m_LastCamera.fWR, pRE->m_LastCamera.fWB, pRE->m_LastCamera.fWT, pRE->m_LastCamera.fNear, pRE->m_LastCamera.fFar);

	IDynTexture** pDT;
	if (!pRE->m_bSplit)
	{
		if (!pRE->m_bScreenImposter)
			pDT = &pRE->m_pTexture;
		else
			pDT = &pRE->m_pScreenTexture;
		if (!*pDT)
			*pDT = new SDynTexture2(iResX, iResY, FT_STATE_CLAMP, "CloudImposter", eTP_Clouds);

		if (*pDT)
		{
			SDepthTexture* pDepth = &gcpRendD3D->m_DepthBufferOrig;
			uint32 nX1, nY1, nW1, nH1;
			(*pDT)->Update(iResX, iResY);
			(*pDT)->GetImageRect(nX1, nY1, nW1, nH1);
			if (nW1 > (int)rd->m_d3dsdBackBuffer.Width || nH1 > (int)rd->m_d3dsdBackBuffer.Height)
				pDepth = rd->FX_GetDepthSurface(nW1, nH1, false);
			(*pDT)->ClearRT();
			(*pDT)->SetRT(0, true, pDepth);
			gTexture = (CTexture*)(*pDT)->GetTexture();

			uint32 nX, nY, nW, nH;
			(*pDT)->GetSubImageRect(nX, nY, nW, nH);
			if (pRE->m_bScreenImposter)
			{
				if (CRenderer::CV_r_cloudsdebug != 2)
					rd->LogStrv("Generating screen '%s' - %s (%d, %d, %d, %d) (%d)\n", gTexture->GetName(), (*pDT)->IsSecondFrame() ? "Second" : "First", nX, nY, nW, nH, gRenDev->GetFrameID(false));
			}
			else
			{
				if (CRenderer::CV_r_cloudsdebug != 1)
					rd->LogStrv("Generating '%s' - %s (%d, %d, %d, %d) (%d)\n", gTexture->GetName(), (*pDT)->IsSecondFrame() ? "Second" : "First", nX, nY, nW, nH, gRenDev->GetFrameID(false));
			}

			int nSize = iResX * iResY * 4;
			pRE->m_MemUpdated += nSize / 1024;
			rd->m_RP.m_PS[rd->m_RP.m_nProcessThreadID].m_CloudImpostersSizeUpdate += nSize;
			DisplayWithoutImpostor(pRE->m_LastCamera);
			(*pDT)->SetUpdateMask();
			(*pDT)->RestoreRT(0, true);
		}
	}
	rd->RT_SetViewport(iOldVP[0], iOldVP[1], iOldVP[2], iOldVP[3]);
	rd->EF_PopMatrix();
	rd->m_RP.m_TI[rd->m_RP.m_nProcessThreadID].m_matProj->Pop();

	return true;
}

bool CRECloud::mfDisplay(bool bDisplayFrontOfSplit)
{
	CD3D9Renderer* rd = gcpRendD3D;
	int nThreadID = rd->m_RP.m_nProcessThreadID;
	CRenderObject* pObj = rd->m_RP.m_pCurObject;
	CREImposter* pRE = (CREImposter*)pObj->GetRE();
	Vec3 vPos = pRE->m_vPos;
	CShader* pSH = rd->m_RP.m_pShader;
	SShaderTechnique* pSHT = rd->m_RP.m_pCurTechnique;
	SShaderPass* pPass = rd->m_RP.m_pCurPass;
#ifndef _RELEASE
	rd->m_RP.m_PS[rd->m_RP.m_nProcessThreadID].m_NumCloudImpostersDraw++;
#endif
	if (CRenderer::CV_r_cloudsdebug == 2 && pRE->m_bScreenImposter)
		return true;
	if (CRenderer::CV_r_cloudsdebug == 1 && !pRE->m_bScreenImposter)
		return true;

	uint32 nPersFlags2 = rd->m_RP.m_PersFlags2;
	rd->m_RP.m_PersFlags2 &= ~(RBPF2_COMMIT_PF | RBPF2_COMMIT_CM);

	uint32 nPasses;

	float fAlpha = pObj->m_fAlpha;
	ColorF col(1, 1, 1, fAlpha);

	if (IsRecursiveRenderView())
	{
		DisplayWithoutImpostor(rd->GetRCamera());
		return true;
	}

	//  if (!pRE->m_pTexture || (bDisplayFrontOfSplit && !pRE->m_pFrontTexture))
	//    Warning("WARNING: CRECloud::mfDisplay: missing texture!");
	//  else
	//  {
	//    IDynTexture *pDT;
	//    if (bDisplayFrontOfSplit)
	//      pDT = pRE->m_pFrontTexture;
	//    else
	//      pDT = pRE->m_pTexture;
	//CTexture::m_Text_NoTexture->Apply(0);
	//    pDT->Apply(0);
	//  }

	IDynTexture* pDT;
	if (!pRE->m_bScreenImposter)
		pDT = pRE->m_pTexture;
	else
		pDT = pRE->m_pScreenTexture;

	float fOffsetU = 0, fOffsetV = 0;
	if (pDT && (!bDisplayFrontOfSplit || (bDisplayFrontOfSplit && pRE->m_pFrontTexture)))
	{
		pDT->Apply(0);

		fOffsetU = 0.5f / (float) pDT->GetWidth();
		fOffsetV = 0.5f / (float) pDT->GetHeight();
	}

	// set depth texture for soft clipping of cloud against scene geometry
	if (0 != CTexture::s_ptexZTarget)
	{
		STexState depthTextState(FILTER_POINT, true);
		CTexture::s_ptexZTarget->Apply(1, CTexture::GetTexState(depthTextState));
	}

	int State = GS_BLSRC_ONE | GS_BLDST_ONEMINUSSRCALPHA | GS_ALPHATEST_GREATER;

	if (pRE->m_bSplit)
	{
		if (!bDisplayFrontOfSplit)
			State |= GS_DEPTHWRITE;
		else
			State |= GS_NODEPTHTEST;
	}

	// Martin test - for clouds particle soft clipping against terrain
	//	State |= GS_NODEPTHTEST;

	// Added for Sun-rays to work with clouds:
	// - force depth writing so that sun-rays interact with clouds.
	// - also make sure we don't make any depth test (it's already done in pixel shader) so that no alpha test artifacts are visible
	//State |= GS_DEPTHWRITE | GS_DEPTHFUNC_NOTEQUAL;

	rd->FX_SetState(State, 0);

	Vec3 x, y, z;

	Vec4 vCloudColorScale(m_fCloudColorScale, 0, 0, 0);

	if (!pRE->m_bScreenImposter)
	{
		z = vPos - pRE->m_LastCamera.vOrigin;
		z.Normalize();
		x = (z ^ pRE->m_LastCamera.vY);
		x.Normalize();
		x *= pRE->m_fRadiusX;
		y = (x ^ z);
		y.Normalize();
		y *= pRE->m_fRadiusY;

		const CRenderCamera& cam = rd->GetRCamera();
		Matrix44A* m = rd->m_RP.m_TI[rd->m_RP.m_nProcessThreadID].m_matView->GetTop();
		cam.GetModelviewMatrix((float*)m);
		m = rd->m_RP.m_TI[rd->m_RP.m_nProcessThreadID].m_matProj->GetTop();
		mathMatrixPerspectiveOffCenter(m, cam.fWL, cam.fWR, cam.fWB, cam.fWT, cam.fNear, cam.fFar);

		if (!IsRecursiveRenderView())
		{
			const SRenderTileInfo& rti = rd->GetRenderTileInfo();
			if (rti.nGridSizeX > 1.f || rti.nGridSizeY > 1.f)
			{
				// shift and scale viewport
				m->m00 *= rti.nGridSizeX;
				m->m11 *= rti.nGridSizeY;
				m->m20 = (rti.nGridSizeX - 1.f) - rti.nPosX * 2.0f;
				m->m21 = -((rti.nGridSizeY - 1.f) - rti.nPosY * 2.0f);
			}
		}

		rd->D3DSetCull(eCULL_None);
		static CCryNameTSCRC techName("Cloud_Imposter");
		pSH->FXSetTechnique(techName);
		pSH->FXBegin(&nPasses, FEF_DONTSETSTATES);
		pSH->FXBeginPass(0);

		static CCryNameR g_CloudColorScaleName("g_CloudColorScale");
		pSH->FXSetPSFloat(g_CloudColorScaleName, &vCloudColorScale, 1);

		Vec3 lPos;
		gEnv->p3DEngine->GetGlobalParameter(E3DPARAM_SKY_HIGHLIGHT_POS, lPos);
		Vec4 lightningPosition(lPos.x, lPos.y, lPos.z, 0.0f);
		static CCryNameR LightningPosName("LightningPos");
		pSH->FXSetVSFloat(LightningPosName, &lightningPosition, 1);

		Vec3 lCol;
		gEnv->p3DEngine->GetGlobalParameter(E3DPARAM_SKY_HIGHLIGHT_COLOR, lCol);
		Vec3 lSize;
		gEnv->p3DEngine->GetGlobalParameter(E3DPARAM_SKY_HIGHLIGHT_SIZE, lSize);
		Vec4 lightningColorSize(lCol.x, lCol.y, lCol.z, lSize.x * 0.01f);
		static CCryNameR LightningColSizeName("LightningColSize");
		pSH->FXSetVSFloat(LightningColSizeName, &lightningColorSize, 1);

		rd->m_RP.m_nCommitFlags |= FC_MATERIAL_PARAMS;
		rd->FX_Commit();

		rd->DrawQuad3D(pRE->m_vQuadCorners[0] + vPos,
		               pRE->m_vQuadCorners[1] + vPos,
		               pRE->m_vQuadCorners[2] + vPos,
		               pRE->m_vQuadCorners[3] + vPos, col, 0 + fOffsetU, 1 - fOffsetV, 1 - fOffsetU, 0 + fOffsetV);

		if (CRenderer::CV_r_impostersdraw & 4)
		{
			rd->FX_SetState(GS_NODEPTHTEST);
			SAuxGeomRenderFlags auxFlags;
			auxFlags.SetDepthTestFlag(e_DepthTestOff);
			rd->GetIRenderAuxGeom()->SetRenderFlags(auxFlags);
			rd->GetIRenderAuxGeom()->DrawAABB(AABB(pRE->m_WorldSpaceBV.GetMin(), pRE->m_WorldSpaceBV.GetMax()), false, Col_White, eBBD_Faceted);
		}
		if (CRenderer::CV_r_impostersdraw & 2)
		{
			CRenderObject* pObject = rd->m_RP.m_pCurObject;
			int colR = ((DWORD)((UINT_PTR)pObject) >> 4) & 0xf;
			int colG = ((DWORD)((UINT_PTR)pObject) >> 8) & 0xf;
			int colB = ((DWORD)((UINT_PTR)pObject) >> 12) & 0xf;
			Vec3 v[4];
			v[0] = vPos - y - x;
			v[1] = vPos - y + x;
			v[2] = vPos + y + x;
			v[3] = vPos + y - x;
			vtx_idx inds[6];
			inds[0] = 0;
			inds[1] = 1;
			inds[2] = 2;
			inds[3] = 0;
			inds[4] = 2;
			inds[5] = 3;

			SAuxGeomRenderFlags auxFlags;
			auxFlags.SetFillMode(e_FillModeWireframe);
			auxFlags.SetDepthTestFlag(e_DepthTestOn);
			rd->GetIRenderAuxGeom()->SetRenderFlags(auxFlags);
			rd->GetIRenderAuxGeom()->DrawTriangles(v, 4, inds, 6, Col_Green);
		}
	}
	else
	{
		x = pRE->m_LastCamera.vX;
		x *= 0.5f * (pRE->m_LastCamera.fWR - pRE->m_LastCamera.fWL);
		y = pRE->m_LastCamera.vY;
		y *= 0.5f * (pRE->m_LastCamera.fWT - pRE->m_LastCamera.fWB);
		z = -pRE->m_LastCamera.vZ;
		z *= pRE->m_LastCamera.fNear;

		if (CRenderer::CV_r_impostersdraw & 4)
			rd->GetIRenderAuxGeom()->DrawAABB(AABB(pRE->m_WorldSpaceBV.GetMin(), pRE->m_WorldSpaceBV.GetMax()), false, Col_Red, eBBD_Faceted);

		//D3DXMATRIX matVP;
		//D3DXMatrixMultiply( &matVP, rd->m_RP.m_TI[nThreadID].m_matView->GetTop(), rd->m_RP.m_TI[nThreadID].m_matProj->GetTop() );

		//const Vec3& center( pRE->GetPosition() );
		//D3DXVECTOR4 centerClipSpace;
		//D3DXVec3Transform( &centerClipSpace, &D3DXVECTOR3( center.x, center.y, center.z ), &matVP );

		//float avgDepth( 0 );
		//if( centerClipSpace.w > 0 )
		//	avgDepth = centerClipSpace.z / centerClipSpace.w;

		// draw a polygon with this texture...
		rd->m_RP.m_TI[rd->m_RP.m_nProcessThreadID].m_matProj->Push();
		Matrix44A* m = (Matrix44A*)rd->m_RP.m_TI[rd->m_RP.m_nProcessThreadID].m_matProj->GetTop();
		mathMatrixOrthoOffCenterLH(m, -1, 1, -1, 1, -1, 1);
		if (!IsRecursiveRenderView())
		{
			const SRenderTileInfo& rti = rd->GetRenderTileInfo();
			if (rti.nGridSizeX > 1.f || rti.nGridSizeY > 1.f)
			{
				// shift and scale viewport
				m->m00 *= rti.nGridSizeX;
				m->m11 *= rti.nGridSizeY;
				m->m30 = -((rti.nGridSizeX - 1.f) - rti.nPosX * 2.0f);
				m->m31 = ((rti.nGridSizeY - 1.f) - rti.nPosY * 2.0f);
			}
		}

		rd->PushMatrix();
		rd->m_RP.m_TI[rd->m_RP.m_nProcessThreadID].m_matView->LoadIdentity();
		pSH->FXSetTechnique("Cloud_ScreenImposter");
		pSH->FXBegin(&nPasses, FEF_DONTSETSTATES);
		pSH->FXBeginPass(0);

		float fNear = pRE->m_fNear;
		float fFar = pRE->m_fFar;
		if (fNear < 0 || fNear > 1)
			fNear = 0.92f;
		if (fFar < 0 || fFar > 1)
			fFar = 0.999f;
		float fZ = (fNear + fFar) * 0.5f;

		if (rd->m_RP.m_TI[gRenDev->m_RP.m_nProcessThreadID].m_PersFlags & RBPF_REVERSE_DEPTH)
			fZ = 1.0f - fZ;

		Vec4 pos(pRE->GetPosition(), 1);
		static CCryNameR vCloudWSPosName("vCloudWSPos");
		pSH->FXSetVSFloat(vCloudWSPosName, &pos, 1);
		static CCryNameR g_CloudColorScaleName("g_CloudColorScale");
		pSH->FXSetPSFloat(g_CloudColorScaleName, &vCloudColorScale, 1);

		Vec3 lPos;
		gEnv->p3DEngine->GetGlobalParameter(E3DPARAM_SKY_HIGHLIGHT_POS, lPos);
		Vec4 lightningPosition(lPos.x, lPos.y, lPos.z, col.a);
		static CCryNameR LightningPosName("LightningPos");
		pSH->FXSetVSFloat(LightningPosName, &lightningPosition, 1);

		Vec3 lCol;
		gEnv->p3DEngine->GetGlobalParameter(E3DPARAM_SKY_HIGHLIGHT_COLOR, lCol);
		Vec3 lSize;
		gEnv->p3DEngine->GetGlobalParameter(E3DPARAM_SKY_HIGHLIGHT_SIZE, lSize);
		Vec4 lightningColorSize(lCol.x, lCol.y, lCol.z, lSize.x * 0.01f);
		static CCryNameR LightningColSizeName("LightningColSize");
		pSH->FXSetVSFloat(LightningColSizeName, &lightningColorSize, 1);

		{
			TempDynVB<SVF_P3F_T2F_T3F> vb;
			vb.Allocate(4);
			SVF_P3F_T2F_T3F* vQuad = vb.Lock();

			Vec3 vCoords[8];
			gcpRendD3D->GetRCamera().CalcVerts(vCoords);
			Vec3 vRT = vCoords[4] - vCoords[0];
			Vec3 vLT = vCoords[5] - vCoords[1];
			Vec3 vLB = vCoords[6] - vCoords[2];
			Vec3 vRB = vCoords[7] - vCoords[3];

			vQuad[0].p.x = -1;
			vQuad[0].p.y = -1;
			vQuad[0].p.z = fZ;
			vQuad[0].st0[0] = 0;
			vQuad[0].st0[1] = 1;
			vQuad[0].st1 = vLB;

			vQuad[1].p.x = 1;
			vQuad[1].p.y = -1;
			vQuad[1].p.z = fZ;
			vQuad[1].st0[0] = 1;
			vQuad[1].st0[1] = 1;
			vQuad[1].st1 = vRB;

			vQuad[3].p.x = 1;
			vQuad[3].p.y = 1;
			vQuad[3].p.z = fZ;
			vQuad[3].st0[0] = 1;
			vQuad[3].st0[1] = 0;
			vQuad[3].st1 = vRT;

			vQuad[2].p.x = -1;
			vQuad[2].p.y = 1;
			vQuad[2].p.z = fZ;
			vQuad[2].st0[0] = 0;
			vQuad[2].st0[1] = 0;
			vQuad[2].st1 = vLT;

			vb.Unlock();
			vb.Bind(0);

			rd->m_RP.m_nCommitFlags |= FC_MATERIAL_PARAMS;
			rd->FX_Commit();
			if (!FAILED(rd->FX_SetVertexDeclaration(0, eVF_P3F_T2F_T3F)))
				rd->FX_DrawPrimitive(eptTriangleStrip, 0, 4);

			vb.Release();
		}

		rd->PopMatrix();
		rd->m_RP.m_TI[rd->m_RP.m_nProcessThreadID].m_matProj->Pop();
		rd->EF_DirtyMatrix();
	}

	pSH->FXEndPass();
	pSH->FXEnd();

	rd->m_RP.m_PersFlags2 = nPersFlags2;

	rd->m_RP.m_pCurObject = pObj;
	rd->m_RP.m_pShader = pSH;
	rd->m_RP.m_pCurTechnique = pSHT;
	rd->m_RP.m_pCurPass = pPass;

	return true;
}

bool CRECloud::mfDraw(CShader* ef, SShaderPass* pPass)
{
	if (!CRenderer::CV_r_impostersdraw)
		return true;

	CD3D9Renderer* rd = gcpRendD3D;
	CRenderObject* pObj = rd->m_RP.m_pCurObject;
	CREImposter* pRE = (CREImposter*)pObj->GetRE();

	mfDisplay(false);

	if (pRE->IsSplit())
	{
		// now display the front half of the split impostor.
		mfDisplay(true);
	}

	return true;
}
