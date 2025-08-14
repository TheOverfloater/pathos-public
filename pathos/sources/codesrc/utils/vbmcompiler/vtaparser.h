/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef VTAPARSER_H
#define VTAPARSER_H

#include <map>
#include <set>

#include "compiler_types.h"
#include "studiocompiler.h"
#include "parserbase.h"

/*
=======================
CVTAParser

=======================
*/
class CVTAParser : public CParserBase
{
public:
	CVTAParser( CStudioModelCompiler& compiler, smdl::submodel_t* psubmodel, const CArray<smdl::bone_transforminfo_t>& bonetransforminfoarray );
	~CVTAParser( void );

public:
	// Processes a file
	virtual bool ProcessFile( const Char* pstrFilename ) override;

protected:
	// Parses nodes from the SMD file loaded
	bool ParseNodes( void );
	// Parses skeleton from the SMD file loaded
	bool ParseSkeleton( void );
	// Parses the vertex animations
	bool ParseVertexAnimation( void );

	// Link up flex vertex with it's triangle vertex
	void LinkFlexVertex( smdl::triangle_vertex_t* psrctrivertex, smdl::flexframe_t* pbaseflexframe );
	// Finalizes flex vertex data
	void FinalizeFlexData( CArray<smdl::flexframe_t*>& ptempflexarray );

public:
	// Reference to studiomodel compiler
	CStudioModelCompiler& m_studioCompiler;
	// Reference to bone transform info array from SMD parser
	const CArray<smdl::bone_transforminfo_t>& m_boneTransformInfoArray;

	// Submodel this VTA belongs to
	smdl::submodel_t* m_pSubModel;
};
#endif // VTAPARSER_H
