/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef GEOMETRYSMDPARSER_H
#define GEOMETRYSMDPARSER_H

#include "smdparser.h"

/*
=======================
CGeometrySMDParser

=======================
*/
class CGeometrySMDParser : public CSMDParser
{
public:
	CGeometrySMDParser( CStudioModelCompiler& compiler, smdl::submodel_t* psubmodel );
	~CGeometrySMDParser( void );

public:
	// Processes a file
	virtual bool ProcessFile( const Char* pstrFilename ) override;

public:
	// Return the bone transform info array
	const CArray<smdl::bone_transforminfo_t>& GetBoneTransformInfoArray( void ) const;

private:
	// Parses the triangles from the SMD file loaded
	bool ParseTriangles( void );
	// Re-sorts normals to be organized by texture
	void ResortNormals( void );

private:
	// Pointer to submodel to load for
	smdl::submodel_t* m_pSubModel;
	// Array of bone transformation infos
	CArray<smdl::bone_transforminfo_t> m_boneTransformInfoArray;
};
#endif // GEOMETRYSMDPARSER_H