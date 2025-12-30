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
#include "collisionsmdparser.h"
#include "options.h"
#include "main.h"
#include "vbm_shared.h"
#include "compiler_math.h"

//===============================================
// @brief Constructor for CCollisionSMDParser class
//
// @param compiler Reference to studio model compiler object
// @psubmodel Submodel to parse data for
//===============================================
CCollisionSMDParser::CCollisionSMDParser( CStudioModelCompiler& studioCompiler, CMCDCompiler& mcdCompiler, mcd::submodel_t* psubmodel, bool reversetriangles ):
	CSMDParser(studioCompiler),
	m_pSubModel(psubmodel),
	m_reverseTriangles(reversetriangles),
	m_mcdCompiler(mcdCompiler)
{
}

//===============================================
// @brief Destructor for CCollisionSMDParser class
//
//===============================================
CCollisionSMDParser::~CCollisionSMDParser( void )
{
}

//===============================================
// @brief Loads and processes an SMD file
//
// @param pstrFilename File path
// @return TRUE if successful, FALSE otherwise
//===============================================
bool CCollisionSMDParser::ProcessFile( const Char* pstrFilename )
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
		if(!qstrcicmp(pstrString, "nodes"))
		{
			result = ParseNodes(m_pSubModel->nodes);
		}
		else if(!qstrcicmp(pstrString, "skeleton"))
		{
			result = ParseSkeleton(m_pSubModel->nodes, m_pSubModel->bones, m_pSubModel->boneimap, m_boneTransformInfoArray);
		}
		else if(!qstrcicmp(pstrString, "triangles"))
		{
			result = ParseTriangles();
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

	if(m_pSubModel->nodes.empty())
	{
		ErrorMsg("No nodes were loaded for '%s'.\n", m_scriptFileName.c_str());
		return false;
	}

	if(m_pSubModel->bones.empty())
	{
		ErrorMsg("No skeleton information was loaded for '%s'.\n", m_scriptFileName.c_str());
		return false;
	}

	// Set up bone mappings
	if(!SetBoneIndexMappings())
	{
		ErrorMsg("Error while setting up bone index mappings for '%s'.\n", m_scriptFileName.c_str());
		return false;
	}

	// Ensure we have a closed mesh
	if(!g_options.isFlagSet(CMP_FL_MCD_NO_NEIGHBOR_CHECK))
	{
		if(!CheckTriangleNeighbors())
		{
			ErrorMsg("Error while checking for holes on collision mesh '%s'.\n", m_scriptFileName.c_str());
			return false;
		}
	}

	return true;
}

//===============================================
// @brief Parses the triangles from the SMD file loaded
//
// @return TRUE if successful, FALSE if an error occurred
//===============================================
bool CCollisionSMDParser::ParseTriangles( void )
{
	if(m_boneTransformInfoArray.empty() || m_pSubModel->bones.empty())
	{
		ErrorMsg("Bone list was empty for %s.\n", __FUNCTION__);
		return false;
	}

	Uint32 triangleCount = 0;
	mcd::triangle_t triangle;

	while(true)
	{
		const Char* pstrString = nullptr;
		if(!ReadString(pstrString, true))
		{
			ErrorMsg("Unexpected EOF while reading 'triangles' block.\n");
			return false;
		}

		if(!qstrcicmp(pstrString, "end"))
			break;

		// The token we read in should be the texture name, so
		// clean it up
		CString textureName = pstrString;
		Uint32 length = textureName.length();

		Int32 i;
		for(i = length - 1; i >= 0; i--)
		{
			if(!SDL_isspace(textureName[i]))
				break;
		}

		if((i+1) < length)
			textureName.erase(i+1, (length - i));

		// Look out for overrides
		const Char* pstrRename = nullptr;
		if(m_studioCompiler.GetTextureRename(textureName.c_str(), pstrRename))
			textureName = pstrRename;

		// Get texture and mesh pointers
		triangle.skinref = m_mcdCompiler.GetTextureIndex(textureName.c_str());

		for(i = 0; i < 3; i++)
		{
			Uint32* ptrivertex = nullptr;
			if(m_reverseTriangles)
				ptrivertex = &triangle.vertexes[2 - i];
			else
				ptrivertex = &triangle.vertexes[i];

			Int32 boneIndex = 0;
			if(!ReadInt32(boneIndex, true))
			{
				ErrorMsg("Couldn't read bone index for triangle.\n");
				return false;
			}

			if(boneIndex < 0 || boneIndex >= m_pSubModel->bones.size())
			{
				ErrorMsg("Bogus bone index %d.\n", boneIndex);
				return false;
			}

			Vector vertexCoord;
			for(Uint32 j = 0; j < 3; j++)
			{
				if(!ReadFloat(vertexCoord[j]))
				{
					ErrorMsg("Couldn't read vertex coordinate %d for triangle.\n", j);
					return false;
				}
			}

			Float dummyValue = 0;
			for(Uint32 j = 0; j < 3; j++)
			{
				if(!ReadFloat(dummyValue))
				{
					ErrorMsg("Couldn't read normal value %d for triangle.\n", j);
					return false;
				}
			}

			for(Uint32 j = 0; j < 2; j++)
			{
				if(!ReadFloat(dummyValue))
				{
					ErrorMsg("Couldn't read texcoord value %d for triangle.\n", j);
					return false;
				}
			}

			// We don't use this
			Int32 numWeights = 0;
			if(ReadInt32(numWeights))
			{
				for(Uint32 j = 0; j < numWeights; j++)
				{
					Int32 dummyIntValue;
					if(!ReadInt32(dummyIntValue))
					{
						ErrorMsg("Couldn't read weight bone index for weight %d for triangle.\n", j);
						return false;
					}

					if(!ReadFloat(dummyValue))
					{
						ErrorMsg("Couldn't read weight value for weight %d for triangle.\n", j);
						return false;
					}
				}
			}

			// Build vertex
			mcd::vertex_t vertex;
			vertex.boneindex = boneIndex;

			// Apply any adjustments
			vertexCoord = m_studioCompiler.ApplyOffset(vertexCoord);
			vertexCoord = m_studioCompiler.ApplyScaling(vertexCoord);

			// Transform to bone space
			Math::VectorInverseTransform(vertexCoord, m_boneTransformInfoArray[boneIndex].matrix, vertex.origin);

			// Add this vertex
			(*ptrivertex) = m_pSubModel->addVertex(vertex);

			// Mark ref counter in bone
			smdl::bone_t& bone = m_pSubModel->bones[boneIndex];
			bone.refcounter++;
		}

		// Add triangles to the mesh
		m_pSubModel->addTriangle(triangle);
		triangleCount++;
	}

	// Resize all arrays to their actual size
	if(m_pSubModel->vertexes.size() != m_pSubModel->nbvertexes)
		m_pSubModel->vertexes.resize(m_pSubModel->nbvertexes);

	if(m_pSubModel->triangles.size() != m_pSubModel->nbtriangles)
		m_pSubModel->triangles.resize(m_pSubModel->nbtriangles);

	// Print out the relevant results
	Msg("Loaded '%s':\n", m_pSubModel->name.c_str());
	Msg("\t%d triangles.\n", triangleCount);
	Msg("\t%d unique vertexes.\n", m_pSubModel->nbvertexes);

	return true;
}

//===============================================
// @brief Sets bone index mappings
//
//===============================================
bool CCollisionSMDParser::SetBoneIndexMappings( void )
{
	// Establish bone index maps
	m_pSubModel->boneimap.resize(m_pSubModel->nodes.size());
	Uint32 nbTableBones = m_studioCompiler.GetNbBones();

	for(Uint32 i = 0; i < m_pSubModel->nodes.size(); i++)
	{
		smdl::bone_node_t& node = m_pSubModel->nodes[i];
		smdl::bone_t& bone = m_pSubModel->bones[i];

		Int32 j = 0;
		for(; j < nbTableBones; j++)
		{
			const smdl::boneinfo_t* pbone = m_studioCompiler.GetBone(j);
			if(!qstrcicmp(pbone->name,node.bonename))
				break;
		}

		if(j == nbTableBones)
		{
			if(bone.refcounter == 0)
			{
				m_pSubModel->boneimap[j] = NO_POSITION;
				bone.globalindex = NO_POSITION;
			}
			else
			{
				ErrorMsg("Bone '%s' in collision mesh '%s' was not present in VBM file.\n", node.bonename.c_str(), m_pSubModel->name.c_str());
				return false;
			}
		}
		else
		{
			m_pSubModel->boneimap[j] = i;
			bone.globalindex = j;
		}
	}

	return true;
}

//===============================================
// @brief Checks for triangles with no neighbors
//
// @return TRUE if all triangles have neighbors, 
// FALSE if one was found without an edge pair
//===============================================
bool CCollisionSMDParser::CheckTriangleNeighbors( void )
{
	for(Uint32 i = 0; i < m_pSubModel->nbtriangles; i++)
	{
		mcd::triangle_t& triangle1 = m_pSubModel->triangles[i];

		for(Uint32 j = 0; j < 3; j++)
		{
			Int32 tri1edgev1index = j;
			Int32 tri1edgev2index = (j + 1) % 3;

			Int32 tri1edgevertex1 = triangle1.vertexes[tri1edgev1index];
			Int32 tri1edgevertex2 = triangle1.vertexes[tri1edgev2index];

			Uint32 k = 0;
			for(; k < m_pSubModel->nbtriangles; k++)
			{
				mcd::triangle_t& triangle2 = m_pSubModel->triangles[k];
				
				Uint32 l = 0;
				for(; l < 3; l++)
				{
					Int32 tri2edgev1index = l;
					Int32 tri2edgev2index = (l + 1) % 3;

					Int32 tri2edgevertex1 = triangle2.vertexes[tri2edgev1index];
					Int32 tri2edgevertex2 = triangle2.vertexes[tri2edgev2index];

					if(tri1edgevertex1 == tri2edgevertex1 && tri1edgevertex2 == tri2edgevertex2)
						break;
				}

				if(l != 3)
					break;
			}

			if(k == m_pSubModel->nbtriangles)
			{
				ErrorMsg("Collision mesh '%s' is not fully closed, a triangle was found missing a neighbor on an edge.\n", m_pSubModel->name.c_str());
				return false;
			}
		}
	}

	return true;
}