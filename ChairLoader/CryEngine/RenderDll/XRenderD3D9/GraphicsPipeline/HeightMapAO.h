// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

#include "Common/GraphicsPipelineStage.h"
#include "Common/FullscreenPass.h"
#include "Common/UtilityPasses.h"

class CHeightMapAOStage : public CGraphicsPipelineStage
{
public:
	void Init();
	void Execute(ShadowMapFrustum*& pHeightMapFrustum, CTexture*& pHeightMapAOScreenDepthTex, CTexture*& pHeightMapAOTex);

private:
	CFullscreenPass m_passSampling;
	CFullscreenPass m_passSmoothing;
	CMipmapGenPass  m_passMipmapGen;

	int32           m_samplerPoint;
	int32           m_samplerLinear;
	int32           m_samplerLinearBorder;
};
