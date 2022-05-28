// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

// -------------------------------------------------------------------------
//  Created:     21/10/2015 by Benjamin Block
//  Description:
// -------------------------------------------------------------------------
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include <CrySerialization/Serializer.h>
#include <CrySerialization/Math.h>
#include <CrySerialization/SmartPtr.h>
#include "ParticleSystem/ParticleFeature.h"
#include "ParticleSystem/ParticleEmitter.h"

#include <CryRenderer/IGpuParticles.h>

CRY_PFX2_DBG

volatile bool gFeatureFluidDynamics = false;

namespace pfx2
{

class CFeatureMotionFluidDynamics : public CParticleFeature
{
public:
	CRY_PFX2_DECLARE_FEATURE

	CFeatureMotionFluidDynamics()
		: CParticleFeature(gpu_pfx2::eGpuFeatureType_MotionFluidDynamics)
		, m_initialVelocity(0.0, 0.0, 5.0)
		, m_stiffness(1000.f)
		, m_gravityConstant(-5.f)
		, m_h(0.5f)
		, m_r0(32.f)
		, m_mass(1.0f)
		, m_maxVelocity(5.0f)
		, m_maxForce(500.f)
		, m_atmosphericDrag(50.f)
		, m_cohesion(1.0f)
		, m_baroclinity(0.25f)
		, m_spread(1.0f)
		, m_particleInfluence(0.1f)
		, m_numberOfIterations(1)
		, m_gridSizeX(16)
		, m_gridSizeY(16)
		, m_gridSizeZ(16)
		, m_numSpawnParticles(10)
	{}

	virtual void AddToComponent(CParticleComponent* pComponent, SComponentParams* pParams) override
	{
		pComponent->AddToUpdateList(EUL_Update, this);

		if (auto pInt = GetGpuInterface())
		{
			gpu_pfx2::SFeatureParametersMotionFluidDynamics params;
			params.initialVelocity = m_initialVelocity;
			params.stiffness = m_stiffness;
			params.gravityConstant = m_gravityConstant;
			params.h = m_h;
			params.r0 = m_r0;
			params.mass = m_mass;
			params.maxVelocity = m_maxVelocity;
			params.maxForce = m_maxForce;
			params.atmosphericDrag = m_atmosphericDrag;
			params.cohesion = m_cohesion;
			params.baroclinity = m_baroclinity;
			params.spread = m_spread;
			params.numberOfIterations = m_numberOfIterations;
			params.gridSizeX = m_gridSizeX;
			params.gridSizeY = m_gridSizeY;
			params.gridSizeZ = m_gridSizeZ;
			params.particleInfluence = m_particleInfluence;
			params.numSpawnParticles = m_numSpawnParticles;
			pInt->SetParameters(params);
		}
	}

	virtual void Serialize(Serialization::IArchive& ar) override
	{
		CParticleFeature::Serialize(ar);
		ar(m_initialVelocity, "initialVelocity", "Initial Velocity");
		ar(m_stiffness, "stiffness", "Stiffness");
		ar(m_gravityConstant, "gravityConstant", "Gravity Constant");
		ar(m_h, "h", "h");
		ar(m_r0, "r0", "Rest Density");
		ar(m_mass, "mass", "Particle Mass");
		ar(m_maxVelocity, "maxVelocity", "Maximum Velocity");
		ar(m_maxForce, "maxForce", "Maximum Force");
		ar(m_atmosphericDrag, "atmosphericDrag", "Atmospheric Drag");
		ar(m_cohesion, "cohesion", "Cohesion");
		ar(m_baroclinity, "baroclinity", "Baroclinity");
		ar(m_spread, "spread", "Spread");
		ar(m_particleInfluence, "particleInfluence", "Particle Influence");
		ar(m_numberOfIterations, "numberOfIterations", "Number Of Iterations");
		ar(m_gridSizeX, "gridSizeX", "Grid Size X");
		ar(m_gridSizeY, "gridSizeY", "Grid Size Y");
		ar(m_gridSizeZ, "gridSizeZ", "Grid Size Z");
		ar(m_numSpawnParticles, "numSpawnParticles", "Particles per Frame");
	}

private:
	Vec3  m_initialVelocity;
	float m_stiffness;
	float m_gravityConstant;
	float m_h;
	float m_r0;
	float m_mass;
	float m_maxVelocity;
	float m_maxForce;
	float m_atmosphericDrag;
	float m_cohesion;
	float m_baroclinity;
	float m_spread;
	float m_particleInfluence;
	int   m_numberOfIterations;
	int   m_gridSizeX;
	int   m_gridSizeY;
	int   m_gridSizeZ;
	int   m_numSpawnParticles;
};

CRY_PFX2_IMPLEMENT_FEATURE(CParticleFeature, CFeatureMotionFluidDynamics, "Motion", "GPU Fluid Dynamics", defaultIcon, ColorB(255, 0, 0));

}
