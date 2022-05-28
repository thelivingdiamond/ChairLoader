// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#include "StdAfx.h"
#include "../Common/RendElements/OpticsElement.h"
#include "../Common/RendElements/RootOpticsElement.h"
#include "../Common/RendElements/FlareSoftOcclusionQuery.h"
#include "../Common/RendElements/OpticsPredef.hpp"
#include "DriverD3D.h"
#include <Cry3DEngine/I3DEngine.h>

static CSoftOcclusionManager g_SoftOcclusionManager;

CRELensOptics::CRELensOptics(void)
{
	mfSetType(eDATA_LensOptics);
	mfUpdateFlags(FCEF_TRANSFORM);
}

CRELensOptics::~CRELensOptics(void)
{
}

bool CRELensOptics::mfCompile(CParserBin& Parser, SParserFrame& Frame)
{
	return true;
}

void CRELensOptics::mfPrepare(bool bCheckOverflow)
{
	if (bCheckOverflow)
		gRenDev->FX_CheckOverflow(0, 0, this);

	gRenDev->m_RP.m_pRE = this;
	gRenDev->m_RP.m_RendNumIndices = 0;
	gRenDev->m_RP.m_RendNumVerts = 0;
}

void CRELensOptics::ProcessGlobalAction()
{
	PROFILE_LABEL_SCOPE("Soft Occlusion Query");

	CFlareSoftOcclusionQuery::InitGlobalResources();
	CFlareSoftOcclusionQuery::BatchReadResults(); // copy to system memory previous frame

	CShader* pShader = CShaderMan::s_ShaderSoftOcclusionQuery;
	static CCryNameTSCRC sTechName("Main");

	uint32 nPass = 0;
	pShader->FXSetTechnique(sTechName);
	pShader->FXBegin(&nPass, FEF_DONTSETTEXTURES);

	g_SoftOcclusionManager.ComputeVisibility();
	g_SoftOcclusionManager.GatherOcclusions();

	pShader->FXEnd();

	CFlareSoftOcclusionQuery::ReadbackSoftOcclQuery(); // update current frame to staging buffer
	for (int i = 0, iSize(g_SoftOcclusionManager.GetSize()); i < iSize; ++i)
	{
		CFlareSoftOcclusionQuery* pSoftOcclusion = g_SoftOcclusionManager.GetSoftOcclusionQuery(i);
		if (pSoftOcclusion == NULL)
			continue;
		pSoftOcclusion->UpdateCachedResults();
	}
}

static int prevFrameID = 0;

bool CRELensOptics::mfDraw(CShader* pShader, SShaderPass* pass)
{
	if (!CRenderer::CV_r_flares)
		return false;

	CD3D9Renderer* pRD = gcpRendD3D;
	CRenderObject* pObj = pRD->m_RP.m_pCurObject;
	SRenderObjData* pOD = pObj->GetObjData();
	const CRenderCamera* cam = &pRD->GetRCamera();

	// Basically LensOptics should be rendered in only FB_TRANSPARENT phase currently so FB_GENERAL is ignored until it's needed.
	if (pRD->m_RP.m_nBatchFilter == FB_GENERAL)
		return false;

	SRenderLight* pLight = pOD ? &gRenDev->m_RP.m_pCurrentRenderView->GetLight(eDLT_DeferredLight, pOD->m_nLightID) : NULL;
	RootOpticsElement* pRootElem = pLight ? (RootOpticsElement*)pLight->GetLensOpticsElement() : NULL;

	if (!pRootElem || pRootElem->GetType() != eFT_Root)
		return false;

	CFlareSoftOcclusionQuery* pOcc = static_cast<CFlareSoftOcclusionQuery*>(pLight->m_pSoftOccQuery);
	if (!pOcc)
		return false;

#if !defined(_RELEASE)
	char buffer[256];
	cry_sprintf(buffer, "%s-%s", pLight->m_sName, pRootElem->GetName().c_str());
	PROFILE_LABEL_SCOPE(buffer);
#endif

	pRootElem->SetOcclusionQuery(pOcc);
	pOcc->SetOccPlaneSizeRatio(pRootElem->GetOccSize());

	RootOpticsElement::SFlareLight flareLight;
	if (pLight->m_Flags & DLF_ATTACH_TO_SUN)
	{
		const float sunHeight = 20000.0f;
		Vec3 sunDirNorm = gEnv->p3DEngine->GetSunDir();
		sunDirNorm.Normalize();

		flareLight.m_vPos = cam->vOrigin + sunDirNorm * sunHeight;
		flareLight.m_cLdrClr = gEnv->p3DEngine->GetSunColor();
		flareLight.m_fRadius = sunHeight;
		flareLight.m_bAttachToSun = true;
		pLight->SetPosition(flareLight.m_vPos);
	}
	else
	{
		flareLight.m_vPos = pObj->GetTranslation();
		ColorF& c = pLight->m_Color;
		flareLight.m_cLdrClr.set(c.r, c.g, c.b, 1.0f);
		flareLight.m_fRadius = pLight->m_fRadius;
		flareLight.m_bAttachToSun = false;
	}

	ColorF cNorm;
	flareLight.m_fClrMultiplier = flareLight.m_cLdrClr.NormalizeCol(cNorm);
	flareLight.m_cLdrClr = cNorm;

	flareLight.m_fViewAngleFalloff = 1.0;
	if (pLight->m_LensOpticsFrustumAngle != 0)
	{
		Vec3 vDirLight2Camera = (cam->vOrigin - flareLight.m_vPos).GetNormalizedFast();
		float viewCos = pLight->m_ProjMatrix.GetColumn(0).Dot(vDirLight2Camera) * 0.5f + 0.5f;
		if (pLight->m_LensOpticsFrustumAngle < 255)
		{
			int angle = (int)((float)pLight->m_LensOpticsFrustumAngle * (360.0f / 255.0f));
			float radAngle = DEG2RAD(angle);
			float halfFrustumCos = cos_tpl(radAngle * 0.5f) * 0.5f + 0.5f;
			float quarterFrustumCos = cos_tpl(radAngle * 0.25f) * 0.5f + 0.5f;
			if (viewCos < quarterFrustumCos)
				flareLight.m_fViewAngleFalloff = max(1.0f - (float)((quarterFrustumCos - viewCos) / (quarterFrustumCos - halfFrustumCos)), 0.0f);
			else
				flareLight.m_fViewAngleFalloff = 1;
		}
	}
	else
	{
		flareLight.m_fViewAngleFalloff = 0.0;
	}

	int curFID = gRenDev->GetFrameID(false);
	if (curFID != prevFrameID)
	{
		if (g_SoftOcclusionManager.Begin())
		{
			ProcessGlobalAction();
			g_SoftOcclusionManager.End();
		}
		prevFrameID = curFID;
	}

	if (pRootElem->ProcessAll(pShader, flareLight))
		g_SoftOcclusionManager.AddSoftOcclusionQuery(pOcc, pLight->GetPosition());

	pRootElem->SetOcclusionQuery(NULL);

	return true;
}

void CRELensOptics::ClearResources()
{
	g_SoftOcclusionManager.ClearResources();
}
