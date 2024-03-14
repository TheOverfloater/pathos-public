/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "functrain.h"
#include "pathcorner.h"

// Default train speed
const Float CFuncTrain::DEFAULT_SPEED = 100;
// Default train volume
const Float CFuncTrain::DEFAULT_VOLUME = 0.85;
// Train center correction offset(to match with HL)
const Vector CFuncTrain::TRAIN_CENTER_OFFSET = Vector(1, 1, 1);

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(func_train, CFuncTrain);

//=============================================
// @brief
//
//=============================================
CFuncTrain::CFuncTrain( edict_t* pedict ):
	CPlatTrainEntity(pedict),
	m_pCurrentTarget(nullptr),
	m_pPreviousTarget(nullptr),
	m_lastTargetName(NO_STRING_VALUE),
	m_currentPathCornerName(NO_STRING_VALUE),
	m_isSoundPlaying(false),
	m_skipSounds(false)
{
}

//=============================================
// @brief
//
//=============================================
CFuncTrain::~CFuncTrain( void )
{
}

//=============================================
// @brief
//
//=============================================
void CFuncTrain::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPlatTrainEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncTrain, m_pCurrentTarget, EFIELD_ENTPOINTER));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncTrain, m_pPreviousTarget, EFIELD_ENTPOINTER));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncTrain, m_lastTargetName, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncTrain, m_currentPathCornerName, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncTrain, m_isSoundPlaying, EFIELD_BOOLEAN));
}

//=============================================
// @brief
//
//=============================================
bool CFuncTrain::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "sounds"))
	{
		m_moveSound = SDL_atoi(kv.value);
		if(m_moveSound < 0 || m_moveSound > (Int32)NB_LEGACY_MOVE_SOUNDS)
		{
			Util::EntityConPrintf(m_pEdict, "Invalid value %d set for '%s'.\n", m_moveSound, kv.keyname);
			m_moveSound = 0;
		}
		return true;
	}
	else
		return CPlatTrainEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CFuncTrain::Spawn( void )
{
	if(!CPlatTrainEntity::Spawn())
		return false;

	if(!m_pState->speed)
		m_pState->speed = DEFAULT_SPEED;

	if(!m_pState->renderamt 
		&& (m_pState->rendermode & RENDERMODE_BITMASK) == RENDER_TRANSCOLOR)
	{
		m_pState->rendermode = RENDER_NORMAL;
		m_pState->effects |= EF_COLLISION;
	}

	SetSpawnProperties();

	if(HasSpawnFlag(FL_SOUND_USE_ORIGIN))
		m_pState->renderfx = RenderFx_SoundOrg;

	if(!TrainSetModel())
		return false;

	m_isSoundPlaying = false;

	if(!m_volume)
		m_volume = DEFAULT_VOLUME;

	if(m_pFields->target != NO_STRING_VALUE)
		m_pState->flags |= FL_INITIALIZE;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CFuncTrain::SetSpawnProperties( void )
{
	m_pState->movetype = MOVETYPE_PUSH;

	if(HasSpawnFlag(FL_NOT_SOLID))
		m_pState->solid = SOLID_NOT;
	else
		m_pState->solid = SOLID_BSP;
}

//=============================================
// @brief
//
//=============================================
bool CFuncTrain::TrainSetModel( void )
{
	if(!SetModel(m_pFields->modelname))
		return false;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CFuncTrain::InitEntity( void )
{
	edict_t* pedict = nullptr;
	while(true)
	{
		pedict = Util::FindEntityByTargetName(pedict, gd_engfuncs.pfnGetString(m_pFields->target));
		if(!pedict)
			break;

		CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
		if(pEntity->IsPathCornerEntity())
			break;
	}

	if(!pedict)
	{
		Util::EntityConPrintf(m_pEdict, "Couldn't find target '%s'.\n", GetTarget());
		Util::RemoveEntity(this);
		return;
	}

	CBaseEntity* pTargetEntity = CBaseEntity::GetClass(pedict);
	if(!pTargetEntity)
	{
		Util::EntityConPrintf(m_pEdict, "Couldn't find target '%s'.\n", GetTarget());
		Util::RemoveEntity(this);
		return;
	}

	m_pFields->target = gd_engfuncs.pfnAllocString(pTargetEntity->GetTarget());
	m_pCurrentTarget = pTargetEntity;

	gd_engfuncs.pfnSetOrigin(m_pEdict, GetDestinationVector(pTargetEntity->GetOrigin()));

	if(m_pFields->targetname == NO_STRING_VALUE || HasSpawnFlag(FL_START_ON))
	{
		SetThink(&CFuncTrain::Next);
		m_pState->nextthink = m_pState->ltime + 0.1;
		m_pState->spawnflags &= ~FL_START_ON;
	}
	else
	{
		m_pState->spawnflags |= FL_WAIT_RETRIGGER;
	}
}

//=============================================
// @brief
//
//=============================================
void CFuncTrain::OnOverrideEntity( void )
{
	if(m_pState->velocity.IsZero() || !m_pState->nextthink)
		return;

	m_pFields->target = m_currentPathCornerName;
	CBaseEntity* pTarget = GetNextTarget();
	if(!pTarget)
	{
		m_pState->nextthink = 0;
		m_pState->velocity.Clear();
	}
	else
	{
		SetThink(&CFuncTrain::Next);
		m_pState->nextthink = m_pState->ltime + 0.1;
	}
}

//=============================================
// @brief
//
//=============================================
void CFuncTrain::Reroute( CBaseEntity* pTarget, Float speed )
{
	// Remember the last target for level changes
	m_currentPathCornerName = m_pFields->target;

	m_pPreviousTarget = pTarget;
	m_pFields->target = gd_engfuncs.pfnAllocString(pTarget->GetTarget());
	m_waitTime = pTarget->GetDelay();

	if(m_pCurrentTarget)
	{
		// Set speed if path_corner's speed is not zero
		Float cornerSpeed = m_pCurrentTarget->GetSpeed();
		if(cornerSpeed != 0)
		{
			m_pState->speed = cornerSpeed;
			Util::EntityConDPrintf(m_pEdict, "Speed set to %.2f.\n", m_pState->speed);
		}

		// Set the avelocity if set to do so
		Vector cornerAVelocity = m_pCurrentTarget->GetAngularVelocity();
		if((!cornerAVelocity.IsZero() || m_pCurrentTarget->HasSpawnFlag(CPathCorner::FL_SET_ZERO_AVELOCITY))
			&& (HasSpawnFlag(FL_ALWAYS_SET_AVELOCITY) || !m_pState->avelocity.IsZero()))
		{
			m_pState->avelocity = cornerAVelocity;
			Util::EntityConDPrintf(m_pEdict, "Angular velocity set to %.2f %.2f %.2f.\n", cornerAVelocity.x, cornerAVelocity.y, cornerAVelocity.z);
		}
	}

	m_pCurrentTarget = pTarget;
	m_lastTargetName = gd_engfuncs.pfnAllocString(pTarget->GetTargetName());

	if(speed)
		m_pState->speed = speed;

	if(m_pCurrentTarget->HasSpawnFlag(CPathCorner::FL_TELEPORT))
	{
		m_pState->effects |= EF_NOINTERP;
		gd_engfuncs.pfnSetOrigin(m_pEdict, GetDestinationVector(pTarget->GetOrigin()));

		// Process next path_corner
		Wait();
	}
	else
	{
		if(!m_isSoundPlaying)
		{
			if(m_stopSoundFile != NO_STRING_VALUE)
				Util::EmitEntitySound(this, m_stopSoundFile, SND_CHAN_ITEM, m_volume);

			m_isSoundPlaying = true;
		}

		m_pState->effects &= ~EF_NOINTERP;
		SetMoveDone(&CFuncTrain::Wait);

		LinearMove(GetDestinationVector(pTarget->GetOrigin()), m_pState->speed);
	}
}

//=============================================
// @brief
//
//=============================================
void CFuncTrain::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	if(HasSpawnFlag(FL_WAIT_RETRIGGER))
	{
		m_pState->spawnflags &= ~FL_WAIT_RETRIGGER;
		Next();
	}
	else
	{
		m_pState->spawnflags |= FL_WAIT_RETRIGGER;

		ClearThinkFunctions();
		m_pState->velocity.Clear();

		if(m_moveSoundFile != NO_STRING_VALUE)
		{
			const Char* pstrSoundFile = gd_engfuncs.pfnGetString(m_moveSoundFile);
			gd_engfuncs.pfnPlayEntitySound(m_pEdict->entindex, pstrSoundFile, SND_FL_STOP, SND_CHAN_VOICE, 0, 0, 0, 0, NO_CLIENT_INDEX);

			// Remember if sound is playing or not
			m_isSoundPlaying = false;
		}

		if(m_stopSoundFile)
			Util::EmitEntitySound(this, m_stopSoundFile, SND_CHAN_ITEM, m_volume);
	}
}

//=============================================
// @brief
//
//=============================================
void CFuncTrain::CallBlocked( CBaseEntity* pBlocker )
{
	// Nudge NPC
	if(pBlocker->IsNPC() && !pBlocker->IsPlayer())
		pBlocker->GroundEntityNudge();

	if(pBlocker->GetFlags() & FL_ONGROUND && pBlocker->GetGroundEntity() == this)
	{
		// Send the blocker flying if he has a z velocity
		Vector blockerVelocity = pBlocker->GetVelocity();
		if(blockerVelocity.z)
		{
			blockerVelocity.z += 200;
			pBlocker->SetVelocity(blockerVelocity);
		}
	}

	// Deal damage if set
	if(m_damageDealt > 0)
		pBlocker->TakeDamage(this, pBlocker, m_damageDealt, DMG_CRUSH);
}

//=============================================
// @brief
//
//=============================================
void CFuncTrain::Wait( void )
{
	if(!HasSpawnFlag(FL_NO_PASS_TRIGGER) && m_pCurrentTarget->HasMessage())
	{
		// Trigger path_corner's track if set
		const Char* pstrPathTarget = m_pCurrentTarget->GetMessage();
		Util::FireTargets(pstrPathTarget, this, this, USE_TOGGLE, 0);
		if(m_pCurrentTarget->HasSpawnFlag(CPathCorner::FL_FIRE_ONCE))
			m_pCurrentTarget->SetMessage(nullptr);
	}

	if(m_pCurrentTarget->HasSpawnFlag(CPathCorner::FL_WAIT_FOR_TRIGGER) || HasSpawnFlag(FL_WAIT_RETRIGGER))
	{
		m_pState->spawnflags |= FL_WAIT_RETRIGGER;

		if(!m_skipSounds)
		{
			if(m_moveSoundFile != NO_STRING_VALUE)
			{
				const Char* pstrSoundFile = gd_engfuncs.pfnGetString(m_moveSoundFile);
				gd_engfuncs.pfnPlayEntitySound(m_pEdict->entindex, pstrSoundFile, SND_FL_STOP, SND_CHAN_VOICE, 0, 0, 0, 0, NO_CLIENT_INDEX);
			}

			if(m_stopSoundFile != NO_STRING_VALUE)
				Util::EmitEntitySound(this, m_stopSoundFile, SND_CHAN_ITEM, m_volume);

			m_isSoundPlaying = false;
		}
		else
		{
			// Clear this
			m_skipSounds = false;
		}

		m_pState->nextthink = 0;

		if(!HasSpawnFlag(FL_DONT_NUDGE_NPCS))
			Util::FixGroundEntities(this);
	}
	else if(m_waitTime != 0)
	{
		// Set next time
		SetThink(&CFuncTrain::Next);
		m_pState->nextthink = m_pState->ltime + m_waitTime;

		if(!m_skipSounds)
		{
			if(m_moveSoundFile != NO_STRING_VALUE)
			{
				const Char* pstrSoundFile = gd_engfuncs.pfnGetString(m_moveSoundFile);
				gd_engfuncs.pfnPlayEntitySound(m_pEdict->entindex, pstrSoundFile, SND_FL_STOP, SND_CHAN_VOICE, 0, 0, 0, 0, NO_CLIENT_INDEX);
			}

			if(m_stopSoundFile != NO_STRING_VALUE)
				Util::EmitEntitySound(this, m_stopSoundFile, SND_CHAN_ITEM, m_volume);

			m_isSoundPlaying = false;
		}
		else
		{
			// Clear this
			m_skipSounds = false;
		}

		if(!HasSpawnFlag(FL_DONT_NUDGE_NPCS))
			Util::FixGroundEntities(this);
	}
	else
	{
		// Just go to next
		Next();
	}
}

//=============================================
// @brief
//
//=============================================
void CFuncTrain::Next( void )
{
	// Find our next target
	CBaseEntity* pTarget = GetNextTarget();
	if(!pTarget || m_pCurrentTarget == pTarget)
	{
		if(m_moveSoundFile != NO_STRING_VALUE)
		{
			const Char* pstrSoundFile = gd_engfuncs.pfnGetString(m_moveSoundFile);
			gd_engfuncs.pfnPlayEntitySound(m_pEdict->entindex, pstrSoundFile, SND_FL_STOP, SND_CHAN_VOICE, 0, 0, 0, 0, NO_CLIENT_INDEX);
		}

		if(m_stopSoundFile != NO_STRING_VALUE)
			Util::EmitEntitySound(this, m_stopSoundFile, SND_CHAN_ITEM, m_volume);

		m_isSoundPlaying = false;

		if(!HasSpawnFlag(FL_DONT_NUDGE_NPCS))
			Util::FixGroundEntities(this);

		if(m_pCurrentTarget == pTarget)
			Util::EntityConPrintf(m_pEdict, "Path corner '%s' targets itself.\n", m_pCurrentTarget->GetTargetName());

		return;
	}

	// Remember for level transition fix-up
	m_currentPathCornerName = m_pFields->target;

	m_pPreviousTarget = pTarget;
	m_pFields->target = gd_engfuncs.pfnAllocString(pTarget->GetTarget());
	m_waitTime = pTarget->GetDelay();

	if(m_pCurrentTarget)
	{
		// Set speed if path_corner's speed is not zero
		Float cornerSpeed = m_pCurrentTarget->GetSpeed();
		if(cornerSpeed != 0)
		{
			m_pState->speed = cornerSpeed;
			Util::EntityConDPrintf(m_pEdict, "Speed set to %.2f.\n", m_pState->speed);
		}

		// Set the avelocity if set to do so
		Vector cornerAVelocity = m_pCurrentTarget->GetAngularVelocity();
		if((!cornerAVelocity.IsZero() || m_pCurrentTarget->HasSpawnFlag(CPathCorner::FL_SET_ZERO_AVELOCITY))
			&& (HasSpawnFlag(FL_ALWAYS_SET_AVELOCITY) || !m_pState->avelocity.IsZero()))
		{
			m_pState->avelocity = cornerAVelocity;
			Util::EntityConDPrintf(m_pEdict, "Angular velocity set to %.2f %.2f %.2f.\n", cornerAVelocity.x, cornerAVelocity.y, cornerAVelocity.z);
		}
	}

	m_pCurrentTarget = pTarget;
	m_lastTargetName = gd_engfuncs.pfnAllocString(pTarget->GetTargetName());

	if(m_pCurrentTarget->HasSpawnFlag(CPathCorner::FL_TELEPORT))
	{
		m_pState->effects |= EF_NOINTERP;
		gd_engfuncs.pfnSetOrigin(m_pEdict, GetDestinationVector(pTarget->GetOrigin()));

		// Process next path_corner
		Wait();
	}
	else
	{
		if(!m_isSoundPlaying)
		{
			if(m_moveSoundFile != NO_STRING_VALUE)
				Util::EmitEntitySound(this, m_moveSoundFile, SND_CHAN_VOICE, m_volume);

			m_isSoundPlaying = true;
		}

		m_pState->effects &= ~EF_NOINTERP;
		SetMoveDone(&CFuncTrain::Wait);

		LinearMove(GetDestinationVector(pTarget->GetOrigin()), m_pState->speed);
	}
}

//=============================================
// @brief
//
//=============================================
Vector CFuncTrain::GetDestinationVector( const Vector& destOrigin )
{
	if(HasSpawnFlag(FL_USE_ORIGIN))
		return destOrigin;
	else
		return destOrigin - (m_pState->mins + m_pState->maxs) * 0.5 - TRAIN_CENTER_OFFSET;
}

//=============================================
// @brief
//
//=============================================
void CFuncTrain::MoveTrainToPathCorner( CBaseEntity* pPathCorner, CBaseEntity* pTargetingPathCorner )
{
	if(!pPathCorner)
		return;

	// Get the name of the entity
	const Char* pstrPathCornerName = pPathCorner->GetTargetName();

	// Verify if it's a path_corner entity
	if(!pPathCorner->IsPathCornerEntity())
	{
		Util::EntityConPrintf(m_pEdict, "Destination '%s' is not a path_corner entity.\n", pPathCorner->GetTargetName());
		return;
	}

	// Get the engine's index
	m_lastTargetName = gd_engfuncs.pfnAllocString(pstrPathCornerName);
	m_pFields->target = gd_engfuncs.pfnAllocString(pPathCorner->GetTarget());

	if(pTargetingPathCorner)
		m_currentPathCornerName = gd_engfuncs.pfnAllocString(pTargetingPathCorner->GetTargetName());
	else
		m_currentPathCornerName = NO_STRING_VALUE;

	m_pPreviousTarget = pTargetingPathCorner;
	m_pCurrentTarget = pPathCorner;

	// Get the valid destination vector
	Vector destPosition = GetDestinationVector(m_pCurrentTarget->GetOrigin());
	gd_engfuncs.pfnSetOrigin(m_pEdict, destPosition);

	// Don't interpolate this change and don't play sounds in Wait
	m_pState->effects |= EF_NOINTERP;
	m_skipSounds = true;

	// Call to wait until further triggering
	Wait();
}