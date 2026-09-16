/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef LEAFBRUSHBVH_H
#define LEAFBRUSHBVH_H

struct mbrush_t;
struct mleaf_t;

/*
=======================
CLeafBrushBVH

=======================
*/
class CLeafBrushBVH
{
private:
	struct brushbvhnode_t
	{
		brushbvhnode_t():
			index(NO_POSITION),
			isleaf(false)
		{
			for(Uint32 i = 0; i < 2; i++)
				childnodes[i] = NO_POSITION;
		}

		Int32 index;
		Vector mins;
		Vector maxs;

		Int32 childnodes[2];
		bool isleaf;

		CArray<Int32> brushindexarray;
	};

public:
	CLeafBrushBVH( mleaf_t* pleaf );
	~CLeafBrushBVH( void );

public:
	// Get brushes potentially impacted by line trace
	void GetLineTraceBrushes( const Vector& start, const Vector& end, CArray<mbrush_t*>& outBrushesArray, Uint32& numOutBrushes, Uint32 outAllocSize );
	// Get brushes potentially impacted by AABB trace
	void GetAABBTraceBrushes( const Vector& start, const Vector& end, const Vector& clipHullMins, const Vector& clipHullMaxs, CArray<mbrush_t*>& outBrushesArray, Uint32& numOutBrushes, Uint32 outAllocSize );
	// Return the number of BVH nodes
	Uint32 GetNodeCount( void ) const { return m_pBVHNodesArray.size(); }

private:
	// Initializes BVH data
	void Init( mleaf_t* pleaf );
	// Releases all data used
	void Clear( void );

	// Updates bounds of a BVH node
	void UpdateBVHNodeBounds( brushbvhnode_t* pnode );
	// Subdivides a BVH node
	void SubdivideBVHNode( brushbvhnode_t* pnode );

private:
	// Add triangle to the list
	void AddNodeBrushes( const brushbvhnode_t* pbvhnode, CArray<mbrush_t*>& outBrushesArray, Uint32& numOutBrushes, Uint32 outAllocSize );
	// Recurse down the tree with a point trace
	void RecurseTreePointTrace( const Vector& start, const Vector& end, const Vector& normdirection, const brushbvhnode_t* pbvhnode, CArray<mbrush_t*>& outBrushesArray, Uint32& numOutBrushes, Uint32 outAllocSize );
	// Recurse down the tree with a point trace
	void RecurseTreeAABBTrace( const Vector& start, const Vector& end, const Vector& extents, const brushbvhnode_t* pbvhnode, bool intersectTest, CArray<mbrush_t*>& outBrushesArray, Uint32& numOutBrushes, Uint32 outAllocSize );

private:
	// BVH nodes array
	CArray<brushbvhnode_t*> m_pBVHNodesArray;
	// Array of brushes
	CArray<mbrush_t*> m_pBrushArray;
};

#endif