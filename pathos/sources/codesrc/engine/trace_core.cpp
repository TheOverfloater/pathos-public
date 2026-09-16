/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

//
// Core traceline functions, includes both legacy clipnode
// versions and the latest brush based collisions. Brush based
// collisions were written referencing Quake 2 code, but with
// some major changes.
//

#include "includes.h"
#include "entity_state.h"
#include "brushmodel.h"
#include "modelcache.h"
#include "system.h"
#include "common.h"
#include "com_math.h"
#include "trace.h"
#include "edict.h"
#include "enginestate.h"
#include "vbmtrace.h"
#include "mcdtrace.h"
#include "sv_world.h"
#include "collision_shared.h"
#include "trace_core.h"

// Hull mins for tracehull
Vector HULL_MINS[MAX_MAP_HULLS] = {
	Vector(0.0f, 0.0f, 0.0f),		// Point hull
	Vector(-16.0f, -16.0f, -36.0f),	// Human hull
	Vector(-32.0f, -32.0f, -32.0f),	// Large hull
	Vector(-16.0f, -16.0f, -18.0f)	// Small hull
};

// Hull maxs for tracehull
Vector HULL_MAXS[MAX_MAP_HULLS] = {
	Vector(0.0f, 0.0f, 0.0f),		// Point hull
	Vector(16.0f, 16.0f, 36.0f),	// Human hull
	Vector(32.0f, 32.0f, 32.0f),	// Large hull
	Vector(16.0f, 16.0f, 18.0f)		// Small hull
};

// Box hull used by collision detection
static mclipnode_t g_boxClipnodes[6];
// Box planes used by collision detection
static plane_t g_boxPlanes[6];
// Hull used by collision detection
static hull_t g_boxHull;
// Counter for number of traces
static Uint32 g_brushTraceCount = 0;

// Leaf pointer array alloc size
static constexpr Uint32 LEAF_ARRAY_ALLOC_SIZE = 128;
// Leaf pointer array
CArray<const mleaf_t*> g_leafArray(LEAF_ARRAY_ALLOC_SIZE);
// Number of leafs in array
Uint32 g_numLeafs = 0;

// Brush pointer array alloc size
static constexpr Uint32 BRUSH_ARRAY_ALLOC_SIZE = 128;
// Leaf pointer array
CArray<mbrush_t*> g_brushArray(BRUSH_ARRAY_ALLOC_SIZE);
// Number of leafs in array
Uint32 g_numBrushes = 0;


// Hull epsilon value
static const Float HULL_EPSILON = 0.03125;

//=============================================
//
//=============================================
Int32 RankForContents( Int32 contents )
{
	switch(contents)
	{
	case CONTENTS_EMPTY:
		return 0;
	case CONTENTS_WATER:
		return 1;
	case CONTENTS_SLIME:
		return 2;
	case CONTENTS_LAVA:
		return 3;
	case CONTENTS_SKY:
		return 4;
	case CONTENTS_SOLID:
		return 5;
	default:
		return 0;
	}
}

//=============================================
//
//=============================================
void TR_InitBoxHull( void )
{
	g_boxHull.pclipnodes = g_boxClipnodes;
	g_boxHull.pplanes = g_boxPlanes;
	g_boxHull.firstclipnode = 0;
	g_boxHull.lastclipnode = 5;

	for(Uint32 i = 0; i < 6; i++)
	{
		Int32 side = i & 1;
		g_boxClipnodes[i].planenum = i;
		g_boxClipnodes[i].children[side] = CONTENTS_EMPTY;
		g_boxClipnodes[i].children[side ^ 1] = (i != 5)  ? (i + 1) : CONTENTS_SOLID;

		g_boxPlanes[i].type = i >> 1;
		g_boxPlanes[i].normal[g_boxPlanes[i].type] = 1.0f;
	}
}

//=============================================
//
//=============================================
const hull_t* TR_HullForBox( const Vector& mins, const Vector& maxs )
{
	g_boxPlanes[0].dist = maxs[0];
	g_boxPlanes[1].dist = mins[0];
	g_boxPlanes[2].dist = maxs[1];
	g_boxPlanes[3].dist = mins[1];
	g_boxPlanes[4].dist = maxs[2];
	g_boxPlanes[5].dist = mins[2];

	return &g_boxHull;
}

//=============================================
//
//=============================================
void TR_MoveBounds( const Vector& start, const Vector& mins, const Vector& maxs, const Vector&end, Vector& boxmins, Vector& boxmaxs )
{
	for(Uint32 i = 0; i < 3; i++)
	{
		if(end[i] > start[i])
		{
			boxmins[i] = start[i] + mins[i] - 1.0f;
			boxmaxs[i] = end[i] + maxs[i] + 1.0f;
		}
		else
		{
			boxmins[i] = end[i] + mins[i] - 1.0f;
			boxmaxs[i] = start[i] + maxs[i] + 1.0f;			
		}
	}
}

//=============================================
//
//=============================================
void TR_MoveBoundsPoint( const Vector& start, const Vector&end, Vector& boxmins, Vector& boxmaxs )
{
	for(Uint32 i = 0; i < 3; i++)
	{
		if(end[i] > start[i])
		{
			boxmins[i] = start[i] - 1.0f;
			boxmaxs[i] = end[i] + 1.0f;
		}
		else
		{
			boxmins[i] = end[i] - 1.0f;
			boxmaxs[i] = start[i] + 1.0f;			
		}
	}
}

//=============================================
//
//=============================================
const hull_t* TR_HullForBSP( const entity_state_t& entity, hull_types_t hulltype, Vector& offset, const Vector& player_mins )
{
	if(hulltype >= MAX_MAP_HULLS || hulltype < 0)
	{
		Con_EPrintf("%s - Bogus hull index %d.\n", __FUNCTION__, hulltype);
		return 0;
	}

	const cache_model_t* pcache = Cache_GetModel(entity.modelindex);
	if(pcache->type != MOD_BRUSH)
	{
		Con_EPrintf("%s called with an pentity %d that doesn't have MOD_BRUSH model type.\n", __FUNCTION__, entity.entindex);
		return nullptr;
	}

	const brushmodel_t* pmodel = pcache->getBrushmodel();
	const hull_t* phull = &pmodel->hulls[hulltype];

	Math::VectorSubtract(phull->clipmins, player_mins, offset);
	Math::VectorAdd(offset, entity.origin, offset);

	return phull;
}

//=============================================
//
//=============================================
const Char* TR_TraceTexture( const entity_state_t& entity, const Vector& start, const Vector& end )
{
	Vector offset;
	Vector start_l;
	Vector end_l;

	const hull_t* phull = nullptr;
	const cache_model_t* pmodel = Cache_GetModel(entity.modelindex);
	if(pmodel->type == MOD_BRUSH)
	{
		Int32 firstnode = 0;
		const brushmodel_t* pbrushmodel = pmodel->getBrushmodel();
		if(entity.entindex > 0)
		{
			phull = TR_HullForBSP(entity, HULL_POINT, offset, ZERO_VECTOR);
			Math::VectorSubtract(start, offset, start_l);
			Math::VectorSubtract(end, offset, end_l);

			firstnode = phull->firstclipnode;

			if(!entity.angles.IsZero())
			{
				Math::RotateToEntitySpace(entity.angles, start_l);
				Math::RotateToEntitySpace(entity.angles, end_l);
			}
		}
		else
		{
			Math::VectorCopy(start, start_l);
			Math::VectorCopy(end, end_l);
		}

		const msurface_t* psurface = Mod_SurfaceAtPoint(pbrushmodel, &pbrushmodel->pnodes[firstnode], start_l, end_l);
		if(psurface)
			return psurface->ptexinfo->ptexture->name.c_str();
		else
			return nullptr;
	}
	else if(pmodel->type == MOD_VBM && (pmodel->cacheflags & CACHE_FL_HAS_MCD))
	{
		Math::VectorSubtract(start, entity.origin, start_l);
		Math::VectorSubtract(end, entity.origin, end_l);

		if(!entity.angles.IsZero())
		{
			Math::RotateToEntitySpace(entity.angles, start_l);
			Math::RotateToEntitySpace(entity.angles, end_l);
		}

		const vbmcache_t* pvbmcache = pmodel->getVBMCache();
		assert(pvbmcache != nullptr);

		const mcdheader_t* pmcdheader = pvbmcache->pmcdheader;
		assert(pmcdheader != nullptr);

		trace_t tr;
		if(g_mcdTrace.TraceLinePoint(start_l, end_l, pmcdheader, entity.body, tr))
			return g_mcdTrace.GetHitTextureName();
		else
			return nullptr;
	}
	else
	{
		Con_Printf("%s - Called on entity %d with model that is not a brush model or a VBM with an MCD file.\n", __FUNCTION__, entity.entindex);
		return nullptr;
	}
}

//=============================================
//
//=============================================
Int32 TR_HullPointContents_ClipNode( const hull_t* phull, Int32 clipnodeidx, const Vector& position )
{
	Int32 index = clipnodeidx;
	while(index >= 0)
	{
		if(index < phull->firstclipnode || index > phull->lastclipnode)
		{
			Con_Printf("%s - Bad node number.\n", __FUNCTION__);
			return 0;
		}

		const mclipnode_t* pclipnode = &phull->pclipnodes[index];
		const plane_t* pplane = &phull->pplanes[pclipnode->planenum];

		Float dp;
		if(pplane->type < 3)
			dp = position[pplane->type] - pplane->dist;
		else
			dp = Math::DotProduct(pplane->normal, position) - pplane->dist;

		if(dp < 0)
			index = pclipnode->children[1];
		else
			index = pclipnode->children[0];
	}

	return index;
}

//=============================================
//
//=============================================
bool TR_RecursiveHullCheck_ClipNode( const hull_t* phull, Int32 clipnodeidx, Double p1f, Double p2f, const Vector& p1, const Vector& p2, trace_t& trace )
{
	if(clipnodeidx < 0)
	{
		if(clipnodeidx != CONTENTS_SOLID)
		{
			// Remove allsolid flags
			trace.flags &= ~FL_TR_ALLSOLID;

			if(clipnodeidx == CONTENTS_EMPTY)
				trace.flags |= FL_TR_INOPEN;
			else
				trace.flags |= FL_TR_INWATER;
		}
		else
		{
			// Mark as start solid
			trace.flags |= FL_TR_STARTSOLID;
		}

		return true;
	}

	// Make sure we're valid
	if(clipnodeidx < phull->firstclipnode || clipnodeidx > phull->lastclipnode || !phull->pplanes)
	{
		Con_Printf("%s - Bad node number %d.\n", __FUNCTION__, clipnodeidx);
		return false;
	}

	// Find the point distances
	const mclipnode_t& node = phull->pclipnodes[clipnodeidx];
	const plane_t& plane = phull->pplanes[node.planenum];

	Double t1, t2;
	if(plane.type < 3)
	{
		t1 = p1[plane.type] - plane.dist;
		t2 = p2[plane.type] - plane.dist;
	}
	else
	{
		t1 = Math::DotProduct(plane.normal, p1) - plane.dist;
		t2 = Math::DotProduct(plane.normal, p2) - plane.dist;
	}

	if(t1 >= 0 && t2 >= 0)
		return TR_RecursiveHullCheck_ClipNode(phull, node.children[0], p1f, p2f, p1, p2, trace);
	if(t1 < 0 && t2 < 0)
		return TR_RecursiveHullCheck_ClipNode(phull, node.children[1], p1f, p2f, p1, p2, trace);

	Double frac;
	if(t1 < 0.0f)
		frac = (t1 + DIST_EPSILON) / (t1 - t2);
	else
		frac = (t1 - DIST_EPSILON) / (t1 - t2);

	frac = clamp(frac, 0, 1);
	if(Common::IsNAN(frac))
		return false;

	Double pdiff = p2f - p1f;
	Double midf = p1f + pdiff * frac;

	Vector point, mid;
	Math::VectorSubtract(p2, p1, point);
	Math::VectorMA(p1, frac, point, mid);

	Int32 side = (t1 < 0) ? 1 : 0;

	// Move up the node
	if(!TR_RecursiveHullCheck_ClipNode(phull, node.children[side], p1f, midf, p1, mid, trace))
		return false;

	// Try going past the node
	if(TR_HullPointContents_ClipNode(phull, node.children[side ^ 1], mid) != CONTENTS_SOLID)
		return TR_RecursiveHullCheck_ClipNode(phull, node.children[side ^ 1], midf, p2f, mid, p2, trace);

	// Never get out of the solid area
	if(trace.flags & FL_TR_ALLSOLID)
		return false;

	if(!side)
	{
		Math::VectorCopy(plane.normal, trace.plane.normal);
		trace.plane.dist = plane.dist;
	}
	else
	{
		Math::VectorScale(plane.normal, -1, trace.plane.normal);
		trace.plane.dist = -plane.dist;
	}

	Int32 contents;
	while(true)
	{
		contents = TR_HullPointContents_ClipNode(phull, phull->firstclipnode, mid);
		if(contents != CONTENTS_SOLID)
			break;

		frac -= 0.1f;
		if(frac < 0.0f)
		{
			trace.fraction = midf;
			Math::VectorCopy(mid, trace.endpos);

			Con_DPrintf("Trace backed up past 0.\n");
			return false;
		}

		midf = p1f + pdiff * frac;

		Math::VectorSubtract(p2, p1, point);
		Math::VectorMA(p1, frac, point, mid);
	}

	if(trace.fraction > midf)
		trace.numhitcontents = 0;

	trace.fraction = midf;
	trace.addHitContents(contents);
	Math::VectorCopy(mid, trace.endpos);
	return false;
}

//=============================================
//
//=============================================
hull_types_t TR_GetHullType( const Vector& mins, const Vector& maxs, hull_types_t hulltype )
{
	// If hulltype is specified, just give out hull
	if(hulltype != HULL_AUTO)
	{
		if(hulltype >= MAX_MAP_HULLS || hulltype < 0 && hulltype != HULL_AUTO)
		{
			Con_Printf("%s - Bogus hull index %d.", __FUNCTION__, hulltype);
			return HULL_NONE;
		}
		else
		{
			// Use the override, if it's valid
			return hulltype;
		}
	}
	else
	{
		// Auto-determine the override
		Vector size;
		Math::VectorSubtract(maxs, mins, size);

		if(size[0] <= 8.0f)
		{
			return HULL_POINT;
		}
		else
		{
			if(size[0] > 36.0f)
				return HULL_LARGE;
			else if(size[2] > 36.0f)
				return HULL_HUMAN;
			else
				return HULL_SMALL;
		}
	}
}

//=============================================
//
//=============================================
void TR_ClipBoxToBrush( const Vector& p1, const Vector& p2, const Vector& mins, const Vector& maxs, trace_t& tr, const brushmodel_t* pbrushmodel, const mbrush_t* pbrush, Int32 brushtypebits )
{
	// Don't bother with empty brushes
	if(!pbrush->numbrushsides)
		return;

	// Check if the brush type is in our mask
	Int32 brushbit = (1<<pbrush->type);
	if(!(brushtypebits & brushbit))
		return;

	Float enterfraction = -1;
	Float leavefraction = 1;

	bool planeback = false;
	const plane_t* poutplane = nullptr;

	bool getout = false;
	bool startout = false;

	for(Uint32 i = 0; i < pbrush->numbrushsides; i++)
	{
		Int32 sideindex = pbrush->firstbrushside + i;
		const mbrushside_t* pside = &pbrushmodel->pbrushsides[sideindex];
		const plane_t* pplane = pside->pplane;

		Vector planenormal = pplane->normal;
		Float planedist = pplane->dist;

		if(pside->planeback)
		{
			Math::VectorScale(planenormal, -1, planenormal);
			planedist = -planedist;
		}

		// TODO: Fix this as Q2 suggested
		Vector offset;
		for(Uint32 j = 0; j < 3; j++)
		{
			if(planenormal[j] < 0)
				offset[j] = maxs[j];
			else
				offset[j] = mins[j];
		}

		Float distance = Math::DotProduct(offset, planenormal);
		distance = planedist - distance;

		// Get distance to start point and check if it's in front of the plane
		Float dist1 = Math::DotProduct(p1, planenormal) - distance;
		Float dist2 = Math::DotProduct(p2, planenormal) - distance;

		if(dist1 > 0)
		{
			// No intersection case
			if(dist2 > 0)
				return;

			startout = true;
		}
		else
		{
			// All points are behind the brush plane
			if(dist2 <= 0)
				continue;

			getout = true;
		}

		if(dist1 > dist2)
		{
			Float fraction = (dist1-HULL_EPSILON);
			if(fraction < 0.0f)
				fraction = 0.0f;
			else
				fraction = fraction / (dist1 - dist2); 

			fraction = clamp(fraction, 0, 1);

			if(fraction > enterfraction)
			{
				enterfraction = fraction;
				poutplane = pplane;
				planeback = pside->planeback;
			}
		}
		else
		{
			Float fraction = (dist1+HULL_EPSILON) / (dist1 - dist2); 
			fraction = clamp(fraction, 0, 1);

			if(fraction < leavefraction)
				leavefraction = fraction;
		}
	}

	if(!startout)
	{
		tr.flags |= FL_TR_STARTSOLID;
		if(!getout)
			tr.flags |= FL_TR_ALLSOLID;

		tr.flags &= ~FL_TR_INOPEN;
	}
	else if(enterfraction < leavefraction && enterfraction > -1 && enterfraction < tr.fraction)
	{
 		if(enterfraction < 0)
			enterfraction = 0;

		// Set fraction and plane
		if(tr.fraction > enterfraction)
			tr.numhitcontents = 0;

		tr.fraction = enterfraction;
		tr.plane = (*poutplane);

		// Reverse if needed
		if(planeback)
		{
			Math::VectorScale(tr.plane.normal, -1, tr.plane.normal);
			tr.plane.dist = -tr.plane.dist;
		}

		tr.addHitContents(pbrush->contents);
	}
}

//=============================================
//
//=============================================
void TR_ClipPointToBrush( const Vector& p1, const Vector& p2, trace_t& tr, const brushmodel_t* pbrushmodel, const mbrush_t* pbrush, Int32 brushtypebits )
{
	// Don't bother with empty brushes
	if(!pbrush->numbrushsides)
		return;

	// Check if the brush type is in our mask
	Int32 brushbit = (1<<pbrush->type);
	if(!(brushtypebits & brushbit))
		return;

	Float enterfraction = -1;
	Float leavefraction = 1;

	bool planeback = false;
	const plane_t* poutplane = nullptr;

	bool getout = false;
	bool startout = false;

	for(Uint32 i = 0; i < pbrush->numbrushsides; i++)
	{
		Int32 sideindex = pbrush->firstbrushside + i;
		const mbrushside_t* pside = &pbrushmodel->pbrushsides[sideindex];
		if(pside->isbevel)
			continue;

		const plane_t* pplane = pside->pplane;
		Vector planenormal = pplane->normal;
		Float planedist = pplane->dist;
		if(pside->planeback)
		{
			Math::VectorScale(planenormal, -1, planenormal);
			planedist = -planedist;
		}

		// Get distance to start point and check if it's in front of the plane
		Float dist1 = Math::DotProduct(p1, planenormal) - planedist;
		if(dist1 > 0)
			startout = true;

		// Get distance to end point and check if it's in front of the plane
		Float dist2 = Math::DotProduct(p2, planenormal) - planedist;
		if(dist2 > 0)
			getout = true;

		if(dist1 > 0)
		{
			// No intersection case
			if(dist2 > 0)
				return;

			startout = true;
		}
		else
		{
			// All points are behind the brush plane
			if(dist2 <= 0)
				continue;

			getout = true;
		}

		if(dist1 > dist2)
		{
			Float fraction = (dist1-HULL_EPSILON);
			if(fraction < 0.0f)
				fraction = 0.0f;
			else
				fraction = fraction / (dist1 - dist2); 

			fraction = clamp(fraction, 0, 1);

			if(fraction > enterfraction)
			{
				enterfraction = fraction;
				poutplane = pplane;
				planeback = pside->planeback;
			}
		}
		else
		{
			Float fraction = dist1 / (dist1 - dist2); 
			fraction = clamp(fraction, 0, 1);

			if(fraction < leavefraction)
				leavefraction = fraction;
		}
	}

	if(!startout)
	{
		tr.flags |= FL_TR_STARTSOLID;
		if(!getout)
			tr.flags |= FL_TR_ALLSOLID;

		tr.flags &= ~FL_TR_INOPEN;
	}
	else if(enterfraction < leavefraction && enterfraction > -1 && enterfraction < tr.fraction)
	{
		if(enterfraction < 0)
			enterfraction = 0;

		// Set fraction and plane
		if(tr.fraction > enterfraction)
			tr.numhitcontents = 0;

		tr.fraction = enterfraction;
		tr.plane = (*poutplane);

		// Reverse if needed
		if(planeback)
		{
			Math::VectorScale(tr.plane.normal, -1, tr.plane.normal);
			tr.plane.dist = -tr.plane.dist;
		}

		tr.addHitContents(pbrush->contents);
	}
}

//=============================================
//
//=============================================
bool TR_TestBoxInBrush( const Vector& position, const Vector& mins, const Vector& maxs, const brushmodel_t* pbrushmodel, const mbrush_t* pbrush, Int32 brushtypebits )
{
	if(!pbrush->numbrushsides)
		return false;

	for(Uint32 i = 0; i < pbrush->numbrushsides; i++)
	{
		Int32 brushindex = pbrush->firstbrushside + i;
		const mbrushside_t* pside = &pbrushmodel->pbrushsides[brushindex];
		const plane_t* pplane = pside->pplane;

		Vector planenormal = pplane->normal;
		Float planedist = pplane->dist;

		if(pside->planeback)
		{
			Math::VectorScale(planenormal, -1, planenormal);
			planedist = -planedist;
		}

		// TODO: Fix this as Q2 suggested
		Vector offset;
		for(Uint32 j = 0; j < 3; j++)
		{
			if(planenormal[j] < 0)
				offset[j] = maxs[j];
			else
				offset[j] = mins[j];
		}

		Float distance = Math::DotProduct(offset, planenormal);
		distance = planedist - distance;

		Float dist1 = Math::DotProduct(position, planenormal) - distance;
		if(dist1 > 0)
			return false;
	}

	return true;
}

//=============================================
//
//=============================================
bool TR_TestPointInBrush( const Vector& position, const brushmodel_t* pbrushmodel, const mbrush_t* pbrush, Int32 brushtypebits )
{
	if(!pbrush->numbrushsides)
		return false;

	for(Uint32 i = 0; i < pbrush->numbrushsides; i++)
	{
		Int32 brushindex = pbrush->firstbrushside + i;
		const mbrushside_t* pside = &pbrushmodel->pbrushsides[brushindex];
		if(pside->isbevel)
			continue;

		const plane_t* pplane = pside->pplane;
		Vector planenormal = pplane->normal;
		Float planedist = pplane->dist;
		if(pside->planeback)
		{
			Math::VectorScale(planenormal, -1, planenormal);
			planedist = -planedist;
		}

		Float dist1 = Math::DotProduct(position, planenormal) - planedist;
		if(dist1 > 0)
			return false;
	}

	return true;
}

//=============================================
//
//=============================================
void TR_TraceBoxToLeaf( const Vector& p1, const Vector& p2, const Vector& mins, const Vector& maxs, trace_t& tr, const brushmodel_t* pbrushmodel, const mleaf_t* pleaf, Int32 brushtypebits )
{
	if(!pleaf->numleafbrushes)
		return;

	if(!pleaf->pleafbrushbvh)
	{
		g_numBrushes = 0;

		for(Uint32 i = 0; i < pleaf->numleafbrushes; i++)
		{
			if(g_numBrushes == g_brushArray.size())
				g_brushArray.resize(g_brushArray.size() + BRUSH_ARRAY_ALLOC_SIZE);

			g_brushArray[i] = pleaf->pfirstleafbrush[i];
			g_numBrushes++;
		}
	}
	else
	{
		// Speed up these tests using BVHs
		pleaf->pleafbrushbvh->GetAABBTraceBrushes(p1, p2, mins, maxs, g_brushArray, g_numBrushes, BRUSH_ARRAY_ALLOC_SIZE);
	}

	for(Uint32 i = 0; i < g_numBrushes; i++)
	{
		mbrush_t* pbrush = g_brushArray[i];

		// Check that the content mask matches
		if(!(brushtypebits & (1<<pbrush->type)))
			continue;

		// See if we already checked this leaf
		if(pbrush->checkcount == g_brushTraceCount)
			continue;

		TR_ClipBoxToBrush(p1, p2, mins, maxs, tr, pbrushmodel, pbrush, brushtypebits);

		// Mark as having been checked
		pbrush->checkcount = g_brushTraceCount;

		if(tr.fraction <= 0.0f)
			return;
	}
}

//=============================================
//
//=============================================
void TR_TracePointToLeaf( const Vector& p1, const Vector& p2, trace_t& tr, const brushmodel_t* pbrushmodel, const mleaf_t* pleaf, Int32 brushtypebits )
{
	if(!pleaf->numleafbrushes)
		return;

	if(!pleaf->pleafbrushbvh)
	{
		g_numBrushes = 0;

		for(Uint32 i = 0; i < pleaf->numleafbrushes; i++)
		{
			if(g_numBrushes == g_brushArray.size())
				g_brushArray.resize(g_brushArray.size() + BRUSH_ARRAY_ALLOC_SIZE);

			g_brushArray[i] = pleaf->pfirstleafbrush[i];
			g_numBrushes++;
		}
	}
	else
	{
		// Speed up these tests using BVHs
		pleaf->pleafbrushbvh->GetLineTraceBrushes(p1, p2, g_brushArray, g_numBrushes, BRUSH_ARRAY_ALLOC_SIZE);
	}

	for(Uint32 i = 0; i < g_numBrushes; i++)
	{
		mbrush_t* pbrush = g_brushArray[i];

		// Check that the content mask matches
		if(!(brushtypebits & (1<<pbrush->type)))
			continue;

		// See if we already checked this leaf
		if(pbrush->checkcount == g_brushTraceCount)
			continue;

		TR_ClipPointToBrush(p1, p2, tr, pbrushmodel, pbrush, brushtypebits);

		// Mark as having been checked
		pbrush->checkcount = g_brushTraceCount;

		if(tr.fraction <= 0.0f)
			return;
	}
}

//=============================================
//
//=============================================
void TR_TestBoxInLeaf( const Vector& position, const Vector& mins, const Vector& maxs, trace_t& tr, const brushmodel_t* pbrushmodel, const mleaf_t* pleaf, Int32 brushtypebits )
{
	Vector absmin, absmax;
	Math::VectorAdd(position, mins, absmin);
	Math::VectorAdd(position, maxs, absmax);

	for(Uint32 i = 0; i < pleaf->numleafbrushes; i++)
	{
		mbrush_t* pbrush = pleaf->pfirstleafbrush[i];

		// Check that the content mask matches
		if(!(brushtypebits & (1<<pbrush->type)))
			continue;

		// See if we already checked this leaf
		if(pbrush->checkcount == g_brushTraceCount)
			continue;

		// Check if we touch the bbox at all
		if(Math::CheckMinsMaxs(absmin, absmax, pbrush->mins, pbrush->maxs))
		{
			pbrush->checkcount = g_brushTraceCount;
			continue;
		}

		// See if it's inside the brush
		if(TR_TestBoxInBrush(position, mins, maxs, pbrushmodel, pbrush, brushtypebits))
		{
			tr.flags |= (FL_TR_STARTSOLID|FL_TR_ALLSOLID);
			tr.fraction = 0;

			tr.addHitContents(pbrush->contents);
		}

		// Mark as having been checked
		pbrush->checkcount = g_brushTraceCount;

		if(!tr.fraction)
			return;
	}
}

//=============================================
//
//=============================================
void TR_TestPointInLeaf( const Vector& position, trace_t& tr, const brushmodel_t* pbrushmodel, const mleaf_t* pleaf, Int32 brushtypebits )
{
	for(Uint32 i = 0; i < pleaf->numleafbrushes; i++)
	{
		mbrush_t* pbrush = pleaf->pfirstleafbrush[i];

		// Check that the content mask matches
		if(!(brushtypebits & (1<<pbrush->type)))
			continue;

		// See if we already checked this leaf
		if(pbrush->checkcount == g_brushTraceCount)
			continue;

		// Check if we touche the bbox at all
		if(!Math::PointInMinsMaxs(position, pbrush->mins, pbrush->maxs))
		{
			pbrush->checkcount = g_brushTraceCount;
			continue;
		}

		// See if it's inside the brush
		if(TR_TestPointInBrush(position, pbrushmodel, pbrush, brushtypebits))
		{
			tr.flags |= (FL_TR_STARTSOLID|FL_TR_ALLSOLID);
			tr.fraction = 0;

			tr.addHitContents(pbrush->contents);
		}

		// Mark as having been checked
		pbrush->checkcount = g_brushTraceCount;

		if(!tr.fraction)
			return;
	}
}

//=============================================
//
//=============================================
void TR_RecursiveHullCheck_BrushBox( mnode_t* pnode, Double p1f, Double p2f, const Vector& p1, const Vector& p2, const Vector& start, const Vector& end, const Vector& mins, const Vector& maxs, const Vector& traceextents, const brushmodel_t* pbrushmodel, Int32 brushtypebits, trace_t& trace, CArray<const mleaf_t*>& listarray, Uint32& listsize )
{
	// Check if we already hit something closer/smaller
	if(trace.fraction <= p1f)
		return;

	// If node contents is negative, we hit a leaf
	if(pnode->contents < 0)
	{
		if(listsize == listarray.size())
			listarray.resize(listarray.size() + LEAF_ARRAY_ALLOC_SIZE);

		const mleaf_t* pleaf = reinterpret_cast<const mleaf_t*>(pnode);
		listarray[listsize] = pleaf;
		listsize++;
		return;
	}

	// Find the points' distances to the plane, and the
	// offset for the size of the box
	const plane_t* pplane = pnode->pplane;

	Double offset;
	Double t1, t2;

	if(pplane->type < 3)
	{
		t1 = p1[pplane->type] - pplane->dist;
		t2 = p2[pplane->type] - pplane->dist;
		offset = traceextents[pplane->type];
	}
	else
	{
		t1 = Math::DotProduct(pplane->normal, p1) - pplane->dist;
		t2 = Math::DotProduct(pplane->normal, p2) - pplane->dist;
		offset = SDL_fabs(traceextents[0] * pplane->normal[0]) + SDL_fabs(traceextents[1] * pplane->normal[1]) + SDL_fabs(traceextents[2] * pplane->normal[2]);
	}

	// See which sides we need to consider
	if(t1 > offset && t2 > offset)
	{
		TR_RecursiveHullCheck_BrushBox(pnode->pchildren[0], p1f, p2f, p1, p2, start, end, mins, maxs, traceextents, pbrushmodel, brushtypebits, trace, listarray, listsize);
		return;
	}
	else if(t1 < -offset && t2 < -offset)
	{
		TR_RecursiveHullCheck_BrushBox(pnode->pchildren[1], p1f, p2f, p1, p2, start, end, mins, maxs, traceextents, pbrushmodel, brushtypebits, trace, listarray, listsize);
		return;
	}

	Int32 side;
	Double frac1, frac2;
	if(t1 < t2)
	{
		Double idist = 1.0f/(t1-t2);
		side = 1;

		frac1 = (t1 - offset - DIST_EPSILON)*idist;
		frac2 = (t1 + offset + DIST_EPSILON)*idist;
	}
	else if(t1 > t2)
	{
		Double idist = 1.0f/(t1-t2);
		side = 0;
		frac1 = (t1 + offset + DIST_EPSILON)*idist;
		frac2 = (t1 - offset - DIST_EPSILON)*idist;
	}
	else
	{
		side = 0;
		frac1 = 1;
		frac2 = 0;
	}

	// Move up the node
	frac1 = clamp(frac1, 0, 1);

	Vector mid;
	Double midf = p1f + (p2f - p1f)*frac1;
	for (Uint32 i = 0; i < 3; i++)
		mid[i] = p1[i] + frac1*(p2[i] - p1[i]);

	TR_RecursiveHullCheck_BrushBox(pnode->pchildren[side], p1f, midf, p1, mid, start, end, mins, maxs, traceextents, pbrushmodel, brushtypebits, trace, listarray, listsize);

	// Go past the node
	frac2 = clamp(frac2, 0, 1);

	midf = p1f + (p2f - p1f)*frac2;
	for (Uint32 i = 0; i < 3; i++)
		mid[i] = p1[i] + frac2*(p2[i] - p1[i]);

	TR_RecursiveHullCheck_BrushBox(pnode->pchildren[side^1], midf, p2f, mid, p2, start, end, mins, maxs, traceextents, pbrushmodel, brushtypebits, trace, listarray, listsize);
}

//=============================================
//
//=============================================
void TR_RecursiveHullCheck_BrushPoint( mnode_t* pnode, Double p1f, Double p2f, const Vector& p1, const Vector& p2, const Vector& start, const Vector& end, const brushmodel_t* pbrushmodel, Int32 brushtypebits, trace_t& trace, CArray<const mleaf_t*>& listarray, Uint32& listsize )
{
	// Check if we already hit something closer/smaller
	if(trace.fraction <= p1f)
		return;

	// If node contents is negative, we hit a leaf
	if(pnode->contents < 0)
	{
		if(listsize == listarray.size())
			listarray.resize(listarray.size() + LEAF_ARRAY_ALLOC_SIZE);

		const mleaf_t* pleaf = reinterpret_cast<const mleaf_t*>(pnode);
		listarray[listsize] = pleaf;
		listsize++;
		return;
	}

	// Find the points' distances to the plane, and the
	// offset for the size of the box
	const plane_t* pplane = pnode->pplane;

	Double t1, t2;
	if(pplane->type < 3)
	{
		t1 = p1[pplane->type] - pplane->dist;
		t2 = p2[pplane->type] - pplane->dist;
	}
	else
	{
		t1 = Math::DotProduct(pplane->normal, p1) - pplane->dist;
		t2 = Math::DotProduct(pplane->normal, p2) - pplane->dist;
	}

	// See which sides we need to consider
	if(t1 > 0 && t2 > 0)
	{
		TR_RecursiveHullCheck_BrushPoint(pnode->pchildren[0], p1f, p2f, p1, p2, start, end, pbrushmodel, brushtypebits, trace, listarray, listsize);
		return;
	}
	else if(t1 < 0 && t2 < 0)
	{
		TR_RecursiveHullCheck_BrushPoint(pnode->pchildren[1], p1f, p2f, p1, p2, start, end, pbrushmodel, brushtypebits, trace, listarray, listsize);
		return;
	}

	Int32 side;
	Double frac1, frac2;
	if(t1 < t2)
	{
		Double idist = 1.0f/(t1-t2);
		side = 1;

		frac1 = (t1 - DIST_EPSILON)*idist;
		frac2 = (t1 + DIST_EPSILON)*idist;
	}
	else if(t1 > t2)
	{
		Double idist = 1.0f/(t1-t2);
		side = 0;
		frac1 = (t1 + DIST_EPSILON)*idist;
		frac2 = (t1 - DIST_EPSILON)*idist;
	}
	else
	{
		side = 0;
		frac1 = 1;
		frac2 = 0;
	}

	// Move up the node
	frac1 = clamp(frac1, 0, 1);

	Vector mid;
	Double midf = p1f + (p2f - p1f)*frac1;
	for (Uint32 i = 0; i < 3; i++)
		mid[i] = p1[i] + frac1*(p2[i] - p1[i]);

	TR_RecursiveHullCheck_BrushPoint(pnode->pchildren[side], p1f, midf, p1, mid, start, end, pbrushmodel, brushtypebits, trace, listarray, listsize);

	// Go past the node
	frac2 = clamp(frac2, 0, 1);

	midf = p1f + (p2f - p1f)*frac2;
	for (Uint32 i = 0; i < 3; i++)
		mid[i] = p1[i] + frac2*(p2[i] - p1[i]);

	TR_RecursiveHullCheck_BrushPoint(pnode->pchildren[side^1], midf, p2f, mid, p2, start, end, pbrushmodel, brushtypebits, trace, listarray, listsize);
}

//=============================================
//
//=============================================
void TR_BoxLeafNumsRecursive( const mnode_t* pstartnode, const Vector& mins, const Vector& maxs, CArray<const mleaf_t*>& listarray, Uint32& listsize, const mnode_t** ptrtopnode )
{
	const mnode_t* pnode = pstartnode;
	while(true)
	{
		// If contents are negative, then we hit a leaf
		if(pnode->contents < 0)
		{
			if(listsize == listarray.size())
				listarray.resize(listarray.size() + LEAF_ARRAY_ALLOC_SIZE);

			const mleaf_t* pleaf = reinterpret_cast<const mleaf_t*>(pnode);
			listarray[listsize] = pleaf;
			listsize++;
			return;
		}
		else
		{
			plane_t* pplane = pnode->pplane;
			Int32 side = Math::BoxOnPlaneSide(mins, maxs, pplane);
			switch(side)
			{
			case SIDE_FRONT:
				pnode = pnode->pchildren[0];
				break;
			case SIDE_BACK:
				pnode = pnode->pchildren[1];
				break;
			default:
				{
					if(ptrtopnode && (*ptrtopnode) == nullptr)
						(*ptrtopnode) = pnode;

					TR_BoxLeafNumsRecursive(pnode->pchildren[0], mins, maxs, listarray, listsize, ptrtopnode);
					pnode = pnode->pchildren[1];
				}
				break;
			}
		}
	}
}

//=============================================
//
//=============================================
Uint32 TR_BoxLeafNums( const Vector& mins, const Vector& maxs, const brushmodel_t* pbrushmodel, Int32 brushtypebits, CArray<const mleaf_t*>& listarray, Uint32& listsize, const mnode_t** ptrtopnode )
{
	// Reset topnode ptr if present
	if(ptrtopnode)
		(*ptrtopnode) = nullptr;

	TR_BoxLeafNumsRecursive(pbrushmodel->pnodes, mins, maxs, listarray, listsize, ptrtopnode);
	return listsize;
}

//=============================================
//
//=============================================
const mleaf_t* TR_PointLeaf( const mnode_t* pheadnode, const Vector& position )
{
	const mnode_t* pnode = pheadnode;
	while(pnode->contents >= 0)
	{
		Float distance;
		const plane_t* pplane = pnode->pplane;
		if(pplane->type < 3)
			distance = position[pplane->type] - pplane->dist;
		else
			distance = Math::DotProduct(pplane->normal, position) - pplane->dist;

		if(distance < 0)
			pnode = pnode->pchildren[1];
		else
			pnode = pnode->pchildren[0];
	}

	const mleaf_t* pleaf = reinterpret_cast<const mleaf_t*>(pnode);
	return pleaf;
}

//=============================================
//
//=============================================
Int32 TR_PointContents_Brush( const brushmodel_t* pbrushmodel, const Vector& position )
{
	const mleaf_t* pleaf = TR_PointLeaf(&pbrushmodel->pnodes[pbrushmodel->headnodeindex], position);
	if(!pleaf)
		return CONTENTS_EMPTY;
	else
		return pleaf->contents;
}

//=============================================
//
//=============================================
Int32 TR_HullPointContents_Brush( const brushmodel_t* pbrushmodel, const Vector& position, const Vector& mins, const Vector& maxs, Int32 brushtypebits )
{
	g_numLeafs = 0;

	Vector extents;
	Math::VectorSubtract(maxs, mins, extents);
	bool ispointcheck = extents.IsZero() ? true : false;

	if(ispointcheck)
	{
		const mleaf_t* pleaf = TR_PointLeaf(&pbrushmodel->pnodes[pbrushmodel->headnodeindex], position);
		if(pleaf)
		{
			g_leafArray[g_numLeafs] = pleaf;
			g_numLeafs++;
		}
	}
	else
	{
		Vector absmin, absmax;
		Math::VectorAdd(mins, position, absmin);
		Math::VectorAdd(maxs, position, absmax);

		TR_BoxLeafNumsRecursive(&pbrushmodel->pnodes[pbrushmodel->headnodeindex], absmin, absmax, g_leafArray, g_numLeafs, nullptr);
	}

	// If we hit no leafs, return empty
	if(!g_numLeafs)
		return CONTENTS_EMPTY;

	Int32 bestContents = CONTENTS_EMPTY;
	Int32 bestRank = -1;

	g_brushTraceCount++;

	for(Uint32 i = 0; i < g_numLeafs; i++)
	{
		const mleaf_t* pleaf = g_leafArray[i];
		if(pleaf->contents == CONTENTS_SOLID)
			return CONTENTS_SOLID;

		if(!pleaf->numleafbrushes)
			continue;
		
		for(Uint32 j = 0; j < pleaf->numleafbrushes; j++)
		{
			mbrush_t* pbrush = pleaf->pfirstleafbrush[j];
			if(!(brushtypebits & (1<<pbrush->type)))
				continue;

			// See if we already checked this leaf
			if(pbrush->checkcount == g_brushTraceCount)
				continue;

			// Test that the point/box is actually inside the brush
			bool result;
			if(ispointcheck)
				result = TR_TestPointInBrush(position, pbrushmodel, pbrush, brushtypebits);
			else
				result = TR_TestBoxInBrush(position, mins, maxs, pbrushmodel, pbrush, brushtypebits);
				
			if(result)
			{
				Int32 contentsRank = RankForContents(pbrush->contents);
				if(bestRank == -1 || contentsRank > bestRank)
				{
					bestContents = pbrush->contents;
					bestRank = contentsRank;
				}
			}
			
			pbrush->checkcount = g_brushTraceCount;
		}
	}

	return bestContents;
}

//=============================================
//
//=============================================
void TR_BrushBoxTrace( const Vector& start, const Vector& end, const Vector& mins, const Vector& maxs, const mnode_t* pheadnode, const brushmodel_t* pbrushmodel, Int32 brushtypebits, trace_t& outtrace )
{
	// Initialize trace, in case of brush based
	// collisions the opposite is true: We start
	// in open, then ALLSOLID/STARTSOLID is set
	outtrace.fraction = 1.0;
	outtrace.flags &= ~FL_TR_ALLSOLID;
	outtrace.flags |= FL_TR_INOPEN;
	outtrace.numhitcontents = 0;

	// Increment trace counter
	g_brushTraceCount++;
	g_numLeafs = 0;

	Vector extents;
	bool ispointcheck = (mins.IsZero() && maxs.IsZero()) ? true : false;
	if(!ispointcheck)
	{
		for(Uint32 i = 0; i < 3; i++)
			extents[i] = -mins[i] > maxs[i] ? -mins[i] : maxs[i];
	}
	else
		extents.Clear();

	Vector delta;
	Math::VectorSubtract(end, start, delta);

	if(delta.IsZero())
	{
		if(ispointcheck)
		{
			const mleaf_t* pleaf = TR_PointLeaf(&pbrushmodel->pnodes[pbrushmodel->headnodeindex], start);
			if(pleaf)
			{
				g_leafArray[g_numLeafs] = pleaf;
				g_numLeafs++;
			}
		}
		else
		{
			Vector c1, c2;
			Math::VectorAdd(start, mins, c1);
			Math::VectorAdd(start, maxs, c2);

			for(Uint32 i = 0; i < 3; i++)
			{
				c1[i] -= 1;
				c2[i] += 1;
			}

			TR_BoxLeafNumsRecursive(&pbrushmodel->pnodes[pbrushmodel->headnodeindex], c1, c2, g_leafArray, g_numLeafs, nullptr);
		}

		// Check if we are only touching solid leafs without brushes
		bool onlySolidLeafs = true;
		for(Uint32 i = 0; i < g_numLeafs; i++)
		{
			const mleaf_t* pleaf = g_leafArray[i];
			if(pleaf->contents != CONTENTS_SOLID)
			{
				onlySolidLeafs = false;
				break;
			}
		}

		if(onlySolidLeafs)
		{
			// If we only visited solid leafs, then we're fully in a solid
			outtrace.flags |= (FL_TR_STARTSOLID|FL_TR_ALLSOLID);
			outtrace.flags &= ~FL_TR_INOPEN;
			outtrace.fraction = 0;
		}
		else
		{
			for(Uint32 i = 0; i < g_numLeafs; i++)
			{
				if(ispointcheck)
					TR_TestPointInLeaf(start, outtrace, pbrushmodel, g_leafArray[i], brushtypebits);
				else
					TR_TestBoxInLeaf(start, mins, maxs, outtrace, pbrushmodel, g_leafArray[i], brushtypebits);

				if(outtrace.allSolid())
					break;
			}
		}

		outtrace.endpos = start;
	}
	else
	{
		Int32 headnode = pbrushmodel->headnodeindex;
		if(ispointcheck)
			TR_RecursiveHullCheck_BrushPoint(&pbrushmodel->pnodes[headnode], 0, 1, start, end, start, end, pbrushmodel, brushtypebits, outtrace, g_leafArray, g_numLeafs);
		else
			TR_RecursiveHullCheck_BrushBox(&pbrushmodel->pnodes[headnode], 0, 1, start, end, start, end, mins, maxs, extents, pbrushmodel, brushtypebits, outtrace, g_leafArray, g_numLeafs);

		// Check if we are only touching solid leafs without brushes
		bool onlySolidLeafs = true;
		for(Uint32 i = 0; i < g_numLeafs; i++)
		{
			const mleaf_t* pleaf = g_leafArray[i];
			if(pleaf->contents != CONTENTS_SOLID)
			{
				onlySolidLeafs = false;
				break;
			}
		}

		if(onlySolidLeafs)
		{
			// If we only visited solid leafs, then we're fully in a solid
			outtrace.flags |= (FL_TR_STARTSOLID|FL_TR_ALLSOLID);
			outtrace.flags &= ~FL_TR_INOPEN;
			outtrace.fraction = 0;
		}
		else
		{
			// Do collision test as normal
			for(Uint32 i = 0; i < g_numLeafs; i++)
			{
				const mleaf_t* pleaf = g_leafArray[i];

				if(ispointcheck)
					TR_TracePointToLeaf(start, end, outtrace, pbrushmodel, pleaf, brushtypebits);
				else
					TR_TraceBoxToLeaf(start, end, mins, maxs, outtrace, pbrushmodel, pleaf, brushtypebits);
			}
		}

		if(outtrace.fraction == 1.0)
			outtrace.endpos = end;
		else if(outtrace.fraction > 0)
			Math::VectorMA(start, outtrace.fraction, delta, outtrace.endpos);
		else
			outtrace.endpos = start;
	}
}

//=============================================
//
//=============================================
void TR_TraceAgainstEntity( const entity_state_t& entity, const cache_model_t* pmodel, entity_vbmhulldata_t* pvbmhulldata, const Vector& start, const Vector& end, hull_types_t hulltype, Int32 flags, const Vector& mins, const Vector& maxs, trace_t& trace )
{
	// Do an inexpensive check first, but only if either the entity is mod_brush, or mod_vbm AND has hitboxes specified to be used
	if(pmodel->cacheindex != WORLD_MODEL_INDEX && (pmodel->type == MOD_BRUSH || pmodel->type == MOD_VBM && (flags & FL_TRACE_HITBOXES || pmodel->flags & STUDIO_MF_TRACE_HITBOX)))
	{
		if(!TR_TracelineBBoxCheck(entity, pmodel, start, end, mins, maxs))
			return;
	}

	Vector lmins, lmaxs;
	if(hulltype == HULL_POINT || hulltype == HULL_AUTO && mins.IsZero() && maxs.IsZero())
	{
		// We don't use mins/maxs in point traces
		lmins.Clear();
		lmaxs.Clear();
	}
	else
	{
		// Don't trace against clipeconomy/func_detail
		if(entity.flags & FL_POINTHULL_ONLY)
			return;

		lmins = mins;
		lmaxs = maxs;
	}

	// Set this only if we passed the basic check
	trace.flags |= FL_TR_ALLSOLID;

	bool rotateResultBack = false;
	collision_method_t method = TR_GetIdealCollisionMethod(entity, pmodel, flags, hulltype);
	switch(method)
	{
	case CM_NO_VALID_METHOD:
		{
			Con_EPrintf("%s - No valid collisions for entity %d.\n", __FUNCTION__, entity.entindex);
			return;
		}
		break;
	case CM_BRUSH_COLLISIONS:
		{
			// Transform to entity local space
			Vector start_l, end_l;
			Math::VectorSubtract(start, entity.origin, start_l);
			Math::VectorSubtract(end, entity.origin, end_l);

			// Rotate to entity local space
			if(!entity.angles.IsZero())
			{
				Math::RotateToEntitySpace(entity.angles, start_l);
				Math::RotateToEntitySpace(entity.angles, end_l);
				rotateResultBack = true;
			}

			// Use brush collision hulls
			const brushmodel_t* pbrushmodel = pmodel->getBrushmodel();
			const mnode_t* pheadnode = &pbrushmodel->pnodes[pbrushmodel->headnodeindex];

			// Mark relevant brush types to search
			Int32 brushtypebits = (1<<BRUSHTYPE_NORMAL);

			if(hulltype != HULL_POINT)
				brushtypebits |= (1<<BRUSHTYPE_CLIP_BRUSH);

			if(flags & FL_TRACE_SKYBRUSHES)
				brushtypebits |= (1<<BRUSHTYPE_SKY);

			TR_BrushBoxTrace(start_l, end_l, lmins, lmaxs, pheadnode, pbrushmodel, brushtypebits, trace);
		}
		break;
	case CM_VBM_HITBOX_HULLS:
		{
			// Retrieve pointer to array
			const CArray<vbmhitboxhull_t>* pvbmhulls = TR_VBMGetHulls(pvbmhulldata, mins, maxs, hulltype, flags, nullptr);
			if(pvbmhulls->empty())
			{
				trace.flags &= ~FL_TR_ALLSOLID;
				return;
			}

			// Trace against VBM hulls
			TR_VBMHullCheck(pvbmhulls, start, end, trace);
		}
		break;
	case CM_MCD_COLLISIONS:
		{
			// Transform to entity local space
			Vector start_l, end_l;
			Math::VectorSubtract(start, entity.origin, start_l);
			Math::VectorSubtract(end, entity.origin, end_l);

			// Rotate to entity local space
			if(!entity.angles.IsZero())
			{
				Math::RotateToEntitySpace(entity.angles, start_l);
				Math::RotateToEntitySpace(entity.angles, end_l);
				rotateResultBack = true;
			}

			// Perform trace
			const vbmcache_t* pvbmcache = pmodel->getVBMCache();
			const mcdheader_t* pmcdheader = pvbmcache->pmcdheader;
			g_mcdTrace.TraceLineAABB(start_l, end_l, lmins, lmaxs, pmcdheader, entity.body, trace);
		}
		break;
	default:
	case CM_CLIPNODE_COLLISIONS:
		{
			// Get hull for entity
			hull_types_t realhulltype = TR_GetHullType(mins, maxs, hulltype);
			const brushmodel_t* pbrushmodel = pmodel->getBrushmodel();
			const hull_t* phull = &pbrushmodel->hulls[realhulltype];

			Vector offset;
			Math::VectorSubtract(phull->clipmins, mins, offset);
			Math::VectorAdd(offset, entity.origin, offset);

			// Transform to entity local space
			Vector start_l, end_l;
			Math::VectorSubtract(start, offset, start_l);
			Math::VectorSubtract(end, offset, end_l);

			// Rotate to entity local space
			if(!entity.angles.IsZero())
			{
				Math::RotateToEntitySpace(entity.angles, start_l);
				Math::RotateToEntitySpace(entity.angles, end_l);
				rotateResultBack = true;
			}

			// Regular trace with clipnode hull
			TR_RecursiveHullCheck_ClipNode(phull, phull->firstclipnode, 0.0f, 1.0f, start_l, end_l, trace);
		}
		break;
	case CM_BBOX_COLLISIONS:
		{	
			Vector hullmins, hullmaxs;
			Math::VectorSubtract(entity.mins, maxs, hullmins);
			Math::VectorSubtract(entity.maxs, mins, hullmaxs);
			const hull_t* phull = TR_HullForBox(hullmins, hullmaxs);

			// Transform to entity local space
			Vector start_l, end_l;
			Math::VectorSubtract(start, entity.origin, start_l);
			Math::VectorSubtract(end, entity.origin, end_l);

			// Reuse clipnode trace logic for this purpose
			TR_RecursiveHullCheck_ClipNode(phull, phull->firstclipnode, 0.0f, 1.0f, start_l, end_l, trace);
		}
		break;
	}

	// If all solid was set, make sure start solid is set too
	if(trace.flags & FL_TR_ALLSOLID)
		trace.flags |= FL_TR_STARTSOLID;

	// If starting in solid, set fraction to 0
	if(trace.flags & FL_TR_STARTSOLID)
		trace.fraction = 0;

	if(trace.fraction != 1.0f)
	{
		// Check if we need to rotate it back from entity space
		if(rotateResultBack)
			Math::RotateFromEntitySpace(entity.angles, trace.plane.normal);

		// Set endpos
		Vector point;
		Math::VectorSubtract(end, start, point);
		Math::VectorMA(start, trace.fraction, point, trace.endpos);
	}

	// If we hit the entity, then set entindex
	if(trace.fraction < 1.0f || (trace.flags & (FL_TR_ALLSOLID|FL_TR_STARTSOLID)))
		trace.hitentity = entity.entindex;
}

//=============================================
//
//=============================================
void TR_PlayerTraceSingleEntity( const entity_state_t& entity, entity_vbmhulldata_t* pvbmhulldata, const Vector& start, const Vector& end, hull_types_t hulltype, Int32 traceflags, const Vector& player_mins, const Vector& player_maxs, trace_t& outtrace )
{
	trace_t trace;
	trace.endpos = end;
	trace.flags = FL_TR_ALLSOLID;
	trace.fraction = 1.0f;
	trace.numhitcontents = 0;

	const cache_model_t* pmodel = Cache_GetModel(entity.modelindex);
	if(!pmodel)
		return;

	// Only supports brush ents for now
	if(pmodel->type != MOD_BRUSH && pmodel->type != MOD_VBM)
		return;
	
	// Verify that client supplied proper hulltype
	if(hulltype == HULL_AUTO || hulltype == HULL_BBOX || hulltype == HULL_NONE)
	{
		Con_EPrintf("%s - Invalid hull type '%d' specified.\n", __FUNCTION__, (int)hulltype);
		return;
	}

	TR_TraceAgainstEntity(entity, pmodel, pvbmhulldata, start, end, hulltype, traceflags, player_mins, player_maxs, trace);

	// Only modify trace if fraction was less
	if(trace.fraction < outtrace.fraction)
	{
		outtrace = trace;
		trace.hitentity = entity.entindex;
	}
}

//=============================================
//
//=============================================
bool TR_TracelineBBoxCheck( const entity_state_t& entity, const cache_model_t* pcachemodel, const Vector& start, const Vector& end, const Vector& mins, const Vector& maxs )
{
	// Because of how hull expansion works, we need to expand the hull of the rotated brush entity
	// by the collision hull BEFORE we rotate those mins/maxs, otherwise the bounding box will
	// not represent the actual expansion.

	bool forcePointCheck = false;
	Vector entitymins, entitymaxs;
	if(pcachemodel->type == MOD_BRUSH && !entity.angles.IsZero())
	{
		Math::VectorAdd(pcachemodel->mins, mins, entitymins);
		Math::VectorAdd(pcachemodel->maxs, maxs, entitymaxs);

		Vector rotatedmins, rotatedmaxs;
		Math::RotateMinsMaxsByAngle(entitymins, entitymaxs, entity.angles, rotatedmins, rotatedmaxs);

		Math::VectorSubtract(rotatedmins, Vector(1, 1, 1), rotatedmins);
		Math::VectorAdd(rotatedmaxs, Vector(1, 1, 1), rotatedmaxs);

		Math::VectorAdd(rotatedmins, entity.origin, entitymins);
		Math::VectorAdd(rotatedmaxs, entity.origin, entitymaxs);

		forcePointCheck = true;
	}
	else
	{
		Math::VectorCopy(entity.absmin, entitymins);
		Math::VectorCopy(entity.absmax, entitymaxs);
	}

	if(forcePointCheck || mins.IsZero() && maxs.IsZero())
	{
		// Point traces are simple
		Vector direction;
		Math::VectorSubtract(end, start, direction);
		direction.Normalize();

		// Cheap test
		if(!CollisionShared::IntersectBBoxPoint(start, end, entitymins, entitymaxs, direction))
			return false;

		// More expensive test
		return CollisionShared::LineIntersectsBounds(&start, &end, entitymins, entitymaxs);
	}
	else
	{
		Vector extents = (maxs - mins) * 0.5;
		if((start - end).Length() < ON_EPSILON)
		{
			// This is an intersection check, which is much simpler than the swept AABB check
			return CollisionShared::IntersectBBoxAABB(start, entitymins, entitymaxs, extents);
		}
		else
		{
			// This one is a little bit more complex
			return CollisionShared::IntersectBBoxSweptAABB(start, end, entitymins, entitymaxs, extents);
		}
	}
}

//=============================================
//
//=============================================
collision_method_t TR_GetIdealCollisionMethod( const entity_state_t& entity, const cache_model_t* pmodel, Int32 flags, hull_types_t hulltype )
{
	if(pmodel->type != MOD_BRUSH && pmodel->type != MOD_VBM)
		return CM_NO_VALID_METHOD;
	else if(pmodel->cacheflags & CACHE_FL_HAS_MCD)
		return CM_MCD_COLLISIONS;
	else if(pmodel->type == MOD_VBM && ((flags & FL_TRACE_HITBOXES) || (pmodel->flags & STUDIO_MF_TRACE_HITBOX)) && !(entity.flags & FL_NO_HITBOX_TRACE))
		return CM_VBM_HITBOX_HULLS;
	else if((pmodel->flags & CACHE_FL_HAS_BRUSH_COLLISIONS) && !(flags & FL_TRACE_FORCE_CLIPNODES))
		return CM_BRUSH_COLLISIONS;
	else if(pmodel->type == MOD_BRUSH)
		return CM_CLIPNODE_COLLISIONS;
	else
		return CM_BBOX_COLLISIONS;
}