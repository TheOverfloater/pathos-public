/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "player.h"
#include "textures_shared.h"
#include "materialdefs.h"
#include "game.h"
#include "flex_shared.h"
#include "flexmanager.h"
#include "ai_basenpc.h"
#include "funcdoor.h"
#include "beam_shared.h"

// Number of glass debris sounds
static const Uint32 NB_GLASS_DEBRIS_SOUNDS = 3;
// Glass debris sounds
static const Char* GLASS_DEBRIS_SOUNDS[NB_GLASS_DEBRIS_SOUNDS] =
{
	"debris/glass_clatter1.wav",
	"debris/glass_clatter2.wav",
	"debris/glass_clatter3.wav"
};

// Number of wood debris sounds
static const Uint32 NB_WOOD_DEBRIS_SOUNDS = 3;
// Wood debris sounds
static const Char* WOOD_DEBRIS_SOUNDS[NB_WOOD_DEBRIS_SOUNDS] =
{
	"debris/wood_clatter1.wav",
	"debris/wood_clatter2.wav",
	"debris/wood_clatter3.wav"
};

// Number of metal debris sounds
static const Uint32 NB_METAL_DEBRIS_SOUNDS = 3;
// Metal debris sounds
static const Char* METAL_DEBRIS_SOUNDS[NB_METAL_DEBRIS_SOUNDS] =
{
	"debris/metal_clatter1.wav",
	"debris/metal_clatter2.wav",
	"debris/metal_clatter3.wav"
};

// Number of flesh debris sounds
static const Uint32 NB_FLESH_DEBRIS_SOUNDS = 7;
// Flesh debris sounds
static const Char* FLESH_DEBRIS_SOUNDS[NB_FLESH_DEBRIS_SOUNDS] =
{
	"debris/flesh_splatter1.wav",
	"debris/flesh_splatter2.wav",
	"debris/flesh_splatter3.wav",
	"debris/flesh_splatter4.wav",
	"debris/flesh_splatter5.wav",
	"debris/flesh_splatter6.wav",
	"debris/flesh_splatter7.wav"
};

// Number of concrete debris sounds
static const Uint32 NB_CONCRETE_DEBRIS_SOUNDS = 3;
// Concrete debris sounds
static const Char* CONCRETE_DEBRIS_SOUNDS[NB_CONCRETE_DEBRIS_SOUNDS] =
{
	"debris/concrete_clatter1.wav",
	"debris/concrete_clatter2.wav",
	"debris/concrete_clatter3.wav"
};

namespace Util
{
	//=============================================
	//
	//=============================================
	void RemoveEntity( CBaseEntity* pEntity )
	{
		if(!pEntity)
			return;

		pEntity->SetFlags(FL_KILLME|FL_REMOVE_ON_SPAWN);
		pEntity->SetTargetName(NO_STRING_VALUE);
	}

	//=============================================
	//
	//=============================================
	void RemoveEntity( edict_t* pEdict )
	{
		if(!pEdict)
			return;

		pEdict->state.flags |= (FL_KILLME|FL_REMOVE_ON_SPAWN);
		pEdict->fields.targetname = NO_STRING_VALUE;
	}

	//=============================================
	//
	//=============================================
	edict_t* FindEntityByString( edict_t* pStartEntity, const Char* pstrFieldName, const Char* pstrValue )
	{
		return ::FindEntityByString(pStartEntity, pstrFieldName, pstrValue);
	}

	//=============================================
	//
	//=============================================
	edict_t* FindEntityByClassname( edict_t* pStartEntity, const Char* pstrValue )
	{
		return Util::FindEntityByString(pStartEntity, "classname", pstrValue);
	}

	//=============================================
	//
	//=============================================
	edict_t* FindEntityByTargetName( edict_t* pStartEntity, const Char* pstrValue )
	{
		return Util::FindEntityByString(pStartEntity, "targetname", pstrValue);
	}

	//=============================================
	//
	//=============================================
	edict_t* FindEntityByTarget( edict_t* pStartEntity, const Char* pstrValue )
	{
		return Util::FindEntityByString(pStartEntity, "target", pstrValue);
	}

	//=============================================
	//
	//=============================================
	edict_t* FindEntityInBBox( edict_t* pStartEntity, const Vector& mins, const Vector& maxs )
	{
		Int32 i;
		if(pStartEntity)
			i = pStartEntity->entindex+1;
		else
			i = 0;

		for(; i < gd_engfuncs.pfnGetNbEdicts(); i++)
		{
			edict_t* pedict = gd_engfuncs.pfnGetEdictByIndex(i);
			if(!pedict || pedict->free)
				continue;

			if(!Math::CheckMinsMaxs(pedict->state.absmin, pedict->state.absmax, mins, maxs))
				return pedict;
		}

		return nullptr;
	}

	//=============================================
	//
	//=============================================
	void SetMoveDirection( entity_state_t& state )
	{
		if(state.angles == Vector(0, -1, 0))
			state.movedir = Vector(0, 0, 1);
		else if(state.angles == Vector(0, -2, 0))
			state.movedir = Vector(0, 0, -1);
		else
			Math::AngleVectors(state.angles, &state.movedir, nullptr, nullptr);

		state.angles = ZERO_VECTOR;
	}

	//=============================================
	//
	//=============================================
	void SetAxisDirection( entity_state_t& state, Int32 flags, Int32 flagz, Int32 flagx )
	{
		if(flags & flagz)
			state.movedir = Vector(0, 0, 1);
		else if(flags & flagx)
			state.movedir = Vector(1, 0, 0);
		else
			state.movedir = Vector(0, 1, 0);
	}

	//=============================================
	//
	//=============================================
	Float GetAxisValue( Int32 flags, const Vector& angles, Int32 flagz, Int32 flagx )
	{
		if(flags & flagz)
			return angles.z;
		else if(flags & flagx)
			return angles.x;
		else
			return angles.y;
	}

	//=============================================
	//
	//=============================================
	Float GetAxisDelta( Int32 flags, const Vector& angle1, const Vector& angle2, Int32 flagz, Int32 flagx )
	{
		if(flags & flagz)
			return angle1.z - angle2.z;
		else if(flags & flagx)
			return angle1.x - angle2.x;
		else
			return angle1.y - angle2.y;
	}

	//=============================================
	//
	//=============================================
	void TraceLine( const Vector& start, const Vector& end, bool ignorenpcs, bool usehitboxes, bool ignoreglass, bool hitcorpses, const edict_t* pignoreent, trace_t& tr )
	{
		Int32 traceflags = FL_TRACE_NORMAL;
		if(ignorenpcs)
			traceflags |= FL_TRACE_NO_NPCS;
		if(ignoreglass)
			traceflags |= FL_TRACE_NO_TRANS;
		if(hitcorpses)
			traceflags |= FL_TRACE_HIT_CORPSES;
		if(usehitboxes)
			traceflags |= FL_TRACE_HITBOXES;

		gd_tracefuncs.pfnTraceLine(start, end, traceflags, HULL_POINT, pignoreent ? pignoreent->entindex : NO_ENTITY_INDEX, tr);
	}

	//=============================================
	//
	//=============================================
	void TraceLine( const Vector& start, const Vector& end, bool ignorenpcs, bool usehitboxes, bool ignoreglass, const edict_t* pignoreent, trace_t& tr )
	{
		Int32 traceflags = FL_TRACE_NORMAL;
		if(ignorenpcs)
			traceflags |= FL_TRACE_NO_NPCS;
		if(ignoreglass)
			traceflags |= FL_TRACE_NO_TRANS;
		if(usehitboxes)
			traceflags |= FL_TRACE_HITBOXES;

		gd_tracefuncs.pfnTraceLine(start, end, traceflags, HULL_POINT, pignoreent ? pignoreent->entindex : NO_ENTITY_INDEX, tr);
	}

	//=============================================
	//
	//=============================================
	void TraceLine( const Vector& start, const Vector& end, bool ignorenpcs, bool usehitboxes, const edict_t* pignoreent, trace_t& tr )
	{
		Int32 traceflags = FL_TRACE_NORMAL;
		if(ignorenpcs)
			traceflags |= FL_TRACE_NO_NPCS;
		if(usehitboxes)
			traceflags |= FL_TRACE_HITBOXES;

		gd_tracefuncs.pfnTraceLine(start, end, traceflags, HULL_POINT, pignoreent ? pignoreent->entindex : NO_ENTITY_INDEX, tr);
	}

	//=============================================
	//
	//=============================================
	void TraceHull( const Vector& start, const Vector& end, bool ignorenpcs, bool usehitboxes, bool ignoreglass, hull_types_t hulltype, const edict_t* pignoreent, trace_t& tr )
	{
		Int32 traceflags = FL_TRACE_NORMAL;
		if(ignorenpcs)
			traceflags |= FL_TRACE_NO_NPCS;
		if(ignoreglass)
			traceflags |= FL_TRACE_NO_TRANS;
		if(usehitboxes)
			traceflags |= FL_TRACE_HITBOXES;

		gd_tracefuncs.pfnTraceLine(start, end, traceflags, hulltype, pignoreent ? pignoreent->entindex : NO_ENTITY_INDEX, tr);
	}

	//=============================================
	//
	//=============================================
	void TraceHull( const Vector& start, const Vector& end, bool ignorenpcs, bool usehitboxes, hull_types_t hulltype, const edict_t* pignoreent, trace_t& tr )
	{
		Int32 traceflags = FL_TRACE_NORMAL;
		if(ignorenpcs)
			traceflags |= FL_TRACE_NO_NPCS;
		if(usehitboxes)
			traceflags |= FL_TRACE_HITBOXES;

		gd_tracefuncs.pfnTraceLine(start, end, traceflags, hulltype, pignoreent ? pignoreent->entindex : NO_ENTITY_INDEX, tr);
	}

	//=============================================
	//
	//=============================================
	void TraceModel( const CBaseEntity* pEntity, const Vector& start, const Vector& end, bool usehitboxes, hull_types_t hulltype, trace_t& tr )
	{
		Int32 traceflags = FL_TRACE_NORMAL;
		if(usehitboxes)
			traceflags |= FL_TRACE_HITBOXES;

		gd_tracefuncs.pfnTraceModel(pEntity->GetEntityIndex(), start, end, hulltype, traceflags, tr);
	}

	//=============================================
	//
	//=============================================
	const Char* TraceTexture( entindex_t hitentity, const Vector& position, const Vector& planeNormal )
	{
		if(hitentity == NO_ENTITY_INDEX)
			return nullptr;

		Vector start = position + planeNormal;
		Vector end = position - planeNormal;
		return gd_tracefuncs.pfnTraceTexture(hitentity, start, end);
	}

	//=============================================
	//
	//=============================================
	void FireTargets( const Char* pstrtargetname, CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
	{
		if(!pstrtargetname || !qstrlen(pstrtargetname))
			return;

		edict_t* ptarget = nullptr;
		while(true)
		{
			ptarget = Util::FindEntityByTargetName(ptarget, pstrtargetname);
			if(!ptarget)
				break;

			if(!Util::IsNullEntity(ptarget))
			{
				if(g_pCvarTriggerDebug->GetValue() >= 1)
					Util::EntityConDPrintf(pCaller->GetEdict(), "Firing target '%s'(%s).\n", gd_engfuncs.pfnGetString(ptarget->fields.targetname), gd_engfuncs.pfnGetString(ptarget->fields.classname));

				CBaseEntity* pentity = CBaseEntity::GetClass(ptarget);
				pentity->CallUse(pActivator, pCaller, useMode, value); 
			}
		}
	}

	//=============================================
	//
	//=============================================
	bool IsMasterTriggered( const Char* pstrtargetname, const CBaseEntity* pentity, const CBaseEntity* pslave )
	{
		if(!pstrtargetname || !qstrlen(pstrtargetname))
			return true;
	
		edict_t* ptargetentity = FindEntityByTargetName(nullptr, pstrtargetname);
		if(Util::IsNullEntity(ptargetentity))
		{
			gd_engfuncs.pfnCon_VPrintf("%s - Master was null for %s entity '%s'.\n", 
				__FUNCTION__, 
				pslave->GetClassName(), 
				pslave->GetTargetName());
			return true;
		}

		CBaseEntity* pMaster = CBaseEntity::GetClass(ptargetentity);
		if(pMaster && (pMaster->GetEntityFlags() & FL_ENTITY_MASTER))
			return pMaster->IsTriggered(pentity);
		else
		{
			gd_engfuncs.pfnCon_DPrintf("%s - Master was not a master for %s entity '%s'.\n", 
				__FUNCTION__, 
				pslave->GetClassName(), 
				pslave->GetTargetName());
			return true;
		}
	}

	//=============================================
	//
	//=============================================
	bool IsNullEntity( const edict_t* pentity )
	{
		if(!pentity || pentity->free || !pentity->pprivatedata || pentity->state.flags & FL_KILLME)
			return true;
		else
			return false;
	}

	//=============================================
	//
	//=============================================
	bool IsNullEntity( const entindex_t entindex )
	{
		if(entindex == NO_ENTITY_INDEX)
			return true;

		edict_t* pedict = gd_engfuncs.pfnGetEdictByIndex(entindex);
		return Util::IsNullEntity(pedict);
	}

	//=============================================
	//
	//=============================================
	bool IsVBMEntity( const edict_t* pentity )
	{
		if(IsNullEntity(pentity) || !pentity->state.modelindex)
			return false;

		const cache_model_t* pmodel = gd_engfuncs.pfnGetModel(pentity->state.modelindex);
		return (gd_engfuncs.pfnGetModelType(*pmodel) == MOD_VBM) ? true : false;
	}

	//=============================================
	//
	//=============================================
	bool IsVBMEntity( const entindex_t entindex )
	{
		if(entindex == NO_ENTITY_INDEX)
			return true;

		edict_t* pedict = gd_engfuncs.pfnGetEdictByIndex(entindex);
		return Util::IsVBMEntity(pedict);
	}

	//=============================================
	//
	//=============================================
	CBaseEntity* GetHostPlayer( void )
	{
		// Host player is always at index 1
		edict_t* pedict = gd_engfuncs.pfnGetEdictByIndex(HOST_CLIENT_ENTITY_INDEX);
		if(!pedict)
			return nullptr;

		if(!pedict->pprivatedata)
			return nullptr;

		CBaseEntity* pPlayer = CBaseEntity::GetClass(pedict);
		return pPlayer;
	}

	//=============================================
	//
	//=============================================
	CBaseEntity* GetPlayerByIndex( Uint32 index )
	{
		edict_t* pedict = gd_engfuncs.pfnGetEdictByIndex(HOST_CLIENT_ENTITY_INDEX+index);
		if(Util::IsNullEntity(pedict))
			return nullptr;
		
		if(pedict->clientindex == NO_CLIENT_INDEX)
			return nullptr;

		return CBaseEntity::GetClass(pedict);
	}

	//=============================================
	//
	//=============================================
	void WarnEmptyEntity( const edict_t* pentity )
	{
		const Char* pstrclassname = gd_engfuncs.pfnGetString(pentity->fields.classname);
		const Vector& origin = pentity->state.origin;

		if(pentity->fields.targetname != NO_STRING_VALUE)
		{
			const Char* pstrtargetname = gd_engfuncs.pfnGetString(pentity->fields.targetname);
			gd_engfuncs.pfnCon_Printf("Warning! - Empty %s(%s) at %.0f %.0f %.0f.\n", pstrclassname, pstrtargetname, origin.x, origin.y, origin.z);
		}
		else
			gd_engfuncs.pfnCon_Printf("Warning! - Empty %s at %.0f %.0f %.0f.\n", pstrclassname, origin.x, origin.y, origin.z);
	}

	//=============================================
	//
	//=============================================
	void EntityConPrintf( const edict_t* pentity, const Char *fmt, ... )
	{
		va_list	vArgPtr;
		Char cMsg[MAX_PATH];
	
		va_start(vArgPtr, fmt);
		vsprintf_s(cMsg, fmt, vArgPtr);
		va_end(vArgPtr);

		if(pentity->fields.targetname != NO_STRING_VALUE)
			gd_engfuncs.pfnCon_Printf("Entity %s(%s) - %s", gd_engfuncs.pfnGetString(pentity->fields.classname), gd_engfuncs.pfnGetString(pentity->fields.targetname), cMsg);
		else
			gd_engfuncs.pfnCon_Printf("Entity %s - %s", gd_engfuncs.pfnGetString(pentity->fields.classname), cMsg);
	}

	//=============================================
	//
	//=============================================
	void EntityConDPrintf( const edict_t* pentity, const Char *fmt, ... )
	{
		va_list	vArgPtr;
		Char cMsg[MAX_PATH];
	
		va_start(vArgPtr, fmt);
		vsprintf_s(cMsg, fmt, vArgPtr);
		va_end(vArgPtr);

		if(pentity->fields.targetname != NO_STRING_VALUE)
			gd_engfuncs.pfnCon_DPrintf("Entity %s(%s) - %s", gd_engfuncs.pfnGetString(pentity->fields.classname), gd_engfuncs.pfnGetString(pentity->fields.targetname), cMsg);
		else
			gd_engfuncs.pfnCon_DPrintf("Entity %s - %s", gd_engfuncs.pfnGetString(pentity->fields.classname), cMsg);
	}

	//=============================================
	//
	//=============================================
	void CreateParticles( const Char* pstrscriptname, const Vector& origin, const Vector& direction, part_script_type_t type )
	{
		if(!pstrscriptname || !qstrlen(pstrscriptname))
			return;

		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.createparticlesystem, nullptr, nullptr);

		for(Uint32 i = 0; i < 3; i++)
			gd_engfuncs.pfnMsgWriteFloat(origin[i]);

		for(Uint32 i = 0; i < 3; i++)
			gd_engfuncs.pfnMsgWriteSmallFloat(direction[i]*360.0f);

		gd_engfuncs.pfnMsgWriteByte(type);
		gd_engfuncs.pfnMsgWriteString(pstrscriptname);
		gd_engfuncs.pfnMsgWriteInt32(0);
		gd_engfuncs.pfnMsgWriteInt32(0);
		gd_engfuncs.pfnMsgWriteByte(0);
		gd_engfuncs.pfnMsgWriteInt16(NO_POSITION);
		gd_engfuncs.pfnMsgWriteByte(PARTICLE_ATTACH_NONE);
		gd_engfuncs.pfnUserMessageEnd();
	}

	//=============================================
	//
	//=============================================
	void CreateParticles( const Char* pstrscriptname, const Vector& origin, const Vector& direction, part_script_type_t type, const edict_t* pentity, Uint32 attachment, Int32 id, Int32 boneindex, Int32 attachflags )
	{
		if(!pstrscriptname || !qstrlen(pstrscriptname))
			return;

		if(Util::IsNullEntity(pentity))
			return;

		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.createparticlesystem, nullptr, nullptr);

		for(Uint32 i = 0; i < 3; i++)
			gd_engfuncs.pfnMsgWriteFloat(origin[i]);

		for(Uint32 i = 0; i < 3; i++)
			gd_engfuncs.pfnMsgWriteSmallFloat(direction[i]*360.0f);

		gd_engfuncs.pfnMsgWriteByte(type);
		gd_engfuncs.pfnMsgWriteString(pstrscriptname);
		gd_engfuncs.pfnMsgWriteInt32(id);
		gd_engfuncs.pfnMsgWriteInt32(pentity->entindex);
		gd_engfuncs.pfnMsgWriteByte(attachment);
		gd_engfuncs.pfnMsgWriteInt16(boneindex);
		gd_engfuncs.pfnMsgWriteByte(attachflags);
		gd_engfuncs.pfnUserMessageEnd();
	}

	//=============================================
	//
	//=============================================
	void PrecacheEntity( const Char* pstrClassname )
	{
		edict_t* pedict = gd_engfuncs.pfnCreateEntity(pstrClassname);
		if(!pedict)
		{
			gd_engfuncs.pfnCon_Printf("Entity '%s' could not be precached.\n", pstrClassname);
			return;
		}

		CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
		if(pEntity)
			pEntity->Spawn();

		RemoveEntity(pedict);
	}

	//=============================================
	//
	//=============================================
	void ShowMessage( const Char* pmsgname, CBaseEntity* pPlayer )
	{
		if(!pPlayer || !pPlayer->IsPlayer())
			return;

		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.showmessage, nullptr, pPlayer->GetEdict());
		gd_engfuncs.pfnMsgWriteString(pmsgname);
		gd_engfuncs.pfnUserMessageEnd();
	}

	//=============================================
	//
	//=============================================
	void ShowMessageAllPlayers( const Char* pmsgname )
	{
		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.showmessage, nullptr, nullptr);
		gd_engfuncs.pfnMsgWriteString(pmsgname);
		gd_engfuncs.pfnUserMessageEnd();
	}

	//=============================================
	//
	//=============================================
	void EmitEntitySound( const CBaseEntity* pEntity, const Char* pstrfilename, snd_channels_t channel, Float volume, Float attenuation, Int32 pitch, Int32 flags, const CBaseEntity* pPlayer, Float timeoffset )
	{
		if(!pEntity)
			return;

		gd_engfuncs.pfnPlayEntitySound(pEntity->GetEntityIndex(), pstrfilename, flags, channel, volume, attenuation, pitch, timeoffset, NO_CLIENT_INDEX);
	}

	//=============================================
	//
	//=============================================
	void EmitEntitySound( const CBaseEntity* pEntity, string_t filename, snd_channels_t channel, Float volume, Float attenuation, Int32 pitch, Int32 flags, const CBaseEntity* pPlayer, Float timeoffset )
	{
		if(!pEntity)
			return;

		if(filename == NO_STRING_VALUE)
			return;

		const Char* pstrfilename = gd_engfuncs.pfnGetString(filename);
		if(!pstrfilename || !qstrlen(pstrfilename))
			return;

		gd_engfuncs.pfnPlayEntitySound(pEntity->GetEntityIndex(), pstrfilename, flags, channel, volume, attenuation, pitch, timeoffset, NO_CLIENT_INDEX);
	}

	//=============================================
	//
	//=============================================
	void EmitAmbientSound( const Vector& origin, const Char* pstrfilename, Float volume, Float attenuation, Int32 pitch, Int32 flags, const CBaseEntity* pEntity, const CBaseEntity* pPlayer, Float timeoffset )
	{
		entindex_t entityindex;
		if(pEntity)
			entityindex = pEntity->GetEntityIndex();
		else
			entityindex = 0;

		gd_engfuncs.pfnPlayAmbientSound(entityindex, pstrfilename, origin, flags, volume, attenuation, pitch, timeoffset, NO_CLIENT_INDEX);
	}

	//=============================================
	//
	//=============================================
	void EmitAmbientSound( const Vector& origin, string_t filename, Float volume, Float attenuation, Int32 pitch, Int32 flags, const CBaseEntity* pEntity, const CBaseEntity* pPlayer, Float timeoffset )
	{
		if(filename == NO_STRING_VALUE)
			return;

		const Char* pstrfilename = gd_engfuncs.pfnGetString(filename);
		if(!pstrfilename || !qstrlen(pstrfilename))
			return;

		entindex_t entityindex;
		if(pEntity)
			entityindex = pEntity->GetEntityIndex();
		else
			entityindex = 0;

		Int32 clientindex;
		if(pPlayer && pPlayer->IsPlayer())
			clientindex = pPlayer->GetClientIndex();
		else
			clientindex = NO_CLIENT_INDEX;

		gd_engfuncs.pfnPlayAmbientSound(entityindex, pstrfilename, origin, flags, volume, attenuation, pitch, timeoffset, clientindex);
	}

	//=============================================
	//
	//=============================================
	void StopEntitySounds( CBaseEntity* pEntity, snd_channels_t channel, const CBaseEntity* pPlayer )
	{
		entindex_t entityindex;
		if(pEntity)
			entityindex = pEntity->GetEntityIndex();
		else
			entityindex = 0;

		Int32 clientindex;
		if(pPlayer && pPlayer->IsPlayer())
			clientindex = pPlayer->GetClientIndex();
		else
			clientindex = NO_CLIENT_INDEX;

		gd_engfuncs.pfnStopEntitySounds(entityindex, channel, clientindex);
	}

	//=============================================
	//
	//=============================================
	void SpawnBloodParticles( const trace_t& tr, bloodcolor_t color, bool isplayer )
	{
		if(color == BLOOD_NONE)
			return;
		
		Int32 attachflags = PARTICLE_ATTACH_NONE;
		Int32 boneindex = Util::GetBoneIndexFromTrace(tr);
		if(boneindex != NO_POSITION)
			attachflags |= (PARTICLE_ATTACH_TO_BONE|PARTICLE_ATTACH_TO_PARENT);

		CBaseEntity* pEntity = Util::GetEntityFromTrace(tr);
		if(!pEntity)
			return;
		
		CString scriptname;
		if(isplayer)
		{
			// Player has specific script
			scriptname = "blood_effects_cluster_player.txt";
		}
		else
		{
			if(pEntity->IsAlive())
				scriptname = "blood_effects_cluster_living.txt";
			else
				scriptname = "blood_effects_cluster.txt";
		}

		Util::CreateParticles(scriptname.c_str(), tr.endpos, tr.plane.normal, PART_SCRIPT_CLUSTER, pEntity->GetEdict(), 0, 0, boneindex, attachflags);
	}

	//=============================================
	//
	//=============================================
	void SpawnBloodParticles( const Vector& origin, const Vector& direction, bloodcolor_t color, bool isplayer )
	{
		if(color == BLOOD_NONE)
			return;

		CString scriptname = isplayer ? "blood_effects_cluster_player.txt" : "blood_effects_cluster.txt";
		Util::CreateParticles(scriptname.c_str(), origin, direction, PART_SCRIPT_CLUSTER);
	}

	//=============================================
	//
	//=============================================
	void SpawnBloodDecal( const trace_t& tr, bloodcolor_t color, bool decalvbm )
	{
		if(color != BLOOD_RED)
			return;

		Int32 decalflags = FL_DECAL_SPECIFIC_TEXTURE;
		if(decalvbm)
			decalflags |= FL_DECAL_VBM;

		decalgroupentry_t* pentry = gDecalList.GetRandom("redblood");
		if(!pentry)
		{
			gd_engfuncs.pfnCon_Printf("%s - Couldn't find decal group '%s'.\n", __FUNCTION__, "redblood");
			return;
		}

		// Create on client
		Util::CreateGenericDecal(tr.endpos, &tr.plane.normal, pentry->name.c_str(), decalflags);

		// Add to save-restore
		gd_engfuncs.pfnAddSavedDecal(tr.endpos, tr.plane.normal, tr.hitentity, pentry->name.c_str(), decalflags);
	}

	//=============================================
	//
	//=============================================
	void CreateGenericDecal( const Vector& origin, const Vector* pnormal, const Char* pstrname, Int32 decalflags, entindex_t entindex, Float life, Float fadetime, Float growthtime, const edict_t* pplayer )
	{
		Uint16 flags = decalflags | FL_DECAL_SERVER;
		if(pnormal)
			flags |= FL_DECAL_HAS_NORMAL;
		if(entindex != NO_ENTITY_INDEX)
			flags |= FL_DECAL_TIED_TO_ENTITY;

		if(!(flags & FL_DECAL_PERSISTENT))
		{
			if(life > 0 || fadetime > 0)
				flags |= FL_DECAL_DIE;

			if(fadetime > 0)
				flags |= FL_DECAL_DIE_FADE;

			if(growthtime > 0)
				flags |= FL_DECAL_GROW;
		}

		if(pplayer)
			gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.creategenericdecal, nullptr, pplayer);
		else
			gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.creategenericdecal, nullptr, nullptr);

		gd_engfuncs.pfnMsgWriteUint16(flags);
		for(Uint32 i = 0; i < 3; i++)
			gd_engfuncs.pfnMsgWriteFloat(origin[i]);
		gd_engfuncs.pfnMsgWriteString(pstrname);
		if(pnormal)
		{
			for(Uint32 i = 0; i < 3; i++)
				gd_engfuncs.pfnMsgWriteSmallFloat((*pnormal)[i]*360.0f);
		}
			
		if(flags & FL_DECAL_TIED_TO_ENTITY)
			gd_engfuncs.pfnMsgWriteInt32(entindex);

		if(flags & FL_DECAL_DIE)
			gd_engfuncs.pfnMsgWriteSmallFloat(life);

		if(flags & FL_DECAL_DIE_FADE)
			gd_engfuncs.pfnMsgWriteSmallFloat(fadetime);

		if(flags & FL_DECAL_GROW)
			gd_engfuncs.pfnMsgWriteSmallFloat(growthtime);

		gd_engfuncs.pfnUserMessageEnd();
	}

	//=============================================
	//
	//=============================================
	void CreateVBMDecal( const Vector& origin, const Vector& normal, const Char* pstrname, const edict_t* pentity, Int32 decalflags )
	{
		if(Util::IsNullEntity(pentity))
			return;

		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.createvbmdecal, nullptr, nullptr);
		gd_engfuncs.pfnMsgWriteInt16(pentity->entindex);
		for(Uint32 i = 0; i < 3; i++)
			gd_engfuncs.pfnMsgWriteFloat(origin[i]);
		for(Uint32 i = 0; i < 3; i++)
			gd_engfuncs.pfnMsgWriteSmallFloat(normal[i]*360.0f);
		gd_engfuncs.pfnMsgWriteString(pstrname);
		gd_engfuncs.pfnMsgWriteByte(decalflags);
		gd_engfuncs.pfnUserMessageEnd();
	}

	//=============================================
	//
	//=============================================
	Float GetDamageForce( const edict_t& entity, Float damage )
	{
		Float force = damage*((32*32*72)/(entity.state.size.x * entity.state.size.y * entity.state.size.z))*5;
		if(force > 1000.0f)
			force = 1000.0f;

		return force;
	}

	//=============================================
	//
	//=============================================
	void ScreenFadePlayer( const edict_t* pplayer, const Vector& color, Float fadetime, Float fadeholdtime, Int32 alpha, Int32 flags, Int32 layer, Float timeoffset )
	{
		color24_t _color(color.x, color.y, color.z);
		ScreenFadePlayer(pplayer, _color, fadetime, fadeholdtime, alpha, flags, layer, timeoffset);
	}

	//=============================================
	//
	//=============================================
	void ScreenFadePlayer( const edict_t* pplayer, const color24_t& color, Float fadetime, Float fadeholdtime, Int32 alpha, Int32 flags, Int32 layer, Float timeoffset )
	{
		if(!pplayer || !(pplayer->state.flags & FL_CLIENT))
			return;

		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.screenfade, nullptr, pplayer);
			gd_engfuncs.pfnMsgWriteSmallFloat(fadetime*100);
			gd_engfuncs.pfnMsgWriteSmallFloat(fadeholdtime*100);
			gd_engfuncs.pfnMsgWriteInt16(flags);
			gd_engfuncs.pfnMsgWriteByte(color.r);
			gd_engfuncs.pfnMsgWriteByte(color.g);
			gd_engfuncs.pfnMsgWriteByte(color.b);
			gd_engfuncs.pfnMsgWriteByte(alpha);
			gd_engfuncs.pfnMsgWriteByte(layer);
			gd_engfuncs.pfnMsgWriteSmallFloat(timeoffset);
		gd_engfuncs.pfnUserMessageEnd();
	}

	//=============================================
	//
	//=============================================
	void ScreenFadeAllPlayers( const Vector& color, Float fadetime, Float fadeholdtime, Int32 alpha, Int32 flags, Int32 layer, Float timeoffset )
	{
		color24_t _color(color.x, color.y, color.z);
		ScreenFadeAllPlayers(_color, fadetime, fadeholdtime, alpha, flags, layer, timeoffset);
	}

	//=============================================
	//
	//=============================================
	void ScreenFadeAllPlayers( const color24_t& color, Float fadetime, Float fadeholdtime, Int32 alpha, Int32 flags, Int32 layer, Float timeoffset )
	{
		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.screenfade, nullptr, nullptr);
			gd_engfuncs.pfnMsgWriteSmallFloat(fadetime*100);
			gd_engfuncs.pfnMsgWriteSmallFloat(fadeholdtime*100);
			gd_engfuncs.pfnMsgWriteInt16(flags);
			gd_engfuncs.pfnMsgWriteByte(color.r);
			gd_engfuncs.pfnMsgWriteByte(color.g);
			gd_engfuncs.pfnMsgWriteByte(color.b);
			gd_engfuncs.pfnMsgWriteByte(alpha);
			gd_engfuncs.pfnMsgWriteByte(layer);
			gd_engfuncs.pfnMsgWriteSmallFloat(timeoffset);
		gd_engfuncs.pfnUserMessageEnd();
	}

	//=============================================
	//
	//=============================================
	void Ricochet( const Vector &position, const Vector &direction, bool metalsound )
	{
		Util::CreateParticles("cluster_impact_metal.txt", position, direction, PART_SCRIPT_CLUSTER);

		if(!metalsound)
			Util::PlayRandomAmbientSound(position, "impact/ricochet%d.wav", 3);
		else
			Util::PlayRandomAmbientSound(position, "impact/metal_impact_bullet%d.wav", 4);
	}

	//=============================================
	//
	//=============================================
	Vector ClampVectorToBox( const Vector& vectoadj, const Vector& boxsize )
	{
		Vector vecadj = vectoadj;
		for(Uint32 i = 0; i < 3; i++)
		{
			if(vecadj[i] > boxsize[i])
				vecadj[i] -= boxsize[i];
			else if(vecadj[i] < -boxsize[i])
				vecadj[i] += boxsize[i];
			else
				vecadj[i] = 0;
		}

		return vecadj.Normalize();
	}

	//=============================================
	//
	//=============================================
	CBaseEntity* FindEntityAtDirection( const Vector& origin, const Vector& angles, const edict_t* pedict )
	{
		trace_t tr;
		Vector forward;

		Math::AngleVectors(angles, &forward, nullptr, nullptr);
		Vector vecEnd = origin + forward * 4096;

		Util::TraceLine(origin, vecEnd, false, true, pedict, tr);
		if(tr.noHit() || tr.allSolid() || tr.startSolid())
			return nullptr;

		edict_t* phitentity = gd_engfuncs.pfnGetEdictByIndex(tr.hitentity);
		if(!phitentity)
			return nullptr;

		CBaseEntity* pEntity = CBaseEntity::GetClass(phitentity);
		return pEntity;
	}

	//=============================================
	//
	//=============================================
	Int32 CalculateLightIllumination( const Vector& origin, const Vector& lightorigin, const Vector& lightcolor, Float lightradius )
	{
		trace_t tr;
		Util::TraceLine(origin, lightorigin, true, false, nullptr, tr);

		if(tr.noHit() || tr.allSolid() || tr.startSolid())
			return 0;

		Float rsquared = lightradius*lightradius;
		Vector lightdir = origin-lightorigin;
		Float dist = Math::DotProduct(lightdir, lightdir);

		Float attn = (dist/rsquared-1)*-1;
		attn = clamp(attn, 0, 1.0);

		return attn*((lightcolor[0]+lightcolor[1]+lightcolor[2])/3.0f);
	}

	//=============================================
	//
	//=============================================
	Int32 GetIllumination( const Vector& position )
	{
		const cache_model_t* pworld = gd_engfuncs.pfnGetModel(WORLD_MODEL_INDEX);
		if(!pworld)
			return 0;

		const brushmodel_t* pbrushmodel = pworld->getBrushmodel();
		if(!pbrushmodel)
			return 0;

		Vector startPos = position + Vector(0, 0, 8);
		Vector endPos = position - Vector(0, 0, 8192);

		Vector lightcolor;
		if(!gd_engfuncs.pfnRecursiveLightPoint(pbrushmodel, pbrushmodel->pnodes, startPos, endPos, lightcolor))
			return 0;

		// Calculate illumination
		Math::VectorScale(lightcolor, 255, lightcolor);
		Int32 illumination = (lightcolor[0]+lightcolor[1]+lightcolor[2])/3;

		for(Int32 i = 0; i < g_pGameVars->numentities; i++)
		{
			edict_t* pedict = gd_engfuncs.pfnGetEdictByIndex(i);
			if(Util::IsNullEntity(pedict))
				continue;

			CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
			if(!pEntity)
				continue;

			rendertype_t rtype = pEntity->GetRenderType();
			if(rtype != RT_NORMAL)
			{
				switch(rtype)
				{
				case RT_ENVELIGHT:
					illumination += Util::CalculateLightIllumination(position, pEntity->GetOrigin(), pEntity->GetRenderColor(), pEntity->GetRenderAmount()*9.5);
					break;
				case RT_ENVDLIGHT:
				case RT_ENVSPOTLIGHT:
					illumination += Util::CalculateLightIllumination(position, pEntity->GetOrigin(), pEntity->GetRenderColor(), pEntity->GetRenderAmount());
					break;
				}
			}
			else
			{
				switch(pEntity->GetRenderFx())
				{
				case RenderFx_Rotlight:
				case RenderFx_RotlightNS:
					{
						lightcolor = pEntity->GetRenderColor();
						if(lightcolor.IsZero())
							lightcolor = Vector(255, 0, 0);

						illumination += Util::CalculateLightIllumination(position, pEntity->GetOrigin(), lightcolor, pEntity->GetRenderAmount());
					}
					break;
				}
			}
		}
		return illumination;
	}

	//=============================================
	//
	//=============================================
	void CreateImpactEffects( const trace_t& tr, const Vector& traceBegin, bool createDecal, bool vbmDecal, bool playSounds, const Char* pstrDecalGroupName )
	{
		if(tr.noHit() || tr.allSolid() || tr.startSolid())
			return;

		edict_t* phitedict = gd_engfuncs.pfnGetEdictByIndex(tr.hitentity);
		if(Util::IsNullEntity(phitedict))
			return;

		CBaseEntity* pHitEntity = CBaseEntity::GetClass(phitedict);
		if(!pHitEntity || !pHitEntity->IsBrushModel())
			return;
		
		Vector traceStart = tr.endpos + tr.plane.normal*4;
		Vector traceEnd = tr.endpos - tr.plane.normal*4;
	
		const Char* pstrTextureName = gd_tracefuncs.pfnTraceTexture(tr.hitentity, traceStart, traceEnd);
		if(!pstrTextureName || !qstrlen(pstrTextureName))
			return;

		const en_material_t* pmaterial = gd_engfuncs.pfnGetMapTextureMaterialScript(pstrTextureName);
		if(!pmaterial)
			return;

		CString materialname;
		if((pHitEntity->GetRenderMode() & RENDERMODE_BITMASK) == RENDER_TRANSTEXTURE)
		{
			// All transparents are glass
			materialname = GLASS_MATERIAL_TYPE_NAME;
		}
		else
		{
			// Take from material script file
			materialname = pmaterial->materialname;
		}

		const CMaterialDefinitions::materialdefinition_t* pdefinition = gMaterialDefinitions.GetDefinitionForMaterial(materialname.c_str());
		if(!pdefinition)
			return;
		
		// Disable decals if flagged for no decalling
		if(pmaterial->flags & (TX_FL_NODECAL|TX_FL_NO_IMPACT_EFFECTS))
			createDecal = false;

		// Play impact sound effect
		if(playSounds && !(pmaterial->flags & TX_FL_NO_IMPACT_EFFECTS) && !pdefinition->sounds.empty())
		{
			Uint32 soundindex = Common::RandomLong(0, pdefinition->sounds.size()-1);
			Util::EmitAmbientSound(tr.endpos, pdefinition->sounds[soundindex].c_str());
		}

		// Create decal
		if(createDecal)
		{
			Int32 flags = FL_DECAL_SPECIFIC_TEXTURE;
			if(pHitEntity->GetEffectFlags() & EF_COLLISION || vbmDecal)
				flags |= FL_DECAL_VBM;

			// So bullets stuck in bulletproof glass don't remain without a decal
			// due to eliminating overlap
			if(pmaterial->flags & TX_FL_BULLETPROOF)
				flags |= FL_DECAL_ALLOWOVERLAP;

			// Choose group based on override param
			const Char* pstrGroupName = (pstrDecalGroupName && qstrlen(pstrDecalGroupName)) ? pstrDecalGroupName : pdefinition->decalgroup.c_str();

			// Get the texture name
			decalgroupentry_t* pentry = gDecalList.GetRandom(pstrGroupName);
			if(pentry)
			{
				// Create on client
				Util::CreateGenericDecal(tr.endpos, &tr.plane.normal, pentry->name.c_str(), flags);

				// Add to save-restore
				gd_engfuncs.pfnAddSavedDecal(tr.endpos, tr.plane.normal, tr.hitentity, pentry->name.c_str(), flags);
			}
			else
			{
				gd_engfuncs.pfnCon_Printf("%s - Couldn't find decal group '%s'.\n", __FUNCTION__, pdefinition->decalgroup.c_str());
			}
		}

		// Create particles
		if(!(pmaterial->flags & TX_FL_NO_IMPACT_EFFECTS))
		{
			// Add spark + sound for bulletproof stuff
			if(pmaterial->flags & TX_FL_BULLETPROOF)
				Util::Ricochet(tr.endpos, tr.plane.normal, false);

			// Spawn particle script
			if(!pdefinition->particlescript.empty())
				Util::CreateParticles(pdefinition->particlescript.c_str(), tr.endpos, tr.plane.normal, pdefinition->scripttype); 

			// Make a water splash
			if(gd_tracefuncs.pfnPointContents(tr.endpos, nullptr) == CONTENTS_WATER)
			{
				if(gd_tracefuncs.pfnPointContents(traceBegin, nullptr) != CONTENTS_WATER)
				{
					Vector vDir = (traceBegin - tr.endpos).Normalize();
					Vector vCur = tr.endpos;
		
					Float diff = 0.1;
					while(true)
					{
						vCur = vCur + vDir*diff;

						if(gd_tracefuncs.pfnPointContents(vCur, nullptr) != CONTENTS_WATER)
							break;
		
						diff += 0.1;
					}
		
					Util::CreateParticles("water_impact_cluster.txt", vCur, Vector(0, 0, 1), PART_SCRIPT_CLUSTER);
				}
			}
		}
	}

	//=============================================
	//
	//=============================================
	void MakeEntityDormant( edict_t* pedict )
	{
		pedict->state.flags |= FL_DORMANT;
		pedict->state.solid = SOLID_NOT;
		pedict->state.effects |= EF_NODRAW;
		pedict->state.nextthink = 0;
		
		gd_engfuncs.pfnSetOrigin(pedict, pedict->state.origin);
	}

	//=============================================
	//
	//=============================================
	void CreateTempModel( const Vector& origin, const Vector& angles, const Vector& velocity, Float life, Uint32 num, const Char* pstrModelname, Int32 sound, Float bouyancy, Float waterfriction, Int32 flags, Int32 body )
	{
		Int32 modelindex = gd_engfuncs.pfnPrecacheModel(pstrModelname);
		if(modelindex == NO_PRECACHE)
		{
			gd_engfuncs.pfnCon_Printf("%s - '%s' was not precached.\n", __FUNCTION__, pstrModelname);
			return;
		}

		CreateTempModel(origin, angles, velocity, life, num, modelindex, sound, bouyancy, waterfriction, flags, body);
	}

	//=============================================
	//
	//=============================================
	void CreateTempModel( const Vector& origin, const Vector& angles, const Vector& velocity, Float life, Uint32 num, Int32 modelindex, Int32 sound, Float bouyancy, Float waterfriction, Int32 flags, Int32 body )
	{
		if(modelindex == NO_PRECACHE)
		{
			gd_engfuncs.pfnCon_Printf("%s - Called with invalid model index.\n", __FUNCTION__);
			return;
		}

		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.createtempentity, nullptr, nullptr);
			gd_engfuncs.pfnMsgWriteByte(TE_TEMPMODEL);

			// Write origin
			for(Uint32 i = 0; i < 3; i++)
				gd_engfuncs.pfnMsgWriteFloat(origin[i]);

			// Write angles
			for(Uint32 i = 0; i < 3; i++)
				gd_engfuncs.pfnMsgWriteSmallFloat(angles[i]);

			for(Uint32 i = 0; i < 3; i++)
				gd_engfuncs.pfnMsgWriteSmallFloat(velocity[i]);

			gd_engfuncs.pfnMsgWriteSmallFloat(life);
			gd_engfuncs.pfnMsgWriteUint16(num);
			gd_engfuncs.pfnMsgWriteUint16(modelindex);
			gd_engfuncs.pfnMsgWriteByte(sound);
			gd_engfuncs.pfnMsgWriteSmallFloat(bouyancy);
			gd_engfuncs.pfnMsgWriteSmallFloat(waterfriction);
			gd_engfuncs.pfnMsgWriteInt32(flags);
			gd_engfuncs.pfnMsgWriteInt16(body);
		gd_engfuncs.pfnUserMessageEnd();
	}

	//=============================================
	//
	//=============================================
	void CreateBreakModel( const Vector& origin, const Vector& size, const Vector& velocity, Uint32 randomvel, Float life, Uint32 num, const Char* pstrModelname, Int32 sound, Float bouyancy, Float waterfriction, Int32 flags )
	{
		Int32 modelindex = gd_engfuncs.pfnPrecacheModel(pstrModelname);
		if(modelindex == NO_PRECACHE)
		{
			gd_engfuncs.pfnCon_Printf("%s - '%s' was not precached.\n", __FUNCTION__, pstrModelname);
			return;
		}

		CreateBreakModel(origin, size, velocity, randomvel, life, num, modelindex, sound, bouyancy, waterfriction, flags);
	}

	//=============================================
	//
	//=============================================
	void CreateBreakModel( const Vector& origin, const Vector& size, const Vector& velocity, Uint32 randomvel, Float life, Uint32 num, Int32 modelindex, Int32 sound, Float bouyancy, Float waterfriction, Int32 flags )
	{
		if(modelindex == NO_PRECACHE)
		{
			gd_engfuncs.pfnCon_Printf("%s - Called with invalid model index.\n", __FUNCTION__);
			return;
		}

		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.createtempentity, nullptr, nullptr);
			gd_engfuncs.pfnMsgWriteByte(TE_BREAKMODEL);
			// Write origin
			for(Uint32 i = 0; i < 3; i++)
				gd_engfuncs.pfnMsgWriteFloat(origin[i]);
			// Write angles
			for(Uint32 i = 0; i < 3; i++)
				gd_engfuncs.pfnMsgWriteSmallFloat(size[i]);
			// Write velocity
			for(Uint32 i = 0; i < 3; i++)
				gd_engfuncs.pfnMsgWriteSmallFloat(velocity[i]);
			gd_engfuncs.pfnMsgWriteUint16(randomvel);
			gd_engfuncs.pfnMsgWriteSmallFloat(life);
			gd_engfuncs.pfnMsgWriteUint16(num);
			gd_engfuncs.pfnMsgWriteUint16(modelindex);
			gd_engfuncs.pfnMsgWriteByte(sound);
			gd_engfuncs.pfnMsgWriteSmallFloat(bouyancy);
			gd_engfuncs.pfnMsgWriteSmallFloat(waterfriction);
			gd_engfuncs.pfnMsgWriteInt32(flags);
		gd_engfuncs.pfnUserMessageEnd();
	}

	//=============================================
	//
	//=============================================
	void CreateBubbles( const Vector& mins, const Vector& maxs, const Float height, const Char* pstrSpritename, Uint32 num, Float speed )
	{
		Int32 modelindex = gd_engfuncs.pfnPrecacheModel(pstrSpritename);
		if(modelindex == NO_PRECACHE)
		{
			gd_engfuncs.pfnCon_Printf("%s - '%s' was not precached.\n", __FUNCTION__, pstrSpritename);
			return;
		}

		CreateBubbles(mins, maxs, height, modelindex, num, speed);
	}

	//=============================================
	//
	//=============================================
	void CreateBubbles( const Vector& mins, const Vector& maxs, const Float height, Int32 modelindex, Uint32 num, Float speed )
	{
		if(modelindex == NO_PRECACHE)
		{
			gd_engfuncs.pfnCon_Printf("%s - Called with invalid model index.\n", __FUNCTION__);
			return;
		}

		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.createtempentity, nullptr, nullptr);
			gd_engfuncs.pfnMsgWriteByte(TE_BUBBLES);
			// mins
			for(Uint32 i = 0; i < 3; i++)
				gd_engfuncs.pfnMsgWriteFloat(mins[i]);
			// maxs
			for(Uint32 i = 0; i < 3; i++)
				gd_engfuncs.pfnMsgWriteFloat(maxs[i]);
			gd_engfuncs.pfnMsgWriteSmallFloat(height);
			gd_engfuncs.pfnMsgWriteUint16(modelindex);
			gd_engfuncs.pfnMsgWriteUint16(num);
			gd_engfuncs.pfnMsgWriteSmallFloat(speed);
		gd_engfuncs.pfnUserMessageEnd();
	}

	//=============================================
	//
	//=============================================
	void CreateBubbleTrail( const Vector& start, const Vector& end, const Float height, const Char* pstrSpritename, Uint32 num, Float speed )
	{
		Int32 modelindex = gd_engfuncs.pfnPrecacheModel(pstrSpritename);
		if(modelindex == NO_PRECACHE)
		{
			gd_engfuncs.pfnCon_Printf("%s - '%s' was not precached.\n", __FUNCTION__, pstrSpritename);
			return;
		}

		CreateBubbleTrail(start, end, height, modelindex, num, speed);
	}

	//=============================================
	//
	//=============================================
	void CreateBubbleTrail( const Vector& start, const Vector& end, const Float height, Int32 modelindex, Uint32 num, Float speed )
	{
		if(modelindex == NO_PRECACHE)
		{
			gd_engfuncs.pfnCon_Printf("%s - Called with invalid model index.\n", __FUNCTION__);
			return;
		}

		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.createtempentity, nullptr, nullptr);
			gd_engfuncs.pfnMsgWriteByte(TE_BUBBLETRAIL);
			// mins
			for(Uint32 i = 0; i < 3; i++)
				gd_engfuncs.pfnMsgWriteFloat(start[i]);
			// maxs
			for(Uint32 i = 0; i < 3; i++)
				gd_engfuncs.pfnMsgWriteFloat(end[i]);
			gd_engfuncs.pfnMsgWriteSmallFloat(height);
			gd_engfuncs.pfnMsgWriteUint16(modelindex);
			gd_engfuncs.pfnMsgWriteUint16(num);
			gd_engfuncs.pfnMsgWriteSmallFloat(speed);
		gd_engfuncs.pfnUserMessageEnd();
	}

	//=============================================
	//
	//=============================================
	void CreateFunnelSprite( const Vector& origin, const Vector& color, Float alpha, const Char* pstrSpritename, bool reverse )
	{
		Int32 modelindex = gd_engfuncs.pfnPrecacheModel(pstrSpritename);
		if(modelindex == NO_PRECACHE)
		{
			gd_engfuncs.pfnCon_Printf("%s - '%s' was not precached.\n", __FUNCTION__, pstrSpritename);
			return;
		}

		CreateFunnelSprite(origin, color, alpha, modelindex, reverse);
	}

	//=============================================
	//
	//=============================================
	void CreateFunnelSprite( const Vector& origin, const Vector& color, Float alpha, Int32 modelindex, bool reverse )
	{
		if(modelindex == NO_PRECACHE)
		{
			gd_engfuncs.pfnCon_Printf("%s - Called with invalid model index.\n", __FUNCTION__);
			return;
		}

		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.createtempentity, nullptr, nullptr);
			gd_engfuncs.pfnMsgWriteByte(TE_FUNNELSPRITE);
			for(Uint32 i = 0; i < 3; i++)
				gd_engfuncs.pfnMsgWriteFloat(origin[i]);
			for(Uint32 i = 0; i < 3; i++)
				gd_engfuncs.pfnMsgWriteSmallFloat(color[i]);

			gd_engfuncs.pfnMsgWriteSmallFloat(alpha);
			gd_engfuncs.pfnMsgWriteUint16(modelindex);
			gd_engfuncs.pfnMsgWriteByte(reverse);
		gd_engfuncs.pfnUserMessageEnd();
	}

	//=============================================
	//
	//=============================================
	void CreateSphereModel( const Vector& origin, Float speed, Float life, Uint32 num, const Char* pstrModelname, Int32 sound, Float bouyancy, Float waterfriction )
	{
		Int32 modelindex = gd_engfuncs.pfnPrecacheModel(pstrModelname);
		if(modelindex == NO_PRECACHE)
		{
			gd_engfuncs.pfnCon_Printf("%s - '%s' was not precached.\n", __FUNCTION__, pstrModelname);
			return;
		}

		CreateSphereModel(origin, speed, life, num, modelindex, sound, bouyancy, waterfriction);
	}

	//=============================================
	//
	//=============================================
	void CreateSphereModel( const Vector& origin, Float speed, Float life, Uint32 num, Int32 modelindex, Int32 sound, Float bouyancy, Float waterfriction )
	{
		if(modelindex == NO_PRECACHE)
		{
			gd_engfuncs.pfnCon_Printf("%s - Called with invalid model index.\n", __FUNCTION__);
			return;
		}

		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.createtempentity, nullptr, nullptr);
			gd_engfuncs.pfnMsgWriteByte(TE_SPHEREMODEL);
			for(Uint32 i = 0; i < 3; i++)
				gd_engfuncs.pfnMsgWriteFloat(origin[i]);

			gd_engfuncs.pfnMsgWriteSmallFloat(speed);
			gd_engfuncs.pfnMsgWriteUint16(num);
			gd_engfuncs.pfnMsgWriteUint16(modelindex);
			gd_engfuncs.pfnMsgWriteByte(sound);
			gd_engfuncs.pfnMsgWriteSmallFloat(bouyancy);
			gd_engfuncs.pfnMsgWriteSmallFloat(waterfriction);
		gd_engfuncs.pfnUserMessageEnd();
	}

	//=============================================
	//
	//=============================================
	void CreateTempSprite( const Vector& origin, const Vector& velocity, Float life, Float scale, const Char* pstrSpritename, rendermode_t rendermode, Int32 renderfx, Float alpha, Int32 sound, Int32 flags )
	{
		Int32 modelindex = gd_engfuncs.pfnPrecacheModel(pstrSpritename);
		if(modelindex == NO_PRECACHE)
		{
			gd_engfuncs.pfnCon_Printf("%s - '%s' was not precached.\n", __FUNCTION__, pstrSpritename);
			return;
		}

		CreateTempSprite(origin, velocity, life, scale, modelindex, rendermode, renderfx, alpha, sound, flags);
	}

	//=============================================
	//
	//=============================================
	void CreateTempSprite( const Vector& origin, const Vector& velocity, Float life, Float scale, Int32 modelindex, rendermode_t rendermode, Int32 renderfx, Float alpha, Int32 sound, Int32 flags )
	{
		if(modelindex == NO_PRECACHE)
		{
			gd_engfuncs.pfnCon_Printf("%s - Called with invalid model index.\n", __FUNCTION__);
			return;
		}

		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.createtempentity, nullptr, nullptr);
			gd_engfuncs.pfnMsgWriteByte(TE_TEMPSPRITE);
			for(Uint32 i = 0; i < 3; i++)
				gd_engfuncs.pfnMsgWriteFloat(origin[i]);
			for(Uint32 i = 0; i < 3; i++)
				gd_engfuncs.pfnMsgWriteSmallFloat(velocity[i]);

			gd_engfuncs.pfnMsgWriteSmallFloat(life);
			gd_engfuncs.pfnMsgWriteSmallFloat(scale);
			gd_engfuncs.pfnMsgWriteUint16(modelindex);
			gd_engfuncs.pfnMsgWriteUint16((Uint16)rendermode);
			gd_engfuncs.pfnMsgWriteUint16(renderfx);
			gd_engfuncs.pfnMsgWriteSmallFloat(alpha);
			gd_engfuncs.pfnMsgWriteByte(sound);
			gd_engfuncs.pfnMsgWriteByte(flags);
		gd_engfuncs.pfnUserMessageEnd();
	}

	//=============================================
	//
	//=============================================
	void CreateParticleExplosion1( const Vector& origin )
	{
		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.createtempentity, nullptr, nullptr);
			gd_engfuncs.pfnMsgWriteByte(TE_PARTICLEEXPLOSION1);
			for(Uint32 i = 0; i < 3; i++)
				gd_engfuncs.pfnMsgWriteFloat(origin[i]);
		gd_engfuncs.pfnUserMessageEnd();
	}

	//=============================================
	//
	//=============================================
	void CreateParticleExplosion2( const Vector& origin, Int32 colorstart, Int32 colorlength )
	{
		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.createtempentity, nullptr, nullptr);
			gd_engfuncs.pfnMsgWriteByte(TE_PARTICLEEXPLOSION2);
			for(Uint32 i = 0; i < 3; i++)
				gd_engfuncs.pfnMsgWriteFloat(origin[i]);
			gd_engfuncs.pfnMsgWriteByte(colorstart);
			gd_engfuncs.pfnMsgWriteByte(colorlength);
		gd_engfuncs.pfnUserMessageEnd();
	}

	//=============================================
	//
	//=============================================
	void CreateBlobExplosion( const Vector& origin )
	{
		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.createtempentity, nullptr, nullptr);
			gd_engfuncs.pfnMsgWriteByte(TE_BLOBEXPLOSION);
			for(Uint32 i = 0; i < 3; i++)
				gd_engfuncs.pfnMsgWriteFloat(origin[i]);
		gd_engfuncs.pfnUserMessageEnd();
	}

	//=============================================
	//
	//=============================================
	void CreateRocketExplosion( const Vector& origin, Int32 color )
	{
		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.createtempentity, nullptr, nullptr);
			gd_engfuncs.pfnMsgWriteByte(TE_ROCKETEXPLOSION);
			for(Uint32 i = 0; i < 3; i++)
				gd_engfuncs.pfnMsgWriteFloat(origin[i]);
			gd_engfuncs.pfnMsgWriteByte(color);
		gd_engfuncs.pfnUserMessageEnd();
	}

	//=============================================
	//
	//=============================================
	void CreateParticleEffect( const Vector& origin, const Vector& velocity, Int32 color, Uint32 count )
	{
		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.createtempentity, nullptr, nullptr);
			gd_engfuncs.pfnMsgWriteByte(TE_PARTICLEEFFECT);
			for(Uint32 i = 0; i < 3; i++)
				gd_engfuncs.pfnMsgWriteFloat(origin[i]);
			for(Uint32 i = 0; i < 3; i++)
				gd_engfuncs.pfnMsgWriteSmallFloat(velocity[i]);
			gd_engfuncs.pfnMsgWriteByte(color);
			gd_engfuncs.pfnMsgWriteUint16(count);
		gd_engfuncs.pfnUserMessageEnd();
	}

	//=============================================
	//
	//=============================================
	void CreateLavaSplash( const Vector& origin )
	{
		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.createtempentity, nullptr, nullptr);
			gd_engfuncs.pfnMsgWriteByte(TE_LAVASPLASH);
			for(Uint32 i = 0; i < 3; i++)
				gd_engfuncs.pfnMsgWriteFloat(origin[i]);
		gd_engfuncs.pfnUserMessageEnd();
	}

	//=============================================
	//
	//=============================================
	void CreateTeleportSplash( const Vector& origin )
	{
		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.createtempentity, nullptr, nullptr);
			gd_engfuncs.pfnMsgWriteByte(TE_TELEPORTSPLASH);
			for(Uint32 i = 0; i < 3; i++)
				gd_engfuncs.pfnMsgWriteFloat(origin[i]);
		gd_engfuncs.pfnUserMessageEnd();
	}

	//=============================================
	//
	//=============================================
	void CreateRocketTrail( const Vector& start, const Vector& end, Uint32 type )
	{
		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.createtempentity, nullptr, nullptr);
			gd_engfuncs.pfnMsgWriteByte(TE_ROCKETTRAIL);
			for(Uint32 i = 0; i < 3; i++)
				gd_engfuncs.pfnMsgWriteFloat(start[i]);
			for(Uint32 i = 0; i < 3; i++)
				gd_engfuncs.pfnMsgWriteFloat(end[i]);
			gd_engfuncs.pfnMsgWriteByte(type);
		gd_engfuncs.pfnUserMessageEnd();
	}

	//=============================================
	//
	//=============================================
	void PrecacheFixedNbSounds( const Char* pstrPattern, Uint32 count )
	{
		CString filepath(pstrPattern);
		Int32 tokenpos = filepath.find(0, "%d");
		if(tokenpos == -1)
		{
			gd_engfuncs.pfnCon_Printf("%s - Number token not found in string '%s'.\n", __FUNCTION__, pstrPattern);
			return;
		}

		for(Uint32 i = 0; i < count; i++)
		{
			CString replacestring(filepath);
			replacestring.erase(tokenpos, 2);

			CString substr;
			substr << (Int32)(i+1);

			replacestring.insert(tokenpos, substr.c_str());
			gd_engfuncs.pfnPrecacheSound(replacestring.c_str());
		}
	}

	//=============================================
	//
	//=============================================
	void PrecacheVariableNbSounds( const Char* pstrPattern, Uint32& outcount )
	{
		CString filepath(pstrPattern);
		Int32 tokenpos = filepath.find(0, "%d");
		if(tokenpos == -1)
		{
			gd_engfuncs.pfnCon_Printf("%s - Number token not found in string '%s'.\n", __FUNCTION__, pstrPattern);
			return;
		}

		Uint32 number = 1;
		while(true)
		{
			CString replacestring(filepath);
			replacestring.erase(tokenpos, 2);

			CString substr;
			substr << (Int32)number;

			replacestring.insert(tokenpos, substr.c_str());
			CString checkpath;
			checkpath << SOUND_FOLDER_BASE_PATH << replacestring;

			if(!gd_filefuncs.pfnFileExists(checkpath.c_str()))
				break;

			Int32 precacheindex = gd_engfuncs.pfnPrecacheSound(replacestring.c_str());
			if(precacheindex == NO_PRECACHE)
				break;

			number++;
		}

		// Set final count
		outcount = (number-1);
		if(!outcount)
			gd_engfuncs.pfnCon_Printf("%s - No files found for pattern '%s'.\n", __FUNCTION__, pstrPattern);
	}

	//=============================================
	//
	//=============================================
	void PrecacheSounds( const Char* pstrPattern, Int32 count )
	{
		CString filepath(pstrPattern);
		Int32 tokenpos = filepath.find(0, "%d");
		if(tokenpos == -1)
		{
			gd_engfuncs.pfnCon_Printf("%s - Number token not found in string '%s'.\n", __FUNCTION__, pstrPattern);
			return;
		}

		for(Int32 i = 0; i < count; i++)
		{
			CString replacestring(filepath);
			replacestring.erase(tokenpos, 2);

			CString substr;
			substr << (i+1);

			replacestring.insert(tokenpos, substr.c_str());
			gd_engfuncs.pfnPrecacheSound(replacestring.c_str());
		}
	}

	//=============================================
	//
	//=============================================
	CString PlayRandomAmbientSound( const Vector& origin, const Char* pstrPattern, Int32 count, Float volume, Float attenuation, Int32 pitch, Int32 flags )
	{
		if(count <= 0)
		{
			gd_engfuncs.pfnCon_Printf("%s - Called with zero count.\n", __FUNCTION__);
			return CString();
		}

		CString filepath(pstrPattern);
		Int32 tokenpos = filepath.find(0, "%d");
		if(tokenpos == -1)
		{
			gd_engfuncs.pfnCon_Printf("%s - Number token not found in string '%s'.\n", __FUNCTION__, pstrPattern);
			return CString();
		}

		CString replacestring(filepath);
		replacestring.erase(tokenpos, 2);

		CString substr;
		substr << (Int32)Common::RandomLong(1, count);

		replacestring.insert(tokenpos, substr.c_str());

		Util::EmitAmbientSound(origin, replacestring.c_str(), volume, attenuation, pitch, flags); 
		return replacestring;
	}

	//=============================================
	//
	//=============================================
	CString PlayRandomEntitySound( CBaseEntity* pEntity, const Char* pstrPattern, Int32 count, snd_channels_t channel, Float volume, Float attenuation, Int32 pitch, Int32 flags )
	{
		if(count <= 0)
		{
			gd_engfuncs.pfnCon_Printf("%s - Called with zero count.\n", __FUNCTION__);
			return CString();
		}

		CString filepath(pstrPattern);
		Int32 tokenpos = filepath.find(0, "%d");
		if(tokenpos == -1)
		{
			gd_engfuncs.pfnCon_Printf("%s - Number token not found in string '%s'.\n", __FUNCTION__, pstrPattern);
			return CString();
		}

		CString replacestring(filepath);
		replacestring.erase(tokenpos, 2);

		CString substr;
		substr << (Int32)Common::RandomLong(1, count);

		replacestring.insert(tokenpos, substr.c_str());

		Util::EmitEntitySound(pEntity, replacestring.c_str(), channel, volume, attenuation, pitch, flags); 
		return replacestring;
	}

	//=============================================
	// @brief
	//
	//=============================================
	void PlayWeaponClatterSound( const edict_t* pedict )
	{
		// Add clatter
		CString soundname;
		switch(Common::RandomLong(0, 2))
		{
		case 0: soundname = "items/weapon_clatter1.wav"; break;
		case 1: soundname = "items/weapon_clatter2.wav"; break;
		case 2: soundname = "items/weapon_clatter3.wav"; break;
		}

		gd_engfuncs.pfnPlayEntitySound(pedict->entindex, soundname.c_str(), SND_FL_NONE, SND_CHAN_BODY, VOL_NORM, ATTN_NORM, Common::RandomLong(90, 110), 0, NO_CLIENT_INDEX);
	}

	//=============================================
	// @brief
	//
	//=============================================
	void AlignEntityToSurface( edict_t* pedict )
	{
		// Reset pitch pitch and roll
		pedict->state.angles[0] = 0;
		pedict->state.angles[2] = 0;

		trace_t tr;
		Util::TraceLine(pedict->state.origin+Vector(0, 0, 8), pedict->state.origin-Vector(0, 0, 8), false, false, pedict, tr);
		if(tr.noHit() || tr.allSolid() || tr.startSolid())
			return;

		// Do not directly use m_pState->angles
		Vector newangles = Math::AdjustAnglesToNormal(tr.plane.normal, pedict->state.angles);
		pedict->state.angles = newangles;
	}

	//=============================================
	// @brief
	//
	//=============================================
	Vector GetRandomBloodVector( void )
	{
		Vector direction(
			Common::RandomFloat(-1, 1),
			Common::RandomFloat(-1, 1),
			Common::RandomFloat(0, 1));
		direction.Normalize();

		return direction;
	}

	//=============================================
	// @brief
	//
	//=============================================
	void ScreenShake( const Vector& origin, Float amplitude, Float frequency, Float duration, Float radius, bool inair, bool disruptcontrols )
	{
		for(Int32 i = 0; i < g_pGameVars->maxclients; i++)
		{
			CBaseEntity* pPlayer = Util::GetPlayerByIndex(i);
			if(!pPlayer || (!inair && !(pPlayer->GetFlags() & FL_ONGROUND)))
				continue;

			Float playeramplitude = 0;
			if(radius > 0)
			{
				Float dist = (origin - pPlayer->GetOrigin()).Length();
				if(dist < radius)
					playeramplitude = amplitude;
			}
			else
				playeramplitude = amplitude;

			if(playeramplitude > 0)
			{
				gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.screenshake, nullptr, pPlayer->GetEdict());
					gd_engfuncs.pfnMsgWriteSmallFloat(amplitude*10);
					gd_engfuncs.pfnMsgWriteSmallFloat(duration*10);
					gd_engfuncs.pfnMsgWriteSmallFloat(frequency*10);
				gd_engfuncs.pfnUserMessageEnd();
			}
		}
	}

	//=============================================
	// @brief
	//
	//=============================================
	void ScreenShakeAll( const Vector& origin, Float amplitude, Float frequency, Float duration )
	{
		Util::ScreenShake(origin, amplitude, frequency, duration, 0, true, false);
	}

	//=============================================
	// @brief
	//
	//=============================================
	void CreateSparks( const Vector& origin )
	{
		Util::CreateParticles("spark_cluster.txt", origin, ZERO_VECTOR, PART_SCRIPT_CLUSTER);
		Util::CreateDynamicLight(origin, 3*Common::RandomFloat(1, 3), 255, 180, 100, Common::RandomFloat(0.1, 0.2), -3*Common::RandomLong(13, 18), 0, FL_DLIGHT_NOSHADOWS);
	}

	//=============================================
	// @brief
	//
	//=============================================
	void CreateDynamicLight( const Vector& origin, Float radius, Int32 r, Int32 g, Int32 b, Float life, Int32 decay, Float decaydelay, byte flags, Int32 entindex, Int32 attachment, Int32 lightstyle )
	{
		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.dynamiclight, nullptr, nullptr);
		for(Uint32 i = 0; i < 3; i++)
			gd_engfuncs.pfnMsgWriteFloat(origin[i]);
			gd_engfuncs.pfnMsgWriteSmallFloat(radius);
			gd_engfuncs.pfnMsgWriteByte(clamp(r, 0, 255));
			gd_engfuncs.pfnMsgWriteByte(clamp(g, 0, 255));
			gd_engfuncs.pfnMsgWriteByte(clamp(b, 0, 255));
			gd_engfuncs.pfnMsgWriteSmallFloat(life);
			gd_engfuncs.pfnMsgWriteSmallFloat(decay);
			gd_engfuncs.pfnMsgWriteSmallFloat(decaydelay);
			gd_engfuncs.pfnMsgWriteByte(flags);
			if(flags & FL_DLIGHT_FOLLOW_ENTITY)
			{
				gd_engfuncs.pfnMsgWriteInt16(entindex);

				if(flags & FL_DLIGHT_USE_ATTACHMENT)
					gd_engfuncs.pfnMsgWriteInt16(attachment);
			}
			if(flags & FL_DLIGHT_USE_LIGHTSTYLES)
				gd_engfuncs.pfnMsgWriteByte(lightstyle);

		gd_engfuncs.pfnUserMessageEnd();
	}

	//=============================================
	// @brief
	//
	//=============================================
	void CreateDynamicLightWithSubkey( const Vector& origin, Float radius, Int32 r, Int32 g, Int32 b, Float life, Int32 decay, Float decaydelay, byte flags, Int32 entindex, Int32 subkey, Int32 attachment, Int32 lightstyle )
	{
		byte _flags = flags;
		if(subkey)
			_flags |= FL_DLIGHT_USE_SUBKEY;

		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.dynamiclight, nullptr, nullptr);
		for(Uint32 i = 0; i < 3; i++)
			gd_engfuncs.pfnMsgWriteFloat(origin[i]);
			gd_engfuncs.pfnMsgWriteSmallFloat(radius);
			gd_engfuncs.pfnMsgWriteByte(clamp(r, 0, 255));
			gd_engfuncs.pfnMsgWriteByte(clamp(g, 0, 255));
			gd_engfuncs.pfnMsgWriteByte(clamp(b, 0, 255));
			gd_engfuncs.pfnMsgWriteSmallFloat(life);
			gd_engfuncs.pfnMsgWriteSmallFloat(decay);
			gd_engfuncs.pfnMsgWriteSmallFloat(decaydelay);
			gd_engfuncs.pfnMsgWriteByte(_flags);
			if(_flags & FL_DLIGHT_FOLLOW_ENTITY)
			{
				gd_engfuncs.pfnMsgWriteInt16(entindex);
				gd_engfuncs.pfnMsgWriteInt16(subkey);

				if(_flags & FL_DLIGHT_USE_ATTACHMENT)
					gd_engfuncs.pfnMsgWriteInt16(attachment);
			}
			if(_flags & FL_DLIGHT_USE_LIGHTSTYLES)
				gd_engfuncs.pfnMsgWriteByte(lightstyle);

		gd_engfuncs.pfnUserMessageEnd();
	}

	//=============================================
	// @brief
	//
	//=============================================
	void ExplosionSound( const Vector& origin )
	{
		CString soundname;
		soundname << "weapons/explosion" << (Int32)Common::RandomLong(1, 3) << ".wav";

		Util::EmitAmbientSound(origin, soundname.c_str(), VOL_NORM, 0.3, PITCH_NORM, SND_FL_NONE);
	}

	//=============================================
	// @brief
	//
	//=============================================
	extern void ReadColor24FromString( const Char* pstrString, color24_t& outcolor )
	{
		CString token;
		Uint32 readindex = 0;

		const Char* pstr = pstrString;
		while(pstr && readindex < 2)
		{
			pstr = Common::Parse(pstr, token);
			if(!Common::IsNumber(token.c_str()))
			{
				gd_engfuncs.pfnCon_Printf("%s - String '%s' has invalid values.\n", __FUNCTION__, pstrString);
				break;
			}

			outcolor[readindex] = SDL_atoi(token.c_str());
			readindex++;
		}

		if(readindex < 2)
		{
			for(Uint32 i = 0; i < 2; i++)
				outcolor[i] = 0;
		}
	}

	//=============================================
	// @brief
	//
	//=============================================
	extern void ReadColor32FromString( const Char* pstrString, color32_t& outcolor )
	{
		CString token;
		Uint32 readindex = 0;

		const Char* pstr = pstrString;
		while(pstr && readindex < 3)
		{
			pstr = Common::Parse(pstr, token);
			if(!Common::IsNumber(token.c_str()))
			{
				gd_engfuncs.pfnCon_Printf("%s - String '%s' has invalid values.\n", __FUNCTION__, pstrString);
				break;
			}

			outcolor[readindex] = SDL_atoi(token.c_str());
			readindex++;
		}

		if(readindex < 3)
		{
			for(Uint32 i = 0; i < 2; i++)
				outcolor[i] = 0;
		}
	}

	//=============================================
	// @brief
	//
	//=============================================
	Float AngleMod( Float angle )
	{
		if(angle < 0)
			return angle + 360*((Int32)(angle/360)+1);
		else
			return angle - 360*((Int32)(angle/360));
	}

	//=============================================
	// @brief
	//
	//=============================================
	Float ApproachAngle( Float targetvalue, Float curvalue, Float speed )
	{
		Float target = Util::AngleMod(targetvalue);
		Float value = target - Util::AngleMod(curvalue);
		Float delta = target - value;

		if(speed < 0)
			speed = -speed;

		if(delta < -180)
			delta += 360;
		else if(delta > 180)
			delta -= 360;

		if(delta > speed)
			value += speed;
		else if(delta < -speed)
			value -= speed;
		else
			value = target;

		return value;
	}

	//=============================================
	// @brief
	//
	//=============================================
	Float AngleDistance( Float next, Float cur )
	{
		Float delta = next-cur;
		if(delta < -180)
			delta += 360;
		else if(delta > 180)
			delta -= 360;

		return delta;
	}

	//=============================================
	// @brief
	//
	//=============================================
	Float Approach( Float target, Float value, Float speed )
	{
		Float outvalue = value;
		Float delta = target - value;
		if(delta > speed)
			outvalue += speed;
		else if(delta < -speed)
			outvalue -= speed;
		else
			outvalue = target;

		return outvalue;
	}

	//=============================================
	// @brief
	//
	//=============================================
	void FixGroundEntities( const CBaseEntity* pGroundEntity, bool noDropToFloor )
	{
		// Find all entities that have this as the ground entity
		for(Int32 i = 1; i < g_pGameVars->numentities; i++)
		{
			edict_t* pEdict = gd_engfuncs.pfnGetEdictByIndex(i);
			if(!pEdict || Util::IsNullEntity(pEdict))
				continue;

			CBaseEntity* pEntity = CBaseEntity::GetClass(pEdict);
			if(!pEntity || !pEntity->IsNPC())
				continue;

			if(pEntity->GetGroundEntity() != pGroundEntity)
				continue;

			if(!noDropToFloor)
			{
				// Nudge this entity
				pEntity->GroundEntityNudge();
			}
			else
			{
				// Let this entity just fall
				pEntity->RemoveFlags(FL_ONGROUND);
				pEntity->SetGroundEntity(nullptr);
			}
		}
	}

	//=============================================
	// @brief
	//
	//=============================================
	const Char* GetDebrisSound( breakmaterials_t material )
	{
		switch(material)
		{
		case MAT_UNBREAKABLE_GLASS:
		case MAT_GLASS:
			return GLASS_DEBRIS_SOUNDS[Common::RandomLong(0, NB_GLASS_DEBRIS_SOUNDS-1)];
			break;
		case MAT_WOOD:
			return WOOD_DEBRIS_SOUNDS[Common::RandomLong(0, NB_WOOD_DEBRIS_SOUNDS-1)];
			break;
		case MAT_COMPUTER:
		case MAT_METAL:
			return METAL_DEBRIS_SOUNDS[Common::RandomLong(0, NB_METAL_DEBRIS_SOUNDS-1)];
			break;
		case MAT_FLESH:
			return FLESH_DEBRIS_SOUNDS[Common::RandomLong(0, NB_FLESH_DEBRIS_SOUNDS-1)];
			break;
		case MAT_ROCKS:
		case MAT_CINDERBLOCK:
			return CONCRETE_DEBRIS_SOUNDS[Common::RandomLong(0, NB_CONCRETE_DEBRIS_SOUNDS-1)];
			break;
		case MAT_CEILINGTILE:
		case MAT_NONE:
		default:
			return nullptr;
			break;
		}
	}

	//=============================================
	// @brief
	//
	//=============================================
	void PrecacheDebrisSounds( breakmaterials_t material )
	{
		Uint32 numsounds = 0;
		const Char** pstrSounds = nullptr;

		switch(material)
		{
		case MAT_UNBREAKABLE_GLASS:
		case MAT_GLASS:
			numsounds = NB_GLASS_DEBRIS_SOUNDS;
			pstrSounds = GLASS_DEBRIS_SOUNDS;
			break;
		case MAT_WOOD:
			numsounds = NB_WOOD_DEBRIS_SOUNDS;
			pstrSounds = WOOD_DEBRIS_SOUNDS;
			break;
		case MAT_COMPUTER:
		case MAT_METAL:
			numsounds = NB_METAL_DEBRIS_SOUNDS;
			pstrSounds = METAL_DEBRIS_SOUNDS;
			break;
		case MAT_FLESH:
			numsounds = NB_FLESH_DEBRIS_SOUNDS;
			pstrSounds = FLESH_DEBRIS_SOUNDS;
			break;
		case MAT_ROCKS:
		case MAT_CINDERBLOCK:
			numsounds = NB_CONCRETE_DEBRIS_SOUNDS;
			pstrSounds = CONCRETE_DEBRIS_SOUNDS;
			break;
		case MAT_CEILINGTILE:
		case MAT_NONE:
		default:
			break;
		}

		if(!numsounds || !pstrSounds)
			return;

		for(Uint32 i = 0; i < numsounds; i++)
		{
			const Char* pstrSound = pstrSounds[i];
			gd_engfuncs.pfnPrecacheSound(pstrSound);
		}
	}

	//=============================================
	// @brief
	//
	//=============================================
	bool IsDataEmpty( const byte* pdata, Uint16 size )
	{
		for(Uint16 i = 0; i < size; i++)
		{
			if(pdata[i] != 0)
				return false;
		}
		return true;
	}

	//=============================================
	// @brief
	//
	//=============================================
	void PrecacheFlexScript( flextypes_t npctype, const Char* pstrscriptname )
	{
		if(!g_pFlexManager)
			return;

		if(!g_pFlexManager->LoadAssociationScript(npctype, pstrscriptname))
		{
			gd_engfuncs.pfnCon_Printf("%s - Failed to load association script '%s'.\n", __FUNCTION__, pstrscriptname);
			return;
		}

		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.precacheflexscript, nullptr, nullptr);
			gd_engfuncs.pfnMsgWriteByte((byte)npctype);
			gd_engfuncs.pfnMsgWriteString(pstrscriptname);
		gd_engfuncs.pfnUserMessageEnd();
	}

	//=============================================
	// @brief
	//
	//=============================================
	bool IsInViewCone( const Vector& origin, const Vector& angles, Float fieldOfView, const Vector& position )
	{
		// Get 2D vector to position
		Vector _2dDir = (position-origin);
		_2dDir.z = 0;
		_2dDir.Normalize();

		// Get 2d forward vector
		Vector _2dForward;
		Vector _2dAngles = angles;
		_2dAngles[PITCH] = _2dAngles[ROLL] = 0;
		Math::AngleVectors(_2dAngles, &_2dForward);

		Float dp = Math::DotProduct(_2dDir, _2dForward);
		if(dp > fieldOfView)
			return true;

		return false;
	}

	//=============================================
	// @brief
	//
	//=============================================
	CBaseEntity* GetEntityFromTrace( const trace_t& tr )
	{
		if(tr.hitentity == NO_ENTITY_INDEX)
			return nullptr;

		edict_t* pedict = gd_engfuncs.pfnGetEdictByIndex(tr.hitentity);
		if(!pedict || Util::IsNullEntity(pedict))
			return nullptr;

		return CBaseEntity::GetClass(pedict);
	}

	//=============================================
	// @brief
	//
	//=============================================
	bool CheckTraceLine( const Vector& startPosition, const Vector& endPosition, bool ignoreNPCs, bool ignoreGlass, edict_t* pIgnoreEntity )
	{
		trace_t tr;
		Util::TraceLine(startPosition, endPosition, ignoreNPCs, true, ignoreGlass, false, pIgnoreEntity, tr);

		return (tr.noHit()) ? true : false;
	}

	//=============================================
	// @brief
	//
	//=============================================
	Float VectorToYaw( const Vector& inVector )
	{
		if(!inVector[YAW] && !inVector[PITCH])
			return 0;

		Float yaw = (Float)(SDL_atan2(inVector[YAW], inVector[PITCH]) * 180.0 / M_PI);
		yaw = Math::AngleMod(yaw);

		return yaw;
	}

	//=============================================
	// @brief
	//
	//=============================================
	Float VectorToPitch( const Vector& inVector )
	{
		if(inVector[YAW] == 0 && inVector[PITCH] == 0)
		{
			if(inVector[ROLL] < 0.0f)
				return 180.0f;
			else
				return -180.0f;
		}

		Float length = inVector.Length2D();
		Float pitch = SDL_atan2(-inVector[ROLL], length);
		return RAD2DEG(pitch);
	}

	//=============================================
	// @brief
	//
	//=============================================
	node_hull_types_t GetNodeHullForNPC( const CBaseEntity* pEntity )
	{
		if(pEntity->GetMoveType() == MOVETYPE_FLY)
			return NODE_FLY_HULL;

		const Vector& mins = pEntity->GetMins();
		if(mins == Vector(-12, -12, 0))
			return NODE_SMALL_HULL;
		else if(mins == VEC_HUMAN_HULL_MIN)
			return NODE_HUMAN_HULL;
		else if(mins == Vector(-32, -32, 0))
			return NODE_LARGE_HULL;
		else
			return NODE_HUMAN_HULL;
	}

	//=============================================
	// @brief
	//
	//=============================================
	Uint64 GetNodeTypeForNPC( const CBaseEntity* pEntity )
	{
		if(pEntity->GetMoveType() == MOVETYPE_FLY)
		{
			if(pEntity->GetWaterLevel() != WATERLEVEL_NONE)
				return AI_NODE_WATER;
			else
				return AI_NODE_AIR;
		}
		else
			return AI_NODE_LAND;
	}

	//=============================================
	// @brief
	//
	//=============================================
	CBaseEntity* GetEntityByIndex( Int32 index )
	{
		if(index == NO_ENTITY_INDEX)
			return nullptr;

		if(Util::IsNullEntity(index))
			return nullptr;

		edict_t* pedict = gd_engfuncs.pfnGetEdictByIndex(index);
		return CBaseEntity::GetClass(pedict);
	}
	
	//=============================================
	// @brief
	//
	//=============================================
	void FindLinkEntities( CBaseEntity* pLinkEntity, CArray<CBaseEntity*>& entitesArray, CBaseEntity* pNPC )
	{
		// If triggered by a trigger_multiple, tell it to wait
		if(pLinkEntity->HasTargetName())
		{
			const Char* pstrTargetName = pLinkEntity->GetTargetName();

			if(pNPC)
			{
				edict_t* pTriggerEdict = Util::FindEntityByTarget(nullptr, pstrTargetName);
				if(pTriggerEdict)
				{
					CBaseEntity* pEntity = CBaseEntity::GetClass(pTriggerEdict);
					if(pEntity->IsTriggerMultipleEntity())
						pEntity->TriggerWait(pNPC);
				}
			}

			// See if there are other doors with the same name
			edict_t* pEdict = nullptr;
			while(true)
			{
				pEdict = Util::FindEntityByTargetName(pEdict, pstrTargetName);
				if(!pEdict)
					break;

				if(pEdict == pLinkEntity->GetEdict())
					continue;

				// Only do anything if it's an actual func_door
				CBaseEntity* pTargetEntity = CBaseEntity::GetClass(pEdict);
				if(!pTargetEntity->IsFuncDoorEntity())
					continue;

				entitesArray.push_back(pTargetEntity);
			}
		}
		else if(pLinkEntity->IsFuncDoorEntity())
		{
			Vector mins, maxs;
			Vector doorOrigin = pLinkEntity->GetOrigin();
			for(Uint32 i = 0; i < 3; i++)
			{
				mins[i] = doorOrigin[i] - CBaseNPC::NPC_DOOR_SEARCH_RADIUS;
				maxs[i] = doorOrigin[i] + CBaseNPC::NPC_DOOR_SEARCH_RADIUS;
			}

			// See if there are other doors with the same name
			edict_t* pEdict = nullptr;
			while(true)
			{
				pEdict = Util::FindEntityInBBox(pEdict, mins, maxs);
				if(!pEdict)
					break;

				if(pEdict == pLinkEntity->GetEdict())
					continue;

				// Only do anything if it's an actual func_door
				CBaseEntity* pTargetEntity = CBaseEntity::GetClass(pEdict);
				if(!pTargetEntity->IsFuncDoorRotatingEntity())
					continue;

				// It needs to be on the same axis, either on x or y
				Vector targetDoorOrigin = pTargetEntity->GetOrigin();
				if(targetDoorOrigin[0] != doorOrigin[0] && targetDoorOrigin[1] != doorOrigin[1])
					continue;

				// Make sure we can actually trigger this
				if(pTargetEntity->HasSpawnFlag(CFuncDoor::FL_NO_NPCS)
					|| pTargetEntity->GetToggleState() == TS_AT_TOP
					|| pTargetEntity->GetToggleState() == TS_GOING_UP)
					continue;

				entitesArray.push_back(pTargetEntity);
			}
		}
	}

	//=============================================
	// @brief
	//
	//=============================================
	void PrintScreenText( Int32 xcoord, Int32 ycoord, Float lifetime, const Char *fmt, ... )
	{
		va_list	vArgPtr;
		Char cMsg[MAX_PATH];
	
		va_start(vArgPtr, fmt);
		vsprintf_s(cMsg, fmt, vArgPtr);
		va_end(vArgPtr);

		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.screentext, nullptr, nullptr);
			gd_engfuncs.pfnMsgWriteString(cMsg);
			gd_engfuncs.pfnMsgWriteInt16(xcoord);
			gd_engfuncs.pfnMsgWriteInt16(ycoord);
			gd_engfuncs.pfnMsgWriteSmallFloat(lifetime);
		gd_engfuncs.pfnUserMessageEnd();
	}

	//=============================================
	// @brief
	//
	//=============================================
	Float DotPoints( const Vector& src, const Vector& check, const Vector& direction )
	{
		Vector lineOfSight;
		lineOfSight = (check - src);
		lineOfSight[2] = 0;
		lineOfSight.Normalize();

		Vector _direction(direction);
		_direction[2] = 0;
		_direction.Normalize();

		return Math::DotProduct(lineOfSight, _direction);
	}

	//=============================================
	// @brief
	//
	//=============================================
	bool CheckToss( CBaseEntity* pEntity, const Vector& spot1, const Vector& spot2, Float gravityAdjust, Vector& outVelocity )
	{
		if(spot2.z - spot1.z > 500)
			return false;

		Vector forward, right, up;
		Math::AngleVectors(pEntity->GetAngles(), &forward, &right, &up);

		Vector _spot2(spot2);
		_spot2 = _spot2 + right * (Common::RandomFloat(-8, 8) + Common::RandomFloat(-16, 16));
		_spot2 = _spot2 + forward * (Common::RandomFloat(-8, 8) + Common::RandomFloat(-16, 16));

		Vector midPoint;
		midPoint = spot1 + (_spot2 - spot1)*0.5;
		Vector traceEnd;
		traceEnd = midPoint + Vector(0, 0, 500);

		trace_t tr;
		Util::TraceLine(midPoint, traceEnd, true, false, pEntity->GetEdict(), tr);
		midPoint = tr.endpos - Vector(0, 0, 15);

		if(midPoint.z < spot1.z || midPoint.z < _spot2.z)
			return false;

		Float dist1 = (midPoint.z - spot1.z);
		Float dist2 = (midPoint.z - _spot2.z);

		Float gravity = gd_engfuncs.pfnGetCvarFloatValue(GRAVITY_CVAR_NAME) * gravityAdjust;
		Float time1 = SDL_sqrt(dist1/(0.5*gravity));
		if(time1 < 0.1)
			return false;

		Float time2 = SDL_sqrt(dist2/(0.5*gravity));

		Vector grenadeVelocity = (_spot2 - spot1)/(time1+time2);
		grenadeVelocity.z = gravity * time1;

		Vector apex = spot1 + grenadeVelocity*time1;
		apex.z = midPoint.z;

		tr = trace_t();
		Util::TraceLine(spot1, apex, false, false, pEntity->GetEdict(), tr);
		if(!tr.noHit())
			return false;

		tr = trace_t();
		Util::TraceLine(_spot2, apex, true, false, pEntity->GetEdict(), tr);
		if(!tr.noHit())
			return false;

		outVelocity = grenadeVelocity;
		return true;
	}

	//=============================================
	// @brief
	//
	//=============================================
	bool CheckThrow( CBaseEntity* pEntity, const Vector& spot1, const Vector& spot2, Float speed, Float gravityAdjust, Vector& outVelocity )
	{
		Vector throwVelocity = (spot2 - spot1);
		Float time = throwVelocity.Length()/speed;
		throwVelocity = throwVelocity * (1.0f/time);

		Float gravity = gd_engfuncs.pfnGetCvarFloatValue(GRAVITY_CVAR_NAME) * gravityAdjust;
		throwVelocity.z += gravity * time * 0.5;

		Vector apex = spot1 + (spot2 - spot1) * 0.5;
		apex.z += 0.5 * gravity * (time*0.5)*(time*0.5);

		trace_t tr;
		Util::TraceLine(spot1, apex, false, false, pEntity->GetEdict(), tr);
		if(!tr.noHit())
			return false;

		Util::TraceLine(spot2, apex, false, false, pEntity->GetEdict(), tr);
		if(!tr.noHit())
			return false;

		outVelocity = throwVelocity;
		return true;
	}

	//=============================================
	// @brief
	//
	//=============================================
	bool IsBoxVisible( CBaseEntity* pLooker, CBaseEntity* pTarget, Vector& targetOrigin, Float size )
	{
		// Don't see through water boundaries
		if(pLooker->GetWaterLevel() != WATERLEVEL_FULL && pTarget->GetWaterLevel() == WATERLEVEL_FULL
			|| pLooker->GetWaterLevel() == WATERLEVEL_FULL && pTarget->GetWaterLevel() != WATERLEVEL_FULL)
			return false;

		const Vector& targetMins = pTarget->GetMins();
		const Vector& targetMaxs = pTarget->GetMaxs();

		Vector lookerOrigin = pLooker->GetOrigin() + pLooker->GetViewOffset();
		for(Uint32 i = 0; i < 5; i++)
		{
			Vector testPosition = pTarget->GetOrigin();
			for(Uint32 j = 0; j < 3; j++)
				testPosition[j] += Common::RandomFloat(targetMins[j] + size, targetMaxs[j] - size);

			trace_t tr;
			Util::TraceLine(lookerOrigin, testPosition, true, false, pLooker->GetEdict(), tr);
			if(tr.noHit())
			{
				targetOrigin = testPosition;
				return true;
			}
		}

		return false;
	}

	//=============================================
	// @brief
	//
	//=============================================
	void CreateBeamPoints( const Vector& startpos, const Vector& endpos, const Char* pstrSpriteName, Uint32 startframe, Float framerate, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Float r, Float g, Float b, Int32 flags )
	{
		Int32 modelindex = gd_engfuncs.pfnPrecacheModel(pstrSpriteName);
		if(modelindex == NO_PRECACHE)
		{
			gd_engfuncs.pfnCon_Printf("%s - '%s' was not precached.\n", __FUNCTION__, pstrSpriteName);
			return;
		}

		Util::CreateBeamPoints(startpos, endpos, modelindex, startframe, framerate, life, width, amplitude, brightness, speed, noisespeed, r, g, b, flags);
	}

	//=============================================
	// @brief
	//
	//=============================================
	void CreateBeamPoints( const Vector& startpos, const Vector& endpos, Int32 modelindex, Uint32 startframe, Float framerate, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Float r, Float g, Float b, Int32 flags )
	{
		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.tempbeam, nullptr, nullptr);
			gd_engfuncs.pfnMsgWriteByte(BEAM_MSG_BEAMPOINTS);
			for(Uint32 i = 0; i < 3; i++)
				gd_engfuncs.pfnMsgWriteFloat(startpos[i]);
			for(Uint32 i = 0; i < 3; i++)
				gd_engfuncs.pfnMsgWriteFloat(endpos[i]);
			gd_engfuncs.pfnMsgWriteInt16(modelindex);
			gd_engfuncs.pfnMsgWriteByte(clamp(startframe, 0, 255));
			gd_engfuncs.pfnMsgWriteSmallFloat(framerate);
			gd_engfuncs.pfnMsgWriteSmallFloat(life*10);
			gd_engfuncs.pfnMsgWriteSmallFloat(width);
			gd_engfuncs.pfnMsgWriteByte(amplitude);
			gd_engfuncs.pfnMsgWriteByte(clamp(r, 0, 255));
			gd_engfuncs.pfnMsgWriteByte(clamp(g, 0, 255));
			gd_engfuncs.pfnMsgWriteByte(clamp(b, 0, 255));
			gd_engfuncs.pfnMsgWriteByte(clamp(brightness*255, 0, 255));
			gd_engfuncs.pfnMsgWriteSmallFloat(speed);
			gd_engfuncs.pfnMsgWriteSmallFloat(noisespeed);
			gd_engfuncs.pfnMsgWriteInt32(flags);
		gd_engfuncs.pfnUserMessageEnd();
	}

	//=============================================
	// @brief
	//
	//=============================================
	void CreateBeamEntities( const CBaseEntity* pstartentity, const CBaseEntity* pendentity, Int32 attachment1, Int32 attachment2, const Char* pstrSpriteName, Uint32 startframe, Float framerate, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Float r, Float g, Float b, Int32 flags )
	{
		Int32 modelindex = gd_engfuncs.pfnPrecacheModel(pstrSpriteName);
		if(modelindex == NO_PRECACHE)
		{
			gd_engfuncs.pfnCon_Printf("%s - '%s' was not precached.\n", __FUNCTION__, pstrSpriteName);
			return;
		}

		Util::CreateBeamEntities(pstartentity, pendentity, attachment1, attachment2, modelindex, startframe, framerate, life, width, amplitude, brightness, speed, noisespeed, r, g, b, flags);
	}

	//=============================================
	// @brief
	//
	//=============================================
	void CreateBeamEntities( const CBaseEntity* pstartentity, const CBaseEntity* pendentity, Int32 attachment1, Int32 attachment2, Int32 modelindex, Uint32 startframe, Float framerate, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Float r, Float g, Float b, Int32 flags )
	{
		if(!pstartentity)
		{
			gd_engfuncs.pfnCon_Printf("%s - Start entity was not a valid entity.\n", __FUNCTION__);
			return;
		}

		if(!pendentity)
		{
			gd_engfuncs.pfnCon_Printf("%s - End entity was not a valid entity.\n", __FUNCTION__);
			return;
		}

		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.tempbeam, nullptr, nullptr);
			gd_engfuncs.pfnMsgWriteByte(BEAM_MSG_BEAMENTS);
			gd_engfuncs.pfnMsgWriteInt16(pstartentity->GetEntityIndex());
			gd_engfuncs.pfnMsgWriteChar(attachment1);
			gd_engfuncs.pfnMsgWriteInt16(pendentity->GetEntityIndex());
			gd_engfuncs.pfnMsgWriteChar(attachment2);
			gd_engfuncs.pfnMsgWriteInt16(modelindex);
			gd_engfuncs.pfnMsgWriteByte(clamp(startframe, 0, 255));
			gd_engfuncs.pfnMsgWriteSmallFloat(framerate);
			gd_engfuncs.pfnMsgWriteSmallFloat(life*10);
			gd_engfuncs.pfnMsgWriteSmallFloat(width);
			gd_engfuncs.pfnMsgWriteByte(amplitude);
			gd_engfuncs.pfnMsgWriteByte(clamp(r, 0, 255));
			gd_engfuncs.pfnMsgWriteByte(clamp(g, 0, 255));
			gd_engfuncs.pfnMsgWriteByte(clamp(b, 0, 255));
			gd_engfuncs.pfnMsgWriteByte(clamp(brightness*255, 0, 255));
			gd_engfuncs.pfnMsgWriteSmallFloat(speed);
			gd_engfuncs.pfnMsgWriteSmallFloat(noisespeed);
			gd_engfuncs.pfnMsgWriteInt32(flags);
		gd_engfuncs.pfnUserMessageEnd();
	}

	//=============================================
	// @brief
	//
	//=============================================
	void CreateBeamEntityPoint( const CBaseEntity* pentity, const Vector& endpos, Int32 attachment, const Char* pstrSpriteName, Uint32 startframe, Float framerate, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Float r, Float g, Float b, Int32 flags )
	{
		Int32 modelindex = gd_engfuncs.pfnPrecacheModel(pstrSpriteName);
		if(modelindex == NO_PRECACHE)
		{
			gd_engfuncs.pfnCon_Printf("%s - '%s' was not precached.\n", __FUNCTION__, pstrSpriteName);
			return;
		}

		Util::CreateBeamEntityPoint(pentity, endpos, attachment, modelindex, startframe, framerate, life, width, amplitude, brightness, speed, noisespeed, r, g, b, flags);
	}

	//=============================================
	// @brief
	//
	//=============================================
	void CreateBeamEntityPoint( const CBaseEntity* pentity, const Vector& endpos, Int32 attachment, Int32 modelindex, Uint32 startframe, Float framerate, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Float r, Float g, Float b, Int32 flags )
	{
		if(!pentity)
		{
			gd_engfuncs.pfnCon_Printf("%s - Start entity was not a valid entity.\n", __FUNCTION__);
			return;
		}

		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.tempbeam, nullptr, nullptr);
			gd_engfuncs.pfnMsgWriteByte(BEAM_MSG_BEAMENTPOINT);
			gd_engfuncs.pfnMsgWriteInt16(pentity->GetEntityIndex());
			gd_engfuncs.pfnMsgWriteChar(attachment);
			for(Uint32 i = 0; i < 3; i++)
				gd_engfuncs.pfnMsgWriteFloat(endpos[i]);
			gd_engfuncs.pfnMsgWriteInt16(modelindex);
			gd_engfuncs.pfnMsgWriteByte(clamp(startframe, 0, 255));
			gd_engfuncs.pfnMsgWriteSmallFloat(framerate);
			gd_engfuncs.pfnMsgWriteSmallFloat(life*10);
			gd_engfuncs.pfnMsgWriteSmallFloat(width);
			gd_engfuncs.pfnMsgWriteByte(amplitude);
			gd_engfuncs.pfnMsgWriteByte(clamp(r, 0, 255));
			gd_engfuncs.pfnMsgWriteByte(clamp(g, 0, 255));
			gd_engfuncs.pfnMsgWriteByte(clamp(b, 0, 255));
			gd_engfuncs.pfnMsgWriteByte(clamp(brightness*255, 0, 255));
			gd_engfuncs.pfnMsgWriteSmallFloat(speed);
			gd_engfuncs.pfnMsgWriteSmallFloat(noisespeed);
			gd_engfuncs.pfnMsgWriteInt32(flags);
		gd_engfuncs.pfnUserMessageEnd();
	}

	//=============================================
	// @brief
	//
	//=============================================
	void CreateBeamSprite( const Vector& startpos, const Vector& endpos, const Char* pstrBeamSpriteName, const Char* pstrSpriteName, Uint32 startframe, Float framerate, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Float r, Float g, Float b, Int32 flags, Float sprscale, rendermode_t sprrendermode, Float spralpha )
	{
		Int32 modelindex = gd_engfuncs.pfnPrecacheModel(pstrBeamSpriteName);
		if(modelindex == NO_PRECACHE)
		{
			gd_engfuncs.pfnCon_Printf("%s - '%s' was not precached.\n", __FUNCTION__, pstrBeamSpriteName);
			return;
		}

		Int32 sprmodelindex = gd_engfuncs.pfnPrecacheModel(pstrSpriteName);
		if(modelindex == NO_PRECACHE)
		{
			gd_engfuncs.pfnCon_Printf("%s - '%s' was not precached.\n", __FUNCTION__, pstrSpriteName);
			return;
		}

		Util::CreateBeamSprite(startpos, endpos, modelindex, sprmodelindex, startframe, framerate, life, width, amplitude, brightness, speed, noisespeed, r, g, b, flags, sprscale, sprrendermode, spralpha);
	}

	//=============================================
	// @brief
	//
	//=============================================
	void CreateBeamSprite( const Vector& startpos, const Vector& endpos, Int32 modelindex, Int32 sprmodelindex, Uint32 startframe, Float framerate, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Float r, Float g, Float b, Int32 flags, Float sprscale, rendermode_t sprrendermode, Float spralpha )
	{
		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.tempbeam, nullptr, nullptr);
			gd_engfuncs.pfnMsgWriteByte(BEAM_MSG_BEAMSPRITE);
			for(Uint32 i = 0; i < 3; i++)
				gd_engfuncs.pfnMsgWriteFloat(startpos[i]);
			for(Uint32 i = 0; i < 3; i++)
				gd_engfuncs.pfnMsgWriteFloat(endpos[i]);
			gd_engfuncs.pfnMsgWriteInt16(modelindex);
			gd_engfuncs.pfnMsgWriteByte(clamp(startframe, 0, 255));
			gd_engfuncs.pfnMsgWriteSmallFloat(framerate);
			gd_engfuncs.pfnMsgWriteSmallFloat(life*10);
			gd_engfuncs.pfnMsgWriteSmallFloat(width);
			gd_engfuncs.pfnMsgWriteByte(amplitude);
			gd_engfuncs.pfnMsgWriteByte(clamp(r, 0, 255));
			gd_engfuncs.pfnMsgWriteByte(clamp(g, 0, 255));
			gd_engfuncs.pfnMsgWriteByte(clamp(b, 0, 255));
			gd_engfuncs.pfnMsgWriteByte(clamp(brightness*255, 0, 255));
			gd_engfuncs.pfnMsgWriteSmallFloat(speed);
			gd_engfuncs.pfnMsgWriteSmallFloat(noisespeed);
			gd_engfuncs.pfnMsgWriteInt32(flags);
			gd_engfuncs.pfnMsgWriteInt16(sprmodelindex);
			gd_engfuncs.pfnMsgWriteSmallFloat(sprscale);
			gd_engfuncs.pfnMsgWriteByte(sprrendermode);
			gd_engfuncs.pfnMsgWriteSmallFloat(clamp(spralpha*255, 0, 255));
		gd_engfuncs.pfnUserMessageEnd();
	}

	//=============================================
	// @brief
	//
	//=============================================
	void CreateBeamTorus( const Vector& startpos, const Vector& endpos, const Char* pstrSpriteName, Uint32 startframe, Float framerate, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Float r, Float g, Float b, Int32 flags )
	{
		Int32 modelindex = gd_engfuncs.pfnPrecacheModel(pstrSpriteName);
		if(modelindex == NO_PRECACHE)
		{
			gd_engfuncs.pfnCon_Printf("%s - '%s' was not precached.\n", __FUNCTION__, pstrSpriteName);
			return;
		}

		Util::CreateBeamTorus(startpos, endpos, modelindex, startframe, framerate, life, width, amplitude, brightness, speed, noisespeed, r, g, b, flags);
	}

	//=============================================
	// @brief
	//
	//=============================================
	void CreateBeamTorus( const Vector& startpos, const Vector& endpos, Int32 modelindex, Uint32 startframe, Float framerate, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Float r, Float g, Float b, Int32 flags )
	{
		CreateBeamOfType(BEAM_TORUS, startpos, endpos, modelindex, startframe, framerate, life, width, amplitude, brightness, speed, noisespeed, r, g, b, flags);
	}

	//=============================================
	// @brief
	//
	//=============================================
	void CreateBeamDisk( const Vector& startpos, const Vector& endpos, const Char* pstrSpriteName, Uint32 startframe, Float framerate, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Float r, Float g, Float b, Int32 flags )
	{
		Int32 modelindex = gd_engfuncs.pfnPrecacheModel(pstrSpriteName);
		if(modelindex == NO_PRECACHE)
		{
			gd_engfuncs.pfnCon_Printf("%s - '%s' was not precached.\n", __FUNCTION__, pstrSpriteName);
			return;
		}

		Util::CreateBeamDisk(startpos, endpos, modelindex, startframe, framerate, life, width, amplitude, brightness, speed, noisespeed, r, g, b, flags);
	}

	//=============================================
	// @brief
	//
	//=============================================
	void CreateBeamDisk( const Vector& startpos, const Vector& endpos, Int32 modelindex, Uint32 startframe, Float framerate, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Float r, Float g, Float b, Int32 flags )
	{
		CreateBeamOfType(BEAM_DISK, startpos, endpos, modelindex, startframe, framerate, life, width, amplitude, brightness, speed, noisespeed, r, g, b, flags);
	}

	//=============================================
	// @brief
	//
	//=============================================
	void CreateBeamCylinder( const Vector& startpos, const Vector& endpos, const Char* pstrSpriteName, Uint32 startframe, Float framerate, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Float r, Float g, Float b, Int32 flags )
	{
		Int32 modelindex = gd_engfuncs.pfnPrecacheModel(pstrSpriteName);
		if(modelindex == NO_PRECACHE)
		{
			gd_engfuncs.pfnCon_Printf("%s - '%s' was not precached.\n", __FUNCTION__, pstrSpriteName);
			return;
		}

		Util::CreateBeamCylinder(startpos, endpos, modelindex, startframe, framerate, life, width, amplitude, brightness, speed, noisespeed, r, g, b, flags);
	}

	//=============================================
	// @brief
	//
	//=============================================
	void CreateBeamCylinder( const Vector& startpos, const Vector& endpos, Int32 modelindex, Uint32 startframe, Float framerate, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Float r, Float g, Float b, Int32 flags )
	{
		CreateBeamOfType(BEAM_CYLINDER, startpos, endpos, modelindex, startframe, framerate, life, width, amplitude, brightness, speed, noisespeed, r, g, b, flags);
	}

	//=============================================
	// @brief
	//
	//=============================================
	void CreateBeamOfType( beam_types_t type, const Vector& startpos, const Vector& endpos, Int32 modelindex, Uint32 startframe, Float framerate, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Float r, Float g, Float b, Int32 flags )
	{
		beam_msgtype_t msgtype;
		switch(type)
		{
		case BEAM_TORUS:
			msgtype = BEAM_MSG_BEAMTORUS;
			break;
		case BEAM_CYLINDER:
			msgtype = BEAM_MSG_BEAMCYLINDER;
			break;
		case BEAM_DISK:
			msgtype = BEAM_MSG_BEAMDISK;
			break;
		default:
			{
				gd_engfuncs.pfnCon_Printf("%s - Invalid type '%d' specified.\n", __FUNCTION__, (Int32)type);
				return;
			}
			break;
		}

		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.tempbeam, nullptr, nullptr);
			gd_engfuncs.pfnMsgWriteByte(msgtype);
			for(Uint32 i = 0; i < 3; i++)
				gd_engfuncs.pfnMsgWriteFloat(startpos[i]);
			for(Uint32 i = 0; i < 3; i++)
				gd_engfuncs.pfnMsgWriteFloat(endpos[i]);
			gd_engfuncs.pfnMsgWriteInt16(modelindex);
			gd_engfuncs.pfnMsgWriteByte(clamp(startframe, 0, 255));
			gd_engfuncs.pfnMsgWriteSmallFloat(framerate);
			gd_engfuncs.pfnMsgWriteSmallFloat(life);
			gd_engfuncs.pfnMsgWriteSmallFloat(width);
			gd_engfuncs.pfnMsgWriteByte(amplitude);
			gd_engfuncs.pfnMsgWriteByte(clamp(r, 0, 255));
			gd_engfuncs.pfnMsgWriteByte(clamp(g, 0, 255));
			gd_engfuncs.pfnMsgWriteByte(clamp(b, 0, 255));
			gd_engfuncs.pfnMsgWriteByte(clamp(brightness*255, 0, 255));
			gd_engfuncs.pfnMsgWriteSmallFloat(speed);
			gd_engfuncs.pfnMsgWriteSmallFloat(noisespeed);
			gd_engfuncs.pfnMsgWriteInt32(flags);
		gd_engfuncs.pfnUserMessageEnd();
	}

	//=============================================
	// @brief
	//
	//=============================================
	void CreateBeamFollow( const CBaseEntity* pentity, Int32 attachment, const Char* pstrSpriteName, Float life, Float width, Float amplitude, Float brightness, Float r, Float g, Float b )
	{
		Int32 modelindex = gd_engfuncs.pfnPrecacheModel(pstrSpriteName);
		if(modelindex == NO_PRECACHE)
		{
			gd_engfuncs.pfnCon_Printf("%s - '%s' was not precached.\n", __FUNCTION__, pstrSpriteName);
			return;
		}

		Util::CreateBeamFollow(pentity, attachment, modelindex, life, width, amplitude, brightness, r, g, b);
	}

	//=============================================
	// @brief
	//
	//=============================================
	void CreateBeamFollow( const CBaseEntity* pentity, Int32 attachment, Int32 modelindex, Float life, Float width, Float amplitude, Float brightness, Float r, Float g, Float b )
	{
		if(!pentity)
		{
			gd_engfuncs.pfnCon_Printf("%s - Entity was not a valid entity.\n", __FUNCTION__);
			return;
		}

		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.tempbeam, nullptr, nullptr);
			gd_engfuncs.pfnMsgWriteByte(BEAM_MSG_BEAMFOLLOW);
			gd_engfuncs.pfnMsgWriteInt16(pentity->GetEntityIndex());
			gd_engfuncs.pfnMsgWriteChar(attachment);
			gd_engfuncs.pfnMsgWriteInt16(modelindex);
			gd_engfuncs.pfnMsgWriteSmallFloat(life*10);
			gd_engfuncs.pfnMsgWriteSmallFloat(width);
			gd_engfuncs.pfnMsgWriteByte(amplitude);
			gd_engfuncs.pfnMsgWriteByte(clamp(r, 0, 255));
			gd_engfuncs.pfnMsgWriteByte(clamp(g, 0, 255));
			gd_engfuncs.pfnMsgWriteByte(clamp(b, 0, 255));
			gd_engfuncs.pfnMsgWriteByte(clamp(brightness*255, 0, 255));
		gd_engfuncs.pfnUserMessageEnd();
	}

	//=============================================
	// @brief
	//
	//=============================================
	void CreateBeamRing( const CBaseEntity* pstartentity, const CBaseEntity* pendentity, Int32 attachment1, Int32 attachment2, const Char* pstrSpriteName, Uint32 startframe, Float framerate, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Float r, Float g, Float b, Int32 flags )
	{
		Int32 modelindex = gd_engfuncs.pfnPrecacheModel(pstrSpriteName);
		if(modelindex == NO_PRECACHE)
		{
			gd_engfuncs.pfnCon_Printf("%s - '%s' was not precached.\n", __FUNCTION__, pstrSpriteName);
			return;
		}

		Util::CreateBeamRing(pstartentity, pendentity, attachment1, attachment2, modelindex, startframe, framerate, life, width, amplitude, brightness, speed, noisespeed, r, g, b, flags);
	}

	//=============================================
	// @brief
	//
	//=============================================
	void CreateBeamRing( const CBaseEntity* pstartentity, const CBaseEntity* pendentity, Int32 attachment1, Int32 attachment2, Int32 modelindex, Uint32 startframe, Float framerate, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Float r, Float g, Float b, Int32 flags )
	{
		if(!pstartentity)
		{
			gd_engfuncs.pfnCon_Printf("%s - Start entity was not a valid entity.\n", __FUNCTION__);
			return;
		}

		if(!pendentity)
		{
			gd_engfuncs.pfnCon_Printf("%s - End entity was not a valid entity.\n", __FUNCTION__);
			return;
		}

		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.tempbeam, nullptr, nullptr);
			gd_engfuncs.pfnMsgWriteByte(BEAM_MSG_BEAMRING);
			gd_engfuncs.pfnMsgWriteInt16(pstartentity->GetEntityIndex());
			gd_engfuncs.pfnMsgWriteChar(attachment1);
			gd_engfuncs.pfnMsgWriteInt16(pendentity->GetEntityIndex());
			gd_engfuncs.pfnMsgWriteChar(attachment2);
			gd_engfuncs.pfnMsgWriteInt16(modelindex);
			gd_engfuncs.pfnMsgWriteByte(clamp(startframe, 0, 255));
			gd_engfuncs.pfnMsgWriteSmallFloat(framerate);
			gd_engfuncs.pfnMsgWriteSmallFloat(life*10);
			gd_engfuncs.pfnMsgWriteSmallFloat(width);
			gd_engfuncs.pfnMsgWriteByte(amplitude);
			gd_engfuncs.pfnMsgWriteByte(clamp(r, 0, 255));
			gd_engfuncs.pfnMsgWriteByte(clamp(g, 0, 255));
			gd_engfuncs.pfnMsgWriteByte(clamp(b, 0, 255));
			gd_engfuncs.pfnMsgWriteByte(clamp(brightness*255, 0, 255));
			gd_engfuncs.pfnMsgWriteSmallFloat(speed);
			gd_engfuncs.pfnMsgWriteSmallFloat(noisespeed);
			gd_engfuncs.pfnMsgWriteInt32(flags);
		gd_engfuncs.pfnUserMessageEnd();
	}

	//=============================================
	// @brief
	//
	//=============================================
	void KillEntityBeams( const CBaseEntity* pentity )
	{
		if(!pentity)
		{
			gd_engfuncs.pfnCon_Printf("%s - Start entity was not a valid entity.\n", __FUNCTION__);
			return;
		}

		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.tempbeam, nullptr, nullptr);
			gd_engfuncs.pfnMsgWriteByte(BEAM_MSG_KILLENTITYBEAMS);
			gd_engfuncs.pfnMsgWriteInt16(pentity->GetEntityIndex());
		gd_engfuncs.pfnUserMessageEnd();
	}

	//=============================================
	// @brief
	//
	//=============================================
	Int32 GetBoneIndexFromTrace( const trace_t& tr )
	{
		if(tr.hitentity == NO_ENTITY_INDEX || tr.hitbox == NO_POSITION)
			return NO_POSITION;

		CBaseEntity* pEntity = Util::GetEntityFromTrace(tr);
		if(!pEntity || !pEntity->HasModelName())
			return NO_POSITION;

		const cache_model_t* pmodel = gd_engfuncs.pfnGetModel(pEntity->GetModelIndex());
		if(!pmodel || pmodel->type != MOD_VBM)
			return NO_POSITION;

		const studiohdr_t* phdr = pmodel->getVBMCache()->pstudiohdr;
		const mstudiobbox_t* pbbox = phdr->getHitBox(tr.hitbox);
		if(!pbbox)
			return NO_POSITION;

		return pbbox->bone;
	}

	//=============================================
	// @brief
	//
	//=============================================
	Int64 GetBodyValueForSubmodel( const Char* pstrModelName, const Char* pstrSubmodelName )
	{
		const cache_model_t* pmodel = gd_engfuncs.pfnGetModelByName(pstrModelName);
		if(!pmodel)
		{
			gd_engfuncs.pfnCon_Printf("%s - Model '%s' was not precached.\n", __FUNCTION__, pstrModelName);
			return NO_POSITION;
		}

		if(pmodel->type != MOD_VBM)
		{
			gd_engfuncs.pfnCon_Printf("%s - Model '%s' is not a VBM model.\n", __FUNCTION__, pstrModelName);
			return NO_POSITION;
		}

		// get pointer to studio data
		const vbmcache_t* pcache = pmodel->getVBMCache();
		const vbmheader_t* pvbmheader = pcache->pvbmhdr;
		if(!pvbmheader)
		{
			gd_engfuncs.pfnCon_Printf("%s - Called on model '%s' which has no valid VBM data.\n", __FUNCTION__, pstrModelName);
			return NO_POSITION;
		}

		Int64 bodyvalue = NO_POSITION;
		for(Uint32 i = 0; i < pvbmheader->numbodyparts; i++)
		{
			const vbmbodypart_t* pbodypart = pvbmheader->getBodyPart(i);

			Uint32 j = 0;
			for(; j < pbodypart->numsubmodels; j++)
			{
				const vbmsubmodel_t* psubmodel = pbodypart->getSubmodel(pvbmheader, j);
				if(!qstrcmp(psubmodel->name, pstrSubmodelName))
				{
					bodyvalue = j*pbodypart->base;
					break;
				}
			}

			if(j != (Uint32)pbodypart->numsubmodels)
				break;
		}

		if(bodyvalue == NO_POSITION)
		{
			gd_engfuncs.pfnCon_Printf("%s - Submodel '%s' not found in '%s'.\n", __FUNCTION__, pstrSubmodelName, pstrModelName);
			return NO_POSITION;
		}

		return bodyvalue;
	}
}