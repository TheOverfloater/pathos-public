/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "ambientgeneric.h"
#include "snd_shared.h"

// TRUE if we're in InitializeEntities
extern bool g_bInInitializeEntities;

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(ambient_generic, CAmbientGeneric);

//=============================================
// @brief
//
//=============================================
CAmbientGeneric::CAmbientGeneric( edict_t* pedict ):
	CPointEntity(pedict),
	m_isActive(false),
	m_isLooping(false),
	m_sndFlags(SND_FL_NONE),
	m_soundRadius(0),
	m_attenuation(0),
	m_volume(0),
	m_startVolume(0),
	m_pitch(PITCH_NORM),
	m_startPitch(0),
	m_volumeFadeInTime(0),
	m_volumeFadeOutTime(0),
	m_pitchFadeInTime(0),
	m_pitchFadeOutTime(0),
	m_beginTime(0),
	m_turnoffBeginTime(0),
	m_turnoffEndTime(0),
	m_emitterEntityName(NO_STRING_VALUE)
{
}

//=============================================
// @brief
//
//=============================================
CAmbientGeneric::~CAmbientGeneric( void )
{
}

//=============================================
// @brief
//
//=============================================
void CAmbientGeneric::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CAmbientGeneric, m_isActive, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CAmbientGeneric, m_isLooping, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CAmbientGeneric, m_sndFlags, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CAmbientGeneric, m_soundRadius, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CAmbientGeneric, m_attenuation, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CAmbientGeneric, m_volume, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CAmbientGeneric, m_startVolume, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CAmbientGeneric, m_pitch, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CAmbientGeneric, m_startPitch, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CAmbientGeneric, m_volumeFadeInTime, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CAmbientGeneric, m_volumeFadeOutTime, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CAmbientGeneric, m_pitchFadeInTime, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CAmbientGeneric, m_pitchFadeOutTime, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CAmbientGeneric, m_beginTime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CAmbientGeneric, m_turnoffBeginTime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CAmbientGeneric, m_turnoffEndTime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CAmbientGeneric, m_emitterEntityName, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CAmbientGeneric, m_emitterEntity, EFIELD_EHANDLE));
}

//=============================================
// @brief
//
//=============================================
bool CAmbientGeneric::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "pitch"))
	{
		m_pitch = SDL_atoi(kv.value);
		m_pitch = clamp(m_pitch, MIN_PITCH, MAX_PITCH);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "pitchstart"))
	{
		m_startPitch = SDL_atoi(kv.value);
		m_startPitch = clamp(m_startPitch, MIN_PITCH, MAX_PITCH);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "spinup"))
	{
		m_pitchFadeInTime = SDL_atof(kv.value);
		if(m_pitchFadeInTime < 0)
			m_pitchFadeInTime = 0;
		return true;
	}
	else if(!qstrcmp(kv.keyname, "spindown"))
	{
		m_pitchFadeOutTime = SDL_atof(kv.value);
		if(m_pitchFadeOutTime < 0)
			m_pitchFadeOutTime = 0;
		return true;
	}
	else if(!qstrcmp(kv.keyname, "radius"))
	{
		m_soundRadius = SDL_atof(kv.value);
		if(m_soundRadius < 0)
			m_soundRadius = 0;
		return true;
	}
	else if(!qstrcmp(kv.keyname, "volstart"))
	{
		m_startVolume = SDL_atof(kv.value) * 10;
		if(m_startVolume < 0)
			m_startVolume = 0;
		return true;
	}
	else if(!qstrcmp(kv.keyname, "fadein"))
	{
		m_volumeFadeInTime = SDL_atof(kv.value);
		if(m_volumeFadeInTime < 0)
			m_volumeFadeInTime = 0;
		return true;
	}
	else if(!qstrcmp(kv.keyname, "fadeout"))
	{
		m_volumeFadeOutTime = SDL_atof(kv.value);
		if(m_volumeFadeOutTime < 0)
			m_volumeFadeOutTime = 0;
		return true;
	}
	else if(!qstrcmp(kv.keyname, "emitterentity"))
	{
		m_emitterEntityName = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CAmbientGeneric::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	// Set volume
	m_volume = m_pState->health * 10;
	m_volume = clamp(m_volume, 0, 100);

	// Check for errors
	if(!m_volume)
	{
		Util::EntityConPrintf(m_pEdict, "No volume set.\n");
		return false;
	}
	// Pitch too
	if(!m_pitch)
	{
		Util::EntityConPrintf(m_pEdict, "No pitch set.\n");
		return false;
	}

	// Set attenuation
	if(HasSpawnFlag(FL_RADIUS_EVERYWHERE))
		m_attenuation = ATTN_NONE;
	else if(HasSpawnFlag(FL_RADIUS_SMALL))
		m_attenuation = ATTN_IDLE;
	else if(HasSpawnFlag(FL_RADIUS_MEDIUM))
		m_attenuation = ATTN_STATIC;
	else if(HasSpawnFlag(FL_RADIUS_LARGE))
		m_attenuation = ATTN_NORM;
	else if(HasSpawnFlag(FL_RADIUS_XL))
		m_attenuation = ATTN_NORM/2.8;
	else if(HasSpawnFlag(FL_RADIUS_XXL))
		m_attenuation = ATTN_NORM/6;
	else if(HasSpawnFlag(FL_RADIUS_XS))
		m_attenuation = ATTN_NORM*2.8;
	else
		m_attenuation = ATTN_STATIC;

	// Check reverbless flag
	if(HasSpawnFlag(FL_REVERBLESS))
		m_sndFlags |= SND_FL_REVERBLESS;
	
	// Check occlusionless flag
	if(HasSpawnFlag(FL_NO_OCCLUSION))
		m_sndFlags |= SND_FL_OCCLUSIONLESS;

	// Check dim others flag
	if(HasSpawnFlag(FL_DIM_OTHERS))
		m_sndFlags |= SND_FL_DIM_OTHERS;

	// Check mute others flag
	if(HasSpawnFlag(FL_MUTE_OTHERS))
		m_sndFlags |= SND_FL_MUTEIGNORE;

	// Check for custom radius
	if(m_soundRadius)
	{
		m_attenuation = m_soundRadius;
		m_sndFlags |= SND_FL_RADIUS;
	}

	// Check for no sound set
	if(m_pFields->message == NO_STRING_VALUE)
	{
		Util::WarnEmptyEntity(m_pEdict);
		return false;
	}

	// Set that this is an ambient sound
	m_sndFlags |= SND_FL_AMBIENT;

	if(HasSpawnFlag(FL_NOT_LOOPING))
	{
		// Only play when called
		m_isActive = false;
		m_isLooping = false;
	}
	else
	{
		// This is a looping sound
		m_isLooping = true;

		if(m_pFields->targetname == NO_STRING_VALUE || !HasSpawnFlag(FL_START_SILENT))
			m_isActive = true;
		else
			m_isActive = false;
	}

	// Set to call initialize function
	if(m_emitterEntityName != NO_STRING_VALUE)
		m_pState->flags |= FL_INITIALIZE;
	else
		m_emitterEntity = this;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CAmbientGeneric::Precache( void )
{
	if(m_pFields->message != NO_STRING_VALUE)
	{
		const Char* pstrSound = gd_engfuncs.pfnGetString(m_pFields->message);
		gd_engfuncs.pfnPrecacheSound(pstrSound);
	}
}

//=============================================
// @brief
//
//=============================================
void CAmbientGeneric::InitEntity( void )
{
	const Char* pstrEmitterName = gd_engfuncs.pfnGetString(m_emitterEntityName);
	if(!pstrEmitterName || !qstrlen(pstrEmitterName))
		return;

	edict_t* pEntity = Util::FindEntityByTargetName(nullptr, pstrEmitterName);
	if(!pEntity)
	{
		Util::EntityConPrintf(m_pEdict, "Couldn't find emitter '%s'.\n", pstrEmitterName);
		m_emitterEntity = this;
		return;
	}

	m_emitterEntity = pEntity;
}

//=============================================
// @brief
//
//=============================================
void CAmbientGeneric::TurnOffThink( void )
{
	m_beginTime = 0;
	m_turnoffEndTime = 0;
	m_turnoffBeginTime = 0;

	SetThink(nullptr);
	m_pState->nextthink = 0;

	if(m_emitterEntity != reinterpret_cast<const CBaseEntity*>(this) && m_emitterEntity->IsVisible())
		Util::EmitEntitySound(m_emitterEntity, m_pFields->message, SND_CHAN_AUTO, 0, 0, 0, SND_FL_STOP);
	else
		Util::EmitAmbientSound(ZERO_VECTOR, m_pFields->message, 0, 0, 0, SND_FL_STOP, this);
}

//=============================================
// @brief
//
//=============================================
void CAmbientGeneric::SendInitMessage( const CBaseEntity* pPlayer )
{
	if((m_isActive || m_turnoffEndTime) && m_isLooping || !m_isLooping && m_beginTime)
	{
		// Don't do anything if emitter entity was freed
		if(!m_emitterEntity)
		{
			Util::RemoveEntity(this);
			return;
		}

		// Calculate the volume
		Float volume = 0;
		if(m_volumeFadeInTime && m_beginTime + m_volumeFadeInTime > g_pGameVars->time)
		{
			Float frac = (g_pGameVars->time - m_beginTime)/m_volumeFadeInTime;
			frac = clamp(frac, 0.0, 1.0);

			volume = m_startVolume * (1.0 - frac) + m_volume * frac;
		}
		else if(m_turnoffBeginTime && m_turnoffEndTime)
		{
			if(m_turnoffEndTime <= g_pGameVars->time)
			{
				TurnOffThink();
				return;
			}

			Float frac = (g_pGameVars->time - m_turnoffBeginTime)/m_volumeFadeOutTime;
			frac = clamp(frac, 0.0, 1.0);

			volume = m_volume * (1.0 - frac);
		}
		else
			volume = m_volume;

		// Calculate the volume
		Int32 pitch = 0;
		if(m_pitchFadeInTime && m_beginTime + m_pitchFadeInTime > g_pGameVars->time)
		{
			Float frac = (g_pGameVars->time - m_beginTime)/m_pitchFadeInTime;
			frac = clamp(frac, 0.0, 1.0);

			pitch = m_startPitch * (1.0 - frac) + m_pitch * frac;
		}
		else if(m_pitchFadeOutTime && m_turnoffEndTime)
		{
			if(m_turnoffEndTime <= g_pGameVars->time)
			{
				TurnOffThink();
				return;
			}

			Float frac = (g_pGameVars->time - m_turnoffBeginTime)/m_pitchFadeOutTime;
			frac = clamp(frac, 0.0, 1.0);

			pitch = m_pitch * (1.0 - frac);
		}
		else
			pitch = m_pitch;

		// Check if we should bother at all
		if(!m_isLooping && m_beginTime)
		{
			const Char* pstrSound = gd_engfuncs.pfnGetString(m_pFields->message);
			Float duration = gd_engfuncs.pfnGetSoundDuration(pstrSound, pitch);
			if((g_pGameVars->time-m_beginTime) > duration)
			{
				// Reset this
				m_beginTime = 0;
				return;
			}
		}
		else if(m_isLooping && !m_beginTime)
		{
			// Just set it anyway
			m_beginTime = g_pGameVars->time;
		}

		// Calculate offset in time
		Float timeoffset = g_pGameVars->time - m_beginTime;

		// Play the sound from the last time position
		if(m_emitterEntity != reinterpret_cast<const CBaseEntity*>(this) && m_emitterEntity->IsVisible())
			Util::EmitEntitySound(m_emitterEntity, m_pFields->message, SND_CHAN_AUTO, volume/100.0f, m_attenuation, pitch, m_sndFlags, pPlayer, timeoffset);
		else
			Util::EmitAmbientSound(m_emitterEntity->GetOrigin(), m_pFields->message, volume/100.0f, m_attenuation, pitch, m_sndFlags, m_emitterEntity, pPlayer, timeoffset);

		// Add effects for pitch changes
		if(pitch != m_pitch)
		{
			Float time = 0;
			Int32 target = 0;
			if(m_pitchFadeInTime && m_beginTime + m_pitchFadeInTime > g_pGameVars->time)
			{
				Float frac = (g_pGameVars->time - m_beginTime)/m_pitchFadeInTime;
				frac = clamp(frac, 0.0, 1.0);
				time = m_pitchFadeInTime * (1.0 - frac);
				target = m_pitch;
			}
			else if(m_pitchFadeOutTime && m_turnoffEndTime)
			{
				Float frac = (g_pGameVars->time - m_turnoffBeginTime)/m_pitchFadeOutTime;
				frac = clamp(frac, 0.0, 1.0);
				time = m_pitchFadeOutTime * (1.0 - frac);
				target = 0;
			}

			if(time > 0)
				gd_engfuncs.pfnApplySoundEffect(m_emitterEntity->GetEntityIndex(), gd_engfuncs.pfnGetString(m_pFields->message), SND_CHAN_AUTO, SND_EF_CHANGE_PITCH, time, target, pPlayer ? pPlayer->GetClientIndex() : NO_CLIENT_INDEX);
		}

		// Add effects for volume changes
		if(volume != m_volume)
		{
			Float time = 0;
			Float target = 0;
			if(m_volumeFadeInTime && m_beginTime + m_volumeFadeInTime > g_pGameVars->time)
			{
				Float frac = (g_pGameVars->time - m_beginTime)/m_volumeFadeInTime;
				frac = clamp(frac, 0.0, 1.0);
				time = m_volumeFadeInTime * (1.0 - frac);
				target = m_volume;
			}
			else if(m_volumeFadeOutTime && m_turnoffEndTime)
			{
				Float frac = (g_pGameVars->time - m_turnoffBeginTime)/m_volumeFadeOutTime;
				frac = clamp(frac, 0.0, 1.0);
				time = m_volumeFadeOutTime * (1.0 - frac);
				target = 0;
			}

			if(time > 0)
				gd_engfuncs.pfnApplySoundEffect(m_emitterEntity->GetEntityIndex(), gd_engfuncs.pfnGetString(m_pFields->message), SND_CHAN_AUTO, SND_EF_CHANGE_VOLUME, time, target/100.0f, pPlayer ? pPlayer->GetClientIndex() : NO_CLIENT_INDEX);
		}
	}
}

//=============================================
// @brief
//
//=============================================
void CAmbientGeneric::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	if(!m_isLooping)
	{
		// Just play the non-looping sound
		PlaySound();
		return;
	}
	else
	{
		// Remember previous state
		bool prevstate = m_isActive;
		switch(useMode)
		{
		case USE_OFF:
			m_isActive = false;
			break;
		case USE_ON:
			m_isActive = true;
			break;
		case USE_TOGGLE:
		default:
			m_isActive = !m_isActive;
			break;
		}

		// Do nothing if state hasn't changed
		if(prevstate == m_isActive)
			return;

		// Play or stop the sound
		if(!m_isActive)
			StopSound();
		else
			PlaySound();
	}
}

//=============================================
// @brief
//
//=============================================
void CAmbientGeneric::PlaySound( void )
{
	// Determine starting pitch/volume
	Float volume = (m_volumeFadeInTime > 0) ? m_startVolume : m_volume;
	Int32 pitch = (m_pitchFadeInTime > 0) ? m_startPitch : m_pitch;

	// Set play begin time
	m_beginTime = g_pGameVars->time;

	if(HasSpawnFlag(FL_MUTE_OTHERS))
		gd_engfuncs.pfnSetMuteAllSounds(true);

	// Send to client to play
	if(m_emitterEntity != reinterpret_cast<const CBaseEntity*>(this) && m_emitterEntity->IsVisible())
		Util::EmitEntitySound(m_emitterEntity, m_pFields->message, SND_CHAN_AUTO, volume/100.0f, m_attenuation, pitch, m_sndFlags);
	else
		Util::EmitAmbientSound(m_emitterEntity->GetOrigin(), m_pFields->message, volume/100.0f, m_attenuation, pitch, m_sndFlags, this, nullptr);

	// Apply any effects for turning on
	ApplyTurnOnEffects();
}

//=============================================
// @brief
//
//=============================================
void CAmbientGeneric::StopSound( void )
{
	// Get turn off time
	Float turnofftime;
	if(g_bInInitializeEntities)
	{
		// Do not set turnoff time if called from InitializeEntities
		turnofftime = 0;
	}
	else
	{
		if(m_volumeFadeOutTime && (!m_pitchFadeOutTime || m_pitchFadeOutTime > m_volumeFadeOutTime))
			turnofftime = m_volumeFadeOutTime;
		else if(m_pitchFadeOutTime && (!m_volumeFadeOutTime || m_volumeFadeOutTime > m_pitchFadeOutTime))
			turnofftime = m_pitchFadeOutTime;
		else if(m_volumeFadeOutTime == m_pitchFadeOutTime)
			turnofftime = m_volumeFadeOutTime;
		else
			turnofftime = 0;
	}

	if(!turnofftime)
	{
		// Just kill it if there's no turn-off time
		if(m_emitterEntity != reinterpret_cast<const CBaseEntity*>(this) && m_emitterEntity->IsVisible())
			Util::EmitEntitySound(m_emitterEntity, m_pFields->message, SND_CHAN_AUTO, 0, 0, 0, SND_FL_STOP);
		else
			Util::EmitAmbientSound(m_emitterEntity->GetOrigin(), m_pFields->message, 0, 0, 0, SND_FL_STOP, this);
	}
	else
	{
		// Set turnoff to shortest fade
		SetThink(&CAmbientGeneric::TurnOffThink);
		m_pState->nextthink = g_pGameVars->time + turnofftime;
		m_turnoffBeginTime = g_pGameVars->time;
		m_turnoffEndTime = g_pGameVars->time + turnofftime;

		ApplyTurnOffEffects();
	}
}

//=============================================
// @brief
//
//=============================================
void CAmbientGeneric::ApplyTurnOnEffects( void )
{
	if(m_turnoffBeginTime)
	{
		m_turnoffEndTime = 0;
		m_turnoffBeginTime = 0;
		SetThink(nullptr);
		m_pState->nextthink = 0;
	}

	if(m_volumeFadeInTime && m_startVolume != m_volume)
		gd_engfuncs.pfnApplySoundEffect(m_emitterEntity->GetEntityIndex(), gd_engfuncs.pfnGetString(m_pFields->message), SND_CHAN_AUTO, SND_EF_CHANGE_VOLUME, m_volumeFadeInTime, m_volume/100.0f, NO_CLIENT_INDEX);

	if(m_pitchFadeInTime && m_startPitch != m_pitch)
		gd_engfuncs.pfnApplySoundEffect(m_emitterEntity->GetEntityIndex(), gd_engfuncs.pfnGetString(m_pFields->message), SND_CHAN_AUTO, SND_EF_CHANGE_PITCH, m_pitchFadeInTime, m_pitch, NO_CLIENT_INDEX);
}

//=============================================
// @brief
//
//=============================================
void CAmbientGeneric::ApplyTurnOffEffects( void )
{
	if(m_volumeFadeOutTime)
		gd_engfuncs.pfnApplySoundEffect(m_emitterEntity->GetEntityIndex(), gd_engfuncs.pfnGetString(m_pFields->message), SND_CHAN_AUTO, SND_EF_CHANGE_VOLUME, m_volumeFadeOutTime, 0, NO_CLIENT_INDEX);

	if(m_pitchFadeOutTime)
		gd_engfuncs.pfnApplySoundEffect(m_emitterEntity->GetEntityIndex(), gd_engfuncs.pfnGetString(m_pFields->message), SND_CHAN_AUTO, SND_EF_CHANGE_PITCH, m_pitchFadeOutTime, 0, NO_CLIENT_INDEX);
}