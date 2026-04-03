/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.

===============================================
*/

#ifndef VBMCONVERT_H
#define VBMCONVERT_H

#include "cbuffer.h"

/*
=======================
CMDLVBMConverter

=======================
*/
class CMDLVBMConverter
{
private:
	// Alloc size for VBM file buffer
	static const Uint32 VBM_TEMP_FILE_ALLOC_SIZE;
	// Vertex alloc size
	static const Uint32 VBM_VERTEXES_ALLOC_SIZE;
	// Triangle index alloc size
	static const Uint32 VBM_INDEXES_ALLOC_SIZE;
	// Triangle alloc size
	static const Uint32 VBM_TRIANGLES_ALLOC_SIZE;

private:
	struct studiovert_t
	{
		studiovert_t():
			vertindex(NO_POSITION),
			normindex(NO_POSITION),
			boneindex(0)
		{
			memset(texcoord, 0, sizeof(texcoord));
		}

		Int32 vertindex;
		Int32 normindex;
		Int32 texcoord[2];
		byte boneindex;
	};

	struct studiotri_t
	{
		studiotri_t():
			processed(false)
		{
		}

		bool processed;
		studiovert_t verts[3];
	};

	struct mesh_group_t
	{
		mesh_group_t():
			numtriangles(0),
			numbones(0),
			ptexture(nullptr),
			skinref(NO_POSITION),
			bonegrp(NO_POSITION)
		{
			memset(bones, 0, sizeof(bones));
		}

		CArray<studiotri_t> trianglesarray;
		Int32 numtriangles;

		byte bones[MAX_SHADER_BONES];
		Int32 numbones;

		const mstudiotexture_t *ptexture;
		Int32 skinref;
		Int32 bonegrp;
	};

public:
	// Constructor
	CMDLVBMConverter( void );
	// Destructor
	~CMDLVBMConverter( void );

public:
	// Converts a studiomodel to a VBM file
	bool ConvertModel( const Char* pstrModelFilePath, const Char* pstrOutputPath, const byte* pstudiomdlfile, const byte* ptexturemdlfile );
	// Resets the converter object
	void Reset( void );

	// Returns the number of bytes written for the last processed model
	Uint32 GetNbBytesWritten( void ) const { return m_nbBytesWritten; }

private:
	// Processes a studiomodel mesh into a VBM mesh
	void ProcessMesh( const mstudiomesh_t *pstudiomesh, const mstudiomodel_t *pstudiosubmodel, const mstudiotexture_t *pstudiotexture );
	// Processes a studiomodel bone's attached triangles into VBM data
	void RecursiveProcessBoneTriangles( Uint32 index, const mstudiobone_t *pstudiobone );
	// Processes a studiomodel bone's attached triangles into VBM data
	bool ProcessConversionGroups( vbmsubmodel_t* pvbmsubmodel );
	// Processes a studiomodel bone's attached triangles into VBM data
	bool ProcessVertex( vbmsubmodel_t* pvbmsubmodel, const studiovert_t *pvert, const mesh_group_t* pgroup );
	// Check if we can merge group bones into one
	void CheckMergeGroups( void );
	// Calculate tangent information for the mesh data
	void CalculateTangents( void );

private:
	// Buffer for file
	CBuffer*				m_pFileBuffer;
	// VBM file header pointer
	vbmheader_t*			m_pVBMHeader;

	// Pointer to studiomodel data
	const studiohdr_t*		m_pStudioHeader;
	// Pointer to texture studiomodel data
	const studiohdr_t*		m_pTextureHeader;

	// Pointer to studiomodel vertex data
	const Vector*			m_pStudioVertexes;
	// Pointer to studiomodel vertex normals data
	const Vector*			m_pStudioNormals;

	// Array of VBM vertexes
	CArray<vbmvertex_t>		m_vbmVertexesArray;
	// VBM vertex array load
	Uint32					m_numVBMVerts;

	// Array of VBM indexes
	CArray<Uint32>			m_vbmIndexesArray;
	// Index array load
	Uint32					m_numIndexes;

	// Array of conversion groups
	CArray<mesh_group_t*>	m_pConversionGroups;
	// Pointer to current conversion group
	mesh_group_t			*m_pCurrentGroup;

	// Array of conversion triangles
	CArray<studiotri_t>		m_conversionTrianglesArray;
	// Total number of conversion triangles
	Uint32					m_numConversionTriangles;
	// Number of triangles processed
	Uint32					m_numProcessed;

	// Array holding the bodypart's reference vertexes
	CArray<studiovert_t>	m_referenceVertexArray;
	// Number of reference vertexes used
	Uint32					m_numRefVerts;
	// Current start vertex index
	Uint32					m_currentStartVertex;

	// Number of bytes written for the last model processed
	Uint32					m_nbBytesWritten;
};

extern CMDLVBMConverter gMDLVBMConverter;
#endif