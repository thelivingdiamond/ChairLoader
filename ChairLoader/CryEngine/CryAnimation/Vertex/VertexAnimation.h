// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

#include <CryAnimation/IVertexAnimation.h>
#include "VertexData.h"

namespace Skeleton {
class CPoseData;
}
class CVertexCommandBuffer;
class CSkin;
class CAttachmentSKIN;
class CAttachmentVCLOTH;
class CClothPiece;
struct SSoftwareVertexFrame;
struct RecTangents;

enum VA_FRAME_FLAGS
{
	VA_FRAME_CULLED = 0x1,
};

struct SVertexFrameState
{
	const SSoftwareVertexFrame* pFrame;
	float                       weight;
	uint32                      flags;
	uint32                      sourceIndex;

	SVertexFrameState() :
		pFrame(NULL),
		weight(0.0f),
		flags(0),
		sourceIndex(-1)
	{
	}
};

struct SVertexSkinData
{
	const DualQuat*                                  pTransformations;
	const JointIdType*                               pTransformationRemapTable;
	uint                                             transformationCount;

	strided_pointer<const Vec3>                      pVertexPositions;
	strided_pointer<const Vec3>                      pVertexPositionsPrevious;
	strided_pointer<const uint32>                    pVertexColors;
	strided_pointer<const Vec2>                      pVertexCoords;
	strided_pointer<const Quat>                      pVertexQTangents;
	strided_pointer<const SoftwareVertexBlendIndex>  pVertexTransformIndices;
	strided_pointer<const SoftwareVertexBlendWeight> pVertexTransformWeights;
	uint                                             vertexTransformCount;

	const vtx_idx*                                   pIndices;
	uint                                             indexCount;

	const STangentUpdateTriangles*                   pTangentUpdateTriangles;
	uint                                             tangetUpdateTriCount;
	const vtx_idx*                                   pTangentUpdateVertIds;
	uint                                             tangetUpdateVertIdsCount;
	uint                                             vertexCount;
};

#ifndef _RELEASE

struct SVertexAnimationStats
{
	ICharacterInstance* pCharInstance;
	string              sCharInstanceName;
	string              sAttachmentName;
	uint                vertexCount;
	uint                activeFrameCount;
	uint                vertexBlendCount;

	SVertexAnimationStats()
	{
		pCharInstance = NULL;
		vertexCount = 0;
		activeFrameCount = 0;
		vertexBlendCount = 0;
	}
};

class CVertexAnimationProfiler
{
public:
	void AddVertexAnimationStats(const SVertexAnimationStats& vertexAnimationStats);
	void Clear();
	void DrawVertexAnimationStats(uint profileType);

protected:
	DynArray<SVertexAnimationStats> m_vertexAnimationStats;
};

extern CVertexAnimationProfiler g_vertexAnimationProfiler;
#endif

class CVertexAnimation :
	public IVertexAnimation
{
public:
	static void RegisterSoftwareRenderMesh(CAttachmentSKIN* pAttachment);
	static void RemoveSoftwareRenderMesh(CAttachmentSKIN* pAttachment);
	static void RegisterSoftwareRenderMesh(CAttachmentVCLOTH* pAttachment);
	static void RemoveSoftwareRenderMesh(CAttachmentVCLOTH* pAttachment);
	static void ClearSoftwareRenderMeshes();

private:
	static std::vector<CAttachmentSKIN*>   s_softwareRenderMeshes;
	static std::vector<CAttachmentSKIN*>   s_newSoftwareRenderMeshes;
	static std::vector<CAttachmentVCLOTH*> s_softwareRenderMeshesVCloth;
	static std::vector<CAttachmentVCLOTH*> s_newSoftwareRenderMeshesVCloth;

public:
	CVertexAnimation();
	~CVertexAnimation();

public:
	void                         SetFrameWeight(const SSoftwareVertexFrame& frame, float weight);
	void                         SetFrameWeightByIndex(const CSkin* pSkin, uint index, float weight);
	void                         SetFrameWeightByName(const ISkin* pISkin, const char* name, float weight);
	void                         ClearAllFramesWeight();

	void                         SetSkinData(const SVertexSkinData& skinData) { m_skinData = skinData; }

	void                         SetClothData(CClothPiece* pClothPiece)       { m_pClothPiece = pClothPiece; }

	bool                         CompileCommands(CVertexCommandBuffer& commandBuffer);

	DynArray<SVertexFrameState>& GetFrameStates() { return m_frameStates; }

	bool                         CreateFrameStates(const CSoftwareVertexFrames& vertexFrames, const CDefaultSkeleton& skeleton);
	void                         UpdateFrameWeightsFromPose(const Skeleton::CPoseData& pose);

private:
	bool CompileAdds(CVertexCommandBuffer& commandBuffer);
	void CompileTangents(CVertexCommandBuffer& commandBuffer);

private:
	DynArray<SVertexFrameState> m_frameStates;
	DynArray<RecTangents>       m_recTangents;

	SVertexSkinData             m_skinData;

	CClothPiece*                m_pClothPiece;

public:
	uint m_RenderMeshId;
	uint m_skinningPoolID;

#ifndef _RELEASE
	// Profile data
	SVertexAnimationStats m_vertexAnimationStats;
#endif
};
