/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "scriptedsequence.h"
#include "ai_basenpc.h"
#include "activity.h"

// Script break conditions
const Uint64 CScriptedSequence::SCRIPT_BREAK_CONDITIONS = (AI_COND_LIGHT_DAMAGE|AI_COND_HEAVY_DAMAGE|AI_COND_SEE_ENEMY|AI_COND_HEAR_SOUND|AI_COND_SEE_HOSTILE_NPC);

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(scripted_sequence, CScriptedSequence);

//=============================================
// @brief
//
//=============================================
CScriptedSequence::CScriptedSequence( edict_t* pedict ):
	CDelayEntity(pedict),
	m_idleSequence(NO_STRING_VALUE),
	m_playSequence(NO_STRING_VALUE),
	m_loopSequence(NO_STRING_VALUE),
	m_exitSequence(NO_STRING_VALUE),
	m_npcName(NO_STRING_VALUE),
	m_lastSequence(NO_SEQUENCE_VALUE),
	m_moveToSetting(SCRIPT_MOVETO_NO),
	m_radius(0),
	m_repeatRange(0),
	m_customSoundMask(0),
	m_scriptDelay(0),
	m_waitForReTrigger(false),
	m_isInterruptable(false),
	m_playLoop(0)
{
}

//=============================================
// @brief
//
//=============================================
CScriptedSequence::~CScriptedSequence( void )
{
}

//=============================================
// @brief
//
//=============================================
void CScriptedSequence::DeclareSaveFields( void )
{
	// Call base class to do it first
	CDelayEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CScriptedSequence, m_idleSequence, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CScriptedSequence, m_playSequence, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CScriptedSequence, m_loopSequence, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CScriptedSequence, m_exitSequence, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CScriptedSequence, m_npcName, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CScriptedSequence, m_moveToSetting, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CScriptedSequence, m_radius, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CScriptedSequence, m_repeatRange, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CScriptedSequence, m_scriptDelay, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CScriptedSequence, m_waitForReTrigger, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CScriptedSequence, m_isInterruptable, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CScriptedSequence, m_playLoop, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CScriptedSequence, m_targetEntity, EFIELD_EHANDLE));
	DeclareSaveField(DEFINE_DATA_FIELD(CScriptedSequence, m_customSoundMask, EFIELD_INT32));
}

//=============================================
// @brief
//
//=============================================
bool CScriptedSequence::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "m_iszIdle"))
	{
		m_idleSequence = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "m_iszPlay"))
	{
		m_playSequence = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "m_iszLoop"))
	{
		m_loopSequence = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "m_iszExit"))
	{
		m_exitSequence = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "m_iszEntity"))
	{
		// Legacy mod support
		if(!qstrncmp(kv.value, "monster_", 8))
		{
			CString tmp(kv.value);
			tmp.erase(0, 8);
			tmp.insert(0, "npc_");
			m_npcName = gd_engfuncs.pfnAllocString(tmp.c_str());
		}
		else
		{
			m_npcName = gd_engfuncs.pfnAllocString(kv.value);
		}
		return true;
	}
	else if(!qstrcmp(kv.keyname, "m_fMoveTo"))
	{
		m_moveToSetting = SDL_atoi(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "m_flRepeat"))
	{
		m_repeatRange = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "m_flRadius"))
	{
		m_radius = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "combatsounds"))
	{
		Int32 value = SDL_atoi(kv.value);
		if(value >= 1)
			m_customSoundMask |= AI_SOUND_COMBAT;

		return true;
	}
	else if(!qstrcmp(kv.keyname, "worldsounds"))
	{
		Int32 value = SDL_atoi(kv.value);
		if(value >= 1)
			m_customSoundMask |= AI_SOUND_WORLD;

		return true;
	}
	else if(!qstrcmp(kv.keyname, "playersounds"))
	{
		Int32 value = SDL_atoi(kv.value);
		if(value >= 1)
			m_customSoundMask |= AI_SOUND_PLAYER;

		return true;
	}
	else if(!qstrcmp(kv.keyname, "dangersounds("))
	{
		Int32 value = SDL_atoi(kv.value);
		if(value >= 1)
			m_customSoundMask |= AI_SOUND_DANGER;

		return true;
	}
	else
		return CDelayEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CScriptedSequence::Spawn( void )
{
	if(!CDelayEntity::Spawn())
		return false;

	m_pState->solid = SOLID_NOT;
	m_pState->movetype = MOVETYPE_NONE;
	m_pState->effects |= EF_NODRAW;
	m_pState->flags |= FL_INITIALIZE;

	if((!HasTargetName() || m_idleSequence != NO_STRING_VALUE)
		&& !HasSpawnFlag(FL_TRIGGER_IDLE_FIRST))
	{
		SetThink(&CScriptedSequence::ScriptedThink);
		m_pState->nextthink = g_pGameVars->time + 1.0;

		// Play indefinitely
		if(HasTargetName())
			m_waitForReTrigger = true;
	}

	// Set interruptable flag
	m_isInterruptable = HasSpawnFlag(FL_NO_INTERRUPTIONS) ? false : true;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CScriptedSequence::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	if(HasSpawnFlag(FL_TRIGGER_IDLE_FIRST))
	{
		SetThink(&CScriptedSequence::ScriptedThink);
		m_pState->nextthink = g_pGameVars->time + 0.1;
		RemoveSpawnFlag(FL_TRIGGER_IDLE_FIRST);
		m_waitForReTrigger = true;
		return;
	}

	if(!m_targetEntity)
	{
		SetThink(&CScriptedSequence::ScriptedThink);
		m_pState->nextthink = g_pGameVars->time;
		return;
	}

	if(m_targetEntity->GetScriptState() == AI_SCRIPT_PLAYING)
	{
		// We need to handle if play and loop sequences match
		bool playAndLoopMatch = PlayAndLoopMatch();
		if(m_loopSequence != NO_STRING_VALUE 
			&& (m_playLoop != SCRIPT_LOOP_INACTIVE || playAndLoopMatch)
			&& (m_playLoop == SCRIPT_LOOP_PLAYING_LOOP || playAndLoopMatch))
		{
			if(m_exitSequence != NO_STRING_VALUE)
			{
				StartSequence(m_targetEntity, gd_engfuncs.pfnGetString(m_exitSequence), true);
				m_playLoop = SCRIPT_LOOP_PLAYING_EXIT;
			}
			else
			{
				m_playLoop = SCRIPT_LOOP_PLAYING_EXIT;
				SetSequenceDone(m_targetEntity);
			}
		}
	}
	else
	{
		// Start immediately after this
		m_waitForReTrigger = false;
	}
}

//=============================================
// @brief
//
//=============================================
void CScriptedSequence::InitEntity( void )
{
	// Pointer to entity we'll use
	CBaseEntity* pEntity = nullptr;
	
	// Find the edict first with the targetname
	edict_t* pedict = nullptr;
	while(true)
	{
		pedict = Util::FindEntityByTargetName(pedict, gd_engfuncs.pfnGetString(m_npcName));
		if(!pedict)
			break;

		if(Util::IsNullEntity(pedict))
			continue;

		pEntity = CBaseEntity::GetClass(pedict);
		if(pEntity->IsNPC())
			break;
	}

	// If not, look by classname
	if(!pEntity)
	{
		pedict = nullptr;
		while(true)
		{
			pedict = Util::FindEntityByClassname(pedict, gd_engfuncs.pfnGetString(m_npcName));
			if(!pedict)
				break;

			if(Util::IsNullEntity(pedict))
				continue;

			pEntity = CBaseEntity::GetClass(pedict);
			if(pEntity->IsNPC())
				break;
		}
	}

	if(!pEntity)
		return;

	if(m_exitSequence != NO_STRING_VALUE)
	{
		const Char* pstrSeqName = gd_engfuncs.pfnGetString(m_exitSequence);
		if(pEntity->IsSequenceLooped(pstrSeqName))
			Util::EntityConDPrintf(m_pEdict, "Warning - Sequence '%s' specified for loop exit sequence is looped, this can cause animation glitches.\n", pstrSeqName);
	}
}

//=============================================
// @brief
//
//=============================================
void CScriptedSequence::SetLoopState( script_loop_t loopstate )
{
	m_playLoop = loopstate;
}

//=============================================
// @brief
//
//=============================================
script_loop_t CScriptedSequence::GetLoopState( void ) const
{
	return (script_loop_t)m_playLoop;
}

//=============================================
// @brief
//
//=============================================
bool CScriptedSequence::StartSequence( CBaseEntity* pNPC, const Char* pstrSequenceName, bool completeOnEmpty )
{
	// If sequence is empty, then finish the scripted_sequence
	if(!qstrlen(pstrSequenceName) && completeOnEmpty)
	{
		SetSequenceDone(pNPC);
		return false;
	}

	// For blending sequences
	pNPC->SetLastActivityTime(g_pGameVars->time);
	if(!pNPC->SetSequence(pstrSequenceName))
		pNPC->SetSequence(0);

	// Make sure activity code doesn't interfere
	pNPC->SetCurrentActivity((activity_t)pNPC->GetIdealActivity());

	Int32 npcSequence = pNPC->GetSequence();
	if(m_lastSequence != npcSequence)
	{
		pNPC->SetFrame(0);
		pNPC->ResetSequenceInfo();
		m_lastSequence = npcSequence;
	}

	for(Uint32 i = 0; i < pNPC->GetNbBlenders(); i++)
		pNPC->SetBlending(i, 0);

	return true;
}

//=============================================
// @brief
//
//=============================================
void CScriptedSequence::ScriptEntityCancel( CScriptedSequence* pScripted )
{
	CBaseEntity* pScriptNPC = pScripted->GetTargetEntity();
	if(!pScriptNPC)
		return;

	if(pScriptNPC->GetNPCState() != NPC_STATE_SCRIPT)
		return;

	pScriptNPC->SetScriptState(AI_SCRIPT_STATE_CLEANUP);
	pScriptNPC->CleanupScriptedSequence();
}

//=============================================
// @brief
//
//=============================================
void CScriptedSequence::CancelScript( void )
{
	gd_engfuncs.pfnCon_VPrintf("%s - Cancelling script '%s.\n", __FUNCTION__, GetTargetName());

	if(!HasTargetName())
	{
		ScriptEntityCancel(this);
		return;
	}
	
	edict_t* pedict = nullptr;
	while(true)
	{
		pedict = Util::FindEntityByTargetName(pedict, GetTargetName());
		if(!pedict)
			break;

		if(Util::IsNullEntity(pedict))
			continue;

		CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
		if(!pEntity->IsScriptedSequenceEntity())
			continue;

		ScriptEntityCancel(reinterpret_cast<CScriptedSequence*>(pEntity));
	}
}

//=============================================
// @brief
//
//=============================================
Int32 CScriptedSequence::GetMoveToSetting( void ) const
{
	return m_moveToSetting;
}

//=============================================
// @brief
//
//=============================================
Int32 CScriptedSequence::GetScriptDelay( void ) const
{
	return m_scriptDelay;
}

//=============================================
// @brief
//
//=============================================
bool CScriptedSequence::IsWaitingToBeTriggered( void ) const
{
	return m_waitForReTrigger;
}

//=============================================
// @brief
//
//=============================================
void CScriptedSequence::SetSequenceDone( CBaseEntity* pNPC )
{
	// If we're not exiting a loop but we have a loop sequence, 
	// then play the loop indefinitely
	if(m_loopSequence != NO_STRING_VALUE && m_playLoop != SCRIPT_LOOP_PLAYING_EXIT)
	{
		StartSequence(pNPC, gd_engfuncs.pfnGetString(m_loopSequence), false);
		// Set that we're playing a loop animation
		m_playLoop = SCRIPT_LOOP_PLAYING_LOOP;
		return;
	}

	// Clean up the NPC
	pNPC->CleanupScriptedSequence();
	FixScriptNPCSchedule(pNPC);

	UseTargets(nullptr, USE_TOGGLE, 0);
	m_playLoop = SCRIPT_LOOP_INACTIVE;

	// Remove if not repeatable
	if(!HasSpawnFlag(FL_REPEATABLE))
	{
		SetThink(&CBaseEntity::RemoveThink);
		m_pState->nextthink = g_pGameVars->time + 0.1;
	}

	m_lastSequence = NO_SEQUENCE_VALUE;
}

//=============================================
// @brief
//
//=============================================
void CScriptedSequence::FixScriptNPCSchedule( CBaseEntity* pNPC )
{
	// if NPC isn't dead, then set him to idle
	if(pNPC->GetIdealNPCState() != NPC_STATE_DEAD)
		pNPC->SetIdealNPCState(NPC_STATE_IDLE);

	pNPC->ClearSchedule();
}

//=============================================
// @brief
//
//=============================================
void CScriptedSequence::ScriptedThink( void )
{
	if(FindNPC())
	{
		PosessEntity();
		Util::EntityConDPrintf(m_pEdict, "Script is using the npc '%s'.\n", gd_engfuncs.pfnGetString(m_npcName));

		ClearThinkFunctions();
		return;
	}

	// Otherwise cancel the script
	CancelScript();
	gd_engfuncs.pfnCon_VPrintf("%s - Script '%s' can't find npc '%s'.\n", __FUNCTION__, gd_engfuncs.pfnGetString(m_pFields->targetname), gd_engfuncs.pfnGetString(m_npcName));

	// Try again if permitted
	if(!HasSpawnFlag(FL_DONT_RETRY))
	{
		SetThink(&CScriptedSequence::ScriptedThink);
		m_pState->nextthink = g_pGameVars->time + 1.0f;
	}
}

//=============================================
// @brief
//
//=============================================
void CScriptedSequence::PosessEntity( void )
{
	if(!m_targetEntity)
		return;

	m_targetEntity->SetScriptEntity(this);
	m_targetEntity->SetGoalEntity(this);
	m_targetEntity->SetTargetEntity(this);

	switch(m_moveToSetting)
	{
	case SCRIPT_MOVETO_NO:
		{
			m_targetEntity->SetScriptState(AI_SCRIPT_WAIT);
		}
		break;
	case SCRIPT_MOVETO_WALK:
	case SCRIPT_MOVETO_WALK_NO_TURN:
		{
			m_targetEntity->SetScriptState(AI_SCRIPT_WALK_TO_MARK);
			DelayStart(true);
		}
		break;
	case SCRIPT_MOVETO_RUN:
	case SCRIPT_MOVETO_RUN_NO_TURN:
		{
			m_targetEntity->SetScriptState(AI_SCRIPT_RUN_TO_MARK);
			DelayStart(true);
		}
		break;
	case SCRIPT_MOVETO_INSTANTENOUS:
		{
			// Move the entity over to this position
			m_targetEntity->SetOrigin(m_pState->origin);
			m_targetEntity->SetVelocity(ZERO_VECTOR);
			m_targetEntity->SetAngularVelocity(ZERO_VECTOR);

			if(HasSpawnFlag(FL_NO_SCRIPT_MOVEMENT))
				m_targetEntity->SetEffectFlag(EF_NOINTERP);

			Vector entityAngles = m_targetEntity->GetAngles();
			entityAngles[YAW] = m_pState->angles[YAW];

			m_targetEntity->SetAngles(entityAngles);
			m_targetEntity->SetIdealYaw(m_pState->angles[YAW]);

			m_targetEntity->SetScriptState(AI_SCRIPT_WAIT);

			// Wait until triggered again
			m_waitForReTrigger = true;

			if(!m_targetEntity->HasSpawnFlag(CBaseNPC::FL_NPC_DONT_FALL)
				&& !HasSpawnFlag(FL_NO_SCRIPT_MOVEMENT))
			{
				m_targetEntity->RemoveFlags(FL_ONGROUND);
				m_targetEntity->GroundEntityNudge(true);
			}
		}
		break;
	case SCRIPT_MOVETO_TURN_TO_FACE:
	case SCRIPT_MOVETO_TURN_TO_PLAYER:
		// These dont do anything
	default:
		break;
	}

	// Set the ideal NPC state
	m_targetEntity->SetIdealNPCState(NPC_STATE_SCRIPT);

	// Play idle animation immediately if we don't need to be triggered to idle
	if(!HasSpawnFlag(FL_TRIGGER_IDLE_FIRST) && m_idleSequence != NO_STRING_VALUE)
	{
		StartSequence(m_targetEntity, gd_engfuncs.pfnGetString(m_idleSequence), false);

		// If the sequence names match, freeze the animation on the npc
		if(!qstrcmp(gd_engfuncs.pfnGetString(m_idleSequence), gd_engfuncs.pfnGetString(m_playSequence)))
			m_targetEntity->SetFramerate(0.0f);
	}
}

//=============================================
// @brief
//
//=============================================
bool CScriptedSequence::FindNPC( void )
{
	if(!HasSpawnFlag(FL_SEARCH_RADIUS))
	{
		edict_t* pedict = nullptr;
		while(true)
		{
			pedict = Util::FindEntityByTargetName(pedict, gd_engfuncs.pfnGetString(m_npcName));
			if(!pedict)
				break;

			CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);

			if(pEntity->IsNPC())
			{
				// check if other script is playing, and if it is, interrupt it
				CScriptedSequence* pPlayingScript = pEntity->GetScriptedSequence();
				if(pPlayingScript && pPlayingScript->GetLoopState() != SCRIPT_LOOP_INACTIVE)
				{
					pPlayingScript->SetLoopState(SCRIPT_LOOP_PLAYING_EXIT);
					pPlayingScript->SetSequenceDone(pEntity);
				}

				// If the NPC can play us, then set it as the target
				if(pEntity->CanPlaySequence(CanOverrideState(), SCRIPT_INTERRUPT_BY_NAME))
				{
					m_targetEntity = pEntity;
					return true;
				}
				else
				{
					gd_engfuncs.pfnCon_DPrintf("%s - Found npc '%s', but cannot play.\n", __FUNCTION__, pEntity->GetTargetName());
				}
			}
		}

		if(!m_targetEntity)
		{
			Vector mins, maxs;
			for(Uint32 i = 0; i < 3; i++)
			{
				mins[i] = m_pState->origin[i] - m_radius;
				maxs[i] = m_pState->origin[i] + m_radius;
			}

			pedict = nullptr;
			while(true)
			{
				pedict = Util::FindEntityInBBox(pedict, mins, maxs);
				if(!pedict)
					break;

				CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
				if(pEntity->IsNPC() && !qstrcmp(pEntity->GetClassName(), gd_engfuncs.pfnGetString(m_npcName)))
				{
					if(pEntity->CanPlaySequence(CanOverrideState(), SCRIPT_INTERRUPT_IDLE))
					{
						m_targetEntity = pEntity;
						return true;
					}
				}
			}
		}
	}
	else
	{
		Vector mins, maxs;
		for(Uint32 i = 0; i < 3; i++)
		{
			mins[i] = m_pState->origin[i] - m_radius;
			maxs[i] = m_pState->origin[i] + m_radius;
		}

		edict_t* pedict = nullptr;
		while(true)
		{
			pedict = Util::FindEntityInBBox(pedict, mins, maxs);
			if(!pedict)
				break;

			CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
			if(pEntity->IsNPC() && !qstrcmp(pEntity->GetTargetName(), gd_engfuncs.pfnGetString(m_npcName)))
			{
				if(pEntity->CanPlaySequence(CanOverrideState(), SCRIPT_INTERRUPT_BY_NAME))
				{
					m_targetEntity = pEntity;
					return true;
				}
			}
		}
	}

	// Reset this always
	m_targetEntity.reset();

	return false;
}

//=============================================
// @brief
//
//=============================================
void CScriptedSequence::DelayStart( bool state )
{
	if(state)
	{
		m_scriptDelay++;
		return;
	}
	
	m_scriptDelay--;
	if(m_scriptDelay <= 0)
		m_waitForReTrigger = false;
}

//=============================================
// @brief
//
//=============================================
bool CScriptedSequence::CanOverrideState( void ) const
{
	return HasSpawnFlag(FL_OVERRIDE_STATE) ? true : false;
}

//=============================================
// @brief
//
//=============================================
void CScriptedSequence::SetAllowInterrupt( bool allowInterrupt )
{
	if(!HasSpawnFlag(FL_NO_INTERRUPTIONS))
		m_isInterruptable = allowInterrupt;
}

//=============================================
// @brief
//
//=============================================
bool CScriptedSequence::CanInterrupt( void )
{
	if(!m_isInterruptable)
		return false;
	else if(m_targetEntity && m_targetEntity->GetDeadState() == DEADSTATE_NONE)
		return true;
	else
		return false;
}

//=============================================
// @brief
//
//=============================================
bool CScriptedSequence::PlayAndLoopMatch( void )
{
	if(m_loopSequence == NO_STRING_VALUE
		|| m_playSequence == NO_STRING_VALUE)
		return false;
	else
		return (!qstrcmp(gd_engfuncs.pfnGetString(m_loopSequence), gd_engfuncs.pfnGetString(m_playSequence))) ? true : false;
}

//=============================================
// @brief
//
//=============================================
bool CScriptedSequence::HasPlaySequence( void ) const
{
	return (m_playSequence == NO_STRING_VALUE) ? false : true;
}

//=============================================
// @brief
//
//=============================================
const Char* CScriptedSequence::GetPlaySequenceName( void )
{
	if(m_playSequence == NO_STRING_VALUE)
		return nullptr;
	else
		return gd_engfuncs.pfnGetString(m_playSequence);
}

//=============================================
// @brief
//
//=============================================
bool CScriptedSequence::HasIdleAnimation( void ) const
{
	return (m_idleSequence == NO_STRING_VALUE) ? false : true;
}

//=============================================
// @brief
//
//=============================================
const Char* CScriptedSequence::GetIdleSequenceName( void )
{
	if(m_idleSequence == NO_STRING_VALUE)
		return nullptr;
	else
		return gd_engfuncs.pfnGetString(m_idleSequence);
}

//=============================================
// @brief
//
//=============================================
const Char* CScriptedSequence::GetLoopSequenceName( void )
{
	if(m_loopSequence == NO_STRING_VALUE)
		return nullptr;
	else
		return gd_engfuncs.pfnGetString(m_loopSequence);
}

//=============================================
// @brief
//
//=============================================
const Char* CScriptedSequence::GetExitSequenceName( void )
{
	if(m_exitSequence == NO_STRING_VALUE)
		return nullptr;
	else
		return gd_engfuncs.pfnGetString(m_exitSequence);
}

//=============================================
// @brief
//
//=============================================
script_loop_t CScriptedSequence::GetLoopState( void )
{
	return (script_loop_t)m_playLoop;
}

//=============================================
// @brief
//
//=============================================
Uint64 CScriptedSequence::GetInterruptSoundMask( void )
{
	Uint64 interruptSoundMask = AI_SOUND_NONE;
	if(CanInterrupt() && HasSpawnFlag(FL_SOUNDS_CAN_INTERRUPT))
	{
		if(m_customSoundMask)
			interruptSoundMask |= m_customSoundMask;
		else
			interruptSoundMask |= (AI_SOUND_COMBAT|AI_SOUND_DANGER);
	}

	return interruptSoundMask;
}

//=============================================
// @brief
//
//=============================================
Uint64 CScriptedSequence::GetIgnoreConditions( void )
{
	Uint64 ingoreConditionMask = SCRIPT_BREAK_CONDITIONS;
	ingoreConditionMask &= ~GetScriptBreakingAIConditions();
	return ingoreConditionMask;
}

//=============================================
// @brief
//
//=============================================
Uint64 CScriptedSequence::GetScriptBreakingAIConditions( void )
{
	Uint64 conditionMask = AI_COND_NONE;
	if(CanInterrupt())
	{
		// Remove these conditions
		conditionMask |= (AI_COND_LIGHT_DAMAGE|AI_COND_HEAVY_DAMAGE);

		// Set "sounds can interrupt" conditions
		if(HasSpawnFlag(FL_SOUNDS_CAN_INTERRUPT))
			conditionMask |= AI_COND_HEAR_SOUND;

		// Set "sounds can interrupt" conditions
		if(HasSpawnFlag(FL_ENEMIES_CAN_INTERRUPT))
			conditionMask |= (AI_COND_SEE_ENEMY|AI_COND_SEE_HOSTILE_NPC);
	}

	return conditionMask;
}

//=============================================
// @brief
//
//=============================================
void CScriptedSequence::ClearTargetEntity( void )
{
	if(!m_targetEntity)
		return;

	m_targetEntity.reset();
}