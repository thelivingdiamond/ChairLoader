// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

enum EHWShaderClass
{
	eHWSC_Vertex   = 0,
	eHWSC_Pixel    = 1,
	eHWSC_Geometry = 2,
	eHWSC_Compute  = 3,
	eHWSC_Domain   = 4,
	eHWSC_Hull     = 5,
	eHWSC_Num      = 6
};

//! This class provide all necessary resources to the shader extracted from material definition.
class CShaderResources : public IRenderShaderResources, public SBaseShaderResources
{
	enum EFlags
	{
		eFlagInvalid = BIT(0),
	};
public:
	std::vector<Vec4>              m_Constants;
	SEfResTexture*                 m_Textures[EFTT_MAX]; // 48 bytes
	SDeformInfo*                   m_pDeformInfo;        // 4 bytes
	TArray<struct SHRenderTarget*> m_RTargets;           // 4
	CCamera*                       m_pCamera;            // 4
	SSkyInfo*                      m_pSky;               // 4
	SDetailDecalInfo*              m_pDetailDecalInfo;   // 4
	CConstantBuffer*               m_pCB;
	uint16                         m_Id;      // 2 bytes
	uint16                         m_IdGroup; // 2 bytes

	// @see EFlags
	uint32 m_flags;

	/////////////////////////////////////////////////////

	float        m_fMinMipFactorLoad;
	volatile int m_nRefCounter;
	int          m_nLastTexture;
	int          m_nFrameLoad;
	uint32       m_nUpdateFrameID;

	// Compiled resource set.
	// For DX12 will prepare list of textures in the global heap.
	std::shared_ptr<class CDeviceResourceSet>               m_pCompiledResourceSet;
	std::shared_ptr<class CGraphicsPipelineStateLocalCache> m_pipelineStateCache;

	uint8 m_nMtlLayerNoDrawFlags;

public:
	//////////////////////////////////////////////////////////////////////////
	void AddTextureMap(int Id)
	{
		assert(Id >= 0 && Id < EFTT_MAX);
		m_Textures[Id] = new SEfResTexture;
	}
	int Size() const
	{
		int nSize = sizeof(CShaderResources);
		for (int i = 0; i < EFTT_MAX; i++)
		{
			if (m_Textures[i])
				nSize += m_Textures[i]->Size();
		}
		nSize += sizeofVector(m_Constants);
		nSize += m_RTargets.GetMemoryUsage();
		if (m_pDeformInfo)
			nSize += m_pDeformInfo->Size();
		if (m_pDetailDecalInfo)
			nSize += m_pDetailDecalInfo->Size();
		return nSize;
	}

	void GetMemoryUsage(ICrySizer* pSizer) const final
	{
		pSizer->AddObject(this, sizeof(*this));

		for (int i = 0; i < EFTT_MAX; i++)
		{
			pSizer->AddObject(m_Textures[i]);
		}
		pSizer->AddObject(m_Constants);
		pSizer->AddObject(m_RTargets);
		pSizer->AddObject(m_pDeformInfo);
		pSizer->AddObject(m_pDetailDecalInfo);
		SBaseShaderResources::GetMemoryUsage(pSizer);

	}

	CShaderResources();
	CShaderResources& operator=(const CShaderResources& src);
	CShaderResources(struct SInputShaderResources* pSrc);

	void                               PostLoad(CShader* pSH);
	void                               AdjustForSpec();
	void                               CreateModifiers(SInputShaderResources* pInRes);
	virtual void                       UpdateConstants(IShader* pSH) final;
	virtual void                       CloneConstants(const IRenderShaderResources* pSrc) final;
	virtual int                        GetResFlags() final                       { return m_ResFlags; }
	virtual void                       SetMaterialName(const char* szName) final { m_szMaterialName = szName; }
	virtual CCamera*                   GetCamera() final                         { return m_pCamera; }
	virtual void                       SetCamera(CCamera* pCam) final            { m_pCamera = pCam; }
	virtual SSkyInfo*                  GetSkyInfo() final                        { return m_pSky; }
	virtual const float&               GetAlphaRef() const final                 { return m_AlphaRef; }
	virtual void                       SetAlphaRef(float alphaRef) final         { m_AlphaRef = alphaRef; }
	virtual SEfResTexture*             GetTexture(int nSlot) const final         { return m_Textures[nSlot]; }
	virtual DynArrayRef<SShaderParam>& GetParameters() final                     { return m_ShaderParams; }
	virtual ColorF                     GetFinalEmittance() final
	{
		const float kKiloScale = 1000.0f;
		return GetColorValue(EFTT_EMITTANCE) * GetStrengthValue(EFTT_EMITTANCE) * (kKiloScale / RENDERER_LIGHT_UNIT_SCALE);
	}
	virtual float GetVoxelCoverage() final                   { return ((float)m_VoxelCoverage) * (1.0f / 255.0f); }

	virtual void  SetMtlLayerNoDrawFlags(uint8 nFlags) final { m_nMtlLayerNoDrawFlags = nFlags; }
	virtual uint8 GetMtlLayerNoDrawFlags() const final       { return m_nMtlLayerNoDrawFlags; }

	void          RT_UpdateConstants(IShader* pSH);
	void          ReleaseConstants();
	inline float  FurAmount()
	{
		return ((float)m_FurAmount) * (1.0f / 255.0f);
	}

	inline uint8 HeatAmount_unnormalized() const
	{
		return m_HeatAmount;
	}

	inline float HeatAmount() const
	{
		const float fHeatRange = MAX_HEATSCALE;
		return ((float)m_HeatAmount) * (1.0f / 255.0f) * fHeatRange; // rescale range
	}

	inline float CloakAmount()
	{
		return ((float)m_CloakAmount) * (1.0f / 255.0f);
	}

	void Reset()
	{
		for (int i = 0; i < EFTT_MAX; i++)
		{
			m_Textures[i] = NULL;
		}

		m_Id = 0;
		m_IdGroup = 0;
		m_nLastTexture = 0;
		m_pDeformInfo = NULL;
		m_pDetailDecalInfo = NULL;
		m_pCamera = NULL;
		m_pSky = NULL;
		m_pCB = NULL;
		m_nMtlLayerNoDrawFlags = 0;
		m_flags = 0;
		m_nUpdateFrameID = 0;
	}
	bool IsEmpty(int nTSlot) const
	{
		if (!m_Textures[nTSlot])
			return true;
		return false;
	}
	bool                  HasDynamicTexModifiers() const;
	bool                  HasLMConstants() const { return (m_Constants.size() > 0); }
	virtual void          SetInputLM(const CInputLightMaterial& lm) final;
	virtual void          ToInputLM(CInputLightMaterial& lm) final;

	virtual const ColorF& GetColorValue(EEfResTextures slot) const final;
	virtual float         GetStrengthValue(EEfResTextures slot) const final;

	virtual void          SetColorValue(EEfResTextures slot, const ColorF& color) final;
	virtual void          SetStrengthValue(EEfResTextures slot, float value) final;

	virtual void          SetInvalid() final;
	// Check if shader resource is valid
	virtual bool          IsValid() { return 0 == (m_flags & eFlagInvalid); };

	~CShaderResources();
	void                      RT_Release();
	virtual void              Release() final;
	virtual void              AddRef() final { CryInterlockedIncrement(&m_nRefCounter); }
	virtual void              ConvertToInputResource(SInputShaderResources* pDst) final;
	virtual CShaderResources* Clone() const final;
	virtual void              SetShaderParams(SInputShaderResources* pDst, IShader* pSH) final;

	virtual size_t            GetResourceMemoryUsage(ICrySizer* pSizer) final;
	virtual SDetailDecalInfo* GetDetailDecalInfo() final { return m_pDetailDecalInfo; }

	void                      Cleanup();
};
typedef _smart_ptr<CShaderResources> CShaderResourcesPtr;
