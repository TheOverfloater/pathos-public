/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "triggerautosave.h"
#include "player.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(trigger_autosave, CTriggerAutoSave);

//=============================================
// @brief
//
//=============================================
CTriggerAutoSave::CTriggerAutoSave( edict_t* pedict ):
	CTriggerEntity(pedict),
	m_triggerState(AS_STATE_DEFAULT)
{
}

//=============================================
// @brief
//
//=============================================
CTriggerAutoSave::~CTriggerAutoSave( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CTriggerAutoSave::Spawn( void )
{
	if(!CTriggerEntity::Spawn())
		return false;

	// Set touch fn
	if(!HasSpawnFlag(FL_USE_ONLY))
		SetTouch(&CTriggerAutoSave::SaveTouch);

	return true;
}

//=============================================
// @brief
//
//=============================================
void CTriggerAutoSave::DeclareSaveFields( void )
{
	// Call base class to do it first
	CTriggerEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerAutoSave, m_triggerState, EFIELD_INT32));
}

//=============================================
// @brief
//
//=============================================
void CTriggerAutoSave::SaveTouch( CBaseEntity* pOther )
{
	// Not in USE ONLY
	if(HasSpawnFlag(FL_USE_ONLY))
		return;

	// Check for master
	if(!IsMasterTriggered())
		return;
	
	// Only player
	if(!pOther->IsPlayer())
		return;

	// Get player ptr
	CPlayerEntity* pPlayer = reinterpret_cast<CPlayerEntity*>(pOther);
	PerformSave(pPlayer);
}

//=============================================
// @brief
//
//=============================================
void CTriggerAutoSave::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	// This only works if we are use only
	if(!HasSpawnFlag(FL_USE_ONLY))
		return;

	// Do the level change
	CBaseEntity* pPlayer;
	if(pActivator->IsPlayer())
		pPlayer = pActivator;
	else
		pPlayer = Util::GetHostPlayer();

	PerformSave(pPlayer);
}

//=============================================
// @brief
//
//=============================================
void CTriggerAutoSave::PerformSave( CBaseEntity* pPlayer )
{
	// Handle night stage specially
	if(HasSpawnFlag(FL_ALL_DAY_STAGES))
	{
		switch(m_triggerState)
		{
		case AS_STATE_NIGHT:
			{
				if(pPlayer->GetDayStage() < DAYSTAGE_NIGHTSTAGE)
					return;
			}
			break;
		case AS_STATE_DAYLIGHT_RETURN:
			{
				if(pPlayer->GetDayStage() != DAYSTAGE_DAYLIGHT_RETURN)
					return;
			}
			break;
		case AS_STATE_DEFAULT:
		case AS_STAGE_EXHAUSTED:
			break;
		}

		// Increment the stage
		m_triggerState++;

		// If we've exhausted all save slots, remove this entity
		if(m_triggerState == AS_STAGE_EXHAUSTED)
		{
			// Disable touch then save
			SetTouch(nullptr);
		}
	}
	else
	{
		// Disable touch then save
		SetTouch(nullptr);
	}

	CString cmd;
	cmd << "autosave";

	if(HasSpawnFlag(FL_PERSISTENT_SAVE))
		cmd << " persistent";

	cmd << "\n";

	// Perform save
	gd_engfuncs.pfnServerCommand(cmd.c_str());

	// Remove if possible
	if(!HasSpawnFlag(FL_ALL_DAY_STAGES) || m_triggerState == AS_STAGE_EXHAUSTED)
	{
		SetThink(&CBaseEntity::RemoveThink);
		m_pState->nextthink = g_pGameVars->time + 0.1;
	}
}