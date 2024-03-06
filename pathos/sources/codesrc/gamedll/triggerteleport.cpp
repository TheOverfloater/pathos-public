/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "triggerteleport.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(trigger_teleport, CTriggerTeleport);

//=============================================
// @brief
//
//=============================================
CTriggerTeleport::CTriggerTeleport( edict_t* pedict ):
	CTriggerEntity(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CTriggerTeleport::~CTriggerTeleport( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CTriggerTeleport::Spawn( void )
{
	if(!CTriggerEntity::Spawn())
		return false;

	if(HasSpawnFlag(FL_RELATIVE) && m_pFields->target == NO_STRING_VALUE)
	{
		Util::EntityConPrintf(m_pEdict, "Relative teleport with no landmark set.\n");
		return false;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CTriggerTeleport::CallTouch( CBaseEntity* pOther )
{
	// Allow changetargets to change destination
	if(m_pFields->target == NO_STRING_VALUE)
		return;

	// Only allow players and npcs
	if(HasSpawnFlag(FL_NO_CLIENTS) && pOther->IsPlayer() 
		|| !HasSpawnFlag(FL_ALLOW_NPCS) && pOther->IsNPC())
		return;

	// Only allow npcs and players
	if(!pOther->IsPlayer() && !pOther->IsNPC())
		return;

	// Check for master
	if(!IsMasterTriggered())
		return;

	const Char* pstrTarget = gd_engfuncs.pfnGetString(m_pFields->target);
	edict_t* pTargetEdict = Util::FindEntityByTargetName(nullptr, pstrTarget);
	if(Util::IsNullEntity(pTargetEdict))
	{
		Util::EntityConPrintf(m_pEdict, "Couldn't find target entity '%s'.\n", pstrTarget);
		return;
	}

	CBaseEntity* pTargetEntity = CBaseEntity::GetClass(pTargetEdict);
	if(!pTargetEntity)
		return;

	// Get target's origin
	Vector targetOrigin = pTargetEntity->GetOrigin();

	// Manage teleportation
	if(!HasSpawnFlag(FL_RELATIVE))
	{
		// Add player origin offset if needed
		if(pOther->IsPlayer())
			targetOrigin[2] += SDL_fabs(VEC_HULL_MIN[2]);

		// Offset player origin abit
		targetOrigin[2] += 4;

		// Remove onground flag
		pOther->RemoveFlags(FL_ONGROUND);
		pOther->SetOrigin(targetOrigin);
		pOther->DropToFloor();

		if(!HasSpawnFlag(FL_KEEP_ANGLES))
		{
			Vector targetAngles = pTargetEntity->GetAngles();
			pOther->SetAngles(targetAngles);
			
			// Set the view angles for players
			if(pOther->IsPlayer())
				pOther->SetViewAngles(targetAngles);
		}

		// Set these to zero
		pOther->SetFallingVelocity(0);
		pOther->SetVelocity(ZERO_VECTOR);
		pOther->SetPunchAngle(ZERO_VECTOR);
		pOther->SetPunchAmount(ZERO_VECTOR);
		pOther->SetWaterLevel(WATERLEVEL_NONE);
	}
	else
	{
		const Char* pstrLandmark = gd_engfuncs.pfnGetString(m_pFields->message);
		edict_t* pLandmarkEdict = Util::FindEntityByTargetName(nullptr, pstrLandmark);
		if(Util::IsNullEntity(pLandmarkEdict))
		{
			Util::EntityConPrintf(m_pEdict, "Couldn't find landmark entity '%s'.\n", pstrLandmark);
			return;
		}

		CBaseEntity* pLandmarkEntity = CBaseEntity::GetClass(pLandmarkEdict);
		if(!pLandmarkEntity)
			return;

		Vector offset = pOther->GetOrigin() - pLandmarkEntity->GetOrigin();
		Vector finalPosition = targetOrigin + offset;

		// Add 4 units to offset
		finalPosition[2] += 4;

		// Remove onground flag
		pOther->RemoveFlags(FL_ONGROUND);
		pOther->SetOrigin(finalPosition);
		pOther->DropToFloor();
	}

	// Trigger target if set
	if(m_pFields->netname != NO_STRING_VALUE)
	{
		const Char* pstrTriggerEntity = gd_engfuncs.pfnGetString(m_pFields->netname);
		edict_t* pTriggerEdict = Util::FindEntityByTargetName(nullptr, pstrTriggerEntity);
		if(Util::IsNullEntity(pTriggerEdict))
		{
			Util::EntityConPrintf(m_pEdict, "Couldn't find trigger entity '%s'.\n", pstrTriggerEntity);
			return;
		}

		CBaseEntity* pTriggerEntity = CBaseEntity::GetClass(pTriggerEdict);
		if(!pTriggerEntity)
			return;

		pTriggerEntity->CallUse(this, this, USE_TOGGLE, 1);
	}
}
