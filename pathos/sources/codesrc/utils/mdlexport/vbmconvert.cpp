/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.

===============================================
*/

//
// mdlexport.c: exports the textures of a .mdl file and creates .pmf entries
// models/<scriptname>.mdl.
//

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
#include "vbmconvert.h"
#include "com_math.h"
#include "main.h"

#define VBM_TEMP_HEADER_SIZE	1024*1024*32
#define MAX_VBM_VERTEXES		262144
#define MAX_VBM_INDEXES			262144

vbmheader_t*	g_pVBMHeader = nullptr;
Uint32			g_iVBMHeaderSize = 0;

studiohdr_t*	g_pStudioHeader = nullptr;
studiohdr_t*	g_pTextureHeader = nullptr;

mstudiomodel_t* g_pSubModels = nullptr;

Vector*			g_pStudioVertexes = nullptr;
Vector*			g_pStudioNormals = nullptr;

vbmvertex_t		g_pVBOVerts[MAX_VBM_VERTEXES];
int				g_iNumVBOVerts = 0;

unsigned int	g_usIndexes[MAX_VBM_INDEXES];
int				g_iNumIndexes = 0;

mesh_group_t	*g_pConvGroups = nullptr;
int				g_iNumConvGroups = 0;
mesh_group_t	*g_pCurGroup = nullptr;

studiotri_t		*g_pConvTriangles = nullptr;
int				g_iNumConvTris = 0;
int				g_iNumProcessed = 0;

// Array holding the bodypart's vertexes
studiovert_t	g_pRefArray[MAX_VBM_VERTEXES];
int				g_iNumRefVerts = 0;

int				g_iCurStart = 0;

/*
====================
VBM_ConvertModel

====================
*/
void VBM_ConvertModel( const Char* pstrModelFilePath, const Char* pstrTextureModelFilePath, const Char* pstrOutputPath )
{
	byte* pmodelfiledata = nullptr;
	byte* ptexturefiledata = nullptr;

	// Load model file
	SDL_RWops* pf = SDL_RWFromFile(pstrModelFilePath, "rb");
	if(!pf)
	{
		printf("Failed to open %s for reading: %s.\n", pstrModelFilePath, SDL_GetError());
		SDL_ClearError();
		return;
	}

	SDL_RWseek(pf, 0, RW_SEEK_END);
	Int32 size = (Int32)SDL_RWtell(pf);
	SDL_RWseek(pf, 0, RW_SEEK_SET);

	pmodelfiledata = new byte[size+1];
	size_t numbytes = SDL_RWread(pf, pmodelfiledata, 1, size);
	SDL_RWclose(pf);

	g_pStudioHeader = (studiohdr_t*)pmodelfiledata;
	g_pTextureHeader = nullptr;

	if(qstrcmp(pstrModelFilePath, pstrTextureModelFilePath))
	{
		pf = SDL_RWFromFile(pstrTextureModelFilePath, "rb");
		if(!pf)
		{
			printf("Failed to open %s for reading: %s.\n", pstrTextureModelFilePath, SDL_GetError());
			SDL_ClearError();
			return;
		}

		SDL_RWseek(pf, 0, RW_SEEK_END);
		size = (Int32)SDL_RWtell(pf);
		SDL_RWseek(pf, 0, RW_SEEK_SET);

		ptexturefiledata = new byte[size+1];
		numbytes = SDL_RWread(pf, pmodelfiledata, 1, size);
		SDL_RWclose(pf);

		g_pTextureHeader = (studiohdr_t*)ptexturefiledata;
	}
	else
	{
		g_pTextureHeader = g_pStudioHeader;
	}


	Uint32 numsubmodels = 0;
	for (Int32 i = 0; i < g_pStudioHeader->numbodyparts; i++)
	{
		const mstudiobodyparts_t* pbodypart = g_pStudioHeader->getBodyPart(i);
		numsubmodels += pbodypart->nummodels;
	}
	if (numsubmodels == 0)
		return;

	g_pVBMHeader = (vbmheader_t *)new byte[VBM_TEMP_HEADER_SIZE];
	memset(g_pVBMHeader, 0, sizeof(byte)*VBM_TEMP_HEADER_SIZE);
	g_iVBMHeaderSize = sizeof(vbmheader_t);

	// Set basic stuff
	g_pVBMHeader->id = VBM_HEADER;
	strcpy_s(g_pVBMHeader->name, pstrModelFilePath);

	vbmsubmodel_t* psubmodels = (vbmsubmodel_t *)((byte *)g_pVBMHeader+g_iVBMHeaderSize);
	Uint32 submodeloffset = g_iVBMHeaderSize;
	g_iVBMHeaderSize += sizeof(vbmsubmodel_t)*numsubmodels;

	// Set up bodyparts	
	g_pVBMHeader->numbodyparts = g_pStudioHeader->numbodyparts;
	g_pVBMHeader->bodypartoffset = g_iVBMHeaderSize;
	g_iVBMHeaderSize += sizeof(vbmbodypart_t)*g_pVBMHeader->numbodyparts;

	Uint32 offset = 0;
	for(Int32 i = 0; i < g_pStudioHeader->numbodyparts; i++)
	{
		const mstudiobodyparts_t* pbodypart = g_pStudioHeader->getBodyPart(i);
		vbmbodypart_t* pvbmbodypart = (vbmbodypart_t*)((byte*)g_pVBMHeader + g_pVBMHeader->bodypartoffset) + i;

		CString str(pbodypart->name);
		if(str.length() > 32)
			str.erase(31, str.length()-31);

		pvbmbodypart->base = pbodypart->base;
		qstrcpy(pvbmbodypart->name, str.c_str());
		pvbmbodypart->numsubmodels = pbodypart->nummodels;

		pvbmbodypart->submodeloffset = submodeloffset + sizeof(vbmsubmodel_t)*offset;
		offset += pbodypart->nummodels;
	}

	// Set up skin families
	g_pVBMHeader->numskinfamilies = g_pTextureHeader->numskinfamilies;
	g_pVBMHeader->numskinref = g_pTextureHeader->numskinref;
	g_pVBMHeader->skinoffset = g_iVBMHeaderSize;
	g_iVBMHeaderSize += sizeof(Int16)*(g_pVBMHeader->numskinfamilies*g_pVBMHeader->numskinref);

	Int16* pvbmskinref = (Int16*)((byte*)g_pVBMHeader + g_pVBMHeader->skinoffset);
	Int16* pskinref = (Int16*)((byte*)g_pTextureHeader + g_pTextureHeader->skinindex);

	// Copy skinfamilies
	memcpy(pvbmskinref, pskinref, sizeof(Int16)*(g_pVBMHeader->numskinfamilies*g_pVBMHeader->numskinref));

	// Copy textures
	g_pVBMHeader->numtextures = g_pTextureHeader->numtextures;
	g_pVBMHeader->textureoffset = g_iVBMHeaderSize;
	g_iVBMHeaderSize += sizeof(vbmtexture_t)*g_pTextureHeader->numtextures;

	vbmtexture_t* pvbmtextures = (vbmtexture_t*)((byte*)g_pVBMHeader + g_pVBMHeader->textureoffset);
	for(Int32 i = 0; i < g_pVBMHeader->numtextures; i++)
	{
		const mstudiotexture_t* ptexture = g_pTextureHeader->getTexture(i);
		vbmtexture_t* pvbmtexture = &pvbmtextures[i];

		pvbmtexture->flags = ptexture->flags;
		pvbmtexture->height = ptexture->height;
		pvbmtexture->width = ptexture->width;
		
		CString texturename(ptexture->name);
		Uint32 ofs = 0;
		while(true)
		{
			ofs = texturename.find(ofs, " ");
			if(ofs == -1)
				break;

			texturename.erase(ofs, 1);
			texturename.insert(ofs, "_");
		}

		qstrcpy(pvbmtexture->name, texturename.c_str());
	}

	// Set bones
	g_pVBMHeader->numboneinfo = g_pStudioHeader->numbones;
	g_pVBMHeader->boneinfooffset = g_iVBMHeaderSize;
	g_iVBMHeaderSize += sizeof(vbmboneinfo_t)*g_pVBMHeader->numboneinfo;

	vbmboneinfo_t* pvbmbones = (vbmboneinfo_t*)((byte*)g_pVBMHeader + g_pVBMHeader->boneinfooffset);
	for(Int32 i = 0; i < g_pStudioHeader->numbones; i++)
	{
		const mstudiobone_t* pbone = g_pStudioHeader->getBone(i);
		
		for(Uint32 j = 0; j < 3; j++)
			pvbmbones[i].position[j] = pbone->value[j];

		for(Uint32 j = 0; j < 3; j++)
			pvbmbones[i].angles[j] = pbone->value[3+j];

		 pvbmbones[i].index = i;
		 qstrcpy(pvbmbones[i].name, pbone->name);
		 pvbmbones[i].parentindex = pbone->parent;
		 pvbmbones[i].flags = pbone->flags;

		 for(Uint32 j = 0; j < 6; j++)
			 pvbmbones[i].scale[j] = pbone->scale[j];

		 for(Uint32 j = 0; j < 3; j++)
			pvbmbones[i].bindtransform[j][j] = 1.0;
	}

	offset = 0;
	for (Int32 i = 0 ; i < g_pStudioHeader->numbodyparts; i++)
	{
		const mstudiobodyparts_t* pbodypart = g_pStudioHeader->getBodyPart(i);
		g_pSubModels = (mstudiomodel_t *)((byte *)g_pStudioHeader + pbodypart->modelindex);

		for (Int32 k = 0; k < pbodypart->nummodels; k++)
		{
			// Get VBM submodel
			vbmsubmodel_t *pvbmsubmodel = &psubmodels[offset]; 
			offset++;

			pvbmsubmodel->flexinfoindex = -1;

			CString str(g_pSubModels[k].name);
			if(str.length() > 32)
				str.erase(31, str.length()-31);

			qstrcpy(pvbmsubmodel->name, str.c_str());

			mstudiomesh_t *psmeshes = (mstudiomesh_t *)((byte *)g_pStudioHeader + g_pSubModels[k].meshindex);

			g_pStudioNormals = (Vector *)((byte *)g_pStudioHeader + g_pSubModels[k].normindex);
			g_pStudioVertexes = (Vector *)((byte *)g_pStudioHeader + g_pSubModels[k].vertindex);

			Uint32 inumtris = 0;
			for (Int32 l = 0; l < g_pSubModels[k].nummesh; l++) 
				inumtris += psmeshes[l].numtris;

			if(!inumtris)
				continue;

			// allocate triangles
			g_pConvTriangles = new studiotri_t[inumtris];
			memset(g_pConvTriangles, 0, sizeof(studiotri_t)*inumtris);

			// Non-chrome first
			for (Int32 l = 0; l < g_pSubModels[k].nummesh; l++) 
			{
				Int32 textureindex = pskinref[psmeshes[l].skinref];
				const mstudiotexture_t* ptexture = g_pTextureHeader->getTexture(textureindex);

				if(ptexture->flags & STUDIO_NF_CHROME)
					continue;

				VBM_SplitMesh(&psmeshes[l], &g_pSubModels[k], ptexture);
			}

			// Now add chromed ones
			for (Int32 l = 0; l < g_pSubModels[k].nummesh; l++) 
			{
				Int32 textureindex = pskinref[psmeshes[l].skinref];
				const mstudiotexture_t* ptexture = g_pTextureHeader->getTexture(textureindex);

				if(!(ptexture->flags & STUDIO_NF_CHROME))
					continue;

				VBM_SplitMesh(&psmeshes[l], &g_pSubModels[k], ptexture);
			}

			// Merge anything you can
			VBM_TryMergeGroups();
			
			// Process current meshes
			VBM_ProcessConvGroups(pvbmsubmodel);
		}
	}

	// Calculate tangents
	VBM_CalculateTangents();

	// Convert VBO data to the optimized format
	vbmvertex_t *pwriteverts = (vbmvertex_t *)((byte *)g_pVBMHeader+g_iVBMHeaderSize);
	g_pVBMHeader->vertexoffset = g_iVBMHeaderSize;
	g_pVBMHeader->numverts = g_iNumVBOVerts;
	g_iVBMHeaderSize += sizeof(vbmvertex_t)*g_iNumVBOVerts;

	for(Int32 i = 0; i < g_iNumVBOVerts; i++)
	{
		Math::VectorCopy(g_pVBOVerts[i].origin, pwriteverts[i].origin);
		Math::VectorCopy(g_pVBOVerts[i].normal, pwriteverts[i].normal);
		pwriteverts[i].texcoord[0] = g_pVBOVerts[i].texcoord[0];
		pwriteverts[i].texcoord[1] = g_pVBOVerts[i].texcoord[1];

		for(Uint32 j = 0; j < MAX_VBM_BONEWEIGHTS; j++)
		{
			pwriteverts[i].boneindexes[j] = g_pVBOVerts[i].boneindexes[j];
			pwriteverts[i].boneweights[j] = g_pVBOVerts[i].boneweights[j];
		}
	}

	//Copy over index array
	Uint32 *pindexes = (Uint32 *)((byte *)g_pVBMHeader+g_iVBMHeaderSize);
	memcpy(pindexes, &g_usIndexes, sizeof(Uint32)*g_iNumIndexes);
	g_pVBMHeader->indexoffset = g_iVBMHeaderSize;
	g_iVBMHeaderSize += sizeof(Uint32)*g_iNumIndexes;

	g_pVBMHeader->numverts = g_iNumVBOVerts;
	g_pVBMHeader->numindexes = g_iNumIndexes;
	g_pVBMHeader->size = g_iVBMHeaderSize;

	g_iNumRefVerts = 0;
	g_iNumVBOVerts = 0;
	g_iNumIndexes = 0;

	//
	// Write the data out
	//
	vbmheader_t *pHeader = (vbmheader_t *)new byte[g_iVBMHeaderSize];
	memcpy(pHeader, g_pVBMHeader, sizeof(byte)*g_iVBMHeaderSize);

	delete [] g_pVBMHeader;
	g_pVBMHeader = NULL;

	// Create base path
	CString outputPathBase;
	outputPathBase << pstrOutputPath << "/models/";

	if(CreateDirectory(outputPathBase.c_str()))
	{
		// Get basename
		CString basename;
		Common::Basename(pstrModelFilePath, basename);
		basename << ".vbm";

		CString outputPath;
		outputPath << outputPathBase << basename;
		FILE *pFile = fopen(outputPath.c_str(), "wb");
	
		if(!pFile)
			return;

		fwrite(pHeader, sizeof(byte)*pHeader->size, 1, pFile);
		fclose(pFile);
	}

	delete[] g_pVBMHeader;
}

/*
====================
VBM_SplitMesh

====================
*/
void VBM_SplitMesh( mstudiomesh_t *pmesh, mstudiomodel_t *psub, const mstudiotexture_t *ptex )
{
	if(!pmesh->numtris)
		return;

	Int32 j = 0;
	byte *pvertbone = ((byte *)g_pStudioHeader + psub->vertinfoindex);
	Int16 *ptricmds = (Int16 *)((byte *)g_pStudioHeader + pmesh->triindex);
	while (j = *(ptricmds++))
	{	
		if (j > 0) 
		{
			// convert triangle strip
			j -= 3;
			studiovert_t indices[3];
			for(Int32 i = 0; i < 3; i++, ptricmds += 4)
			{
				indices[i].vertindex = ptricmds[0];
				indices[i].normindex = ptricmds[1];
				indices[i].texcoord[0] = ptricmds[2];
				indices[i].texcoord[1] = ptricmds[3];
				indices[i].boneindex = pvertbone[ptricmds[0]];

				memcpy(&g_pConvTriangles[g_iNumConvTris].verts[i], &indices[i], sizeof(studiovert_t));
			}
			g_iNumConvTris++;

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

				if (!reverse)
				{
					memcpy(&g_pConvTriangles[g_iNumConvTris].verts[0], &indices[2], sizeof(studiovert_t));
					memcpy(&g_pConvTriangles[g_iNumConvTris].verts[1], &indices[1], sizeof(studiovert_t));
					memcpy(&g_pConvTriangles[g_iNumConvTris].verts[2], &indices[0], sizeof(studiovert_t));
					g_iNumConvTris++;
				}
				else
				{
					memcpy(&g_pConvTriangles[g_iNumConvTris].verts[0], &indices[0], sizeof(studiovert_t));
					memcpy(&g_pConvTriangles[g_iNumConvTris].verts[1], &indices[1], sizeof(studiovert_t));
					memcpy(&g_pConvTriangles[g_iNumConvTris].verts[2], &indices[2], sizeof(studiovert_t));
					g_iNumConvTris++;
				}
				reverse = !reverse;
			}
		}
		else
		{
			// convert triangle fan
			j = -j-3;
			studiovert_t indices[3];
			for(Int32 i = 0; i < 3; i++, ptricmds += 4)
			{
				indices[i].vertindex = ptricmds[0];
				indices[i].normindex = ptricmds[1];
				indices[i].texcoord[0] = ptricmds[2];
				indices[i].texcoord[1] = ptricmds[3];
				indices[i].boneindex = pvertbone[ptricmds[0]];

				memcpy(&g_pConvTriangles[g_iNumConvTris].verts[i], &indices[i], sizeof(studiovert_t));
			}
			g_iNumConvTris++;

			for( ; j > 0; j--, ptricmds += 4)
			{
				indices[1] = indices[2];
				indices[2].vertindex = ptricmds[0]; 
				indices[2].normindex = ptricmds[1];
				indices[2].texcoord[0] = ptricmds[2];
				indices[2].texcoord[1] = ptricmds[3];
				indices[2].boneindex = pvertbone[ptricmds[0]];

				memcpy(&g_pConvTriangles[g_iNumConvTris].verts[0], &indices[0], sizeof(studiovert_t));
				memcpy(&g_pConvTriangles[g_iNumConvTris].verts[1], &indices[1], sizeof(studiovert_t));
				memcpy(&g_pConvTriangles[g_iNumConvTris].verts[2], &indices[2], sizeof(studiovert_t));
				g_iNumConvTris++;
			}
		}
	}

	Int32 icurnum = g_iNumConvGroups;
	g_pConvGroups = (mesh_group_t *)Common::ResizeArray(g_pConvGroups, sizeof(mesh_group_t), g_iNumConvGroups);
	g_pCurGroup = &g_pConvGroups[g_iNumConvGroups]; g_iNumConvGroups++; 

	// process and recurse through all bones
	mstudiobone_t *pbones = (mstudiobone_t *)((byte *)g_pStudioHeader + g_pStudioHeader->boneindex);
	for(Int32 i = 0; i < g_pStudioHeader->numbones; i++)
	{
		if(pbones[i].parent != -1)
			continue;

		VBM_RecurseBone(i, &pbones[i]);
	}

	// fill in tex pointers
	for(Int32 i = icurnum; i < g_iNumConvGroups; i++)
	{
		g_pConvGroups[i].skinref = pmesh->skinref;
		g_pConvGroups[i].ptex = ptex;
	}
}

/*
====================
VBM_ProcessConvGroups

====================
*/
void VBM_ProcessConvGroups( vbmsubmodel_t* pvbosubmodel )
{
	vbmmesh_t *pmeshes = (vbmmesh_t *)((byte *)g_pVBMHeader+g_iVBMHeaderSize);
	pvbosubmodel->meshoffset = g_iVBMHeaderSize; 
	g_iVBMHeaderSize += sizeof(vbmmesh_t)*g_iNumConvGroups;
	pvbosubmodel->nummeshes = g_iNumConvGroups;

	// do shit here
	for(Int32 i = 0; i < pvbosubmodel->nummeshes; i++)
	{
		pmeshes[i].start_index = g_iNumIndexes;
		pmeshes[i].skinref = g_pConvGroups[i].skinref;

		if(g_pConvGroups[i].numbones)
		{
			byte *pbones = ((byte *)g_pVBMHeader+g_iVBMHeaderSize);
			pmeshes[i].boneoffset = g_iVBMHeaderSize;
			g_iCurStart = g_iNumRefVerts;

			memcpy(pbones, g_pConvGroups[i].bones, sizeof(byte)*g_pConvGroups[i].numbones);
			g_iVBMHeaderSize += sizeof(byte)*g_pConvGroups[i].numbones; pmeshes[i].numbones = g_pConvGroups[i].numbones;
		}

		for(Int32 j = 0; j < g_pConvGroups[i].numtris; j++)
		{
			for(Int32 l = 0; l < 3; l++)
			{
				studiovert_t *pvert = &g_pConvGroups[i].triangles[j].verts[l];
				VBM_ProcessVertex(pvert, &g_pConvGroups[i]);
			}
		}

		// set end
		pmeshes[i].num_indexes = g_iNumIndexes-pmeshes[i].start_index;
	}

	for(Int32 i = 0; i < g_iNumConvGroups; i++)
	{
		if(g_pConvGroups[i].triangles)
			delete[] g_pConvGroups[i].triangles;
	}

	delete [] g_pConvGroups;
	g_pConvGroups = nullptr;
	g_iNumConvGroups = 0;
			
	delete [] g_pConvTriangles;
	g_pConvTriangles = nullptr;
	g_iNumConvTris = 0;
}
/*
====================
VBM_TryMergeGroups

====================
*/
void VBM_TryMergeGroups( void )
{
	if(g_iNumConvGroups <= 1)
		return;

	Int32 ilast = 0;
	mesh_group_t *pgrp1 = g_pConvGroups;
	for(Int32 j = 1; j < g_iNumConvGroups; j++)
	{
		mesh_group_t *pgrp2 = &g_pConvGroups[j];
		if((pgrp2->ptex->flags & STUDIO_NF_CHROME) && !(pgrp1->ptex->flags & STUDIO_NF_CHROME)
			||!(pgrp2->ptex->flags & STUDIO_NF_CHROME) && (pgrp1->ptex->flags & STUDIO_NF_CHROME))
		{
			pgrp1 = pgrp2; 
			ilast = j;
			continue;
		}

		// optimize lists
		Int32 l = 0;
		for(; l < pgrp2->numbones; l++)
		{
			Int32 m = 0;
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
			pgrp2->bonegrp = ilast;
			continue;
		}

		// try reverse-checking
		l = 0;
		for(; l < pgrp1->numbones; l++)
		{
			Int32 m = 0;
			for(; m < pgrp2->numbones; m++)
			{
				if(pgrp2->bones[m] == pgrp1->bones[l])
					break;
			}

			if(m == pgrp2->numbones)
				break;
		}

		if(l == pgrp1->numbones)
		{
			mesh_group_t save;
			memcpy(&save, pgrp2, sizeof(mesh_group_t));
			memcpy(pgrp2, pgrp1, sizeof(mesh_group_t));
			memcpy(pgrp1, &save, sizeof(mesh_group_t));
			pgrp2->numbones = 0;
			pgrp2->bonegrp = ilast;
		}

		Int32 numdiff = 0;
		for(int l = 0; l < pgrp2->numbones; l++)
		{
			Int32 m = 0;
			for(; m < pgrp1->numbones; m++)
			{
				if(pgrp1->bones[m] == pgrp2->bones[l])
					break;
			}

			if(m == pgrp1->numbones)
				numdiff++;
		}

		// see if we can merge
		if((pgrp1->numbones+numdiff) > MAX_SHADER_BONES)
		{
			pgrp1 = pgrp2; 
			ilast = j;
			continue;
		}

		for(Int32 l = 0; l < pgrp2->numbones; l++)
		{
			Int32 m = 0;
			for(; m < pgrp1->numbones; m++)
			{
				if(pgrp1->bones[m] == pgrp2->bones[l])
					break;
			}

			if(m ==  pgrp1->numbones)
			{
				pgrp1->bones[pgrp1->numbones] = pgrp2->bones[l];
				pgrp1->numbones++;
			}
		}

		pgrp2->numbones = 0;
		pgrp2->bonegrp = ilast;
	}
}

/*
====================
VBM_ProcessVertex

====================
*/
void VBM_ProcessVertex( studiovert_t *pvert, mesh_group_t* pgroup )
{
	Int32 n = g_iCurStart;
	for(; n < g_iNumRefVerts; n++)
	{
		studiovert_t* pstvert = &g_pRefArray[n];

		if(pvert->normindex == pstvert->normindex 
		&& pvert->vertindex == pstvert->vertindex
		&& pvert->texcoord[0] == pstvert->texcoord[0] 
		&& pvert->texcoord[1] == pstvert->texcoord[1])
		{
			g_usIndexes[g_iNumIndexes] = n;
			g_iNumIndexes++;
			break;
		}
	}

	if(n != g_iNumRefVerts)
		return;

	g_usIndexes[g_iNumIndexes] = g_iNumRefVerts;
	g_iNumIndexes++;

	studiovert_t* pstvert = &g_pRefArray[g_iNumRefVerts];
	g_iNumRefVerts++;

	pstvert->vertindex = pvert->vertindex;
	pstvert->normindex = pvert->normindex;
	pstvert->texcoord[0] = pvert->texcoord[0];
	pstvert->texcoord[1] = pvert->texcoord[1];

	vbmvertex_t* pvbmvertex = &g_pVBOVerts[g_iNumVBOVerts];
	g_iNumVBOVerts++;

	pvbmvertex->origin = g_pStudioVertexes[pvert->vertindex];
	pvbmvertex->normal = g_pStudioNormals[pvert->normindex];
	pvbmvertex->texcoord[0] = (Float)pvert->texcoord[0]*1.0f/pgroup->ptex->width;
	pvbmvertex->texcoord[1] = (Float)pvert->texcoord[1]*1.0f/pgroup->ptex->height;

	if(!pgroup->numbones)
	{
		mesh_group_t *pgrp = &g_pConvGroups[pgroup->bonegrp];
		for(Int32 n = 0; n < pgrp->numbones; n++)
		{
			if(pgrp->bones[n] == pvert->boneindex)
			{
				pvbmvertex->boneindexes[0] = n*3;
				pvbmvertex->boneweights[0] = 255;
				break;
			}
		}
	}
	else
	{
							
		for(Int32 n = 0; n < pgroup->numbones; n++)
		{
			if(pgroup->bones[n] == pvert->boneindex)
			{
				pvbmvertex->boneindexes[0] = n*3;
				pvbmvertex->boneweights[0] = 255;
				break;
			}
		}
	}
}
/*
====================
VBM_CalculateTangents

====================
*/
void VBM_CalculateTangents( void )
{
	// Calculate tangents
	Vector *s_tangents = new Vector[g_iNumVBOVerts];
	memset(s_tangents, 0, sizeof(Vector)*g_iNumVBOVerts);

	Vector *t_tangents = new Vector[g_iNumVBOVerts];
	memset(t_tangents, 0, sizeof(Vector)*g_iNumVBOVerts);

	for(Int32 i = 0; i < g_pVBMHeader->numbodyparts; i++)
	{
		vbmbodypart_t* pbodypart = g_pVBMHeader->getBodyPart(i);

		for(Int32 j = 0; j < pbodypart->numsubmodels; j++)
		{
			vbmsubmodel_t* psubmodel = pbodypart->getSubmodel(g_pVBMHeader, j);

			for(Int32 k = 0; k < psubmodel->nummeshes; k++)
			{
				vbmmesh_t *pmesh = (vbmmesh_t *)((byte *)g_pVBMHeader + psubmodel->meshoffset)+k;

				for(Int32 l = 0; l < pmesh->num_indexes; l += 3)
				{
					vbmvertex_t *v0 = g_pVBOVerts+g_usIndexes[pmesh->start_index+l];
					vbmvertex_t *v1 = g_pVBOVerts+g_usIndexes[pmesh->start_index+l+1];
					vbmvertex_t *v2 = g_pVBOVerts+g_usIndexes[pmesh->start_index+l+2];

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

					s_tangents[g_usIndexes[pmesh->start_index+l]] = s_tangents[g_usIndexes[pmesh->start_index+l]] + sdir;
					s_tangents[g_usIndexes[pmesh->start_index+l+1]] = s_tangents[g_usIndexes[pmesh->start_index+l+1]] + sdir;
					s_tangents[g_usIndexes[pmesh->start_index+l+2]] = s_tangents[g_usIndexes[pmesh->start_index+l+2]] + sdir;

					t_tangents[g_usIndexes[pmesh->start_index+l]] = t_tangents[g_usIndexes[pmesh->start_index+l]] + tdir;
					t_tangents[g_usIndexes[pmesh->start_index+l+1]] = t_tangents[g_usIndexes[pmesh->start_index+l+1]] + tdir;
					t_tangents[g_usIndexes[pmesh->start_index+l+2]] = t_tangents[g_usIndexes[pmesh->start_index+l+2]] + tdir;
				}
			}
		}
	}

	// Save tangents as well
	for(Int32 i = 0; i < g_iNumVBOVerts; i++)
	{
		Vector tangent = (s_tangents[i] - g_pVBOVerts[i].normal * Math::DotProduct(g_pVBOVerts[i].normal, s_tangents[i])).Normalize();
		Math::VectorCopy(tangent, g_pVBOVerts[i].tangent);

		Vector vCross;
		Math::CrossProduct(g_pVBOVerts[i].normal, s_tangents[i], vCross);

		Float flDot = Math::DotProduct(vCross, t_tangents[i]);
		g_pVBOVerts[i].tangent[3] = sgn(flDot);
	}

	// Free data
	delete [] s_tangents;
	delete [] t_tangents;
}

/*
====================
VBM_RecurseBone

====================
*/
void VBM_RecurseBone( Int32 index, mstudiobone_t *pbone )
{
	for(Int32 i = 0; i < g_iNumConvTris; i++)
	{
		if(g_pConvTriangles[i].processed)
			continue;

		if(g_pConvTriangles[i].verts[0].boneindex != index
			&& g_pConvTriangles[i].verts[1].boneindex != index
			&& g_pConvTriangles[i].verts[2].boneindex != index)
			continue;

		if(g_pCurGroup->numbones)
		{
			Int32 numadd = 0;
			for(Int32 j = 0; j < 3; j++)
			{
				Int32 k = 0; 
				for(; k < g_pCurGroup->numbones; k++)
				{
					if(g_pConvTriangles[i].verts[j].boneindex == g_pCurGroup->bones[k])
						break;
				}

				if(k == g_pCurGroup->numbones)
					numadd++;
			}

			if((g_pCurGroup->numbones+numadd) > MAX_SHADER_BONES)
			{
				g_pConvGroups = (mesh_group_t *)Common::ResizeArray(g_pConvGroups, sizeof(mesh_group_t), g_iNumConvGroups);
				g_pCurGroup = &g_pConvGroups[g_iNumConvGroups]; g_iNumConvGroups++;
			}
		}

		for(Int32 j = 0; j < 3; j++)
		{
			Int32 k = 0; 
			for(; k < g_pCurGroup->numbones; k++)
			{
				if(g_pConvTriangles[i].verts[j].boneindex == g_pCurGroup->bones[k])
					break;
			}

			if(k == g_pCurGroup->numbones)
			{
				g_pCurGroup->bones[g_pCurGroup->numbones] = g_pConvTriangles[i].verts[j].boneindex;
				g_pCurGroup->numbones++;
			}
		}

		g_pCurGroup->triangles = (studiotri_t *)Common::ResizeArray(g_pCurGroup->triangles, sizeof(studiotri_t), g_pCurGroup->numtris);
		memcpy(&g_pCurGroup->triangles[g_pCurGroup->numtris], &g_pConvTriangles[i], sizeof(studiotri_t)); 
		g_pConvTriangles[i].processed = true; g_pCurGroup->numtris++;
	}

	mstudiobone_t *pbones = (mstudiobone_t *)((byte *)g_pStudioHeader + g_pStudioHeader->boneindex);
	for(int i = 0; i < g_pStudioHeader->numbones; i++)
	{
		if(pbones[i].parent != index || i == index)
			continue;

		VBM_RecurseBone(i, &pbones[i]);
	}
}

