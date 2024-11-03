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
#include "pbspv2file.h"
#include "pbspv2.h"
#include "system.h"
#include "logfile.h"
#include "enginestate.h"
#include "bsp_shared.h"
#include "miniz.h"

//=============================================
// @brief
//
//=============================================
brushmodel_t* PBSPV2_Load( const byte* pfile, const dpbspv2header_t* pheader, const Char* pstrFilename )
{
	// Create the brushmodel_t object
	brushmodel_t* pmodel = new brushmodel_t();
	
	// Set the version info
	pmodel->name = pstrFilename;
	pmodel->version = pheader->version;
	pmodel->freedata = true;

	// Load the lumps
	if(!PBSPV2_LoadVertexes(pfile, (*pmodel), pheader->lumps[PBSPV2_LUMP_VERTEXES])
		|| !PBSPV2_LoadEdges(pfile, (*pmodel), pheader->lumps[PBSPV2_LUMP_EDGES])
		|| !PBSPV2_LoadSurfedges(pfile, (*pmodel), pheader->lumps[PBSPV2_LUMP_SURFEDGES])
		|| !PBSPV2_LoadTextures(pfile, (*pmodel), pheader->lumps[PBSPV2_LUMP_TEXTURES])
		|| !PBSPV2_LoadDefaultLighting(pfile, (*pmodel), pheader->lumps[PBSPV2_LUMP_LIGHTING_DEFAULT])
		|| !PBSPV2_LoadLightingDataLayer(pfile, (*pmodel), pheader->lumps[PBSPV2_LUMP_LIGHTING_AMBIENT], SURF_LIGHTMAP_AMBIENT)
		|| !PBSPV2_LoadLightingDataLayer(pfile, (*pmodel), pheader->lumps[PBSPV2_LUMP_LIGHTING_DIFFUSE], SURF_LIGHTMAP_DIFFUSE)
		|| !PBSPV2_LoadLightingDataLayer(pfile, (*pmodel), pheader->lumps[PBSPV2_LUMP_LIGHTING_VECTORS], SURF_LIGHTMAP_VECTORS)
		|| !PBSPV2_LoadTexinfo(pfile, (*pmodel), pheader->lumps[PBSPV2_LUMP_TEXINFO])
		|| !PBSPV2_LoadPlanes(pfile, (*pmodel), pheader->lumps[PBSPV2_LUMP_PLANES])
		|| !PBSPV2_LoadFaces(pfile, (*pmodel), pheader->lumps[PBSPV2_LUMP_FACES])
		|| !PBSPV2_LoadMarksurfaces(pfile, (*pmodel), pheader->lumps[PBSPV2_LUMP_MARKSURFACES])
		|| !PBSPV2_LoadVisibility(pfile, (*pmodel), pheader->lumps[PBSPV2_LUMP_VISIBILITY])
		|| !PBSPV2_LoadLeafs(pfile, (*pmodel), pheader->lumps[PBSPV2_LUMP_LEAFS])
		|| !PBSPV2_LoadNodes(pfile, (*pmodel), pheader->lumps[PBSPV2_LUMP_NODES])
		|| !PBSPV2_LoadClipnodes(pfile, (*pmodel), pheader->lumps[PBSPV2_LUMP_CLIPNODES])
		|| !PBSPV2_LoadEntities(pfile, (*pmodel), pheader->lumps[PBSPV2_LUMP_ENTITIES])
		|| !PBSPV2_LoadSubmodels(pfile, (*pmodel), pheader->lumps[PBSPV2_LUMP_MODELS]))
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
bool PBSPV2_LoadVertexes( const byte* pfile, brushmodel_t& model, const dpbspv2lump_t& lump )
{
	// Check if sizes are correct
	if(lump.size % sizeof(dpbspv2vertex_t))
	{
		Con_EPrintf("%s - Inconsistent lump size in '%s'.\n", __FUNCTION__, model.name.c_str());
		return false;
	}

	// Load the data in
	Uint32 count = lump.size/sizeof(dpbspv2vertex_t);
	const dpbspv2vertex_t* pinverts = reinterpret_cast<const dpbspv2vertex_t*>(pfile + lump.offset);
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
bool PBSPV2_LoadEdges( const byte* pfile, brushmodel_t& model, const dpbspv2lump_t& lump )
{
	// Check if sizes are correct
	if(lump.size % sizeof(dpbspv2edge_t))
	{
		Con_EPrintf("%s - Inconsistent lump size in '%s'.\n", __FUNCTION__, model.name.c_str());
		return false;
	}

	// Load the data in
	Uint32 count = lump.size/sizeof(dpbspv2edge_t);
	const dpbspv2edge_t* pinedges = reinterpret_cast<const dpbspv2edge_t*>(pfile + lump.offset);
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
bool PBSPV2_LoadSurfedges( const byte* pfile, brushmodel_t& model, const dpbspv2lump_t& lump )
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
bool PBSPV2_LoadTextures( const byte* pfile, brushmodel_t& model, const dpbspv2lump_t& lump )
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
bool PBSPV2_DecompressLightingData( const byte* pfile, brushmodel_t& model, const dpbspv2lump_t& lump, color24_t*& pdestptr, Uint32& destsize )
{
	const dpbspv2lmapdata_t* plightmapdata = reinterpret_cast<const dpbspv2lmapdata_t*>(pfile + lump.offset);
	if(plightmapdata->noncompressedsize % sizeof(color24_t))
	{
		Con_EPrintf("%s - Inconsistent decompressed data size in '%s'.\n", __FUNCTION__, model.name.c_str());
		return false;
	}

	const byte* prawdatasrc = pfile + plightmapdata->dataoffset;
	byte* poutputdataptr = new byte[plightmapdata->noncompressedsize];
	memset(poutputdataptr, 0, sizeof(byte)*plightmapdata->noncompressedsize);

	switch(plightmapdata->compression)
	{
	case PBSPV2_LMAP_COMPRESSION_NONE:
		memcpy(poutputdataptr, prawdatasrc, sizeof(byte)*plightmapdata->noncompressedsize);
		break;
	case PBSPV2_LMAP_COMPRESSION_MINIZ:
		{
			mz_ulong destinationsize = plightmapdata->noncompressedsize;
			Int32 status = uncompress(poutputdataptr, &destinationsize, prawdatasrc, plightmapdata->datasize);
			if(status != MZ_OK)
			{
				Con_EPrintf("%s - Miniz uncompress failed with error code %d.\n", __FUNCTION__, status);
				delete[] poutputdataptr;
				return false;
			}

			if(plightmapdata->noncompressedsize != destinationsize)
			{
				Con_EPrintf("%s - Miniz uncompress produced inconsistent output size (expected %d, got %d instead).\n", __FUNCTION__, plightmapdata->noncompressedsize, destinationsize);
				delete[] poutputdataptr;
				return false;
			}
		}
		break;
	}

	pdestptr = reinterpret_cast<color24_t*>(poutputdataptr);
	destsize = plightmapdata->noncompressedsize;

	return true;
}

//=============================================
// @brief
//
//=============================================
bool PBSPV2_LoadDefaultLighting( const byte* pfile, brushmodel_t& model, const dpbspv2lump_t& lump )
{
	if(!lump.size)
		return true;

	// Check if sizes are correct
	if(lump.size != sizeof(dpbspv2lmapdata_t))
	{
		Con_EPrintf("%s - Inconsistent lump size in '%s'.\n", __FUNCTION__, model.name.c_str());
		return false;
	}

	return PBSPV2_DecompressLightingData(pfile, model, lump, model.plightdata[SURF_LIGHTMAP_DEFAULT], model.lightdatasize);
}

//=============================================
// @brief
//
//=============================================
bool PBSPV2_LoadLightingDataLayer( const byte* pfile, brushmodel_t& model, const dpbspv2lump_t& lump, surf_lmap_layers_t layer )
{
	if(!lump.size)
		return true;

	// Check if sizes are correct
	if(lump.size != sizeof(dpbspv2lmapdata_t))
	{
		Con_EPrintf("%s - Inconsistent lump size in '%s'.\n", __FUNCTION__, model.name.c_str());
		return false;
	}

	Uint32 datasize = 0;
	bool result = PBSPV2_DecompressLightingData(pfile, model, lump, model.plightdata[layer], datasize);
	if(result)
	{
		if(datasize != model.lightdatasize)
		{
			Con_EPrintf("%s - Inconsistent lump size %d in '%s' for light data layer %d, expected size was %d.\n", __FUNCTION__, lump.size, model.name.c_str(), layer, model.lightdatasize);
			return false;
		}
	}

	return result;
}

//=============================================
// @brief
//
//=============================================
bool PBSPV2_LoadPlanes( const byte* pfile, brushmodel_t& model, const dpbspv2lump_t& lump )
{
	// Check if sizes are correct
	if(lump.size % sizeof(dpbspv2plane_t))
	{
		Con_EPrintf("%s - Inconsistent lump size in '%s'.\n", __FUNCTION__, model.name.c_str());
		return false;
	}

	// Load the data in
	Uint32 count = lump.size/sizeof(dpbspv2plane_t);
	const dpbspv2plane_t* pinplanes = reinterpret_cast<const dpbspv2plane_t*>(pfile + lump.offset);
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
bool PBSPV2_LoadTexinfo( const byte* pfile, brushmodel_t& model, const dpbspv2lump_t& lump )
{
	// Check if sizes are correct
	if(lump.size % sizeof(dpbspv2texinfo_t))
	{
		Con_EPrintf("%s - Inconsistent lump size in '%s'.\n", __FUNCTION__, model.name.c_str());
		return false;
	}

	// Load the data in
	Uint32 count = lump.size/sizeof(dpbspv2texinfo_t);
	const dpbspv2texinfo_t* pintexinfos = reinterpret_cast<const dpbspv2texinfo_t*>(pfile + lump.offset);
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
bool PBSPV2_LoadFaces( const byte* pfile, brushmodel_t& model, const dpbspv2lump_t& lump )
{
	// Check if sizes are correct
	if(lump.size % sizeof(dpbspv2face_t))
	{
		Con_EPrintf("%s - Inconsistent lump size in '%s'.\n", __FUNCTION__, model.name.c_str());
		return false;
	}

	// Load the data in
	Uint32 count = lump.size/sizeof(dpbspv2face_t);
	const dpbspv2face_t* pinfaces = reinterpret_cast<const dpbspv2face_t*>(pfile + lump.offset);
	msurface_t* poutsurfaces = new msurface_t[count];
	
	model.psurfaces = poutsurfaces;
	model.numsurfaces = count;

	// Check if we have any bump map data
	for(Uint32 i = 0; i < count; i++)
	{
		msurface_t* pout = &poutsurfaces[i];

		pout->firstedge = pinfaces[i].firstedge;
		pout->numedges = pinfaces[i].numedges;
		pout->flags = 0;
		pout->base_samplesize = PBSPV2_LM_SAMPLE_SIZE;

		Float sampleScale = pinfaces[i].samplescale;
		if(sampleScale <= 0)
			sampleScale = 1.0;

		pout->lightmapdivider = pout->base_samplesize / sampleScale;

		Uint32 planeindex = pinfaces[i].planenum;
		Int32 side = pinfaces[i].side;
		if(side)
			pout->flags |= SURF_PLANEBACK;

		pout->pplane = &model.pplanes[planeindex];
		
		Int32 texinfoindex = pinfaces[i].texinfo;
		pout->ptexinfo = &model.ptexinfos[texinfoindex];

		if(!BSP_CalcSurfaceExtents(pout, model))
			return false;

		for(Uint32 j = 0; j < MAX_SURFACE_STYLES; j++)
			pout->styles[j] = pinfaces[i].lmstyles[j];
		
		if(pinfaces[i].lightoffset != -1)
		{
			// We only have the base layer
			pout->psamples[SURF_LIGHTMAP_DEFAULT] = reinterpret_cast<color24_t*>(reinterpret_cast<byte*>(model.plightdata[SURF_LIGHTMAP_DEFAULT]) + pinfaces[i].lightoffset);
			pout->lightoffset = pinfaces[i].lightoffset;

			// Also set for other layers if we have valid data on them
			if(model.plightdata[SURF_LIGHTMAP_AMBIENT] 
				&& model.plightdata[SURF_LIGHTMAP_DIFFUSE] 
				&& model.plightdata[SURF_LIGHTMAP_VECTORS])
			{
				for(Uint32 j = 1; j < NB_SURF_LIGHTMAP_LAYERS; j++)
					pout->psamples[j] = reinterpret_cast<color24_t*>(reinterpret_cast<byte*>(model.plightdata[j]) + pout->lightoffset);
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

	return true;
}

//=============================================
// @brief
//
//=============================================
bool PBSPV2_LoadMarksurfaces( const byte* pfile, brushmodel_t& model, const dpbspv2lump_t& lump )
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
			Con_EPrintf("PBSPV2_LoadFaces - Bad surface index.\n");
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
bool PBSPV2_LoadVisibility( const byte* pfile, brushmodel_t& model, const dpbspv2lump_t& lump )
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
bool PBSPV2_LoadLeafs( const byte* pfile, brushmodel_t& model, const dpbspv2lump_t& lump )
{
	// Check if sizes are correct
	if(lump.size % sizeof(dpbspv2leaf_t))
	{
		Con_EPrintf("%s - Inconsistent lump size in '%s'.\n", __FUNCTION__, model.name.c_str());
		return false;
	}

	// Load the data in
	Uint32 count = lump.size/sizeof(dpbspv2leaf_t);
	const dpbspv2leaf_t* pinleafs = reinterpret_cast<const dpbspv2leaf_t*>(pfile + lump.offset);
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
bool PBSPV2_LoadNodes( const byte* pfile, brushmodel_t& model, const dpbspv2lump_t& lump )
{
	// Check if sizes are correct
	if(lump.size % sizeof(dpbspv2node_t))
	{
		Con_EPrintf("%s - Inconsistent lump size in '%s'.\n", __FUNCTION__, model.name.c_str());
		return false;
	}

	// Load the data in
	Uint32 count = lump.size/sizeof(dpbspv2node_t);
	const dpbspv2node_t* pinnodes = reinterpret_cast<const dpbspv2node_t*>(pfile + lump.offset);
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
bool PBSPV2_LoadClipnodes( const byte* pfile, brushmodel_t& model, const dpbspv2lump_t& lump )
{
	// Check if sizes are correct
	if(lump.size % sizeof(dpbspv2clipnode_t))
	{
		Con_EPrintf("%s - Inconsistent lump size in '%s'.\n", __FUNCTION__, model.name.c_str());
		return false;
	}

	// Load the data in
	Uint32 count = lump.size/sizeof(dpbspv2clipnode_t);
	const dpbspv2clipnode_t* pinnodes = reinterpret_cast<const dpbspv2clipnode_t*>(pfile + lump.offset);
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
bool PBSPV2_LoadEntities( const byte* pfile, brushmodel_t& model, const dpbspv2lump_t& lump )
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
bool PBSPV2_LoadSubmodels( const byte* pfile, brushmodel_t& model, const dpbspv2lump_t& lump )
{
	// Check if sizes are correct
	if(lump.size % sizeof(dpbspv2model_t))
	{
		Con_EPrintf("%s - Inconsistent lump size in '%s'.\n", __FUNCTION__, model.name.c_str());
		return false;
	}

	// Load the data in
	Uint32 count = lump.size/sizeof(dpbspv2model_t);
	const dpbspv2model_t* pinmodels = reinterpret_cast<const dpbspv2model_t*>(pfile + lump.offset);
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

		for(Uint32 j = 0; j < PBSPV2_MAX_MAP_HULLS; j++)
			poutmodels[i].headnode[j] = pinmodels[i].headnode[j];

		poutmodels[i].visleafs = pinmodels[i].visleafs;
		poutmodels[i].firstface = pinmodels[i].firstface;
		poutmodels[i].numfaces = pinmodels[i].numfaces;
	}

	return true;
}
