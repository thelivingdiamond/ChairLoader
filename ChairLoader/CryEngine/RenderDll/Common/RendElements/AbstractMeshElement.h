// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

struct SVF_P3F_C4B_T2F;
class AbstractMeshElement
{
protected:
	std::vector<SVF_P3F_C4B_T2F> m_vertBuf;
	std::vector<uint16>          m_idxBuf;
	bool                         m_meshDirty;

	virtual void ApplyMesh();
	virtual void ApplyVert();
	virtual void ApplyIndices();

	//	Render the mesh. Must have ApplyMesh called before this to make sure
	//	all data are copied and all states are set.
	void DrawMeshTriList();

	//	Render the mesh in wireframe mode. Primarily for debugging. NO FXCommit needed.
	//	Must have ApplyMesh called before this to make sure
	//	all data are copied and all states are set.
	void DrawMeshWireframe();

	// Custom Mesh Generation function.
	// Force to generate the mesh only.
	// This method doesn't alter the mark-dirty flag
	virtual void GenMesh() = 0;

	// Validate the internal mesh representation.
	// This regenerates the mesh when the related data is modified
	virtual void ValidateMesh()
	{
		if (m_meshDirty)
		{
			GenMesh();
			m_meshDirty = false;
		}
	}

	int GetMeshDataSize() const
	{
		return m_vertBuf.size() * sizeof(SVF_P3F_C4B_T2F) + m_idxBuf.size() * sizeof(uint16) + sizeof(bool);
	}

public:
	AbstractMeshElement() :
		m_meshDirty(true)
	{
	}
	virtual ~AbstractMeshElement() {}

	SVF_P3F_C4B_T2F* GetVertBufData()  { return &m_vertBuf[0]; }
	int              GetVertCount()    { return m_vertBuf.size(); }
	int              GetVertBufSize()  { return GetVertCount() * sizeof(SVF_P3F_C4B_T2F); }

	uint16*          GetIndexBufData() { return &m_idxBuf[0]; }
	int              GetIndexCount()   { return m_idxBuf.size(); }
	int              GetIndexBufSize() { return GetIndexCount() * sizeof(uint16); }
};
