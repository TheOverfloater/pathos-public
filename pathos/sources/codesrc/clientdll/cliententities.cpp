/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "cliententities.h"
#include "entitymanager.h"
#include "studio.h"
#include "animevent.h"
#include "animevents.h"
#include "cl_entity.h"
#include "clientdll.h"
#include "snd_shared.h"
#include "ladder.h"
#include "viewmodel.h"

// Spark effect particle script
static const Char SPARK_EFFECT_SCRIPT_FILE[] = "spark_cluster.txt";

//=============================================
// @brief
//
//=============================================
void CL_AddEntities( void )
{
	// Add any client-side entities
	gEntityManager.Frame();
}

//=============================================
// @brief
//
//=============================================
void CL_ParseEntityList( void )
{
	gEntityManager.ParseEntities();
}

//=============================================
// @brief
//
//=============================================
void CL_GetClientEntityList( const struct entitydata_t*& pEntitiesPtr, Uint32& numEntities )
{
	const CArray<entitydata_t>& entityList = gEntityManager.GetEntityList();

	pEntitiesPtr = &entityList[0];
	numEntities = entityList.size();
}

//=============================================
// @brief
//
//=============================================
void CL_FreeEntityData( void )
{
	gEntityManager.FreeEntityData();
}

//=============================================
// @brief
//
//=============================================
void CL_AdjustEntityTimers( entity_state_t* pstate, Double jointime )
{
	if(!pstate)
		return;

	if(pstate->animtime)
		pstate->animtime -= jointime;

	if(pstate->ltime)
		pstate->ltime -= jointime;
}

//=============================================
// @brief
//
//=============================================
void CL_VBMEvent( const struct mstudioevent_t *pvbmevent, struct cl_entity_t *pentity )
{
	if(pvbmevent->event <= EVENT_CLIENT_START_INDEX)
		return;

	switch(pvbmevent->event)
	{
	case EVENT_CLIENTDLL_SPARK_A1:
		{
			cl_efxapi.pfnSpawnParticleSystem(pentity->getAttachment(0), Vector(0, 0, 0), PART_SCRIPT_CLUSTER, SPARK_EFFECT_SCRIPT_FILE, pentity->entindex, pentity->entindex, 0, NO_POSITION, 0);
		}
		break;
	case EVENT_CLIENTDLL_PLAYSOUND_A1:
		{
			cl_engfuncs.pfnPlayEntitySound(pentity->entindex, SND_CHAN_WEAPON, pvbmevent->options, VOL_NORM, ATTN_NORM, PITCH_NORM, SND_FL_NONE, 0);
		}
		break;
	case EVENT_CLIENTDLL_PLAYSOUND_A1_CHAN_ITEM:
		{
			cl_engfuncs.pfnPlayEntitySound(pentity->entindex, SND_CHAN_ITEM, pvbmevent->options, VOL_NORM, ATTN_NORM, PITCH_NORM, SND_FL_NONE, 0);
		}
		break;
	case EVENT_CLIENTDLL_PLAYSOUND_A1_PLAYER:
		{
			cl_entity_t* pplayer = cl_engfuncs.pfnGetLocalPlayer();
			if(!pplayer)
				return;

			cl_engfuncs.pfnPlayEntitySound(pplayer->entindex, SND_CHAN_BODY, pvbmevent->options, VOL_NORM, ATTN_NORM, PITCH_NORM, SND_FL_NONE, 0);
		}
		break;
	case EVENT_CLIENTDLL_MUZZLEFLASH_SIMPLE_A1:
		{
			EV_SimpleMuzzleFlash(pentity->getAttachment(0), pentity->curstate.angles, pentity->entindex, 0);
		}
		break;
	case EVENT_CLIENTDLL_MUZZLEFLASH_SIMPLE_A2:
		{
			EV_SimpleMuzzleFlash(pentity->getAttachment(1), pentity->curstate.angles, pentity->entindex, 1);
		}
		break;
	case EVENT_CLIENTDLL_MUZZLEFLASH_SIMPLE_A3:
		{
			EV_SimpleMuzzleFlash(pentity->getAttachment(2), pentity->curstate.angles, pentity->entindex, 2);
		}
		break;
	case EVENT_CLIENTDLL_MUZZLEFLASH_SIMPLE_A4:
		{
			EV_SimpleMuzzleFlash(pentity->getAttachment(3), pentity->curstate.angles, pentity->entindex, 3);
		}
		break;
	case EVENT_CLIENTDLL_MUZZLEFLASH_2A_A1:
		{
			EV_ViewModelMuzzleflash(pentity, pvbmevent, pentity->getAttachment(0), 0);
		}
		break;
	case EVENT_CLIENTDLL_EJECTBULLET_A1:
		{
			EV_EjectBullet(pentity->getAttachment(0), pvbmevent, pentity);
		}
		break;
	case EVENT_CLIENTDLL_EJECTBULLET_A2:
		{
			EV_EjectBullet(pentity->getAttachment(1), pvbmevent, pentity);
		}
		break;
	case EVENT_CLIENTDLL_EJECTBULLET_A3:
		{
			EV_EjectBullet(pentity->getAttachment(2), pvbmevent, pentity);
		}
		break;
	case EVENT_CLIENTDLL_EJECTBULLET_A4:
		{
			EV_EjectBullet(pentity->getAttachment(3), pvbmevent, pentity);
		}
		break;
	case EVENT_CLIENTDLL_MUZZLEFLASH_1A_A1:
		{
			EV_AngleForwardMuzzleFlash(0, pentity->getAttachment(0), pentity, pvbmevent);
		}
		break;
	case EVENT_CLIENTDLL_MUZZLEFLASH_1A_A2:
		{
			EV_AngleForwardMuzzleFlash(1, pentity->getAttachment(1), pentity, pvbmevent);
		}
		break;
	case EVENT_CLIENTDLL_CREATE_PARTICLE_SYSTEM_A1:
		{
			cl_efxapi.pfnSpawnParticleSystem(pentity->getAttachment(0), Vector(0, 0, 0), PART_SCRIPT_SYSTEM, pvbmevent->options, pentity->entindex, pentity->entindex, 0, NO_POSITION, PARTICLE_ATTACH_NONE);
		}
		break;
	case EVENT_CLIENTDLL_CREATE_PARTICLE_SYSTEM_A2:
		{
			cl_efxapi.pfnSpawnParticleSystem(pentity->getAttachment(1), Vector(0, 0, 0), PART_SCRIPT_SYSTEM, pvbmevent->options, pentity->entindex, pentity->entindex, 1, NO_POSITION, PARTICLE_ATTACH_NONE);
		}
		break;
	case EVENT_CLIENTDLL_CREATE_PARTICLE_SYSTEM_A3:
		{
			cl_efxapi.pfnSpawnParticleSystem(pentity->getAttachment(2), Vector(0, 0, 0), PART_SCRIPT_SYSTEM, pvbmevent->options, pentity->entindex, pentity->entindex, 2, NO_POSITION, PARTICLE_ATTACH_NONE);
		}
		break;
	case EVENT_CLIENTDLL_CREATE_PARTICLE_SYSTEM_A4:
		{
			cl_efxapi.pfnSpawnParticleSystem(pentity->getAttachment(3), Vector(0, 0, 0), PART_SCRIPT_SYSTEM, pvbmevent->options, pentity->entindex, pentity->entindex, 3, NO_POSITION, PARTICLE_ATTACH_NONE);
		}
		break;
	case EVENT_CLIENTDLL_CREATE_PARTICLE_CLUSTER_A1:
		{
			cl_efxapi.pfnSpawnParticleSystem(pentity->getAttachment(0), Vector(0, 0, 0), PART_SCRIPT_CLUSTER, pvbmevent->options, pentity->entindex, pentity->entindex, 0, NO_POSITION, PARTICLE_ATTACH_NONE);
		}
		break;
	case EVENT_CLIENTDLL_CREATE_PARTICLE_CLUSTER_A2:
		{
			cl_efxapi.pfnSpawnParticleSystem(pentity->getAttachment(1), Vector(0, 0, 0), PART_SCRIPT_CLUSTER, pvbmevent->options, pentity->entindex, pentity->entindex, 1, NO_POSITION, PARTICLE_ATTACH_NONE);
		}
		break;
	case EVENT_CLIENTDLL_CREATE_PARTICLE_CLUSTER_A3:
		{
			cl_efxapi.pfnSpawnParticleSystem(pentity->getAttachment(2), Vector(0, 0, 0), PART_SCRIPT_CLUSTER, pvbmevent->options, pentity->entindex, pentity->entindex, 2, NO_POSITION, PARTICLE_ATTACH_NONE);
		}
		break;
	case EVENT_CLIENTDLL_CREATE_PARTICLE_CLUSTER_A4:
		{
			cl_efxapi.pfnSpawnParticleSystem(pentity->getAttachment(3), Vector(0, 0, 0), PART_SCRIPT_CLUSTER, pvbmevent->options, pentity->entindex, pentity->entindex, 3, NO_POSITION, PARTICLE_ATTACH_NONE);
		}
		break;
	case EVENT_CLIENTDLL_LADDER_STEP_SOUND:
		{
			gLadder.PlayStepSound();
		}
		break;
	case EVENT_CLIENTDLL_NPC_STEP_SOUND:
		{
			EV_NPC_StepSound_Legacy(pentity, pvbmevent);
		}
		break;
	case EVENT_CLIENTDLL_VAPORTRAIL_SMOKE:
		{
			EV_WeaponVaporTrail_Smoke(pentity->getAttachment(0), pvbmevent, pentity);
		}
		break;
	case EVENT_CLIENTDLL_VAPORTRAIL_GLOW:
		{
			EV_WeaponVaporTrail_Glow(pentity->getAttachment(0), pvbmevent, pentity);
		}
		break;
	}
}