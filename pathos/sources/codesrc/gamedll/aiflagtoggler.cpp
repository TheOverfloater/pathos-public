/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "aiflagtoggler.h"
#include "ai_basenpc.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(ai_flagtoggler, CAIFlagToggler);

//=============================================
// @brief
//
//=============================================
CAIFlagToggler::CAIFlagToggler( edict_t* pedict ):
	CPointEntity(pedict),
	m_mode(FLAGTOGGLER_DISABLE),
	m_flag(FLAG_WAITTILLSEEN)
{
}

//=============================================
// @brief
//
//=============================================
CAIFlagToggler::~CAIFlagToggler( void )
{
}

//=============================================
// @brief
//
//=============================================
void CAIFlagToggler::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CAIFlagToggler, m_mode, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CAIFlagToggler, m_flag, EFIELD_INT32));
}

//=============================================
// @brief
//
//=============================================
bool CAIFlagToggler::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "mode"))
	{
		m_mode = SDL_atoi(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "flag"))
	{
		m_flag = SDL_atoi(kv.value);
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CAIFlagToggler::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	if(m_pFields->target == NO_STRING_VALUE)
	{
		Util::WarnEmptyEntity(m_pEdict);
		return false;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CAIFlagToggler::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	const Char* pstrTargetEntityName = gd_engfuncs.pfnGetString(m_pFields->target);

	edict_t* pTargetEdict = Util::FindEntityByTargetName(nullptr, pstrTargetEntityName);
	if(Util::IsNullEntity(pTargetEdict))
	{
		Util::EntityConPrintf(m_pEdict, "Could not find target '%s'.\n", pstrTargetEntityName);
		return;
	}

	CBaseEntity* pTargetEntity = CBaseEntity::GetClass(pTargetEdict);
	if(!pTargetEntity->IsNPC())
	{
		Util::EntityConPrintf(m_pEdict, "Entity '%s' was not an NPC.\n", pstrTargetEntityName);
		return;
	}

	Uint64 flagToSet = 0;
	switch(m_flag)
	{
		case FLAG_WAITTILLSEEN: 
			flagToSet = CBaseNPC::FL_NPC_WAIT_TILL_SEEN; 
			break;
		case FLAG_GAG: 
			flagToSet = CBaseNPC::FL_NPC_GAG; 
			break;
		case FLAG_MONSTERCLIP: 
			flagToSet = CBaseNPC::FL_NPC_USE_NPC_CLIP; 
			break;
		case FLAG_REMEMBER: 
			flagToSet = CBaseNPC::FL_NPC_DONT_FORGET_PLAYER; 
			break;
		case FLAG_PRISONER: 
			flagToSet = CBaseNPC::FL_NPC_PRISONER; 
			break;
		case FLAG_WAITFORSCRIPT: 
			flagToSet = CBaseNPC::FL_NPC_WAIT_FOR_SCRIPT; 
			break;
		case FLAG_PREDISASTER: 
			flagToSet = CBaseNPC::FL_NPC_IDLE; 
			break;
		case FLAG_FADECORPSE: 
			flagToSet = CBaseNPC::FL_NPC_FADE_CORPSE; 
			break;
		case FLAG_IMMORTAL:
			flagToSet = CBaseNPC::FL_NPC_IMMORTAL; 
			break;
		case FLAG_DONTFALL: 
			flagToSet = CBaseNPC::FL_NPC_DONT_FALL; 
			break;
		case FLAG_NOWEAPONDROP:
			if(m_mode == FLAGTOGGLER_ENABLE && !pTargetEntity->CanDropWeapons())
				pTargetEntity->SetCanDropWeapons(true);
			else if(m_mode == FLAGTOGGLER_DISABLE && pTargetEntity->CanDropWeapons())
				pTargetEntity->SetCanDropWeapons(false);
			break;
		case FLAG_NOPUSHING: 
			flagToSet = CBaseNPC::FL_NPC_NO_PUSHING; 
			break;
		default:
			Util::EntityConPrintf(m_pEdict, "Invalid flag set: %d\n", m_flag);
			return;
	}

	if(flagToSet)
	{
		if(flagToSet & CBaseNPC::FL_NPC_USE_NPC_CLIP)
		{
			if( m_mode == FLAGTOGGLER_ENABLE )
				pTargetEntity->RemoveFlags(FL_NPC_CLIP);
			else if( m_mode == FLAGTOGGLER_DISABLE )
				pTargetEntity->SetFlags(FL_NPC_CLIP);
			else
				Util::EntityConPrintf(m_pEdict, "Invalid mode set: %d\n", m_mode);
		}
		else
		{
			if( m_mode == FLAGTOGGLER_ENABLE )
				pTargetEntity->SetSpawnFlag(flagToSet);
			else if( m_mode == FLAGTOGGLER_DISABLE )
				pTargetEntity->RemoveSpawnFlag(flagToSet);
			else
				Util::EntityConPrintf(m_pEdict, "Invalid mode set: %d\n", m_mode);
		}
	}
}