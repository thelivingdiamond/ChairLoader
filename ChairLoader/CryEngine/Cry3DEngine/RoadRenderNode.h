// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#ifndef ROAD_RENDERNODE_H
#define ROAD_RENDERNODE_H

class CRoadRenderNode : public IRoadRenderNode, public Cry3DEngineBase
{
public:
	CRoadRenderNode();
	virtual ~CRoadRenderNode();

	// IRoadRenderNode implementation
	virtual void SetVertices(const Vec3* pVerts, int nVertsNum, float fTexCoordBegin, float fTexCoordEnd, float fTexCoordBeginGlobal, float fTexCoordEndGlobal);
	virtual void SetSortPriority(uint8 sortPrio);
	virtual void SetIgnoreTerrainHoles(bool bVal);
	virtual void SetPhysicalize(bool bVal) { if (m_bPhysicalize != bVal) { ScheduleRebuild(); m_bPhysicalize = bVal; } }

	// IRenderNode implementation
	virtual const char*         GetEntityClassName(void) const { return "RoadObjectClass"; }
	virtual const char*         GetName(void) const            { return "RoadObjectName"; }
	virtual Vec3                GetPos(bool) const;
	virtual void                Render(const SRendParams& RendParams, const SRenderingPassInfo& passInfo);

	virtual IPhysicalEntity*    GetPhysics(void) const             { return m_pPhysEnt; }
	virtual void                SetPhysics(IPhysicalEntity* pPhys) { m_pPhysEnt = pPhys; }

	virtual void                SetMaterial(IMaterial* pMat);
	virtual IMaterial*          GetMaterial(Vec3* pHitPos = NULL) const;
	virtual IMaterial*          GetMaterialOverride() { return m_pMaterial; }
	virtual float               GetMaxViewDist();
	virtual EERType             GetRenderNodeType();
	virtual struct IRenderMesh* GetRenderMesh(int nLod)     { return nLod == 0 ? m_pRenderMesh.get() : NULL; }
	virtual void                GetMemoryUsage(ICrySizer* pSizer) const;
	virtual const AABB          GetBBox() const             { return m_WSBBox; }
	virtual void                SetBBox(const AABB& WSBBox) { m_WSBBox = WSBBox; }
	virtual void                FillBBox(AABB& aabb);
	virtual void                OffsetPosition(const Vec3& delta);
	virtual void                OnRenderNodeBecomeVisible(const SRenderingPassInfo& passInfo);
	virtual void                GetClipPlanes(Plane* pPlanes, int nPlanesNum, int nVertId);
	virtual void                GetTexCoordInfo(float* pTexCoordInfo);
	virtual uint8               GetSortPriority()           { return m_sortPrio; }

	virtual void                SetLayerId(uint16 nLayerId) { m_nLayerId = nLayerId; Get3DEngine()->C3DEngine::UpdateObjectsLayerAABB(this); }
	virtual uint16              GetLayerId()                { return m_nLayerId; }

	static bool                 ClipTriangle(PodArray<Vec3>& lstVerts, PodArray<vtx_idx>& lstInds, int nStartIdxId, Plane* pPlanes);
	using IRenderNode::Physicalize;
	virtual void                Dephysicalize(bool bKeepIfReferenced = false);
	void                        Compile();
	void                        ScheduleRebuild();
	void                        OnTerrainChanged();

	_smart_ptr<IRenderMesh> m_pRenderMesh;
	_smart_ptr<IMaterial>   m_pMaterial;
	PodArray<Vec3>          m_arrVerts;
	float                   m_arrTexCoors[2];
	float                   m_arrTexCoorsGlobal[2];

	uint8                   m_sortPrio;
	bool                    m_bIgnoreTerrainHoles;
	bool                    m_bPhysicalize;

	IPhysicalEntity*        m_pPhysEnt;

	AABB                    m_WSBBox;

	uint16                  m_nLayerId;

	// tmp buffers used during road mesh creation
	static PodArray<SVF_P3F_C4B_T2S> m_lstVerticesMerged;
	static PodArray<vtx_idx>         m_lstIndicesMerged;
	static PodArray<SPipTangents>    m_lstTangMerged;

	static PodArray<Vec3>            m_lstVerts;
	static PodArray<vtx_idx>         m_lstIndices;

	static PodArray<SPipTangents>    m_lstTang;
	static PodArray<SVF_P3F_C4B_T2S> m_lstVertices;

	static CPolygonClipContext       s_tmpClipContext;

	static void GetMemoryUsageStatic(ICrySizer* pSizer)
	{
		SIZER_COMPONENT_NAME(pSizer, "RoadRenderNodeStaticData");

		pSizer->AddObject(m_lstVerticesMerged);
		pSizer->AddObject(m_lstIndicesMerged);
		pSizer->AddObject(m_lstTangMerged);

		pSizer->AddObject(m_lstVerts);
		pSizer->AddObject(m_lstIndices);

		pSizer->AddObject(m_lstTang);
		pSizer->AddObject(m_lstVertices);

		pSizer->AddObject(s_tmpClipContext);
	}

	static void FreeStaticMemoryUsage()
	{
		m_lstVerticesMerged.Reset();
		m_lstIndicesMerged.Reset();
		m_lstTangMerged.Reset();

		m_lstVerts.Reset();
		m_lstIndices.Reset();

		m_lstTang.Reset();
		m_lstVertices.Reset();

		s_tmpClipContext.Reset();
	}
};
#endif
