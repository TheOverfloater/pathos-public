/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"

#include "vector.h"
#include "edict.h"
#include "gamedll.h"
#include "gdll_interface.h"
#include "entities.h"
#include "baseentity.h"
#include "com_math.h"
#include "gamevars.h"
#include "envfog.h"
#include "globalstate.h"
#include "funcmonitor.h"
#include "funcportalsurface.h"
#include "save_shared.h"
#include "cache_model.h"
#include "brushmodel_shared.h"
#include "ehandle.h"
#include "player.h"
#include "envpossky.h"

// TRUE if we're in InitializeEntities
bool g_bInInitializeEntities = false;

//
// Data given by engine for the save-restore process
//
saverestore_data_t g_saveRestoreData;

//
// Field definitions for entity_state_t
//
entity_data_desc_t g_edictStateFields[] = 
{
	DEFINE_DATA_FIELD( entity_state_t, origin, EFIELD_COORD ),
	DEFINE_DATA_FIELD( entity_state_t, prevorigin, EFIELD_COORD ),
	DEFINE_DATA_FIELD( entity_state_t, velocity, EFIELD_VECTOR ),
	DEFINE_DATA_FIELD( entity_state_t, basevelocity, EFIELD_VECTOR ),
	DEFINE_DATA_FIELD( entity_state_t, movedir, EFIELD_VECTOR ),
	DEFINE_DATA_FIELD( entity_state_t, angles, EFIELD_VECTOR ),
	DEFINE_DATA_FIELD( entity_state_t, avelocity, EFIELD_VECTOR ),
	DEFINE_DATA_FIELD( entity_state_t, punchangles, EFIELD_VECTOR ),
	DEFINE_DATA_FIELD( entity_state_t, punchamount, EFIELD_VECTOR ),
	DEFINE_DATA_FIELD( entity_state_t, viewangles, EFIELD_VECTOR ),
	DEFINE_DATA_FIELD( entity_state_t, view_offset, EFIELD_VECTOR ),
	DEFINE_DATA_FIELD( entity_state_t, fixangles, EFIELD_BOOLEAN ),
	DEFINE_DATA_FIELD( entity_state_t, idealpitch, EFIELD_FLOAT ),
	DEFINE_DATA_FIELD( entity_state_t, pitchspeed, EFIELD_FLOAT ),
	DEFINE_DATA_FIELD( entity_state_t, idealyaw, EFIELD_FLOAT ),
	DEFINE_DATA_FIELD( entity_state_t, yawspeed, EFIELD_FLOAT ),
	DEFINE_DATA_FIELD( entity_state_t, speed, EFIELD_FLOAT ),
	DEFINE_DATA_FIELD( entity_state_t, stamina, EFIELD_FLOAT ),
	DEFINE_DATA_FIELD( entity_state_t, modelindex, EFIELD_INT32 ),
	DEFINE_DATA_FIELD( entity_state_t, absmin, EFIELD_COORD ),
	DEFINE_DATA_FIELD( entity_state_t, absmax, EFIELD_COORD ),
	DEFINE_DATA_FIELD_GLOBAL( entity_state_t, mins, EFIELD_VECTOR ),
	DEFINE_DATA_FIELD_GLOBAL( entity_state_t, maxs, EFIELD_VECTOR ),
	DEFINE_DATA_FIELD_GLOBAL( entity_state_t, size, EFIELD_VECTOR ),
	DEFINE_DATA_FIELD( entity_state_t, ltime, EFIELD_TIME ),
	DEFINE_DATA_FIELD( entity_state_t, nextthink, EFIELD_TIME ),
	DEFINE_DATA_FIELD( entity_state_t, movetype, EFIELD_INT32 ),
	DEFINE_DATA_FIELD( entity_state_t, solid, EFIELD_INT32 ),
	DEFINE_DATA_FIELD( entity_state_t, skin, EFIELD_INT32 ),
	DEFINE_DATA_FIELD( entity_state_t, body, EFIELD_INT64 ),
	DEFINE_DATA_FIELD( entity_state_t, effects, EFIELD_INT64 ),
	DEFINE_DATA_FIELD( entity_state_t, gravity, EFIELD_FLOAT ),
	DEFINE_DATA_FIELD( entity_state_t, friction, EFIELD_FLOAT ),
	DEFINE_DATA_FIELD( entity_state_t, sequence, EFIELD_INT32 ),
	DEFINE_DATA_FIELD( entity_state_t, frame, EFIELD_FLOAT ),
	DEFINE_DATA_FIELD( entity_state_t, animtime, EFIELD_TIME ),
	DEFINE_DATA_FIELD( entity_state_t, framerate, EFIELD_FLOAT ),
	DEFINE_DATA_FIELD_ARRAY( entity_state_t, controllers, EFIELD_FLOAT, MAX_CONTROLLERS ),
	DEFINE_DATA_FIELD_ARRAY( entity_state_t, blending, EFIELD_FLOAT, MAX_BLENDING ),
	DEFINE_DATA_FIELD( entity_state_t, scale, EFIELD_FLOAT ),
	DEFINE_DATA_FIELD( entity_state_t, rendertype, EFIELD_INT32 ),
	DEFINE_DATA_FIELD( entity_state_t, rendermode, EFIELD_INT32 ),
	DEFINE_DATA_FIELD( entity_state_t, renderamt, EFIELD_FLOAT ),
	DEFINE_DATA_FIELD( entity_state_t, rendercolor, EFIELD_VECTOR ),
	DEFINE_DATA_FIELD( entity_state_t, renderfx, EFIELD_INT32 ),
	DEFINE_DATA_FIELD( entity_state_t, numsegments, EFIELD_INT32 ),
	DEFINE_DATA_FIELD( entity_state_t, lightorigin, EFIELD_COORD ),
	DEFINE_DATA_FIELD( entity_state_t, health, EFIELD_FLOAT ),
	DEFINE_DATA_FIELD( entity_state_t, maxhealth, EFIELD_FLOAT ),
	DEFINE_DATA_FIELD( entity_state_t, takedamage, EFIELD_FLOAT ),
	DEFINE_DATA_FIELD( entity_state_t, frags, EFIELD_INT32 ),
	DEFINE_DATA_FIELD( entity_state_t, weapons, EFIELD_INT64 ),
	DEFINE_DATA_FIELD( entity_state_t, buttons, EFIELD_INT32 ),
	DEFINE_DATA_FIELD( entity_state_t, oldbuttons, EFIELD_INT32 ),
	DEFINE_DATA_FIELD( entity_state_t, impulse, EFIELD_INT32 ),
	DEFINE_DATA_FIELD( entity_state_t, dmginflictor, EFIELD_ENTINDEX ),
	DEFINE_DATA_FIELD( entity_state_t, enemy, EFIELD_ENTINDEX ),
	DEFINE_DATA_FIELD( entity_state_t, aiment, EFIELD_ENTINDEX ),
	DEFINE_DATA_FIELD( entity_state_t, owner, EFIELD_ENTINDEX ),
	DEFINE_DATA_FIELD( entity_state_t, groundent, EFIELD_ENTINDEX ),
	DEFINE_DATA_FIELD( entity_state_t, parent, EFIELD_ENTINDEX ),
	DEFINE_DATA_FIELD( entity_state_t, spawnflags, EFIELD_INT64 ),
	DEFINE_DATA_FIELD( entity_state_t, flags, EFIELD_UINT64 ),
	DEFINE_DATA_FIELD( entity_state_t, waterlevel, EFIELD_INT32 ),
	DEFINE_DATA_FIELD( entity_state_t, watertype, EFIELD_INT32 ),
	DEFINE_DATA_FIELD( entity_state_t, maxspeed, EFIELD_FLOAT ),
	DEFINE_DATA_FIELD( entity_state_t, fov, EFIELD_FLOAT ),
	DEFINE_DATA_FIELD( entity_state_t, forcehull, EFIELD_INT32 ),
	DEFINE_DATA_FIELD( entity_state_t, induck, EFIELD_BOOLEAN ),
	DEFINE_DATA_FIELD( entity_state_t, timestepsound, EFIELD_INT32 ),
	DEFINE_DATA_FIELD( entity_state_t, swimtime, EFIELD_INT32 ),
	DEFINE_DATA_FIELD( entity_state_t, ducktime, EFIELD_INT32 ),
	DEFINE_DATA_FIELD( entity_state_t, planezcap, EFIELD_FLOAT ),
	DEFINE_DATA_FIELD( entity_state_t, fallvelocity, EFIELD_FLOAT ),
	DEFINE_DATA_FIELD( entity_state_t, parentoffset, EFIELD_VECTOR ),
	DEFINE_DATA_FIELD( entity_state_t, endpos, EFIELD_COORD ),
	DEFINE_DATA_FIELD( entity_state_t, startpos, EFIELD_COORD ),
	DEFINE_DATA_FIELD( entity_state_t, armorvalue, EFIELD_FLOAT ),
	DEFINE_DATA_FIELD( entity_state_t, deadstate, EFIELD_INT32 ),
	DEFINE_DATA_FIELD( entity_state_t, dmgtaken, EFIELD_FLOAT ),
	DEFINE_DATA_FIELD( entity_state_t, iuser1, EFIELD_INT32 ),
	DEFINE_DATA_FIELD( entity_state_t, iuser2, EFIELD_INT32 ),
	DEFINE_DATA_FIELD( entity_state_t, iuser3, EFIELD_INT32 ),
	DEFINE_DATA_FIELD( entity_state_t, iuser4, EFIELD_INT32 ),
	DEFINE_DATA_FIELD( entity_state_t, iuser5, EFIELD_INT32 ),
	DEFINE_DATA_FIELD( entity_state_t, iuser6, EFIELD_INT32 ),
	DEFINE_DATA_FIELD( entity_state_t, iuser7, EFIELD_INT32 ),
	DEFINE_DATA_FIELD( entity_state_t, iuser8, EFIELD_INT32 ),
	DEFINE_DATA_FIELD( entity_state_t, fuser1, EFIELD_FLOAT ),
	DEFINE_DATA_FIELD( entity_state_t, fuser2, EFIELD_FLOAT ),
	DEFINE_DATA_FIELD( entity_state_t, fuser3, EFIELD_FLOAT ),
	DEFINE_DATA_FIELD( entity_state_t, fuser4, EFIELD_FLOAT ),
	DEFINE_DATA_FIELD( entity_state_t, vuser1, EFIELD_VECTOR ),
	DEFINE_DATA_FIELD( entity_state_t, vuser2, EFIELD_VECTOR ),
	DEFINE_DATA_FIELD( entity_state_t, vuser3, EFIELD_VECTOR ),
	DEFINE_DATA_FIELD( entity_state_t, vuser4, EFIELD_VECTOR ),
};

entity_data_desc_t g_edictStringFields[] = 
{
	DEFINE_DATA_FIELD( edict_fields_t, classname, EFIELD_STRING ),
	DEFINE_DATA_FIELD_GLOBAL( edict_fields_t, globalname, EFIELD_STRING ),
	DEFINE_DATA_FIELD_GLOBAL( edict_fields_t, modelname, EFIELD_MODELNAME ),

	DEFINE_DATA_FIELD_GLOBAL( edict_fields_t, target, EFIELD_STRING ),
	DEFINE_DATA_FIELD_GLOBAL( edict_fields_t, targetname, EFIELD_STRING ),
	DEFINE_DATA_FIELD( edict_fields_t, netname, EFIELD_STRING ),
	DEFINE_DATA_FIELD( edict_fields_t, message, EFIELD_STRING ),
	DEFINE_DATA_FIELD_GLOBAL( edict_fields_t, parent, EFIELD_STRING ),
	DEFINE_DATA_FIELD( edict_fields_t, viewmodel, EFIELD_STRING ),
	DEFINE_DATA_FIELD( edict_fields_t, noise, EFIELD_STRING ),
	DEFINE_DATA_FIELD( edict_fields_t, noise1, EFIELD_STRING ),
	DEFINE_DATA_FIELD( edict_fields_t, noise2, EFIELD_STRING ),
	DEFINE_DATA_FIELD( edict_fields_t, noise3, EFIELD_STRING )
};

//=============================================
// @brief
//
//=============================================
bool DispatchSpawn( edict_t* pedict )
{
	if(Util::IsNullEntity(pedict))
		return false;

	CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);

	// Spawn the entity
	bool result = pEntity->Spawn();
	if(pedict->state.flags & FL_REMOVE_ON_SPAWN)
	{
		// If flagged for removal, remove it right now
		gd_engfuncs.pfnRemoveEntity(pedict);
		return result;
	}

	// Manage global state
	if(pedict->fields.globalname != NO_STRING_VALUE)
		ManageEntityGlobalState(pedict, pEntity);

	// Make sure it's linked properly
	gd_engfuncs.pfnSetOrigin(pedict, pedict->state.origin);

	return result;
}

//=============================================
// @brief
//
//=============================================
bool UpdateTransitionedGlobalEntity( edict_t* pedict )
{
	const Char* pstrGlobalName = gd_engfuncs.pfnGetString(pedict->fields.globalname);
	if(!pstrGlobalName || !qstrlen(pstrGlobalName))
	{
		Util::EntityConPrintf(pedict, "Transitioning global entity with no globalname!\n");
		return false;
	}

	const globalstate_t* pglobalstate = gGlobalStates.GetGlobalState(pstrGlobalName);
	if(!qstrcmp(pglobalstate->mapname, g_pGameVars->levelname))
	{
		Util::EntityConPrintf(pedict, "Transitioning global entity's state was already set!\n");
		return false;
	}

	// Update the global state of this entity
	gGlobalStates.UpdateGlobalStateMapName(pstrGlobalName);

	// Call entity to restore stuff
	CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
	if(pEntity)
		pEntity->OnOverrideEntity();

	return true;
}

//=============================================
// @brief
//
//=============================================
void ManageEntityGlobalState( edict_t* pedict, CBaseEntity* pEntity )
{
	// Manage global states
	const Char* pstrGlobalName = pEntity->GetGlobalName();
	if(pEntity->HasGlobalName())
	{
		const globalstate_t* pglobalstate = gGlobalStates.GetGlobalState(pstrGlobalName);
		if(pglobalstate)
		{
			if(pglobalstate->state == GLOBAL_DEAD)
			{
				// Entity is dead, so remove it
				gd_engfuncs.pfnRemoveEntity(pedict);
			}
			else if(qstrcmp(pglobalstate->mapname, g_pGameVars->levelname))
			{
				// Make it dormant until restored
				pEntity->MakeEntityDormant();
			}
		}
		else
		{
			// Just spawned, so set state to ON
			gGlobalStates.SetGlobalState(pstrGlobalName, GLOBAL_ON);
		}
	}
}

//=============================================
// @brief
//
//=============================================
void DispatchThink( edict_t* pedict )
{
	if(Util::IsNullEntity(pedict))
		return;

	CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
	pEntity->CallThink();
}

//=============================================
// @brief
//
//=============================================
void DispatchTouch( edict_t* pedict, edict_t* pother )
{
	if(Util::IsNullEntity(pedict) || Util::IsNullEntity(pother))
		return;

	CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
	CBaseEntity* pOtherEntity = CBaseEntity::GetClass(pother);
	pEntity->CallTouch(pOtherEntity);
}

//=============================================
// @brief
//
//=============================================
void DispatchBlocked( edict_t* pedict, edict_t* pother )
{
	if(Util::IsNullEntity(pedict))
		return;

	CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
	CBaseEntity* pOtherEntity = CBaseEntity::GetClass(pother);

	pEntity->CallBlocked(pOtherEntity);
}

//=============================================
// @brief
//
//=============================================
bool DispatchRestore( edict_t* pedict, bool istransferglobalentity )
{
	if(Util::IsNullEntity(pedict))
		return false;

	CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);

	// Manage global states of entities
	if(!istransferglobalentity && pedict->fields.globalname != NO_STRING_VALUE)
		ManageEntityGlobalState(pedict, pEntity);

	// Don't restore dormant entities, transition save will do that
	if(pEntity->IsDormant())
		return true;

	if(istransferglobalentity)
	{
		// Update transitioned global entity
		if(!UpdateTransitionedGlobalEntity(pedict))
			return false;
	}

	// Precache resources
	pEntity->Precache();

	// Restore the entity
	return pEntity->Restore();
}

//=============================================
// @brief
//
//=============================================
void DispatchDeclareSaveFields( edict_t* pedict )
{
	if(Util::IsNullEntity(pedict))
		return;

	CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
	pEntity->DeclareSaveFields();
}

//=============================================
// @brief
//
//=============================================
void DispatchCrossedWater( edict_t* pedict, bool entering )
{
	switch(Common::RandomLong(0, 2))
	{
	case 0:
		Util::EmitAmbientSound(pedict->state.origin, "impact/water_splash1.wav", VOL_NORM, ATTN_MEDIUM, Common::RandomLong(90, 110));
		break;
	case 1:
		Util::EmitAmbientSound(pedict->state.origin, "impact/water_splash2.wav", VOL_NORM, ATTN_MEDIUM, Common::RandomLong(90, 110));
		break;
	case 2:
		Util::EmitAmbientSound(pedict->state.origin, "impact/water_splash3.wav", VOL_NORM, ATTN_MEDIUM, Common::RandomLong(90, 110));
		break;
	}

	Util::CreateParticles("water_impact_cluster.txt", pedict->state.origin, Vector(0, 0, 1), PART_SCRIPT_CLUSTER);
}

//=============================================
// @brief
//
//=============================================
bool ShouldCollide( edict_t* pedict, edict_t* pOther )
{
	return true;
}

//=============================================
// @brief
//
//=============================================
bool RunEntityPhysics( edict_t* pedict )
{
	return false;
}

//=============================================
// @brief
//
//=============================================
void FreeEntity( edict_t* pedict )
{
	if(!pedict->pprivatedata)
		return;

	// Call to clean up
	CBaseEntity* pEntity = reinterpret_cast<CBaseEntity*>(pedict->pprivatedata);
	pEntity->FreeEntity();

	// Delete instance
	delete pEntity;

	// Reset ptr
	pedict->pprivatedata = nullptr;
}

//=============================================
// @brief
//
//=============================================
void OnAimentFreed( edict_t* pedict )
{
	if(!pedict->pprivatedata)
		return;

	// Call to clean up
	CBaseEntity* pEntity = reinterpret_cast<CBaseEntity*>(pedict->pprivatedata);
	pEntity->OnAimentFreed();
}

//=============================================
// @brief
//
//=============================================
void SetAbsBox( edict_t* pedict )
{
	if(Util::IsNullEntity(pedict))
		return;

	CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
	pEntity->SetObjectCollisionBox();
}

//=============================================
// @brief
//
//=============================================
entity_data_desc_t CheckSaveField( entity_data_desc_t desc, Uint64 typesize, Int32 count, const Char* pstrObjectName, const Char* pstrVariableName )
{
	Uint64 expectedsize;
	switch(desc.type)
	{
	case EFIELD_FLOAT:
		expectedsize = sizeof(Float);
		break;
	case EFIELD_TIME:
	case EFIELD_DOUBLE:
		expectedsize = sizeof(Double);
		break;
	case EFIELD_UINT16:
		expectedsize = sizeof(Uint16);
		break;
	case EFIELD_INT16:
		expectedsize = sizeof(Int16);
		break;
	case EFIELD_UINT32:
		expectedsize = sizeof(Uint32);
		break;
	case EFIELD_INT32:
		expectedsize = sizeof(Int32);
		break;
	case EFIELD_UINT64:
		expectedsize = sizeof(Uint64);
		break;
	case EFIELD_INT64:
		expectedsize = sizeof(Int64);
		break;
	case EFIELD_VECTOR:
	case EFIELD_COORD:
		expectedsize = sizeof(Vector);
		break;
	case EFIELD_BOOLEAN:
		expectedsize = sizeof(bool);
		break;
	case EFIELD_BYTE:
		expectedsize = sizeof(byte);
		break;
	case EFIELD_CHAR:
		expectedsize = sizeof(Char);
		break;
	case EFIELD_FUNCPTR:
		expectedsize = sizeof(void*);
		break;
	case EFIELD_ENTINDEX:
		expectedsize = sizeof(entindex_t);
		break;
	case EFIELD_ENTPOINTER:
		expectedsize = sizeof(CBaseEntity*);
		break;
	case EFIELD_EDICT:
		expectedsize = sizeof(edict_t*);
		break;
	case EFIELD_ENTSTATE:
		expectedsize = sizeof(entity_state_t*);
		break;
	case EFIELD_EHANDLE:
		expectedsize = sizeof(CEntityHandle);
		break;
	case EFIELD_MODELNAME:
	case EFIELD_SOUNDNAME:
	case EFIELD_STRING:
		expectedsize = sizeof(string_t);
		break;
	default:
		gd_engfuncs.pfnCon_EPrintf("%s - Unknown type '%d' for field.\n", __FUNCTION__, (Int32)desc.type);
		expectedsize = 0;
		break;
	}

	expectedsize *= count;
	if(expectedsize != typesize)
	{
		gd_engfuncs.pfnCon_Printf("%s - Type size(%d bytes) for member variable '%s' of object '%s' does not match field type size(%d bytes).\n", 
			__FUNCTION__, typesize, pstrVariableName, pstrObjectName, expectedsize);
	}

	return desc;
}

//=============================================
// @brief
//
//=============================================
bool ManageKeyvalue( entity_state_t& es, edict_fields_t& ef, const keyvalue_t& kv )
{
	// Override parent, as it's both in state and fields
	if(!qstrcmp(kv.keyname, "parent"))
	{
		ef.parent = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}

	Uint32 nbFields = sizeof(g_edictStateFields)/sizeof(entity_data_desc_t);
	for(Uint32 i = 0; i < nbFields; i++)
	{
		entity_data_desc_t& field = g_edictStateFields[i];
		if(!qstrcmp(kv.keyname, field.fieldname))
		{
			switch(field.type)
			{
			case EFIELD_FLOAT:
				{
					Float value = (Float)SDL_atof(kv.value);
					byte* pdest = ((byte *)&es + field.offset);
					memcpy(pdest, &value, sizeof(Float));
				}
				break;

			case EFIELD_TIME:
			case EFIELD_DOUBLE:
				{
					Double value = SDL_atof(kv.value);
					byte* pdest = ((byte *)&es + field.offset);
					memcpy(pdest, &value, sizeof(Double));
				}
				break;

			case EFIELD_UINT16:
				(*(Uint16*)((byte *)&es + field.offset)) = SDL_atoi(kv.value);
				break;
			case EFIELD_INT16:
				(*(Int16*)((byte *)&es + field.offset)) = SDL_atoi(kv.value);
				break;

			case EFIELD_UINT32:
				(*(Uint32*)((byte *)&es + field.offset)) = SDL_atoi(kv.value);
				break;
			case EFIELD_INT32:
				(*(Int32*)((byte *)&es + field.offset)) = SDL_atoi(kv.value);
				break;

			case EFIELD_UINT64:
				(*(Uint64*)((byte *)&es + field.offset)) = SDL_atoi(kv.value);
				break;
			case EFIELD_INT64:
				(*(Int64*)((byte *)&es + field.offset)) = SDL_atoi(kv.value);
				break;

			case EFIELD_VECTOR:
			case EFIELD_COORD:
				Common::StringToVector(kv.value, (*(Vector*)((byte *)&es + field.offset)));
				break;
				
			case EFIELD_BOOLEAN:
				(*(bool*)((byte *)&es + field.offset)) = (SDL_atoi(kv.value) == 1) ? true : false;
				break;

			case EFIELD_BYTE:
			case EFIELD_CHAR:
			case EFIELD_FUNCPTR:
			case EFIELD_ENTINDEX:
			case EFIELD_ENTPOINTER:
			case EFIELD_EDICT:
			case EFIELD_ENTSTATE:
			case EFIELD_EHANDLE:
			case EFIELD_MODELNAME:
			case EFIELD_SOUNDNAME:
			case EFIELD_STRING:
			default:
				gd_engfuncs.pfnCon_EPrintf("Error: Invalid field type %d for keyvalue '%s' for entity_state_t.\n", field.type, kv.keyname);
				return false;
			}
			
			return true;
		}
	}

	nbFields = sizeof(g_edictStringFields)/sizeof(entity_data_desc_t);
	for(Uint32 i = 0; i < nbFields; i++)
	{
		entity_data_desc_t& field = g_edictStringFields[i];
		if(!qstrcmp(kv.keyname, field.fieldname))
		{
			switch(field.type)
			{
			case EFIELD_MODELNAME:
			case EFIELD_SOUNDNAME:
			case EFIELD_STRING:
				{
					(*(string_t*)((byte *)&ef + field.offset)) = gd_engfuncs.pfnAllocString(kv.value);
				}
				break;

			case EFIELD_BYTE:
			case EFIELD_CHAR:
			case EFIELD_BOOLEAN:
			case EFIELD_TIME:
			case EFIELD_FLOAT:
			case EFIELD_UINT16:
			case EFIELD_INT16:
			case EFIELD_UINT32:
			case EFIELD_INT32:
			case EFIELD_UINT64:
			case EFIELD_INT64:
			case EFIELD_VECTOR:
			case EFIELD_COORD:
			case EFIELD_ENTINDEX:
			case EFIELD_ENTPOINTER:
			case EFIELD_EDICT:
			case EFIELD_ENTSTATE:
			case EFIELD_EHANDLE:
			case EFIELD_FUNCPTR:
			case EFIELD_DOUBLE:
			default:
				gd_engfuncs.pfnCon_EPrintf("Error: Invalid field type %d for keyvalue '%s' for edict_fields_t.\n", field.type, kv.keyname);
				return false;
			}

			return true;
		}
	}

	return false;
}

//=============================================
// @brief
//
//=============================================
void SaveEntityState( entity_state_t& es, bool istransitionsave )
{
	Uint32 nbFields = sizeof(g_edictStateFields)/sizeof(entity_data_desc_t);
	for(Uint32 i = 0; i < nbFields; i++)
	{
		entity_data_desc_t& field = g_edictStateFields[i];
		byte* pdata = ((byte *)&es + field.offset);

		switch(field.type)
		{
		case EFIELD_FLOAT:
			{
				if(!istransitionsave && !(field.flags & EFIELD_SAVE_ALWAYS) && Util::IsDataEmpty(pdata, sizeof(Float)*field.size))
					continue;

				gd_engfuncs.pfnSaveWriteFloat(field.fieldname.c_str(), pdata, field.size, field.type); 
			}
			break;
		case EFIELD_DOUBLE:
			{
				if(!istransitionsave && !(field.flags & EFIELD_SAVE_ALWAYS) && Util::IsDataEmpty(pdata, sizeof(Double)*field.size))
					continue;

				gd_engfuncs.pfnSaveWriteDouble(field.fieldname.c_str(), pdata, field.size, field.type); 
			}
			break;
		case EFIELD_ENTINDEX:
			{
				if(!istransitionsave && !(field.flags & EFIELD_SAVE_ALWAYS))
				{
					Uint32 j = 0;
					for(; j < field.size; j++)
					{
						const entindex_t* pvalue = reinterpret_cast<entindex_t*>(pdata)+j;
						if(*pvalue != NO_ENTITY_INDEX)
							break;
					}

					if(j == field.size)
						continue;
				}

				gd_engfuncs.pfnSaveWriteEntindex(field.fieldname.c_str(), pdata, field.size, field.type); 
			}
			break;
		case EFIELD_ENTPOINTER:
			{
				bool hasData = false;
				entindex_t *ptempbuffer = new entindex_t[field.size];
				for(Uint32 j = 0; j < field.size; j++)
				{
					CBaseEntity* pEntity = (reinterpret_cast<CBaseEntity**>(pdata))[j];
					if(pEntity)
					{
						ptempbuffer[j] = pEntity->GetEntityIndex();
						hasData = true;
					}
					else
						ptempbuffer[j] = NO_ENTITY_INDEX;
				}

				if(!istransitionsave && !(field.flags & EFIELD_SAVE_ALWAYS) && !hasData)
					continue;

				gd_engfuncs.pfnSaveWriteEntindex(field.fieldname.c_str(), reinterpret_cast<byte*>(ptempbuffer), field.size, field.type);
				delete[] ptempbuffer;
			}
			break;
		case EFIELD_EDICT:
			{
				bool hasData = false;
				entindex_t *ptempbuffer = new entindex_t[field.size];
				for(Uint32 j = 0; j < field.size; j++)
				{
					edict_t* pEntity = (reinterpret_cast<edict_t**>(pdata))[j];
					if(pEntity)
					{
						ptempbuffer[j] = pEntity->entindex;
						hasData = true;
					}
					else
						ptempbuffer[j] = NO_ENTITY_INDEX;
				}

				if(!istransitionsave && !(field.flags & EFIELD_SAVE_ALWAYS) && !hasData)
					continue;

				gd_engfuncs.pfnSaveWriteEntindex(field.fieldname.c_str(), reinterpret_cast<byte*>(ptempbuffer), field.size, field.type);
				delete[] ptempbuffer;
			}
			break;
		case EFIELD_ENTSTATE:
			{
				bool hasData = false;
				entindex_t *ptempbuffer = new entindex_t[field.size];
				for(Uint32 j = 0; j < field.size; j++)
				{
					entity_state_t* pEntity = (reinterpret_cast<entity_state_t**>(pdata))[j];
					if(pEntity)
					{
						ptempbuffer[j] = pEntity->entindex;
						hasData = true;
					}
					else
						ptempbuffer[j] = NO_ENTITY_INDEX;
				}

				if(!istransitionsave && !(field.flags & EFIELD_SAVE_ALWAYS) && !hasData)
					continue;

				gd_engfuncs.pfnSaveWriteEntindex(field.fieldname.c_str(), reinterpret_cast<byte*>(ptempbuffer), field.size, field.type);
				delete[] ptempbuffer;
			}
			break;
		case EFIELD_VECTOR:
			{
				if(!istransitionsave && !(field.flags & EFIELD_SAVE_ALWAYS))
				{
					Uint32 j = 0;
					for(; j < field.size; j++)
					{
						const Vector* pvalue = reinterpret_cast<Vector*>(pdata)+j;
						if(!pvalue->IsZero())
							break;
					}

					if(j == field.size)
						continue;
				}

				gd_engfuncs.pfnSaveWriteVector(field.fieldname.c_str(), pdata, field.size, field.type);
			}
			break;
		case EFIELD_COORD:
			{
				// Coord should always be written, because when restored, we need this entry to do so, and
				// entities like brush models can have zero origins that are still valid positions
				gd_engfuncs.pfnSaveWriteCoord(field.fieldname.c_str(), pdata, field.size, field.type);
			}
			break;
		case EFIELD_INT16:
			{
				if(!istransitionsave && !(field.flags & EFIELD_SAVE_ALWAYS) && Util::IsDataEmpty(pdata, sizeof(Int16)*field.size))
					continue;

				gd_engfuncs.pfnSaveWriteInt16(field.fieldname.c_str(), pdata, field.size, field.type);
			}
			break;
		case EFIELD_UINT16:
			{
				if(!istransitionsave && !(field.flags & EFIELD_SAVE_ALWAYS) && Util::IsDataEmpty(pdata, sizeof(Uint16)*field.size))
					continue;

				gd_engfuncs.pfnSaveWriteUint16(field.fieldname.c_str(), pdata, field.size, field.type);
			}
			break;
		case EFIELD_INT32:
			{
				if(!istransitionsave && !(field.flags & EFIELD_SAVE_ALWAYS) && Util::IsDataEmpty(pdata, sizeof(Int32)*field.size))
					continue;

				gd_engfuncs.pfnSaveWriteInt32(field.fieldname.c_str(), pdata, field.size, field.type);
			}
			break;
		case EFIELD_UINT32:
			{
				if(!istransitionsave && !(field.flags & EFIELD_SAVE_ALWAYS) && Util::IsDataEmpty(pdata, sizeof(Uint32)*field.size))
					continue;

				gd_engfuncs.pfnSaveWriteUint32(field.fieldname.c_str(), pdata, field.size, field.type);
			}
			break;
		case EFIELD_INT64:
			{
				if(!istransitionsave && !(field.flags & EFIELD_SAVE_ALWAYS) && Util::IsDataEmpty(pdata, sizeof(Int64)*field.size))
					continue;

				gd_engfuncs.pfnSaveWriteInt64(field.fieldname.c_str(), pdata, field.size, field.type);
			}
			break;
		case EFIELD_UINT64:
			{
				if(!istransitionsave && !(field.flags & EFIELD_SAVE_ALWAYS) && Util::IsDataEmpty(pdata, sizeof(Uint64)*field.size))
					continue;

				gd_engfuncs.pfnSaveWriteUint64(field.fieldname.c_str(), pdata, field.size, field.type);
			}
			break;
		case EFIELD_FUNCPTR:
			{
				CString functionName;
				void** functionPtr = reinterpret_cast<void**>(&(*pdata));
				if(*functionPtr)
				{
					// Get function name based on pointer address
					functionName = gd_engfuncs.pfnNameForFunction(*functionPtr);

					// Report error
					if(functionName.empty())
						gd_engfuncs.pfnCon_Printf("Couldn't find name for %s function '%p'.\n", field.fieldname.c_str(), reinterpret_cast<void*>(&*functionPtr));
				}
				
				if(!istransitionsave && functionName.empty())
					continue;
					
				gd_engfuncs.pfnSaveWriteRawString(field.fieldname.c_str(), reinterpret_cast<const byte*>(functionName.c_str()), field.type);
			}
			break;
		case EFIELD_BOOLEAN:
			{
				if(!istransitionsave && !(field.flags & EFIELD_SAVE_ALWAYS) && Util::IsDataEmpty(pdata, sizeof(bool)*field.size))
					continue;

				gd_engfuncs.pfnSaveWriteBool(field.fieldname.c_str(), pdata, field.size, field.type);
			}
			break;
		case EFIELD_BYTE:
			{
				if(!istransitionsave && !(field.flags & EFIELD_SAVE_ALWAYS) && Util::IsDataEmpty(pdata, sizeof(byte)*field.size))
					continue;

				gd_engfuncs.pfnSaveWriteByte(field.fieldname.c_str(), pdata, field.size, field.type);
			}
			break;
		case EFIELD_CHAR:
			{
				if(!istransitionsave && !(field.flags & EFIELD_SAVE_ALWAYS) && Util::IsDataEmpty(pdata, sizeof(Char)*field.size))
					continue;

				gd_engfuncs.pfnSaveWriteChar(field.fieldname.c_str(), pdata, field.size, field.type);
			}
			break;
		case EFIELD_TIME:
			{
				if(!istransitionsave && !(field.flags & EFIELD_SAVE_ALWAYS) && Util::IsDataEmpty(pdata, sizeof(Double)*field.size))
					continue;

				gd_engfuncs.pfnSaveWriteTime(field.fieldname.c_str(), pdata, field.size, field.type); 
			}
			break;
		case EFIELD_EHANDLE:
		case EFIELD_STRING:
		case EFIELD_MODELNAME:
		case EFIELD_SOUNDNAME:
		default:
			gd_engfuncs.pfnCon_EPrintf("Error: Invalid field type %d for field '%s' for entity_state_t.\n", field.type, field.fieldname.c_str());
			break;
		}
	}
}

//=============================================
// @brief
//
//=============================================
void SaveEntityFields( edict_fields_t& ef, bool istransitionsave )
{
	Uint32 nbFields = sizeof(g_edictStringFields)/sizeof(entity_data_desc_t);
	for(Uint32 i = 0; i < nbFields; i++)
	{
		entity_data_desc_t& field = g_edictStringFields[i];
		byte* pdata = ((byte *)&ef + field.offset);

		switch(field.type)
		{
		case EFIELD_STRING:
		case EFIELD_MODELNAME:
		case EFIELD_SOUNDNAME:
			{
				if(!istransitionsave && !(field.flags & EFIELD_SAVE_ALWAYS))
				{
					Uint32 j = 0;
					for(; j < field.size; j++)
					{
						string_t stringIndex = Common::ByteToUint32(reinterpret_cast<byte *>(pdata) + sizeof(string_t)*j);
						if(stringIndex != NO_STRING_VALUE)
							break;
					}

					if(j == field.size)
						continue;
				}

				gd_engfuncs.pfnSaveWriteString(field.fieldname.c_str(), pdata, field.size, field.type);
			}
			break;
		case EFIELD_FLOAT:
		case EFIELD_DOUBLE:
		case EFIELD_ENTINDEX:
		case EFIELD_ENTPOINTER:
		case EFIELD_EDICT:
		case EFIELD_ENTSTATE:
		case EFIELD_EHANDLE:
		case EFIELD_VECTOR:
		case EFIELD_COORD:
		case EFIELD_INT16:
		case EFIELD_UINT16:
		case EFIELD_INT32:
		case EFIELD_UINT32:
		case EFIELD_INT64:
		case EFIELD_UINT64:
		case EFIELD_FUNCPTR:
		case EFIELD_BOOLEAN:
		case EFIELD_BYTE:
		case EFIELD_CHAR:
		case EFIELD_TIME:
		default:
			gd_engfuncs.pfnCon_EPrintf("Error: Invalid field type %d for field '%s' for edict_fields_t.\n", field.type, field.fieldname.c_str());
			break;
		}
	}
}

//=============================================
// @brief
//
//=============================================
bool KeyValue( edict_t* pedict, const keyvalue_t& keyvalue )
{
	if(!qstrcmp(keyvalue.keyname, "zhlt_noclip")
		|| !qstrcmp(keyvalue.keyname, "zhlt_lightflags")
		|| !qstrcmp(keyvalue.keyname, "mapversion")
		|| !qstrcmp(keyvalue.keyname, "compiler")
		|| !qstrcmp(keyvalue.keyname, "wad"))
		return true;

	if(!pedict->pprivatedata)
		return false;

	CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);

	// See if entity_state_t and edict_fields_t manages it
	if(!pEntity->ShouldOverrideKeyValue(keyvalue.keyname) 
		&& ManageKeyvalue( pedict->state, pedict->fields, keyvalue ))
		return true;

	// Allow entity to check it
	if(pEntity->KeyValue(keyvalue))
		return true;

	return false;
}

//=============================================
// @brief
//
//=============================================
void SetupVisiblity( edict_t* pclient, byte*& ppvs, byte*& ppas )
{
	if(Util::IsNullEntity(pclient))
	{
		ppvs = nullptr;
		ppas = nullptr;
		return;
	}

	CBaseEntity* pEntity = CBaseEntity::GetClass(pclient);
	if(!pEntity || !pEntity->IsPlayer())
	{
		ppvs = nullptr;
		ppas = nullptr;
		return;
	}

	// Set the view origin
	Vector viewOrg = pEntity->GetVISEyePosition();

	// Grab PVS and PAS data
	ppvs = gd_engfuncs.pfnSetPVS( viewOrg );
	ppas = gd_engfuncs.pfnSetPAS( viewOrg );

	// See if we need to deal with an additional eye position
	Vector additionalPosition;
	if(pEntity->GetAdditionalVISPosition(additionalPosition))
	{
		const cache_model_t* pcache = gd_engfuncs.pfnGetModel(WORLD_MODEL_INDEX);
		if(pcache->type == MOD_BRUSH)
		{
			const brushmodel_t* pbrushmodel = pcache->getBrushmodel();
			const mleaf_t* pleaf = gd_engfuncs.pfnPointInLeaf(additionalPosition, (*pbrushmodel));
			if(pleaf)
			{
				gd_engfuncs.pfnLeafPVS(g_pVISBuffer, g_visBufferSize, (*pleaf), (*pbrushmodel));

				// Add the contents to the serve PVS
				Uint32 bytecount = (pbrushmodel->numleafs + 7) >> 3;
				for(Uint32 i = 0; i < bytecount; i++)
					ppvs[i] |= g_pVISBuffer[i];
			}
		}
	}
}

//=============================================
// @brief
//
//=============================================
bool CheckVisibility( const edict_t& client, const edict_t& entity, const byte* pset )
{
	// If set is null, just send everything
	if(!pset)
		return true;

	// If it's NOVIS, always send
	if((entity.state.effects & EF_NOVIS) || (entity.state.renderfx == RenderFx_NoVISChecks))
		return true;

	// Always send current client
	if(&entity == &client)
		return true;

	// Check skybox if we're a sky entity
	if(entity.state.renderfx == RenderFx_SkyEnt || 
		entity.state.renderfx == RenderFx_SkyEntNC || 
		entity.state.rendertype == RT_SKYWATERENT)
	{
		// Check skybox entity visibility
		if(!CEnvPosSky::CheckSkyboxVisibility(&entity))
			return false;
	}
	else
	{
		// Monitor is a special case. If a monitor sees this entity,
		// always include it in the packet
		if(CFuncMonitor::CheckVISForEntity(&client, &entity, pset))
			return true;

		// Portal is a special case. If a portal sees this entity,
		// always include it in the packet
		if(CFuncPortalSurface::CheckVISForEntity(&client, &entity, pset))
			return true;

		// Check visibility on non-hosts
		if(!Common::CheckVisibility(entity.leafnums, pset))
			return false;

		// Check if fog culls it out
		if(CEnvFog::FogCull(client, entity))
			return false;
	}

	// Try culling by entity's distance
	const CBaseEntity* pEntity = CBaseEntity::GetClass(&entity);
	if(pEntity->CullByVisibilityDistance(&client))
		return false;

	return true;
}

//=============================================
// @brief
//
//=============================================
bool AddPacketEntity( entity_state_t& state, entindex_t entindex, edict_t& entity, const edict_t& client, const byte* pset )
{
	// Only with a valid model
	if(entity.state.modelindex == 0 && !(entity.state.flags & FL_CLIENT))
		return false;

	// Ignore visibility checks and EF_NODRAW on special entities
	if(!(entity.state.effects & EF_ALWAYS_SEND))
	{
		// Don't send entities with EF_NODRAW, only if it's the host
		if((entity.state.effects & EF_NODRAW) && &entity != &client)
			return false;
	}

	// Perform VIS checks
	if(!CheckVisibility(client, entity, pset))
		return false;

	// Call entity's function to manage pre-packet add functions
	CBaseEntity* pEntity = CBaseEntity::GetClass(&entity);
	if(!pEntity)
		return false;

	// Let entity do stuff before adding
	pEntity->PreAddPacket();

	// Clear state each time
	state = entity_state_t();

	// assign data
	state.entindex		= entindex;
	state.animtime		= entity.state.animtime;
	state.modelindex	= entity.state.modelindex;
	state.frame			= entity.state.frame;
	state.framerate		= entity.state.framerate;
	state.skin			= entity.state.skin;
	state.body			= entity.state.body;
	state.effects		= entity.state.effects;
	state.flags			= entity.state.flags;
	state.movetype		= entity.state.movetype;
	state.solid			= entity.state.solid;
	state.health		= entity.state.health;
	state.scale			= entity.state.scale;
	state.rendertype	= entity.state.rendertype;
	state.frags			= entity.state.frags;
	state.friction		= entity.state.friction;
	state.rendermode	= entity.state.rendermode;
	state.renderamt		= entity.state.renderamt;
	state.renderfx		= entity.state.renderfx;
	state.numsegments	= entity.state.numsegments;
	state.waterlevel	= entity.state.waterlevel;
	state.gravity		= entity.state.gravity;
	state.sequence		= entity.state.sequence;

	state.iuser1		= entity.state.iuser1;
	state.iuser2		= entity.state.iuser2;
	state.iuser3		= entity.state.iuser3;
	state.iuser4		= entity.state.iuser4;
	state.iuser5		= entity.state.iuser5;
	state.iuser6		= entity.state.iuser6;
	state.iuser7		= entity.state.iuser7;
	state.iuser8		= entity.state.iuser8;

	state.fuser1		= entity.state.fuser1;
	state.fuser2		= entity.state.fuser2;
	state.fuser3		= entity.state.fuser3;
	state.fuser4		= entity.state.fuser4;

	state.parent		= entity.state.parent;
	state.parentoffset	= entity.state.parentoffset;

	state.lightorigin	= entity.state.lightorigin;

	if(entity.state.flags & FL_CLIENT)
	{
		// These are only used by players
		state.fov			= entity.state.fov;
		state.buttons		= entity.state.buttons;
		state.oldbuttons	= entity.state.oldbuttons;
		state.weapons		= entity.state.weapons;
		state.fallvelocity	= entity.state.fallvelocity;
		state.stamina		= entity.state.stamina;
		state.induck		= entity.state.induck;
		state.ducktime		= entity.state.ducktime;
		state.waterjumptime	= entity.state.waterjumptime;
		state.swimtime		= entity.state.swimtime;
		state.timestepsound	= entity.state.timestepsound;
		state.stepleft		= entity.state.stepleft;
		state.planezcap		= entity.state.planezcap;
		state.weaponanim	= entity.state.weaponanim;
	}

	// Handle controllers
	for(Uint32 i = 0; i < MAX_CONTROLLERS; i++)
		state.controllers[i] = entity.state.controllers[i];

	// Handle blending controllers
	for(Uint32 i = 0; i < MAX_BLENDING; i++)
		state.blending[i] = entity.state.blending[i];

	// Groundent
	if(entity.state.groundent != NO_ENTITY_INDEX)
		state.groundent = entity.state.groundent;
	else
		state.groundent = NO_ENTITY_INDEX;

	// Aiment
	if(entity.state.aiment != NO_ENTITY_INDEX)
		state.aiment = entity.state.aiment;
	else
		state.aiment = NO_ENTITY_INDEX;

	// Owner
	if(entity.state.owner != NO_ENTITY_INDEX)
		state.owner = entity.state.owner;
	else
		state.owner = NO_ENTITY_INDEX;

	// Copy vector types
	Math::VectorCopy(entity.state.origin, state.origin);
	Math::VectorCopy(entity.state.angles, state.angles);
	Math::VectorCopy(entity.state.velocity, state.velocity);
	Math::VectorCopy(entity.state.avelocity, state.avelocity);
	Math::VectorCopy(entity.state.basevelocity, state.basevelocity);

	Math::VectorCopy(entity.state.viewangles, state.viewangles);
	Math::VectorCopy(entity.state.view_offset, state.view_offset);
	Math::VectorCopy(entity.state.punchangles, state.punchangles);

	Math::VectorCopy(entity.state.mins, state.mins);
	Math::VectorCopy(entity.state.maxs, state.maxs);

	Math::VectorCopy(entity.state.startpos, state.startpos);
	Math::VectorCopy(entity.state.endpos, state.endpos);

	Math::VectorCopy(entity.state.rendercolor, state.rendercolor);
	Math::VectorCopy(entity.state.punchangles, state.punchangles);

	Math::VectorCopy(entity.state.vuser1, state.vuser1);
	Math::VectorCopy(entity.state.vuser2, state.vuser2);
	Math::VectorCopy(entity.state.vuser3, state.vuser3);
	Math::VectorCopy(entity.state.vuser4, state.vuser4);

	if(entity.state.effects & EF_UPDATEMODEL)
		entity.state.effects |= EF_UPDATEMODEL;

	return true;
}

//=============================================
// @brief
//
//=============================================
void BeginLoadSave( bool isLoadSave, bool isTransitionSave, bool isTransitionLoad, const Vector* pLandmarkOffset, const CArray<entindex_t>& entityIndexArray )
{
	g_saveRestoreData.transitionsave = isTransitionSave;
	g_saveRestoreData.transitionload = isTransitionLoad;
	g_saveRestoreData.pentityindexarray = &entityIndexArray;

	if(pLandmarkOffset)
		g_saveRestoreData.landmarkOffset = (*pLandmarkOffset);

	if(isLoadSave)
	{
		// Clear global states if it's a regular load save
		gGlobalStates.Clear();
	}
}

//=============================================
// @brief
//
//=============================================
bool ReadEntityStateData( edict_t* pedict, const Char* fieldname, const byte* pdata, Uint32 datasize, Uint32 blockindex, bool istransferglobalentity )
{
	Uint32 nbFields = sizeof(g_edictStateFields)/sizeof(entity_data_desc_t);

	for(Uint32 i = 0; i < nbFields; i++)
	{
		entity_data_desc_t& field = g_edictStateFields[i];
		if(!qstrcmp(fieldname, field.fieldname))
		{
			// Do not modify global fields of transitioning entities
			if(istransferglobalentity && (field.flags & EFIELD_GLOBAL))
				return true;

			assert(blockindex < field.size);

			switch(field.type)
			{
			case EFIELD_CHAR:
				{
					Char *pDestPtr = (Char*)((byte*)&pedict->state + field.offset);
					memcpy(pDestPtr, pdata, sizeof(byte)*field.size);
				}
				break;
			case EFIELD_BYTE:
				{
					byte *pDestPtr = ((byte*)&pedict->state + field.offset);
					memcpy(pDestPtr, pdata, sizeof(byte)*field.size);
				}
				break;
			case EFIELD_ENTINDEX:
				{
					entindex_t entindex = Common::ByteToInt32(pdata);
					if(entindex != NO_ENTITY_INDEX)
						entindex = (*g_saveRestoreData.pentityindexarray)[entindex];

					entindex_t *pDestPtr = (entindex_t *)((byte*)&pedict->state + field.offset);
					(*pDestPtr) = entindex;
				}
				break;
			case EFIELD_ENTPOINTER:
				{
					entindex_t entindex = Common::ByteToInt32(pdata);
					if(entindex == NO_ENTITY_INDEX)
						return true;

					entindex_t realindex = (*g_saveRestoreData.pentityindexarray)[entindex];
					edict_t* pEdict = gd_engfuncs.pfnGetEdictByIndex(realindex);
					CBaseEntity* pEntity = CBaseEntity::GetClass(pEdict);

					CBaseEntity **pDestPtr = (CBaseEntity **)((byte*)&pedict->state + field.offset);
					(*pDestPtr) = pEntity;
				}
				break;
			case EFIELD_EDICT:
				{
					entindex_t entindex = Common::ByteToInt32(pdata);
					if(entindex == NO_ENTITY_INDEX)
						return true;

					entindex_t realindex = (*g_saveRestoreData.pentityindexarray)[entindex];
					edict_t* pEdict = gd_engfuncs.pfnGetEdictByIndex(realindex);

					edict_t** pDestPtr = (edict_t**)((byte *)&pedict->state + field.offset);
					(*pDestPtr) = pEdict;
				}
				break;
			case EFIELD_ENTSTATE:
				{
					entindex_t entindex = Common::ByteToInt32(pdata);
					if(entindex == NO_ENTITY_INDEX)
						return true;

					Uint32 realindex = (*g_saveRestoreData.pentityindexarray)[entindex];
					edict_t* pEdict = gd_engfuncs.pfnGetEdictByIndex(realindex);

					entity_state_t** pEntStatePtr = (entity_state_t**)((byte *)&pedict->state + field.offset);
					(*pEntStatePtr) = &pEdict->state;
				}
				break;
			case EFIELD_TIME:
				{
					Double timeValue = Common::ByteToDouble(pdata);
					if(g_saveRestoreData.transitionsave && timeValue != 0)
						timeValue += g_pGameVars->time;

					byte* pdest = ((byte *)&pedict->state + field.offset) + blockindex * sizeof(Double);
					memcpy(pdest, &timeValue, sizeof(Double));
				}
				break;
			case EFIELD_FLOAT:
				{
					Float floatValue = Common::ByteToFloat(pdata);

					byte* pdest = ((byte *)&pedict->state + field.offset) + blockindex * sizeof(Float);
					memcpy(pdest, &floatValue, sizeof(Float));
				}
				break;
			case EFIELD_DOUBLE:
				{
					Double doubleValue = Common::ByteToDouble(pdata);

					byte* pdest = ((byte *)&pedict->state + field.offset) + blockindex * sizeof(Double);
					memcpy(pdest, &doubleValue, sizeof(Double));
				}
				break;
			case EFIELD_INT16:
				{
					Int16 intValue = Common::ByteToInt16(pdata);
					(*((Int16*)((byte *)&pedict->state + field.offset) + blockindex)) = intValue;
				}
				break;
			case EFIELD_UINT16:
				{
					Uint32 intValue = Common::ByteToUint16(pdata);
					(*((Uint32*)((byte *)&pedict->state + field.offset) + blockindex)) = intValue;
				}
				break;
			case EFIELD_INT32:
				{
					Int32 intValue = Common::ByteToInt32(pdata);
					(*((Int32*)((byte *)&pedict->state + field.offset) + blockindex)) = intValue;
				}
				break;
			case EFIELD_UINT32:
				{
					Uint32 intValue = Common::ByteToUint32(pdata);
					(*((Uint32*)((byte *)&pedict->state + field.offset) + blockindex)) = intValue;
				}
				break;
			case EFIELD_INT64:
				{
					Int64 intValue = Common::ByteToInt64(pdata);
					(*((Int64*)((byte *)&pedict->state + field.offset) + blockindex)) = intValue;
				}
				break;
			case EFIELD_UINT64:
				{
					Uint64 intValue = Common::ByteToUint64(pdata);
					(*((Uint64*)((byte *)&pedict->state + field.offset) + blockindex)) = intValue;
				}
				break;
			case EFIELD_VECTOR:
				{
					Vector vecValue = *(reinterpret_cast<const Vector*>(pdata));
					(*((Vector*)((byte *)&pedict->state + field.offset) + blockindex)) = vecValue;
				}
				break;
			case EFIELD_COORD:
				{
					Vector vecValue = *(reinterpret_cast<const Vector*>(pdata));
					if(g_saveRestoreData.transitionsave)
						Math::VectorAdd(vecValue, g_saveRestoreData.landmarkOffset, vecValue);

					(*((Vector*)((byte *)&pedict->state + field.offset) + blockindex)) = vecValue;
				}
				break;
			case EFIELD_BOOLEAN:
				{
					bool boolValue = ((*pdata) == 0) ? false : true;
					(*((bool*)((byte *)&pedict->state + field.offset) + blockindex)) = boolValue;
				}
				break;
			case EFIELD_MODELNAME:
			case EFIELD_SOUNDNAME:
			case EFIELD_STRING:
			default:
				gd_engfuncs.pfnCon_EPrintf("Error: Invalid field type %d for keyvalue '%s' for entity_state_t.\n", field.type, fieldname);
				return false;
			}

			return true;
		}
	}

	// Manage any missing fields
	gd_engfuncs.pfnCon_EPrintf("Error: Field '%s' not found in entity_state_t.\n", fieldname);
	return false;
}

//=============================================
// @brief
//
//=============================================
bool ReadEntityFieldData( edict_t* pedict, const Char* fieldname, const byte* pdata, Uint32 datasize, Uint32 blockindex, bool istransferglobalentity )
{
	Uint32 nbFields = sizeof(g_edictStringFields)/sizeof(entity_data_desc_t);
	for(Uint32 i = 0; i < nbFields; i++)
	{
		entity_data_desc_t& field = g_edictStringFields[i];
		if(!qstrcmp(fieldname, field.fieldname))
		{
			// Do not modify global fields of transitioning entities
			if(istransferglobalentity && (field.flags & EFIELD_GLOBAL))
				return true;

			assert(blockindex < field.size);

			switch(field.type)
			{
			case EFIELD_MODELNAME:
			case EFIELD_SOUNDNAME:
			case EFIELD_STRING:
				{
					CString value(reinterpret_cast<const Char*>(pdata), datasize);
					string_t& str = (*((string_t*)((byte *)&pedict->fields + field.offset) + blockindex));
					str = gd_engfuncs.pfnAllocString(value.c_str());
				}
				break;

			case EFIELD_TIME:
			case EFIELD_FLOAT:
			case EFIELD_INT16:
			case EFIELD_UINT16:
			case EFIELD_INT32:
			case EFIELD_UINT32:
			case EFIELD_INT64:
			case EFIELD_UINT64:
			case EFIELD_VECTOR:
			case EFIELD_COORD:
			default:
				gd_engfuncs.pfnCon_EPrintf("Error: Invalid field type %d for save field '%s' for entity_state_t.\n", field.type, fieldname);
				return false;
			}

			return true;
		}
	}

	// Manage any missing fields
	gd_engfuncs.pfnCon_EPrintf("Error: Field '%s' not found in entity_fields_t.\n", fieldname);
	return true;
}

//=============================================
// @brief
//
//=============================================
bool ReadEntityClassData( edict_t* pedict, const Char* fieldname, const byte* pdata, Uint32 datasize, Uint32 blockindex, bool istransferglobalentity )
{
	CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
	if(!pEntity)
		return false;

	return pEntity->ReadEntityClassData(fieldname, pdata, datasize, blockindex, istransferglobalentity);
}

//=============================================
// @brief
//
//=============================================
void RunEntityInitFunctions( bool triggerautos )
{
	// Initialize all but trigger_auto entities first
	Uint32 numEntities = gd_engfuncs.pfnGetNbEdicts();
	for(Uint32 i = 0; i < numEntities; i++)
	{
		edict_t* pedict = gd_engfuncs.pfnGetEdictByIndex(i);
		if(pedict->free || !pedict->pprivatedata)
			continue;

		CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);

		if(triggerautos && !pEntity->IsTriggerAutoEntity()
			|| !triggerautos && pEntity->IsTriggerAutoEntity())
			continue;

		// Initialize parenting if needed
		if(pedict->fields.parent != NO_STRING_VALUE)
			pEntity->InitParenting();

		if(pedict->state.flags & FL_INITIALIZE)
		{
			pEntity->InitEntity();

			// Remove flag
			pedict->state.flags &= ~FL_INITIALIZE;
		}

		// For npcs after save-restore, reset the
		// ground entity
		if(pEntity->IsNPC())
			pEntity->GroundEntityNudge(true);
	}
}

//=============================================
// @brief
//
//=============================================
void InitializeEntities( void )
{
	// Set this so specific entities behave differently
	// on game initialization
	g_bInInitializeEntities = true;

	// Initialize all other entities first
	RunEntityInitFunctions(false);
	// Initialize trigger_autos after all other entities
	RunEntityInitFunctions(true);

	// Reset this
	g_bInInitializeEntities = false;
}


//=============================================
// @brief
//
//=============================================
void SendEntityInitMessages( edict_t* pPlayer )
{
	if(!pPlayer)
		return;

	CBaseEntity* pPlayerEntity = CBaseEntity::GetClass(pPlayer);
	if(!pPlayerEntity)
		return;

	Uint32 numEntities = gd_engfuncs.pfnGetNbEdicts();
	for(Uint32 i = 1; i < numEntities; i++)
	{
		edict_t* pedict = gd_engfuncs.pfnGetEdictByIndex(i);
		if(pedict->free || !pedict->pprivatedata)
			continue;

		CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
		pEntity->SendInitMessage(pPlayerEntity);
	}
}

//=============================================
//
//=============================================
edict_t* FindEntityByString( edict_t* pStartEntity, const Char* pstrFieldName, const Char* pstrValue )
{
	if(!pstrFieldName || !qstrlen(pstrFieldName))
		return nullptr;

	if(!pstrValue || !qstrlen(pstrValue))
		return nullptr;

	Int32 fieldoffset = -1;
	for(Uint32 i = 0; i < sizeof(g_edictStringFields)/sizeof(entity_data_desc_t); i++)
	{
		if(!qstrcmp(g_edictStringFields[i].fieldname, pstrFieldName))
		{
			fieldoffset = g_edictStringFields[i].offset;
			break;
		}
	}

	if(fieldoffset == -1)
	{
		gd_engfuncs.pfnCon_Printf("%s - Invalid field name '%s'.\n", __FUNCTION__, pstrFieldName);
		return nullptr;
	}

	Int32 i = pStartEntity ? (pStartEntity->entindex+1) : 0;
	for(; i < gd_engfuncs.pfnGetNbEdicts(); i++)
	{
		edict_t* pedict = gd_engfuncs.pfnGetEdictByIndex(i);
		if(!pedict || pedict->free)
			continue;

		// Retrieve the field value
		string_t& str = (*(string_t*)((byte *)&pedict->fields + fieldoffset));
		if(str == NO_STRING_VALUE)
			continue;

		if(!qstrcmp(gd_engfuncs.pfnGetString(str), pstrValue))
			return pedict;
	}

	return nullptr;
}

//=============================================
//
//=============================================
edict_t* FindGlobalEntity( const Char* pstrClassname, const Char* pstrGlobalName )
{
	for(Int32 i = 0; i < gd_engfuncs.pfnGetNbEdicts(); i++)
	{
		edict_t* pedict = gd_engfuncs.pfnGetEdictByIndex(i);
		if(!pedict || pedict->free)
			continue;

		if(!qstrcmp(gd_engfuncs.pfnGetString(pedict->fields.classname), pstrClassname)
			&& !qstrcmp(gd_engfuncs.pfnGetString(pedict->fields.globalname), pstrGlobalName))
			return pedict;
	}

	return nullptr;
}

//=============================================
// @brief
//
//=============================================
bool GetTransitioningEntities( const byte* pPVS, const Vector* pTransitionMins, const Vector* pTransitionMaxs, const Char* pstrLandmarkName, const Vector& landmarkPosition, Int32 *pTransitionList, Uint32& numEntities, Uint32 maxEntities )
{
	// Make sure the local host is inside the PVS of this landmark
	CBaseEntity* pPlayer = Util::GetHostPlayer();
	if(!pPlayer || !pPlayer->IsPlayer())
	{
		gd_engfuncs.pfnCon_Printf("%s - Failed to get player.\n", __FUNCTION__);
		return false;
	}

	const edict_t* playeredict = pPlayer->GetEdict();
	if(!Common::CheckVisibility(playeredict->leafnums, pPVS))
	{
		gd_engfuncs.pfnCon_Printf("%s - Player is not inside the PVS of landmark '%s'.\n", __FUNCTION__, pstrLandmarkName);
		return false;
	}

	// Make sure it touches the trigger_transition
	if(pTransitionMins && pTransitionMaxs && Math::CheckMinsMaxs((*pTransitionMins), (*pTransitionMaxs), playeredict->state.absmin, playeredict->state.absmax))
	{
		gd_engfuncs.pfnCon_Printf("%s - Player is not inside trigger_transition entity '%s'.\n", __FUNCTION__, pstrLandmarkName);
		return false;
	}

	// Now go through all edicts and find which ones intersect
	Uint32 numentities = gd_engfuncs.pfnGetNbEdicts();
	for(Uint32 i = 1; i < numentities; i++)
	{
		edict_t* pedict = gd_engfuncs.pfnGetEdictByIndex(i);

		// Only transfer valid edicts
		if(Util::IsNullEntity(pedict))
			continue;

		// Only transition if flagged to be able to
		CBaseEntity* pentity = CBaseEntity::GetClass(pedict);
		if(!(pentity->GetEntityFlags() & FL_ENTITY_TRANSITION) && (!qstrlen(pentity->GetGlobalName()) || pentity->IsDormant()))
			continue;

		// Check VIS for this object
		if(!Common::CheckVisibility(pedict->leafnums, pPVS))
			continue;

		// If we have a trigger_transition, make sure it intersects
		if(pTransitionMins && pTransitionMaxs)
		{
			if(Math::CheckMinsMaxs((*pTransitionMins), (*pTransitionMaxs), pedict->state.absmin, pedict->state.absmax))
				continue;
		}

		if(numEntities == maxEntities)
		{
			gd_engfuncs.pfnCon_Printf("%s - Exceeded maxEntities.\n", __FUNCTION__);
			break;
		}

#ifdef _DEBUG
		// Print debug info
		CString msg;
		msg << "Entity " << gd_engfuncs.pfnGetString(pedict->fields.classname);
		if(pedict->fields.targetname != NO_STRING_VALUE)
			msg << "(" << gd_engfuncs.pfnGetString(pedict->fields.targetname) << ")";
		msg << " marked for transition.\n";

		gd_engfuncs.pfnCon_DPrintf(msg.c_str());
#endif //DEBUG

		// Add to the list
		pTransitionList[numEntities] = pedict->entindex;
		numEntities++;
	}

	// Check for errors
	if(!numEntities)
	{
		gd_engfuncs.pfnCon_Printf("%s - No entities in transition list.\n");
		return false;
	}

	// Make sure player made it onto the list
	Uint32 i = 0;
	for(; i < numEntities; i++)
	{
		if(pTransitionList[i] == playeredict->entindex)
			break;
	}

	if(i == numEntities)
	{
		gd_engfuncs.pfnCon_Printf("%s - Host player is not in transition list.\n");
		return false;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void AdjustEntityPositions( edict_t* pedict, Vector prevmins )
{
	if(!pedict)
		return;

	// NOTE: This is the way HL1 accounts for bmodels with no origin
	// brushes, and it's just so fucking evil. I wish I could come up
	// with a better solution
	// Adjust by difference in mins
	Vector posAdj = pedict->state.mins - prevmins;

	// Get nb of fields
	Uint32 nbFields = sizeof(g_edictStateFields)/sizeof(entity_data_desc_t);

	for(Uint32 i = 0; i < nbFields; i++)
	{
		entity_data_desc_t& field = g_edictStateFields[i];

		// Do not modify global fields of transitioning entities
		if(field.flags & EFIELD_GLOBAL)
			continue;

		for(Uint32 j = 0; j < field.size; j++)
		{
			byte* pdata = ((byte*)&pedict->state) + field.offset;

			switch(field.type)
			{
			case EFIELD_COORD:
				{
					Vector& vecValue = *(reinterpret_cast<Vector*>(pdata) + j);
					vecValue -= posAdj;
				}
				break;
			default:
				break;
			}
		}
	}

	// Fix for class data also
	CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
	if(pEntity)
		pEntity->AdjustEntityPositions(posAdj);

	// Re-link edict
	gd_engfuncs.pfnSetOrigin(pedict, pedict->state.origin); 
}