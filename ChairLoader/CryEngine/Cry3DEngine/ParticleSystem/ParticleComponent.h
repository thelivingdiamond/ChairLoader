// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

// -------------------------------------------------------------------------
//  Created:     29/01/2015 by Filipe amim
//  Description:
// -------------------------------------------------------------------------
//
////////////////////////////////////////////////////////////////////////////

#ifndef PARTICLECOMPONENT_H
#define PARTICLECOMPONENT_H

#pragma once

#include "ParticleCommon.h"
#include "ParticleContainer.h"
#include "Features/ParamTraits.h"

namespace pfx2
{

SERIALIZATION_ENUM_DECLARE(EAnimationCycle, : uint8,
                           Once,
                           Loop,
                           Mirror
                           )

struct STextureAnimation
{
	TValue<float, USoftLimit<60>>       m_frameRate;     //!< Anim framerate; 0 = 1 cycle / particle life.
	TValue<uint16, THardLimits<1, 256>> m_frameCount;    //!< Number of tiles (frames) of animation
	EAnimationCycle                     m_cycleMode;     //!< How animation cycles.
	bool                                m_frameBlending; //!< Blend textures between frames.

	// To do: Add random and curve modifiers

	STextureAnimation()
		: m_cycleMode(EAnimationCycle::Once), m_frameBlending(true), m_ageScale(1.0f), m_animPosScale(1.0f)
	{
	}

	bool IsAnimating() const
	{
		return m_frameCount > 1;
	}
	bool HasAbsoluteFrameRate() const
	{
		return m_frameRate > 0.0f;
	}
	float GetAnimPosAbsolute(float age) const
	{
		// Select anim frame based on particle age.
		float animPos = age * m_ageScale;
		switch (m_cycleMode)
		{
		case EAnimationCycle::Once:
			animPos = min(animPos, 1.f);
			break;
		case EAnimationCycle::Loop:
			animPos = fmod(animPos, 1.f);
			break;
		case EAnimationCycle::Mirror:
			animPos = 1.f - abs(fmod(animPos, 2.f) - 1.f);
			break;
		}
		return animPos * m_animPosScale;
	}
	float GetAnimPosRelative(float relAge) const
	{
		return relAge * m_animPosScale;
	}

	void Serialize(Serialization::IArchive& ar);

private:
	float m_ageScale;
	float m_animPosScale;

	void  Update();
};

struct SVisibilityParams
{
	UFloat m_viewDistanceMultiple;         // Multiply standard view distance calculated from max particle size and e_ParticlesMinDrawPixels
	UFloat m_maxScreenSize;                // Override cvar e_ParticlesMaxDrawScreen, fade out near camera
	UFloat m_minCameraDistance;
	UFloat m_maxCameraDistance;

	SVisibilityParams()
		: m_viewDistanceMultiple(1.0f)
		, m_maxScreenSize(2.0f)
	{}
};

struct SComponentParams
{
	SComponentParams();
	SComponentParams(const CParticleComponent& component);

	void  Serialize(Serialization::IArchive& ar);

	void  Reset();
	void  Validate(CParticleComponent* pComponent, Serialization::IArchive* ar = 0);
	float GetPrimeTime() const;
	bool  IsValid() const     { return m_isValid; }
	bool  HasChildren() const { return !m_subComponentIds.empty(); }
	bool  IsSecondGen() const { return m_parentId != gInvalidId; }
	void  MakeMaterial(CParticleComponent* pComponent);

	// PFX2_TODO : Reorder from larger to smaller
	const CParticleComponent* m_pComponent;
	SParticleShaderData       m_shaderData;
	_smart_ptr<IMaterial>     m_pMaterial;
	_smart_ptr<IMeshObj>      m_pMesh;
	string                    m_diffuseMap;
	uint32                    m_renderObjectFlags;
	size_t                    m_instanceDataStride;
	STextureAnimation         m_textureAnimation;
	float                     m_scaleParticleCount;
	float                     m_baseParticleLifeTime;
	float                     m_maxParticleSize;
	float                     m_renderObjectSortBias;
	SVisibilityParams         m_visibility;
	size_t                    m_particleRange;
	int                       m_renderStateFlags;
	std::vector<TComponentId> m_subComponentIds;
	TComponentId              m_parentId;
	uint8                     m_particleObjFlags;
	bool                      m_meshCentered;
	bool                      m_isValid;
};

class CParticleComponent : public IParticleComponent
{
public:
	CParticleComponent();

	// IParticleComponent
	virtual void                                     SetChanged() override;
	virtual void                                     SetEnabled(bool enabled) override { SetChanged(); m_enabled.Set(enabled); }
	virtual bool                                     IsEnabled() const override        { return m_enabled; }
	virtual bool                                     IsVisible() const override        { return m_visible; }
	virtual void                                     SetVisible(bool visible) override { m_visible.Set(visible); }
	virtual void                                     Serialize(Serialization::IArchive& ar) override;
	virtual void                                     SetName(const char* name) override;
	virtual const char*                              GetName() const override        { return m_name.c_str(); }
	virtual uint                                     GetNumFeatures() const override { return m_features.size(); }
	virtual IParticleFeature*                        GetFeature(uint featureIdx) const override;
	virtual void                                     AddFeature(uint placeIdx, const SParticleFeatureParams& featureParams) override;
	virtual void                                     RemoveFeature(uint featureIdx) override;
	virtual void                                     SwapFeatures(const uint* swapIds, uint numSwapIds) override;
	virtual Vec2                                     GetNodePosition() const override;
	virtual void                                     SetNodePosition(Vec2 position) override;

	const SRuntimeInitializationParameters&          GetRuntimeInitializationParameters() const                                   { return m_runtimeInitializationParameters; };
	void                                             SetRuntimeInitializationParameters(SRuntimeInitializationParameters& params) { m_runtimeInitializationParameters = params; }
	virtual gpu_pfx2::IParticleFeatureGpuInterface** GetGpuUpdateList(EUpdateList list, int& size) const override;
	// ~IParticleComponent

	void                                  PreCompile();
	void                                  ResolveDependencies();
	void                                  Compile();
	void                                  FinalizeCompile();

	TComponentId                          GetComponentId() const        { return m_componentId; }
	CParticleEffect*                      GetEffect() const             { return m_pEffect; }
	uint                                  GetNumFeatures(EFeatureType type) const;
	CParticleFeature*                     GetCFeature(size_t idx) const { return m_features[idx]; }

	void                                  AddToUpdateList(EUpdateList list, CParticleFeature* pFeature);
	TInstanceDataOffset                   AddInstanceData(size_t size);
	void                                  AddParticleData(EParticleDataType type);
	void                                  AddParticleData(EParticleVec3Field type);
	void                                  AddParticleData(EParticleQuatField type);
	const std::vector<CParticleFeature*>& GetUpdateList(EUpdateList list) const         { return m_updateLists[list]; }

	const SComponentParams& GetComponentParams() const                    { return m_componentParams; }
	bool                    UseParticleData(EParticleDataType type) const { return m_useParticleData[type]; }

	bool                    SetSecondGeneration(CParticleComponent* pParentComponent);
	CParticleComponent*     GetParentComponent() const;

	void                    Render(ICommonParticleComponentRuntime* pRuntime, const SRenderContext& renderContext, IRenderNode* pNode);
private:
	void                    AddRemoveParticles(const SUpdateContext& context);
	void                    UpdateNewBorns(const SUpdateContext& context, const SUpdateRange& updateRange);
	void                    UpdateFeatures(const SUpdateContext& context, const SUpdateRange& updateRange);
	void                    Integrate(const SUpdateRange& updateRange, float deltaTime);
	void                    PostUpdateFeatures(const SUpdateContext& context, const SUpdateRange& updateRange);

	void                    AgeUpdate(const SUpdateRange& updateRange, float deltaTime);
	void                    LinearIntegral(const SUpdateRange& updateRange, float deltaTime);
	void                    QuadraticIntegral(const SUpdateRange& updateRange, float deltaTime);
	void                    ExpDragIntegral(const SUpdateRange& updateRange, float deltaTime);

	void                    DebugStabilityCheck();

	friend class CParticleEffect;
	Vec2                                                 m_nodePosition;
	CParticleEffect*                                     m_pEffect;
	TComponentId                                         m_componentId;
	string                                               m_name;
	SComponentParams                                     m_componentParams;
	std::vector<TParticleFeaturePtr>                     m_features;
	std::vector<CParticleFeature*>                       m_updateLists[EUL_Count];
	std::vector<gpu_pfx2::IParticleFeatureGpuInterface*> m_gpuUpdateLists[EUL_Count];
	bool                                                 m_useParticleData[EPDT_Count];
	SEnable                                              m_enabled;
	SEnable                                              m_visible;
	bool                                                 m_dirty;

	SRuntimeInitializationParameters                     m_runtimeInitializationParameters;
};

}

#endif // PARTICLECOMPONENT_H
