/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef MCDCOMPILER_H
#define MCDCOMPILER_H

#include <map>
#include <set>

#include "compiler_types.h"
#include "studiocompiler.h"
#include "mcdformat.h"

/*
=======================
CMCDCompiler

=======================
*/
class CMCDCompiler
{
public:
	// File buffer allocation size
	static const Uint32 MCD_FILEBUFFER_ALLOC_SIZE;

public:
	CMCDCompiler( CStudioModelCompiler& studioCompiler );
	~CMCDCompiler( void );

public:
	// Processes and writes the MCD file
	bool CreateMCDFile( void );
	// Clears any data used by the class
	void Clear( void );

	// Returns a texture skinref for a texture name
	Int32 GetTextureIndex( const Char* pstrTextureName );

private:
	// Creates clipping hull data from triangle data
	void CreateSubmodelBVH( void );
	// Updates bounds of a BVH node
	void UpdateBVHNodeBounds( mcd::bvhnode_t* pnode );
	// Subdivides a BVH node
	void SubdivideBVHNode( mcd::bvhnode_t* pnode );
	// Writes the final output
	bool WriteFile( void );

private:
	// Studiomodel compiler object
	CStudioModelCompiler& m_studioCompiler;

	// Bone transforms array
	CArray<smdl::bone_transforminfo_t> m_boneTransformInfoArray;
	// Array of submodels
	CArray<mcd::submodel_t*> m_pSubmodelsArray;
	// Bodyparts array
	CArray<mcd::bodypart_t*> m_pBodyPartsArray;
	// Array of textures
	CArray<CString> m_texturesArray;
	// Array of bones
	CArray<mcd::bone_t> m_bonesArray;

	// Current submodel
	mcd::submodel_t* m_pSubModel;

	// File buffer for writing the MCD file
	CBuffer* m_pFileBuffer;
	// MCD header we're writing to
	mcdheader_t* m_pMCDHeader;
};
#endif //MCDCOMPILER_H