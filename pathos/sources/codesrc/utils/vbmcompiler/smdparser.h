/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef SMDPARSER_H
#define SMDPARSER_H

#include <map>
#include <set>

#include "compiler_types.h"
#include "studiocompiler.h"
#include "parserbase.h"

/*
=======================
CSMDParser

=======================
*/
class CSMDParser : public CParserBase
{
public:
	CSMDParser( CStudioModelCompiler& compiler );
	~CSMDParser( void );

protected:
	// Parses nodes from the SMD file loaded
	bool ParseNodes( CArray<smdl::bone_node_t>& destArray );
	// Parses skeleton from the SMD file loaded
	bool ParseSkeleton( CArray<smdl::bone_node_t>& nodesarray, CArray<smdl::bone_t>& bonesarray, CArray<Int32>& boneimap, CArray<smdl::bone_transforminfo_t>& transformsarray );
	// Sets up the skeleton and it's matrices
	void SetupSkeleton( CArray<smdl::bone_node_t>& nodesarray, CArray<smdl::bone_t>& bonesarray, CArray<Int32>& boneimap, CArray<smdl::bone_transforminfo_t>& transformsarray );

public:
	// Reference to studiomodel compiler
	CStudioModelCompiler& m_studioCompiler;
};
#endif // SMDPARSER_H