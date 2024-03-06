/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "funcpushable.h"
#include "buttonbits.h"

// Default push sound pattern
const Char CFuncPushable::PUSHABLE_DEFAULT_PUSH_SOUND[] = "debris/wood_push%d.wav";
// Maximum friction
const Float CFuncPushable::MAX_FRICTION = 399;
// Minimum sound delay time
const Float CFuncPushable::MIN_SOUND_DELAY = 0.7;

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(func_pushable, CFuncPushable);

//=============================================
// @brief
//
//=============================================
CFuncPushable::CFuncPushable( edict_t* pedict ):
	CFuncBreakable(pedict),
	m_pushSoundsPattern(NO_STRING_VALUE),
	m_nbPushSounds(0),
	m_maxSpeed(0),
	m_nextSoundTime(0)
{
}

//=============================================
// @brief
//
//=============================================
CFuncPushable::~CFuncPushable( void )
{
}

//=============================================
// @brief
//
//=============================================
void CFuncPushable::DeclareSaveFields( void )
{
	// Call base class to do it first
	CFuncBreakable::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncPushable, m_pushSoundsPattern, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncPushable, m_maxSpeed, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncPushable, m_nextSoundTime, EFIELD_TIME));
}

//=============================================
// @brief
//
//=============================================
bool CFuncPushable::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "size"))
	{
		// This is actually not used, and would cause errors
		return true;
	}
	else if(!qstrcmp(kv.keyname, "buoyancy"))
	{
		m_pState->skin = SDL_atof(kv.value);
		return true;
	}
	else
		return CFuncBreakable::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CFuncPushable::Spawn( void )
{
	// Set push sound pattern
	m_pushSoundsPattern = gd_engfuncs.pfnAllocString(PUSHABLE_DEFAULT_PUSH_SOUND);

	// Set precache objects
	SetPrecacheObjects();

	if(!CDelayEntity::Spawn())
		return false;

	if(HasSpawnFlag(FL_BREAKABLE))
	{
		if(HasSpawnFlag(FL_BREAK_ON_TRIGGER_ONLY))
			m_pState->takedamage = TAKEDAMAGE_NO;
		else
			m_pState->takedamage = TAKEDAMAGE_YES;

		m_angle = m_pState->angles.y;
		m_pState->angles.y = 0;
	}

	m_pState->solid = SOLID_BBOX;
	m_pState->movetype = MOVETYPE_PUSHSTEP;

	if(!SetModel(m_pFields->modelname))
		return false;

	if(HasSpawnFlag(FL_BREAKABLE) && !HasSpawnFlag(FL_BREAK_ON_TRIGGER_ONLY))
		SetTouch(&CFuncBreakable::BreakTouch);

	if(m_pState->friction > (MAX_FRICTION-1))
		m_pState->friction = (MAX_FRICTION-1);

	// Determine max speed
	m_maxSpeed = MAX_FRICTION - m_pState->friction;
	m_pState->flags |= FL_FLOAT;

	// List it off the ground a bit
	m_pState->origin.z += 1;
	gd_engfuncs.pfnSetOrigin(m_pEdict, m_pState->origin);

	// Set up bouyancy properly
	m_pState->skin = (m_pState->skin
		* (m_pState->maxs.x - m_pState->mins.x)
		* (m_pState->maxs.y - m_pState->mins.y)) *0.0005;

	m_nextSoundTime = 0;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CFuncPushable::Precache( void )
{
	if(m_pushSoundsPattern != NO_STRING_VALUE)
	{
		const Char* pstrSoundPattern = gd_engfuncs.pfnGetString(m_pushSoundsPattern);
		Util::PrecacheVariableNbSounds(pstrSoundPattern, m_nbPushSounds);
	}

	if(HasSpawnFlag(FL_BREAKABLE))
		CFuncBreakable::Precache();
}

//=============================================
// @brief
//
//=============================================
void CFuncPushable::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	if(!pActivator || !pActivator->IsPlayer())
	{
		if(HasSpawnFlag(FL_BREAKABLE))
			CFuncBreakable::CallUse(pActivator, pCaller, useMode, value);

		// Don't do anything else
		return;
	}
	else
	{
		// Allow player to move it
		Move(pActivator, false);
	}
}

//=============================================
// @brief
//
//=============================================
void CFuncPushable::CallTouch( CBaseEntity* pOther )
{
	if(pOther->IsWorldSpawn())
		return;

	Move(pOther, true);
}

//=============================================
// @brief
//
//=============================================
bool CFuncPushable::TakeDamage( CBaseEntity* pInflictor, CBaseEntity* pAttacker, Float amount, Int32 damageFlags )
{
	if(HasSpawnFlag(FL_BREAKABLE))
		return CFuncBreakable::TakeDamage(pInflictor, pAttacker, amount, damageFlags);
	else
		return true;
}

//=============================================
// @brief
//
//=============================================
Float CFuncPushable::GetMaxSpeed( void ) const
{
	return m_maxSpeed;
}

//=============================================
// @brief
//
//=============================================
void CFuncPushable::Move( CBaseEntity* pEntity, bool push )
{
	// Check if mover is standing on us
	if(m_pState->flags & FL_ONGROUND && pEntity->GetGroundEntity() == this)
	{
		if(m_pState->waterlevel > WATERLEVEL_NONE)
			m_pState->velocity.z += pEntity->GetVelocity().z*0.1;

		return;
	}

	bool isPlayerTouch = false;
	if(pEntity->IsPlayer())
	{
		if(push && !(pEntity->GetButtonBits() & (IN_USE|IN_FORWARD)))
			return;

		isPlayerTouch = true;
	}

	Float factor = 0;
	if(isPlayerTouch)
	{
		if(!(pEntity->GetFlags() & FL_ONGROUND))
		{
			if(m_pState->waterlevel < WATERLEVEL_LOW)
				return;

			factor = 0.1;
		}
		else
		{
			factor = 1.0;
		}
	}
	else
	{
		factor = 0.25;
	}

	Vector velocity = m_pState->velocity;
	Vector otherVelocity = pEntity->GetVelocity();
	m_pState->velocity.x += otherVelocity.x * factor;
	if(SDL_fabs(m_pState->velocity.x) > SDL_fabs(otherVelocity.x))
		m_pState->velocity.x = otherVelocity.x;

	m_pState->velocity.y += otherVelocity.y * factor;
	if(SDL_fabs(m_pState->velocity.y) > SDL_fabs(otherVelocity.y))
		m_pState->velocity.y = otherVelocity.y;

	Float length = SDL_sqrt(velocity.x*velocity.x+velocity.y*velocity.y);
	if(push && length > m_maxSpeed)
	{
		m_pState->velocity.x = ((velocity.x*m_maxSpeed)/length);
		m_pState->velocity.y = ((velocity.y*m_maxSpeed)/length);
	}

	if(isPlayerTouch)
	{
		otherVelocity.x = velocity.x;
		otherVelocity.y = velocity.y;
		pEntity->SetVelocity(otherVelocity);

		if(m_pushSoundsPattern != NO_STRING_VALUE && 
			length > 0 && m_pState->flags & FL_ONGROUND)
		{
			if(m_nextSoundTime < g_pGameVars->time)
			{
				const Char* pstrSoundPattern = gd_engfuncs.pfnGetString(m_pushSoundsPattern);
				Util::PlayRandomEntitySound(this, pstrSoundPattern, m_nbPushSounds, SND_CHAN_WEAPON);

				m_nextSoundTime = g_pGameVars->time + MIN_SOUND_DELAY;
			}
		}
	}
}