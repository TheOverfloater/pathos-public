/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef COLLISIONSMDPARSER_H
#define COLLISIONSMDPARSER_H

#include "smdparser.h"
#include "mcdcompiler.h"

/*
=======================
CCollisionSMDParser

=======================
*/
class CCollisionSMDParser : public CSMDParser
{
public:
	CCollisionSMDParser( CStudioModelCompiler& studioCompiler, CMCDCompiler& mcdCompiler, mcd::submodel_t* psubmodel, bool reversetriangles );
	~CCollisionSMDParser( void );

public:
	// Processes a file
	virtual bool ProcessFile( const Char* pstrFilename ) override;

private:
	// Parses the triangles from the SMD file loaded
	bool ParseTriangles( void );
	// Sets bone index mappings
	bool SetBoneIndexMappings( void );
	// Checks for triangles with no neighbors
	bool CheckTriangleNeighbors( void );

private:
	// Pointer to submodel to load for
	mcd::submodel_t* m_pSubModel;
	// TRUE if we want to reverse triangles
	bool m_reverseTriangles;
	// Array of bone transformation infos
	CArray<smdl::bone_transforminfo_t> m_boneTransformInfoArray;
	// MCD compiler object
	CMCDCompiler& m_mcdCompiler;
};
#endif // COLLISIONSMDPARSER_H