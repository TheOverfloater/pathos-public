/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "com_math.h"
#include "textures_shared.h"
#include "brushmodel.h"
#include "frustum.h"
#include "system.h"
#include "r_common.h"

//=============================================
// @brief
//
//=============================================
const mleaf_t* Mod_PointInLeaf( const Vector& position, const brushmodel_t& model )
{
	if(!model.pnodes)
		return nullptr;

	mnode_t* pnode = model.pnodes;
	while(true)
	{
		if(pnode->contents < 0)
			return reinterpret_cast<mleaf_t*>(pnode);

		plane_t* pplane = pnode->pplane;
		Float dp = Math::DotProduct(position, pplane->normal) - pplane->dist;
		if(dp > 0)
			pnode = pnode->pchildren[0];
		else
			pnode = pnode->pchildren[1];
	}

	return nullptr;
}

//=============================================
// @brief
//
//=============================================
byte* Mod_DecompressVIS( byte* pbuffer, Uint32 bufsize, byte* pin, const brushmodel_t& model, Uint32 bytecount )
{
	byte* pout = pbuffer;
	byte *ppin = pin;

	if(!ppin)
	{
		// Make all visible if there's no vis info
		for(Uint32 i = 0; i < bytecount; i++)
		{
			(*pout) = 0xFF;
			pout++;

			if((pout-pbuffer) >= bufsize)
			{
				Con_EPrintf("VIS buffer overflow.\n");
				return pbuffer;
			}
		}

		return pbuffer;
	}

	// Mark visible visleafs
	while(pout < pbuffer + bytecount)
	{
		if(ppin - model.pvisdata >= model.visdatasize)
			break;

		if(*ppin)
		{
			(*pout) = (*ppin);
			pout++; ppin++;

			if((pout - pbuffer) >= bufsize)
			{
				Con_EPrintf("VIS buffer overflow.\n");
				return pbuffer;
			}

			continue;
		}

		Uint32 count = ppin[1];
		ppin += 2;

		if(count > pbuffer + bytecount - pout)
			count = pbuffer + bytecount - pout;

		memset(pout, 0, sizeof(byte)*count);
		pout += count;

		if((pout - pbuffer) >= bufsize)
		{
			Con_EPrintf("VIS buffer overflow.\n");
			return pbuffer;
		}
	}

	return pbuffer;
}

//=============================================
// @brief
//
//=============================================
byte* Mod_DecompressPAS( byte* pbuffer, Uint32 bufsize, byte* pin, const brushmodel_t& model, Uint32 bytecount )
{
	byte* pout = pbuffer;
	byte *ppin = pin;

	if(!ppin)
	{
		// Make all visible if there's no vis info
		for(Uint32 i = 0; i < bytecount; i++)
		{
			(*pout) = 0xFF;
			pout++;

			if((pout-pbuffer) >= bufsize)
			{
				Con_EPrintf("VIS buffer overflow.\n");
				return pbuffer;
			}
		}

		return pbuffer;
	}

	// Mark visible visleafs
	while(pout < pbuffer + bytecount)
	{
		if(ppin - model.ppasdata >= model.pasdatasize)
			break;

		if(*ppin)
		{
			(*pout) = (*ppin);
			pout++; ppin++;

			if((pout - pbuffer) >= bufsize)
			{
				Con_EPrintf("VIS buffer overflow.\n");
				return pbuffer;
			}

			continue;
		}

		Uint32 count = ppin[1];
		ppin += 2;

		if(count > pbuffer + bytecount - pout)
			count = pbuffer + bytecount - pout;

		memset(pout, 0, sizeof(byte)*count);
		pout += count;

		if((pout - pbuffer) >= bufsize)
		{
			Con_EPrintf("VIS buffer overflow.\n");
			return pbuffer;
		}
	}

	return pbuffer;
}

//=============================================
// @brief
//
//=============================================
const byte* Mod_LeafPVS( byte* pbuffer, Uint32 bufsize, const mleaf_t& leaf, const brushmodel_t& model )
{
	Uint32 bytecount = (model.numleafs + 7) >> 3;
	if(&leaf == model.pleafs)
		return Mod_DecompressVIS(pbuffer, bufsize, nullptr, model, bytecount);
	else
		return Mod_DecompressVIS(pbuffer, bufsize, leaf.pcompressedvis, model, bytecount);
}

//=============================================
// @brief
//
//=============================================
const byte* Mod_LeafPAS( byte* pbuffer, Uint32 bufsize, const mleaf_t& leaf, const brushmodel_t& model )
{
	Uint32 bytecount = (model.numleafs + 7) / 8;
	if(&leaf == model.pleafs)
		return Mod_DecompressPAS(pbuffer, bufsize, nullptr, model, bytecount);
	else
		return Mod_DecompressPAS(pbuffer, bufsize, leaf.pcompressedpas, model, bytecount);
}

//=============================================
// @brief
//
//=============================================
byte* Mod_CompressVIS( const byte* pin, Uint32* psize, const brushmodel_t& model, Int32 visbuffersize )
{
	Int32 rownum = (model.numleafs+7)>>3;
	byte* pvisdata = new byte[visbuffersize];
	byte* pdest = pvisdata;

	for(Int32 j = 0; j < rownum; j++)
	{
		*pdest = pin[j];
		pdest++;

		if(pin[j])
			continue;

		Int32 rep = 1;
		for(j++; j < rownum; j++)
		{
			if(pin[j] || rep == 255)
				break;
			else
				rep++;
		}
		
		*pdest = rep;
		pdest++;

		j--;
	}

	if(psize)
		(*psize) = pdest - pvisdata;

	return pvisdata;
}

//=============================================
// @brief
//
//=============================================
const msurface_t* Mod_SurfaceAtPoint( const brushmodel_t* pmodel, const mnode_t* pnode, const Vector& start, const Vector& end )
{
	if(pnode->contents < 0)
		return nullptr;

	plane_t* pplane = pnode->pplane;
	Float front = Math::DotProduct(start, pplane->normal) - pplane->dist;
	Float back = Math::DotProduct(end, pplane->normal) - pplane->dist;

	bool s = (front < 0.0f) ? true : false;
	bool t = (back < 0.0f) ? true : false;

	if(t == s)
		return Mod_SurfaceAtPoint(pmodel, pnode->pchildren[s], start, end);

	Vector mid, point;
	Float frac = front / (front - back);
	Math::VectorSubtract(end, start, point);
	Math::VectorMA(start, frac, point, mid);

	const msurface_t* psurface = Mod_SurfaceAtPoint(pmodel, pnode->pchildren[s], start, mid);
	if(psurface)
		return psurface;

	for(Uint32 i = 0; i < pnode->numsurfaces; i++)
	{
		psurface = &pmodel->psurfaces[pnode->firstsurface + i];
		mtexinfo_t *ptexinfo = psurface->ptexinfo;

		Int32 ds = (Int32)(Math::DotProduct(mid, ptexinfo->vecs[0]) + ptexinfo->vecs[0][3]);
		Int32 dt = (Int32)(Math::DotProduct(mid, ptexinfo->vecs[1]) + ptexinfo->vecs[1][3]);

		if(ds >= psurface->texturemins[0] && dt >= psurface->texturemins[1])
		{
			if((ds - psurface->texturemins[0]) <= psurface->extents[0] &&
				(dt - psurface->texturemins[1]) <= psurface->extents[1])
				return psurface;
		}
	}

	return Mod_SurfaceAtPoint(pmodel, pnode->pchildren[s ^ 1], mid, end);
}

//=============================================
//
//=============================================
void Mod_FindTouchedLeafs( const brushmodel_t* pworld, CArray<Uint32>& leafnumsarray, const Vector& mins, const Vector& maxs, mnode_t* pnode )
{
	if(pnode->contents == CONTENTS_SOLID)
		return;

	if(pnode->contents < 0)
	{
		mleaf_t* pleaf = reinterpret_cast<mleaf_t*>(pnode);
		Uint32 leafnum = pleaf - pworld->pleafs - 1;

		leafnumsarray.push_back(leafnum);
		return;
	}

	plane_t* pplane = pnode->pplane;
	Int32 sides = BoxOnPlaneSide(mins, maxs, pplane);

	// Recurse down the sides
	if(sides & 1)
		Mod_FindTouchedLeafs(pworld, leafnumsarray, mins, maxs, pnode->pchildren[0]);

	if(sides & 2)
		Mod_FindTouchedLeafs(pworld, leafnumsarray, mins, maxs, pnode->pchildren[1]);
}

//=============================================
//
//=============================================
bool Mod_RecursiveLightPoint( const brushmodel_t* pworld, mnode_t *pnode, const Vector &start, const Vector &end, Vector &color )
{
	if (pnode->contents < 0)
		return false;
	
	plane_t* pplane = pnode->pplane;
	Float front = Math::DotProduct (start, pplane->normal) - pplane->dist;
	Float back = Math::DotProduct (end, pplane->normal) - pplane->dist;
	Int32 side = front < 0;
	
	if ( (back < 0) == side )
		return Mod_RecursiveLightPoint(pworld, pnode->pchildren[side], start, end, color);
	
	Vector mid;
	Float frac = front / (front-back);
	mid[0] = start[0] + (end[0] - start[0])*frac;
	mid[1] = start[1] + (end[1] - start[1])*frac;
	mid[2] = start[2] + (end[2] - start[2])*frac;
	
	// go down front side	
	if (Mod_RecursiveLightPoint(pworld, pnode->pchildren[side], start, mid, color)) 
		return true;
		
	if ((back < 0) == side)
		return true;

	msurface_t* psurfaces = pworld->psurfaces + pnode->firstsurface;
	for (Uint32 i = 0; i < pnode->numsurfaces; i++)
	{
		msurface_t* psurf = &psurfaces[i];
		if (psurf->flags & (SURF_DRAWTURB | SURF_DRAWSKY) || !psurf->psamples)
			continue;	// no lightmaps

		mtexinfo_t* ptexinfo = psurf->ptexinfo;
		Int32 s = Math::DotProduct(mid, ptexinfo->vecs[0])+ptexinfo->vecs[0][3];
		Int32 t = Math::DotProduct(mid, ptexinfo->vecs[1])+ptexinfo->vecs[1][3];

		if (s < psurf->texturemins[0] || t < psurf->texturemins[1])
			continue;
		
		Int32 ds = s - psurf->texturemins[0];
		Int32 dt = t - psurf->texturemins[1];
		
		if (ds > psurf->extents[0] || dt > psurf->extents[1])
			continue;

		ds >>= 4;
		dt >>= 4;

		color24_t* plightmap = psurf->psamples;
		plightmap += dt * ((psurf->extents[0]>>4)+1) + ds;

		Float flIntensity = (plightmap->r + plightmap->g + plightmap->b)/3;
		Float flScale = flIntensity/35;

		if(flScale > 1.0) 
			flScale = 1.0;

		Common::ParseColor(color, plightmap);
		Math::VectorScale(color, flScale, color);

		return true;
	}

	// go down back side
	return Mod_RecursiveLightPoint(pworld, pnode->pchildren[!side], mid, end, color);
}

//=============================================
//
//=============================================
bool Mod_RecursiveLightPoint_BumpData( const brushmodel_t* pworld, mnode_t *pnode, const Vector &start, const Vector &end, Vector &ambientcolor, Vector& diffusecolor, Vector& lightdir )
{
	if (pnode->contents < 0)
		return false;
	
	plane_t* pplane = pnode->pplane;
	Float front = Math::DotProduct (start, pplane->normal) - pplane->dist;
	Float back = Math::DotProduct (end, pplane->normal) - pplane->dist;
	Int32 side = front < 0;
	
	if ( (back < 0) == side )
		return Mod_RecursiveLightPoint_BumpData(pworld, pnode->pchildren[side], start, end, ambientcolor, diffusecolor, lightdir);
	
	Vector mid;
	Float frac = front / (front-back);
	mid[0] = start[0] + (end[0] - start[0])*frac;
	mid[1] = start[1] + (end[1] - start[1])*frac;
	mid[2] = start[2] + (end[2] - start[2])*frac;
	
	// go down front side	
	if (Mod_RecursiveLightPoint_BumpData(pworld, pnode->pchildren[side], start, mid, ambientcolor, diffusecolor, lightdir)) 
		return true;
		
	if ((back < 0) == side)
		return true;

	msurface_t* psurfaces = pworld->psurfaces + pnode->firstsurface;
	for (Uint32 i = 0; i < pnode->numsurfaces; i++)
	{
		msurface_t* psurf = &psurfaces[i];
		if (psurf->flags & (SURF_DRAWTURB | SURF_DRAWSKY))
			continue;	// no lightmaps

		mtexinfo_t* ptexinfo = psurf->ptexinfo;
		Int32 s = Math::DotProduct(mid, ptexinfo->vecs[0])+ptexinfo->vecs[0][3];
		Int32 t = Math::DotProduct(mid, ptexinfo->vecs[1])+ptexinfo->vecs[1][3];

		if (s < psurf->texturemins[0] || t < psurf->texturemins[1])
			continue;
		
		Int32 ds = s - psurf->texturemins[0];
		Int32 dt = t - psurf->texturemins[1];
		
		if (ds > psurf->extents[0] || dt > psurf->extents[1])
			continue;

		if (!psurf->psamples)
			continue;

		ds >>= 4;
		dt >>= 4;

		Int32 ambientindex = R_StyleIndex(psurf, LM_AMBIENT_STYLE);
		Int32 diffuseindex = R_StyleIndex(psurf, LM_DIFFUSE_STYLE);
		Int32 lightvecsindex = R_StyleIndex(psurf, LM_LIGHTVECS_STYLE);

		// Fail if the surface has no bump data
		if(ambientindex == -1 || diffuseindex == -1 || lightvecsindex == -1)
			return false;

		Uint32 xsize = (psurf->extents[0]>>4)+1;
		Uint32 ysize = (psurf->extents[1]>>4)+1;
		Uint32 size = xsize*ysize;

		// Get ambient light
		color24_t* pambientlightmap = psurf->psamples + size * ambientindex;
		pambientlightmap += dt * xsize + ds;

		Float flIntensity = (pambientlightmap->r + pambientlightmap->g + pambientlightmap->b)/3;
		Float flScale = flIntensity/35;

		Common::ParseColor(ambientcolor, pambientlightmap);
		Math::VectorScale(ambientcolor, flScale, ambientcolor);

		// Get diffuse light
		color24_t* pdiffuselightmap = psurf->psamples + size * diffuseindex;
		pdiffuselightmap += dt * xsize + ds;

		flIntensity = (pdiffuselightmap->r + pdiffuselightmap->g + pdiffuselightmap->b)/3;
		flScale = flIntensity/35;

		Common::ParseColor(diffusecolor, pdiffuselightmap);
		Math::VectorScale(diffusecolor, flScale, diffusecolor);

		// Get light direction
		color24_t* plightdirdata = psurf->psamples + size * lightvecsindex;
		plightdirdata += dt * ((psurf->extents[0]>>4)+1) + ds;

		// Turn byte data to light vectors
		lightdir = Vector(plightdirdata->r, plightdirdata->g, plightdirdata->b);
		Math::VectorScale(lightdir, 1.0f/255.0f, lightdir);

		for(Uint32 j = 0; j < 3; j++)
			lightdir[j] = (2.0f * lightdir[j]) - 1.0;

		// Reverse green
		lightdir[1] *= -1.0;
		return true;
	}

	// go down back side
	return Mod_RecursiveLightPoint_BumpData(pworld, pnode->pchildren[!side], mid, end, ambientcolor, diffusecolor, lightdir);
}