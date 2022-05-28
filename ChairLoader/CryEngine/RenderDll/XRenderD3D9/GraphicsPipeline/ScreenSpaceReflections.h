// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

#include "Common/GraphicsPipelineStage.h"
#include "Common/FullscreenPass.h"
#include "Common/UtilityPasses.h"

class CScreenSpaceReflectionsStage : public CGraphicsPipelineStage
{
public:
	void Init();
	void Execute();

private:
	CFullscreenPass   m_passRaytracing;
	CFullscreenPass   m_passComposition;
	CStretchRectPass  m_passCopy;
	CStretchRectPass  m_passDownsample0;
	CStretchRectPass  m_passDownsample1;
	CStretchRectPass  m_passDownsample2;
	CGaussianBlurPass m_passBlur0;
	CGaussianBlurPass m_passBlur1;
	CGaussianBlurPass m_passBlur2;

	int32             m_samplerPoint;
	int32             m_samplerLinear;
	int32             m_samplerLinearBorder;

	Matrix44          m_prevViewProj[MAX_GPU_NUM];
};
