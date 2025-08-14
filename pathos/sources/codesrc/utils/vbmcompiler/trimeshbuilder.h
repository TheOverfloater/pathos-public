/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef TRIMESHBUILDER_H
#define TRIMESHBUILDER_H

#include "includes.h"
#include "compiler_types.h"
#include "studiocompiler.h"

/*
=======================
CTriangleMeshBuilder

=======================
*/
class CTriangleMeshBuilder
{
public:
	// Strip allocation size
	static const Uint32 STRIP_ALLOC_SIZE;
	// Strip allocation size
	static const Uint32 TRICMD_ALLOC_SIZE;

	struct strip_t
	{
		strip_t():
			vertex(NO_POSITION),
			triangle(NO_POSITION)
		{}

		Int32 vertex;
		Int32 triangle;
	};

	struct triangleinfo_t
	{
		triangleinfo_t():
			usedbits(0),
			peak(0)
		{
			for(Uint32 i = 0; i < 3; i++)
			{
				neighbortriangles[i] = NO_POSITION;
				neighboredges[i] = 0;
			}
		}

		Int32 neighbortriangles[3];
		Int32 neighboredges[3];
		Int32 usedbits;
		Uint32 peak;
	};

public:
	CTriangleMeshBuilder( const smdl::mesh_t* pmesh );
	~CTriangleMeshBuilder( void );

	// Finds neighbor triangle
	void FindNeighbor( Int32 startTriangle, Int32 startVertex );
	// Get length of a triangle strip
	Uint32 GetStripLength( Int32 startTriangle, Int32 startVertex );
	// Get length of a triangle fan
	Uint32 GetFanLength( Int32 startTriangle, Int32 startVertex );
	// Build triangle mesh
	Uint32 BuildTriangleMesh( byte*& poutdata );

private:
	// Array of strip data
	CArray<strip_t> m_stripsArray;
	// Strip count
	Uint32 m_numStrips;

	// Array of triangle info arrays
	CArray<triangleinfo_t> m_triangleInfosArray;

	// Pointer to triangle data
	const smdl::triangle_vertex_t* m_pTriangles;
	// Pointer to current mesh
	const smdl::mesh_t* m_pMesh;

	// Triangle command buffer
	CArray<Int16> m_commandsArray;
	// Number of commands in buffer
	Uint32 m_numCommands;
};
#endif // TRIMESHBUILDER_H