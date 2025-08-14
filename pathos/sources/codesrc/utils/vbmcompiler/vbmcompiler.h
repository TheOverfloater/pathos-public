/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef VBMCOMPILER_H
#define VBMCOMPILER_H

#include <map>
#include <set>

#include "compiler_types.h"
#include "studiocompiler.h"

/*
=======================
CVBMCompiler

=======================
*/
class CVBMCompiler
{
public:
	// File buffer allocation size
	static const Uint32 VBM_FILEBUFFER_ALLOC_SIZE;
	// Triangle alloc size
	static const Uint32 TRIANGLE_ALLOC_SIZE;
	// Index buffer alloc size
	static const Uint32 INDEX_BUFFER_ALLOC_SIZE;
	// Vertex buffer alloc size
	static const Uint32 VERTEX_BUFFER_ALLOC_SIZE;
	// Reference frame file name
	static const Char REFERENCE_FRAME_FILENAME[];

private:
	struct conversion_vertex_t
	{
		conversion_vertex_t():
			vertindex(NO_POSITION),
			normindex(NO_POSITION),
			flexindex(NO_POSITION),
			weightindex(NO_POSITION),
			refboneindex(NO_POSITION)
		{
			for(Uint32 i = 0; i < 2; i++)
				texcoords[i] = 0;
		}
			
		Int32 vertindex;
		Int32 normindex;
		Int32 flexindex;
		Float texcoords[2];
		Int32 weightindex;
		Int32 refboneindex;
	};

	struct conversion_triangle_t
	{
		conversion_triangle_t():
			processed(false)
		{}

		conversion_vertex_t vertexes[3];
		bool processed;
	};

	struct final_vertex_t
	{
		final_vertex_t():
			flexvertexindex(NO_POSITION),
			numweights(0),
			refboneindex(NO_POSITION)
		{
			for(Uint32 i = 0; i < MAX_VBM_BONEWEIGHTS; i++)
			{
				boneindexes[i] = 0;
				boneweights[i] = 0;
			}

			for(Uint32 i = 0; i < 4; i++)
				tangent[i] = 0;
		}

		Vector origin;
		Vector normal;
		vec4_t tangent;
		Float texcoords[2];
		Int32 flexvertexindex;


		Float boneindexes[MAX_VBM_BONEWEIGHTS];
		Float boneweights[MAX_VBM_BONEWEIGHTS];
		Uint32 numweights;

		Int32 refboneindex;
	};

	struct conversion_group_t
	{
		conversion_group_t():
			numtriangles(0),
			numbones(0),
			ptexture(nullptr),
			skinref(NO_POSITION),
			bonegroup(NO_POSITION)
		{
			memset(boneindexes, 0, sizeof(boneindexes));
		}

		CString name;

		CArray<conversion_triangle_t> triangles;
		Uint32 numtriangles;

		Int32 boneindexes[MAX_SHADER_BONES];
		Uint32 numbones;

		vbmtexture_t* ptexture;

		Int32 skinref;
		Int32 bonegroup;
	};

public:
	CVBMCompiler( CStudioModelCompiler& studioCompiler );
	~CVBMCompiler( void );

public:
	// Processes and writes the VBM file
	bool CreateVBMFile( void );
	// Clears any data used by the class
	void Clear( void );

	// Adds a new flex controller
	bool AddFlexController( const Char* pstrControllerName, Float minValue, Float maxValue, vbmflexinterp_t interpType, const CArray<vbm::flexcontroller_vta_t> vtaArray );

private:
	// Processes a single vertex
	bool ProcessVertex( Int32 startVertexIndex, conversion_group_t* pbonegroup, const conversion_vertex_t* pvertex );
	// Processes a submodel's data into VBM form
	bool ProcessCurrentSubmodel( void );
	// Finalizes a submodel's data
	bool FinalizeCurrentSubmodel( void );
	// Processes and/or splits a mesh to keep it under the bones limit
	void ProcessMesh( smdl::mesh_t* psrcmesh, vbmtexture_t* ptexture );
	// Get bones required by a triangle
	void GetAddBones( CArray<Int32>& addbonesarray, const conversion_triangle_t& triangle );
	// Recurse through a bone and add it's associated triangles, then go through the children
	void RecursiveAddBoneTriangles( Int32 boneindex, const smdl::boneinfo_t* psrcbone );
	// Calculate tangents for the vertexes
	void CalculateTangents( vbmvertex_t* pfinalvertexes );
	// Optimize mesh data, check for bone merging, etc
	void OptimizeMeshes( void );
	// Write bone info in the VBM file
	void WriteBoneData( void );
	// Write VBM flex controller data
	void WriteFlexControllers( void );
	// Write flex data to the buffer
	void WriteFlexData( void );

	// Allocates a new conversion group
	conversion_group_t* AllocConversionGroup( void );
	// Convert weight float to byte data
	byte WeightToByte( Float value );

	// Loads reference frame file
	bool LoadReferenceFrameFile( void );
	// Writes the final output
	bool WriteFile( void );

private:
	// Studiomodel compiler object
	CStudioModelCompiler& m_studioCompiler;
	// Skinrefs array
	const CArray<CArray<Int32>>& m_skinRefsArray;

	// Flex controller array
	CArray<vbm::flexcontroller_t> m_flexControllerArray;

	// VBM header pointer
	vbmheader_t* m_pVBMHeader;

	// Conversion triangles array
	CArray<conversion_triangle_t> m_conversionTrianglesArray;
	// Number of conversion triangles
	Uint32 m_nbConversionTriangles;

	// Current conversion group
	conversion_group_t* m_pCurrentConversionGroup;
	// Conversion groups array
	CArray<conversion_group_t*> m_pConversionGroupsArray;

	// Array of source submodels
	CArray<const smdl::submodel_t*> m_pSrcSubmodelsArray;

	// Current source submodel
	const smdl::submodel_t* m_pSourceSubmodel;
	// Current destination submodel
	vbmsubmodel_t* m_pDestinationSubmodel;
	// Current flex info object
	vbmflexinfo_t* m_pDestFlexInfo;

	// Current flex index
	Int32 m_currentFlexIndex;
	// Reference frame object
	vbm::ref_frameinfo_t m_referenceFrame;

	// Index buffer array
	CArray<Uint32> m_indexesArray;
	// Number of indexes in array
	Uint32 m_numIndexes;

	// Vertex buffer array
	CArray<final_vertex_t> m_vertexesArray;
	// Number of vertexes in array
	Uint32 m_numVertexes;

	// File buffer for writing the VBM file
	CBuffer* m_pFileBuffer;
};
#endif //VBMCOMPILER_H