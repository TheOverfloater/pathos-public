/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "ambientoggstream.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(ambient_fmodstream, CAmbientOggStream);
LINK_ENTITY_TO_CLASS(ambient_oggstream, CAmbientOggStream);

//=============================================
// @brief
//
//=============================================
CAmbientOggStream::CAmbientOggStream( edict_t* pedict ):
	CPointEntity(pedict),
	m_isActive(false),
	m_fadeInTime(0),
	m_fadeOutTime(0),
	m_channel(MUSIC_CHANNEL_0)
{
}

//=============================================
// @brief
//
//=============================================
CAmbientOggStream::~CAmbientOggStream( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CAmbientOggStream::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	if(m_channel < 0 || m_channel >= NB_MUSIC_CHANNELS)
	{
		Util::EntityConPrintf(m_pEdict, "Channel setting '%d' is not a valid channel.\n", m_channel);
		return false;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CAmbientOggStream::Precache( void )
{
	gd_engfuncs.pfnPrecacheSound(gd_engfuncs.pfnGetString(m_pFields->message));
}

//=============================================
// @brief
//
//=============================================
void CAmbientOggStream::DeclareSaveFields( void )
{
	CPointEntity::DeclareSaveFields();

	DeclareSaveField(DEFINE_DATA_FIELD(CAmbientOggStream, m_isActive, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CAmbientOggStream, m_fadeInTime, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CAmbientOggStream, m_fadeOutTime, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CAmbientOggStream, m_channel, EFIELD_INT32));
}

//=============================================
// @brief
//
//=============================================
bool CAmbientOggStream::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "fadetime"))
	{
		m_fadeOutTime = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "fadeintime"))
	{
		m_fadeInTime = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "channel"))
	{
		m_channel = SDL_atoi(kv.value);
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
void CAmbientOggStream::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	// Either use activator, or assume it's local player
	CBaseEntity* pEntity;
	if(pActivator && pActivator->IsPlayer())
		pEntity = pActivator;
	else
		pEntity = Util::GetHostPlayer();

	Int32 flags = OGG_FL_NONE;
	bool previousState = m_isActive;

	// Get path to file, it's always needed
	const Char* pstrFilepath = gd_engfuncs.pfnGetString(m_pFields->message);
	if(!pstrFilepath || !qstrlen(pstrFilepath))
	{
		// If no file is specified, just terminate any music playing
		flags |= OGG_FL_STOP;
	}
	else
	{
		switch(useMode)
		{
		case USE_ON:
			{
				if(HasSpawnFlag(FL_LOOP_MUSIC))
				{
					flags |= OGG_FL_LOOP;
					m_isActive = true;
				}
			}
			break;
		case USE_OFF:
			{
				flags |= OGG_FL_STOP;
				if(HasSpawnFlag(FL_LOOP_MUSIC))
					m_isActive = false;
			}
			break;
		case USE_TOGGLE:
			{
				if(HasSpawnFlag(FL_LOOP_MUSIC))
				{
					if(m_isActive)
					{
						flags |= OGG_FL_STOP;
						m_isActive = false;
					}
					else
					{
						flags |= OGG_FL_LOOP;
						m_isActive = true;
					}
				}
			}
			break;
		}
	}

	// Make sure we don't execute it unnecessarily
	if(HasSpawnFlag(FL_LOOP_MUSIC) && m_isActive == previousState)
		return;

	if(!(flags & OGG_FL_STOP))
	{
		// Play the music with fade if set, 
		pEntity->PlayMusic(pstrFilepath, m_channel, m_fadeInTime, flags);
	}
	else
	{
		// Stop the music, opportunistically with fade out
		pEntity->StopMusic(pstrFilepath, m_channel, m_fadeOutTime);
	}

	if(!HasSpawnFlag(FL_LOOP_MUSIC) && HasSpawnFlag(FL_REMOVE_ON_TRIGGER))
		Util::RemoveEntity(m_pEdict);
}