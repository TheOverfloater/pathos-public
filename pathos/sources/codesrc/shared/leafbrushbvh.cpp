/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "com_math.h"
#include "leafbrushbvh.h"
#include "collision_shared.h"
#include "brushmodel_shared.h"


//=============================================
// @brief Default constructor
//
//=============================================
CLeafBrushBVH::CLeafBrushBVH( mleaf_t* pleaf )
{
	Init(pleaf);
}

//=============================================
// @brief Destructor
//
//=============================================
CLeafBrushBVH::~CLeafBrushBVH( void )
{
	Clear();
}

//=============================================
// @brief Initializes BVH data
//
//=============================================
void CLeafBrushBVH::Init( mleaf_t* pleaf )
{
	// Collect leaf brushes
	for(Uint32 i = 0; i < pleaf->numleafbrushes; i++)
		m_pBrushArray.push_back(pleaf->pfirstleafbrush[i]);

	// Create root node
	brushbvhnode_t* prootnode = new brushbvhnode_t();
	prootnode->brushindexarray.resize(m_pBrushArray.size());
	for(Uint32 i = 0; i < m_pBrushArray.size(); i++)
		prootnode->brushindexarray[i] = i;

	// Create nodes
	prootnode->index = m_pBVHNodesArray.size();
	m_pBVHNodesArray.push_back(prootnode);

	UpdateBVHNodeBounds(prootnode);
	SubdivideBVHNode(prootnode);
}

//=============================================
// @brief Releases all data used
//
//=============================================
void CLeafBrushBVH::Clear( void )
{
	if(!m_pBVHNodesArray.empty())
	{
		for(Uint32 i = 0; i < m_pBVHNodesArray.size(); i++)
			delete m_pBVHNodesArray[i];

		m_pBVHNodesArray.clear();
	}

	if(!m_pBrushArray.empty())
		m_pBrushArray.clear();
}

//===============================================
// @brief Updates bounds of a BVH node
//
//===============================================
void CLeafBrushBVH::UpdateBVHNodeBounds( brushbvhnode_t* pnode )
{
	// Reset to null
	pnode->mins = NULL_MINS;
	pnode->maxs = NULL_MAXS;

	// Get all tris to calculate bounding box of this node
	for(Uint32 i = 0; i < pnode->brushindexarray.size(); i++)
	{
		Int32 brushindex = pnode->brushindexarray[i];
		mbrush_t* pbrush = m_pBrushArray[brushindex];

		for(Uint32 j = 0; j < 3; j++)
		{
			for(Uint32 k = 0; k < 3; k++)
			{
				if(pnode->mins[k] > pbrush->mins[k])
					pnode->mins[k] = pbrush->mins[k];

				if(pnode->maxs[k] < pbrush->maxs[k])
					pnode->maxs[k] = pbrush->maxs[k];
			}
		}
	}
}

//===============================================
// @brief Subdivides a BVH node
//
//===============================================
void CLeafBrushBVH::SubdivideBVHNode( brushbvhnode_t* pnode )
{
	// Find our longest axis
	Vector extents = (pnode->maxs - pnode->mins);
	
	Int32 j = 0;
	for(Uint32 i = 1; i < 3; i++)
	{
		if(extents[i] > extents[j])
			j = i;
	}

	// Assign brushes from parent to children based on split position
	Float splitPosition = pnode->mins[j] + extents[j] * 0.5;
	
	CArray<Int32> leftNodeBrushes;
	CArray<Int32> rightNodeBrushes;
	for(Uint32 i = 0; i < pnode->brushindexarray.size(); i++)
	{
		Int32 brushindex = pnode->brushindexarray[i];
		mbrush_t* pbrush = m_pBrushArray[brushindex];

		if(pbrush->centroid[j] < splitPosition)
			leftNodeBrushes.push_back(brushindex);
		else
			rightNodeBrushes.push_back(brushindex);
	}

	if(leftNodeBrushes.empty() || rightNodeBrushes.empty())
	{
		// If left or right node is empty, we're in a leaf node
		pnode->isleaf = true;
		return;
	}

	// Create left child node
	Int32 leftChildIndex = m_pBVHNodesArray.size();
	pnode->childnodes[0] = leftChildIndex;

	brushbvhnode_t* pLeftChild = new brushbvhnode_t();
	pLeftChild->index = leftChildIndex;
	pLeftChild->brushindexarray = leftNodeBrushes;
	m_pBVHNodesArray.push_back(pLeftChild);

	UpdateBVHNodeBounds(pLeftChild);

	// Create right child node
	Int32 rightChildIndex = m_pBVHNodesArray.size();
	pnode->childnodes[1] = rightChildIndex;

	brushbvhnode_t* pRightChild = new brushbvhnode_t();
	pRightChild->index = rightChildIndex;
	pRightChild->brushindexarray = rightNodeBrushes;
	m_pBVHNodesArray.push_back(pRightChild);

	UpdateBVHNodeBounds(pRightChild);

	// Clear this node, as it's not a leaf node
	pnode->brushindexarray.clear();
	pnode->isleaf = false;

	// Recurse further down the children
	SubdivideBVHNode(pLeftChild);
	SubdivideBVHNode(pRightChild);
}

//=============================================
// @brief Recurse down the tree with a point trace
//
//=============================================
void CLeafBrushBVH::RecurseTreePointTrace( const Vector& start, const Vector& end, const Vector& normdirection, const brushbvhnode_t* pbvhnode, CArray<mbrush_t*>& outBrushesArray, Uint32& numOutBrushes, Uint32 outAllocSize )
{
	if(!CollisionShared::IntersectBBoxPoint(start, end, pbvhnode->mins, pbvhnode->maxs, normdirection))
		return;

	if(pbvhnode->isleaf)
	{
		// If leaf, add brushes to the list
		AddNodeBrushes(pbvhnode, outBrushesArray, numOutBrushes, outAllocSize);
	}
	else
	{
		const brushbvhnode_t* pchildnode = m_pBVHNodesArray[pbvhnode->childnodes[0]];
		RecurseTreePointTrace(start, end, normdirection, pchildnode, outBrushesArray, numOutBrushes, outAllocSize);

		pchildnode = m_pBVHNodesArray[pbvhnode->childnodes[1]];
		RecurseTreePointTrace(start, end, normdirection, pchildnode, outBrushesArray, numOutBrushes, outAllocSize);	
	}
}

//=============================================
// @brief Recurse down the tree with a point trace
//
//=============================================
void CLeafBrushBVH::RecurseTreeAABBTrace( const Vector& start, const Vector& end, const Vector& extents, const brushbvhnode_t* pbvhnode, bool intersectTest, CArray<mbrush_t*>& outBrushesArray, Uint32& numOutBrushes, Uint32 outAllocSize )
{
	if(!intersectTest)
	{
		if(!CollisionShared::IntersectBBoxSweptAABB(start, end, pbvhnode->mins, pbvhnode->maxs, extents))
			return;
	}
	else
	{
		if(!CollisionShared::IntersectBBoxAABB(start, pbvhnode->mins, pbvhnode->maxs, extents))
			return;
	}

	if(pbvhnode->isleaf)
	{
		// If leaf, add brushes to the list
		AddNodeBrushes(pbvhnode, outBrushesArray, numOutBrushes, outAllocSize);
	}
	else
	{
		const brushbvhnode_t* pchildnode = m_pBVHNodesArray[pbvhnode->childnodes[0]];
		RecurseTreeAABBTrace(start, end, extents, pchildnode, intersectTest, outBrushesArray, numOutBrushes, outAllocSize);

		pchildnode = m_pBVHNodesArray[pbvhnode->childnodes[1]];
		RecurseTreeAABBTrace(start, end, extents, pchildnode, intersectTest, outBrushesArray, numOutBrushes, outAllocSize);	
	}
}

//=============================================
// @brief Add node's brushes to the list
//
//=============================================
void CLeafBrushBVH::AddNodeBrushes( const brushbvhnode_t* pbvhnode, CArray<mbrush_t*>& outBrushesArray, Uint32& numOutBrushes, Uint32 outAllocSize )
{
	for(Uint32 i = 0; i < pbvhnode->brushindexarray.size(); i++)
	{
		// Check if array needs to be extended
		if(numOutBrushes == outBrushesArray.size())
		{
			Uint32 arraySize = outBrushesArray.size()+outAllocSize;
			outBrushesArray.resize(arraySize);
		}

		Int32 brushindex = pbvhnode->brushindexarray[i];
		mbrush_t* pbrush = m_pBrushArray[brushindex];

		outBrushesArray[numOutBrushes] = pbrush;
		numOutBrushes++;
	}
}

//=============================================
// @brief Get brushes potentially impacted by line trace
//
//=============================================
void CLeafBrushBVH::GetLineTraceBrushes( const Vector& start, const Vector& end, CArray<mbrush_t*>& outBrushesArray, Uint32& numOutBrushes, Uint32 outAllocSize )
{
	// Reset
	numOutBrushes = 0;

	// Calculate direction
	Vector normdirection;
	Math::VectorSubtract(end, start, normdirection);
	normdirection.Normalize();

	RecurseTreePointTrace(start, end, normdirection, m_pBVHNodesArray[0], outBrushesArray, numOutBrushes, outAllocSize);
}

//=============================================
// @brief Get brushes potentially impacted by AABB trace
//
//=============================================
void CLeafBrushBVH::GetAABBTraceBrushes( const Vector& start, const Vector& end, const Vector& clipHullMins, const Vector& clipHullMaxs, CArray<mbrush_t*>& outBrushesArray, Uint32& numOutBrushes, Uint32 outAllocSize )
{
	// Reset
	numOutBrushes = 0;

	// Determine if we need an intersection test, or a swept test
	bool intersectTest = ((start - end).Length() > 0) ? true : false;
	// This actually needs to be the half-extents, not full extents
	Vector extents = (clipHullMaxs - clipHullMins) * 0.5;

	RecurseTreeAABBTrace(start, end, extents, m_pBVHNodesArray[0], intersectTest, outBrushesArray, numOutBrushes, outAllocSize);
}