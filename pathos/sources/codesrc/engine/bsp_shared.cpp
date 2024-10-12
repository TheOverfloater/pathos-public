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
#include "bsp_shared.h"
#include "system.h"
#include "logfile.h"
#include "enginestate.h"


//=============================================
// @brief
//
//=============================================
void BSP_SetNodeParent( mnode_t* pnode, mnode_t* pparent )
{
	pnode->pparent = pparent;
	if(pnode->contents < 0)
		return;

	BSP_SetNodeParent(pnode->pchildren[0], pnode);
	BSP_SetNodeParent(pnode->pchildren[1], pnode);
}

//=============================================
// @brief
//
//=============================================
bool BSP_CalcSurfaceExtents( msurface_t* psurf, brushmodel_t& model )
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
			Float val = pvertex->origin[0] * static_cast<Double>(ptexinfo->vecs[j][0]) 
					+ pvertex->origin[1] * static_cast<Double>(ptexinfo->vecs[j][1])
					+ pvertex->origin[2] * static_cast<Double>(ptexinfo->vecs[j][2])
					+ static_cast<Double>(ptexinfo->vecs[j][3]);

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
		Int16 boundsmin = static_cast<Int16>(SDL_floor(mins[i]/psurf->lightmapdivider));
		Int16 boundsmax = static_cast<Int16>(SDL_ceil(maxs[i]/psurf->lightmapdivider));

		psurf->texturemins[i] = boundsmin*psurf->lightmapdivider;
		psurf->extents[i] = (boundsmax - boundsmin) * psurf->lightmapdivider;

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
void BSP_MakeHullZero( brushmodel_t& model )
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
void BSP_SetupPAS( brushmodel_t& model )
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