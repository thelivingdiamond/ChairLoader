// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#ifndef _D3DVOLUMETRICCLOUDS_
#define _D3DVOLUMETRICCLOUDS_

class CVolumetricCloudManager
{
public:
	CVolumetricCloudManager();
	~CVolumetricCloudManager();

	// generate the 3d textures
	void GenerateVolumeTextures();

	// render the clouds
	void RenderClouds();

	void PushCloudBlocker(const Vec3& pos, const Vec3& param, int flag);

	bool IsRenderable() const;

	bool IsEnabledVolumetricCloudsShadow() const { return m_enableVolumetricCloudsShadow; }

	Vec4 GetVolumetricCloudShadowParam(class CD3D9Renderer* r, const Vec2& windOffset, const Vec2& vTiling) const;

private:
	void BuildCloudBlockerList();
	void BuildCloudBlockerSSList();

	bool CreateResources();

	bool AreTexturesValid() const;

	// generate the 3d shadow texture
	void GenerateCloudShadowVol();

	void MakeCloudShaderParam(struct VCCloudRenderContext& context, struct VCCloudShaderParam& param);
	void SetShaderParameters(class CShader& shader, const struct VCCloudShaderParam& p, const struct VCCloudRenderContext& context);

	bool GenerateCloudDensityAndShadow();

private:
	static const uint32 MaxFrameNum = 4;
	static const uint32 MaxEyeNum = 2;

	struct SCloudBlocker
	{
		Vec3 pos;
		Vec3 param;
	};

	int                   m_nUpdateFrameID[MaxEyeNum];
	bool                  m_enableVolumetricCloudsShadow;
	int32                 m_cleared;
	int32                 m_tick;
	Matrix44A             m_viewProj[MaxEyeNum][MaxFrameNum];

	CTexture*             m_downscaledTex[MaxEyeNum][2];
	CTexture*             m_prevScaledDepthBuffer[MaxEyeNum];
	CTexture*             m_cloudDepthBuffer;
	CTexture*             m_downscaledTemp;
	CTexture*             m_downscaledMinTex[MaxEyeNum][2];
	CTexture*             m_downscaledLeftEye;
	CTexture*             m_cloudDensityTex;
	CTexture*             m_cloudShadowTex;

	TArray<Vec4>          m_blockerPosArray;
	TArray<Vec4>          m_blockerParamArray;
	TArray<SCloudBlocker> m_blockerArray[RT_COMMAND_BUF_COUNT];

	TArray<Vec4>          m_blockerSSPosArray;
	TArray<Vec4>          m_blockerSSParamArray;
	TArray<SCloudBlocker> m_blockerSSArray[RT_COMMAND_BUF_COUNT];
};

#endif
