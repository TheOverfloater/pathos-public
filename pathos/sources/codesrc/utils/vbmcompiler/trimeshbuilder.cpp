/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include <SDL.h>
#ifdef WIN32
#include <Windows.h>
#endif

#include "includes.h"
#include "studiocompiler.h"
#include "trimeshbuilder.h"
#include "main.h"
#include "compiler_math.h"

// Strip allocation size
const Uint32 CTriangleMeshBuilder::STRIP_ALLOC_SIZE = 512;
// Strip allocation size
const Uint32 CTriangleMeshBuilder::TRICMD_ALLOC_SIZE = 1024;

//===============================================
// @brief Constructor for CGeometrySMDParser class
//
// @param pmesh Mesh to build triangle mesh for
//===============================================
CTriangleMeshBuilder::CTriangleMeshBuilder( const smdl::mesh_t* pmesh ):
	m_numStrips(0),
	m_pMesh(pmesh),
	m_pTriangles(&pmesh->trivertexes[0]),
	m_numCommands(0)
{
	Uint32 triangleCount = pmesh->trivertexes.size()/3;
	m_triangleInfosArray.resize(triangleCount);

	m_stripsArray.resize(STRIP_ALLOC_SIZE);
	m_commandsArray.resize(TRICMD_ALLOC_SIZE);

	for(Uint32 i = 0; i < m_triangleInfosArray.size(); i++)
		m_triangleInfosArray[i].peak = m_triangleInfosArray.size();
}

//===============================================
// @brief Destructor for CGeometrySMDParser class
//
//===============================================
CTriangleMeshBuilder::~CTriangleMeshBuilder( void )
{
}

//===============================================
// @brief Finds neighbor triangle
//
// @param startTriangle Starting triangle's index
// @param startVertex First vertex if the edge to check for
//===============================================
void CTriangleMeshBuilder::FindNeighbor( Int32 startTriangle, Int32 startVertex )
{
	const smdl::triangle_vertex_t* plaststarttrivertex = &m_pTriangles[startTriangle*3];

	// Get first edge vertex
	Int32 m1index = (startVertex + 1) % 3;
	smdl::triangle_vertex_t m1 = plaststarttrivertex[m1index];

	// Get second edge vertex
	Int32 m2index = (startVertex) % 3;
	smdl::triangle_vertex_t m2 = plaststarttrivertex[m2index];

	// Starting triangle
	triangleinfo_t& startTri = m_triangleInfosArray[startTriangle];

	Int32 nextTriVertex = (startTriangle + 1) * 3;
	const smdl::triangle_vertex_t* pcheck = &m_pTriangles[nextTriVertex];
	Uint32 triangleCount = m_pMesh->trivertexes.size() / 3;
	for(Uint32 i = startTriangle + 1; i < triangleCount; i++, pcheck += 3)
	{
		triangleinfo_t& triinfo = m_triangleInfosArray[i];

		// check if all neighbors are used
		if(triinfo.usedbits == 7)
			continue;

		for(Uint32 j = 0; j < 3; j++)
		{
			if(memcmp(&pcheck[j], &m1, sizeof(smdl::triangle_vertex_t))
				|| memcmp(&pcheck[(j+1)%3], &m2, sizeof(smdl::triangle_vertex_t)))
				continue;

			startTri.neighbortriangles[startVertex] = i;
			startTri.neighboredges[startVertex] = j;
			startTri.usedbits |= (1<<startVertex);

			triangleinfo_t& neighborTri = m_triangleInfosArray[i];
			neighborTri.neighbortriangles[j] = startTriangle;
			neighborTri.neighboredges[j] = startVertex;
			neighborTri.usedbits |= (1<<j);
			return;
		}
	}
}

//===============================================
// @brief Build a triangle strip and return it's length
//
// @param startTriangle Starting triangle's index
// @param startVertex First vertex if the edge to check for
// @return Length of the triangle strip
//===============================================
Uint32 CTriangleMeshBuilder::GetStripLength( Int32 startTriangle, Int32 startVertex )
{
	if((m_numStrips+3) >= m_stripsArray.size())
		m_stripsArray.resize(m_stripsArray.size()+STRIP_ALLOC_SIZE);

	m_numStrips = 0;
	for(Uint32 i = 0; i < 3; i++)
	{
		strip_t& strip = m_stripsArray[m_numStrips];
		m_numStrips++;

		strip.vertex = (startVertex + i) % 3;
		strip.triangle = startTriangle;
	}

	triangleinfo_t& startTri = m_triangleInfosArray[startTriangle];
	startTri.usedbits = 2;

	Int32 currentTriangleIndex = startTriangle;
	Int32 currentVertexIndex = startVertex;

	while(true)
	{
		Int32 index;
		if(m_numStrips % 2 != 0)
			index = (currentVertexIndex + 1) % 3;
		else
			index = (currentVertexIndex + 2) % 3;

		triangleinfo_t& currentTriangle = m_triangleInfosArray[currentTriangleIndex];
		Int32 neighborTriangleIndex = currentTriangle.neighbortriangles[index];
		Int32 neighborEdge = currentTriangle.neighboredges[index];

		if(neighborTriangleIndex == NO_POSITION)
			break;

		triangleinfo_t& neighborTri = m_triangleInfosArray[neighborTriangleIndex];
		if(neighborTri.usedbits)
			break;

		if(m_numStrips == m_stripsArray.size())
			m_stripsArray.resize(m_stripsArray.size()+STRIP_ALLOC_SIZE);

		strip_t& strip = m_stripsArray[m_numStrips];
		m_numStrips++;

		strip.vertex = (neighborEdge + 2) % 3;
		strip.triangle = neighborTriangleIndex;

		neighborTri.usedbits = 2;

		currentTriangleIndex = neighborTriangleIndex;
		currentVertexIndex = neighborEdge;
	}

	for(Uint32 i = 0; i < m_triangleInfosArray.size(); i++)
	{
		triangleinfo_t& triangle = m_triangleInfosArray[i];
		if(triangle.usedbits == 2)
			triangle.usedbits = 0;
	}

	return m_numStrips;
}

//===============================================
// @brief Build a triangle fan and return it's length
//
// @param startTriangle Starting triangle's index
// @param startVertex First vertex if the edge to check for
// @return Length of the triangle fan
//===============================================
Uint32 CTriangleMeshBuilder::GetFanLength( Int32 startTriangle, Int32 startVertex )
{
	if((m_numStrips+3) >= m_stripsArray.size())
		m_stripsArray.resize(m_stripsArray.size()+STRIP_ALLOC_SIZE);

	m_numStrips = 0;
	for(Uint32 i = 0; i < 3; i++)
	{
		strip_t& strip = m_stripsArray[m_numStrips];
		m_numStrips++;

		strip.vertex = (startVertex + i) % 3;
		strip.triangle = startTriangle;
	}

	triangleinfo_t& startTri = m_triangleInfosArray[startTriangle];
	startTri.usedbits = 2;

	Int32 currentTriangleIndex = startTriangle;
	Int32 currentVertexIndex = startVertex;

	while(true)
	{
		Int32 index = (currentVertexIndex + 2) % 3;
		triangleinfo_t& currentTriangle = m_triangleInfosArray[currentTriangleIndex];

		Int32 neighborTriangleIndex = currentTriangle.neighbortriangles[index];
		Int32 neighborEdge = currentTriangle.neighboredges[index];

		if(neighborTriangleIndex == NO_POSITION)
			break;

		triangleinfo_t& neighborTri = m_triangleInfosArray[neighborTriangleIndex];
		if(neighborTri.usedbits)
			break;

		if(m_numStrips == m_stripsArray.size())
			m_stripsArray.resize(m_stripsArray.size()+STRIP_ALLOC_SIZE);

		strip_t& strip = m_stripsArray[m_numStrips];
		m_numStrips++;

		strip.vertex = (neighborEdge + 2) % 3;
		strip.triangle = neighborTriangleIndex;

		neighborTri.usedbits = 2;

		currentTriangleIndex = neighborTriangleIndex;
		currentVertexIndex = neighborEdge;
	}

	for(Uint32 i = 0; i < m_triangleInfosArray.size(); i++)
	{
		triangleinfo_t& triangle = m_triangleInfosArray[i];
		if(triangle.usedbits == 2)
			triangle.usedbits = 0;
	}

	return m_numStrips;
}

//===============================================
// @brief Build triangle mesh
//
// @param poutdata Reference to pointer that'll hold the output data
// @return Size of the output data in bytes
//===============================================
Uint32 CTriangleMeshBuilder::BuildTriangleMesh( byte*& poutdata )
{
	CArray<Int32> bestVertexes(m_triangleInfosArray.size());
	CArray<Int32> bestTriangles(m_triangleInfosArray.size());

	for(Uint32 i = 0; i < m_triangleInfosArray.size(); i++)
	{
		triangleinfo_t& triangle = m_triangleInfosArray[i];
		for(Uint32 j = 0; j < 3; j++)
		{
			if(triangle.usedbits & (1<<j))
				continue;

			FindNeighbor(i, j);
		}
	}

	// Reset all used flags
	for(Uint32 i = 0; i < m_triangleInfosArray.size(); i++)
		m_triangleInfosArray[i].usedbits = 0;

	Uint32 i = 0;
	while(i < m_triangleInfosArray.size())
	{
		triangleinfo_t& triangle1 = m_triangleInfosArray[i];
		if(triangle1.usedbits)
		{
			i++;
			continue;
		}

		Uint32 maxlength = -1;
		Int32 bestlength = 0;
		Int32 besttype = 0;

		for(Uint32 k = i; k < m_triangleInfosArray.size() && bestlength < 127; k++)
		{
			triangleinfo_t& triangle2 = m_triangleInfosArray[k];
			if(triangle2.usedbits)
				continue;

			if(triangle2.peak <= bestlength)
				continue;

			Uint32 localpeak = 0;
			for(Uint32 l = 0; l < 2; l++)
			{
				for(Uint32 m = 0; m < 3; m++)
				{
					Uint32 length;
					if(l == 0)
						length = GetStripLength(k, m);
					else
						length = GetFanLength(k, m);

					if(length > bestlength)
					{
						besttype = l;
						bestlength = length;

						if(bestTriangles.size() < bestlength)
							bestTriangles.resize(bestlength);

						if(bestVertexes.size() < bestlength)
							bestVertexes.resize(bestlength);

						for(Uint32 n = 0; n < bestlength; n++)
						{
							strip_t& strip = m_stripsArray[n];
							bestTriangles[n] = strip.triangle;
							bestVertexes[n] = strip.vertex;
						}
					}

					if(length > localpeak)
						localpeak = length;
				}
			}

			triangle2.peak = localpeak;
			if(localpeak == maxlength)
				break;
		}

		maxlength = bestlength;

		for(Uint32 k = 0; k < bestlength; k++)
			m_triangleInfosArray[bestTriangles[k]].usedbits = 1;

		// Ensure we have enough space in the array
		Uint32 addCount = (1 + bestlength * 4);
		if((m_numCommands + addCount) >= m_commandsArray.size())
			m_commandsArray.resize(m_commandsArray.size() + TRICMD_ALLOC_SIZE);

		// Strip is marked by a negative value, fan is positive
		if(besttype == 1)
			m_commandsArray[m_numCommands] = -bestlength;
		else
			m_commandsArray[m_numCommands] = bestlength;

		m_numCommands++;

		for(Uint32 k = 0; k < bestlength; k++)
		{
			Int32 triangleVertex = (bestTriangles[k] * 3) + bestVertexes[k];
			const smdl::triangle_vertex_t* pvertex = &m_pTriangles[triangleVertex];

			m_commandsArray[m_numCommands] = pvertex->vertexindex;
			m_numCommands++;

			m_commandsArray[m_numCommands] = pvertex->normalindex;
			m_numCommands++;

			for(Uint32 l = 0; l < 2; l++)
			{
				m_commandsArray[m_numCommands] = pvertex->int_texcoords[l];
				m_numCommands++;
			}
		}
	}

	// Terminate tricmd sequence with a zero amount
	m_commandsArray.push_back(0);
	m_numCommands++;

	// Set output data and return the size of the data
	poutdata = reinterpret_cast<byte*>(&m_commandsArray[0]);
	return m_numCommands * sizeof(Int16);
}