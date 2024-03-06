/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "r_nsmoothing.h"
#include "system.h"

// max bbox size for normal smoothing groups
const Float CNormalSmoothing::CNM_BBOXSIZE = 1024;
// max block size for allocating vertexes
const Float CNormalSmoothing::CNM_ALLOCSIZE = 32;
// Max depth of node generation
const Uint32 CNormalSmoothing::MAX_NODE_DEPTH = 12;

//=============================================
// @brief
//
//=============================================
CNormalSmoothing::CNormalSmoothing( const Vector& worldMins, const Vector& worldMaxs, Uint32 numvertexes, Float blendangle ):
	m_pVertexIdxMapping(nullptr),
	m_pVertexGrpMapping(nullptr),
	m_numVertexes(numvertexes),
	m_normalBlend(0)
{
	m_pVertexIdxMapping = new Uint32[m_numVertexes];
	memset(m_pVertexIdxMapping, 0, sizeof(int)*m_numVertexes);

	m_pVertexGrpMapping = new vertexgroup_t*[m_numVertexes];
	memset(m_pVertexGrpMapping, 0, sizeof(vertexgroup_t*)*m_numVertexes);

	m_normalBlend = cos( blendangle * (M_PI / 180.0));

	// Create the nodes
	CreateVertexNode(worldMins, worldMaxs, 0);
}

//=============================================
// @brief
//
//=============================================
CNormalSmoothing::~CNormalSmoothing()
{
	if(m_pVertexIdxMapping)
	{
		delete[] m_pVertexIdxMapping;
		m_pVertexIdxMapping = nullptr;
	}

	if(m_pVertexGrpMapping)
	{
		delete[] m_pVertexGrpMapping;
		m_pVertexGrpMapping = nullptr;
	}

	if(!m_vertexGroupPtrsArray.empty())
	{
		for(Uint32 i = 0; i < m_vertexGroupPtrsArray.size(); i++)
			delete m_vertexGroupPtrsArray[i];
	}

	for(Uint32 i = 0; i < m_vertexNodesArray.size(); i++)
		delete m_vertexNodesArray[i];
}

//=============================================
// @brief
//
//=============================================
CNormalSmoothing::vertexnode_t* CNormalSmoothing::CreateVertexNode( const Vector& mins, const Vector& maxs, Int32 depth )
{
	// Create new node
	vertexnode_t* pnode = new vertexnode_t;

	// Always set mins/maxs
	pnode->mins = mins;
	pnode->maxs = maxs;

	// Add to the list
	m_vertexNodesArray.push_back(pnode);

	if(depth == MAX_NODE_DEPTH)
	{
		// Create vertex group
		vertexgroup_t* pgroup = new vertexgroup_t;
		m_vertexGroupPtrsArray.push_back(pgroup);
		
		// Mark as last node
		pnode->axis = -1;
		pnode->pchildren[0] = nullptr;
		pnode->pchildren[1] = nullptr;
		pnode->pgroup = pgroup;
		return pnode;
	}

	Vector size;
	Math::VectorSubtract(mins, maxs, size);

	if(size[0] > size[1])
		pnode->axis = 0;
	else
		pnode->axis = 1;

	pnode->dist = 0.5 * (maxs[pnode->axis] + mins[pnode->axis]);

	Vector mins1, maxs1;
	Vector mins2, maxs2;
	
	Math::VectorCopy(mins, mins1);
	Math::VectorCopy(maxs, maxs1);
	Math::VectorCopy(mins, mins2);
	Math::VectorCopy(maxs, maxs2);

	maxs1[pnode->axis] = pnode->dist;
	mins2[pnode->axis] = pnode->dist;

	pnode->pchildren[0] = CreateVertexNode(mins2, maxs2, depth+1);
	pnode->pchildren[1] = CreateVertexNode(mins1, maxs1, depth+1);

	return pnode;
}

//=============================================
// @brief
//
//=============================================
CNormalSmoothing::vertexnode_t* CNormalSmoothing::RecursiveAddVertex( vertexnode_t* pnode, const Vector& origin, const Vector& normal, Int32 index )
{
	if(!Math::PointInMinsMaxs(origin, pnode->mins, pnode->maxs))
		return nullptr;

	if(pnode->pgroup)
	{
		vertexgroup_t* pgroup = pnode->pgroup;

		Uint32 i = 0;
		for(; i < pgroup->numvertexes; i++)
		{
			vertex_t& vertex = pgroup->vertexes[i];

			if(Math::VectorCompare(origin, vertex.origin)
				&& Math::DotProduct(normal, vertex.normal) > m_normalBlend)
				break;
		}

		if(i == pgroup->numvertexes)
		{
			if(pgroup->numvertexes == pgroup->vertexes.size())
				pgroup->vertexes.resize(pgroup->vertexes.size() + CNM_ALLOCSIZE);

			vertex_t& vertex = pgroup->vertexes[pgroup->numvertexes];
			pgroup->numvertexes++;

			Math::VectorCopy(origin, vertex.origin);
			Math::VectorCopy(normal, vertex.normal);
		}
		else
		{
			vertex_t& vertex = pgroup->vertexes[i];
			Math::VectorScale(vertex.normal, 0.5, vertex.normal);
			Math::VectorMA(vertex.normal, 0.5, normal, vertex.normal);
		}

		m_pVertexIdxMapping[index] = i;
		return pnode;
	}
	else
	{
		if(origin[pnode->axis] > pnode->dist)
			return RecursiveAddVertex(pnode->pchildren[0], origin, normal, index);
		else
			return RecursiveAddVertex(pnode->pchildren[1], origin, normal, index);
	}
}

//=============================================
// @brief
//
//=============================================
void CNormalSmoothing::ManageVertex( const Vector& origin, const Vector& normal, Uint32 index )
{
	if(index > m_numVertexes)
		return;

	// Add this vertex recursively to the tree
	vertexnode_t* pnode = RecursiveAddVertex(m_vertexNodesArray[0], origin, normal, index);
	if(!pnode)
	{
		Con_Printf("%s - Failed to add vertex %d to a group.\n", __FUNCTION__, index);
		return;
	}

	// set group mapping
	m_pVertexGrpMapping[index] = pnode->pgroup;
}

//=============================================
// @brief
//
//=============================================
const Vector* CNormalSmoothing::GetVertexNormal( Uint32 index )
{
	if(index > m_numVertexes)
		return nullptr;

	vertexgroup_t* pgroup = m_pVertexGrpMapping[index];
	if(!pgroup)
		return nullptr;

	Uint32 vertexidx = m_pVertexIdxMapping[index];
	const vertex_t& vertex = pgroup->vertexes[vertexidx];

	return &vertex.normal;
}
