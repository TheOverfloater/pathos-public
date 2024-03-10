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
#include "bspv30.h"
#include "system.h"
#include "logfile.h"
#include "enginestate.h"
#include "miptex.h"

//=============================================
// @brief
//
//=============================================
brushmodel_t* BSPV30_Load( const byte* pfile, const dheader_t* pheader, const Char* pstrFilename )
{
	// Create the brushmodel_t object
	brushmodel_t* pmodel = new brushmodel_t();
	
	// Set the version info
	pmodel->name = pstrFilename;
	pmodel->version = pheader->version;
	pmodel->freedata = true;

	// Load the lumps
	if(!BSPV30_LoadVertexes(pfile, (*pmodel), pheader->lumps[LUMP_VERTEXES])
		|| !BSPV30_LoadEdges(pfile, (*pmodel), pheader->lumps[LUMP_EDGES])
		|| !BSPV30_LoadSurfedges(pfile, (*pmodel), pheader->lumps[LUMP_SURFEDGES])
		|| !BSPV30_LoadTextures(pfile, (*pmodel), pheader->lumps[LUMP_TEXTURES])
		|| !BSPV30_LoadLighting(pfile, (*pmodel), pheader->lumps[LUMP_LIGHTING])
		|| !BSPV30_LoadTexinfo(pfile, (*pmodel), pheader->lumps[LUMP_TEXINFO])
		|| !BSPV30_LoadPlanes(pfile, (*pmodel), pheader->lumps[LUMP_PLANES])
		|| !BSPV30_LoadFaces(pfile, (*pmodel), pheader->lumps[LUMP_FACES])
		|| !BSPV30_LoadMarksurfaces(pfile, (*pmodel), pheader->lumps[LUMP_MARKSURFACES])
		|| !BSPV30_LoadVisibility(pfile, (*pmodel), pheader->lumps[LUMP_VISIBILITY])
		|| !BSPV30_LoadLeafs(pfile, (*pmodel), pheader->lumps[LUMP_LEAFS])
		|| !BSPV30_LoadNodes(pfile, (*pmodel), pheader->lumps[LUMP_NODES])
		|| !BSPV30_LoadClipnodes(pfile, (*pmodel), pheader->lumps[LUMP_CLIPNODES])
		|| !BSPV30_LoadEntities(pfile, (*pmodel), pheader->lumps[LUMP_ENTITIES])
		|| !BSPV30_LoadSubmodels(pfile, (*pmodel), pheader->lumps[LUMP_MODELS]))
	{
		delete pmodel;
		return nullptr;
	}

	// Set up everything else
	BSPV30_MakeHullZero((*pmodel));

	return pmodel;
}

//=============================================
// @brief
//
//=============================================
bool BSPV30_LoadVertexes( const byte* pfile, brushmodel_t& model, const lump_t& lump )
{
	// Check if sizes are correct
	if(lump.size % sizeof(dv30vertex_t))
	{
		Con_EPrintf("%s - Inconsistent lump size in '%s'.\n", __FUNCTION__, model.name.c_str());
		return false;
	}

	// Load the data in
	Uint32 count = lump.size/sizeof(dv30vertex_t);
	const dv30vertex_t* pinverts = reinterpret_cast<const dv30vertex_t*>(pfile + lump.offset);
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
bool BSPV30_LoadEdges( const byte* pfile, brushmodel_t& model, const lump_t& lump )
{
	// Check if sizes are correct
	if(lump.size % sizeof(dv30edge_t))
	{
		Con_EPrintf("%s - Inconsistent lump size in '%s'.\n", __FUNCTION__, model.name.c_str());
		return false;
	}

	// Load the data in
	Uint32 count = lump.size/sizeof(dv30edge_t);
	const dv30edge_t* pinedges = reinterpret_cast<const dv30edge_t*>(pfile + lump.offset);
	medge_t* poutedges = new medge_t[count];

	model.pedges = poutedges;
	model.numedges = count;

	for(Uint32 i = 0; i < count; i++)
	{
		poutedges[i].vertexes[0] = Common::ByteToUint16((const byte*)&pinedges[i].vertexes[0]);
		poutedges[i].vertexes[1] = Common::ByteToUint16((const byte*)&pinedges[i].vertexes[1]);
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
bool BSPV30_LoadSurfedges( const byte* pfile, brushmodel_t& model, const lump_t& lump )
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
bool BSPV30_LoadTextures( const byte* pfile, brushmodel_t& model, const lump_t& lump )
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
bool BSPV30_LoadLighting( const byte* pfile, brushmodel_t& model, const lump_t& lump )
{
	if(!lump.size)
		return true;

	// Check if sizes are correct
	if(lump.size % sizeof(color24_t))
	{
		Con_EPrintf("%s - Inconsistent lump size in '%s'.\n", __FUNCTION__, model.name.c_str());
		return false;
	}

	model.plightdata = reinterpret_cast<color24_t *>(new byte[lump.size]);
	model.lightdatasize = lump.size;

	const byte *psrc = (pfile + lump.offset);
	memcpy(model.plightdata, psrc, sizeof(byte)*lump.size);

	return true;
}

//=============================================
// @brief
//
//=============================================
bool BSPV30_LoadPlanes( const byte* pfile, brushmodel_t& model, const lump_t& lump )
{
	// Check if sizes are correct
	if(lump.size % sizeof(dv30plane_t))
	{
		Con_EPrintf("%s - Inconsistent lump size in '%s'.\n", __FUNCTION__, model.name.c_str());
		return false;
	}

	// Load the data in
	Uint32 count = lump.size/sizeof(dv30plane_t);
	const dv30plane_t* pinplanes = reinterpret_cast<const dv30plane_t*>(pfile + lump.offset);
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
		poutplanes[i].type = (planetype_t)pinplanes[i].type;
		poutplanes[i].signbits = bits;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
bool BSPV30_LoadTexinfo( const byte* pfile, brushmodel_t& model, const lump_t& lump )
{
	// Check if sizes are correct
	if(lump.size % sizeof(dv30texinfo_t))
	{
		Con_EPrintf("%s - Inconsistent lump size in '%s'.\n", __FUNCTION__, model.name.c_str());
		return false;
	}

	// Load the data in
	Uint32 count = lump.size/sizeof(dv30texinfo_t);
	const dv30texinfo_t* pintexinfos = reinterpret_cast<const dv30texinfo_t*>(pfile + lump.offset);
	mtexinfo_t* pouttexinfos = new mtexinfo_t[count];
	
	model.ptexinfos = pouttexinfos;
	model.numtexinfos = count;

	for(Uint32 i = 0; i < count; i++)
	{
		// Copy the alignment info
		memcpy(pouttexinfos[i].vecs, pintexinfos[i].vecs, sizeof(Float)*8);

		pouttexinfos[i].flags = pintexinfos[i].flags;
		Int32 textureindex = pintexinfos[i].miptex;

		if(textureindex >= (Int32)model.numtextures)
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
bool BSPV30_LoadFaces( const byte* pfile, brushmodel_t& model, const lump_t& lump )
{
	// Check if sizes are correct
	if(lump.size % sizeof(dv30face_t))
	{
		Con_EPrintf("%s - Inconsistent lump size in '%s'.\n", __FUNCTION__, model.name.c_str());
		return false;
	}

	// Load the data in
	Uint32 count = lump.size/sizeof(dv30face_t);
	const dv30face_t* pinfaces = reinterpret_cast<const dv30face_t*>(pfile + lump.offset);
	msurface_t* poutsurfaces = new msurface_t[count];
	
	model.psurfaces = poutsurfaces;
	model.numsurfaces = count;

	for(Uint32 i = 0; i < count; i++)
	{
		msurface_t* pout = &poutsurfaces[i];

		pout->firstedge = pinfaces[i].firstedge;
		pout->numedges = Common::ByteToInt16((const byte *)&pinfaces[i].numedges);
		pout->flags = 0;

		Uint16 planeindex = Common::ByteToUint16((const byte *)&pinfaces[i].planenum);
		Int16 side = Common::ByteToInt16((const byte *)&pinfaces[i].side);
		if(side)
			pout->flags |= SURF_PLANEBACK;

		pout->pplane = &model.pplanes[planeindex];
		
		Int16 texinfoindex = Common::ByteToInt16((const byte*)&pinfaces[i].texinfo);
		pout->ptexinfo = &model.ptexinfos[texinfoindex];

		if(!BSPV30_CalcSurfaceExtents(pout, model))
			return false;

		for(Uint32 j = 0; j < V30_MAX_LIGHTMAPS; j++)
			pout->styles[j] = pinfaces[i].lmstyles[j];
		
		if(pinfaces[i].lightoffset != -1)
		{
			pout->psamples = reinterpret_cast<color24_t*>(reinterpret_cast<byte*>(model.plightdata) + pinfaces[i].lightoffset);
			pout->lightoffset = pinfaces[i].lightoffset;
		}
		else
		{
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
bool BSPV30_LoadMarksurfaces( const byte* pfile, brushmodel_t& model, const lump_t& lump )
{
	// Check if sizes are correct
	if(lump.size % sizeof(Int16))
	{
		Con_EPrintf("%s - Inconsistent lump size in '%s'.\n", __FUNCTION__, model.name.c_str());
		return false;
	}

	// Load the data in
	Uint32 count = lump.size/sizeof(Int16);
	const Uint16* pinmarksurfaces = reinterpret_cast<const Uint16*>(pfile + lump.offset);
	msurface_t** poutmarksurfaces = new msurface_t*[count];

	model.pmarksurfaces = poutmarksurfaces;
	model.nummarksurfaces = count;

	for(Uint32 i = 0; i < count; i++)
	{
		Uint16 surfindex = Common::ByteToUint16((const byte*)&pinmarksurfaces[i]);
		if(surfindex >= count)
		{
			Con_EPrintf("BSPV30_LoadFaces - Bad surface index.\n");
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
bool BSPV30_LoadVisibility( const byte* pfile, brushmodel_t& model, const lump_t& lump )
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
bool BSPV30_LoadLeafs( const byte* pfile, brushmodel_t& model, const lump_t& lump )
{
	// Check if sizes are correct
	if(lump.size % sizeof(dv30leaf_t))
	{
		Con_EPrintf("%s - Inconsistent lump size in '%s'.\n", __FUNCTION__, model.name.c_str());
		return false;
	}

	// Load the data in
	Uint32 count = lump.size/sizeof(dv30leaf_t);
	const dv30leaf_t* pinleafs = reinterpret_cast<const dv30leaf_t*>(pfile + lump.offset);
	mleaf_t* poutleafs = new mleaf_t[count];

	model.pleafs = poutleafs;
	model.numleafs = count;

	for(Uint32 i = 0; i < count; i++)
	{
		mleaf_t* pout = &poutleafs[i];

		for(Uint32 j = 0; j < 3; j++)
		{
			pout->mins[j] = Common::ByteToInt16((const byte*)&pinleafs[i].mins[j]);
			pout->maxs[j] = Common::ByteToInt16((const byte*)&pinleafs[i].maxs[j]);
		}

		pout->contents = pinleafs[i].contents;

		Uint16 marksurfindex = Common::ByteToUint16((const byte*)&pinleafs[i].firstmarksurface);
		pout->pfirstmarksurface = &model.pmarksurfaces[marksurfindex];
		pout->nummarksurfaces = Common::ByteToUint16((const byte*)&pinleafs[i].nummarksurfaces);

		if(pinleafs[i].visoffset != -1)
			pout->pcompressedvis = model.pvisdata + pinleafs[i].visoffset;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
bool BSPV30_LoadNodes( const byte* pfile, brushmodel_t& model, const lump_t& lump )
{
	// Check if sizes are correct
	if(lump.size % sizeof(dv30node_t))
	{
		Con_EPrintf("%s - Inconsistent lump size in '%s'.\n", __FUNCTION__, model.name.c_str());
		return false;
	}

	// Load the data in
	Uint32 count = lump.size/sizeof(dv30node_t);
	const dv30node_t* pinnodes = reinterpret_cast<const dv30node_t*>(pfile + lump.offset);
	mnode_t* poutnodes = new mnode_t[count];

	model.pnodes = poutnodes;
	model.numnodes = count;

	for(Uint32 i = 0; i < count; i++)
	{
		mnode_t* pout = &poutnodes[i];

		for(Uint32 j = 0; j < 3; j++)
		{
			pout->mins[j] = Common::ByteToInt16((const byte*)&pinnodes[i].mins[j]);
			pout->maxs[j] = Common::ByteToInt16((const byte*)&pinnodes[i].maxs[j]);
		}

		pout->pplane = &model.pplanes[pinnodes[i].planenum];

		pout->firstsurface = Common::ByteToUint16((const byte*)&pinnodes[i].firstface);
		pout->numsurfaces = Common::ByteToUint16((const byte*)&pinnodes[i].numfaces);

		for(Uint32 j = 0; j < 2; j++)
		{
			Int16 nodeidx = Common::ByteToInt16((const byte*)&pinnodes[i].children[j]);

			if(nodeidx >= 0)
				pout->pchildren[j] = &model.pnodes[nodeidx];
			else
				pout->pchildren[j] = (mnode_t*)(&model.pleafs[-1-nodeidx]);
		}
	}

	// Set linkage info on nodes
	BSPV30_SetNodeParent(model.pnodes, nullptr);
	return true;
}

//=============================================
// @brief
//
//=============================================
bool BSPV30_LoadClipnodes( const byte* pfile, brushmodel_t& model, const lump_t& lump )
{
	// Check if sizes are correct
	if(lump.size % sizeof(dv30clipnode_t))
	{
		Con_EPrintf("%s - Inconsistent lump size in '%s'.\n", __FUNCTION__, model.name.c_str());
		return false;
	}

	// Load the data in
	Uint32 count = lump.size/sizeof(dv30clipnode_t);
	const dv30clipnode_t* pinnodes = reinterpret_cast<const dv30clipnode_t*>(pfile + lump.offset);
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
		poutnodes[i].children[0] = Common::ByteToInt16((const byte*)&pinnodes[i].children[0]);
		poutnodes[i].children[1] = Common::ByteToInt16((const byte*)&pinnodes[i].children[1]);
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
bool BSPV30_LoadEntities( const byte* pfile, brushmodel_t& model, const lump_t& lump )
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
bool BSPV30_LoadSubmodels( const byte* pfile, brushmodel_t& model, const lump_t& lump )
{
	// Check if sizes are correct
	if(lump.size % sizeof(dv30model_t))
	{
		Con_EPrintf("%s - Inconsistent lump size in '%s'.\n", __FUNCTION__, model.name.c_str());
		return false;
	}

	// Load the data in
	Uint32 count = lump.size/sizeof(dv30model_t);
	const dv30model_t* pinmodels = reinterpret_cast<const dv30model_t*>(pfile + lump.offset);
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

		for(Uint32 j = 0; j < V30_MAX_MAP_HULLS; j++)
			poutmodels[i].headnode[j] = pinmodels[i].headnode[j];

		poutmodels[i].visleafs = pinmodels[i].visleafs;
		poutmodels[i].firstface = pinmodels[i].firstface;
		poutmodels[i].numfaces = pinmodels[i].numfaces;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void BSPV30_SetNodeParent( mnode_t* pnode, mnode_t* pparent )
{
	pnode->pparent = pparent;
	if(pnode->contents < 0)
		return;

	BSPV30_SetNodeParent(pnode->pchildren[0], pnode);
	BSPV30_SetNodeParent(pnode->pchildren[1], pnode);
}

//=============================================
// @brief
//
//=============================================
bool BSPV30_CalcSurfaceExtents( msurface_t* psurf, brushmodel_t& model )
{
	Vector vmins = NULL_MINS;
	Vector vmaxs = NULL_MAXS;

	Float mins[2] = { NULL_MINS[0], NULL_MINS[1] };
	Float maxs[2] = { NULL_MAXS[0], NULL_MAXS[1] };

	mtexinfo_t* ptexinfo = psurf->ptexinfo;
	for(Uint32 i = 0; i < psurf->numedges; i++)
	{
		// Get the vertex
		mvertex_t* pvertex = nullptr;
		Int32 edgeindex = model.psurfedges[psurf->firstedge+i];
		if(edgeindex >= 0)
			pvertex = &model.pvertexes[model.pedges[edgeindex].vertexes[0]];
		else
			pvertex = &model.pvertexes[model.pedges[-edgeindex].vertexes[1]];

		// Set mins/maxs coords
		for(Uint32 j = 0; j < 3; j++)
		{
			if(pvertex->origin[j] < vmins[j])
				vmins[j] = pvertex->origin[j];

			if(pvertex->origin[j] > vmaxs[j])
				vmaxs[j] = pvertex->origin[j];
		}

		// Calculate surface extents
		for(Uint32 j = 0; j < 2; j++)
		{
			Float val = pvertex->origin[0] * (Double)ptexinfo->vecs[j][0] 
					+ pvertex->origin[1] * (Double)ptexinfo->vecs[j][1]
					+ pvertex->origin[2] * (Double)ptexinfo->vecs[j][2]
					+ (Double)ptexinfo->vecs[j][3];

			if(val < mins[j])
				mins[j] = val;

			if(val > maxs[j])
				maxs[j] = val;
		}
	}

	// Set mins/maxs
	psurf->mins = vmins;
	psurf->maxs = vmaxs;

	for(Uint32 i = 0; i < 2; i++)
	{
		Int16 boundsmin = (Int16)SDL_floor(mins[i]/V30_LM_BASE_SAMPLE_SIZE);
		Int16 boundsmax = (Int16)SDL_ceil(maxs[i]/V30_LM_BASE_SAMPLE_SIZE);

		psurf->texturemins[i] = boundsmin*V30_LM_BASE_SAMPLE_SIZE;
		psurf->extents[i] = (boundsmax - boundsmin) * V30_LM_BASE_SAMPLE_SIZE;

		if(!(ptexinfo->flags & TEXFLAG_SPECIAL) && psurf->extents[i] > MAX_SURFACE_EXTENTS)
		{
			Con_EPrintf("%s: Bad surface extents.\n", __FUNCTION__);
			return false;
		}
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void BSPV30_MakeHullZero( brushmodel_t& model )
{
	hull_t* phull = &model.hulls[0];
	phull->pclipnodes = new mclipnode_t[model.numnodes];
	phull->firstclipnode = 0;
	phull->lastclipnode = model.numnodes;
	phull->pplanes = model.pplanes;

	for(Uint32 i = 0; i < model.numnodes; i++)
	{
		mclipnode_t* pnode = &phull->pclipnodes[i];
		pnode->planenum = model.pnodes[i].pplane - model.pplanes;

		for(Uint32 j = 0; j < 2; j++)
		{
			mnode_t* pchild = model.pnodes[i].pchildren[j];
			if(pchild->contents < 0)
				pnode->children[j] = pchild->contents;
			else
				pnode->children[j] = pchild - model.pnodes;
		}
	}
}

//=============================================
// @brief
//
//=============================================
void BSPV30_SetupPAS( brushmodel_t& model )
{
	byte* ppasdata = new byte[ens.visbuffersize];
	memset(ppasdata, 0, sizeof(byte)*ens.visbuffersize);

	// Set up PAS
	Int32 num = model.numleafs + 1;
	Int32 rowwords = (num + 31)>>5;
	Int32 rowbytes = rowwords * 4;

	Int32 *visofs = new Int32[num];
	byte *uncompressed_vis = new byte[rowbytes*num];
	byte *uncompressed_pas = new byte[rowbytes*num];
	byte *compressed_pas = new byte[rowbytes*num*4];

	byte *vismap, *vismap_p;
	vismap = vismap_p = compressed_pas;
	byte *scan = uncompressed_vis;

	for( Int32 i = 0; i < num; i++, scan += rowbytes )
		memcpy( scan, Mod_LeafPVS(ppasdata, ens.visbuffersize, model.pleafs[i], model), sizeof(byte)*rowbytes );

	Uint32 rowsize = 0, total_size = 0;
	Uint32 *dest = reinterpret_cast<Uint32 *>(uncompressed_pas);
	scan = uncompressed_vis;

	for( Int32 i = 0; i < num; i++, dest += rowwords, scan += rowbytes )
	{
		memcpy( dest, scan, sizeof(byte)*rowbytes );

		for( Int32 j = 0; j < rowbytes; j++ )
		{
			Int32 bitbyte = scan[j];
			if( !bitbyte ) 
				continue;

			for( Int32 k = 0; k < 8; k++ )
			{
				if(!( bitbyte & ( 1<<k )))
					continue;

				Int32 index = ((j<<3) + k + 1);
				if( index >= num ) 
					continue;

				Uint32 *src = reinterpret_cast<Uint32 *>(uncompressed_vis + index * rowwords);
				for( Int32 l = 0; l < rowwords; l++ )
					dest[l] |= src[l];
			}
		}

		byte *comp = Mod_CompressVIS( reinterpret_cast<byte *>(dest), &rowsize, model, ens.visbuffersize );
		visofs[i] = vismap_p - vismap; 
		total_size += rowsize;

		memcpy( vismap_p, comp, sizeof(byte)*rowsize );
		vismap_p += rowsize;

		// Delete temp data
		delete[] comp;
	}

	// Allocate final data array
	model.ppasdata = new byte[total_size];
	memcpy(model.ppasdata, compressed_pas, sizeof(byte)*total_size);
	model.pasdatasize = total_size;

	for( Uint32 i = 0; i < model.numleafs; i++ )
		model.pleafs[i].pcompressedpas = model.ppasdata + visofs[i];

	delete[] compressed_pas;
	delete[] uncompressed_vis;
	delete[] uncompressed_pas;
	delete[] visofs;
	delete[] ppasdata;
}