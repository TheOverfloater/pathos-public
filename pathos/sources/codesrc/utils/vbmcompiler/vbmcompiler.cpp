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
#include "vbmcompiler.h"
#include "options.h"
#include "main.h"
#include "compiler_math.h"
#include "refframesmdparser.h"
#include "filefuncs.h"

// Equalness epsilon value
static const Float EQUAL_EPSILON = 0.001;

// File buffer allocation size
const Uint32 CVBMCompiler::VBM_FILEBUFFER_ALLOC_SIZE = 1024*1024;
// Triangle alloc size
const Uint32 CVBMCompiler::TRIANGLE_ALLOC_SIZE = 512;
// Index buffer alloc size
const Uint32 CVBMCompiler::INDEX_BUFFER_ALLOC_SIZE = 2048;
// Vertex buffer alloc size
const Uint32 CVBMCompiler::VERTEX_BUFFER_ALLOC_SIZE = 1024;
// Reference frame file name
const Char CVBMCompiler::REFERENCE_FRAME_FILENAME[] = "reference_frame";

//===============================================
// @brief Constructor for CVBMCompiler class
//
// @param studioCompiler reference to studiomodel compiler object
//===============================================
CVBMCompiler::CVBMCompiler( CStudioModelCompiler& studioCompiler ):
	m_studioCompiler(studioCompiler),
	m_skinRefsArray(m_studioCompiler.GetSkinRefsArray()),
	m_pVBMHeader(nullptr),
	m_nbConversionTriangles(0),
	m_pCurrentConversionGroup(nullptr),
	m_pSourceSubmodel(nullptr),
	m_pDestinationSubmodel(nullptr),
	m_pDestFlexInfo(nullptr),
	m_currentFlexIndex(NO_POSITION),
	m_numIndexes(0),
	m_numVertexes(0),
	m_pFileBuffer(nullptr)
{
	m_indexesArray.resize(INDEX_BUFFER_ALLOC_SIZE);
	m_vertexesArray.resize(VERTEX_BUFFER_ALLOC_SIZE);
}

//===============================================
// @brief Destructor for CVBMCompiler class
//
//===============================================
CVBMCompiler::~CVBMCompiler( void )
{
	Clear();
}

//===============================================
// @brief Clears any data used by the class
//
//===============================================
void CVBMCompiler::Clear( void )
{
	if(!m_conversionTrianglesArray.empty())
		m_conversionTrianglesArray.clear();

	if(!m_indexesArray.empty())
		m_indexesArray.clear();

	if(!m_vertexesArray.empty())
		m_vertexesArray.clear();

	if(!m_flexControllerArray.empty())
		m_flexControllerArray.clear();

	if(!m_pSrcSubmodelsArray.empty())
		m_pSrcSubmodelsArray.clear();

	if(!m_pConversionGroupsArray.empty())
	{
		for(Uint32 i = 0; i < m_pConversionGroupsArray.size(); i++)
			delete m_pConversionGroupsArray[i];

		m_pConversionGroupsArray.clear();
	}

	if(m_pFileBuffer)
	{
		delete m_pFileBuffer;
		m_pFileBuffer = nullptr;
	}

	m_pVBMHeader = nullptr;
	m_pCurrentConversionGroup = nullptr;
	m_pSourceSubmodel = nullptr;
	m_pDestinationSubmodel = nullptr;
	m_pDestFlexInfo = nullptr;

	m_nbConversionTriangles = 0;
	m_currentFlexIndex = NO_POSITION;
	m_numIndexes = 0;
	m_numVertexes = 0;

	m_referenceFrame.clear();
}

//===============================================
// @brief Adds a new flex controller
//
// @param pstrControllerName Name of controller
// @param minValue Minimum value of controller on flexes
// @param maxValue Maximum value of controller on flexes
// @param interpType Interpolation method identifier
// @param vtaArray Array of VTAs used
// @return TRUE if successful, FALSE if there was an error
//===============================================
bool CVBMCompiler::AddFlexController( const Char* pstrControllerName, Float minValue, Float maxValue, vbmflexinterp_t interpType, const CArray<vbm::flexcontroller_vta_t> vtaArray )
{
	for(Uint32 i = 0; i < m_flexControllerArray.size(); i++)
	{
		vbm::flexcontroller_t controller = m_flexControllerArray[i];
		if(!qstrcmp(controller.name, pstrControllerName))
		{
			ErrorMsg("Flex controller with name '%s' already defined.\n", pstrControllerName);
			return false;
		}
	}

	vbm::flexcontroller_t newController;
	newController.name = pstrControllerName;
	newController.minvalue = minValue;
	newController.maxvalue = maxValue;
	newController.type = interpType;
	newController.vtas = vtaArray;
	m_flexControllerArray.push_back(newController);

	return true;
}

//===============================================
// @brief Processes a single vertex
//
// @param startVertexIndex First vertex index to look for matches from
// @param pbonegroup Conversion group that has our bone indexes
// @param pvertex Vertex to process
// @return TRUE on success, FAIL if an error occurred
//===============================================
bool CVBMCompiler::ProcessVertex( Int32 startVertexIndex, conversion_group_t* pbonegroup, const conversion_vertex_t* pvertex )
{
	// Check buffer limit
	if(m_numIndexes == m_indexesArray.size())
		m_indexesArray.resize(m_indexesArray.size() + INDEX_BUFFER_ALLOC_SIZE);

	smdl::vertex_weightinfo_t& weight = m_pSourceSubmodel->weightinfos[pvertex->weightindex];
	smdl::normal_t& smdnormal = m_pSourceSubmodel->normals[pvertex->normindex];
	smdl::vertex_t& smdvertex = m_pSourceSubmodel->vertexes[pvertex->vertindex];

	for(Uint32 i = startVertexIndex; i < m_numVertexes; i++)
	{
		const final_vertex_t& checkvertex = m_vertexesArray[i];
		if(checkvertex.numweights != weight.numweights)
			continue;

		Uint32 j = 0;
		for(; j < weight.numweights; j++)
		{
			Int32 boneindex = pbonegroup->boneindexes[(Int32)(checkvertex.boneindexes[j]/3)];
			if(boneindex != weight.boneindexes[j] || checkvertex.boneweights[j] != weight.weights[j])
				break;
		}

		if(j != weight.numweights)
			continue;

		if(Math::VectorCompare(smdnormal.normal, checkvertex.normal) 
			&& Math::VectorCompare(smdvertex.position, checkvertex.origin) 
			&& SDL_fabs(pvertex->texcoords[0]-checkvertex.texcoords[0]) <= EQUAL_EPSILON
			&& SDL_fabs(pvertex->texcoords[1]-checkvertex.texcoords[1]) <= EQUAL_EPSILON)
		{
			m_indexesArray[m_numIndexes] = i;
			m_numIndexes++;
			return true;
		}
	}

	// Check buffer limit
	if(m_numVertexes == m_vertexesArray.size())
		m_vertexesArray.resize(m_vertexesArray.size() + VERTEX_BUFFER_ALLOC_SIZE);

	// Add new index
	m_indexesArray[m_numIndexes] = m_numVertexes;
	m_numIndexes++;

	// Add a new vertex
	Int32 vertexIndex = m_numVertexes;
	final_vertex_t& newVertex = m_vertexesArray[vertexIndex];
	m_numVertexes++;

	newVertex.origin = smdvertex.position;
	newVertex.normal = smdnormal.normal;

	for(Uint32 i = 0; i < 2; i++)
		newVertex.texcoords[i] = pvertex->texcoords[i];

	newVertex.flexvertexindex = pvertex->flexindex;
	newVertex.numweights = weight.numweights;
	newVertex.refboneindex = pvertex->refboneindex;

	// Set flex info if present
	if(m_pSourceSubmodel->pflexmodel && !m_pSourceSubmodel->pflexmodel->pflexes.empty())
	{
		if(pvertex->flexindex && m_pDestFlexInfo->first_vertex == NO_POSITION)
			m_pDestFlexInfo->first_vertex = vertexIndex;

		if(pvertex->flexindex)
			m_pDestFlexInfo->num_vertexes = m_numVertexes - m_pDestFlexInfo->first_vertex;
	}
	
	for(Uint32 i = 0; i < weight.numweights; i++)
	{
		Uint32 j = 0;
		for(; j < pbonegroup->numbones; j++)
		{
			if(pbonegroup->boneindexes[j] == weight.boneindexes[i])
			{
				newVertex.boneweights[i] = weight.weights[i];
				newVertex.boneindexes[i] = j*3;
				break;
			}
		}

		if(j == pbonegroup->numbones)
		{
			ErrorMsg("Couldn't find boneindex %d in bone group.\n", (int)weight.boneindexes[i]);
			return false;
		}
	}

	return true;
}

//===============================================
// @brief Processes a submodel's data into VBM form
//
// @return TRUE on success, FAIL if an error occurred
//===============================================
bool CVBMCompiler::ProcessCurrentSubmodel( void )
{
	// don't bother if the submodel is empty
	if(!m_pSourceSubmodel->numvertexes || m_pSourceSubmodel->pmeshes.empty())
	{
		m_pDestinationSubmodel->flexinfoindex = NO_POSITION;
		return true;
	}

	// Set flex info index if needed
	if(m_pSourceSubmodel->pflexmodel && !m_pSourceSubmodel->pflexmodel->pflexes.empty())
	{
		m_pDestinationSubmodel->flexinfoindex = m_currentFlexIndex;
		m_currentFlexIndex++;
	}
	else
		m_pDestinationSubmodel->flexinfoindex = NO_POSITION;

	// Allocate the triangles
	Uint32 numTriangles = 0;
	for(Uint32 i = 0; i < m_pSourceSubmodel->pmeshes.size(); i++)
	{
		smdl::mesh_t* pmesh = m_pSourceSubmodel->pmeshes[i];
		numTriangles += (pmesh->numtrivertexes / 3);
	}

	// Allocate required triangles
	if(m_conversionTrianglesArray.size() < numTriangles)
		m_conversionTrianglesArray.resize(numTriangles);

	m_nbConversionTriangles = 0;

	// Get textures pointer
	vbmtexture_t* ptextures = reinterpret_cast<vbmtexture_t*>(reinterpret_cast<byte *>(m_pVBMHeader) + m_pVBMHeader->textureoffset);

	// Normal ones first
	for (Uint32 i = 0; i < m_pSourceSubmodel->pmeshes.size(); i++) 
	{
		smdl::mesh_t* psrcmesh = m_pSourceSubmodel->pmeshes[i];
		Int32 skinref = m_skinRefsArray[0][psrcmesh->skinref];

		vbmtexture_t* ptexture = &ptextures[skinref];
		if(ptexture->flags & STUDIO_NF_CHROME)
			continue;

		ProcessMesh(psrcmesh, ptexture);
	}

	// Do chrome last, to optimize on state switches
	for (Uint32 i = 0; i < m_pSourceSubmodel->pmeshes.size(); i++) 
	{
		smdl::mesh_t* psrcmesh = m_pSourceSubmodel->pmeshes[i];
		Int32 skinref = m_skinRefsArray[0][psrcmesh->skinref];

		vbmtexture_t* ptexture = &ptextures[skinref];
		if(!(ptexture->flags & STUDIO_NF_CHROME))
			continue;

		ProcessMesh(psrcmesh, ptexture);
	}

	// Merge anything you can
	OptimizeMeshes();

	// Process the submodels
	if(!FinalizeCurrentSubmodel())
		return false;

	// Release data used
	for(Uint32 i = 0; i < m_pConversionGroupsArray.size(); i++)
		delete m_pConversionGroupsArray[i];

	m_pConversionGroupsArray.clear();
	return true;
}

//===============================================
// @brief Finalizes a submodel's data
//
// @return TRUE on success, FAIL if an error occurred
//===============================================
bool CVBMCompiler::FinalizeCurrentSubmodel( void )
{
	Uint32 maxSize = sizeof(vbmsubmodel_t::name);
	qstrcpy_s(m_pDestinationSubmodel->name, m_pSourceSubmodel->name.c_str(), maxSize);

	Uint32 dataSize = m_pConversionGroupsArray.size() * sizeof(vbmmesh_t);
	m_pDestinationSubmodel->meshoffset = m_pFileBuffer->getsize();
	m_pDestinationSubmodel->nummeshes = m_pConversionGroupsArray.size();
	m_pFileBuffer->append(nullptr, dataSize);

	// Set flex related stuff if we have any
	if(m_pSourceSubmodel->pflexmodel && !m_pSourceSubmodel->pflexmodel->pflexes.empty())
	{
		m_pDestFlexInfo = reinterpret_cast<vbmflexinfo_t *>(reinterpret_cast<byte *>(m_pVBMHeader) + m_pVBMHeader->flexinfooffset) + m_pDestinationSubmodel->flexinfoindex;
		m_pDestFlexInfo->first_vertex = NO_POSITION;
		m_pFileBuffer->addpointer(reinterpret_cast<void**>(&m_pDestFlexInfo));
	}

	// do shit here
	vbmmesh_t* pdestmeshes = reinterpret_cast<vbmmesh_t *>(reinterpret_cast<byte *>(m_pVBMHeader) + m_pDestinationSubmodel->meshoffset);
	m_pFileBuffer->addpointer(reinterpret_cast<void**>(&pdestmeshes));

	for(Uint32 i = 0, j = m_numVertexes; i < m_pDestinationSubmodel->nummeshes; i++)
	{
		m_pCurrentConversionGroup = m_pConversionGroupsArray[i];

		vbmmesh_t* pdestmesh = &pdestmeshes[i];
		pdestmesh->start_index = m_numIndexes;
		pdestmesh->skinref = m_pCurrentConversionGroup->skinref;

		if(m_pCurrentConversionGroup->numbones)
		{
			pdestmesh->boneoffset = m_pFileBuffer->getsize();
			pdestmesh->numbones = m_pCurrentConversionGroup->numbones;

			dataSize = m_pCurrentConversionGroup->numbones * sizeof(byte);
			m_pFileBuffer->append(nullptr, dataSize);

			byte* pdestbones = (reinterpret_cast<byte *>(m_pVBMHeader) + pdestmesh->boneoffset);
			for(Uint32 k = 0; k < pdestmesh->numbones; k++)
				pdestbones[k] = m_pCurrentConversionGroup->boneindexes[k];

			j = m_numVertexes;
		}

		conversion_group_t* pbonegrp;
		if(!m_pCurrentConversionGroup->numbones)
			pbonegrp = m_pConversionGroupsArray[m_pCurrentConversionGroup->bonegroup];
		else
			pbonegrp = m_pCurrentConversionGroup;

		for(Uint32 k = 0; k < m_pCurrentConversionGroup->numtriangles; k++)
		{
			const conversion_triangle_t& triangle = m_pCurrentConversionGroup->triangles[k];

			for(Uint32 l = 0; l < 3; l++)
			{
				const conversion_vertex_t* pvertex = &triangle.vertexes[l];
				if(!ProcessVertex(j, pbonegrp, pvertex))
				{
					if(m_pDestFlexInfo)
						m_pFileBuffer->removepointer(reinterpret_cast<void**>(&m_pDestFlexInfo));

					m_pFileBuffer->removepointer(reinterpret_cast<void**>(&pdestmeshes));
					return false;
				}
			}
		}

		// set end
		pdestmesh->num_indexes = m_numIndexes - pdestmesh->start_index;
	}

	if(m_pDestFlexInfo)
		m_pFileBuffer->removepointer(reinterpret_cast<void**>(&m_pDestFlexInfo));

	m_pFileBuffer->removepointer(reinterpret_cast<void**>(&pdestmeshes));
	return true;
}

//===============================================
// @brief Processes and/or splits a mesh to keep it under the bones limit
//
// @param psrcmesh Source mesh to split/process
// @param ptexture Texture that belongs to this mesh
//===============================================
void CVBMCompiler::ProcessMesh( smdl::mesh_t* psrcmesh, vbmtexture_t* ptexture )
{
	// Could happen, I guess?
	if(!psrcmesh->numtrivertexes)
		return;

	for(Uint32 i = 0, j = 0; i < psrcmesh->numtrivertexes; i += 3, j++)
	{
		conversion_triangle_t& convtri = m_conversionTrianglesArray[m_nbConversionTriangles];
		m_nbConversionTriangles++;

		for(Uint32 k = 0; k < 3; k++)
		{
			conversion_vertex_t& destvertex = convtri.vertexes[k];
			smdl::triangle_vertex_t& srcvertex = psrcmesh->trivertexes[i + k];
			smdl::vertex_t& smdvertex = m_pSourceSubmodel->vertexes[srcvertex.vertexindex];

			destvertex.flexindex = srcvertex.flexindex;
			destvertex.normindex = srcvertex.normalindex;
			destvertex.refboneindex = smdvertex.boneindex;
			destvertex.vertindex = srcvertex.vertexindex;
			destvertex.weightindex = srcvertex.weightindex;

			destvertex.texcoords[0] = srcvertex.texcoords[0];
			destvertex.texcoords[1] = 1.0 - srcvertex.texcoords[1];
		}

		convtri.processed = false;
	}

	Int32 j = m_pConversionGroupsArray.size();
	m_pCurrentConversionGroup = AllocConversionGroup();
	m_pCurrentConversionGroup->name = ptexture->name;

	// process and recurse through all bones
	Uint32 numbones = m_studioCompiler.GetNbBones();
	for(Int32 i = 0; i < numbones; i++)
	{
		const smdl::boneinfo_t* pbone = m_studioCompiler.GetBone(i);
		if(pbone->parent_index != -1)
			continue;

		RecursiveAddBoneTriangles(i, pbone);
	}

	// fill in tex pointers
	for(Int32 i = j; i < m_pConversionGroupsArray.size(); i++)
	{
		conversion_group_t* pgrp = m_pConversionGroupsArray[i];
		pgrp->skinref = psrcmesh->skinref;
		pgrp->ptexture = ptexture;
	}
}

//===============================================
// @brief Allocates a new conversion group
//
// @return New empty conversion group
//===============================================
CVBMCompiler::conversion_group_t* CVBMCompiler::AllocConversionGroup( void )
{
	conversion_group_t* pnew = new conversion_group_t;
	m_pConversionGroupsArray.push_back(pnew);
	return pnew;
}

//===============================================
// @brief Get bones required by a triangle
//
// @param addbonesarray Array that holds the number of added bones
// @param triangle Triangle we're getting bones for
//===============================================
void CVBMCompiler::GetAddBones( CArray<Int32>& addbonesarray, const conversion_triangle_t& triangle )
{
	// Ensure it's empty
	if(!addbonesarray.empty())
		addbonesarray.clear();

	// Collect the used bones
	for(Uint32 i = 0; i < 3; i++)
	{
		Int32 weightIndex = triangle.vertexes[i].weightindex;
		smdl::vertex_weightinfo_t& weightInfo = m_pSourceSubmodel->weightinfos[weightIndex];

		for(Uint32 j = 0; j < weightInfo.numweights; j++)
		{
			if(!weightInfo.weights[j])
				continue;

			// Seek it in the group's bones before adding
			Uint32 k = 0;
			if(m_pCurrentConversionGroup->numbones)
			{
				for(; k < m_pCurrentConversionGroup->numbones; k++)
				{
					if(weightInfo.boneindexes[j] == m_pCurrentConversionGroup->boneindexes[k])
						break;
				}
			}

			if(k == m_pCurrentConversionGroup->numbones)
			{
				// make sure it wasn't already added to the list
				for(k = 0; k < addbonesarray.size(); k++)
				{
					if(weightInfo.boneindexes[j] == addbonesarray[k])
						break;
				}

				if(k == addbonesarray.size())
					addbonesarray.push_back(weightInfo.boneindexes[j]);
			}
		}
	}
}

//===============================================
// @brief Recurse through a bone and add it's associated triangles, then go through the children
//
// @param boneindex Index of bone to add triangles for
// @param psrcbone Pointer to the bone
//===============================================
void CVBMCompiler::RecursiveAddBoneTriangles( Int32 boneindex, const smdl::boneinfo_t* psrcbone )
{
	CArray<Int32> addbones;
	for(Uint32 i = 0; i < m_nbConversionTriangles; i++)
	{
		conversion_triangle_t& triangle = m_conversionTrianglesArray[i];
		if(triangle.processed)
			continue;

		// See if we're important for this triangle
		Uint32 j = 0;
		for(; j < 3; j++)
		{
			Int32 weightIndex = triangle.vertexes[j].weightindex;
			smdl::vertex_weightinfo_t& weightInfo = m_pSourceSubmodel->weightinfos[weightIndex];

			Uint32 k = 0;
			for(; k < weightInfo.numweights; k++)
			{
				if(weightInfo.boneindexes[k] == boneindex)
					break;
			}

			if(k != weightInfo.numweights)
				break;
		}

		if(j == 3)
			continue;

		// See if we need to add any bones
		GetAddBones(addbones, triangle);

		if(!addbones.empty())
		{
			Uint32 fullNbBones = m_pCurrentConversionGroup->numbones + addbones.size();
			if(fullNbBones > MAX_SHADER_BONES)
			{
				// We're full, so allocate a new group
				conversion_group_t* pnew = AllocConversionGroup();
				pnew->name = m_pCurrentConversionGroup->name;
				m_pCurrentConversionGroup = pnew;
				
				Msg("Submodel '%s' mesh '%s' was split after bone limit was reached.\n", m_pSourceSubmodel->name.c_str(), pnew->name.c_str());
				
				// If we got split, recalculate the needed bones
				GetAddBones(addbones, triangle);
			}

			for(j = 0; j < addbones.size(); j++)
			{
				m_pCurrentConversionGroup->boneindexes[m_pCurrentConversionGroup->numbones] = addbones[j];
				m_pCurrentConversionGroup->numbones++;
			}
		}

		// Resize the buffer if needed
		if(m_pCurrentConversionGroup->numtriangles == m_pCurrentConversionGroup->triangles.size())
			m_pCurrentConversionGroup->triangles.resize(m_pCurrentConversionGroup->triangles.size() + TRIANGLE_ALLOC_SIZE);

		// Add this as a new triangle
		m_pCurrentConversionGroup->triangles[m_pCurrentConversionGroup->numtriangles] = triangle;
		m_pCurrentConversionGroup->numtriangles++;

		// mark original triangle as having been processed
		triangle.processed = true;
	}

	// Recurse through our children next
	Uint32 numbones = m_studioCompiler.GetNbBones();
	for(Int32 i = 0; i < numbones; i++)
	{
		const smdl::boneinfo_t* pbone = m_studioCompiler.GetBone(i);
		if(pbone->parent_index != boneindex || i == boneindex)
			continue;

		RecursiveAddBoneTriangles(i, pbone);
	}
}

//===============================================
// @brief Calculate tangents for the vertexes
//
// @param pfinalvertexes Pointer to output array of vertexes
//===============================================
void CVBMCompiler::CalculateTangents( vbmvertex_t* pfinalvertexes )
{
	CArray<Vector> s_tangents(m_numVertexes);
	CArray<Vector> t_tangents(m_numVertexes);

	for(Uint32 i = 0; i < m_pVBMHeader->numbodyparts; i++)
	{
		vbmbodypart_t* pbodypart = (vbmbodypart_t *)((byte *)m_pVBMHeader + m_pVBMHeader->bodypartoffset)+i;
		for(Uint32 j = 0; j < pbodypart->numsubmodels; j++)
		{
			vbmsubmodel_t* pvbosubmodel = (vbmsubmodel_t *)((byte *)m_pVBMHeader + pbodypart->submodeloffset)+j;
			for(Uint32 k = 0; k < pvbosubmodel->nummeshes; k++)
			{
				vbmmesh_t *pmesh = (vbmmesh_t *)((byte *)m_pVBMHeader + pvbosubmodel->meshoffset)+k;
				for(Uint32 l = 0; l < pmesh->num_indexes; l += 3)
				{
					Vector sdir, tdir;
					float x1, x2, y1, y2, z1, z2;
					float s1, s2, t1, t2;
					float div, r;

					vbmvertex_t* pv[3];
					for(Uint32 m = 0; m < 3; m++)
					{
						Int32 vertindex = m_indexesArray[pmesh->start_index+l+m];
						pv[m] = &pfinalvertexes[vertindex];
					}

					x1 = pv[1]->origin[0] - pv[0]->origin[0];
					x2 = pv[2]->origin[0] - pv[0]->origin[0];
					y1 = pv[1]->origin[1] - pv[0]->origin[1];
					y2 = pv[2]->origin[1] - pv[0]->origin[1];
					z1 = pv[1]->origin[2] - pv[0]->origin[2];
					z2 = pv[2]->origin[2] - pv[0]->origin[2];

					s1 = pv[1]->texcoord[0] - pv[0]->texcoord[0];
					s2 = pv[2]->texcoord[0] - pv[0]->texcoord[0];
					t1 = pv[1]->texcoord[1] - pv[0]->texcoord[1];
					t2 = pv[2]->texcoord[1] - pv[0]->texcoord[1];

					div = (s1 * t2 - s2 * t1);
					r = div == 0.0f ? 0.0F : 1.0F/div;

					sdir[0] = (t2 * x1 - t1 * x2) * r;
					sdir[1] = (t2 * y1 - t1 * y2) * r;
					sdir[2] = (t2 * z1 - t1 * z2) * r;
					tdir[0] = (s1 * x2 - s2 * x1) * r;
					tdir[1] = (s1 * y2 - s2 * y1) * r;
					tdir[2] = (s1 * z2 - s2 * z1) * r;

					for(Uint32 m = 0; m < 3; m++)
					{
						Int32 vertexIndex = m_indexesArray[pmesh->start_index+l+m];
						Math::VectorAdd(s_tangents[vertexIndex], sdir, s_tangents[vertexIndex]);
						Math::VectorAdd(t_tangents[vertexIndex], tdir, t_tangents[vertexIndex]);
					}
				}
			}
		}
	}

	// Save final data into the write array
	for(Uint32 i = 0; i < m_numVertexes; i++)
	{
		vbmvertex_t& vertex = pfinalvertexes[i];

		Vector tangent;
		for(Uint32 j = 0; j < 3; j++)
			tangent[j] = (s_tangents[i][j] - vertex.normal[j] * Math::DotProduct(vertex.normal, s_tangents[i]));

		Math::VectorNormalize(tangent);
		Math::VectorCopy(tangent, vertex.tangent);

		Vector cross;
		Math::CrossProduct(vertex.normal, s_tangents[i], cross);
		vertex.tangent[3] = Math::DotProduct(cross, t_tangents[i]) > 0 ? 1 : -1;
	}
}

//===============================================
// @brief Optimize mesh data, check for bone merging, etc
//
//===============================================
void CVBMCompiler::OptimizeMeshes( void )
{
	if(m_pConversionGroupsArray.size() == 1)
		return;

	conversion_group_t** ppgrp1 = &m_pConversionGroupsArray[0];
	conversion_group_t* pgrp1 = *ppgrp1;

	Uint32 i = 0;
	for(Uint32 j = 1; j < m_pConversionGroupsArray.size(); j++)
	{
		conversion_group_t** ppgrp2 = &m_pConversionGroupsArray[j];
		conversion_group_t* pgrp2 = *ppgrp2;

		// Check if the last mesh shares this mesh's bones
		Uint32 l = 0;
		for(; l < pgrp2->numbones; l++)
		{
			Uint32 m = 0;
			for(; m < pgrp1->numbones; m++)
			{
				if(pgrp1->boneindexes[m] == pgrp2->boneindexes[l])
					break;
			}

			if(m == pgrp1->numbones)
				break;
		}

		if(l == pgrp2->numbones)
		{
			pgrp2->numbones = 0;
			pgrp2->bonegroup = i;
			continue;
		}

		// Now check if this mesh has the last mesh's bones
		for(l = 0; l < pgrp1->numbones; l++)
		{
			Uint32 m = 0;
			for(; m < pgrp2->numbones; m++)
			{
				if(pgrp2->boneindexes[m] == pgrp1->boneindexes[l])
					break;
			}

			if(m == pgrp2->numbones)
				break;
		}

		// If successful, swap the meshes
		if(l == pgrp1->numbones)
		{
			conversion_group_t* ptmp = pgrp2;
			(*ppgrp2) = (*ppgrp1);
			(*ppgrp1) = ptmp;

			(*ppgrp2)->numbones = 0;
			(*ppgrp2)->bonegroup = i;
			pgrp1 = ptmp;
			continue;
		}

		// See if we can merge the two lists
		CArray<Int32> newbones;
		for(l = 0; l < pgrp2->numbones; l++)
		{
			int m = 0;
			for(; m < pgrp1->numbones; m++)
			{
				if(pgrp1->boneindexes[m] == pgrp2->boneindexes[l])
					break;
			}

			if(m == pgrp1->numbones)
			{
				if( newbones.size() == MAX_SHADER_BONES )
					break;

				newbones.push_back(pgrp2->boneindexes[l]);
			}
		}

		// Only possible if we're under the limit
		if((pgrp1->numbones+newbones.size()) > MAX_SHADER_BONES)
		{
			pgrp1 = pgrp2; 
			i = j;
			continue;
		}

		// Merge in the new bones
		for(l = 0; l < newbones.size(); l++)
		{
			pgrp1->boneindexes[pgrp1->numbones] = newbones[l];
			pgrp1->numbones++;
		}

		pgrp2->numbones = 0;
		pgrp2->bonegroup = i;
	}
}

//===============================================
// @brief Write bone info in the VBM file
//
//===============================================
void CVBMCompiler::WriteBoneData( void )
{
	float matrix[3][4];

	Uint32 boneCount = m_studioCompiler.GetNbBones();
	m_pVBMHeader->boneinfooffset = m_pFileBuffer->getsize();
	m_pVBMHeader->numboneinfo = boneCount;

	Uint32 dataSize = boneCount * sizeof(vbmboneinfo_t);
	m_pFileBuffer->append(nullptr, dataSize);

	vbmboneinfo_t* pdestbones = reinterpret_cast<vbmboneinfo_t*>(reinterpret_cast<byte*>(m_pVBMHeader) + m_pVBMHeader->boneinfooffset);
	for(Uint32 i = 0; i < boneCount; i++)
	{
		const smdl::boneinfo_t* psrcbone = m_studioCompiler.GetBone(i);
		vbmboneinfo_t* pdestbone = &pdestbones[i];

		Uint32 maxSize = sizeof(vbmboneinfo_t::name);
		qstrcpy_s(pdestbone->name, psrcbone->name.c_str(), maxSize);

		pdestbone->parentindex = psrcbone->parent_index;
		Int32 pose_index = m_referenceFrame.bonemap_inverse[i];

		for(Uint32 j = 0; j < 3; j++)
			pdestbone->position[j] = psrcbone->position[j];

		for(Uint32 j = 0; j < 3; j++)
			pdestbone->angles[j] = psrcbone->rotation[j];

		for(Uint32 j = 0; j < 3; j++)
		{
			pdestbone->scale[j] = psrcbone->position_scale[j];
			pdestbone->scale[3+j] = psrcbone->rotation_scale[j];
		}

		// Get the bind transform matrix from the inverse base pose
		smdl::bone_transforminfo_t& transinfo = m_referenceFrame.bonetransforms[pose_index];
		Math::CopyMatrix(transinfo.matrix, matrix);

		// Invert the matrix for use as the bind transform during rendering
		CompilerMath::InvertMatrix(matrix, pdestbone->bindtransform);
	}
}

//===============================================
// @brief Write VBM flex controller data
//
//===============================================
void CVBMCompiler::WriteFlexControllers( void )
{
	Uint32 dataSize = m_flexControllerArray.size() * sizeof(vbmflexcontroller_t);
	m_pVBMHeader->flexcontrolleroffset = m_pFileBuffer->getsize();
	m_pVBMHeader->numflexcontrollers = m_flexControllerArray.size();
	m_pFileBuffer->append(nullptr, dataSize);

	vbmflexcontroller_t* pdestflexcontrollers = reinterpret_cast<vbmflexcontroller_t*>(reinterpret_cast<byte*>(m_pVBMHeader) + m_pVBMHeader->flexcontrolleroffset);
	for(Uint32 i = 0; i < m_flexControllerArray.size(); i++)
	{
		vbm::flexcontroller_t& srccontroller = m_flexControllerArray[i];
		vbmflexcontroller_t* pdestcontroller = &pdestflexcontrollers[i];

		Uint32 maxSize = sizeof(vbmflexcontroller_t::name);
		qstrcpy_s(pdestcontroller->name, srccontroller.name.c_str(), maxSize);

		pdestcontroller->interpmode = srccontroller.type;
		pdestcontroller->minvalue = srccontroller.minvalue;
		pdestcontroller->maxvalue = srccontroller.maxvalue;
	}
}

//===============================================
// @brief Write flex data to the buffer
//
//===============================================
void CVBMCompiler::WriteFlexData( void )
{
	Uint32 maxflexverts = SDL_floor((VBM_FLEXTEXTURE_SIZE / 3) * VBM_FLEXTEXTURE_SIZE);
	Uint32 nbSubmodels = m_studioCompiler.GetNbSubmodels();

	// Count the number of flexes
	Uint32 numflexmeshes = 0;
	for(Uint32 i = 0; i < m_pSrcSubmodelsArray.size(); i++)
	{
		const smdl::submodel_t* psrcsubmodel = m_pSrcSubmodelsArray[i];
		if(psrcsubmodel->pflexmodel && !psrcsubmodel->pflexmodel->pflexes.empty())
			numflexmeshes++;
	}

	// No flexes
	if(!numflexmeshes)
		return;

	// Set the flag
	m_pVBMHeader->flags |= VBM_HAS_FLEXES;

	// Set the data in the vbm file
	m_pVBMHeader->flexinfooffset = m_pFileBuffer->getsize();
	m_pVBMHeader->numflexinfo = numflexmeshes;
	Uint32 dataSize = sizeof(vbmflexinfo_t)*numflexmeshes;
	m_pFileBuffer->append(nullptr, dataSize);

	Uint32 flexindex = 0;
	vbmflexinfo_t* pflexinfos = reinterpret_cast<vbmflexinfo_t*>(reinterpret_cast<byte*>(m_pVBMHeader) + m_pVBMHeader->flexinfooffset);
	m_pFileBuffer->addpointer(reinterpret_cast<void**>(&pflexinfos));
	
	for(Uint32 i = 0; i < m_pSrcSubmodelsArray.size(); i++)
	{
		const smdl::submodel_t* psrcsubmodel = m_pSrcSubmodelsArray[i];
		if(!psrcsubmodel->pflexmodel || psrcsubmodel->pflexmodel->pflexes.empty())
			continue;

		vbmflexinfo_t* pflexinfo = &pflexinfos[flexindex];
		flexindex++;

		m_pFileBuffer->addpointer(reinterpret_cast<void**>(&pflexinfo));

		// Set the indexes into the flex controller array
		pflexinfo->flexcontrolleridxoffset = m_pFileBuffer->getsize();
		pflexinfo->numflexes = psrcsubmodel->pflexmodel->pflexes.size();
		dataSize = psrcsubmodel->pflexmodel->pflexes.size() * sizeof(byte);
		m_pFileBuffer->append(nullptr, dataSize);

		// Seek out the controllers managing this flexinfo
		byte* pflexcontrollerindexes = (reinterpret_cast<byte*>(m_pVBMHeader) + pflexinfo->flexcontrolleridxoffset);
		m_pFileBuffer->addpointer(reinterpret_cast<void**>(&pflexcontrollerindexes));
		
		for(Uint32 j = 0; j < pflexinfo->numflexes; j++)
			pflexcontrollerindexes[j] = 255;

		// Link up with flex controllers
		for(Uint32 j = 0; j < m_flexControllerArray.size(); j++)
		{
			vbm::flexcontroller_t& controller = m_flexControllerArray[j];
			for(Uint32 k = 0; k < controller.vtas.size(); k++)
			{
				vbm::flexcontroller_vta_t& flexvta = controller.vtas[k];
				if(!qstrcmp(psrcsubmodel->pflexmodel->name, flexvta.name))
				{
					pflexcontrollerindexes[flexvta.flexindex+1] = j;
					break;
				}
			}
		}

		// Disregard "neutral" when checking for errors
		for(Uint32 j = 1; j < pflexinfo->numflexes; j++)
		{
			if(pflexcontrollerindexes[j] == 255)
				WarningMsg("Warning: Flex %d is not referenced by any flex controller in VTA %s.vta\n", j, psrcsubmodel->pflexmodel->name.c_str());
		}

		// Count the total amount of flex vertexes
		Uint32 numflexverts = 0;
		for(Uint32 j = 0; j < psrcsubmodel->pflexmodel->pflexes.size(); j++)
		{
			smdl::flexframe_t* pframe = psrcsubmodel->pflexmodel->pflexes[j];
			numflexverts += pframe->vertexes.size();
		}

		// Allocate buffer for all flex verts
		pflexinfo->flexvertoffset = m_pFileBuffer->getsize();
		pflexinfo->numflexvert = numflexverts;
		dataSize = sizeof(vbmflexvertex_t)*numflexverts;
		m_pFileBuffer->append(nullptr, dataSize);

		// Allocate flex vertex infos
		Uint32 vertexCount = psrcsubmodel->pflexmodel->pflexes[0]->vertexes.size();
		pflexinfo->flexvertinfooffset = m_pFileBuffer->getsize();
		pflexinfo->numflexvertinfo = vertexCount;

		dataSize = sizeof(vbmflexvertinfo_t) * vertexCount;
		m_pFileBuffer->append(nullptr, dataSize);
		
		// Clear out the flex info offsets
		vbmflexvertinfo_t* pflexvertinfo = reinterpret_cast<vbmflexvertinfo_t*>(reinterpret_cast<byte *>(m_pVBMHeader) + pflexinfo->flexvertinfooffset);
		for(Uint32 j = 0; j < vertexCount; j++)
		{
			for(Uint32 k = 0; k < MAX_VBM_FLEXES; k++)
				pflexvertinfo[j].vertinfoindexes[k] = -1;
		}

		// Set the vertex data
		vbmflexvertex_t* pflexverts = reinterpret_cast<vbmflexvertex_t *>(reinterpret_cast<byte*>(m_pVBMHeader) + pflexinfo->flexvertoffset);
		for(Uint32 j = 0, index = 0; j < psrcsubmodel->pflexmodel->pflexes.size(); j++)
		{
			smdl::flexframe_t* pflex = psrcsubmodel->pflexmodel->pflexes[j];
			for(Uint32 k = 0; k < pflex->vertexes.size(); k++)
			{
				const smdl::flexvertex_t& srcvertex = pflex->vertexes[k];
				vbmflexvertex_t* pdestvertex = &pflexverts[index];

				// Rotate into the reference pose
				Int32 boneindex = m_referenceFrame.bonemap_inverse[srcvertex.boneindex];
				smdl::bone_transforminfo_t& transInfo = m_referenceFrame.bonetransforms[boneindex];

				Math::VectorRotate(srcvertex.origin, transInfo.matrix, pdestvertex->originoffset);
				Math::VectorRotate(srcvertex.normal, transInfo.matrix, pdestvertex->normaloffset);

				// Determine how many verts we can have per row
				Int32 row_verts = VBM_FLEXTEXTURE_SIZE/3;
				Int32 tcy = (srcvertex.vertexindex + 1) / row_verts;
				Int32 tcx = (srcvertex.vertexindex + 1) % row_verts;

				pflexverts[index].offset = (tcy*VBM_FLEXTEXTURE_SIZE + tcx)*3;

				pflexvertinfo[srcvertex.vertexindex].vertinfoindexes[j] = index;
				index++;
			}
		}

		const Char* pstrVTAName = psrcsubmodel->pflexmodel->name.c_str();
		Float limitPercentage = (static_cast<Float>(vertexCount) / static_cast<Float>(maxflexverts))*100.0f;

		Msg("\t- VTA %s - flex verts %d/%d(%.2f)\n", pstrVTAName, vertexCount, maxflexverts, limitPercentage);

		m_pFileBuffer->removepointer(reinterpret_cast<void**>(&pflexcontrollerindexes));
		m_pFileBuffer->removepointer(reinterpret_cast<void**>(&pflexinfo));
	}

	m_pFileBuffer->removepointer(reinterpret_cast<void**>(&pflexinfos));
}

//===============================================
// @brief Convert weight float to byte data
//
// @param value Value to convert to a byte
// @return The value converted to the 0-255 range
//===============================================
byte CVBMCompiler::WeightToByte( Float value )
{
	Float flvalue = value*255.0f;
	if(flvalue > 255.0f)
		flvalue = 255.0f;
	else if(flvalue < 0)
		flvalue = 0;

	Float fraction = flvalue-floor(flvalue);
	if(fraction >= 0.5)
		return static_cast<byte>(SDL_ceil(flvalue));
	else
		return static_cast<byte>(SDL_floor(flvalue));
}

//===============================================
// @brief Loads reference frame file
//
// @return TRUE on success, FAIL if an error occurred
//===============================================
bool CVBMCompiler::LoadReferenceFrameFile( void )
{
	// Try loading the file first
	CReferenceFrameSMDParser parser(m_studioCompiler, m_referenceFrame);
	if(parser.ProcessFile(REFERENCE_FRAME_FILENAME))
		return true;

	// Ideally the user should specify their own reference_frame.smd, but most
	// of the time it should be okay to use the global table as a reference
	WarningMsg("Reference frame file '%s.smd' not found or invalid, defaulting to global table.\n", REFERENCE_FRAME_FILENAME);
	m_referenceFrame.clear();

	// If failed, copy from the global table
	Uint32 boneNb = m_studioCompiler.GetNbBones();

	// Set up bones
	m_referenceFrame.nodes.resize(boneNb);
	m_referenceFrame.bones.resize(boneNb);
	m_referenceFrame.bonetransforms.resize(boneNb);
	m_referenceFrame.bonemap_inverse.resize(boneNb);

	for(Uint32 i = 0; i < boneNb; i++)
	{
		const smdl::boneinfo_t* psrcbone = m_studioCompiler.GetBone(i);

		// Set the bone node
		smdl::bone_node_t& destnode = m_referenceFrame.nodes[i];
		destnode.bonename = psrcbone->name;
		destnode.parentindex = psrcbone->parent_index;
		destnode.ismirrored = false; // Already taken care of

		// Set bone
		smdl::bone_t& destbone = m_referenceFrame.bones[i];
		destbone.globalindex = i;
		destbone.position = psrcbone->position;
		destbone.rotation = psrcbone->rotation;
		destbone.refcounter = 1; // Doesn't matter

		// The indexes are 1:1
		m_referenceFrame.bonemap_inverse[i] = i;

		// Set transforms
		CompilerMath::SetupBoneTransforms(destnode, destbone, m_referenceFrame.bonetransforms[i], m_referenceFrame.bonetransforms);
	}

	return true;
}

//===============================================
// @brief Writes the final output
//
// @return TRUE on success, FAIL if an error occurred
//===============================================
bool CVBMCompiler::CreateVBMFile( void )
{
	// Must be called first
	if(!LoadReferenceFrameFile())
	{
		ErrorMsg("Failed to load reference frame file.\n");
		return false;
	}

	m_pFileBuffer = new CBuffer(VBM_FILEBUFFER_ALLOC_SIZE);

	// Allocate and clear the data
	m_pVBMHeader = reinterpret_cast<vbmheader_t*>(m_pFileBuffer->getbufferdata());
	m_pFileBuffer->addpointer(reinterpret_cast<void**>(&m_pVBMHeader));
	m_pFileBuffer->append(nullptr, sizeof(vbmheader_t));

	// Set basic stuff
	m_pVBMHeader->id = VBM_HEADER;

	Uint32 maxSize = sizeof(vbmheader_t::name);
	qstrcpy_s(m_pVBMHeader->name, g_options.outputname.c_str(), maxSize);

	// Set up texture info array first, we'll need it
	Uint32 nbTextures = m_studioCompiler.GetNbTextures();
	Uint32 dataSize = nbTextures * sizeof(vbmtexture_t);

	m_pVBMHeader->textureoffset = m_pFileBuffer->getsize();
	m_pVBMHeader->numtextures = nbTextures;
	m_pFileBuffer->append(nullptr, dataSize);
	
	vbmtexture_t* pvbmtextures = reinterpret_cast<vbmtexture_t*>(reinterpret_cast<byte*>(m_pVBMHeader) + m_pVBMHeader->textureoffset);
	m_pFileBuffer->addpointer(reinterpret_cast<void**>(&pvbmtextures));

	for(Uint32 i = 0; i < nbTextures; i++)
	{
		const smdl::texture_t* psrctexture = m_studioCompiler.GetTexture(i);
		vbmtexture_t* pdesttexture = &pvbmtextures[i];

		maxSize = sizeof(vbmtexture_t::name);
		qstrcpy_s(pdesttexture->name, psrctexture->name.c_str(), maxSize);

		pdesttexture->flags = psrctexture->flags;
		pdesttexture->width = psrctexture->width;
		pdesttexture->height = psrctexture->height;
	}

	// Allocate all submodels
	Uint32 nbSubmodels = m_studioCompiler.GetNbSubmodels();
	Int32 submodelOffset = m_pFileBuffer->getsize();
	dataSize = nbSubmodels * sizeof(vbmsubmodel_t);
	m_pFileBuffer->append(nullptr, dataSize);

	vbmsubmodel_t* pvbmsubmodels = reinterpret_cast<vbmsubmodel_t*>(reinterpret_cast<byte*>(m_pVBMHeader) + submodelOffset);
	m_pFileBuffer->addpointer(reinterpret_cast<void**>(&pvbmsubmodels));

	for(Uint32 i = 0; i < nbSubmodels; i++)
		m_pSrcSubmodelsArray.push_back(m_studioCompiler.GetSubModel(i));

	// Allocate body parts
	m_pVBMHeader->numbodyparts = m_studioCompiler.GetNbBodyParts();
	m_pVBMHeader->bodypartoffset = m_pFileBuffer->getsize();
	dataSize = m_pVBMHeader->numbodyparts * sizeof(vbmbodypart_t);
	m_pFileBuffer->append(nullptr, dataSize);

	vbmbodypart_t* pvbmbodyparts = reinterpret_cast<vbmbodypart_t*>(reinterpret_cast<byte*>(m_pVBMHeader) + m_pVBMHeader->bodypartoffset);
	m_pFileBuffer->addpointer(reinterpret_cast<void**>(&pvbmbodyparts));

	// Set up the bodyparts
	for (Uint32 i = 0, j = 0; i < m_pVBMHeader->numbodyparts; i++)
	{
		const smdl::bodypart_t* psrcbodypart = m_studioCompiler.GetBodyPart(i);
		vbmbodypart_t* pdestbodypart = &pvbmbodyparts[i];

		maxSize = sizeof(vbmbodypart_t::name);
		qstrcpy_s(pdestbodypart->name, psrcbodypart->name.c_str(), maxSize);

		pdestbodypart->base = psrcbodypart->base;
		pdestbodypart->numsubmodels = psrcbodypart->psubmodels.size();
		pdestbodypart->submodeloffset = reinterpret_cast<byte *>(&pvbmsubmodels[j]) - reinterpret_cast<byte*>(m_pVBMHeader);
		j += psrcbodypart->psubmodels.size();
	}

	// Write flex information
	WriteFlexControllers();
	WriteFlexData();

	// Write bones
	WriteBoneData();

	// Set this to zero
	m_currentFlexIndex = 0;
	m_pFileBuffer->addpointer(reinterpret_cast<void**>(&m_pDestinationSubmodel));

	// Now set up the data required
	for (Uint32 i = 0; i < m_pSrcSubmodelsArray.size(); i++)
	{
		m_pSourceSubmodel = m_pSrcSubmodelsArray[i];
		m_pDestinationSubmodel = &pvbmsubmodels[i];
		
		// Process normal submodels
		if(!ProcessCurrentSubmodel())
		{
			ErrorMsg("Error encountered while processing VBM submodel '%s'.'\n", m_pSourceSubmodel->name.c_str());
			return false;
		}

		// Process any LODs
		if(m_pDestinationSubmodel->numlods)
		{
			dataSize = m_pSourceSubmodel->plods.size() * sizeof(vbmlod_t);
			m_pDestinationSubmodel->lodoffset = m_pFileBuffer->getsize();
			m_pDestinationSubmodel->numlods = m_pSourceSubmodel->plods.size();
			m_pFileBuffer->append(nullptr, dataSize);

			vbmlod_t* pdestlods = reinterpret_cast<vbmlod_t*>(reinterpret_cast<byte*>(m_pVBMHeader) + m_pDestinationSubmodel->lodoffset);
			m_pFileBuffer->addpointer(reinterpret_cast<void**>(&pdestlods));

			for(Uint32 j = 0; j < m_pSourceSubmodel->plods.size(); j++)
			{
				vbmlod_t* pdestlod = &pdestlods[j];
				smdl::lod_t* psrclod = m_pSourceSubmodel->plods[j];
				m_pFileBuffer->addpointer(reinterpret_cast<void**>(&pdestlod));

				dataSize = sizeof(vbmsubmodel_t);
				pdestlod->submodeloffset = m_pFileBuffer->getsize();
				m_pFileBuffer->append(nullptr, dataSize);

				pdestlod->type = static_cast<vbmlod_type_t>(psrclod->lodtype);
				pdestlod->distance = psrclod->distance;

				m_pSourceSubmodel = psrclod->plodmodel;
				m_pDestinationSubmodel = reinterpret_cast<vbmsubmodel_t*>(reinterpret_cast<byte*>(m_pVBMHeader) + pdestlod->submodeloffset);

				// Process normal submodels
				if(!ProcessCurrentSubmodel())
				{
					ErrorMsg("Error encountered while processing VBM LOD submodel '%s'.'\n", m_pSourceSubmodel->name.c_str());
					return false;
				}

				m_pFileBuffer->removepointer(reinterpret_cast<void**>(&pdestlod));
			}

			m_pFileBuffer->removepointer(reinterpret_cast<void**>(&pdestlods));
		}
	}

	m_pFileBuffer->removepointer(reinterpret_cast<void**>(&m_pDestinationSubmodel));

	// Convert VBO data to the optimized format
	m_pVBMHeader->vertexoffset = m_pFileBuffer->getsize();
	m_pVBMHeader->numverts = m_numVertexes;
	dataSize = m_numVertexes * sizeof(vbmvertex_t);
	m_pFileBuffer->append(nullptr, dataSize);

	// Copy the needed data
	vbmvertex_t* pwritevertexes = reinterpret_cast<vbmvertex_t*>(reinterpret_cast<byte*>(m_pVBMHeader) + m_pVBMHeader->vertexoffset);
	for(Uint32 i = 0; i < m_numVertexes; i++)
	{
		// Rotate into the reference pose
		const final_vertex_t& srcvertex = m_vertexesArray[i];
		vbmvertex_t* pdestvertex = &pwritevertexes[i];

		Int32 boneindex = m_referenceFrame.bonemap_inverse[srcvertex.refboneindex];
		smdl::bone_transforminfo_t& transInfo = m_referenceFrame.bonetransforms[boneindex];
		
		Math::VectorTransform(srcvertex.origin, transInfo.matrix, pdestvertex->origin);
		Math::VectorRotate(srcvertex.normal, transInfo.matrix, pdestvertex->normal);

		for(Uint32 j = 0; j < 2; j++)
			pdestvertex->texcoord[j] = srcvertex.texcoords[j];

		pdestvertex->flexvertindex = srcvertex.flexvertexindex;

		for(Uint32 j = 0; j < MAX_VBM_BONEWEIGHTS; j++)
		{
			pdestvertex->boneindexes[j] = srcvertex.boneindexes[j];
			pdestvertex->boneweights[j] = WeightToByte(srcvertex.boneweights[j]);
		}
	}

	// Calculate the tangents
	CalculateTangents( pwritevertexes );

	// Copy over index array
	m_pVBMHeader->indexoffset = m_pFileBuffer->getsize();
	m_pVBMHeader->numindexes = m_numIndexes;
	dataSize = m_numIndexes * sizeof(Uint32);
	m_pFileBuffer->append(&m_indexesArray[0], dataSize);

	// Set skinfamilies and skinrefs
	m_pVBMHeader->skinoffset = m_pFileBuffer->getsize();
	m_pVBMHeader->numskinfamilies = m_studioCompiler.GetNbSkinFamilies();
	m_pVBMHeader->numskinref = m_studioCompiler.GetNbSkinRefs();
	dataSize = sizeof(Int16)*m_pVBMHeader->numskinfamilies*m_pVBMHeader->numskinref;
	m_pFileBuffer->append(nullptr, dataSize);

	Int16* pdestskinrefs = reinterpret_cast<Int16*>(reinterpret_cast<byte*>(m_pVBMHeader) + m_pVBMHeader->skinoffset);
	for(Uint32 i = 0; i < m_pVBMHeader->numskinfamilies; i++)
	{
		for(Uint32 j = 0; j < m_pVBMHeader->numskinref; j++)
			pdestskinrefs[i*m_pVBMHeader->numskinref+j] = m_skinRefsArray[i][j];
	}

	// Set final size
	m_pVBMHeader->size = m_pFileBuffer->getsize();

	Msg("VBM compile done: %d vertexes, %d indexes, %d bodyparts, %d submodels.\n\tOutput size: %.f mbytes.\n",
		m_numVertexes, m_numIndexes, m_pVBMHeader->numbodyparts, m_pSrcSubmodelsArray.size(), Common::BytesToMegaBytes(m_pVBMHeader->size));

	// Write the output
	bool result = WriteFile();

	m_pFileBuffer->removepointer(reinterpret_cast<void**>(&pvbmbodyparts));
	m_pFileBuffer->removepointer(reinterpret_cast<void**>(&pvbmsubmodels));
	m_pFileBuffer->removepointer(reinterpret_cast<void**>(&pvbmtextures));

	// Clear everything out
	Clear();

	return result;
}

//===============================================
// @brief Writes the final output
//
// @return TRUE on success, FAIL if an error occurred
//===============================================
bool CVBMCompiler::WriteFile( void )
{
	CString outputPath;
	if(g_options.outputname.find(0, ":") != CString::CSTRING_NO_POSITION)
		outputPath << g_options.outputname << ".vbm";
	else
		outputPath << g_options.basedirectory << PATH_SLASH_CHAR << g_options.outputname << ".vbm";

	outputPath = Common::CleanupPath(outputPath.c_str());

	const byte* pdata = reinterpret_cast<const byte*>(m_pFileBuffer->getbufferdata());
	if(!g_fileInterface.pfnWriteFile(pdata, m_pVBMHeader->size, outputPath.c_str(), false))
	{
		ErrorMsg("Failed to open %s for writing: %s.\n", outputPath.c_str());
		return false;
	}

	Msg("Wrote VBM file '%s', %.2f mbytes.\n", outputPath.c_str(), Common::BytesToMegaBytes(m_pVBMHeader->size));
	return true;
}