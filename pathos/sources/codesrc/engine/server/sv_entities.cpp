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
#include "sv_world.h"
#include "system.h"

#include "enginestate.h"
#include "edictmanager.h"
#include "brushmodel.h"
#include "modelcache.h"
#include "frustum.h"
#include "sv_move.h"

//=============================================
//
//=============================================
bool SV_SetModel( edict_t* pedict, const Char* pstrFilepath, bool setbounds )
{
	if(!pstrFilepath || !qstrlen(pstrFilepath))
	{
		Con_Printf("%s - Empty model name specified for entity '%s'.\n", __FUNCTION__, SV_GetString(pedict->fields.classname));
		return false;
	}

	sv_model_t* psvmodel = nullptr;
	for(Uint32 i = 0; i < svs.modelcache.size(); i++)
	{
		if(!qstrcmp(svs.modelcache[i].modelname, pstrFilepath))
		{
			psvmodel = &svs.modelcache[i];
			break;
		}
	}

	cache_model_t* pmodel = nullptr;
	if(!psvmodel)
	{
		const byte* phashdata = reinterpret_cast<const byte*>(pstrFilepath);
		if(svs.promptshashlist.addhash(phashdata, qstrlen(pstrFilepath)))
			Con_Printf("%s - Model '%s' not precached for entity '%s'.\n", __FUNCTION__, pstrFilepath, SV_GetString(pedict->fields.classname));

		CString fileextension(pstrFilepath+qstrlen(pstrFilepath)-4);
		fileextension.tolower();

		if(!qstrcmp(fileextension, ".spr"))
		{
			if(!svs.perrorsprite)
				return false;

			pmodel = svs.perrorsprite;
			pedict->fields.modelname = SV_AllocString(svs.perrorsprite->name.c_str());
			pedict->state.modelindex = svs.perrorsprite->cacheindex;
		}
		else if(!qstrcmp(fileextension, ".mdl"))
		{
			if(!svs.perrormodel)
				return false;

			pmodel = svs.perrormodel;
			pedict->fields.modelname = SV_AllocString(svs.perrormodel->name.c_str());
			pedict->state.modelindex = svs.perrormodel->cacheindex;
		}
		else
		{
			// Shouldn't happen
			return false;
		}
	}
	else
	{
		pmodel = gModelCache.GetModelByIndex(psvmodel->cache_index);
		if(!pmodel)
		{
			const byte* phashdata = reinterpret_cast<const byte*>(pstrFilepath);
			if(svs.promptshashlist.addhash(phashdata, qstrlen(pstrFilepath)))
				Con_Printf("%s - Could not load '%s'.\n", __FUNCTION__, pstrFilepath);
			return false;
		}

		pedict->fields.modelname = SV_AllocString(pstrFilepath);
		pedict->state.modelindex = pmodel->cacheindex;
	}

	if(setbounds)
	{
		// Set size from model
		SV_SetMinsMaxs(pedict, pmodel->mins, pmodel->maxs);
	}

	return true;
}

//=============================================
//
//=============================================
void SV_SetMinsMaxs( edict_t* pedict, const Vector& mins, const Vector& maxs )
{
	for(Uint32 i = 0; i < 3; i++)
	{
		if(mins[i] > maxs[i])
		{
			CString errormsg;
			errormsg << __FUNCTION__ << " - Backwards mins maxs on " << SV_GetString(pedict->fields.classname) << " entity";
			if(pedict->fields.targetname != NO_STRING_VALUE)
				errormsg << "(" << SV_GetString(pedict->fields.targetname) << ")";

			Con_EPrintf(errormsg.c_str());
			break;
		}
	}

	pedict->state.mins = mins;
	pedict->state.maxs = maxs;

	Math::VectorAdd(pedict->state.origin, pedict->state.mins, pedict->state.absmin);
	Math::VectorAdd(pedict->state.origin, pedict->state.maxs, pedict->state.absmax);

	Math::VectorSubtract(maxs, mins, pedict->state.size);

	// Update the leafnums
	pedict->leafnums.clear();
	SV_LinkEdict(pedict, false);
}

//=============================================
//
//=============================================
void SV_SetSize( edict_t* pedict, const Vector& size )
{
	for(Uint32 i = 0; i < 3; i++)
	{
		pedict->state.mins[i] = size[i] + 1;
		pedict->state.maxs[i] = -size[i] - 1;
	}

	Math::VectorAdd(pedict->state.origin, pedict->state.mins, pedict->state.absmin);
	Math::VectorAdd(pedict->state.origin, pedict->state.maxs, pedict->state.absmax);

	pedict->state.size = size;

	// Update the leafnums
	pedict->leafnums.clear();
	SV_LinkEdict(pedict, false);
}

//=============================================
//
//=============================================
void SV_SetOrigin( edict_t* pedict, const Vector& origin )
{
	Math::VectorCopy(origin, pedict->state.origin);

	Math::VectorAdd(pedict->state.origin, pedict->state.mins, pedict->state.absmin);
	Math::VectorAdd(pedict->state.origin, pedict->state.maxs, pedict->state.absmax);

	// Update the leafnums
	pedict->leafnums.clear();
	SV_LinkEdict(pedict, false);
}

//=============================================
//
//=============================================
void SV_AddToTouched( entindex_t hitent, trace_t& trace, const Vector& velocity )
{
	if(hitent == NO_ENTITY_INDEX)
		return;

	for(Uint32 i = 0; i < svs.numpmovetraces; i++)
	{
		if(svs.pmovetraces[i].hitentity == hitent)
			return;
	}

	if(svs.numpmovetraces >= MAX_TOUCHENTS)
	{
		Con_Printf("%s - Exceeded MAX_TOUCHENTS on player %d.\n", __FUNCTION__, svs.pmoveplayerindex);
		return;
	}

	// Add to the stack
	Math::VectorCopy(velocity, trace.deltavelocity);
	svs.pmovetraces[svs.numpmovetraces] = trace;
	svs.numpmovetraces++;
}

//=============================================
//
//=============================================
Int32 SV_GetNumEdicts( void )
{
	return gEdicts.GetNbEdicts();
}

//=============================================
//
//=============================================
edict_t* SV_GetEdictByIndex( entindex_t entindex )
{
	if(entindex >= (Int32)gEdicts.GetNbEdicts())
	{
		Con_Printf("%s - Bogus entity index %d.\n", __FUNCTION__, entindex);
		return nullptr;
	}

	return gEdicts.GetEdict(entindex);
}

//=============================================
//
//=============================================
bool SV_InitPrivateData( edict_t* pedict, const Char* pstrClassname )
{
	// Init the gamedll interface
	pfnPrivateData_t pfn = reinterpret_cast<pfnPrivateData_t>(SDL_LoadFunction(svs.pdllhandle, pstrClassname));
	if(!pfn)
		return false;

	// Call the function
	pfn(pedict);

	// Call to declare save fields
	svs.dllfuncs.pfnDispatchDeclareSaveFields(pedict);

	return true;
}

//=============================================
//
//=============================================
edict_t* SV_CreateEntity( const Char* pstrClassName )
{
	if(ens.gamestate == GAME_INACTIVE)
	{
		Con_EPrintf("%s - Called on inactive game\n", __FUNCTION__);
		return nullptr;
	}

	if(!pstrClassName || !qstrlen(pstrClassName))
	{
		Con_EPrintf("%s - No classname specified.\n", __FUNCTION__);
		return nullptr;
	}

	// Allocate the edict
	edict_t* pedict = gEdicts.AllocEdict();
	if(!pedict)
	{
		Con_EPrintf("%s - Could not allocate edict for '%s'.\n", __FUNCTION__, pstrClassName);
		return nullptr;
	}

	if(!SV_InitPrivateData(pedict, pstrClassName))
	{
		Con_EPrintf("%s - Could not allocate private data for edict with classname '%s'.\n", __FUNCTION__, pstrClassName);
		gEdicts.FreeEdict(pedict);
		return nullptr;
	}

	// Set classname
	pedict->fields.classname = SV_AllocString(pstrClassName);

	return pedict;
}

//=============================================
//
//=============================================
void SV_RemoveEntity( edict_t* pentity )
{
	if(!pentity)
		return;

	gEdicts.FreeEdict(pentity);
}

//=============================================
//
//=============================================
bool SV_DropToFloor( edict_t* pentity )
{
	Vector traceEnd;
	traceEnd = pentity->state.origin - Vector(0, 0, 255);
	
	trace_t tr;
	Int32 traceFlags = FL_TRACE_NORMAL;
	if(pentity->state.flags & FL_NPC_CLIP)
		traceFlags |= FL_TRACE_NPC_CLIP;

	SV_Move(tr, pentity->state.origin, pentity->state.mins, pentity->state.maxs, traceEnd, traceFlags, pentity, (hull_types_t)pentity->state.forcehull);
	if(tr.allSolid() || tr.startSolid() || tr.noHit())
		return false;
	
	SV_SetOrigin(pentity, tr.endpos);
	pentity->state.flags |= FL_ONGROUND;
	pentity->state.groundent = tr.hitentity;

	return true;
}

//=============================================
//
//=============================================
bool SV_NPC_WalkMove( edict_t* pentity, Float yaw, Float dist, walkmove_t movemode )
{
	if(!(pentity->state.flags & (FL_SWIM|FL_FLY|FL_ONGROUND)))
		return false;

	Vector move;
	move[0] = SDL_cos(yaw*2.0f * M_PI/360.0f) * dist;
	move[1] = SDL_sin(yaw*2.0f * M_PI/360.0f) * dist;

	bool result = false;
	switch(movemode)
	{
	case WALKMOVE_WORLDONLY:
		result = SV_NPC_MoveTest(pentity, move, true);
		break;
	case WALKMOVE_CHECKONLY:
		result = SV_NPC_MoveStep(pentity, move, false, false);
		break;
	case WALKMOVE_NO_NPCS:
		result = SV_NPC_MoveStep(pentity, move, false, true);
		break;
	case WALKMOVE_NORMAL:
	default:
		result = SV_NPC_MoveStep(pentity, move, true, false);
		break;
	}

	return result;
}