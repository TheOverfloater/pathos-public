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
#include "system.h"
#include "com_math.h"
#include "sv_world.h"
#include "sv_physics.h"
#include "sv_entities.h"

#include "texturemanager.h"
#include "brushmodel.h"
#include "modelcache.h"
#include "enginestate.h"
#include "edictmanager.h"
#include "gdll_interface.h"
#include "console.h"

// Holds variables for server physics
svphysics_t g_serverPhysics;

// Max clipping planes
#define MAX_CLIP_PLANES 5

CCVar* g_psv_maxvelocity = nullptr;
CCVar* g_psv_gravity = nullptr;
CCVar* g_psv_bounce = nullptr;
CCVar* g_psv_stepsize = nullptr;
CCVar* g_psv_friction = nullptr;
CCVar* g_psv_stopspeed = nullptr;

//=============================================
//
//=============================================
void SV_Physics_Init( void )
{
	// Create cvars
	g_psv_maxvelocity = gConsole.CreateCVar(CVAR_FLOAT, (FL_CV_SV_ONLY|FL_CV_NOTIFY), "sv_maxvelocity", "8192", "Max velocity limit for entities.");
	g_psv_gravity = gConsole.CreateCVar(CVAR_FLOAT, (FL_CV_SV_ONLY|FL_CV_NOTIFY), GRAVITY_CVAR_NAME, "800", "Gravity factor.");
	g_psv_bounce = gConsole.CreateCVar(CVAR_FLOAT, (FL_CV_SV_ONLY|FL_CV_NOTIFY), "sv_bounce", "1", "Bounce value.");
	g_psv_stepsize = gConsole.CreateCVar(CVAR_FLOAT, (FL_CV_SV_ONLY|FL_CV_NOTIFY), "sv_stepsize", "18", "Maximum step size for players and npcs.");
	g_psv_friction = gConsole.CreateCVar(CVAR_FLOAT, (FL_CV_SV_ONLY|FL_CV_NOTIFY), "sv_friction", "4", "Friction value.");
	g_psv_stopspeed = gConsole.CreateCVar(CVAR_FLOAT, (FL_CV_SV_ONLY|FL_CV_NOTIFY), "sv_stopspeed", "100", "Stop speed.");

	// Get max edicts
	Uint32 maxEdicts;
	if(ens.arg_max_edicts != 0)
		maxEdicts = ens.arg_max_edicts;
	else
		maxEdicts = DEFAULT_MAX_EDICTS;

	if(!g_serverPhysics.savedmovingentities.empty())
		g_serverPhysics.savedmovingentities.clear();

	g_serverPhysics.savedmovingentities.resize(maxEdicts);
	g_serverPhysics.numsavedmovingents = 0;
}

//=============================================
//
//=============================================
void SV_CheckVelocity( edict_t* pedict )
{
	for(Uint32 i = 0; i < 3; i++)
	{
		// Check velocity
		if(pedict->state.velocity.IsNAN(i))
		{
			Con_Printf("Warning: Got a NaN velocity on %s.\n", SV_GetString(pedict->fields.classname));
			pedict->state.velocity[i] = 0;
		}

		// Check origin
		if(pedict->state.origin.IsNAN(i))
		{
			Con_Printf("Warning: Got a NaN origin on %s.\n", SV_GetString(pedict->fields.classname));
			pedict->state.origin[i] = 0;
		}

		// Check velocity bounds
		Float maxvelocity = g_psv_maxvelocity->GetValue();
		if(pedict->state.velocity[i] > maxvelocity)
		{
			Con_Printf("Warning: Velocity %f too high on %s.\n", pedict->state.velocity[i], SV_GetString(pedict->fields.classname));
			pedict->state.origin[i] = maxvelocity;
		}
		else if(pedict->state.velocity[i] < -maxvelocity)
		{
			Con_Printf("Warning: Velocity %f too low on %s.\n", pedict->state.velocity[i], SV_GetString(pedict->fields.classname));
			pedict->state.origin[i] = -maxvelocity;
		}
	}
}

//=============================================
//
//=============================================
void SV_ClipVelocity( const Vector& in, const Vector& normal, Vector& out, Float overbounce )
{
	Float backoff = Math::DotProduct(in, normal)*overbounce;

	for(Uint32 i = 0; i < 3; i++)
	{
		Float change = normal[i] * backoff;
		out[i] = in[i] - change;

		// Cap if it's too low
		if(out[i] > -STOP_EPSILON && out[i] < STOP_EPSILON)
			out[i] = 0.0f;
	}
}

//=============================================
//
//=============================================
void SV_FlyMove( edict_t* pedict, Double time, Float bounce )
{
	Vector planes[MAX_CLIP_PLANES];

	// Save these for later
	Vector savedvelocity = pedict->state.velocity;
	Vector origvelocity = savedvelocity;

	Int32 moveflags = FL_TRACE_NORMAL;
	if(pedict->state.solid == SOLID_TRIGGER || pedict->state.solid == SOLID_NOT)
		moveflags |= FL_TRACE_NO_NPCS;

	if(pedict->state.flags & FL_NPC_CLIP)
		moveflags |= FL_TRACE_NPC_CLIP;

	// total time for this movement
	Int32 numplanes = 0;
	Double timeleft = time;

	for(Uint32 i = 0; i < MAX_CLIP_PLANES-1; i++)
	{
		if(Math::IsVectorZero(pedict->state.velocity))
			break;

		// Try moving all the way
		Vector move;
		Math::VectorMA(pedict->state.origin, timeleft, pedict->state.velocity, move);

		// See if we were successful
		trace_t trace;
		SV_Move(trace, pedict->state.origin, pedict->state.mins, pedict->state.maxs, move, moveflags, pedict, (hull_types_t)pedict->state.forcehull);
		if(trace.flags & FL_TR_ALLSOLID)
		{
			// We're trapped in a solid
			pedict->state.velocity.Clear();
			break;
		}

		if(trace.fraction > 0)
		{
			trace_t test;
			SV_Move(test, trace.endpos, pedict->state.mins, pedict->state.maxs, trace.endpos, moveflags, pedict, (hull_types_t)pedict->state.forcehull);
			if(!(test.flags & FL_TR_ALLSOLID))
			{
				pedict->state.origin = trace.endpos;
				origvelocity = pedict->state.velocity;
				numplanes = 0;
			}
		}

		// Break if we moved all the way
		if(trace.fraction == 1)
			break;

		// Check if the result is valid
		if(trace.hitentity == NO_ENTITY_INDEX)
		{
			Con_Printf("%s - Trace returned no entity.\n", __FUNCTION__);
			return;
		}

		// Stop if we're on the ground
		if(trace.plane.normal[2] > 0.7)
		{
			edict_t* pentity = gEdicts.GetEdict(trace.hitentity);
			if(pentity->state.solid == SOLID_BSP
				|| pentity->state.solid == SOLID_SLIDEBOX
				|| pentity->state.movetype == MOVETYPE_PUSHSTEP
				|| pentity->state.flags & FL_CLIENT)
			{
				pedict->state.flags |= FL_ONGROUND;
				pedict->state.groundent = pentity->entindex;
			}

			if((pedict->state.flags & FL_ONGROUND) && pedict->state.movetype == MOVETYPE_TOSS)
			{
				pedict->state.velocity.Clear();
				SV_Impact(pedict, pentity, trace);
				return;
			}
		}

		// Run impact code
		edict_t* phitedict = gEdicts.GetEdict(trace.hitentity);
		SV_Impact(pedict, phitedict, trace);
		if(pedict->free)
			break;

		// reduce frametime by total time left * fraction
		timeleft -= timeleft * trace.fraction;

		// Did we run out of planes to clip against?
		if(numplanes >= MAX_CLIP_PLANES)
		{
			pedict->state.velocity.Clear();
			break;
		}

		// Add to the planes list
		planes[numplanes] = trace.plane.normal;
		numplanes++;

		Vector newvelocity;
		if(numplanes == 1 && pedict->state.movetype == MOVETYPE_WALK 
			&& (!(pedict->state.flags & FL_ONGROUND) || pedict->state.friction != 1.0))
		{
			Float d;
			if(planes[0][2] <= 0.7)
				d = (1.0 - pedict->state.friction)*g_psv_bounce->GetValue() + 1.0f;
			else
				d = 1.0;

			SV_ClipVelocity(origvelocity, planes[0], newvelocity, d);

			// Go along this planes
			pedict->state.velocity = newvelocity;
			origvelocity = newvelocity;
		}
		else
		{
			Int32 j, k;
			for(j = 0; j < numplanes; j++)
			{
				SV_ClipVelocity(origvelocity, planes[j], newvelocity, bounce);
				for(k = 0; k < numplanes; k++)
				{
					if(k != j && Math::DotProduct(newvelocity, planes[k]) < 0)
						break;
				}

				if(k == numplanes)
					break;
			}

			if(j != numplanes)
			{
				// go along this plane
				pedict->state.velocity = newvelocity;
			}
			else
			{
				// go along the crease
				if(numplanes != 2)
				{
					pedict->state.velocity.Clear();
					break;
				}

				Vector dir;
				Math::CrossProduct(planes[0], planes[1], dir);
				Float d = Math::DotProduct(dir, pedict->state.velocity);
				Math::VectorScale(dir, d, pedict->state.velocity);
			}

			// stop if the velocity got inverted
			if(Math::DotProduct(pedict->state.velocity, savedvelocity) <= 0)
			{
				pedict->state.velocity.Clear();
				break;
			}
		}
	}
}

//=============================================
//
//=============================================
bool SV_CheckWater( edict_t* pedict )
{
	// Set origin to center
	Vector origin;
	for(Uint32 i = 0; i < 2; i++)
		origin[i] = (pedict->state.absmax[i] + pedict->state.absmin[i])*0.5;
	origin[2] = pedict->state.absmin[2] + 1;

	pedict->state.watertype = CONTENTS_EMPTY;
	pedict->state.waterlevel = WATERLEVEL_NONE;

	// Set group mask
	ens.tr_groupmask = pedict->state.groupinfo;

	Int32 contents = SV_PointContents(origin);
	if(contents <= CONTENTS_WATER && contents > CONTENTS_MIN)
	{
		pedict->state.watertype = contents;
		pedict->state.waterlevel = WATERLEVEL_LOW;

		if(pedict->state.absmin[2] != pedict->state.absmax[2])
		{
			ens.tr_groupmask = pedict->state.groupinfo;
			// test with the height in the center now
			origin[2] = (pedict->state.absmax[2] + pedict->state.absmin[2])*0.5f;

			Int32 truecontents = SV_PointContents(origin);
			if(truecontents <= CONTENTS_WATER && truecontents >= CONTENTS_MIN)
			{
				// We're halfway in water
				ens.tr_groupmask = pedict->state.groupinfo;
				pedict->state.waterlevel = WATERLEVEL_MID;

				// test with the view offset now
				origin[2] = pedict->state.origin[2];
				Math::VectorAdd(origin, pedict->state.view_offset, origin);
				truecontents = SV_PointContents(origin);

				if(truecontents <= CONTENTS_WATER && truecontents > CONTENTS_MIN)
					pedict->state.waterlevel = WATERLEVEL_FULL;
			}
		}
		else
		{
			// entity is fully submerged
			pedict->state.waterlevel = WATERLEVEL_FULL;
		}
	}

	return (pedict->state.waterlevel > WATERLEVEL_LOW) ? true : false;
}

//=============================================
//
//=============================================
void SV_WaterMove( edict_t* pedict )
{
	if(pedict->state.movetype == MOVETYPE_NOCLIP)
		return;

	if(pedict->state.health <= 0)
		return;

	if(pedict->state.waterlevel == WATERLEVEL_NONE)
	{
		if(pedict->state.flags & FL_INWATER)
		{
			// call server to play water sounds
			svs.dllfuncs.pfnDispatchCrossedWater(pedict, false);
			pedict->state.flags &= ~FL_INWATER;
		}

		return;
	}
	else if(!(pedict->state.flags & FL_INWATER))
	{
		// play to call water sounds on server
		if(pedict->state.watertype == CONTENTS_WATER)
			svs.dllfuncs.pfnDispatchCrossedWater(pedict, true);

		pedict->state.flags |= FL_INWATER;
	}

	// dampen velocity if not waterjumping
	if(!(pedict->state.flags & FL_WATERJUMP))
		Math::VectorMA(pedict->state.velocity, (pedict->state.waterlevel * svs.frametime * 0.8f), pedict->state.velocity, pedict->state.velocity);
}

//=============================================
//
//=============================================
Float SV_RecursiveWaterLevel( const Vector& origin, Float mins, Float maxs, Int32 depth )
{
	Float waterlevel = ((mins - maxs)*0.5f + maxs);
	
	Int32 _depth = depth;
	_depth++;

	if(_depth > 5)
		return waterlevel;

	Vector testorigin = origin;
	testorigin[2] += waterlevel;

	if(SV_PointContents(testorigin) == CONTENTS_WATER)
		return SV_RecursiveWaterLevel(origin, mins, waterlevel, _depth);
	else
		return SV_RecursiveWaterLevel(origin, waterlevel, maxs, _depth);
}
//=============================================
//
//=============================================
Float SV_Submerged( edict_t* pedict )
{
	Vector center;
	Math::VectorScale(pedict->state.absmax, 0.5, center);
	Math::VectorMA(center, 0.5, pedict->state.absmin, center);

	Float waterlevel = pedict->state.absmin[2] - center[2];
	switch(pedict->state.waterlevel)
	{
	case WATERLEVEL_LOW:
		return SV_RecursiveWaterLevel(center, 0, waterlevel, 0) - waterlevel;
		break;
	case WATERLEVEL_MID:
		return SV_RecursiveWaterLevel(center, pedict->state.absmax[2] - center[2], 0, 0) - waterlevel;
		break;
	case WATERLEVEL_FULL:
		{
			Vector origin = center;
			origin[2] = pedict->state.absmax[2];

			ens.tr_groupmask = pedict->state.groupinfo;
			if(SV_PointContents(origin) == CONTENTS_WATER)
				return (pedict->state.maxs[2] - pedict->state.mins[2]);

			return SV_RecursiveWaterLevel(center, pedict->state.absmax[2] - center[2], 0, 0) - waterlevel;
		}
	default:
		return 0;
	}
}

//=============================================
//
//=============================================
void SV_CheckWaterTransition( edict_t* pedict )
{
	// Set origin to center
	Vector origin;
	for(Uint32 i = 0; i < 2; i++)
		origin[i] = (pedict->state.absmax[i] + pedict->state.absmin[i])*0.5;
	origin[2] = pedict->state.absmin[2] + 1;

	ens.tr_groupmask = pedict->state.groupinfo;

	Int32 contents = SV_PointContents(origin);
	if(pedict->state.watertype == CONTENTS_NONE)
	{
		// just spawned
		pedict->state.watertype = contents;
		pedict->state.waterlevel = WATERLEVEL_LOW;
		return;
	}

	// see if we just crossed boundaries
	if(contents > CONTENTS_WATER || contents <= CONTENTS_MIN)
	{
		if(pedict->state.watertype != CONTENTS_EMPTY)
			svs.dllfuncs.pfnDispatchCrossedWater(pedict, false);

		pedict->state.watertype = CONTENTS_EMPTY;
		pedict->state.waterlevel = WATERLEVEL_NONE;
		return;
	}

	// See if we just crossed into water
	if(pedict->state.watertype == CONTENTS_EMPTY)
	{
		svs.dllfuncs.pfnDispatchCrossedWater(pedict, true);
		pedict->state.velocity[2] *= 0.5f;
	}

	pedict->state.watertype = contents;
	pedict->state.waterlevel = WATERLEVEL_LOW;

	// check if it's a point entity
	if(pedict->state.absmin[2] == pedict->state.absmax[2])
	{
		pedict->state.waterlevel = WATERLEVEL_FULL;
		return;
	}

	ens.tr_groupmask = pedict->state.groupinfo;
	origin[2] = (pedict->state.absmin[2] + pedict->state.absmax[2])*0.5f;
	contents = SV_PointContents(origin);
	if(contents <= CONTENTS_WATER && contents > CONTENTS_MIN)
	{
		pedict->state.waterlevel = WATERLEVEL_MID;
		origin[2] = pedict->state.origin[2] + pedict->state.view_offset[2];

		ens.tr_groupmask = pedict->state.groupinfo;
		contents = SV_PointContents(origin);
		if(contents <= CONTENTS_WATER && contents > CONTENTS_MIN)
			pedict->state.waterlevel = WATERLEVEL_FULL;
	}
}

//=============================================
//
//=============================================
bool SV_CheckGround( edict_t* pedict )
{
	// Determine if it's on the ground
	Vector origin, absmin, absmax;
	Math::VectorAdd(pedict->state.origin, pedict->state.mins, absmin);
	Math::VectorAdd(pedict->state.origin, pedict->state.maxs, absmax);

	origin[2] = absmin[2] - 1.0f;

	for(Uint32 x = 0; x <= 1; x++)
	{
		for(Uint32 y = 0; y <= 1; y++)
		{
			origin[0] = x ? absmax[0] : absmin[0];
			origin[1] = y ? absmax[1] : absmin[1];

			ens.tr_groupmask = pedict->state.groupinfo;

			if(SV_PointContents(origin) == CONTENTS_SOLID)
				return true;
		}
	}

	return false;
}

//=============================================
//
//=============================================
bool SV_CheckBottomPrecise( edict_t* pedict, const Vector& mins, const Vector& maxs )
{
	// Do a more precise check
	Vector origin;
	origin[2] = mins[2] + g_psv_stepsize->GetValue();

	Vector stop;
	for(Uint32 i = 0; i < 2; i++)
		origin[i] = stop[i] = (mins[i] + maxs[i]) * 0.5f;

	Float stepsize = g_psv_stepsize->GetValue();
	stop[2] = origin[2] - 2.0f * stepsize;

	Int32 moveFlags = FL_TRACE_NO_NPCS;
	if(pedict->state.flags & FL_NPC_CLIP)
		moveFlags |= FL_TRACE_NPC_CLIP;

	// Do a trace test
	trace_t trace;
	SV_Move(trace, origin, ZERO_VECTOR, ZERO_VECTOR, stop, moveFlags, pedict, (hull_types_t)pedict->state.forcehull);
	if(trace.fraction == 1.0f)
		return false;

	Float mid, bottom;
	mid = bottom = trace.endpos[2];

	// the corners must be within 16 units of the midpoint
	for(Uint32 x2 = 0; x2 <= 1; x2++)
	{
		for(Uint32 y2 = 0; y2 <= 1; y2++)
		{
			origin[0] = stop[0] = x2 ? maxs[0] : mins[0];
			origin[1] = stop[1] = y2 ? maxs[1] : mins[1];

			SV_Move(trace, origin, ZERO_VECTOR, ZERO_VECTOR, stop, moveFlags, pedict, (hull_types_t)pedict->state.forcehull);
			if(trace.fraction != 1.0f && trace.endpos[2] > bottom)
				bottom = trace.endpos[2];

			if(trace.fraction == 1.0f || mid - trace.endpos[2] > stepsize)
				return false;
		}
	}

	return true;
}

//=============================================
//
//=============================================
bool SV_CheckBottom( edict_t* pedict )
{
	Vector absmin, absmax, origin;
	Math::VectorAdd(pedict->state.origin, pedict->state.mins, absmin);
	Math::VectorAdd(pedict->state.origin, pedict->state.maxs, absmax);

	origin[2] = absmin[2] - 1.0f;

	for(Uint32 x1 = 0; x1 <= 1; x1++)
	{
		for(Uint32 y1 = 0; y1 <= 1; y1++)
		{
			origin[0] = x1 ? absmax[0] : absmin[0];
			origin[1] = y1 ? absmax[1] : absmin[1];

			ens.tr_groupmask = pedict->state.groupinfo;

			if(SV_PointContents(origin) != CONTENTS_SOLID)
				return SV_CheckBottomPrecise(pedict, absmin, absmax);
		}
	}

	return true;
}

//=============================================
//
//=============================================
void SV_AddGravity( edict_t* pedict )
{
	Float edictGravity;
	if(pedict->state.gravity)
		edictGravity = pedict->state.gravity;
	else
		edictGravity = 1.0f;

	pedict->state.velocity[2] -= edictGravity * g_psv_gravity->GetValue() * svs.frametime;
	pedict->state.velocity[2] += pedict->state.basevelocity[2] * svs.frametime;
	// Clear base velocity
	pedict->state.basevelocity[2] = 0.0f;

	SV_CheckVelocity(pedict);
}

//=============================================
//
//=============================================
void SV_Impact( edict_t* pentity1, edict_t* pentity2, const trace_t& trace )
{
	svs.gamevars.time = svs.time;

	if((pentity1->state.flags & FL_KILLME) || (pentity2->state.flags & FL_KILLME))
		return;

	if(pentity1->state.groupinfo && pentity2->state.groupinfo)
	{
		if(ens.tr_groupop)
		{
			if(ens.tr_groupop == TR_GROUPOP_NAND && (pentity1->state.groupinfo & pentity2->state.groupinfo))
				return;
		}
		else
		{
			if(!(pentity1->state.groupinfo & pentity2->state.groupinfo))
				return;
		}
	}

	if(pentity1->state.solid != SOLID_NOT)
	{
		svs.gamevars.globaltrace = trace;
		svs.dllfuncs.pfnDispatchTouch(pentity1, pentity2);

		if((pentity1->state.flags & FL_KILLME) || (pentity2->state.flags & FL_KILLME))
			return;
	}

	if(pentity2->state.solid != SOLID_NOT)
	{
		svs.gamevars.globaltrace = trace;
		svs.dllfuncs.pfnDispatchTouch(pentity2, pentity1);
	}
}

//=============================================
//
//=============================================
void SV_PushEntity( edict_t* pentity, const Vector& push, trace_t& trace )
{
	Vector end;
	Math::VectorAdd(push, pentity->state.origin, end);

	Int32 moveFlags = FL_TRACE_NORMAL;
	if(pentity->state.solid == SOLID_TRIGGER || pentity->state.solid == SOLID_NOT)
		moveFlags |= FL_TRACE_NO_NPCS;

	if(pentity->state.flags & FL_NPC_CLIP)
		moveFlags |= FL_TRACE_NPC_CLIP;
	
	SV_Move(trace, pentity->state.origin, pentity->state.mins, pentity->state.maxs, end, moveFlags, pentity, (hull_types_t)pentity->state.forcehull);

	if(trace.fraction != 0 && !(trace.flags & FL_TR_ALLSOLID))
	{
		Math::VectorCopy(trace.endpos, pentity->state.origin);
		SV_LinkEdict(pentity, true);
	}

	if(trace.hitentity != NO_ENTITY_INDEX)
	{
		edict_t* phitedict = gEdicts.GetEdict(trace.hitentity);
		SV_Impact(pentity, phitedict, trace);
	}
}

//=============================================
//
//=============================================
bool SV_PushRotate( edict_t* ppusher, Float movetime )
{
	trace_t trace;
	Vector amove, pushorig;
	Vector forward, right, up;
	Vector curForward, curRight, curUp;

	if(ppusher->state.avelocity.IsZero())
	{
		ppusher->state.ltime += movetime;
		return true;
	}

	Math::VectorScale(ppusher->state.avelocity, movetime, amove);
	Math::AngleVectors(ppusher->state.angles, &forward, &right, &up);
	pushorig = ppusher->state.angles;

	// Move the entity to it's final position
	Math::VectorAdd(ppusher->state.angles, amove, ppusher->state.angles);
	Math::AngleVectorsTranspose(ppusher->state.angles, &curForward, &curRight, &curUp);
	ppusher->state.ltime += movetime;
	SV_LinkEdict(ppusher, false);

	if(ppusher->state.solid == SOLID_NOT)
		return true;

	// See if any solid entities are inside the final position
	g_serverPhysics.numsavedmovingents = 0;
	for(Int32 i = 1; i < (Int32)gEdicts.GetNbEdicts(); i++)
	{
		edict_t* pother = gEdicts.GetEdict(i);
		if(pother->free)
			continue;

		if(pother->state.movetype == MOVETYPE_PUSH
			|| pother->state.movetype == MOVETYPE_NONE
			|| pother->state.movetype == MOVETYPE_FOLLOW
			|| pother->state.movetype == MOVETYPE_NOCLIP)
			continue;

		// See if the entity is standing on us
		if(!(pother->state.flags & FL_ONGROUND) || pother->state.groundent != ppusher->entindex)
		{
			if(Math::CheckMinsMaxs(pother->state.absmin, pother->state.absmax, ppusher->state.absmin, ppusher->state.absmax))
				continue;

			// See if we're inside the final position
			if(SV_TestEntityPosition(pother, (hull_types_t)pother->state.forcehull) != ppusher)
				continue;
		}

		// remove the onground flag for non-players
		if(pother->state.movetype != MOVETYPE_WALK)
			pother->state.flags &= ~FL_ONGROUND;

		Vector entityOrigin;
		Math::VectorCopy(pother->state.origin, entityOrigin);

		saved_move_t* psave = &g_serverPhysics.savedmovingentities[g_serverPhysics.numsavedmovingents];
		g_serverPhysics.numsavedmovingents++;

		Math::VectorCopy(entityOrigin, psave->saved_origin);
		psave->psave_edict = pother;

		Vector start;
		if(pother->state.movetype == MOVETYPE_PUSHSTEP)
		{
			Vector origin;
			Math::VectorScale(pother->state.absmin, 0.5, origin);
			Math::VectorMA(origin, 0.5, pother->state.absmax, origin);
			Math::VectorSubtract(origin, pother->state.origin, start);
		}
		else
		{
			Math::VectorSubtract(pother->state.origin, ppusher->state.origin, start);
		}

		// Calculate destination position
		Vector move;
		move[0] = Math::DotProduct(forward, start);
		move[1] = -Math::DotProduct(right, start);
		move[2] = Math::DotProduct(up, start);

		Vector end;
		end[0] = Math::DotProduct(curForward, move);
		end[1] = Math::DotProduct(curRight, move);
		end[2] = Math::DotProduct(curUp, move);

		Vector push;
		Math::VectorSubtract(end, start, push);

		// Try moving the contacted entity
		ppusher->state.solid = SOLID_NOT;
		SV_PushEntity(pother, push, trace);
		ppusher->state.solid = SOLID_BSP;

		if(pother->state.movetype != MOVETYPE_PUSHSTEP)
		{
			if(pother->state.flags & FL_CLIENT)
			{
				pother->state.addavelocity = true;
				pother->state.avelocity[YAW] += amove[YAW];
			}
			else
			{
				pother->state.angles[YAW] += amove[YAW];
			}
		}

		// If it's still inside the ppusher, block
		if(SV_TestEntityPosition(pother, (hull_types_t)pother->state.forcehull) == ppusher)
		{
			if(pother->state.mins[0] == pother->state.maxs[0])
				continue;

			if(pother->state.solid == SOLID_NOT 
				|| pother->state.solid == SOLID_TRIGGER
				|| pother->state.flags & FL_DEAD)
				continue;

			Math::VectorCopy(entityOrigin, pother->state.origin);
			SV_LinkEdict(pother, TRUE);

			Math::VectorCopy(pushorig, ppusher->state.angles);
			SV_LinkEdict(ppusher, FALSE);

			ppusher->state.ltime -= movetime;
			svs.dllfuncs.pfnDispatchBlocked(ppusher, pother);

			// Move back any entities we already moved
			for(Uint32 j = 0; j < g_serverPhysics.numsavedmovingents; j++)
			{
				saved_move_t* psaved = &g_serverPhysics.savedmovingentities[j];
				Math::VectorCopy(psaved->saved_origin, psaved->psave_edict->state.origin);

				if(psaved->psave_edict->state.flags & FL_CLIENT)
					psaved->psave_edict->state.avelocity[1] = 0.0f;
				else if(psaved->psave_edict->state.movetype != MOVETYPE_PUSHSTEP)
					psaved->psave_edict->state.angles[YAW] -= amove[YAW];

				SV_LinkEdict(psaved->psave_edict, FALSE);
			}

			return false;
		}
	}

	return true;
}

//=============================================
//
//=============================================
bool SV_PushMove_UnstickGroundEntity( edict_t* ppusher, edict_t* pother, const Vector& prevOrigin, const Vector& movedOrigin )
{
	// Try by moving back the npc a bit on the distance traveled
	Double fraction = 0;
	bool canUnstick = false;
	while(fraction < 1.0f)
	{
		Vector testPosition = prevOrigin*fraction + movedOrigin * (1.0 - fraction);
		fraction += 0.1f;

		pother->state.origin = testPosition;

		if(SV_TestEntityPosition(pother, (hull_types_t)pother->state.forcehull) != ppusher)
		{
			canUnstick = true;
			break;
		}
	}

	// Try moving the entity opposite of the move direction until they are clear
	if(!canUnstick && !ppusher->state.movedir.IsZero())
	{
		Vector moveDirection = ppusher->state.movedir;
		Math::VectorScale(moveDirection, -1, moveDirection);
		moveDirection.Normalize();

		// Go to a maximum of 4 units
		for(Float dist = 0.1; dist < 4.0f; dist += 0.1)
		{
			Vector testOrigin = movedOrigin + moveDirection * dist;
			pother->state.origin = testOrigin;

			if(SV_TestEntityPosition(pother, (hull_types_t)pother->state.forcehull) != ppusher)
			{
				canUnstick = true;
				break;
			}
		}
	}

	if(canUnstick)
	{
		// Try using DropToFloor to land the entity exactly
		// on this ground entity
		if(!SV_DropToFloor(pother))
		{
			// Remove groundent and onground flag
			pother->state.flags &= ~FL_ONGROUND;
			pother->state.groundent = NO_ENTITY_INDEX;

			// If SV_DropToFloor fails, just link the edict
			// and have him land a bit later
			SV_LinkEdict(pother, true);
		}
	}

	return canUnstick;
}

//=============================================
//
//=============================================
void SV_PushMove( edict_t* ppusher, Float movetime )
{
	if(Math::IsVectorZero(ppusher->state.velocity))
	{
		ppusher->state.ltime = movetime + ppusher->state.ltime;
		return;
	}

	trace_t trace;
	Vector move, mins, maxs;
	Math::VectorScale(ppusher->state.velocity, movetime, move);
	Math::VectorAdd(ppusher->state.absmin, move, mins);
	Math::VectorAdd(ppusher->state.absmax, move, maxs);

	Vector pushorigin;
	Math::VectorCopy(ppusher->state.origin, pushorigin);

	// Move the pusher to it's final position
	Math::VectorAdd(ppusher->state.origin, move, ppusher->state.origin);
	ppusher->state.ltime += movetime;
	SV_LinkEdict(ppusher, FALSE);

	if(ppusher->state.solid == SOLID_NOT)
		return;

	// See if any solid entites are in the final position
	g_serverPhysics.numsavedmovingents = 0;
	for(Int32 i = 1; i < (Int32)gEdicts.GetNbEdicts(); i++)
	{
		edict_t* pother = gEdicts.GetEdict(i);
		if(pother->free)
			continue;

		if(pother->state.movetype == MOVETYPE_PUSH
			|| pother->state.movetype == MOVETYPE_NONE
			|| pother->state.movetype == MOVETYPE_FOLLOW
			|| pother->state.movetype == MOVETYPE_NOCLIP)
			continue;

		// See if the entity is standing on us
		if(!(pother->state.flags & FL_ONGROUND) || pother->state.groundent != ppusher->entindex)
		{
			if(Math::CheckMinsMaxs(pother->state.absmin, pother->state.absmax, ppusher->state.absmin, ppusher->state.absmax))
				continue;

			// See if we're inside the final position
			if(SV_TestEntityPosition(pother, (hull_types_t)pother->state.forcehull) != ppusher)
				continue;
		}

		// remove the onground flag for non-players
		if(pother->state.movetype != MOVETYPE_WALK)
			pother->state.flags &= ~FL_ONGROUND;

		Vector entityOrigin;
		Math::VectorCopy(pother->state.origin, entityOrigin);

		saved_move_t* psave = &g_serverPhysics.savedmovingentities[g_serverPhysics.numsavedmovingents];
		g_serverPhysics.numsavedmovingents++;

		Math::VectorCopy(entityOrigin, psave->saved_origin);
		psave->psave_edict = pother;

		// Try moving the contacted entity
		ppusher->state.solid = SOLID_NOT;
		SV_PushEntity(pother, move, trace);
		ppusher->state.solid = SOLID_BSP;

		// If it's still inside the pusher, block
		if(SV_TestEntityPosition(pother, (hull_types_t)pother->state.forcehull) == ppusher)
		{
			if(pother->state.groundent == ppusher->entindex)
			{
				// Sometimes on-ground entities get stuck, so try to unstick them(TODO: precision issue or what?)
				if(SV_PushMove_UnstickGroundEntity(ppusher, pother, entityOrigin, pother->state.origin))
					continue;
			}

			if(pother->state.mins[0] == pother->state.maxs[0])
				continue;

			if(pother->state.solid == SOLID_NOT || pother->state.solid == SOLID_TRIGGER)
			{
				// corpse
				pother->state.mins[0] = pother->state.mins[1] = 0;
				pother->state.maxs[0] = pother->state.maxs[1] = 0;
				pother->state.maxs[2] = pother->state.mins[2];
				continue;
			}

			Math::VectorCopy(entityOrigin, pother->state.origin);
			SV_LinkEdict(pother, TRUE);

			Math::VectorCopy(pushorigin, ppusher->state.origin);
			SV_LinkEdict(ppusher, FALSE);

			ppusher->state.ltime -= movetime;
			svs.dllfuncs.pfnDispatchBlocked(ppusher, pother);

			// Move back any entities we already moved
			for(Uint32 j = 0; j < g_serverPhysics.numsavedmovingents; j++)
			{
				saved_move_t* psaved = &g_serverPhysics.savedmovingentities[j];
				Math::VectorCopy(psaved->saved_origin, psaved->psave_edict->state.origin);
				SV_LinkEdict(psaved->psave_edict, FALSE);
			}

			return;
		}
	}
}

//=============================================
//
//=============================================
bool SV_RunThink( edict_t* pedict )
{
	if(!(pedict->state.flags & FL_KILLME))
	{
		Float thinktime = pedict->state.nextthink;

		// Not thinking yet
		if(thinktime <= 0 || thinktime > (svs.time + svs.frametime))
			return true;

		// Don't allow it to be less than frametime
		if(thinktime < svs.time)
			thinktime = svs.time;

		// This needs to be cleared BEFORE calling think functions
		// because the game dll might want to set a new think time
		pedict->state.nextthink = 0;
		svs.gamevars.time = thinktime;
		svs.dllfuncs.pfnDispatchThink(pedict);
	}

	// catch freed edicts here too
	if(pedict->state.flags & FL_KILLME)
		gEdicts.FreeEdict(pedict);

	return (pedict->free ? false : true);
}

//=============================================
//
//=============================================
void SV_Physics_ManageParentage( edict_t* pedict )
{
	if(!(pedict->state.flags & FL_PARENTED)
		|| pedict->state.parent == NO_ENTITY_INDEX)
		return;

	// Perform think functions
	SV_RunThink(pedict);

	Vector oldorigin = pedict->state.origin;
	Vector oldangles = pedict->state.angles;

	// Only really dumb shit for now
	edict_t* pparent = gEdicts.GetEdict(pedict->state.parent);
	if(pparent)
		Math::VectorAdd(pparent->state.origin, pedict->state.parentoffset, pedict->state.origin);

	// Set angles too if needed
	if(pedict->state.effects & EF_TRACKANGLES)
		pedict->state.angles = pparent->state.angles;

	// Update links if we moved
	if(oldorigin != pedict->state.origin
		|| oldangles != pedict->state.angles)
		SV_LinkEdict(pedict, true);
}

//=============================================
//
//=============================================
void SV_Physics_None( edict_t* pedict )
{
	// Peform think functions
	SV_RunThink(pedict);
}

//=============================================
//
//=============================================
void SV_Physics_Pusher( edict_t* pedict )
{
	Double thinktime = pedict->state.nextthink;
	Double oldtime = pedict->state.ltime;
	Double movetime;

	// Estimate movetime
	if(thinktime < oldtime + svs.frametime)
	{
		movetime = thinktime - pedict->state.ltime;
		if(movetime < 0)
			movetime = 0;
	}
	else
		movetime = svs.frametime;

	if(movetime > 0)
	{
		if(!Math::IsVectorZero(pedict->state.avelocity))
		{
			if(!Math::IsVectorZero(pedict->state.velocity))
			{
				if(SV_PushRotate(pedict, movetime))
				{
					Float savetime = pedict->state.ltime;
					// Reset the local time before rotation
					pedict->state.ltime = oldtime;
					SV_PushMove(pedict, movetime);

					if(pedict->state.ltime < savetime)
						pedict->state.ltime = savetime;
				}
			}
			else
			{
				SV_PushRotate(pedict, movetime);
			}
		}
		else
		{
			SV_PushMove(pedict, movetime);
		}
	}

	// Limit the angles
	for(Uint32 i = 0; i < 3; i++)
	{
		if(pedict->state.angles[i] < -3600 || pedict->state.angles[i] > 3600)
			pedict->state.angles[i] = SDL_fmod(pedict->state.angles[i], 3600);
	}

	// Avoid levelchange bugs
	if(!thinktime)
		return;

	// Call think functions
	if(thinktime > oldtime && !(pedict->state.flags & FL_KILLME)
		&& ((pedict->state.flags & FL_ALWAYSTHINK) || thinktime <= pedict->state.ltime))
	{
		// This needs to be cleared BEFORE calling think functions
		// because the game dll might want to set a new think time
		pedict->state.nextthink = 0;
		svs.gamevars.time = svs.time;
		svs.dllfuncs.pfnDispatchThink(pedict);
	}
}

//=============================================
//
//=============================================
void SV_Physics_Follow( edict_t* pedict )
{
	// Peform think functions
	if(!SV_RunThink(pedict))
		return;

	// Make sure the parent is valid
	if(pedict->state.aiment == NO_ENTITY_INDEX)
	{
		Con_Printf("Entity %s set to MOVETYPE_FOLLOW but with a nullptr aiment.\n", SV_GetString(pedict->fields.classname));
		pedict->state.movetype = MOVETYPE_NONE;
		return;
	}

	// Get parent ptr
	edict_t* pparent = gEdicts.GetEdict(pedict->state.aiment);
	if(!pparent || pparent->free)
	{
		Con_Printf("Entity %s with MOVETYPE_FOLLOW has an invalid aiment.\n", SV_GetString(pedict->fields.classname));
		pedict->state.movetype = MOVETYPE_NONE;
		return;
	}

	// Only bother if parent moved/changed angles
	if(pparent->state.origin == pedict->state.origin && pparent->state.angles == pedict->state.angles)
		return;

	// Set origin and angles
	Math::VectorAdd(pparent->state.origin, pedict->state.viewangles, pedict->state.origin);
	Math::VectorCopy(pparent->state.angles, pedict->state.angles);

	SV_LinkEdict(pedict, true);
}

//=============================================
//
//=============================================
void SV_Physics_Noclip( edict_t* pedict )
{
	// Peform think functions
	if(!SV_RunThink(pedict))
		return;

	// Only bother if we have an actual velocity
	if(Math::IsVectorZero(pedict->state.velocity) && Math::IsVectorZero(pedict->state.avelocity))
		return;

	Math::VectorMA(pedict->state.origin, svs.frametime, pedict->state.velocity, pedict->state.origin);
	Math::VectorMA(pedict->state.angles, svs.frametime, pedict->state.avelocity, pedict->state.angles);

	// Noclip entities don't touch triggers
	SV_LinkEdict(pedict, false);
}

//=============================================
//
//=============================================
void SV_Physics_Step( edict_t* pedict )
{
	SV_WaterMove(pedict);
	SV_CheckVelocity(pedict);

	bool prevonground = (pedict->state.flags & FL_ONGROUND) ? true : false;
	bool inwater = SV_CheckWater(pedict);

	if((pedict->state.flags & FL_FLOAT) && pedict->state.waterlevel > WATERLEVEL_NONE)
	{
		Float buoyancy = SV_Submerged(pedict) * pedict->state.skin * svs.frametime;

		SV_AddGravity(pedict);
		pedict->state.velocity[2] += buoyancy;
	}

	// Add gravity to anything by flying or swimming monsters
	if(!prevonground && !(pedict->state.flags & FL_FLY) && (!(pedict->state.flags & FL_SWIM) || pedict->state.waterlevel <= WATERLEVEL_NONE) && !inwater)
		SV_AddGravity(pedict);

	if(!Math::IsVectorZero(pedict->state.velocity) || !Math::IsVectorZero(pedict->state.basevelocity))
	{
		// Remove onground flag
		pedict->state.flags &= ~FL_ONGROUND;

		// apply friction
		if(prevonground && (pedict->state.health > 0 || SV_CheckBottom(pedict)))
		{
			Float speed = SDL_sqrt(pedict->state.velocity[0] * pedict->state.velocity[0] + 
				pedict->state.velocity[1] * pedict->state.velocity[1]);
			if(speed)
			{
				Float friction = g_psv_friction->GetValue() * pedict->state.friction;
				pedict->state.friction = 1.0;

				Float stopspeed = g_psv_stopspeed->GetValue();
				Float control = (speed < stopspeed) ? stopspeed : speed;
				Float newspeed = speed - (svs.frametime * control * friction);
				newspeed = _min(newspeed, 0);

				newspeed = newspeed / speed;
				pedict->state.velocity[0] *= newspeed;
				pedict->state.velocity[1] *= newspeed;
			}
		}

		Math::VectorAdd(pedict->state.velocity, pedict->state.basevelocity, pedict->state.velocity);
		SV_CheckVelocity(pedict);

		SV_FlyMove(pedict, svs.frametime, 1.0f);
		SV_CheckVelocity(pedict);

		Math::VectorSubtract(pedict->state.velocity, pedict->state.basevelocity, pedict->state.velocity);
		SV_CheckVelocity(pedict);

		// Determine if it's on the ground
		if(SV_CheckGround(pedict))
			pedict->state.flags |= FL_ONGROUND;

		SV_LinkEdict(pedict, true);
	}
	else if(svs.gamevars.force_retouch)
	{
		trace_t trace;
		SV_Move(trace, pedict->state.origin, pedict->state.mins, pedict->state.maxs, pedict->state.origin, FL_TRACE_NORMAL, pedict, (hull_types_t)pedict->state.forcehull);

		if(trace.fraction < 1 || trace.flags & FL_TR_STARTSOLID)
		{
			edict_t* phitedict = gEdicts.GetEdict(trace.hitentity);
			SV_Impact(pedict, phitedict, trace);
		}
	}

	if(!Math::IsVectorZero(pedict->state.avelocity))
		Math::VectorMA(pedict->state.angles, svs.frametime, pedict->state.avelocity, pedict->state.angles);

	// Run think code
	SV_RunThink(pedict);
	// Check water
	SV_CheckWaterTransition(pedict);
}

//=============================================
//
//=============================================
void SV_Physics_Toss( edict_t* pedict )
{
	SV_CheckWater(pedict);

	// Peform think functions
	if(!SV_RunThink(pedict))
		return;

	// Get groundent if set
	edict_t* pgroundent = nullptr;
	if(pedict->state.groundent != NO_ENTITY_INDEX)
		pgroundent = gEdicts.GetEdict(pedict->state.groundent);

	// Clear onground of we are moving upwards, or if our ground entity is an npc or a player
	if(pedict->state.velocity[2] > 0 || !pgroundent || (pgroundent->state.flags & (FL_NPC|FL_CLIENT)))
		pedict->state.flags &= ~FL_ONGROUND;

	// Don't do anything if we're on the ground and not moving
	if((pedict->state.flags & FL_ONGROUND) && Math::IsVectorZero(pedict->state.velocity))
	{
		pedict->state.avelocity.Clear();

		if(Math::IsVectorZero(pedict->state.basevelocity))
			return;
	}

	SV_CheckVelocity(pedict);
	SV_AddGravity(pedict);

	// Move angles and origin
	Math::VectorMA(pedict->state.angles, svs.frametime, pedict->state.avelocity, pedict->state.angles);
	Math::VectorAdd(pedict->state.velocity, pedict->state.basevelocity, pedict->state.velocity);
	SV_CheckVelocity(pedict);

	// Perform movement
	SV_FlyMove(pedict, svs.frametime, 1.1f);
	SV_CheckVelocity(pedict);

	Math::VectorSubtract(pedict->state.velocity, pedict->state.basevelocity, pedict->state.velocity);
	SV_CheckVelocity(pedict);

	// Determine if it's on the ground
	if(SV_CheckGround(pedict))
		pedict->state.flags |= FL_ONGROUND;

	// Link edict to world
	SV_LinkEdict(pedict, true);

	// Run think code
	SV_RunThink(pedict);
	// Check water
	SV_CheckWaterTransition(pedict);
}

//=============================================
//
//=============================================
void SV_Physics_Bounce( edict_t* pedict )
{
	SV_CheckWater(pedict);

	// Peform think functions
	if(!SV_RunThink(pedict))
		return;

	// Get groundent if set
	edict_t* pgroundent = nullptr;
	if(pedict->state.groundent != NO_ENTITY_INDEX)
		pgroundent = gEdicts.GetEdict(pedict->state.groundent);

	// Clear onground of we are moving upwards, or if our ground entity is an npc or a player
	if(pedict->state.velocity[2] > 0 || !pgroundent || (pgroundent->state.flags & (FL_NPC|FL_CLIENT)))
		pedict->state.flags &= ~FL_ONGROUND;

	// Don't do anything if we're on the ground and not moving
	if((pedict->state.flags & FL_ONGROUND) 
		&& Math::IsVectorZero(pedict->state.velocity))
	{
		pedict->state.avelocity.Clear();

		if(Math::IsVectorZero(pedict->state.basevelocity))
			return;
	}

	SV_CheckVelocity(pedict);

	// Add gravity if applicable
	if(pedict->state.movetype != MOVETYPE_FLY)
		SV_AddGravity(pedict);

	// Move angles and origin
	Math::VectorMA(pedict->state.angles, svs.frametime, pedict->state.avelocity, pedict->state.angles);
	Math::VectorAdd(pedict->state.velocity, pedict->state.basevelocity, pedict->state.velocity);
	SV_CheckVelocity(pedict);

	// Test movement
	Vector move;
	Math::VectorScale(pedict->state.velocity, svs.frametime, move);
	Math::VectorSubtract(pedict->state.velocity, pedict->state.basevelocity, pedict->state.velocity);

	trace_t trace;
	SV_PushEntity(pedict, move, trace);
	SV_CheckVelocity(pedict);

	// If we started in a solid object, it means we're stuck
	if(trace.flags & FL_TR_ALLSOLID)
	{
		pedict->state.avelocity.Clear();
		pedict->state.velocity.Clear();
		return;
	}

	// If we made it through, we can skip the rest
	if(trace.fraction == 1.0)
	{
		SV_CheckWaterTransition(pedict);
		return;
	}

	if(pedict->free)
		return;

	Float backoff;
	if(pedict->state.movetype == MOVETYPE_BOUNCE)
		backoff = 2.0f - pedict->state.friction;
	else
		backoff = 1.0f;

	// Clip velocity by the plane and the backoff value
	SV_ClipVelocity(pedict->state.velocity, trace.plane.normal, pedict->state.velocity, backoff);

	// Stop the entity if it's on ground
	if(trace.plane.normal[2] > 0.7f)
	{
		Math::VectorAdd(pedict->state.velocity, pedict->state.basevelocity, move);
		Float velocity = Math::DotProduct(move, move);

		if(move[2] < g_psv_gravity->GetValue() * svs.frametime)
		{
			// It's rolling on the ground, so add friction
			pedict->state.flags |= FL_ONGROUND;
			pedict->state.velocity[2] = 0;

			edict_t* phitedict = gEdicts.GetEdict(trace.hitentity);
			pedict->state.groundent = phitedict->entindex;
		}

		if(velocity < 900.0f || pedict->state.movetype != MOVETYPE_BOUNCE)
		{
			pedict->state.flags |= FL_ONGROUND;

			edict_t* phitedict = gEdicts.GetEdict(trace.hitentity);
			pedict->state.groundent = phitedict->entindex;

			pedict->state.avelocity.Clear();
			pedict->state.velocity.Clear();
		}
		else
		{
			Float scale = (1.0f - trace.fraction) * svs.frametime * 0.9f;
			Math::VectorScale(pedict->state.velocity, scale, move);
			Math::VectorMA(move, scale, pedict->state.basevelocity, move);

			SV_PushEntity(pedict, move, trace);
			if(pedict->free)
				return;
		}
	}

	// Check water
	SV_CheckWaterTransition(pedict);
}

//=============================================
//
//=============================================
void SV_Physics( void )
{
	// Set gamevars
	svs.gamevars.frametime = ens.frametime;
	svs.gamevars.time = svs.time;

	Uint32 nbEdicts = gEdicts.GetNbEdicts();
	for(Uint32 i = 0; i < nbEdicts; i++)
	{
		edict_t* pedict = gEdicts.GetEdict(i);
		if(pedict->free || (pedict->state.flags & FL_PARENTED))
			continue;

		// Force retouch for everything
		if(svs.gamevars.force_retouch)
			SV_LinkEdict(pedict, true);

		// Skip clients
		if(i > 0 && i <= svs.maxclients)
			continue;

		if(pedict->state.flags & FL_ONGROUND)
		{
			// Get groundent if set
			edict_t* pgroundentity = nullptr;
			if(pedict->state.groundent != NO_ENTITY_INDEX)
				pgroundentity = gEdicts.GetEdict(pedict->state.groundent);

			if(pgroundentity && (pgroundentity->state.flags & FL_CONVEYOR))
			{
				if(pedict->state.flags & FL_BASEVELOCITY)
					Math::VectorMA(pedict->state.basevelocity, pgroundentity->state.speed, pgroundentity->state.movedir, pedict->state.basevelocity);
				else
					Math::VectorScale(pgroundentity->state.movedir, pgroundentity->state.speed, pedict->state.basevelocity);

				pedict->state.flags |= FL_BASEVELOCITY;
			}
		}

		if(!(pedict->state.flags & FL_BASEVELOCITY))
		{
			Math::VectorMA(pedict->state.velocity, (svs.frametime * 0.5 + 1.0), pedict->state.basevelocity, pedict->state.velocity);
			Math::VectorClear(pedict->state.basevelocity);
		}

		pedict->state.flags &= ~FL_BASEVELOCITY;

		if(svs.dllfuncs.pfnRunEntityPhysics(pedict))
		{
			// Perform any think functions
			SV_RunThink(pedict);
		}
		else
		{
			switch(pedict->state.movetype)
			{
			case MOVETYPE_NONE:
				SV_Physics_None(pedict);
				break;
			case MOVETYPE_PUSH:
				SV_Physics_Pusher(pedict);
				break;
			case MOVETYPE_FOLLOW:
				SV_Physics_Follow(pedict);
				break;
			case MOVETYPE_NOCLIP:
				SV_Physics_Noclip(pedict);
				break;
			case MOVETYPE_STEP:
			case MOVETYPE_PUSHSTEP:
				SV_Physics_Step(pedict);
				break;
			case MOVETYPE_TOSS:
				SV_Physics_Toss(pedict);
				break;
			case MOVETYPE_FLY:
			case MOVETYPE_BOUNCE:
				SV_Physics_Bounce(pedict);
				break;
			default:
				Con_Printf("Unknown movetype %d for entity %s.\n", pedict->state.movetype, SV_GetString(pedict->fields.classname));
				break;
			}
		}

		// Kill the entity if flagged to do so
		if(pedict->state.flags & FL_KILLME)
			gEdicts.FreeEdict(pedict);
	}

	nbEdicts = gEdicts.GetNbEdicts();
	for(Uint32 i = 0; i < nbEdicts; i++)
	{
		edict_t* pedict = gEdicts.GetEdict(i);
		if(pedict->free || !(pedict->state.flags & FL_PARENTED))
			continue;

		SV_Physics_ManageParentage(pedict);
	}

	if(svs.gamevars.force_retouch)
		svs.gamevars.force_retouch = false;
}