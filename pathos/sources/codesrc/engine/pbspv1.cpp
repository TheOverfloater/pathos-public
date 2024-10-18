/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "com_math.h"
#include "file.h"
#include "texturemanager.h"
#include "brushmodel.h"
#include "pbspv1file.h"
#include "pbspv1.h"
#include "system.h"
#include "logfile.h"
#include "enginestate.h"
#include "bsp_shared.h"

//=============================================
// @brief
//
//=============================================
brushmodel_t* PBSPV1_Load( const byte* pfile, const dpbspv1header_t* pheader, const Char* pstrFilename )
{
	// Create the brushmodel_t object
	brushmodel_t* pmodel = new brushmodel_t();
	
	// Set the version info
	pmodel->name = pstrFilename;
	pmodel->version = pheader->version;
	pmodel->freedata = true;

	// Load the lumps
	if(!PBSPV1_LoadVertexes(pfile, (*pmodel), pheader->lumps[PBSPV1_LUMP_VERTEXES])
		|| !PBSPV1_LoadEdges(pfile, (*pmodel), pheader->lumps[PBSPV1_LUMP_EDGES])
		|| !PBSPV1_LoadSurfedges(pfile, (*pmodel), pheader->lumps[PBSPV1_LUMP_SURFEDGES])
		|| !PBSPV1_LoadTextures(pfile, (*pmodel), pheader->lumps[PBSPV1_LUMP_TEXTURES])
		|| !PBSPV1_LoadLighting(pfile, (*pmodel), pheader->lumps[PBSPV1_LUMP_LIGHTING])
		|| !PBSPV1_LoadTexinfo(pfile, (*pmodel), pheader->lumps[PBSPV1_LUMP_TEXINFO])
		|| !PBSPV1_LoadPlanes(pfile, (*pmodel), pheader->lumps[PBSPV1_LUMP_PLANES])
		|| !PBSPV1_LoadFaces(pfile, (*pmodel), pheader->lumps[PBSPV1_LUMP_FACES])
		|| !PBSPV1_LoadMarksurfaces(pfile, (*pmodel), pheader->lumps[PBSPV1_LUMP_MARKSURFACES])
		|| !PBSPV1_LoadVisibility(pfile, (*pmodel), pheader->lumps[PBSPV1_LUMP_VISIBILITY])
		|| !PBSPV1_LoadLeafs(pfile, (*pmodel), pheader->lumps[PBSPV1_LUMP_LEAFS])
		|| !PBSPV1_LoadNodes(pfile, (*pmodel), pheader->lumps[PBSPV1_LUMP_NODES])
		|| !PBSPV1_LoadClipnodes(pfile, (*pmodel), pheader->lumps[PBSPV1_LUMP_CLIPNODES])
		|| !PBSPV1_LoadEntities(pfile, (*pmodel), pheader->lumps[PBSPV1_LUMP_ENTITIES])
		|| !PBSPV1_LoadSubmodels(pfile, (*pmodel), pheader->lumps[PBSPV1_LUMP_MODELS]))
	{
		delete pmodel;
		return nullptr;
	}

	// Set up everything else
	BSP_MakeHullZero((*pmodel));

	return pmodel;
}

//=============================================
// @brief
//
//=============================================
bool PBSPV1_LoadVertexes( const byte* pfile, brushmodel_t& model, const dpbspv1lump_t& lump )
{
	// Check if sizes are correct
	if(lump.size % sizeof(dpbspv1vertex_t))
	{
		Con_EPrintf("%s - Inconsistent lump size in '%s'.\n", __FUNCTION__, model.name.c_str());
		return false;
	}

	// Load the data in
	Uint32 count = lump.size/sizeof(dpbspv1vertex_t);
	const dpbspv1vertex_t* pinverts = reinterpret_cast<const dpbspv1vertex_t*>(pfile + lump.offset);
	mvertex_t* poutverts = new mvertex_t[count];

	model.pvertexes = poutverts;
	model.numvertexes = count;

	for(Uint32 i = 0; i < count; i++)
		Math::VectorCopy(pinverts[i].origin, poutverts[i].origin);

	return true;
}

//=============================================
// @brief
//
//=============================================
bool PBSPV1_LoadEdges( const byte* pfile, brushmodel_t& model, const dpbspv1lump_t& lump )
{
	// Check if sizes are correct
	if(lump.size % sizeof(dpbspv1edge_t))
	{
		Con_EPrintf("%s - Inconsistent lump size in '%s'.\n", __FUNCTION__, model.name.c_str());
		return false;
	}

	// Load the data in
	Uint32 count = lump.size/sizeof(dpbspv1edge_t);
	const dpbspv1edge_t* pinedges = reinterpret_cast<const dpbspv1edge_t*>(pfile + lump.offset);
	medge_t* poutedges = new medge_t[count];

	model.pedges = poutedges;
	model.numedges = count;

	for(Uint32 i = 0; i < count; i++)
	{
		poutedges[i].vertexes[0] = pinedges[i].vertexes[0];
		poutedges[i].vertexes[1] = pinedges[i].vertexes[1];
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
bool PBSPV1_LoadSurfedges( const byte* pfile, brushmodel_t& model, const dpbspv1lump_t& lump )
{
	// Check if sizes are correct
	if(lump.size % sizeof(Int32))
	{
		Con_EPrintf("%s - Inconsistent lump size in '%s'.\n", __FUNCTION__, model.name.c_str());
		return false;
	}

	// Load the data in
	Uint32 count = lump.size/sizeof(Int32);
	const Int32* pinedges = reinterpret_cast<const Int32*>(pfile + lump.offset);
	Int32* poutedges = new Int32[count];

	model.psurfedges = poutedges;
	model.numsurfedges = count;

	memcpy(poutedges, pinedges, sizeof(Int32)*count);
	return true;
}

//=============================================
// @brief
//
//=============================================
bool PBSPV1_LoadTextures( const byte* pfile, brushmodel_t& model, const dpbspv1lump_t& lump )
{
	if(!lump.size)
	{
		Con_EPrintf("%s - No textures present in '%s'.\n", __FUNCTION__, model.name.c_str());
		return true;
	}

	// Get texture counts
	const dmiptexlump_t* pmiptexlump = reinterpret_cast<const dmiptexlump_t*>(pfile + lump.offset);

	model.numtextures = pmiptexlump->nummiptex;
	model.ptextures = new mtexture_t[model.numtextures];

	for(Uint32 i = 0; i < model.numtextures; i++)
	{
		// Get pointer to miptex data
		const dmiptex_t* pmiptex = reinterpret_cast<const dmiptex_t*>(reinterpret_cast<const byte*>(pmiptexlump) + pmiptexlump->dataoffsets[i]);

		// We only get the name and width/height here
		mtexture_t* ptexture = &model.ptextures[i];
		ptexture->name = pmiptex->name;
		ptexture->width = pmiptex->width;
		ptexture->height = pmiptex->height;
	}

	// Handle animated textures
	for(Uint32 i = 0; i < model.numtextures; i++)
	{
		mtexture_t* ptexture = &model.ptextures[i];
		if(ptexture->name[0] != '+' && ptexture->name[0] != '-')
			continue;

		if(ptexture->panim_next)
			continue;

		mtexture_t* panims[MAX_TEXTURE_ANIMS];
		memset(panims, 0, sizeof(mtexture_t*)*MAX_TEXTURE_ANIMS);

		mtexture_t* paltanims[MAX_TEXTURE_ANIMS];
		memset(paltanims, 0, sizeof(mtexture_t*)*MAX_TEXTURE_ANIMS);

		// Check the letter
		Int32 max = ptexture->name[1];
		Int32 altmax = 0;

		if(max >= 'a' && max <= 'z')
			max -= 32;

		if(max < '0' || max > '9')
		{
			if(max < 'A' || max > 'J')
			{
				Con_Printf("Bad animating texture '%s'.\n", ptexture->name.c_str());
				continue;
			}

			altmax = max - 'A';
			max = 0;

			paltanims[altmax] = ptexture;
			altmax++;
		}
		else
		{
			max -= '0';
			altmax = 0;
			panims[max] = ptexture;
			max++;
		}

		// Check ahead
		for(Uint32 j = i+1; j < model.numtextures; j++)
		{
			mtexture_t* pnext = &model.ptextures[j];
			if(!pnext)
				continue;

			if(pnext->name[0] != '+' && pnext->name[0] != '-')
				continue;

			// Only check ones with the same name
			if(qstrcmp(ptexture->name.c_str()+2, pnext->name.c_str()+2))
				continue;

			Int32 num = pnext->name[1];
			if(num >= 'a' && num <= 'z')
				num -= 'a' - 'A';

			if(num < '0' || num > '9')
			{
				if(num < 'A' || num > 'J')
				{
					Con_Printf("Bad animating texture '%s'.\n", ptexture->name.c_str());
					continue;
				}

				num -= 'A';
				paltanims[num] = pnext;
				if((num+1) > altmax)
					altmax = num + 1;
			}
			else
			{
				num -= '0';
				panims[num] = pnext;
				if((num+1) > max)
					max = num+1;
			}
		}

		// Link the animating textures
		for(Int32 j = 0; j < max; j++)
		{
			mtexture_t* pnext = panims[j];
			if(!pnext)
			{
				Con_EPrintf("Missing frame %d animating texture '%s'.\n", j, ptexture->name.c_str());
				continue;
			}

			pnext->anim_min = j;
			pnext->anim_total = max;
			pnext->anim_max = j + 1;
			pnext->panim_next = panims[(j+1)%max];

			if(altmax)
				pnext->palt_anims = paltanims[0];
		}

		// Link alt anims too
		for(Int32 j = 0; j < altmax; j++)
		{
			mtexture_t* pnext = paltanims[j];
			if(!pnext)
			{
				Con_EPrintf("Missing frame %d animating texture '%s'.\n", j, ptexture->name.c_str());
				continue;
			}

			pnext->anim_min = j;
			pnext->anim_total = altmax;
			pnext->anim_max = j + 1;
			pnext->panim_next = paltanims[(j+1)%altmax];

			if(max)
				pnext->palt_anims = panims[0];
		}
	}
	
	return true;
}

//=============================================
// @brief
//
//=============================================
bool PBSPV1_LoadLighting( const byte* pfile, brushmodel_t& model, const dpbspv1lump_t& lump )
{
	if(!lump.size)
		return true;

	// Check if sizes are correct
	if(lump.size % sizeof(color24_t))
	{
		Con_EPrintf("%s - Inconsistent lump size in '%s'.\n", __FUNCTION__, model.name.c_str());
		return false;
	}

	model.pbaselightdata[SURF_LIGHTMAP_DEFAULT] = reinterpret_cast<color24_t *>(new byte[lump.size]);
	model.lightdatasize = lump.size;

	const byte *psrc = (pfile + lump.offset);
	memcpy(model.pbaselightdata[SURF_LIGHTMAP_DEFAULT], psrc, sizeof(byte)*lump.size);

	model.plightdata[SURF_LIGHTMAP_DEFAULT] = model.pbaselightdata[SURF_LIGHTMAP_DEFAULT];

	return true;
}

//=============================================
// @brief
//
//=============================================
bool PBSPV1_LoadPlanes( const byte* pfile, brushmodel_t& model, const dpbspv1lump_t& lump )
{
	// Check if sizes are correct
	if(lump.size % sizeof(dpbspv1plane_t))
	{
		Con_EPrintf("%s - Inconsistent lump size in '%s'.\n", __FUNCTION__, model.name.c_str());
		return false;
	}

	// Load the data in
	Uint32 count = lump.size/sizeof(dpbspv1plane_t);
	const dpbspv1plane_t* pinplanes = reinterpret_cast<const dpbspv1plane_t*>(pfile + lump.offset);
	plane_t* poutplanes = new plane_t[count];

	model.pplanes = poutplanes;
	model.numplanes = count;

	for(Uint32 i = 0; i < count; i++)
	{
		Uint32 bits = 0;
		for(Uint32 j = 0; j < 3; j++)
		{
			poutplanes[i].normal[j] = pinplanes[i].normal[j];
			if(poutplanes[i].normal[j] < 0)
				bits |= 1 << j;
		}

		poutplanes[i].dist = pinplanes[i].dist;
		poutplanes[i].type = static_cast<planetype_t>(pinplanes[i].type);
		poutplanes[i].signbits = bits;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
bool PBSPV1_LoadTexinfo( const byte* pfile, brushmodel_t& model, const dpbspv1lump_t& lump )
{
	// Check if sizes are correct
	if(lump.size % sizeof(dpbspv1texinfo_t))
	{
		Con_EPrintf("%s - Inconsistent lump size in '%s'.\n", __FUNCTION__, model.name.c_str());
		return false;
	}

	// Load the data in
	Uint32 count = lump.size/sizeof(dpbspv1texinfo_t);
	const dpbspv1texinfo_t* pintexinfos = reinterpret_cast<const dpbspv1texinfo_t*>(pfile + lump.offset);
	mtexinfo_t* pouttexinfos = new mtexinfo_t[count];
	
	model.ptexinfos = pouttexinfos;
	model.numtexinfos = count;

	for(Uint32 i = 0; i < count; i++)
	{
		// Copy the alignment info
		memcpy(pouttexinfos[i].vecs, pintexinfos[i].vecs, sizeof(Float)*8);

		pouttexinfos[i].flags = pintexinfos[i].flags;
		Int32 textureindex = pintexinfos[i].miptex;

		if(textureindex >= static_cast<Int32>(model.numtextures))
		{
			Con_EPrintf("Invalid texture index '%d' in '%s'.\n", textureindex, model.name.c_str());
			continue;
		}

		pouttexinfos[i].ptexture = &model.ptextures[textureindex];
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
bool PBSPV1_LoadFaces( const byte* pfile, brushmodel_t& model, const dpbspv1lump_t& lump )
{
	// Check if sizes are correct
	if(lump.size % sizeof(dpbspv1face_t))
	{
		Con_EPrintf("%s - Inconsistent lump size in '%s'.\n", __FUNCTION__, model.name.c_str());
		return false;
	}

	// Load the data in
	Uint32 count = lump.size/sizeof(dpbspv1face_t);
	const dpbspv1face_t* pinfaces = reinterpret_cast<const dpbspv1face_t*>(pfile + lump.offset);
	msurface_t* poutsurfaces = new msurface_t[count];
	
	model.psurfaces = poutsurfaces;
	model.numsurfaces = count;

	// If we need to re-organize lighting data
	byte* plightdata[NB_SURF_LIGHTMAP_LAYERS] = { nullptr };
	Uint32 lightdatasize = 0;

	bool hasBumpMapData = false;
	for(Uint32 i = 0; i < count; i++)
	{
		Uint32 j = 0;
		for(; j < MAX_SURFACE_STYLES; j++)
		{
			if(pinfaces[i].lmstyles[j] == PBSPV1_LM_AMBIENT_STYLE
				|| pinfaces[i].lmstyles[j] == PBSPV1_LM_DIFFUSE_STYLE
				|| pinfaces[i].lmstyles[j] == PBSPV1_LM_LIGHTVECS_STYLE)
			{
				break;
			}
		}

		if(j != MAX_SURFACE_STYLES)
		{
			hasBumpMapData = true;
			break;
		}
	}

	// If we found bump data, allocate the lightmaps
	if(hasBumpMapData)
	{
		for(Uint32 j = 0; j < NB_SURF_LIGHTMAP_LAYERS; j++)
		{
			plightdata[j] = new byte[model.lightdatasize];
			memset(plightdata[j], 0, sizeof(model.lightdatasize));
		}
	}

	// Check if we have any bump map data
	for(Uint32 i = 0; i < count; i++)
	{
		msurface_t* pout = &poutsurfaces[i];

		pout->firstedge = pinfaces[i].firstedge;
		pout->numedges = pinfaces[i].numedges;
		pout->flags = 0;

		Uint32 planeindex = pinfaces[i].planenum;
		Int32 side = pinfaces[i].side;
		if(side)
			pout->flags |= SURF_PLANEBACK;

		pout->pplane = &model.pplanes[planeindex];
		
		Int32 texinfoindex = pinfaces[i].texinfo;
		pout->ptexinfo = &model.ptexinfos[texinfoindex];

		pout->lightmapdivider = PBSPV1_LM_SAMPLE_SIZE;
		if(!BSP_CalcSurfaceExtents(pout, model))
			return false;

		for(Uint32 j = 0; j < MAX_SURFACE_STYLES; j++)
			pout->styles[j] = pinfaces[i].lmstyles[j];
		
		if(pinfaces[i].lightoffset != -1)
		{
			Int32 ambientIndex = Mod_StyleIndex(pout, PBSPV1_LM_AMBIENT_STYLE);
			Int32 diffuseIndex = Mod_StyleIndex(pout, PBSPV1_LM_DIFFUSE_STYLE);
			Int32 vectorsIndex = Mod_StyleIndex(pout, PBSPV1_LM_LIGHTVECS_STYLE);
		
			// Manage reorganization of lighting data into what the engine needs
			if(hasBumpMapData && ambientIndex != NO_POSITION && diffuseIndex != NO_POSITION && vectorsIndex != NO_POSITION)
			{
				// Calculate extents
				Uint32 xsize = (pout->extents[0] / pout->lightmapdivider)+1;
				Uint32 ysize = (pout->extents[1] / pout->lightmapdivider)+1;
				Uint32 size = xsize*ysize;

				// Set up indexes
				Int32 layerIndexes[NB_SURF_LIGHTMAP_LAYERS] = { 
					0,				// SURF_LIGHTMAP_DEFAULT
					vectorsIndex,	 // SURF_LIGHTMAP_VECTORS
					ambientIndex,	 // SURF_LIGHTMAP_AMBIENT
					diffuseIndex,	 // SURF_LIGHTMAP_DIFFUSE
				};

				for(Uint32 j = 0; j < NB_SURF_LIGHTMAP_LAYERS; j++)
				{
					// Set final data offset and copy the data
					byte* pdestination = plightdata[j] + lightdatasize;
					color24_t* psrclightdata = reinterpret_cast<color24_t*>(reinterpret_cast<byte*>(model.plightdata[SURF_LIGHTMAP_DEFAULT]) + pinfaces[i].lightoffset) + size * layerIndexes[j];
					memcpy(pdestination, psrclightdata, sizeof(color24_t)*size);
				}

				// Set light data offset and increase size
				pout->lightoffset = lightdatasize;
				lightdatasize += size*sizeof(color24_t);
			}
			else
			{
				// We only have the base layer
				pout->psamples[SURF_LIGHTMAP_DEFAULT] = reinterpret_cast<color24_t*>(reinterpret_cast<byte*>(model.plightdata[SURF_LIGHTMAP_DEFAULT]) + pinfaces[i].lightoffset);
				pout->lightoffset = pinfaces[i].lightoffset;
			}

			// If any of these were set, clear the styles entirely past position 0
			if(ambientIndex != NO_POSITION || diffuseIndex != NO_POSITION || vectorsIndex != NO_POSITION)
			{
				for(Uint32 j = 1; j < MAX_SURFACE_STYLES; j++)
					pout->styles[j] = NULL_LIGHTSTYLE_INDEX;
			}
		}
		else
		{
			// No light data at all
			pout->lightoffset = -1;
		}

		// Flag sky surfaces
		if(!qstrncmp(pout->ptexinfo->ptexture->name.c_str(), "sky", 3))
			pout->flags |= SURF_DRAWSKY;
	}

	// Re-organize light data if needed
	if(hasBumpMapData)
	{
		// Delete original lightdata
		if(model.plightdata[SURF_LIGHTMAP_DEFAULT])
			delete[] model.plightdata[SURF_LIGHTMAP_DEFAULT];

		// Resize the lighting data to it's final size
		for(Uint32 i = 0; i < NB_SURF_LIGHTMAP_LAYERS; i++)
		{
			byte* pfinaldata = new byte[lightdatasize];
			memcpy(pfinaldata, plightdata[i], sizeof(byte)*lightdatasize);

			// Set pointers and data sizes
			model.pbaselightdata[i] = reinterpret_cast<color24_t*>(pfinaldata);
			model.plightdata[i] = model.pbaselightdata[i];

			// Delete temporary array we made
			delete[] plightdata[i];
		}

		// Set final size
		model.lightdatasize = lightdatasize;

		// Now modify the data ptrs
		for(Uint32 i = 0; i < model.numsurfaces; i++)
		{
			msurface_t* psurface = &model.psurfaces[i];
			if(psurface->lightoffset == -1)
				continue;

			for(Uint32 j = 0; j < NB_SURF_LIGHTMAP_LAYERS; j++)
				psurface->psamples[j] = reinterpret_cast<color24_t*>(reinterpret_cast<byte*>(model.plightdata[j]) + psurface->lightoffset);
		}
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
bool PBSPV1_LoadMarksurfaces( const byte* pfile, brushmodel_t& model, const dpbspv1lump_t& lump )
{
	// Check if sizes are correct
	if(lump.size % sizeof(Int32))
	{
		Con_EPrintf("%s - Inconsistent lump size in '%s'.\n", __FUNCTION__, model.name.c_str());
		return false;
	}

	// Load the data in
	Uint32 count = lump.size/sizeof(Int32);
	const Uint32* pinmarksurfaces = reinterpret_cast<const Uint32*>(pfile + lump.offset);
	msurface_t** poutmarksurfaces = new msurface_t*[count];

	model.pmarksurfaces = poutmarksurfaces;
	model.nummarksurfaces = count;

	for(Uint32 i = 0; i < count; i++)
	{
		Uint32 surfindex = pinmarksurfaces[i];
		if(surfindex >= count)
		{
			Con_EPrintf("PBSPV1_LoadFaces - Bad surface index.\n");
			return false;
		}

		poutmarksurfaces[i] = &model.psurfaces[surfindex];
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
bool PBSPV1_LoadVisibility( const byte* pfile, brushmodel_t& model, const dpbspv1lump_t& lump )
{
	if(!lump.size)
		return true;

	model.pvisdata = new byte[lump.size];
	memset(model.pvisdata, 0, lump.size);
	model.visdatasize = lump.size;

	memcpy(model.pvisdata, pfile + lump.offset, lump.size);

	return true;
}

//=============================================
// @brief
//
//=============================================
bool PBSPV1_LoadLeafs( const byte* pfile, brushmodel_t& model, const dpbspv1lump_t& lump )
{
	// Check if sizes are correct
	if(lump.size % sizeof(dpbspv1leaf_t))
	{
		Con_EPrintf("%s - Inconsistent lump size in '%s'.\n", __FUNCTION__, model.name.c_str());
		return false;
	}

	// Load the data in
	Uint32 count = lump.size/sizeof(dpbspv1leaf_t);
	const dpbspv1leaf_t* pinleafs = reinterpret_cast<const dpbspv1leaf_t*>(pfile + lump.offset);
	mleaf_t* poutleafs = new mleaf_t[count];

	model.pleafs = poutleafs;
	model.numleafs = count;

	for(Uint32 i = 0; i < count; i++)
	{
		mleaf_t* pout = &poutleafs[i];

		for(Uint32 j = 0; j < 3; j++)
		{
			pout->mins[j] = Common::ByteToInt16(reinterpret_cast<const byte*>(&pinleafs[i].mins[j]));
			pout->maxs[j] = Common::ByteToInt16(reinterpret_cast<const byte*>(&pinleafs[i].maxs[j]));
		}

		pout->contents = pinleafs[i].contents;

		Uint32 marksurfindex = pinleafs[i].firstmarksurface;
		pout->pfirstmarksurface = &model.pmarksurfaces[marksurfindex];
		pout->nummarksurfaces = pinleafs[i].nummarksurfaces;

		if(pinleafs[i].visoffset != -1)
			pout->pcompressedvis = model.pvisdata + pinleafs[i].visoffset;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
bool PBSPV1_LoadNodes( const byte* pfile, brushmodel_t& model, const dpbspv1lump_t& lump )
{
	// Check if sizes are correct
	if(lump.size % sizeof(dpbspv1node_t))
	{
		Con_EPrintf("%s - Inconsistent lump size in '%s'.\n", __FUNCTION__, model.name.c_str());
		return false;
	}

	// Load the data in
	Uint32 count = lump.size/sizeof(dpbspv1node_t);
	const dpbspv1node_t* pinnodes = reinterpret_cast<const dpbspv1node_t*>(pfile + lump.offset);
	mnode_t* poutnodes = new mnode_t[count];

	model.pnodes = poutnodes;
	model.numnodes = count;

	for(Uint32 i = 0; i < count; i++)
	{
		mnode_t* pout = &poutnodes[i];

		for(Uint32 j = 0; j < 3; j++)
		{
			pout->mins[j] = Common::ByteToInt16(reinterpret_cast<const byte*>(&pinnodes[i].mins[j]));
			pout->maxs[j] = Common::ByteToInt16(reinterpret_cast<const byte*>(&pinnodes[i].maxs[j]));
		}

		pout->pplane = &model.pplanes[pinnodes[i].planenum];

		pout->firstsurface = pinnodes[i].firstface;
		pout->numsurfaces = pinnodes[i].numfaces;

		for(Uint32 j = 0; j < 2; j++)
		{
			Int32 nodeidx = pinnodes[i].children[j];

			if(nodeidx >= 0)
				pout->pchildren[j] = &model.pnodes[nodeidx];
			else
				pout->pchildren[j] = reinterpret_cast<mnode_t*>(&model.pleafs[-1-nodeidx]);
		}
	}

	// Set linkage info on nodes
	BSP_SetNodeParent(model.pnodes, nullptr);
	return true;
}

//=============================================
// @brief
//
//=============================================
bool PBSPV1_LoadClipnodes( const byte* pfile, brushmodel_t& model, const dpbspv1lump_t& lump )
{
	// Check if sizes are correct
	if(lump.size % sizeof(dpbspv1clipnode_t))
	{
		Con_EPrintf("%s - Inconsistent lump size in '%s'.\n", __FUNCTION__, model.name.c_str());
		return false;
	}

	// Load the data in
	Uint32 count = lump.size/sizeof(dpbspv1clipnode_t);
	const dpbspv1clipnode_t* pinnodes = reinterpret_cast<const dpbspv1clipnode_t*>(pfile + lump.offset);
	mclipnode_t* poutnodes = new mclipnode_t[count];

	model.pclipnodes = poutnodes;
	model.numclipnodes = count;

	// Set hull 1
	hull_t* phull = &model.hulls[1];
	phull->pclipnodes = poutnodes;
	phull->firstclipnode = 0;
	phull->lastclipnode = count-1;
	phull->pplanes = model.pplanes;
	
	phull->clipmins[0] = -16;
	phull->clipmins[1] = -16;
	phull->clipmins[2] = -36;
	phull->clipmaxs[0] = 16;
	phull->clipmaxs[1] = 16;
	phull->clipmaxs[2] = 36;

	// Set hull 2
	phull = &model.hulls[2];
	phull->pclipnodes = poutnodes;
	phull->firstclipnode = 0;
	phull->lastclipnode = count-1;
	phull->pplanes = model.pplanes;
	
	phull->clipmins[0] = -32;
	phull->clipmins[1] = -32;
	phull->clipmins[2] = -32;
	phull->clipmaxs[0] = 32;
	phull->clipmaxs[1] = 32;
	phull->clipmaxs[2] = 32;

	// Set hull 3
	phull = &model.hulls[3];
	phull->pclipnodes = poutnodes;
	phull->firstclipnode = 0;
	phull->lastclipnode = count-1;
	phull->pplanes = model.pplanes;
	
	phull->clipmins[0] = -16;
	phull->clipmins[1] = -16;
	phull->clipmins[2] = -18;
	phull->clipmaxs[0] = 16;
	phull->clipmaxs[1] = 16;
	phull->clipmaxs[2] = 18;

	for(Uint32 i = 0; i < count; i++)
	{
		poutnodes[i].planenum = pinnodes[i].planenum;
		poutnodes[i].children[0] = pinnodes[i].children[0];
		poutnodes[i].children[1] = pinnodes[i].children[1];
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
bool PBSPV1_LoadEntities( const byte* pfile, brushmodel_t& model, const dpbspv1lump_t& lump )
{
	if(!lump.size)
		return true;

	model.pentdata = new Char[lump.size];

	const byte* pdata = pfile + lump.offset;
	memcpy(model.pentdata, pdata, sizeof(Char)*lump.size);
	
	return true;
}

//=============================================
// @brief
//
//=============================================
bool PBSPV1_LoadSubmodels( const byte* pfile, brushmodel_t& model, const dpbspv1lump_t& lump )
{
	// Check if sizes are correct
	if(lump.size % sizeof(dpbspv1model_t))
	{
		Con_EPrintf("%s - Inconsistent lump size in '%s'.\n", __FUNCTION__, model.name.c_str());
		return false;
	}

	// Load the data in
	Uint32 count = lump.size/sizeof(dpbspv1model_t);
	const dpbspv1model_t* pinmodels = reinterpret_cast<const dpbspv1model_t*>(pfile + lump.offset);
	mmodel_t* poutmodels = new mmodel_t[count];

	model.psubmodels = poutmodels;
	model.numsubmodels = count;

	for(Uint32 i = 0; i < count; i++)
	{
		for(Uint32 j = 0; j < 3; j++)
			poutmodels[i].mins[j] = pinmodels[i].mins[j] - 1;

		for(Uint32 j = 0; j < 3; j++)
			poutmodels[i].maxs[j] = pinmodels[i].maxs[j] - 1;

		for(Uint32 j = 0; j < 3; j++)
			poutmodels[i].origin[j] = pinmodels[i].origin[j];

		for(Uint32 j = 0; j < PBSPV1_MAX_MAP_HULLS; j++)
			poutmodels[i].headnode[j] = pinmodels[i].headnode[j];

		poutmodels[i].visleafs = pinmodels[i].visleafs;
		poutmodels[i].firstface = pinmodels[i].firstface;
		poutmodels[i].numfaces = pinmodels[i].numfaces;
	}

	return true;
}
