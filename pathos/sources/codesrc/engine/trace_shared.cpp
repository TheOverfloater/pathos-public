/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

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

// Box hull used by collision detection
static mclipnode_t g_boxClipnodes[6];
// Box planes used by collision detection
static plane_t g_boxPlanes[6];
// Hull used by collision detection
static hull_t g_boxHull;

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
	if(pmodel->type != MOD_BRUSH)
	{
		Con_Printf("%s - Called on entity %d with model that is not a brush model.\n", __FUNCTION__, entity.entindex);
		return nullptr;
	}

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

	return nullptr;
}

//=============================================
//
//=============================================
Int32 TR_HullPointContents( const hull_t* phull, Int32 clipnodeidx, const Vector& position )
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
bool TR_RecursiveHullCheck( const hull_t* phull, Int32 clipnodeidx, Double p1f, Double p2f, const Vector& p1, const Vector& p2, trace_t& trace )
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
		return TR_RecursiveHullCheck(phull, node.children[0], p1f, p2f, p1, p2, trace);
	if(t1 < 0 && t2 < 0)
		return TR_RecursiveHullCheck(phull, node.children[1], p1f, p2f, p1, p2, trace);

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
	if(!TR_RecursiveHullCheck(phull, node.children[side], p1f, midf, p1, mid, trace))
		return false;

	// Try going past the node
	if(TR_HullPointContents(phull, node.children[side ^ 1], mid) != CONTENTS_SOLID)
		return TR_RecursiveHullCheck(phull, node.children[side ^ 1], midf, p2f, mid, p2, trace);

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

	while(TR_HullPointContents(phull, phull->firstclipnode, mid) == CONTENTS_SOLID)
	{
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

	trace.fraction = midf;
	Math::VectorCopy(mid, trace.endpos);
	return false;
}

//=============================================
//
//=============================================
void TR_PlayerTraceSingleEntity( const entity_state_t& entity, entity_vbmhulldata_t* pvbmhulldata, const Vector& start, const Vector& end, hull_types_t hulltype, Int32 traceflags, const Vector& player_mins, const Vector& player_maxs, trace_t& outtrace )
{
	Vector offset;
	Vector mins;
	Vector maxs;
	Vector start_l;
	Vector end_l;

	trace_t trace;

	const cache_model_t* pcache = Cache_GetModel(entity.modelindex);
	if(!pcache)
		return;

	// Only supports brush ents for now
	if(pcache->type != MOD_BRUSH && pcache->type != MOD_VBM)
		return;
	
	const hull_t* phull = nullptr;
	const CArray<vbmhitboxhull_t>* pvbmhulls = nullptr;

	if(pcache->type == MOD_BRUSH)
	{
		const brushmodel_t* pmodel = pcache->getBrushmodel();
		phull = &pmodel->hulls[hulltype];

		Math::VectorSubtract(phull->clipmins, player_mins, offset);
		Math::VectorAdd(offset, entity.origin, offset);
	}
	else
	{
		if(pvbmhulldata && (traceflags & FL_TRACE_HITBOXES || pcache->flags & STUDIO_MF_TRACE_HITBOX))
		{
			pvbmhulls = TR_VBMGetHulls(pvbmhulldata, player_mins, player_maxs, hulltype, traceflags, &offset);
		}
		else
		{
			// Offset is the origin
			Math::VectorCopy(entity.origin, offset);
			Math::VectorSubtract(entity.mins, player_maxs, mins);
			Math::VectorSubtract(entity.maxs, player_mins, maxs);
			phull = TR_HullForBox(mins, maxs);
		}
	}

	// Transform to entity space
	Math::VectorSubtract(start, offset, start_l);
	Math::VectorSubtract(end, offset, end_l);

	if(entity.solid == SOLID_BSP && !entity.angles.IsZero())
	{
		Math::RotateToEntitySpace(entity.angles, start_l);
		Math::RotateToEntitySpace(entity.angles, end_l);
	}

	trace.endpos = end;
	trace.flags = FL_TR_ALLSOLID;
	trace.fraction = 1.0f;

	if(!pvbmhulls)
	{
		// Only a single hull to trace against
		TR_RecursiveHullCheck(phull, phull->firstclipnode, 0.0, 1.0, start_l, end_l, trace);
	}
	else
	{
		if(pvbmhulls->empty())
		{
			trace.flags &= ~FL_TR_ALLSOLID;
			return;
		}

		// Trace against VBM
		TR_VBMHullCheck(pvbmhulls, start_l, end_l, trace);
	}

	if(trace.flags & FL_TR_ALLSOLID)
	{
		trace.flags |= FL_TR_STARTSOLID;
		trace.hitentity = entity.entindex;
	}

	if(trace.flags & FL_TR_STARTSOLID)
		trace.fraction = 0.0;

	if(trace.fraction != 1.0)
	{
		if(entity.solid == SOLID_BSP && !entity.angles.IsZero())
			Math::RotateFromEntitySpace(entity.angles, trace.plane.normal);

		Vector point;
		Math::VectorSubtract(end, start, point);
		Math::VectorMA(start, trace.fraction, point, trace.endpos);

		trace.hitentity = entity.entindex;
	}

	if(trace.fraction < outtrace.fraction)
	{
		memcpy(&outtrace, &trace, sizeof(trace_t));
		trace.hitentity = entity.entindex;
	}
}