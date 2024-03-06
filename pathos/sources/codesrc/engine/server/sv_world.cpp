/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "com_math.h"
#include "edict.h"
#include "sv_main.h"
#include "sv_entities.h"
#include "system.h"

#include "enginestate.h"
#include "edictmanager.h"
#include "brushmodel.h"
#include "modelcache.h"
#include "frustum.h"
#include "sv_world.h"
#include "trace_shared.h"
#include "vbmtrace.h"
#include "sv_physics.h"

// Hull mins for tracehull
static const Vector HULL_MINS[MAX_MAP_HULLS] = {
	Vector(0.0f, 0.0f, 0.0f),		// Point hull
	Vector(-16.0f, -16.0f, -36.0f),	// Human hull
	Vector(-32.0f, -32.0f, -32.0f),	// Large hull
	Vector(-16.0f, -16.0f, -18.0f)	// Small hull
};

// Hull maxs for tracehull
static const Vector HULL_MAXS[MAX_MAP_HULLS] = {
	Vector(0.0f, 0.0f, 0.0f),		// Point hull
	Vector(16.0f, 16.0f, 36.0f),	// Human hull
	Vector(32.0f, 32.0f, 32.0f),	// Large hull
	Vector(16.0f, 16.0f, 18.0f)		// Small hull
};

//=============================================
//
//=============================================
void SV_UnlinkEdict( edict_t* pentity )
{
	if(!pentity->parealink || !pentity->pareachain)
		return;

	pentity->pareachain->remove(pentity->parealink);
	pentity->pareachain = nullptr;
	pentity->parealink = nullptr;
}

//=============================================
//
//=============================================
void SV_LinkEdict( edict_t* pentity, bool touchtriggers )
{
	// Unlink from old position
	if(pentity->pareachain && pentity->parealink)
		SV_UnlinkEdict(pentity);

	// Don't add world or unused entities
	if(pentity->entindex == WORLDSPAWN_ENTITY_INDEX || pentity->free)
		return;

	// Set the abs box
	svs.dllfuncs.pfnSetAbsBox(pentity);

	if(pentity->state.movetype == MOVETYPE_FOLLOW && pentity->state.aiment != NO_ENTITY_INDEX)
	{
		// Copy leafnums from our parent pentity
		edict_t* paiment = gEdicts.GetEdict(pentity->state.aiment);
		pentity->leafnums = paiment->leafnums;
	}
	else
	{
		// Clear leafnums
		if(!pentity->leafnums.empty())
			pentity->leafnums.clear();

		// Define the mins/maxs for checking leaves
		Vector checkmins = pentity->state.absmin - Vector(1, 1, 1);
		Vector checkmaxs = pentity->state.absmax + Vector(1, 1, 1);

		// Find the leaves we're touching
		Mod_FindTouchedLeafs(ens.pworld, pentity->leafnums, checkmins, checkmaxs, ens.pworld->pnodes);
	}

	// Ignore non-solid objects
	if(pentity->state.solid == SOLID_NOT && pentity->state.skin >= -1)
		return;

	if(pentity->state.solid == SOLID_BSP && !pentity->state.modelindex && pentity->fields.modelname != NO_STRING_VALUE)
	{
		Con_Printf("Entity %s has no model set.\n", SV_GetString(pentity->fields.classname));
		return;
	}

	// Find the area node
	areanode_t* pnode = &svs.areanodes[0];
	while(true)
	{
		if(pnode->axis == -1)
			break;

		if(pentity->state.absmin[pnode->axis] > pnode->dist)
			pnode = pnode->pchildren[0];
		else if(pentity->state.absmax[pnode->axis] < pnode->dist)
			pnode = pnode->pchildren[1];
		else 
			break;
	}

	EdictChainType_t::link_t* plink = nullptr;
	if(pentity->state.solid == SOLID_TRIGGER)
	{
		plink = pnode->trigger_edicts.add(pentity);
		pentity->pareachain = &pnode->trigger_edicts;
	}
	else
	{
		plink = pnode->solid_edicts.add(pentity);
		pentity->pareachain = &pnode->solid_edicts;
	}

	pentity->parealink = plink;

	if(touchtriggers && !g_serverPhysics.touchlinksemaphore)
	{
		g_serverPhysics.touchlinksemaphore = true;
		SV_TouchLinks(pentity, &svs.areanodes[0]);
		g_serverPhysics.touchlinksemaphore = false;
	}
}

//=============================================
//
//=============================================
const hull_t* SV_HullForBSP( const edict_t* pentity, const Vector& mins, const Vector& maxs, Vector* poffset, hull_types_t hulltype )
{
	// Get model
	const cache_model_t* pcache = Cache_GetModel(pentity->state.modelindex);
	if(Cache_GetModelType(*pcache) != MOD_BRUSH)
	{
		Con_Printf("%s - Hit pentity %s with no model(%s).", __FUNCTION__, SV_GetString(pentity->fields.classname), SV_GetString(pentity->fields.modelname));
		return nullptr;
	}

	const brushmodel_t* pmodel = pcache->getBrushmodel();

	// If hulltype is specified, just give out hull
	if(hulltype != HULL_AUTO)
	{
		if(hulltype >= MAX_MAP_HULLS || hulltype < 0 && hulltype != HULL_AUTO)
		{
			Con_Printf("%s - Bogus hull index %d.", __FUNCTION__, hulltype);
			return nullptr;
		}

		const hull_t* phull = &pmodel->hulls[hulltype];

		if(poffset)
		{
			Math::VectorSubtract(phull->clipmins, mins, *poffset);
			Math::VectorAdd(*poffset, pentity->state.origin, *poffset);
		}

		return phull;
	}

	Vector size;
	Math::VectorSubtract(maxs, mins, size);

	const hull_t *phull = nullptr;
	if(size[0] < 8.0f)
	{
		phull = &pmodel->hulls[HULL_POINT];

		if(poffset)
			Math::VectorCopy(phull->clipmins, *poffset);
	}
	else
	{
		if(size[0] <= 36.0f)
		{
			if(size[2] <= 36.0f)
				phull = &pmodel->hulls[HULL_SMALL];
			else
				phull = &pmodel->hulls[HULL_HUMAN];
		}
		else
		{
			phull = &pmodel->hulls[HULL_LARGE];
		}

		if(poffset)
			Math::VectorSubtract(phull->clipmins, mins, *poffset);
	}

	if(poffset)
		Math::VectorAdd(*poffset, pentity->state.origin, *poffset);

	return phull;
}

//=============================================
//
//=============================================
const hull_t* SV_HullForBSP( Int32 entity, hull_types_t hulltype, Vector* poffset )
{
	if(hulltype >= MAX_MAP_HULLS || hulltype < 0)
	{
		Con_EPrintf("%s - Bogus hull index %d.\n", __FUNCTION__, hulltype);
		return 0;
	}

	edict_t* pedict = gEdicts.GetEdict(entity);
	if(!pedict)
		return nullptr;

	const cache_model_t* pcache = Cache_GetModel(pedict->state.modelindex);
	if(pcache->type != MOD_BRUSH)
	{
		Con_EPrintf("%s called with an pentity %d that doesn't have MOD_BRUSH model type.\n", __FUNCTION__, entity);
		return nullptr;
	}

	const brushmodel_t* pmodel = pcache->getBrushmodel();
	const hull_t* phull = &pmodel->hulls[hulltype];

	if(poffset)
	{
		Math::VectorSubtract(phull->clipmins, svs.player_mins[hulltype], *poffset);
		Math::VectorAdd(*poffset, pedict->state.origin, *poffset);
	}

	return phull;
}

//=============================================
//
//=============================================
void SV_TouchLinks( edict_t* pentity, areanode_t* pnode )
{
	pnode->trigger_edicts.begin();
	while(!pnode->trigger_edicts.end())
	{
		edict_t* ptouchedict = pnode->trigger_edicts.get();
		pnode->trigger_edicts.next();

		if(ptouchedict == pentity)
			continue;

		if(pentity->state.groupinfo && ptouchedict->state.groupinfo)
		{
			if(ens.tr_groupop)
			{
				if(ens.tr_groupop == TR_GROUPOP_NAND && (pentity->state.groupinfo & ptouchedict->state.groupinfo))
					continue;
			}
			else
			{
				if(!(pentity->state.groupinfo & ptouchedict->state.groupinfo))
					continue;
			}
		}

		if((pentity->state.flags & FL_KILLME) || (ptouchedict->state.flags & FL_KILLME))
			continue;

		if(ptouchedict->state.solid != SOLID_TRIGGER)
			continue;

		// Check if the bounding boxes intersect eachother
		if(Math::CheckMinsMaxs(pentity->state.absmin, pentity->state.absmax, ptouchedict->state.absmin, ptouchedict->state.absmax))
			continue;

		if(ptouchedict->state.modelindex && gModelCache.GetType(ptouchedict->state.modelindex) == MOD_BRUSH)
		{
			// Get the hull type
			Vector offset, localOrigin;
			const hull_t* phull = SV_HullForBSP(ptouchedict, pentity->state.mins, pentity->state.maxs, &offset);

			// Offset the test position for the test on this hull
			Math::VectorSubtract(pentity->state.origin, offset, localOrigin);

			// Rotate by angles if needed
			if(!ptouchedict->state.angles.IsZero())
				Math::RotateToEntitySpace(ptouchedict->state.angles, localOrigin);

			// Test for hull intersection with this model
			if(TR_HullPointContents(phull, phull->firstclipnode, localOrigin) != CONTENTS_SOLID)
				continue;
		}

		svs.gamevars.time = svs.time;
		svs.dllfuncs.pfnDispatchTouch(ptouchedict, pentity);
	}

	// Recurse down both sides
	if(pnode->axis == -1)
		return;

	if(pentity->state.absmax[pnode->axis] > pnode->dist)
		SV_TouchLinks(pentity, pnode->pchildren[0]);

	if(pentity->state.absmin[pnode->axis] < pnode->dist)
		SV_TouchLinks(pentity, pnode->pchildren[1]);
}

//=============================================
//
//=============================================
struct areanode_t* SV_CreateAreaNode( Int32 depth, const Vector& mins, const Vector& maxs )
{
	if(svs.numareanodes == MAX_AREA_NODES)
		return nullptr;

	// Add the new node to the array
	areanode_t& node = svs.areanodes[svs.numareanodes];
	svs.numareanodes++;

	if(depth == MAX_AREA_DEPTH)
	{
		node.axis = -1;
		node.pchildren[0] = nullptr;
		node.pchildren[1] = nullptr;
		return &node;
	}

	Vector size;
	Math::VectorSubtract(mins, maxs, size);

	if(size[0] > size[1])
		node.axis = 0;
	else
		node.axis = 1;

	node.dist = 0.5 * (maxs[node.axis] + mins[node.axis]);

	Vector mins1, maxs1;
	Vector mins2, maxs2;
	
	Math::VectorCopy(mins, mins1);
	Math::VectorCopy(maxs, maxs1);
	Math::VectorCopy(mins, mins2);
	Math::VectorCopy(maxs, maxs2);

	maxs1[node.axis] = node.dist;
	mins2[node.axis] = node.dist;

	node.pchildren[0] = SV_CreateAreaNode(depth+1, mins2, maxs2);
	node.pchildren[1] = SV_CreateAreaNode(depth+1, mins1, maxs1);

	return &node;
}

//=============================================
//
//=============================================
Int32 SV_LinkContents( areanode_t& node, const Vector& position )
{
	areanode_t* pnode = &node;

	while(TRUE)
	{
		pnode->solid_edicts.begin();
		while(!pnode->solid_edicts.end())
		{
			edict_t* ptouchedict = pnode->solid_edicts.get();
			pnode->solid_edicts.next();

			if(!ptouchedict->state.modelindex)
				continue;

			if(ptouchedict->state.groupinfo)
			{
				if(ens.tr_groupop)
				{
					if(ens.tr_groupop == TR_GROUPOP_NAND && (ptouchedict->state.groupinfo & ens.tr_groupmask))
						continue;
				}
				else
				{
					if(!(ptouchedict->state.groupinfo & ens.tr_groupmask))
						continue;
				}
			}

			if(gModelCache.GetType(ptouchedict->state.modelindex) != MOD_BRUSH)
				continue;

			// Check if the position is in the bounding box of this pentity
			if(position[0] > ptouchedict->state.absmax[0]
			|| position[1] > ptouchedict->state.absmax[1]
			|| position[2] > ptouchedict->state.absmax[2]
			|| position[0] < ptouchedict->state.absmin[0]
			|| position[1] < ptouchedict->state.absmin[1]
			|| position[2] < ptouchedict->state.absmin[2])
				continue;

			Int32 contents = ptouchedict->state.skin;
			if(contents < -100 || contents > 100)
				Con_Printf("Invalid contents on trigger field: %s\n", SV_GetString(ptouchedict->fields.classname));

			// Get the hull type
			Vector offset, localOrigin;
			const hull_t* phull = SV_HullForBSP(ptouchedict, ZERO_VECTOR, ZERO_VECTOR, &offset);

			// Offset the test position for the test on this hull
			Math::VectorSubtract(position, offset, localOrigin);

			// Test for hull intersection with this model
			if(TR_HullPointContents(phull, phull->firstclipnode, localOrigin) != CONTENTS_EMPTY)
				return contents;
		}

		if(pnode->axis == -1)
			return CONTENTS_EMPTY;

		if(position[pnode->axis] > pnode->dist)
		{
			pnode = pnode->pchildren[0];
			continue;
		}
		else if(position[pnode->axis] < pnode->dist)
		{
			pnode = pnode->pchildren[1];
			continue;
		}

		break;
	}

	return CONTENTS_EMPTY;
}

//=============================================
//
//=============================================
const hull_t* SV_HullForEntity( const edict_t* pentity, const Vector& mins, const Vector& maxs, Vector* poffset, hull_types_t hulltype )
{
	// Decide which clipping hull to use
	if(pentity->state.solid == SOLID_BSP)
	{
		// explicit hulls in the bsp model
		if(pentity->state.movetype != MOVETYPE_PUSH && pentity->state.movetype != MOVETYPE_PUSHSTEP)
			Con_Printf("%s - SOLID_BSP without MOVETYPE_PUSH for %s.\n", __FUNCTION__, SV_GetString(pentity->fields.classname));

		return SV_HullForBSP(pentity, mins, maxs, poffset, hulltype);
	}

	Vector hullmins, hullmaxs;
	Math::VectorSubtract(pentity->state.mins, maxs, hullmins);
	Math::VectorSubtract(pentity->state.maxs, mins, hullmaxs);

	if(poffset)
		Math::VectorCopy(pentity->state.origin, *poffset);

	return TR_HullForBox(hullmins, hullmaxs);
}

//=============================================
//
//=============================================
void SV_SingleClipMoveToEntity( edict_t* pentity, const Vector& start, const Vector& mins, const Vector& maxs, const Vector& end, trace_t& trace, Int32 flags, hull_types_t hulltype )
{
	// Set basic parameters
	trace.endpos = end;
	trace.fraction = 1.0;
	trace.flags |= FL_TR_ALLSOLID;

	Vector offset;
	cache_model_t* pmodel = gModelCache.GetModelByIndex(pentity->state.modelindex);
	if(!pmodel)
		return;

	const hull_t* phull = nullptr;
	const CArray<vbmhitboxhull_t>* pvbmhulls = nullptr;

	// Get the appropriate hull
	if(pmodel->type == MOD_VBM && (flags & FL_TRACE_HITBOXES || pmodel->flags & STUDIO_MF_TRACE_HITBOX))
	{
		// Set VBM hull data if not already set
		TR_VBMSetHullInfo(pentity->pvbmhulldata, pmodel, mins, maxs, pentity->state, svs.time, hulltype);
		// Retrieve pointer to array
		pvbmhulls = TR_VBMGetHulls(pentity->pvbmhulldata, mins, maxs, hulltype, flags, &offset);
	}
	else
	{
		// Retrieve regular hull
		phull = SV_HullForEntity(pentity, mins, maxs, &offset, hulltype);
	}

	Vector start_l;
	Vector end_l;
	Math::VectorSubtract(start, offset, start_l);
	Math::VectorSubtract(end, offset, end_l);

	// Rotate the pentity if needed
	if(pmodel->type != MOD_VBM && pentity->state.solid == SOLID_BSP && !pentity->state.angles.IsZero())
	{
		Math::RotateToEntitySpace(pentity->state.angles, start_l);
		Math::RotateToEntitySpace(pentity->state.angles, end_l);
	}

	if(!pvbmhulls)
	{
		// Regular trace
		TR_RecursiveHullCheck(phull, phull->firstclipnode, 0.0f, 1.0f, start_l, end_l, trace);
	}
	else
	{
		if(pvbmhulls->empty())
		{
			trace.flags &= ~FL_TR_ALLSOLID;
			return;
		}

		// Trace against VBM hulls
		TR_VBMHullCheck(pvbmhulls, start_l, end_l, trace);
	}

	if(trace.fraction != 1.0f)
	{
		if(pmodel->type == MOD_BRUSH && pentity->state.solid == SOLID_BSP && !pentity->state.angles.IsZero())
			Math::RotateFromEntitySpace(pentity->state.angles, trace.plane.normal);

		Vector point;
		Math::VectorSubtract(end, start, point);
		Math::VectorMA(start, trace.fraction, point, trace.endpos);
	}

	if(trace.fraction < 1.0f || (trace.flags & (FL_TR_ALLSOLID|FL_TR_STARTSOLID)))
		trace.hitentity = pentity->entindex;
}

//=============================================
//
//=============================================
void SV_SingleClipMoveToEntityPoint( edict_t* pentity, const Vector& start, const Vector& end, Int32 flags, trace_t& trace )
{
	// Set basic parameters
	trace.endpos = end;
	trace.fraction = 1.0;
	trace.flags |= FL_TR_ALLSOLID;

	const cache_model_t* pmodel = Cache_GetModel(pentity->state.modelindex);
	if(!pmodel)
	{
		Con_EPrintf("%s - Called on entity %s with no model.\n", __FUNCTION__, SV_GetString(pentity->fields.classname));
		return;
	}

	if(pmodel->type != MOD_BRUSH && pmodel->type != MOD_VBM)
	{
		Con_EPrintf("%s - Called on entity %s with model that is not a brush or vbm model.\n", __FUNCTION__, SV_GetString(pentity->fields.classname));
		return;
	}

	const hull_t* phull = nullptr;
	const CArray<vbmhitboxhull_t>* pvbmhulls = nullptr;

	Vector start_l;
	Vector end_l;

	// Get the appropriate hull
	if(pmodel->type == MOD_VBM && (flags & FL_TRACE_HITBOXES || pmodel->flags & STUDIO_MF_TRACE_HITBOX) && !(pentity->state.flags & FL_NO_HITBOX_TRACE))
	{
		// Set VBM hull data if not already set
		TR_VBMSetHullInfo(pentity->pvbmhulldata, pmodel, ZERO_VECTOR, ZERO_VECTOR, pentity->state, svs.time, HULL_POINT);
		// Retrieve pointer to array
		pvbmhulls = TR_VBMGetHulls(pentity->pvbmhulldata, ZERO_VECTOR, ZERO_VECTOR, HULL_POINT, flags, nullptr);

		Math::VectorCopy(start, start_l);
		Math::VectorCopy(end, end_l);
	}
	else
	{
		// Retrieve regular hull
		phull = SV_HullForEntity(pentity, ZERO_VECTOR, ZERO_VECTOR, nullptr, HULL_POINT); 

		Math::VectorSubtract(start, pentity->state.origin, start_l);
		Math::VectorSubtract(end, pentity->state.origin, end_l);
	}

	// Rotate the pentity if needed
	if(pmodel->type == MOD_BRUSH && pentity->state.solid == SOLID_BSP && !pentity->state.angles.IsZero())
	{
		Math::RotateToEntitySpace(pentity->state.angles, start_l);
		Math::RotateToEntitySpace(pentity->state.angles, end_l);
	}

	if(pmodel->type == MOD_BRUSH || !pvbmhulls)
	{
		// Regular trace
		TR_RecursiveHullCheck(phull, phull->firstclipnode, 0.0f, 1.0f, start_l, end_l, trace);
	}
	else
	{
		if(pvbmhulls->empty())
		{
			trace.flags &= ~FL_TR_ALLSOLID;
			return;
		}

		// Trace against VBM hulls
		TR_VBMHullCheck(pvbmhulls, start_l, end_l, trace);
	}

	if(trace.fraction != 1.0f)
	{
		if(pmodel->type == MOD_BRUSH && pentity->state.solid == SOLID_BSP && !pentity->state.angles.IsZero())
			Math::RotateFromEntitySpace(pentity->state.angles, trace.plane.normal);

		Vector point;
		Math::VectorSubtract(end, start, point);
		Math::VectorMA(start, trace.fraction, point, trace.endpos);
	}

	if(trace.fraction < 1.0f || (trace.flags & FL_TR_STARTSOLID))
		trace.hitentity = pentity->entindex;
}

//=============================================
//
//=============================================
Int32 SV_PointContents( const Vector& position, Int32* truecontents )
{
	Int32 contents = TR_HullPointContents(&ens.pworld->hulls[0], 0, position);
	if(truecontents)
		*truecontents = contents;
	
	if(contents != CONTENTS_EMPTY)
		return contents;

	Int32 entitycontents = SV_LinkContents(svs.areanodes[0], position);
	if(entitycontents != CONTENTS_EMPTY)
	{
		if(truecontents)
			*truecontents = contents;

		return entitycontents;
	}
	else
		return contents;
}

//=============================================
//
//=============================================
Int32 SV_TruePointContents( const Vector& position )
{
	return TR_HullPointContents(&ens.pworld->hulls[0], ens.pworld->hulls[0].firstclipnode, position);
}

//=============================================
//
//=============================================
void SV_ClipToLinks( areanode_t& node, moveclip_t& clip, Int32 flags, hull_types_t hulltype )
{
	node.solid_edicts.begin();
	while(!node.solid_edicts.end())
	{
		edict_t* ptouchedict = node.solid_edicts.get();
		node.solid_edicts.next();

		if(ptouchedict->state.groupinfo && clip.pignore_edict && clip.pignore_edict->state.groupinfo)
		{
			if(ens.tr_groupop)
			{
				if(ens.tr_groupop == TR_GROUPOP_NAND && (clip.pignore_edict->state.groupinfo & ptouchedict->state.groupinfo))
					continue;
			}
			else
			{
				if(!(clip.pignore_edict->state.groupinfo & ptouchedict->state.groupinfo))
					continue;
			}
		}

		// Ignore non-solids, or the ignore pentity
		if(ptouchedict->state.solid == SOLID_NOT || ptouchedict == clip.pignore_edict)
			continue;

		// Shouldn't happen really
		if(ptouchedict->state.solid == SOLID_TRIGGER)
		{
			Con_Printf("Error: SV_ClipToLinks - Trigger pentity(%s) in clipping list!\n", SV_GetString(ptouchedict->fields.classname));
			continue;
		}

		// See if the game wants us to collide with this
		if(!svs.dllfuncs.pfnShouldCollide(clip.pignore_edict, ptouchedict))
			continue;

		// Monsterclip handling
		if(ptouchedict->state.solid == SOLID_BSP)
		{
			if((ptouchedict->state.flags & FL_NPC_CLIP) && !(clip.flags & FL_TRACE_NPC_CLIP))
				continue;
		}
		else // everything else is npcs
		{
			// Ignore everything but pushables(still I want to support these?)
			if((clip.flags & FL_TRACE_NO_NPCS) && ptouchedict->state.movetype != MOVETYPE_PUSHSTEP)
				continue;
		}

		// Do not hit corpses unless specified so
		if(!(clip.flags & FL_TRACE_HIT_CORPSES) && (ptouchedict->state.flags & FL_DEAD || ptouchedict->state.deadstate != DEADSTATE_NONE))
			continue;

		// Ignore transparent entities if set
		if((clip.flags & FL_TRACE_NO_TRANS) && ptouchedict->state.rendermode != RENDER_NORMAL && !(ptouchedict->state.flags & FL_WORLDBRUSH))
			continue;

		// See if it's in the box
		if(Math::CheckMinsMaxs(clip.boxmins, clip.boxmaxs, ptouchedict->state.absmin, ptouchedict->state.absmax))
			continue;

		if(clip.pignore_edict && clip.pignore_edict->state.size[0] && !ptouchedict->state.size[0])
			continue; // Don't let points intersect

		// If all of the trace is solid, exit
		if(clip.trace.flags & FL_TR_ALLSOLID)
			return;

		if(clip.pignore_edict)
		{
			// Don't clip against children
			if(ptouchedict->state.owner == clip.pignore_edict->entindex)
				continue; 

			// Don't clip against owner
			if(clip.pignore_edict->state.owner == ptouchedict->entindex)
				continue; 
		}

		trace_t trace;
		if(ptouchedict->state.flags & FL_NPC)
			SV_SingleClipMoveToEntity(ptouchedict, *clip.pstart, clip.mins2, clip.maxs2, *clip.pend, trace, clip.flags, hulltype);
		else
			SV_SingleClipMoveToEntity(ptouchedict, *clip.pstart, *clip.pmins1, *clip.pmaxs1, *clip.pend, trace, clip.flags, hulltype);

		if((trace.flags & (FL_TR_ALLSOLID|FL_TR_STARTSOLID)) || trace.fraction < clip.trace.fraction)
		{
			trace.hitentity = ptouchedict->entindex;
			clip.trace = trace;

			if(clip.trace.flags & FL_TR_STARTSOLID)
				clip.trace.flags |= FL_TR_STARTSOLID;
		}
	}

	// Recurse down both sides
	if(node.axis == -1)
		return;

	if(clip.boxmaxs[node.axis] > node.dist)
		SV_ClipToLinks(*node.pchildren[0], clip, flags, hulltype);

	if(clip.boxmins[node.axis] < node.dist)
		SV_ClipToLinks(*node.pchildren[1], clip, flags, hulltype);
}

//=============================================
//
//=============================================
void SV_ClipToWorldBrush( areanode_t& node, moveclip_t& clip, Int32 flags, hull_types_t hulltype )
{
	node.solid_edicts.begin();
	while(!node.solid_edicts.end())
	{
		edict_t* ptouchedict = node.solid_edicts.get();
		node.solid_edicts.next();

		if(ptouchedict->state.solid != SOLID_BSP || !(ptouchedict->state.flags & FL_WORLDBRUSH))
			continue;

		// See if it's in the box
		if(Math::CheckMinsMaxs(clip.boxmins, clip.boxmaxs, ptouchedict->state.absmin, ptouchedict->state.absmax))
			continue;

		// If all of the trace is solid, exit
		if(clip.trace.flags & FL_TR_ALLSOLID)
			return;

		trace_t trace;
		SV_SingleClipMoveToEntity(ptouchedict, *clip.pstart, *clip.pmins1, *clip.pmaxs1, *clip.pend, trace, hulltype);

		if((trace.flags & (FL_TR_ALLSOLID|FL_TR_STARTSOLID)) || trace.fraction < clip.trace.fraction)
		{
			trace.hitentity = ptouchedict->entindex;
			clip.trace = trace;

			if(clip.trace.flags & FL_TR_STARTSOLID)
				clip.trace.flags |= FL_TR_STARTSOLID;
		}
	}

	// Recurse down both sides
	if(node.axis == -1)
		return;

	if(clip.boxmaxs[node.axis] > node.dist)
		SV_ClipToLinks(*node.pchildren[0], clip, flags, hulltype);

	if(clip.boxmaxs[node.axis] < node.dist)
		SV_ClipToLinks(*node.pchildren[1], clip, flags, hulltype);
}

//=============================================
//
//=============================================
void SV_ClipToLinksPoint( areanode_t& node, moveclip_t& clip, Int32 flags )
{
	node.solid_edicts.begin();
	while(!node.solid_edicts.end())
	{
		edict_t* ptouchedict = node.solid_edicts.get();
		node.solid_edicts.next();

		if(ptouchedict->state.groupinfo && clip.pignore_edict && clip.pignore_edict->state.groupinfo)
		{
			if(ens.tr_groupop)
			{
				if(ens.tr_groupop == TR_GROUPOP_NAND && (clip.pignore_edict->state.groupinfo & ptouchedict->state.groupinfo))
					continue;
			}
			else
			{
				if(!(clip.pignore_edict->state.groupinfo & ptouchedict->state.groupinfo))
					continue;
			}
		}

		// Ignore non-solids, or the ignore pentity
		if(ptouchedict->state.solid == SOLID_NOT || ptouchedict == clip.pignore_edict)
			continue;

		// Shouldn't happen really
		if(ptouchedict->state.solid == SOLID_TRIGGER)
		{
			Con_Printf("%s - Trigger pentity(%s) in clipping list!\n", __FUNCTION__, SV_GetString(ptouchedict->fields.classname));
			continue;
		}

		// Check for no models flag
		if(flags & FL_TRACE_NO_MODELS)
		{
			const cache_model_t* pmodel = Cache_GetModel(ptouchedict->state.modelindex);
			if(!pmodel)
			{
				Con_EPrintf("%s - Called on entity %s with no model.\n", __FUNCTION__, SV_GetString(ptouchedict->fields.classname));
				continue;
			}

			if(pmodel->type == MOD_VBM)
				continue;
		}

		// See if the game wants us to collide with this
		if(!svs.dllfuncs.pfnShouldCollide(clip.pignore_edict, ptouchedict))
			continue;

		// Monsterclip handling
		if(ptouchedict->state.solid == SOLID_BSP)
		{
			if((ptouchedict->state.flags & FL_NPC_CLIP) && !(clip.flags & FL_TRACE_NPC_CLIP))
				continue;
		}
		else // everything else is npcs
		{
			// Ignore everything but pushables(still I want to support these?)
			if((clip.flags & FL_TRACE_NO_NPCS) && ptouchedict->state.movetype != MOVETYPE_PUSHSTEP)
				continue;
		}

		// Ignore transparent entities if set
		if((clip.flags & FL_TRACE_NO_TRANS) && ptouchedict->state.rendermode != RENDER_NORMAL && ((clip.flags & FL_TRACE_NO_TRANS_WORLDBRUSH) || !(ptouchedict->state.flags & FL_WORLDBRUSH)))
			continue;

		// Do not hit corpses unless specified so
		if(!(clip.flags & FL_TRACE_HIT_CORPSES) && (ptouchedict->state.flags & FL_DEAD || ptouchedict->state.deadstate != DEADSTATE_NONE))
			continue;

		// See if it's in the box
		if(Math::CheckMinsMaxs(clip.boxmins, clip.boxmaxs, ptouchedict->state.absmin, ptouchedict->state.absmax))
			continue;

		if(clip.pignore_edict && clip.pignore_edict->state.size[0] && !ptouchedict->state.size[0])
			continue; // Don't let points intersect

		// If all of the trace is solid, exit
		if(clip.trace.flags & FL_TR_ALLSOLID)
			return;

		if(clip.pignore_edict)
		{
			// Don't clip against children
			if(ptouchedict->state.owner == clip.pignore_edict->entindex)
				continue; 

			// Don't clip against owner
			if(clip.pignore_edict->state.owner == ptouchedict->entindex)
				continue; 
		}

		trace_t trace;
		SV_SingleClipMoveToEntityPoint(ptouchedict, *clip.pstart, *clip.pend, flags, trace);

		if((trace.flags & (FL_TR_ALLSOLID|FL_TR_STARTSOLID)) || trace.fraction < clip.trace.fraction)
		{
			trace.hitentity = ptouchedict->entindex;
			clip.trace = trace;

			if(clip.trace.flags & FL_TR_STARTSOLID)
				clip.trace.flags |= FL_TR_STARTSOLID;
		}
	}

	// Recurse down both sides
	if(node.axis == -1)
		return;

	if(clip.boxmaxs[node.axis] > node.dist)
		SV_ClipToLinksPoint(*node.pchildren[0], clip, flags);

	if(clip.boxmins[node.axis] < node.dist)
		SV_ClipToLinksPoint(*node.pchildren[1], clip, flags);
}

//=============================================
//
//=============================================
void SV_Move( trace_t& trace, const Vector& start, const Vector& mins, const Vector& maxs, const Vector& end, Int32 flags, edict_t* pignore_edict, hull_types_t hulltype )
{
	moveclip_t clip;
	edict_t* pworld = gEdicts.GetEdict(WORLDSPAWN_ENTITY_INDEX);
	if(hulltype == HULL_POINT)
		SV_SingleClipMoveToEntityPoint(pworld, start, end, flags, clip.trace);
	else
		SV_SingleClipMoveToEntity(pworld, start, mins, maxs, end, clip.trace, flags, hulltype);

	if(!(flags & FL_TRACE_WORLD_ONLY) 
		&& !clip.trace.allSolid()
		&& !clip.trace.startSolid())
	{
		Vector traceEndpos = clip.trace.endpos;
		Double traceFraction = clip.trace.fraction;

		clip.trace.fraction = 1.0f;
		clip.pstart = &start;
		clip.pend = &traceEndpos;

		clip.flags = flags;
		clip.pignore_edict = pignore_edict;

		clip.pmins1 = &mins;
		clip.pmaxs1 = &maxs;

		if(clip.flags & FL_TRACE_EXTRASIZE)
		{
			clip.mins2 = Vector(-15, -15, -15);
			clip.maxs2 = Vector(15, 15, 15);
		}
		else
		{
			clip.mins2 = mins;
			clip.maxs2 = maxs;
		}

		TR_MoveBounds(start, clip.mins2, clip.maxs2, traceEndpos, clip.boxmins, clip.boxmaxs);
		if(hulltype == HULL_POINT)
			SV_ClipToLinksPoint(svs.areanodes[0], clip, flags);
		else
			SV_ClipToLinks(svs.areanodes[0], clip, flags, hulltype);

		clip.trace.fraction *= traceFraction;
	}

	trace = clip.trace;
	svs.gamevars.globaltrace = clip.trace;
}

//=============================================
//
//=============================================
void SV_MoveNoEntities( trace_t& trace, const Vector& start, const Vector& mins, const Vector& maxs, const Vector& end, Int32 flags, edict_t* pignore_edict, hull_types_t hulltype )
{
	Vector traceEndpos;

	moveclip_t clip;
	edict_t* pworld = gEdicts.GetEdict(WORLDSPAWN_ENTITY_INDEX);
	SV_SingleClipMoveToEntity(pworld, start, mins, maxs, end, clip.trace, flags, hulltype);

	if(!(flags & FL_TRACE_WORLD_ONLY) 
		&& !clip.trace.allSolid()
		&& !clip.trace.startSolid())
	{
		Math::VectorCopy(clip.trace.endpos, traceEndpos);
		Float traceFraction = clip.trace.fraction;

		clip.trace.fraction = 1.0f;
		clip.pstart = &start;
		clip.pend = &end;

		clip.flags = flags;
		clip.pignore_edict = pignore_edict;

		clip.pmins1 = &mins;
		clip.pmaxs1 = &maxs;

		if(clip.flags & FL_TRACE_EXTRASIZE)
		{
			clip.mins2 = Vector(-15, -15, -15);
			clip.maxs2 = Vector(15, 15, 15);
		}
		else
		{
			clip.mins2 = mins;
			clip.maxs2 = maxs;
		}

		TR_MoveBounds(start, clip.mins2, clip.maxs2, traceEndpos, clip.boxmins, clip.boxmaxs);
		SV_ClipToWorldBrush(svs.areanodes[0], clip, flags, hulltype);

		clip.trace.fraction *= traceFraction;
		svs.gamevars.globaltrace = clip.trace;
	}

	trace = clip.trace;
}

//=============================================
//
//=============================================
void SV_Move_Point( trace_t& trace, const Vector& start, const Vector& end, Int32 flags, edict_t* pignore_edict )
{
	moveclip_t clip;
	edict_t* pworld = gEdicts.GetEdict(WORLDSPAWN_ENTITY_INDEX);
	SV_SingleClipMoveToEntityPoint(pworld, start, end, flags, clip.trace);

	if(!(flags & FL_TRACE_WORLD_ONLY) 
		&& !clip.trace.allSolid()
		&& !clip.trace.startSolid())
	{
		Vector traceEndpos = clip.trace.endpos;
		Float traceFraction = clip.trace.fraction;

		clip.trace.fraction = 1.0f;
		clip.pstart = &start;
		clip.pend = &traceEndpos;

		clip.flags = flags;
		clip.pignore_edict = pignore_edict;

		clip.mins2 = ZERO_VECTOR;
		clip.maxs2 = ZERO_VECTOR;

		TR_MoveBoundsPoint(start, traceEndpos, clip.boxmins, clip.boxmaxs);
		SV_ClipToLinksPoint(svs.areanodes[0], clip, flags);

		clip.trace.fraction *= traceFraction;
	}

	trace = clip.trace;
	svs.gamevars.globaltrace = clip.trace;
}

//=============================================
//
//=============================================
edict_t* SV_TestEntityPosition( edict_t* pentity, hull_types_t hulltype )
{
	Int32 flags = FL_TR_NONE;
	if(pentity->state.flags & FL_NPC_CLIP)
		flags |= FL_TRACE_NPC_CLIP;

	trace_t trace;
	SV_Move(trace, pentity->state.origin, pentity->state.mins, pentity->state.maxs, pentity->state.origin, (trace_flags_t)flags, pentity, hulltype);

	if(trace.flags & FL_TR_ALLSOLID)
	{
		svs.gamevars.globaltrace = trace;
		return gEdicts.GetEdict(trace.hitentity);
	}

	return nullptr;
}

//=============================================
//
//=============================================
Int32 SV_TestPlayerPosition( hull_types_t hulltype, Int32 flags, const Vector& position )
{
	trace_t trace;
	SV_PlayerTrace(position, position, flags, hulltype, NO_ENTITY_INDEX, trace);

	return trace.hitentity;
}

//=============================================
//
//=============================================
void SV_PlayerTrace( const Vector& start, const Vector& end, Int32 traceflags, hull_types_t hulltype, Int32 ignore_ent, trace_t& trace )
{
	if(hulltype < 0 || hulltype >= MAX_MAP_HULLS)
	{
		Con_EPrintf("%s - Bogus hull index %d.\n", __FUNCTION__, hulltype);
		return;
	}

	// set basics
	trace.fraction = 1.0;
	trace.hitentity = NO_ENTITY_INDEX;
	trace.endpos = end;

	// trace against world first
	edict_t* pworld = gEdicts.GetEdict(WORLDSPAWN_ENTITY_INDEX);
	TR_PlayerTraceSingleEntity(pworld->state, pworld->pvbmhulldata, start, end, hulltype, traceflags, svs.player_mins[hulltype], svs.player_maxs[hulltype], trace);

	// Trace against entities if applicable
	if(!(traceflags & FL_TRACE_WORLD_ONLY))
	{
		Vector tracemins, tracemaxs;
		TR_MoveBoundsPoint(start, end, tracemins, tracemaxs);
		Math::VectorAdd(tracemins, svs.player_mins[hulltype], tracemins);
		Math::VectorAdd(tracemaxs, svs.player_maxs[hulltype], tracemaxs);

		for(Int32 i = 1; i < (Int32)gEdicts.GetNbEdicts(); i++)
		{
			if(i == ignore_ent || i == ((Int32)svs.pmoveplayerindex+1))
				continue;

			// Get pointer to edict
			edict_t* pentity = gEdicts.GetEdict(i);
			if(!pentity)
				continue;

			if(!pentity->state.modelindex)
				continue;

			if(pentity->state.solid == SOLID_NOT)
				continue;

			if(pentity->state.flags & FL_NPC_CLIP && pentity->state.solid == SOLID_BSP)
				continue;

			if(pentity->state.flags & FL_CLIENT && pentity->state.health <= 0)
				continue;

			if(pentity->state.flags & FL_PMOVE_IGNORE)
				continue;

			if(pentity->state.solid == SOLID_TRIGGER)
				continue;

			if(pentity->state.owner == svs.clients[svs.pmoveplayerindex].pedict->entindex)
				continue;

			// Do not hit corpses unless specified so
			if(!(traceflags & FL_TRACE_HIT_CORPSES) && (pentity->state.flags & FL_DEAD || pentity->state.deadstate != DEADSTATE_NONE))
				continue;

			// Check for specific flags;
			if(traceflags & FL_TRACE_NO_TRANS && pentity->state.rendermode != RENDER_NORMAL)
				continue;

			Vector entitymins, entitymaxs;
			Math::VectorAdd(pentity->state.absmin, svs.player_mins[hulltype], entitymins);
			Math::VectorAdd(pentity->state.absmax, svs.player_maxs[hulltype], entitymaxs);

			// Optimize on traceline
			if(Math::CheckMinsMaxs(tracemins, tracemaxs, entitymins, entitymaxs))
				continue;

			// Update VBM hull data if needed
			cache_model_t* pmodel = gModelCache.GetModelByIndex(pentity->state.modelindex);
			if(!pmodel)
				return;

			// Set VBM data if needed
			if(pmodel->type == MOD_VBM && (pmodel->flags & STUDIO_MF_TRACE_HITBOX || traceflags & FL_TRACE_HITBOXES))
				TR_VBMSetHullInfo(pentity->pvbmhulldata, pmodel, svs.player_mins[hulltype], svs.player_maxs[hulltype], pentity->state, svs.time, hulltype);

			TR_PlayerTraceSingleEntity(pentity->state, pentity->pvbmhulldata, start, end, hulltype, traceflags, svs.player_mins[hulltype], svs.player_maxs[hulltype], trace);
		}
	}
}

//=============================================
//
//=============================================
void SV_TraceLine( const Vector& start, const Vector& end, Int32 traceflags, hull_types_t hulltype, Int32 ignore_ent, trace_t& trace )
{
	if((ignore_ent < 0 || ignore_ent >= (Int32)gEdicts.GetNbEdicts()) && ignore_ent != NO_ENTITY_INDEX)
	{
		Con_Printf("%s - Bogus entity index %d.\n", __FUNCTION__, ignore_ent);
		return;
	}

	edict_t* pignoreent = nullptr;
	if(ignore_ent != NO_ENTITY_INDEX)
		pignoreent = gEdicts.GetEdict(ignore_ent);

	if(hulltype == HULL_POINT)
		SV_Move_Point(trace, start, end, traceflags, pignoreent);
	else
		SV_Move(trace, start, HULL_MINS[hulltype], HULL_MAXS[hulltype], end, traceflags, pignoreent, hulltype);
}

//=============================================
//
//=============================================
Float SV_TraceModel( entindex_t entindex, const Vector& start, const Vector& end, hull_types_t hulltype, Int32 flags, trace_t& trace )
{
	if(hulltype >= MAX_MAP_HULLS || hulltype < 0 && hulltype != HULL_AUTO)
	{
		Con_EPrintf("%s - Bogus hull index %d.\n", __FUNCTION__, hulltype);
		return 0;
	}

	if(entindex < 0 || entindex >= (Int32)gEdicts.GetNbEdicts())
	{
		Con_Printf("%s - Bogus entity index %d.\n", __FUNCTION__, entindex);
		return 0;
	}

	edict_t* pedict = gEdicts.GetEdict(entindex);
	if(!pedict)
	{
		Con_Printf("%s - No such entity with index %d.\n", __FUNCTION__, entindex);
		return 0;
	}

	Int32 savedmovetype = -1, savedsolidity = -1;

	cache_model_t* pmodel = gModelCache.GetModelByIndex(pedict->state.modelindex);
	if(pmodel && pmodel->type == MOD_BRUSH)
	{
		savedmovetype = pedict->state.movetype;
		savedsolidity = pedict->state.solid;

		pedict->state.movetype = MOVETYPE_PUSH;
		pedict->state.solid = SOLID_BSP;
	}

	SV_SingleClipMoveToEntity(pedict, start, HULL_MINS[hulltype], HULL_MAXS[hulltype], end, trace, flags, hulltype);

	if(savedmovetype != -1 && savedsolidity != -1)
	{
		pedict->state.movetype = savedmovetype;
		pedict->state.solid = savedsolidity;
	}

	return trace.fraction;
}