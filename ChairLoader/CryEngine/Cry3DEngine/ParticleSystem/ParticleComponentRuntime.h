// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

// -------------------------------------------------------------------------
//  Created:     02/04/2015 by Filipe amim
//  Description:
// -------------------------------------------------------------------------
//
////////////////////////////////////////////////////////////////////////////

#ifndef PARTICLECOMPONENTRUNTIME_H
#define PARTICLECOMPONENTRUNTIME_H

#pragma once

#include "ParticleCommon.h"
#include "ParticleComponent.h"

namespace pfx2
{

class CParticleComponentRuntime : public ICommonParticleComponentRuntime, public IParticleVertexCreator, public _i_reference_target_t
{
public:
	typedef std::vector<SInstance, stl::aligned_alloc<SInstance, CRY_PFX2_PARTICLES_ALIGNMENT>> TInstances;

public:
	CParticleComponentRuntime(CParticleEffect* pEffect, CParticleEmitter* pEmitter, CParticleComponent* pComponent);

	// ICommonParticleComponentRuntime
	// PFX2_TODO : Figure out a way to do static dispatches
	virtual const AABB&                GetBounds() const override { return m_bounds; }
	virtual CParticleComponentRuntime* GetCpuRuntime() override   { return this; }
	virtual void                       ReparentParticles(const uint* swapIds, const uint numSwapIds) override;
	virtual bool                       IsValidRuntimeForInitializationParameters(const SRuntimeInitializationParameters& parameters) override;
	virtual void                       MainPreUpdate() override;
	virtual void                       AccumCounts(SParticleCounts& counts) override;
	// ~ICommonParticleComponentRuntime

	void                      Initialize();
	void                      Reset();
	CParticleEffect*          GetEffect() const          { return m_pEffect; }
	CParticleEmitter*         GetEmitter() const         { return m_pEmitter; }
	CParticleComponent*       GetComponent() const       { return m_pComponent; }
	const SComponentParams&   GetComponentParams() const { return m_pComponent->GetComponentParams(); }

	CParticleContainer&       GetParentContainer();
	const CParticleContainer& GetParentContainer() const;
	CParticleContainer&       GetContainer()            { return m_container; }
	const CParticleContainer& GetContainer() const      { return m_container; }

	virtual bool              IsActive() const override { return m_active; }
	void                      SetActive(bool active);
	void                      UpdateAll(const SUpdateContext& context);
	void                      AddRemoveNewBornsParticles(const SUpdateContext& context);
	void                      UpdateParticles(const SUpdateContext& context);
	void                      CalculateBounds();
	void                      WaitUntilUpdateEnded();
	ILINE void                SetStateUpdateStarted() { m_updateState.SetRunning(); }
	ILINE void                SetStateUpdateEnded()   { m_updateState.SetStopped(); }

	virtual void              ComputeVertices(const SCameraInfo& camInfo, CREParticle* pRE, uint64 uRenderFlags, float fMaxPixels) override;
	virtual void              AddSubInstances(SInstance* pInstances, size_t count) override;
	void                      RemoveSubInstance(size_t instanceId);
	void                      RemoveAllSubInstances();
	size_t                    GetNumInstances() const       { return m_subInstances.size(); }
	const SInstance& GetInstance(size_t idx) const { return m_subInstances[idx]; }
	IPidStream       GetInstanceParentIds() const  { return IPidStream(&(m_subInstances.data())->m_parentId, gInvalidId); }
	template<typename T>
	T*               GetSubInstanceData(size_t instanceId, TInstanceDataOffset offset);
	void             SpawnParticles(CParticleContainer::SSpawnEntry const& entry);
	void             GetSpatialExtents(const SUpdateContext& context, Array<const float, uint> scales, Array<float, uint> extents);

private:
	void AddRemoveParticles(const SUpdateContext& context);
	void UpdateNewBorns(const SUpdateContext& context);
	void UpdateFeatures(const SUpdateContext& context);
	void PostUpdateFeatures(const SUpdateContext& context);

	void AgeUpdate(const SUpdateContext& context);

	void AlignInstances();

	void DebugStabilityCheck();

	CParticleContainer                           m_container;
	TInstances                                   m_subInstances;
	std::vector<byte>                            m_subInstanceData;
	std::vector<CParticleContainer::SSpawnEntry> m_spawnEntries;
	CParticleEffect*                             m_pEffect;
	CParticleEmitter*                            m_pEmitter;
	CParticleComponent*                          m_pComponent;
	JobManager::SJobState                        m_updateState;
	AABB m_bounds;
	bool m_active;
};

}

#include "ParticleComponentImpl.h"

#endif // PARTICLECOMPONENTRUNTIME_H
