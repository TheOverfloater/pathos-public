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
#include "refframesmdparser.h"
#include "options.h"
#include "main.h"
#include "vbm_shared.h"
#include "compiler_math.h"

//===============================================
// @brief Constructor for CReferenceFrameSMDParser class
//
// @param compiler Reference to studiomodel compiler object
// @param referenceFrameInfo Reference frame object that'll hold our data
//===============================================
CReferenceFrameSMDParser::CReferenceFrameSMDParser( CStudioModelCompiler& compiler, vbm::ref_frameinfo_t& referenceFrameInfo ):
	CSMDParser(compiler),
	m_referenceFrameInfo(referenceFrameInfo)
{
}

//===============================================
// @brief Destructor for CReferenceFrameSMDParser class
//
//===============================================
CReferenceFrameSMDParser::~CReferenceFrameSMDParser( void )
{
}

//===============================================
// @brief Loads and processes an SMD file
//
// @param pstrFilename File path
// @return TRUE if successful, FALSE otherwise
//===============================================
bool CReferenceFrameSMDParser::ProcessFile( const Char* pstrFilename )
{
	CString filePath;
	filePath << pstrFilename << ".smd";
	if(!OpenScriptFile(filePath.c_str()))
	{
		ErrorMsg("Failed to open '%s' for processing.\n", filePath.c_str());
		Clear();
		return false;
	}

	Msg("Processing '%s'.\n", m_scriptFileName.c_str());

	// First read the version
	const Char* pstrString = nullptr;
	if(!ReadString(pstrString))
	{
		ErrorMsg("Failed to read 'version' token in %s'.\n", m_scriptFileName.c_str());
		Clear();
		return false;
	}

	if(qstrcicmp(pstrString, "version"))
	{
		ErrorMsg("Expected 'version', got '%s' instead in '%s'.\n", pstrString, m_scriptFileName.c_str());
		Clear();
		return false;
	}

	// Now read the version number
	Int32 versionNumber = 0;
	if(!ReadInt32(versionNumber))
	{
		ErrorMsg("Failed to read 'version' number token in %s'.\n", m_scriptFileName.c_str());
		Clear();
		return false;
	}

	if(versionNumber != SMD_VERSION)
	{
		ErrorMsg("SMD file '%s' has invalid version %d, %d expected.\n", m_scriptFileName.c_str(), versionNumber, SMD_VERSION);
		Clear();
		return false;
	}

	// Now parse the contents of the script
	while(true)
	{
		if(!ReadString(pstrString, true))
			break;

		bool result = false;
		if(!qstrcmp(pstrString, "nodes"))
		{
			result = ParseNodes(m_referenceFrameInfo.nodes);
		}
		else if(!qstrcmp(pstrString, "skeleton"))
		{
			result = ParseSkeleton(m_referenceFrameInfo.nodes, m_referenceFrameInfo.bones, m_referenceFrameInfo.bonemap_inverse, m_referenceFrameInfo.bonetransforms);
		}
		else
		{
			ErrorMsg("Unknown token '%s' in %s'.\n", pstrString, m_scriptFileName.c_str());
			Clear();
			return false;
		}

		if(!result)
		{
			ErrorMsg("Failure encountered when parsing '%s'.\n", m_scriptFileName.c_str());
			Clear();
			return false;
		}
	}

	// Link bone index map up
	for(Uint32 i = 0; i < m_referenceFrameInfo.bones.size(); i++)
	{
		smdl::bone_t& bone = m_referenceFrameInfo.bones[i];
		smdl::bone_node_t& node = m_referenceFrameInfo.nodes[i];

		const smdl::boneinfo_t* pglobalbone = m_studioCompiler.GetBone(node.bonename.c_str());
		if(!pglobalbone)
		{
			bone.globalindex = NO_POSITION;
		}
		else
		{
			bone.globalindex = pglobalbone->index;
			m_referenceFrameInfo.bonemap_inverse[pglobalbone->index] = i;
		}
	}

	// Next up, find global bone array bones in out list
	bool result = true;
	Uint32 nbGlobalBones = m_studioCompiler.GetNbBones();
	for(Uint32 i = 0; i < nbGlobalBones; i++)
	{
		const smdl::boneinfo_t* pglobalbone = m_studioCompiler.GetBone(i);

		Uint32 j = 0;
		for(; j < m_referenceFrameInfo.nodes.size(); j++)
		{
			smdl::bone_node_t& refnode = m_referenceFrameInfo.nodes[j];
			if(!qstrcmp(refnode.bonename, pglobalbone->name))
				break;
		}

		if(j == m_referenceFrameInfo.nodes.size())
		{
			ErrorMsg("Couldn't find bone '%s' in '%s.smd' bone list.\n", pglobalbone->name.c_str(), pstrFilename);
			result = false;
		}
	}

	return result;
}
