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
#include "vtaparser.h"
#include "options.h"
#include "main.h"

//===============================================
// @brief Constructor for CVTAParser class
//
// @param compiler Reference to studiomodel compiler object
// @param psubmodel Submodel this VTA belongs to
// @param bonetransforminfoarray Bone transform info array from submodel
//===============================================
CVTAParser::CVTAParser( CStudioModelCompiler& compiler, smdl::submodel_t* psubmodel, const CArray<smdl::bone_transforminfo_t>& bonetransforminfoarray ):
	m_studioCompiler(compiler),
	m_boneTransformInfoArray(bonetransforminfoarray),
	m_pSubModel(psubmodel)
{
}

//===============================================
// @brief Destructor for CVTAParser class
//
//===============================================
CVTAParser::~CVTAParser( void )
{
}


//===============================================
// @brief Loads and processes an VTA file
//
// @param pstrFilename File path
// @return TRUE if successful, FALSE otherwise
//===============================================
bool CVTAParser::ProcessFile( const Char* pstrFilename )
{
	CString filePath;
	filePath << pstrFilename << ".vta";
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

	if(versionNumber != VTA_VERSION)
	{
		ErrorMsg("VTA file '%s' has invalid version %d, %d expected.\n", m_scriptFileName.c_str(), versionNumber, VTA_VERSION);
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
			result = ParseNodes();
		}
		else if(!qstrcmp(pstrString, "skeleton"))
		{
			result = ParseSkeleton();
		}
		else if(!qstrcmp(pstrString, "vertexanimation"))
		{
			result = ParseVertexAnimation();
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

	return true;
}

//===============================================
// @brief Parses the nodes list from the VTA file loaded
//
// @return TRUE if successful, FALSE if an error occurred
//===============================================
bool CVTAParser::ParseNodes( void )
{
	// Note: Cannonfodder's VTA exporter exports this node block, but it is totally
	// useless when it comes to processing VTA data, as the vertexes have no bones
	// tied to them in the VTA file. We just use this function to parse till the 'end'.

	const Char* pstrString = nullptr;
	while(true)
	{
		// Needs to be read as string, as it can either be a number
		// or the 'end' command that terminates parsing
		if(!ReadString(pstrString, true))
		{
			ErrorMsg("Incomplete 'nodes' block, missing 'end'.\n");
			return false;
		}

		// Break if reached end
		if(!qstrcicmp(pstrString, "end"))
			break;
	}

	return true;
}

//===============================================
// @brief Parses skeleton from the VTA file loaded
//
// @return TRUE if successful, FALSE if an error occurred
//===============================================
bool CVTAParser::ParseSkeleton( void )
{
	// Note: Cannonfodder's VTA exporter exports this skeleton block, but it is totally
	// useless when it comes to processing VTA data, as the vertexes have no bones
	// tied to them in the VTA file. We just use this function to parse till the 'end'.

	const Char* pstrString = nullptr;
	while(true)
	{
		// Needs to be read as string, as it can either be a number
		// or the 'end' command that terminates parsing
		if(!ReadString(pstrString, true))
		{
			ErrorMsg("Incomplete 'skeleton' block, missing 'end'.\n");
			return false;
		}

		// Break if reached end
		if(!qstrcicmp(pstrString, "end"))
			break;
	}

	return true;
}

//===============================================
// @brief Parses vertex animation info from the VTA file loaded
//
// @return TRUE if successful, FALSE if an error occurred
//===============================================
bool CVTAParser::ParseVertexAnimation( void )
{
	if(m_pSubModel->bones.empty())
	{
		ErrorMsg("Submodel bone list was empty for ParseVertexAnimation.\n");
		return false;
	}

	if(m_pSubModel->pflexmodel)
	{
		ErrorMsg("Submodel '%s' already has flexes defined.\n", m_pSubModel->name.c_str());
		return false;
	}

	// Allocate temporary array for processing
	CArray<smdl::flexframe_t*> ptempflexframes;
	// Pointer to current flex frame
	smdl::flexframe_t* pcurrentframe = nullptr;

	while(true)
	{
		const Char* pstrString = nullptr;
		if(!ReadString(pstrString, true))
		{
			ErrorMsg("Unexpected EOF while reading 'triangles' block.\n");
			return false;
		}

		if(!qstrcmp(pstrString, "end"))
		{
			// Reached end of block
			break;
		}
		else if(!qstrcmp(pstrString, "time"))
		{
			Int32 frameIndex;
			if(!ReadInt32(frameIndex))
			{
				ErrorMsg("Couldn't read time index for 'frame'.\n");
				return false;
			}

			if(frameIndex >= ptempflexframes.size())
				ptempflexframes.resize(frameIndex+1);

			if(!ptempflexframes[frameIndex])
				ptempflexframes[frameIndex] = new smdl::flexframe_t();

			pcurrentframe = ptempflexframes[frameIndex];
		}
		else
		{
			if(!pcurrentframe)
			{
				ErrorMsg("Missing 'time' command before vertex animation data.\n");
				return false;
			}

			if(!Common::IsNumber(pstrString))
			{
				ErrorMsg("Expected numerical value for vertex index, got '%s' instead.\n", pstrString);
				return false;
			}

			Int32 vertexIndex = SDL_atoi(pstrString);

			Vector vertexCoord;
			for(Uint32 j = 0; j < 3; j++)
			{
				if(!ReadFloat(vertexCoord[j]))
				{
					ErrorMsg("Couldn't read vertex coordinate %d for triangle.\n", j);
					return false;
				}
			}

			for(Uint32 j = 0; j < 3; j++)
				vertexCoord[j] = static_cast<Int32>(vertexCoord[j] * VERTEX_ROUNDING_VALUE) / VERTEX_ROUNDING_VALUE;

			Vector normalValue;
			for(Uint32 j = 0; j < 3; j++)
			{
				if(!ReadFloat(normalValue[j]))
				{
					ErrorMsg("Couldn't read normal value %d for triangle.\n", j);
					return false;
				}
			}

			// Check for duplicacy
			Uint32 i = 0;
			for(; i < pcurrentframe->numvertexes; i++)
			{
				smdl::flexvertex_t& vertex = pcurrentframe->vertexes[i];
				if(Math::VectorCompare(vertex.origin, vertexCoord)&& Math::VectorCompare(vertex.normal, normalValue))
					break;
			}

			if(i != pcurrentframe->numvertexes)
				continue;

			// Check allocation for resizing
			if(pcurrentframe->numvertexes >= pcurrentframe->vertexes.size())
				pcurrentframe->vertexes.resize(pcurrentframe->vertexes.size() + VERTEX_ALLOCATION_COUNT);

			// Add the vertex
			smdl::flexvertex_t& flexvert = pcurrentframe->vertexes[pcurrentframe->numvertexes];
			pcurrentframe->numvertexes++;

			flexvert.vertexindex = vertexIndex;
			flexvert.smd_normindex = NO_POSITION;
			flexvert.smd_vertindex = NO_POSITION;
			flexvert.animated = false;
			flexvert.origin = vertexCoord;
			flexvert.normal = normalValue;
		}
	}

	// Mark non-animated vertexes
	Uint32 animatedVertexCount = 0;
	smdl::flexframe_t* pbaseframe = ptempflexframes[0];
	for(Uint32 i = 0; i < pbaseframe->numvertexes; i++)
	{
		Uint32 j = 1;
		for(; j < ptempflexframes.size(); j++)
		{
			if(!ptempflexframes[j])
				continue;

			pcurrentframe = ptempflexframes[j];

			Uint32 k = 0;
			for(; k < pcurrentframe->numvertexes; k++)
			{
				if(pcurrentframe->vertexes[k].vertexindex == pbaseframe->vertexes[i].vertexindex)
					break;
			}

			if(k != pcurrentframe->numvertexes)
				break;
		}

		// Mark if it's animated
		if(j != ptempflexframes.size())
		{
			pbaseframe->vertexes[i].animated = true;
			animatedVertexCount++;
		}
	}

	if(!animatedVertexCount)
	{
		ErrorMsg("No animated vertexes were found in the VTA file.\n");
		return false;
	}

	// Check for the limit based on texture size
	Uint32 maxFlexVertexes = SDL_floor(VBM_FLEXTEXTURE_SIZE/3) * VBM_FLEXTEXTURE_SIZE;
	if(animatedVertexCount >= maxFlexVertexes)
	{
		ErrorMsg("VTA exceeds MAX_VBM_FLEXVERTS\n");
		return false;
	}

	// Finalize the vertex data
	FinalizeFlexData(ptempflexframes);

	// Print infos
	Uint32 finalVertexCount = m_pSubModel->pflexmodel->pflexes[0]->numvertexes;
	Uint32 nbDiscarded = ptempflexframes[0]->numvertexes - finalVertexCount;
	Msg("Loaded '%s':\n\t%d animated vertexes.\n\t%d discarded vertexes.\n\t%d frames.\n", m_pSubModel->vtaname.c_str(), finalVertexCount, nbDiscarded, m_pSubModel->pflexmodel->pflexes.size());
	
	// Release our temporary array
	if(!ptempflexframes.empty())
	{
		for(Uint32 i = 0; i < ptempflexframes.size(); i++)
			delete ptempflexframes[i];

		ptempflexframes.clear();
	}	
	return true;
}

//===============================================
// @brief Look up flex vertex matching an SMD vertex
// 
// @param psrctrivertex SMD vertex to find flex vertex for
// @param pbaseflexframe Base(frame index 0) flex frame
//===============================================
void CVTAParser::LinkFlexVertex( smdl::triangle_vertex_t* psrctrivertex, smdl::flexframe_t* pbaseflexframe )
{
	// Remember best match
	Int32 bestIndex = NO_POSITION;
	Float bestNormalDP = -1;
	Float bestDistance = -1;

	// Some VTAs sometimes coalesce vertexes, so try to find the best match and not an exact one
	for(Uint32 i = 0; i < pbaseflexframe->numvertexes; i++)
	{
		smdl::flexvertex_t& curVertex = pbaseflexframe->vertexes[i];
		if(!curVertex.animated)
			continue;

		const smdl::vertex_t& smdVertex = m_pSubModel->vertexes[psrctrivertex->vertexindex];
		Float diff = (smdVertex.pos_original - curVertex.origin).Length();
		if(diff > 0.01) // Only allow for a very small margin
			continue;

		const smdl::normal_t& smdNormal = m_pSubModel->normals[psrctrivertex->normalindex];
		Float dp = Math::DotProduct(smdNormal.normal_original, curVertex.normal);
		if(bestIndex == NO_POSITION || diff < bestDistance || dp < bestNormalDP)
		{
			bestIndex = i;
			bestNormalDP = dp;
			bestDistance = diff;
		}
	}

	// If we have a best match, then mark it
	if(bestIndex != NO_POSITION)
	{
		// Set index in tri
		psrctrivertex->flexindex = bestIndex;

		smdl::flexvertex_t& bestVertex = pbaseflexframe->vertexes[bestIndex];
		bestVertex.refcount++;

		if(bestVertex.smd_normindex == NO_POSITION || bestVertex.smd_vertindex == NO_POSITION)
		{
			bestVertex.smd_normindex = psrctrivertex->normalindex;
			bestVertex.smd_vertindex = psrctrivertex->vertexindex;
			bestVertex.boneindex = m_pSubModel->vertexes[psrctrivertex->vertexindex].boneindex;
		}
	}
}

//===============================================
// @brief Finalizes flex vertex data
// 
// @param ptempflexarray Temporary flex array
//===============================================
void CVTAParser::FinalizeFlexData( CArray<smdl::flexframe_t*>& ptempflexarray )
{
	// Allocate flex model object
	smdl::flexmodel_t* pflexmodel = new smdl::flexmodel_t();
	m_pSubModel->pflexmodel = pflexmodel;
	m_pSubModel->pflexmodel->name = m_pSubModel->vtaname;

	pflexmodel->pflexes.resize(ptempflexarray.size());
	for(Uint32 i = 0; i < ptempflexarray.size(); i++)
		pflexmodel->pflexes[i] = new smdl::flexframe_t();

	smdl::flexframe_t* psrcbaseframe = ptempflexarray[0];
	Int32* pVertexIndexMap = new Int32[psrcbaseframe->numvertexes];
	memset(pVertexIndexMap, 0, sizeof(Int32)*psrcbaseframe->numvertexes);

	// Match triverts with the most ideal flex vertex
	for(Uint32 i = 0; i < m_pSubModel->pmeshes.size(); i++)
	{
		smdl::triangle_vertex_t* psrctriangle = &m_pSubModel->pmeshes[i]->trivertexes[0];
		for(Uint32 j = 0; j < m_pSubModel->pmeshes[i]->numtrivertexes; j++, psrctriangle++)
			LinkFlexVertex(psrctriangle, psrcbaseframe);
	}

	for(Uint32 i = 0; i < psrcbaseframe->numvertexes; i++)
	{
		smdl::flexvertex_t& srcBaseVertex = psrcbaseframe->vertexes[i];
		if(!srcBaseVertex.animated)
			continue;

		// Check for non-linked ones
		if(srcBaseVertex.smd_normindex == NO_POSITION || srcBaseVertex.smd_vertindex == NO_POSITION)
		{
			WarningMsg("VTA vertex with ID %d not found in smd '%s'.\n", srcBaseVertex.vertexindex, m_pSubModel->name.c_str());
			continue;
		}

		// Start with the base frame
		smdl::flexframe_t* pdestframe = pflexmodel->pflexes[0];
		if(pdestframe->numvertexes == pdestframe->vertexes.size())
			pdestframe->vertexes.resize(pdestframe->vertexes.size() + VERTEX_ALLOCATION_COUNT);

		// Set final base vertex
		smdl::flexvertex_t& dstBaseVertex = pdestframe->vertexes[pdestframe->numvertexes];
		dstBaseVertex.animated = srcBaseVertex.animated;
		dstBaseVertex.refcount = srcBaseVertex.refcount;
		dstBaseVertex.smd_vertindex = srcBaseVertex.smd_vertindex;
		dstBaseVertex.smd_normindex = srcBaseVertex.smd_normindex;
		dstBaseVertex.vertexindex = pdestframe->numvertexes;
		dstBaseVertex.boneindex = srcBaseVertex.boneindex;
		pdestframe->numvertexes++;
		
		// Mark final position
		pVertexIndexMap[i] = dstBaseVertex.vertexindex;

		// Transform the vertexes into their final positions
		smdl::vertex_t& smdVertex = m_pSubModel->vertexes[dstBaseVertex.smd_vertindex];
		smdl::normal_t& smdNormal = m_pSubModel->normals[dstBaseVertex.smd_normindex];

		Vector vertexPosition = m_studioCompiler.ApplyOffset(srcBaseVertex.origin);
		vertexPosition = m_studioCompiler.ApplyScaling(vertexPosition);

		// Move vertex position to object space
		smdl::bone_transforminfo_t& boneTransInfo = m_boneTransformInfoArray[smdVertex.boneindex];
		Vector tmp = vertexPosition;
		Math::VectorInverseTransform(tmp, boneTransInfo.matrix, vertexPosition);

		// Turn it into an offset
		Math::VectorSubtract(vertexPosition, smdVertex.position, dstBaseVertex.origin);

		// Move normal into object space
		boneTransInfo = m_boneTransformInfoArray[smdNormal.boneindex];
		tmp = srcBaseVertex.normal;
		Math::VectorInverseRotate(tmp, boneTransInfo.matrix, dstBaseVertex.normal);
		dstBaseVertex.normal.Normalize();

		// Turn it into an offset
		Math::VectorSubtract(dstBaseVertex.normal, smdNormal.normal, dstBaseVertex.normal);

		// Now populate other frames also
		for(Uint32 j = 1; j < ptempflexarray.size(); j++)
		{
			smdl::flexframe_t* psrcframe = ptempflexarray[j];

			for(Uint32 k = 0; k < psrcframe->numvertexes; k++)
			{
				smdl::flexvertex_t& srcFrameVertex = psrcframe->vertexes[k];
				if(srcFrameVertex.vertexindex == srcBaseVertex.vertexindex)
				{
					pdestframe = pflexmodel->pflexes[j];

					// Ensure we have space
					if(pdestframe->numvertexes == pdestframe->vertexes.size())
						pdestframe->vertexes.resize(pdestframe->vertexes.size() + VERTEX_ALLOCATION_COUNT);

					smdl::flexvertex_t& dstFrameVertex = pdestframe->vertexes[pdestframe->numvertexes];
					pdestframe->numvertexes++;

					dstFrameVertex.vertexindex = dstBaseVertex.vertexindex;
					dstFrameVertex.animated = dstBaseVertex.animated;
					dstFrameVertex.smd_normindex = dstBaseVertex.smd_normindex;
					dstFrameVertex.smd_vertindex = dstBaseVertex.smd_vertindex;
					dstFrameVertex.boneindex = dstBaseVertex.boneindex;
					dstFrameVertex.refcount = dstBaseVertex.refcount;

					Vector vertexPosition = m_studioCompiler.ApplyOffset(srcFrameVertex.origin);
					vertexPosition = m_studioCompiler.ApplyScaling(vertexPosition);

					// Move vertex position to object space
					smdl::bone_transforminfo_t& boneTransInfo = m_boneTransformInfoArray[smdVertex.boneindex];
					Vector tmp2 = vertexPosition;
					Math::VectorInverseTransform(tmp2, boneTransInfo.matrix, vertexPosition);

					// Turn it into an offset
					Math::VectorSubtract(vertexPosition, smdVertex.position, dstFrameVertex.origin);

					// Move normal into object space
					boneTransInfo = m_boneTransformInfoArray[smdNormal.boneindex];
					Vector tmp = srcFrameVertex.normal;
					Math::VectorInverseRotate(tmp, boneTransInfo.matrix, dstFrameVertex.normal);
					dstFrameVertex.normal.Normalize();

					// Turn it into an offset
					Math::VectorSubtract(dstFrameVertex.normal, smdNormal.normal, dstFrameVertex.normal);
					break;
				}
			}
		}
	}

	// Remap triangle flex indexes to the final index positions
	for(Uint32 i = 0; i < m_pSubModel->pmeshes.size(); i++)
	{
		smdl::mesh_t* pmesh = m_pSubModel->pmeshes[i];
		for(Uint32 j = 0; j < pmesh->numtrivertexes; j++)
		{
			smdl::triangle_vertex_t& trivertex = pmesh->trivertexes[j];
			if(trivertex.flexindex != NO_POSITION)
				trivertex.flexindex = pVertexIndexMap[trivertex.flexindex];
		}
	}

	// Resize all arrays to final sizes
	for(Uint32 i = 0; i < pflexmodel->pflexes.size(); i++)
	{
		smdl::flexframe_t* pframe = pflexmodel->pflexes[i];
		pframe->vertexes.resize(pframe->numvertexes);
	}

	// Release data used
	delete[] pVertexIndexMap;
}