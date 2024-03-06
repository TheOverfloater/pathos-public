/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "funcrotating.h"

// Array of fan sounds
const Char* CFuncRotating::g_pFanSounds[NUM_FAN_SOUND_OPTIONS] = 
{
	"",
	"fans/fan1.wav",
	"fans/fan2.wav",
	"fans/fan3.wav",
	"fans/fan4.wav",
	"fans/fan5.wav"
};

// Fan minimum pitch
const Float CFuncRotating::FAN_MIN_PITCH = 30;
// Fan maximum pitch
const Float CFuncRotating::FAN_MAX_PITCH = 100;

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(func_rotating, CFuncRotating);

//=============================================
// @brief
//
//=============================================
CFuncRotating::CFuncRotating( edict_t* pedict ):
	CBaseEntity(pedict),
	m_fanFriction(0),
	m_fanAttenuation(0),
	m_volume(0),
	m_pitch(0),
	m_damage(0),
	m_sounds(0),
	m_fanSound(NO_STRING_VALUE),
	m_lastThinkTime(0),
	m_fanState(FAN_STATE_OFF)
{
}

//=============================================
// @brief
//
//=============================================
CFuncRotating::~CFuncRotating( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CFuncRotating::Spawn( void )
{
	if(!CBaseEntity::Spawn())
		return false;

	// Do not set exactly to PITCH_NORM
	m_pitch = PITCH_NORM-1;

	// Make sure we have a volume
	if(!m_volume)
		m_volume = 1.0;

	// Set attenuation
	if(HasSpawnFlag(FL_SND_SMALL_RADIUS))
		m_fanAttenuation = ATTN_IDLE;
	else if(HasSpawnFlag(FL_SND_MEDIUM_RADIUS))
		m_fanAttenuation = ATTN_STATIC;
	else
		m_fanAttenuation = ATTN_NORM;

	// Prevent divide by zero
	if(!m_fanFriction)
		m_fanFriction = 1.0;

	// Set move direction based on spawnflags
	Util::SetAxisDirection(*m_pState, m_pState->spawnflags, FL_ROTATE_Z_AXIS, FL_ROTATE_X_AXIS);

	// Reverse rotation if needed
	if(HasSpawnFlag(FL_ROTATE_REVERSE))
		Math::VectorScale(m_pState->movedir, -1, m_pState->movedir);

	if(HasSpawnFlag(FL_ROTATING_NOT_SOLID))
	{
		m_pState->solid = SOLID_NOT;
		m_pState->skin = CONTENTS_EMPTY;
	}
	else
	{
		m_pState->solid = SOLID_BSP;
	}

	// Always MOVETYPE_PUSH regardless of solidity
	m_pState->movetype = MOVETYPE_PUSH;

	if(m_pState->speed < 0)
		m_pState->speed = 0;

	if(HasSpawnFlag(FL_ROTATE_START_ON))
		m_pState->flags |= FL_INITIALIZE;

	if(HasSpawnFlag(FL_INFLICT_DAMAGE))
		SetTouch(&CFuncRotating::HurtTouch);

	if(!SetModel(m_pFields->modelname))
		return false;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CFuncRotating::Precache( void )
{
	const Char* pstrSoundFile = nullptr;
	if(m_pFields->message != NO_STRING_VALUE)
	{
		pstrSoundFile = gd_engfuncs.pfnGetString(m_pFields->message);
	}
	else if(m_sounds)
	{
		if(m_sounds < 0 || m_sounds >= NUM_FAN_SOUND_OPTIONS)
		{
			Util::EntityConPrintf(m_pEdict, "Invalid fan sound set.\n");
			return;
		}

		// Set the file
		pstrSoundFile = g_pFanSounds[m_sounds];
	}

	if(pstrSoundFile)
	{
		m_fanSound = gd_engfuncs.pfnAllocString(pstrSoundFile);
		gd_engfuncs.pfnPrecacheSound(pstrSoundFile);
	}

	if(!m_pState->avelocity.IsZero())
	{
		SetThink(&CFuncRotating::SpinUpThink);
		m_pState->nextthink = m_pState->ltime + 1.5;
	}
}

//=============================================
// @brief
//
//=============================================
bool CFuncRotating::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "fanfriction"))
	{
		m_fanFriction = SDL_atof(kv.value)/100.0f;
		return true;
	}
	else if(!qstrcmp(kv.keyname, "dmg"))
	{
		m_damage = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "volume"))
	{
		m_volume = SDL_atof(kv.value)/10.0f;
		m_volume = clamp(m_volume, 0.0, 1.0);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "spawnorigin"))
	{
		Vector origin;
		Common::StringToVector(kv.value, origin);
		if(!origin.IsZero())
			m_pState->origin = origin;
		return true;
	}
	else if(!qstrcmp(kv.keyname, "fanfriction"))
	{
		m_sounds = SDL_atoi(kv.value);
		return true;
	}
	else
		return CBaseEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
void CFuncRotating::DeclareSaveFields( void )
{
	// Call base class to handle it's own
	CBaseEntity::DeclareSaveFields();

	DeclareSaveField(DEFINE_DATA_FIELD(CFuncRotating, m_fanFriction, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncRotating, m_fanAttenuation, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncRotating, m_volume, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncRotating, m_pitch, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncRotating, m_sounds, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncRotating, m_fanSound, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncRotating, m_damage, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncRotating, m_fanState, EFIELD_INT32));
}

//=============================================
// @brief
//
//=============================================
void CFuncRotating::HurtTouch( CBaseEntity* pOther )
{
	if(pOther->GetTakeDamage() == TAKEDAMAGE_NO)
		return;

	Float dmgamount = m_pState->avelocity.Length() / 10.0f;
	pOther->TakeDamage(this, this, dmgamount, DMG_CRUSH);

	Vector velocity = (pOther->GetOrigin() - GetBrushModelCenter()).Normalize() * dmgamount;
	pOther->SetVelocity(velocity);
}

//=============================================
// @brief
//
//=============================================
void CFuncRotating::SpinUpThink( void )
{
	m_pState->nextthink = m_pState->ltime + 0.1;

	if(!m_lastThinkTime)
	{
		m_lastThinkTime = g_pGameVars->time;
		return;
	}

	Double thinkTime = g_pGameVars->time - m_lastThinkTime;
	m_lastThinkTime = g_pGameVars->time;

	Math::VectorMA(m_pState->avelocity, (m_pState->speed*m_fanFriction*thinkTime), m_pState->movedir, m_pState->avelocity);

	if(SDL_fabs(m_pState->avelocity.x) >= SDL_fabs(m_pState->movedir.x*m_pState->speed) 
		&& SDL_fabs(m_pState->avelocity.y) >= SDL_fabs(m_pState->movedir.y*m_pState->speed)
		&& SDL_fabs(m_pState->avelocity.z) >= SDL_fabs(m_pState->movedir.z*m_pState->speed))
	{
		// Set final speed
		m_pState->avelocity = m_pState->movedir * m_pState->speed;

		// Update on client
		if(m_fanSound != NO_STRING_VALUE)
		{
			gd_engfuncs.pfnPlayEntitySound(m_pEdict->entindex, 
				gd_engfuncs.pfnGetString(m_fanSound),
				SND_FL_CHANGE_PITCH|SND_FL_CHANGE_VOLUME,
				SND_CHAN_STATIC,
				m_volume,
				m_fanAttenuation,
				FAN_MAX_PITCH,
				0,
				NO_CLIENT_INDEX);
		}

		SetThink(&CFuncRotating::RotateThink);
		RotateThink();

		// Set state to off
		m_fanState = FAN_STATE_ON;
		m_pState->flags &= ~FL_ALWAYSTHINK;
	}
	else
	{
		// Otherwise just ramp volume and pitch
		RampPitchAndVolume();
	}
}

//=============================================
// @brief
//
//=============================================
void CFuncRotating::SpinDownThink( void )
{
	m_pState->nextthink = m_pState->ltime + 0.1;

	if(!m_lastThinkTime)
	{
		m_lastThinkTime = g_pGameVars->time;
		return;
	}

	Double thinkTime = g_pGameVars->time - m_lastThinkTime;
	m_lastThinkTime = g_pGameVars->time;

	Math::VectorMA(m_pState->avelocity, (m_pState->speed*m_fanFriction*thinkTime) * -1, m_pState->movedir, m_pState->avelocity);

	Float movedir;
	if(m_pState->movedir.x != 0)
		movedir = m_pState->movedir.x;
	else if(m_pState->movedir.y != 0)
		movedir = m_pState->movedir.y;
	else
		movedir = m_pState->movedir.z;

	if((movedir > 0 && m_pState->avelocity.x <= 0 && m_pState->avelocity.y <= 0 && m_pState->avelocity.z <= 0) 
		|| (movedir < 0 && m_pState->avelocity.x >= 0 && m_pState->avelocity.y >= 0 && m_pState->avelocity.z >= 0))
	{
		// Set final speed
		m_pState->avelocity.Clear();

		// Update on client
		if(m_fanSound != NO_STRING_VALUE)
		{
			gd_engfuncs.pfnPlayEntitySound(m_pEdict->entindex, 
				gd_engfuncs.pfnGetString(m_fanSound),
				SND_FL_STOP,
				SND_CHAN_STATIC,
				0,
				0,
				0,
				0,
				NO_CLIENT_INDEX);
		}

		ClearThinkFunctions();

		// Set state to off
		m_fanState = FAN_STATE_OFF;
		m_pState->flags &= ~FL_ALWAYSTHINK;
	}
	else
	{
		// Otherwise just ramp volume and pitch
		RampPitchAndVolume();
	}
}

//=============================================
// @brief
//
//=============================================
void CFuncRotating::RotateThink( void )
{
	m_pState->nextthink = m_pState->ltime + 10;
}

//=============================================
// @brief
//
//=============================================
void CFuncRotating::InitEntity( void )
{
	// Used for delayed initialization
	CallUse(this, this, USE_TOGGLE, 0);

	m_pState->flags &= ~FL_INITIALIZE;
}

//=============================================
// @brief
//
//=============================================
void CFuncRotating::RampPitchAndVolume( void )
{
	// Get angular velocity
	Vector vecvel = m_pState->avelocity;
	Float anglevelocity = abs(vecvel.x != 0 ? vecvel.x : (vecvel.y != 0 ? vecvel.y : vecvel.z));

	// Determine target
	Vector vecdir = m_pState->movedir;
	Float targetvelocity = abs(abs(vecdir.x != 0 ? vecdir.x : (vecdir.y != 0 ? vecdir.y : vecdir.z)) * m_pState->speed);

	// Calculate pitch
	Int32 pitch = (Int32(FAN_MIN_PITCH + (FAN_MAX_PITCH-FAN_MIN_PITCH)*(anglevelocity/targetvelocity)));
	if(pitch == PITCH_NORM)
		pitch = PITCH_NORM-1;

	// Calculate volume
	Float volume = m_volume * (anglevelocity/targetvelocity);

	// Update on client
	if(m_fanSound != NO_STRING_VALUE)
	{
		gd_engfuncs.pfnPlayEntitySound(m_pEdict->entindex, 
			gd_engfuncs.pfnGetString(m_fanSound),
			SND_FL_CHANGE_PITCH|SND_FL_CHANGE_VOLUME,
			SND_CHAN_STATIC,
			volume,
			m_fanAttenuation,
			pitch,
			0,
			NO_CLIENT_INDEX);
	}
}

//=============================================
// @brief
//
//=============================================
void CFuncRotating::CallUse( CBaseEntity* pacticator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	if(m_fanState == FAN_STATE_ON 
		|| m_fanState == FAN_STATE_SPINUP)
	{
		// Start spinning up
		SetThink(&CFuncRotating::SpinDownThink);
		// Set next think time
		m_pState->nextthink = m_pState->ltime + 0.1;
		// Set state
		m_fanState = FAN_STATE_SPINDOWN;
		m_pState->flags |= FL_ALWAYSTHINK;
		m_lastThinkTime = 0;
	}
	else if(HasSpawnFlag(FL_ACCELERATE_DECELERATE)
		&& !(m_pState->flags & FL_INITIALIZE))
	{
		SetThink(&CFuncRotating::SpinUpThink);
		// Set next think time
		m_pState->nextthink = m_pState->ltime + 0.1;

		// Set sound
		if(m_fanSound != NO_STRING_VALUE)
		{
			gd_engfuncs.pfnPlayEntitySound(m_pEdict->entindex, 
				gd_engfuncs.pfnGetString(m_fanSound),
				SND_FL_NONE,
				SND_CHAN_STATIC,
				m_volume*0.01,
				m_fanAttenuation,
				FAN_MIN_PITCH,
				0,
				NO_CLIENT_INDEX);
		}

		// Set state
		m_fanState = FAN_STATE_SPINUP;
		m_pState->flags |= FL_ALWAYSTHINK;
		m_lastThinkTime = 0;
	}
	else
	{
		Math::VectorScale(m_pState->movedir, m_pState->speed, m_pState->avelocity);

		SetThink(&CFuncRotating::RotateThink);
		RotateThink();

		// Set sound
		if(m_fanSound != NO_STRING_VALUE)
		{
			gd_engfuncs.pfnPlayEntitySound(m_pEdict->entindex, 
				gd_engfuncs.pfnGetString(m_fanSound),
				SND_FL_NONE,
				SND_CHAN_STATIC,
				m_volume,
				m_fanAttenuation,
				FAN_MAX_PITCH,
				0,
				NO_CLIENT_INDEX);
		}

		// Set state
		m_fanState = FAN_STATE_ON;
		m_pState->flags &= ~FL_ALWAYSTHINK;
		m_lastThinkTime = 0;
	}
}

//=============================================
// @brief
//
//=============================================
void CFuncRotating::CallBlocked( CBaseEntity* pOther )
{
	if(!pOther)
		return;

	pOther->TakeDamage(this, this, m_damage, DMG_CRUSH);
}