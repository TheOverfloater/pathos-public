/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef R_NSMOOTHING_H
#define R_NSMOOTHING_H

#include "com_math.h"

/*
====================
CNormalSmoothing

====================
*/
class CNormalSmoothing
{
public:
	static const Float CNM_BBOXSIZE;
	static const Float CNM_ALLOCSIZE;
	static const Uint32 MAX_NODE_DEPTH;

public:
	struct vertex_t
	{
		Vector origin;
		Vector normal;
	};

	struct vertexgroup_t
	{
		vertexgroup_t():
			numvertexes(0)
			{}

		CArray<vertex_t> vertexes;
		Uint32 numvertexes;
	};

	struct vertexnode_t
	{
		vertexnode_t():
			axis(0),
			index(0),
			dist(0),
			pgroup(nullptr)
		{
			memset(pchildren, 0, sizeof(pchildren));
		}

		vertexnode_t(const vertexnode_t& src):
			axis(src.axis),
			index(src.index),
			dist(src.dist),
			mins(src.mins),
			maxs(src.maxs),
			pgroup(src.pgroup)
		{
			for(Uint32 i = 0; i < 2; i++)
				pchildren[i] = src.pchildren[i];
		}

		Int32 axis;
		Int32 index;
		Float dist;

		Vector mins;
		Vector maxs;

		vertexnode_t* pchildren[2];

		struct vertexgroup_t* pgroup;
	};

public:
	CNormalSmoothing( const Vector& worldMins, const Vector& worldMaxs, Uint32 numvertexes, Float blendangle );
	~CNormalSmoothing();

public:
	void ManageVertex( const Vector& origin, const Vector& normal, Uint32 index );
	const Vector* GetVertexNormal( Uint32 index );

private:
	vertexnode_t* CreateVertexNode( const Vector& mins, const Vector& maxs, Int32 depth );
	vertexnode_t* RecursiveAddVertex( vertexnode_t* pnode, const Vector& origin, const Vector& normal, Int32 index );

private:
	CArray<vertexgroup_t*> m_vertexGroupPtrsArray;

	Uint32* m_pVertexIdxMapping;
	vertexgroup_t** m_pVertexGrpMapping;
	Uint32 m_numVertexes;

	CArray<vertexnode_t*> m_vertexNodesArray;

	Float m_normalBlend;
};
#endif //R_NSMOOTHING