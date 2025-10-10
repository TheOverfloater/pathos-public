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
#include "geometrysmdparser.h"
#include "options.h"
#include "main.h"
#include "vbm_shared.h"
#include "compiler_math.h"

//===============================================
// @brief Constructor for CGeometrySMDParser class
//
// @param compiler Reference to studio model compiler object
// @psubmodel Submodel to parse data for
//===============================================
CGeometrySMDParser::CGeometrySMDParser( CStudioModelCompiler& compiler, smdl::submodel_t* psubmodel ):
	CSMDParser(compiler),
	m_pSubModel(psubmodel)
{
}

//===============================================
// @brief Destructor for CGeometrySMDParser class
//
//===============================================
CGeometrySMDParser::~CGeometrySMDParser( void )
{
}

//===============================================
// @brief Loads and processes an SMD file
//
// @param pstrFilename File path
// @return TRUE if successful, FALSE otherwise
//===============================================
bool CGeometrySMDParser::ProcessFile( const Char* pstrFilename )
{
	CString filePath;
	filePath << pstrFilename;
	
	if(filePath.find(0, ".smd", true) == CString::CSTRING_NO_POSITION)
		filePath << ".smd";

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

	// Sort normals by skinref
	ResortNormals();

	return true;
}

//===============================================
// @brief Re-sorts normals to be organized by texture
//
//===============================================
void CGeometrySMDParser::ResortNormals( void )
{
	// We need to re-sort normals by mesh, as GoldSrc needs this
	// for it's chrome effect to work
	CArray<Int32> normalIndexMap(m_pSubModel->normals.size());
	CArray<smdl::normal_t> origNormalsArray(m_pSubModel->normals);

	Uint32 k = 0;
	for(Uint32 i = 0; i < m_pSubModel->pmeshes.size(); i++)
	{
		smdl::mesh_t* pmesh = m_pSubModel->pmeshes[i];
		for(Uint32 j = 0; j < origNormalsArray.size(); j++)
		{
			smdl::normal_t& normal = origNormalsArray[j];
			if(normal.skinref == pmesh->skinref)
			{
				m_pSubModel->normals[k] = normal;
				normalIndexMap[j] = k;
				k++;

				pmesh->numnorms++;
			}
		}
	}

	// Do the same for the triangle data
	for(Uint32 i = 0; i < m_pSubModel->pmeshes.size(); i++)
	{
		smdl::mesh_t* pmesh = m_pSubModel->pmeshes[i];
		for(Uint32 j = 0; j < pmesh->numtrivertexes; j++)
		{
			smdl::triangle_vertex_t& trivert = pmesh->trivertexes[j];
			trivert.normalindex = normalIndexMap[trivert.normalindex];
		}
	}
}

//===============================================
// @brief Parses the triangles from the SMD file loaded
//
// @return TRUE if successful, FALSE if an error occurred
//===============================================
bool CGeometrySMDParser::ParseTriangles( void )
{
	if(m_boneTransformInfoArray.empty() || m_pSubModel->bones.empty())
	{
		ErrorMsg("Bone list was empty for ParseTriangles.\n");
		return false;
	}
	
	Float normalBlend = m_studioCompiler.GetNormalMergeTreshold();
	Float minWeightTreshold = m_studioCompiler.GetWeightTreshold();

	smdl::triangle_vertex_t triVertexes[3];
	Vector triNormals[3];
	Vector triVertexCoords[3];

	Uint32 triangleCount = 0;
	Uint32 reversedTriangleCount = 0;
	Uint32 badNormalTriangleCount = 0;

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
		smdl::texture_t* pTexture = m_studioCompiler.GetTextureForName(textureName.c_str());
		smdl::mesh_t* pMesh = m_pSubModel->getMesh(pTexture->skinref);

		for(i = 0; i < 3; i++)
		{
			smdl::triangle_vertex_t* ptrivertex = nullptr;
			if(m_pSubModel->reverseTriangles)
				ptrivertex = &triVertexes[2 - i];
			else
				ptrivertex = &triVertexes[i];

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

			Vector normalValue;
			for(Uint32 j = 0; j < 3; j++)
			{
				if(!ReadFloat(normalValue[j]))
				{
					ErrorMsg("Couldn't read normal value %d for triangle.\n", j);
					return false;
				}
			}

			normalValue.Normalize();

			for(Uint32 j = 0; j < 2; j++)
			{
				if(!ReadFloat(ptrivertex->texcoords[j]))
				{
					ErrorMsg("Couldn't read texcoord value %d for triangle.\n", j);
					return false;
				}
			}

			// Get nb of weights
			Int32 bones[MAX_VBM_BONEWEIGHTS] = { 0 };
			Float weights[MAX_VBM_BONEWEIGHTS] = { 0 };

			Int32 numWeights = 0;
			if(ReadInt32(numWeights))
			{
				// Ensure it cannot be larger than the limit
				Uint32 readWeights;
				if(numWeights > MAX_VBM_BONEWEIGHTS)
					readWeights = MAX_VBM_BONEWEIGHTS;
				else
					readWeights = numWeights;

				for(Uint32 j = 0; j < numWeights; j++)
				{
					Int32 weightBoneIndex;
					if(!ReadInt32(weightBoneIndex))
					{
						ErrorMsg("Couldn't read weight bone index for weight %d for triangle.\n", j);
						return false;
					}

					if(weightBoneIndex < 0 || weightBoneIndex >= m_pSubModel->bones.size())
					{
						ErrorMsg("Bogus weight bone index %d for weight %d for triangle.\n", weightBoneIndex, j);
						return false;
					}

					Float weight;
					if(!ReadFloat(weight))
					{
						ErrorMsg("Couldn't read weight value for weight %d for triangle.\n", j);
						return false;
					}

					if(j < readWeights)
					{
						bones[j] = weightBoneIndex;
						weights[j] = weight;
					}
				}
			}
			else
			{
				// Only one bone
				bones[0] = boneIndex;
				weights[0] = 1.0;
				numWeights = 1;
			}

			if(g_options.isFlagSet(CMP_FL_TAG_BAD_NORMALS|CMP_FL_TAG_REVERSED_TRIANGLES))
			{
				triVertexCoords[i] = vertexCoord;
				triNormals[i] = normalValue;
			}

			// Build vertex
			smdl::vertex_t vertex;
			vertex.boneindex = boneIndex;
			vertex.pos_original = vertexCoord;

			// Apply any adjustments
			vertexCoord = m_studioCompiler.ApplyOffset(vertexCoord);
			vertexCoord = m_studioCompiler.ApplyScaling(vertexCoord);

			// Transform to bone space
			Math::VectorInverseTransform(vertexCoord, m_boneTransformInfoArray[boneIndex].matrix, vertex.position);

			for(Uint32 j = 0; j < 3; j++)
				vertex.position[j] = static_cast<Int32>(vertex.position[j] * VERTEX_ROUNDING_VALUE) / VERTEX_ROUNDING_VALUE;

			for(Uint32 j = 0; j < 3; j++)
				vertex.pos_original[j] = static_cast<Int32>(vertex.pos_original[j] * VERTEX_ROUNDING_VALUE) / VERTEX_ROUNDING_VALUE;

			// Build normal
			smdl::normal_t normal;
			normal.boneindex = boneIndex;
			normal.skinref = pTexture->skinref;
			normal.normal_original = normalValue;
			Math::VectorInverseRotate(normalValue, m_boneTransformInfoArray[boneIndex].matrix, normal.normal);

			ptrivertex->normalindex = m_pSubModel->addNormal(normal, normalBlend);
			ptrivertex->vertexindex = m_pSubModel->addVertex(vertex);
			ptrivertex->weightindex = m_pSubModel->addWeightInfo(weights, bones, numWeights, minWeightTreshold);
		}

		// Add triangles to the mesh
		pMesh->addTriangleVertexes(triVertexes);
		triangleCount++;

		// Check for erroneous normals if set
		if(g_options.isFlagSet(CMP_FL_TAG_BAD_NORMALS))
		{
			if(Math::DotProduct(triNormals[0], triNormals[1]) < 0
				|| Math::DotProduct(triNormals[1], triNormals[2]) < 0
				|| Math::DotProduct(triNormals[2], triNormals[0]) < 0)
			{
				smdl::texture_t* pTexture = m_studioCompiler.GetTextureForName("tag_bad_normals");
				smdl::mesh_t* pMesh = m_pSubModel->getMesh(pTexture->skinref);

				// Add to new mesh
				pMesh->addTriangleVertexes(triVertexes);
				badNormalTriangleCount++;
			}
		}

		// Check for reversed triangles
		if(g_options.isFlagSet(CMP_FL_TAG_REVERSED_TRIANGLES))
		{
			Vector a1, a2, sn;
			Vector dotvec;

			Math::VectorSubtract(triVertexCoords[1], triVertexCoords[0], a1);
			Math::VectorSubtract(triVertexCoords[2], triVertexCoords[0], a2);
			Math::CrossProduct(a1, a2, sn);
			sn.Normalize();

			for(Uint32 i = 0; i < 3; i++)
				dotvec[i] = Math::DotProduct(sn, triNormals[i]);

			if(dotvec[0] < 0 || dotvec[1] < 0 || dotvec[2] < 0)
			{
				smdl::texture_t* pTexture = m_studioCompiler.GetTextureForName("tag_reverse_triangles");
				smdl::mesh_t* pMesh = m_pSubModel->getMesh(pTexture->skinref);

				// Add to new mesh
				pMesh->addTriangleVertexes(triVertexes);
				reversedTriangleCount++;
			}
		}
	}

	// Resize all arrays to their actual size
	if(m_pSubModel->vertexes.size() != m_pSubModel->numvertexes)
		m_pSubModel->vertexes.resize(m_pSubModel->numvertexes);

	if(m_pSubModel->normals.size() != m_pSubModel->numnormals)
		m_pSubModel->normals.resize(m_pSubModel->numnormals);

	if(m_pSubModel->weightinfos.size() != m_pSubModel->numweightinfos)
		m_pSubModel->weightinfos.resize(m_pSubModel->numweightinfos);

	// Resize mesh triangles to actual size
	for(Uint32 i = 0; i < m_pSubModel->pmeshes.size(); i++)
	{
		smdl::mesh_t* pmesh = m_pSubModel->pmeshes[i];
		pmesh->trivertexes.resize(pmesh->numtrivertexes);
	}

	// Print out the relevant results
	Msg("Loaded '%s':\n", m_pSubModel->name.c_str());
	Msg("\t%d triangles.\n", triangleCount);
	Msg("\t%d unique vertexes.\n", m_pSubModel->numvertexes);
	Msg("\t%d unique normal.\n", m_pSubModel->numnormals);
	Msg("\t%d unique vertex weights.\n", m_pSubModel->numweightinfos);
	
	if(reversedTriangleCount > 0)
		Msg("\t%d reversed triangles found in '%s'.\n", reversedTriangleCount, m_pSubModel->name.c_str());
	if(badNormalTriangleCount > 0)
		Msg("\t%d triangles with bad normals found in '%s'.\n", badNormalTriangleCount, m_pSubModel->name.c_str());

	// Check against limits
	if(!g_options.isFlagSet(CMP_FL_NO_STUDIOMDL_VERT_LIMIT) && !g_options.isFlagSet(CMP_FL_STRIP_STUDIO_TRI_DATA))
	{
		// Check against vertexes
		if(m_pSubModel->numvertexes >= MAXSTUDIOVERTS_REF)
		{
			ErrorMsg("Vertexes exceed MAXSTUDIOVERTS(%d > %d).\n", m_pSubModel->numvertexes, MAXSTUDIOVERTS_REF);
			return false;
		}

		// Check against normals
		if(m_pSubModel->numnormals >= MAXSTUDIOVERTS_REF)
		{
			ErrorMsg("Normals exceed MAXSTUDIOVERTS(%d > %d).\n", m_pSubModel->numnormals, MAXSTUDIOVERTS_REF);
			return false;
		}
	}

	return true;
}

//===============================================
// @brief Return the bone transform info array
//
// @return Reference to the bone transform info array
//===============================================
const CArray<smdl::bone_transforminfo_t>& CGeometrySMDParser::GetBoneTransformInfoArray( void ) const
{
	return m_boneTransformInfoArray;
}
