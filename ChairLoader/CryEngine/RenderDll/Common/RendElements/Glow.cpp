// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#include "StdAfx.h"
#include "Glow.h"
#include "../Textures/Texture.h"
#include "../CryNameR.h"
#include "../../RenderDll/XRenderD3D9/DriverD3D.h"

#if defined(FLARES_SUPPORT_EDITING)
	#define MFPtr(FUNC_NAME) (Optics_MFPtr)(&Glow::FUNC_NAME)
void Glow::InitEditorParamGroups(DynArray<FuncVariableGroup>& groups)
{
	COpticsElement::InitEditorParamGroups(groups);

	FuncVariableGroup glowGroup;
	glowGroup.SetName("Glow");
	glowGroup.AddVariable(new OpticsMFPVariable(e_INT, "Polygon factor", "Polygons factor", this, MFPtr(SetPolygonFactor), MFPtr(GetPolygonFactor), 0, 128.0f));
	glowGroup.AddVariable(new OpticsMFPVariable(e_FLOAT, "Focus factor", "Focus factor", this, MFPtr(SetFocusFactor), MFPtr(GetFocusFactor)));
	glowGroup.AddVariable(new OpticsMFPVariable(e_FLOAT, "Gamma", "Gamma", this, MFPtr(SetGamma), MFPtr(GetGamma)));
	groups.push_back(glowGroup);
}
	#undef MFPtr
#endif

void Glow::Load(IXmlNode* pNode)
{
	COpticsElement::Load(pNode);

	XmlNodeRef pGlowNode = pNode->findChild("Glow");
	if (pGlowNode)
	{
		int nPolygonFactor(0);
		if (pGlowNode->getAttr("Polygonfactor", nPolygonFactor))
			SetPolygonFactor(nPolygonFactor);

		float fFocusFactor(m_fFocusFactor);
		if (pGlowNode->getAttr("Focusfactor", fFocusFactor))
			SetFocusFactor(fFocusFactor);

		float fGamma(m_fGamma);
		if (pGlowNode->getAttr("Gamma", fGamma))
			SetGamma(fGamma);
	}
}

void Glow::GenMesh()
{
	float ringPos = 1;
	MeshUtil::GenDisk(m_fSize, (int)m_fPolyonFactor, 1, true, m_globalColor, &ringPos, m_vertBuf, m_idxBuf);
}

void Glow::ApplyDistributionParamsPS(CShader* shader)
{
	static CCryNameR lumaParamsName("lumaParams");
	Vec4 lumaParams(m_fFocusFactor, m_fGamma, 0, 0);
	shader->FXSetPSFloat(lumaParamsName, &lumaParams, 1);
}

void Glow::DrawMesh()
{
	gcpRendD3D->FX_Commit();
	DrawMeshTriList();
}

void Glow::Render(CShader* shader, Vec3 vSrcWorldPos, Vec3 vSrcProjPos, SAuxParams& aux)
{
	if (!IsVisible())
		return;

	PROFILE_LABEL_SCOPE("Glow");

	gRenDev->m_RP.m_FlagsShader_RT = 0;

	static CCryNameTSCRC pGlowTechName("Glow");
	shader->FXSetTechnique(pGlowTechName);
	uint nPass;

	shader->FXBegin(&nPass, FEF_DONTSETTEXTURES);

	ApplyGeneralFlags(shader);
	shader->FXBeginPass(0);

	ApplyCommonVSParams(shader, vSrcWorldPos, vSrcProjPos);
	ApplyExternTintAndBrightnessVS(shader, m_globalColor, m_globalFlareBrightness);

	static CCryNameR meshCenterName("meshCenterAndBrt");
	float x = computeMovementLocationX(vSrcProjPos);
	float y = computeMovementLocationY(vSrcProjPos);
	const Vec4 meshCenterParam(x, y, vSrcProjPos.z, 1);
	shader->FXSetVSFloat(meshCenterName, &meshCenterParam, 1);

	ApplyDistributionParamsPS(shader);

	ValidateMesh();
	ApplyMesh();
	DrawMesh();

	shader->FXEndPass();

	shader->FXEnd();
}
