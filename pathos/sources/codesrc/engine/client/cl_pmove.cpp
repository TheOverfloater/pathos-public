/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "com_math.h"
#include "entity_state.h"
#include "common.h"
#include "cl_main.h"
#include "constants.h"
#include "brushmodel.h"
#include "modelcache.h"
#include "trace.h"
#include "trace_shared.h"
#include "cl_utils.h"
#include "enginestate.h"
#include "system.h"
#include "cl_pmove.h"
#include "vbmtrace.h"

//=============================================
//
//=============================================
Int32 CL_TestPlayerPosition( hull_types_t hulltype, Int32 flags, const class Vector& position )
{
	trace_t trace;
	CL_PlayerTrace(position, position, flags, hulltype, NO_ENTITY_INDEX, trace);

	return trace.hitentity;
}

//=============================================
//
//=============================================
Int32 CL_PointContents( const Vector& position, Int32* truecontents )
{
	Int32 contents = TR_HullPointContents(&ens.pworld->hulls[0], 0, position);
	if(truecontents)
		*truecontents = contents;
	
	if(contents != CONTENTS_EMPTY)
		return contents;

	// Get localplayer
	cl_entity_t* pplayer = CL_GetLocalPlayer();
	if(!pplayer)
		return contents;

	Vector mins, maxs;
	for(Int32 i = 1; i < cls.numentities; i++)
	{
		cl_entity_t* pentity = CL_GetEntityByIndex(i);
		if(!pentity)
			break;

		if(!pentity->curstate.modelindex)
			continue;

		if(pentity->curstate.msg_num != pplayer->curstate.msg_num)
			continue;

		Math::VectorAdd(pentity->curstate.origin, pentity->curstate.mins, mins);
		Math::VectorAdd(pentity->curstate.origin, pentity->curstate.maxs, maxs);

		Math::VectorSubtract(mins, Vector(1, 1, 1), mins);
		Math::VectorAdd(maxs, Vector(1, 1, 1), maxs);

		if(!Math::PointInMinsMaxs(position, mins, maxs))
			continue;

		const cache_model_t* pmodel = Cache_GetModel(pentity->curstate.modelindex);
		if(!pmodel || pmodel->type != MOD_BRUSH)
			continue;

		Vector lposition = position;
		if(!pentity->curstate.origin.IsZero())
			Math::VectorSubtract(lposition, pentity->curstate.origin, lposition);

		if(!pentity->curstate.angles.IsZero())
			Math::RotateToEntitySpace(pentity->curstate.angles, lposition);

		const brushmodel_t* pbrushmodel = pmodel->getBrushmodel();
		contents = TR_HullPointContents(&pbrushmodel->hulls[0], pbrushmodel->hulls[0].firstclipnode, lposition);
		if(contents != CONTENTS_EMPTY)
		{
			if(pentity->curstate.solid == SOLID_NOT && pentity->curstate.skin <= CONTENTS_WATER && pentity->curstate.skin >= CONTENTS_LAVA)
				return pentity->curstate.skin;
			else
				return contents;
		}
	}

	return CONTENTS_EMPTY;
}

//=============================================
//
//=============================================
Int32 CL_PointContents( cl_entity_t* pentity, const Vector& position )
{
	if(!pentity)
	{
		// Default to world entity
		pentity = CL_GetEntityByIndex(WORLDSPAWN_ENTITY_INDEX);
	}

	if(!pentity->pmodel || pentity->pmodel->type != MOD_BRUSH)
		return CONTENTS_EMPTY;

	Vector lposition = position;
	if(!pentity->curstate.origin.IsZero())
		Math::VectorSubtract(lposition, pentity->curstate.origin, lposition);

	if(!pentity->curstate.angles.IsZero())
		Math::RotateToEntitySpace(pentity->curstate.angles, lposition);

	const brushmodel_t* pbrushmodel = pentity->pmodel->getBrushmodel();
	const hull_t* phull = &pbrushmodel->hulls[0];

	return TR_HullPointContents(phull, phull->firstclipnode, lposition);
}

//=============================================
//
//=============================================
Int32 CL_TruePointContents( const Vector& position )
{
	return CL_PointContents(position, nullptr);
}

//=============================================
//
//=============================================
void CL_PlayerTrace( const Vector& start, const Vector& end, Int32 traceflags, hull_types_t hulltype, Int32 ignore_ent, trace_t& trace )
{
	if(hulltype < 0 || hulltype >= MAX_MAP_HULLS)
	{
		Con_EPrintf("%s - Bogus hull index %d.\n", __FUNCTION__, hulltype);
		return;
	}

	Vector mins;
	Vector maxs;

	// set basics
	trace.fraction = 1.0;
	trace.hitentity = NO_ENTITY_INDEX;
	trace.endpos = end;

	// Get localplayer
	cl_entity_t* pplayer = CL_GetLocalPlayer();
	if(!pplayer)
		return;

	// trace against world first
	cl_entity_t* pworld = CL_GetEntityByIndex(WORLDSPAWN_ENTITY_INDEX);
	TR_PlayerTraceSingleEntity(pworld->curstate, nullptr, start, end, hulltype, traceflags, cls.pminfo.player_mins[hulltype], cls.pminfo.player_maxs[hulltype], trace);

	// Trace against entities if applicable
	if(!(traceflags & FL_TRACE_WORLD_ONLY))
	{
		Vector tracemins, tracemaxs;
		TR_MoveBoundsPoint(start, end, tracemins, tracemaxs);
		Math::VectorAdd(tracemins, cls.pminfo.player_mins[hulltype], tracemins);
		Math::VectorAdd(tracemaxs, cls.pminfo.player_maxs[hulltype], tracemaxs);

		for(Int32 i = 1; i < cls.numentities; i++)
		{
			if(i == ignore_ent || i == pplayer->entindex)
				continue;

			// Get pointer to edict
			cl_entity_t* pentity = CL_GetEntityByIndex(i);
			if(!pentity)
				continue;

			if(!pentity->pmodel)
				continue;

			if((traceflags & FL_TRACE_NO_MODELS) && pentity->pmodel->type == MOD_VBM)
				continue;

			if((traceflags & FL_TRACE_NO_NPCS) && pentity->curstate.flags & FL_NPC)
				continue;

			// Do not consider triggers
			if(pentity->curstate.solid == SOLID_TRIGGER)
				continue;

			if(pentity->curstate.solid == SOLID_NOT)
				continue;

			if(pentity->curstate.flags & FL_NPC_CLIP && pentity->curstate.solid == SOLID_BSP)
				continue;

			if(pentity->curstate.flags & FL_CLIENT && pentity->curstate.health <= 0)
				continue;

			if(pentity->curstate.flags & FL_PMOVE_IGNORE)
				continue;

			if(pentity->curstate.owner == pplayer->entindex)
				continue;

			// Check for specific flags
			if(traceflags & FL_TRACE_NO_TRANS && pentity->curstate.rendermode != RENDER_NORMAL)
				continue;

			// Do not hit corpses unless specified so
			if(!(traceflags & FL_TRACE_HIT_CORPSES) && (pentity->curstate.flags & FL_DEAD || pentity->curstate.deadstate != DEADSTATE_NONE))
				continue;

			// Don't test against if it's no longer in the packet list
			if(pplayer->curstate.msg_num != pentity->curstate.msg_num)
				continue;

			if(Cache_GetModelType(*pentity->pmodel) == MOD_BRUSH && !pentity->curstate.angles.IsZero())
			{
				for(Uint32 j = 0; j < 3; j++)
				{
					mins[j] = pentity->curstate.origin[j] - pentity->pmodel->radius;
					maxs[j] = pentity->curstate.origin[j] + pentity->pmodel->radius;
				}
			}
			else
			{
				Math::VectorAdd(pentity->curstate.origin, pentity->curstate.mins, mins);
				Math::VectorAdd(pentity->curstate.origin, pentity->curstate.maxs, maxs);
			}

			Math::VectorAdd(mins, cls.pminfo.player_mins[hulltype], mins);
			Math::VectorAdd(maxs, cls.pminfo.player_maxs[hulltype], maxs);

			// Optimize on traceline
			if(Math::CheckMinsMaxs(tracemins, tracemaxs, mins, maxs))
				continue;

			// Update VBM hull data if needed
			if(pentity->pmodel->type == MOD_VBM && (pentity->pmodel->flags & STUDIO_MF_TRACE_HITBOX || traceflags & FL_TRACE_HITBOXES))
				TR_VBMSetHullInfo(pentity->pvbmhulldata, pentity->pmodel, cls.pminfo.player_mins[hulltype], cls.pminfo.player_maxs[hulltype], pentity->curstate, cls.cl_time, hulltype);

			TR_PlayerTraceSingleEntity(pentity->curstate, pentity->pvbmhulldata, start, end, hulltype, traceflags, cls.pminfo.player_mins[hulltype], cls.pminfo.player_maxs[hulltype], trace);
		}
	}
}

//=============================================
//
//=============================================
void CL_TraceLine( const Vector& start, const Vector& end, Int32 traceflags, hull_types_t hulltype, Int32 ignore_ent, trace_t& trace )
{
	CL_PlayerTrace(start, end, traceflags, hulltype, ignore_ent, trace);
}

//=============================================
//
//=============================================
const hull_t* CL_HullForBSP( Int32 entity, hull_types_t hulltype, Vector* poffset )
{
	if(hulltype >= MAX_MAP_HULLS || hulltype < 0)
	{
		Con_EPrintf("%s - Bogus hull index %d.\n", __FUNCTION__, hulltype);
		return 0;
	}

	cl_entity_t* pentity = CL_GetEntityByIndex(entity);
	if(!pentity)
		return nullptr;

	const cache_model_t* pcache = Cache_GetModel(pentity->curstate.modelindex);
	if(pcache->type != MOD_BRUSH)
	{
		Con_EPrintf("%s called with an pentity %d that doesn't have MOD_BRUSH model type.\n", __FUNCTION__, entity);
		return nullptr;
	}

	const brushmodel_t* pmodel = pcache->getBrushmodel();
	const hull_t* phull = &pmodel->hulls[hulltype];

	if(poffset)
	{
		Math::VectorSubtract(phull->clipmins, cls.pminfo.player_mins[hulltype], *poffset);
		Math::VectorAdd(*poffset, pentity->curstate.origin, *poffset);
	}

	return phull;
}

//=============================================
//
//=============================================
Float CL_TraceModel( Int32 entity, const Vector& start, const Vector& end, hull_types_t hulltype, Int32 flags, trace_t& trace )
{
	if(hulltype >= MAX_MAP_HULLS || hulltype < 0 && hulltype != HULL_AUTO)
	{
		Con_EPrintf("%s - Bogus hull index %d.\n", __FUNCTION__, hulltype);
		return 0;
	}

	cl_entity_t* pentity = CL_GetEntityByIndex(entity);
	if(!pentity)
		return 0;

	TR_PlayerTraceSingleEntity(pentity->curstate, pentity->pvbmhulldata, start, end, hulltype, flags, cls.pminfo.player_mins[hulltype], cls.pminfo.player_maxs[hulltype], trace);

	return trace.fraction;
}

//=============================================
//
//=============================================
const Char* CL_TraceTexture( Int32 groundentity, const Vector& start, const Vector& end )
{
	if(groundentity < 0 || groundentity >= (Int32)cls.entities.size())
	{
		Con_Printf("%s - Bogus entity index %d.\n", __FUNCTION__, groundentity);
		return nullptr;
	}

	cl_entity_t* pentity = CL_GetEntityByIndex(groundentity);
	if(!pentity)
		return nullptr;

	if(!pentity->curstate.modelindex)
	{
		Con_Printf("%s - Called on entity %d with no model set.\n", __FUNCTION__, groundentity);
		return nullptr;
	}
	
	return TR_TraceTexture(pentity->curstate, start, end);
}
