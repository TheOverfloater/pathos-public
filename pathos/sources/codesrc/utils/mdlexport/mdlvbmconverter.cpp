/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.

===============================================
*/

#pragma warning( disable : 4244 )
#pragma warning( disable : 4237 )
#pragma warning( disable : 4305 )

#include <SDL.h>
#ifdef WIN32
#include <Windows.h>
#endif

#include "includes.h"
#include "common.h"
#include "studio.h"
#include "constants.h"
#include "tgaformat.h"
#include "mdlvbmconverter.h"
#include "com_math.h"
#include "main.h"
#include "utils_common.h"
#include "utils_filefuncs.h"

// Object definition
CMDLVBMConverter gMDLVBMConverter;

// Alloc size for VBM file buffer
const Uint32 CMDLVBMConverter::VBM_TEMP_FILE_ALLOC_SIZE	= 1024*1024*1;
// Vertex alloc size
const Uint32 CMDLVBMConverter::VBM_VERTEXES_ALLOC_SIZE	= 32768;
// Triangle index alloc size
const Uint32 CMDLVBMConverter::VBM_INDEXES_ALLOC_SIZE	= 32768;
// Triangle alloc size
const Uint32 CMDLVBMConverter::VBM_TRIANGLES_ALLOC_SIZE	= 512;

//===============================================
// @brief Constructor for CMDLVBMConverter class
//
//===============================================
CMDLVBMConverter::CMDLVBMConverter( void ):
	m_pFileBuffer(nullptr),
	m_pVBMHeader(nullptr),
	m_pStudioHeader(nullptr),
	m_pTextureHeader(nullptr),
	m_pStudioVertexes(nullptr),
	m_pStudioNormals(nullptr),
	m_numVBMVerts(0),
	m_numIndexes(0),
	m_pCurrentGroup(nullptr),
	m_numConversionTriangles(0),
	m_numProcessed(0),
	m_numRefVerts(0),
	m_currentStartVertex(0),
	m_nbBytesWritten(0)
{
}

//===============================================
// @brief Destructor for CMDLVBMConverter class
//
//===============================================
CMDLVBMConverter::~CMDLVBMConverter( void )
{
	Reset();
}

//===============================================
// @brief Resets the object
//
//===============================================
void CMDLVBMConverter::Reset( void )
{
	if(m_pFileBuffer)
	{
		delete m_pFileBuffer;
		m_pFileBuffer = nullptr;
	}

	m_pStudioHeader = nullptr;
	m_pTextureHeader = nullptr;

	m_pStudioVertexes = nullptr;
	m_pStudioNormals = nullptr;

	if(!m_vbmVertexesArray.empty())
		m_vbmVertexesArray.clear();

	m_numVBMVerts = 0;

	if(!m_vbmIndexesArray.empty())
		m_vbmIndexesArray.clear();

	m_numIndexes = 0;

	if(!m_pConversionGroups.empty())
	{
		for(Uint32 i = 0; i < m_pConversionGroups.size(); i++)
			delete m_pConversionGroups[i];

		m_pConversionGroups.clear();
	}

	m_pCurrentGroup = nullptr;

	if(!m_conversionTrianglesArray.empty())
		m_conversionTrianglesArray.clear();

	m_numConversionTriangles = 0;
	m_numProcessed = 0;

	if(!m_referenceVertexArray.empty())
		m_referenceVertexArray.clear();

	m_numRefVerts = 0;
	m_currentStartVertex = 0;
	m_nbBytesWritten = 0;
}

//===============================================
// @brief Converts an MDL file to a VBM file
//
// @param pstrModelFilePath Path to mdl file
// @param pstrOutputPath Output folder path
// @param pstudiomdlfile Pointer to studiomdl file
// @param ptexturemdlfile Pointer to texture hdr file
// @return TRUE if successful, FALSE otherwise
//===============================================
bool CMDLVBMConverter::ConvertModel( const Char* pstrModelFilePath, const Char* pstrOutputPath, const byte* pstudiomdlfile, const byte* ptexturemdlfile )
{
	// Ensure everything is cleared
	Reset();

	m_pStudioHeader = reinterpret_cast<const studiohdr_t*>(pstudiomdlfile);
	m_pTextureHeader = reinterpret_cast<const studiohdr_t*>(ptexturemdlfile);

	Uint32 numsubmodels = 0;
	for (Int32 i = 0; i < m_pStudioHeader->numbodyparts; i++)
	{
		const mstudiobodyparts_t* pbodypart = m_pStudioHeader->getBodyPart(i);
		numsubmodels += pbodypart->nummodels;
	}

	m_pFileBuffer = new CBuffer(VBM_TEMP_FILE_ALLOC_SIZE);
	m_pVBMHeader = reinterpret_cast<vbmheader_t*>(m_pFileBuffer->getbufferdata());
	m_pFileBuffer->addpointer(reinterpret_cast<void**>(&m_pVBMHeader));
	m_pFileBuffer->append(nullptr, sizeof(vbmheader_t));

	// Set basic stuff
	m_pVBMHeader->id = VBM_HEADER;
	qstrcpy_s(m_pVBMHeader->name, pstrModelFilePath, sizeof(vbmheader_t::name));

	Uint32 submodeldataoffset = m_pFileBuffer->getdatasize();
	vbmsubmodel_t* psubmodels = reinterpret_cast<vbmsubmodel_t *>(reinterpret_cast<byte *>(m_pVBMHeader) + submodeldataoffset);
	m_pFileBuffer->addpointer(reinterpret_cast<void**>(&psubmodels));
	m_pFileBuffer->append(nullptr, sizeof(vbmsubmodel_t) * numsubmodels);

	// Set up bodyparts	
	m_pVBMHeader->numbodyparts = m_pStudioHeader->numbodyparts;
	m_pVBMHeader->bodypartoffset = m_pFileBuffer->getdatasize();
	m_pFileBuffer->append(nullptr, sizeof(vbmbodypart_t) * m_pVBMHeader->numbodyparts);

	Uint32 offset = 0;
	for(Int32 i = 0; i < m_pStudioHeader->numbodyparts; i++)
	{
		const mstudiobodyparts_t* pbodypart = m_pStudioHeader->getBodyPart(i);
		vbmbodypart_t* pvbmbodypart = reinterpret_cast<vbmbodypart_t*>(reinterpret_cast<byte*>(m_pVBMHeader) + m_pVBMHeader->bodypartoffset) + i;

		pvbmbodypart->base = pbodypart->base;
		qstrcpy_s(pvbmbodypart->name, pbodypart->name, sizeof(vbmbodypart_t::name));
		pvbmbodypart->numsubmodels = pbodypart->nummodels;

		pvbmbodypart->submodeloffset = submodeldataoffset + sizeof(vbmsubmodel_t)*offset;
		offset += pbodypart->nummodels;
	}

	// Set up skin families
	m_pVBMHeader->numskinfamilies = m_pTextureHeader->numskinfamilies;
	m_pVBMHeader->numskinref = m_pTextureHeader->numskinref;
	m_pVBMHeader->skinoffset = m_pFileBuffer->getdatasize();

	const Int16* pskinref = reinterpret_cast<const Int16*>(reinterpret_cast<const byte*>(m_pTextureHeader) + m_pTextureHeader->skinindex);
	Uint32 skindatasize = sizeof(Int16)*(m_pVBMHeader->numskinfamilies*m_pVBMHeader->numskinref);
	m_pFileBuffer->append(pskinref, skindatasize);

	// Copy textures
	m_pVBMHeader->numtextures = m_pTextureHeader->numtextures;
	m_pVBMHeader->textureoffset = m_pFileBuffer->getdatasize();
	m_pFileBuffer->append(nullptr, sizeof(vbmtexture_t)*m_pVBMHeader->numtextures);

	vbmtexture_t* pvbmtextures = reinterpret_cast<vbmtexture_t*>(reinterpret_cast<byte*>(m_pVBMHeader) + m_pVBMHeader->textureoffset);
	for(Int32 i = 0; i < m_pVBMHeader->numtextures; i++)
	{
		const mstudiotexture_t* ptexture = m_pTextureHeader->getTexture(i);
		vbmtexture_t* pvbmtexture = &pvbmtextures[i];

		pvbmtexture->flags = ptexture->flags;
		pvbmtexture->height = ptexture->height;
		pvbmtexture->width = ptexture->width;
		
		// Erase any spaces from the filename
		CString texturename(ptexture->name);
		Uint32 ofs = 0;
		while(true)
		{
			ofs = texturename.find(ofs, " ");
			if(ofs == CString::CSTRING_NO_POSITION)
				break;

			texturename.erase(ofs, 1);
			texturename.insert(ofs, "_");
		}

		qstrcpy_s(pvbmtexture->name, texturename.c_str(), sizeof(vbmtexture_t::name));
	}

	// Set bones
	m_pVBMHeader->numboneinfo = m_pStudioHeader->numbones;
	m_pVBMHeader->boneinfooffset = m_pFileBuffer->getdatasize();
	m_pFileBuffer->append(nullptr, sizeof(vbmboneinfo_t)*m_pVBMHeader->numboneinfo);

	vbmboneinfo_t* pvbmbones = reinterpret_cast<vbmboneinfo_t*>(reinterpret_cast<byte*>(m_pVBMHeader) + m_pVBMHeader->boneinfooffset);
	for(Int32 i = 0; i < m_pStudioHeader->numbones; i++)
	{
		const mstudiobone_t* pbone = m_pStudioHeader->getBone(i);
		
		for(Uint32 j = 0; j < 3; j++)
			pvbmbones[i].position[j] = pbone->value[j];

		for(Uint32 j = 0; j < 3; j++)
			pvbmbones[i].angles[j] = pbone->value[3+j];

		 pvbmbones[i].index = i;
		 qstrcpy_s(pvbmbones[i].name, pbone->name, sizeof(vbmboneinfo_t::name));
		 pvbmbones[i].parentindex = pbone->parent;
		 pvbmbones[i].flags = pbone->flags;

		 for(Uint32 j = 0; j < 6; j++)
			 pvbmbones[i].scale[j] = pbone->scale[j];

		 for(Uint32 j = 0; j < 3; j++)
			pvbmbones[i].bindtransform[j][j] = 1.0;
	}

	// Convert meshes and save bodyparts
	offset = 0;
	for (Int32 i = 0 ; i < m_pStudioHeader->numbodyparts; i++)
	{
		const mstudiobodyparts_t* pbodypart = m_pStudioHeader->getBodyPart(i);
		for (Int32 k = 0; k < pbodypart->nummodels; k++)
		{
			// Get VBM submodel
			const mstudiomodel_t* pstudiosubmodel = pbodypart->getSubmodel(m_pStudioHeader, k);
			vbmsubmodel_t *pvbmsubmodel = &psubmodels[offset]; 
			offset++;

			pvbmsubmodel->flexinfoindex = -1;
			qstrcpy_s(pvbmsubmodel->name, pstudiosubmodel->name, sizeof(vbmsubmodel_t::name));
			m_pFileBuffer->addpointer(reinterpret_cast<void**>(&pvbmsubmodel));

			// Get ptrs to normal and vertex data
			m_pStudioNormals = pstudiosubmodel->getNormals(m_pStudioHeader);
			m_pStudioVertexes = pstudiosubmodel->getVertexes(m_pStudioHeader);

			Uint32 inumtris = 0;
			for (Int32 l = 0; l < pstudiosubmodel->nummesh; l++)
			{
				const mstudiomesh_t* pmesh = pstudiosubmodel->getMesh(m_pStudioHeader, l);
				inumtris += pmesh->numtris;
			}

			// Skip if empty
			if(!inumtris)
				continue;

			// allocate triangles
			m_conversionTrianglesArray.resize(inumtris);

			// Non-chrome first
			for (Int32 l = 0; l < pstudiosubmodel->nummesh; l++) 
			{
				const mstudiomesh_t* pmesh = pstudiosubmodel->getMesh(m_pStudioHeader, l);
				Int32 textureindex = pskinref[pmesh->skinref];
				const mstudiotexture_t* ptexture = m_pTextureHeader->getTexture(textureindex);

				if(ptexture->flags & STUDIO_NF_CHROME)
					continue;

				ProcessMesh(pmesh, pstudiosubmodel, ptexture);
			}

			// Now add chromed ones
			for (Int32 l = 0; l < pstudiosubmodel->nummesh; l++) 
			{
				const mstudiomesh_t* pmesh = pstudiosubmodel->getMesh(m_pStudioHeader, l);
				Int32 textureindex = pskinref[pmesh->skinref];
				const mstudiotexture_t* ptexture = m_pTextureHeader->getTexture(textureindex);

				if(!(ptexture->flags & STUDIO_NF_CHROME))
					continue;

				ProcessMesh(pmesh, pstudiosubmodel, ptexture);
			}

			// Merge anything you can
			CheckMergeGroups();
			
			// Process current meshes
			if(!ProcessConversionGroups(pvbmsubmodel))
			{
				ErrorMsg("Error encountered while converting '%s'.\n", pstrModelFilePath);
				Reset();
				return false;
			}

			m_pFileBuffer->removepointer(reinterpret_cast<void**>(&pvbmsubmodel));
		}
	}

	// Calculate tangents
	if(m_numVBMVerts > 0 && m_numIndexes > 0)
		CalculateTangents();

	// Convert VBO data to the optimized format
	if(m_numVBMVerts > 0)
	{
		m_pVBMHeader->vertexoffset = m_pFileBuffer->getdatasize();
		m_pVBMHeader->numverts = m_numVBMVerts;
		m_pFileBuffer->append(&m_vbmVertexesArray[0], sizeof(vbmvertex_t)*m_numVBMVerts);
	}

	// Copy over index array
	if(m_numIndexes > 0)
	{
		m_pVBMHeader->indexoffset = m_pFileBuffer->getdatasize();
		m_pVBMHeader->numindexes = m_numIndexes;
		m_pFileBuffer->append(&m_vbmIndexesArray[0], sizeof(Uint32)*m_numIndexes);
	}

	m_pVBMHeader->size = m_pFileBuffer->getdatasize();

	Msg("VBM conversion done: %d vertexes, %d indexes, %d bodyparts, %d submodels.\n\tOutput size: %.6f mbytes.\n",
		m_numVBMVerts, m_numIndexes, m_pVBMHeader->numbodyparts, numsubmodels, Common::BytesToMegaBytes(m_pVBMHeader->size));

	//
	// Write the data out
	//

	// Create base path
	CString outputPathBase;
	outputPathBase << pstrOutputPath << "/models/";

	if(!g_fileInterface.pfnCreateDirectory(outputPathBase.c_str()))
	{
		ErrorMsg("Failed to create directory '%s'.\n", outputPathBase.c_str());
		Reset();
		return false;
	}

	// Get basename
	CString basename;
	Common::Basename(pstrModelFilePath, basename);
	basename << ".vbm";

	CString outputPath;
	outputPath << outputPathBase << basename;

	const byte* pwritedata = reinterpret_cast<const byte*>(m_pFileBuffer->getbufferdata());
	Uint32 writedatasize = sizeof(byte)*m_pFileBuffer->getdatasize();

	if(!g_fileInterface.pfnWriteFile(pwritedata, writedatasize, outputPath.c_str(), false))
	{
		ErrorMsg("Failed to write file '%s'.\n", outputPath.c_str());
		Reset();
		return false;
	}

	m_nbBytesWritten = writedatasize;
	m_pFileBuffer->removepointer(reinterpret_cast<void**>(&m_pVBMHeader));
	m_pFileBuffer->removepointer(reinterpret_cast<void**>(&psubmodels));

	// Reset the class
	Reset();
	return true;
}

//===============================================
// @brief Converts a studiomodel mesh into a VBM mesh
//
// @param pstudiomesh Pointer to studiomodel mesh
// @param pstudiosubmodel Pointer to studiomodel submodel
// @param pstudiotexture Pointer to studiomodel texture
//===============================================
void CMDLVBMConverter::ProcessMesh( const mstudiomesh_t *pstudiomesh, const mstudiomodel_t *pstudiosubmodel, const mstudiotexture_t *pstudiotexture )
{
	if(!pstudiomesh->numtris)
		return;

	Int32 j = 0;
	const byte *pvertbone = (reinterpret_cast<const byte *>(m_pStudioHeader) + pstudiosubmodel->vertinfoindex);
	const Int16 *ptricmds = reinterpret_cast<const Int16 *>(reinterpret_cast<const byte *>(m_pStudioHeader) + pstudiomesh->triindex);
	while (j = *(ptricmds++))
	{	
		if (j > 0) 
		{
			// convert triangle strip
			studiotri_t* pconvtriangle = &m_conversionTrianglesArray[m_numConversionTriangles];
			m_numConversionTriangles++;

			j -= 3;
			studiovert_t indices[3];
			for(Int32 i = 0; i < 3; i++, ptricmds += 4)
			{
				indices[i].vertindex = ptricmds[0];
				indices[i].normindex = ptricmds[1];
				indices[i].texcoord[0] = ptricmds[2];
				indices[i].texcoord[1] = ptricmds[3];
				indices[i].boneindex = pvertbone[ptricmds[0]];

				memcpy(&pconvtriangle->verts[i], &indices[i], sizeof(studiovert_t));
			}

			bool reverse = false;
			for( ; j > 0; j--, ptricmds += 4)
			{
				indices[0] = indices[1];
				indices[1] = indices[2];
				indices[2].vertindex = ptricmds[0]; 
				indices[2].normindex = ptricmds[1];
				indices[2].texcoord[0] = ptricmds[2];
				indices[2].texcoord[1] = ptricmds[3];
				indices[2].boneindex = pvertbone[ptricmds[0]];

				pconvtriangle = &m_conversionTrianglesArray[m_numConversionTriangles];
				m_numConversionTriangles++;

				if (!reverse)
				{
					memcpy(&pconvtriangle->verts[0], &indices[2], sizeof(studiovert_t));
					memcpy(&pconvtriangle->verts[1], &indices[1], sizeof(studiovert_t));
					memcpy(&pconvtriangle->verts[2], &indices[0], sizeof(studiovert_t));
				}
				else
				{
					memcpy(&pconvtriangle->verts[0], &indices[0], sizeof(studiovert_t));
					memcpy(&pconvtriangle->verts[1], &indices[1], sizeof(studiovert_t));
					memcpy(&pconvtriangle->verts[2], &indices[2], sizeof(studiovert_t));
				}
				reverse = !reverse;
			}
		}
		else
		{
			// convert triangle fan
			studiotri_t* pconvtriangle = &m_conversionTrianglesArray[m_numConversionTriangles];
			m_numConversionTriangles++;

			j = -j-3;
			studiovert_t indices[3];
			for(Int32 i = 0; i < 3; i++, ptricmds += 4)
			{
				indices[i].vertindex = ptricmds[0];
				indices[i].normindex = ptricmds[1];
				indices[i].texcoord[0] = ptricmds[2];
				indices[i].texcoord[1] = ptricmds[3];
				indices[i].boneindex = pvertbone[ptricmds[0]];

				memcpy(&pconvtriangle->verts[i], &indices[i], sizeof(studiovert_t));
			}

			for( ; j > 0; j--, ptricmds += 4)
			{
				indices[1] = indices[2];
				indices[2].vertindex = ptricmds[0]; 
				indices[2].normindex = ptricmds[1];
				indices[2].texcoord[0] = ptricmds[2];
				indices[2].texcoord[1] = ptricmds[3];
				indices[2].boneindex = pvertbone[ptricmds[0]];

				pconvtriangle = &m_conversionTrianglesArray[m_numConversionTriangles];
				m_numConversionTriangles++;

				memcpy(&pconvtriangle->verts[0], &indices[0], sizeof(studiovert_t));
				memcpy(&pconvtriangle->verts[1], &indices[1], sizeof(studiovert_t));
				memcpy(&pconvtriangle->verts[2], &indices[2], sizeof(studiovert_t));
			}
		}
	}

	// Create new group
	Uint32 currentgroupindex = m_pConversionGroups.size();
	m_pConversionGroups.resize(m_pConversionGroups.size()+1);
	m_pConversionGroups[currentgroupindex] = new mesh_group_t();
	m_pCurrentGroup = m_pConversionGroups[currentgroupindex];

	// process and recurse through all bones
	const mstudiobone_t *pbones = reinterpret_cast<const mstudiobone_t *>(reinterpret_cast<const byte *>(m_pStudioHeader) + m_pStudioHeader->boneindex);
	for(Uint32 i = 0; i < m_pStudioHeader->numbones; i++)
	{
		if(pbones[i].parent != NO_POSITION)
			continue;

		RecursiveProcessBoneTriangles(i, &pbones[i]);
	}

	// fill in tex pointers
	for(Uint32 i = currentgroupindex; i < m_pConversionGroups.size(); i++)
	{
		m_pConversionGroups[i]->skinref = pstudiomesh->skinref;
		m_pConversionGroups[i]->ptexture = pstudiotexture;
	}
}

//===============================================
// @brief Processes a studiomodel bone's attached triangles into VBM data
//
// @param index Index of the current bone being processed
// @param pstudiobone Pointer to current studiomodel bone data
//===============================================
void CMDLVBMConverter::RecursiveProcessBoneTriangles( Uint32 index, const mstudiobone_t *pstudiobone )
{
	for(Uint32 i = 0; i < m_numConversionTriangles; i++)
	{
		studiotri_t& triangle = m_conversionTrianglesArray[i];
		if(triangle.processed)
			continue;

		if(triangle.verts[0].boneindex != index 
			&& triangle.verts[1].boneindex != index 
			&& triangle.verts[2].boneindex != index)
			continue;

		// Check what new bones we need to add, and if we need to, split
		if(m_pCurrentGroup->numbones)
		{
			Uint32 numadd = 0;
			for(Uint32 j = 0; j < 3; j++)
			{
				Int32 k = 0; 
				for(; k < m_pCurrentGroup->numbones; k++)
				{
					if(triangle.verts[j].boneindex == m_pCurrentGroup->bones[k])
						break;
				}

				if(k == m_pCurrentGroup->numbones)
					numadd++;
			}

			if((m_pCurrentGroup->numbones+numadd) > MAX_SHADER_BONES)
			{
				Uint32 currentgroupindex = m_pConversionGroups.size();
				m_pConversionGroups.resize(m_pConversionGroups.size()+1);
				m_pConversionGroups[currentgroupindex] = new mesh_group_t();
				m_pCurrentGroup = m_pConversionGroups[currentgroupindex];
			}
		}
		
		// Now determine the final bones we need to add
		for(Uint32 j = 0; j < 3; j++)
		{
			Uint32 k = 0; 
			for(; k < m_pCurrentGroup->numbones; k++)
			{
				if(triangle.verts[j].boneindex == m_pCurrentGroup->bones[k])
					break;
			}

			if(k == m_pCurrentGroup->numbones)
			{
				m_pCurrentGroup->bones[m_pCurrentGroup->numbones] = triangle.verts[j].boneindex;
				m_pCurrentGroup->numbones++;
			}
		}

		// Ensure we have enough in the array
		if(m_pCurrentGroup->numtriangles == m_pCurrentGroup->trianglesarray.size())
			m_pCurrentGroup->trianglesarray.resize(m_pCurrentGroup->trianglesarray.size() + VBM_TRIANGLES_ALLOC_SIZE);

		m_pCurrentGroup->trianglesarray[m_pCurrentGroup->numtriangles] = m_conversionTrianglesArray[i];
		m_pCurrentGroup->numtriangles++;

		// Mark as processed
		m_conversionTrianglesArray[i].processed = true;
	}

	for(Uint32 i = 0; i < m_pStudioHeader->numbones; i++)
	{
		const mstudiobone_t* pbone = m_pStudioHeader->getBone(i);
		if(pbone->parent != index || i == index)
			continue;

		RecursiveProcessBoneTriangles(i, pbone);
	}
}

//===============================================
// @brief Processes a studiomodel bone's attached 
// triangles into VBM data
//
// @param pvbmsubmodel Pointer to vbm submodel 
// we're processing conversion groups for
//===============================================
bool CMDLVBMConverter::ProcessConversionGroups( vbmsubmodel_t* pvbmsubmodel )
{
	pvbmsubmodel->meshoffset = m_pFileBuffer->getdatasize();
	pvbmsubmodel->nummeshes = m_pConversionGroups.size();
	m_pFileBuffer->append(nullptr, sizeof(vbmmesh_t)*m_pConversionGroups.size());

	vbmmesh_t* pvbmmeshes = reinterpret_cast<vbmmesh_t *>(reinterpret_cast<byte *>(m_pVBMHeader) + pvbmsubmodel->meshoffset);
	m_pFileBuffer->addpointer(reinterpret_cast<void**>(&pvbmmeshes));

	// Process the vertex and triangle data
	for(Uint32 i = 0; i < pvbmsubmodel->nummeshes; i++)
	{
		mesh_group_t* pconvgroup = m_pConversionGroups[i];
		vbmmesh_t* pvbmmesh = &pvbmmeshes[i];
		m_pFileBuffer->addpointer(reinterpret_cast<void**>(&pvbmmesh));

		pvbmmesh->start_index = m_numIndexes;
		pvbmmesh->skinref = pconvgroup->skinref;

		if(pconvgroup->numbones)
		{
			// Save bone data
			pvbmmesh->boneoffset = m_pFileBuffer->getdatasize();
			pvbmmesh->numbones = pconvgroup->numbones;
			m_pFileBuffer->append(pconvgroup->bones, sizeof(byte)*pconvgroup->numbones);

			// Change current start vertex index
			m_currentStartVertex = m_numRefVerts;
		}

		for(Uint32 j = 0; j < pconvgroup->numtriangles; j++)
		{
			for(Int32 l = 0; l < 3; l++)
			{
				studiovert_t *pvertex = &pconvgroup->trianglesarray[j].verts[l];
				if(!ProcessVertex(pvbmsubmodel, pvertex, pconvgroup))
					return false;
			}
		}

		// set end
		pvbmmeshes[i].num_indexes = m_numIndexes-pvbmmeshes[i].start_index;
		m_pFileBuffer->removepointer(reinterpret_cast<void**>(&pvbmmesh));
	}

	// Delete conversion groups
	if(!m_pConversionGroups.empty())
	{
		for(Uint32 i = 0; i < m_pConversionGroups.size(); i++)
			delete m_pConversionGroups[i];

		m_pConversionGroups.clear();
	}

	// Delete triangles
	if(!m_conversionTrianglesArray.empty())
		m_conversionTrianglesArray.clear();

	m_numConversionTriangles = 0;
	m_numProcessed = 0;

	// Remove ptr
	m_pFileBuffer->removepointer(reinterpret_cast<void**>(&pvbmmeshes));
	return true;
}

//===============================================
// @brief Processes a studiomodel vertex into it's final form
//
// @param pvert Pointer to vertex to be processed
// @param pgroup Triangle conversion group to process
// @return TRUE if successful, FALSE if error occurred
//===============================================
bool CMDLVBMConverter::ProcessVertex( vbmsubmodel_t* pvbmsubmodel, const studiovert_t *pvert, const mesh_group_t* pgroup )
{
	// Allocate new elements if needed
	if(m_numIndexes == m_vbmIndexesArray.size())
		m_vbmIndexesArray.resize(m_vbmIndexesArray.size() + VBM_INDEXES_ALLOC_SIZE);

	// See if matching vertex already exists
	Uint32 i = m_currentStartVertex;
	for(; i < m_numRefVerts; i++)
	{
		studiovert_t* pstvert = &m_referenceVertexArray[i];

		if(pvert->normindex == pstvert->normindex 
			&& pvert->vertindex == pstvert->vertindex
			&& pvert->texcoord[0] == pstvert->texcoord[0] 
			&& pvert->texcoord[1] == pstvert->texcoord[1]
			&& pvert->boneindex == pstvert->boneindex)
		{
			m_vbmIndexesArray[m_numIndexes] = i;
			m_numIndexes++;
			break;
		}
	}

	if(i != m_numRefVerts)
		return true;

	// If not, add a new vertex
	m_vbmIndexesArray[m_numIndexes] = m_numRefVerts;
	m_numIndexes++;

	if(m_numRefVerts == m_referenceVertexArray.size())
		m_referenceVertexArray.resize(m_referenceVertexArray.size() + VBM_VERTEXES_ALLOC_SIZE);

	studiovert_t* pstvert = &m_referenceVertexArray[m_numRefVerts];
	m_numRefVerts++;

	pstvert->vertindex = pvert->vertindex;
	pstvert->normindex = pvert->normindex;
	pstvert->texcoord[0] = pvert->texcoord[0];
	pstvert->texcoord[1] = pvert->texcoord[1];

	// Allocate new elements if needed
	if(m_numVBMVerts == m_vbmVertexesArray.size())
		m_vbmVertexesArray.resize(m_vbmVertexesArray.size() + VBM_VERTEXES_ALLOC_SIZE);

	vbmvertex_t* pvbmvertex = &m_vbmVertexesArray[m_numVBMVerts];
	m_numVBMVerts++;

	pvbmvertex->origin = m_pStudioVertexes[pvert->vertindex];
	pvbmvertex->normal = m_pStudioNormals[pvert->normindex];
	pvbmvertex->texcoord[0] = (Float)pvert->texcoord[0]*1.0f/pgroup->ptexture->width;
	pvbmvertex->texcoord[1] = (Float)pvert->texcoord[1]*1.0f/pgroup->ptexture->height;

	const mesh_group_t *pgrp;
	if(!pgroup->numbones)
		pgrp = m_pConversionGroups[pgroup->bonegrp];
	else
		pgrp = pgroup;

	i = 0;
	for(; i < pgrp->numbones; i++)
	{
		if(pgrp->bones[i] == pvert->boneindex)
		{
			pvbmvertex->boneindexes[0] = i*3;
			pvbmvertex->boneweights[0] = 255;
			break;
		}
	}

	if(i == pgrp->numbones)
	{
		ErrorMsg("Couldn't find bone index '%d' for mesh with texture '%s' in submodel '%s'.\n", pvert->boneindex, pgroup->ptexture->name, pvbmsubmodel->name);
		return false;
	}
	else
	{
		return true;
	}
}

//===============================================
// @brief Check if we can merge group bones into one
//
//===============================================
void CMDLVBMConverter::CheckMergeGroups( void )
{
	if(m_pConversionGroups.size() == 1)
		return;

	mesh_group_t** ppgrp1 = &m_pConversionGroups[0];
	mesh_group_t* pgrp1 = *ppgrp1;

	Uint32 i = 0;
	for(Uint32 j = 1; j < m_pConversionGroups.size(); j++)
	{
		mesh_group_t** ppgrp2 = &m_pConversionGroups[j];
		mesh_group_t* pgrp2 = *ppgrp2;

		// Check if the last mesh shares this mesh's bones
		Uint32 l = 0;
		for(; l < pgrp2->numbones; l++)
		{
			Uint32 m = 0;
			for(; m < pgrp1->numbones; m++)
			{
				if(pgrp1->bones[m] == pgrp2->bones[l])
					break;
			}

			if(m == pgrp1->numbones)
				break;
		}

		if(l == pgrp2->numbones)
		{
			pgrp2->numbones = 0;
			pgrp2->bonegrp = i;
			continue;
		}

		// Now check if this mesh has the last mesh's bones
		for(l = 0; l < pgrp1->numbones; l++)
		{
			Uint32 m = 0;
			for(; m < pgrp2->numbones; m++)
			{
				if(pgrp2->bones[m] == pgrp1->bones[l])
					break;
			}

			if(m == pgrp2->numbones)
				break;
		}

		// If successful, swap the meshes
		if(l == pgrp1->numbones)
		{
			mesh_group_t* ptmp = pgrp2;
			(*ppgrp2) = (*ppgrp1);
			(*ppgrp1) = ptmp;

			(*ppgrp2)->numbones = 0;
			(*ppgrp2)->bonegrp = i;
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
				if(pgrp1->bones[m] == pgrp2->bones[l])
					break;
			}

			if(m == pgrp1->numbones)
			{
				if( newbones.size() == MAX_SHADER_BONES )
					break;

				newbones.push_back(pgrp2->bones[l]);
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
			pgrp1->bones[pgrp1->numbones] = newbones[l];
			pgrp1->numbones++;
		}

		pgrp2->numbones = 0;
		pgrp2->bonegrp = i;
	}
}

//===============================================
// @brief Calculate tangent information for the mesh data
//
//===============================================
void CMDLVBMConverter::CalculateTangents( void )
{
	// Calculate tangents
	Vector *s_tangents = new Vector[m_numVBMVerts];
	memset(s_tangents, 0, sizeof(Vector)*m_numVBMVerts);

	Vector *t_tangents = new Vector[m_numVBMVerts];
	memset(t_tangents, 0, sizeof(Vector)*m_numVBMVerts);

	for(Int32 i = 0; i < m_pVBMHeader->numbodyparts; i++)
	{
		vbmbodypart_t* pbodypart = m_pVBMHeader->getBodyPart(i);

		for(Int32 j = 0; j < pbodypart->numsubmodels; j++)
		{
			vbmsubmodel_t* psubmodel = pbodypart->getSubmodel(m_pVBMHeader, j);

			for(Int32 k = 0; k < psubmodel->nummeshes; k++)
			{
				vbmmesh_t *pmesh = psubmodel->getMesh(m_pVBMHeader, k);

				for(Int32 l = 0; l < pmesh->num_indexes; l += 3)
				{
					Int32 index1 = m_vbmIndexesArray[pmesh->start_index+l];
					Int32 index2 = m_vbmIndexesArray[pmesh->start_index+l+1];
					Int32 index3 = m_vbmIndexesArray[pmesh->start_index+l+2];

					vbmvertex_t *v0 = &m_vbmVertexesArray[index1];
					vbmvertex_t *v1 = &m_vbmVertexesArray[index2];
					vbmvertex_t *v2 = &m_vbmVertexesArray[index3];

					Float x1 = v1->origin.x - v0->origin.x;
					Float x2 = v2->origin.x - v0->origin.x;
					Float y1 = v1->origin.y - v0->origin.y;
					Float y2 = v2->origin.y - v0->origin.y;
					Float z1 = v1->origin.z - v0->origin.z;
					Float z2 = v2->origin.z - v0->origin.z;

					Float s1 = v1->texcoord[0] - v0->texcoord[0];
					Float s2 = v2->texcoord[0] - v0->texcoord[0];
					Float t1 = v1->texcoord[1] - v0->texcoord[1];
					Float t2 = v2->texcoord[1] - v0->texcoord[1];

					Float div = (s1 * t2 - s2 * t1);
					Float r = div == 0.0f ? 0.0F : 1.0F/div;

					Vector sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
					Vector tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);

					s_tangents[index1] = s_tangents[index1] + sdir;
					s_tangents[index2] = s_tangents[index2] + sdir;
					s_tangents[index3] = s_tangents[index3] + sdir;

					t_tangents[index1] = t_tangents[index1] + tdir;
					t_tangents[index2] = t_tangents[index2] + tdir;
					t_tangents[index3] = t_tangents[index3] + tdir;
				}
			}
		}
	}

	// Save tangents as well
	for(Int32 i = 0; i < m_numVBMVerts; i++)
	{
		vbmvertex_t& vertex = m_vbmVertexesArray[i];
		Vector tangent = (s_tangents[i] - vertex.normal * Math::DotProduct(vertex.normal, s_tangents[i])).Normalize();
		Math::VectorCopy(tangent, vertex.tangent);

		Vector vCross;
		Math::CrossProduct(vertex.normal, s_tangents[i], vCross);

		Float flDot = Math::DotProduct(vCross, t_tangents[i]);
		vertex.tangent[3] = sgn(flDot);
	}

	// Free data
	delete [] s_tangents;
	delete [] t_tangents;
}


