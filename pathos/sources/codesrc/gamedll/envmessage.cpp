/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envmessage.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_message, CEnvMessage);

//=============================================
// @brief
//
//=============================================
CEnvMessage::CEnvMessage( edict_t* pedict ):
	CPointEntity(pedict),
	m_msgSound(NO_STRING_VALUE),
	m_msgVolume(0),
	m_msgAttenuation(0)
{
}

//=============================================
// @brief
//
//=============================================
CEnvMessage::~CEnvMessage( void )
{
}

//=============================================
// @brief
//
//=============================================
void CEnvMessage::DeclareSaveFields( void )
{
	CPointEntity::DeclareSaveFields();

	DeclareSaveField(DEFINE_DATA_FIELD(CEnvMessage, m_msgSound, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvMessage, m_msgVolume, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvMessage, m_msgAttenuation, EFIELD_INT32));
}

//=============================================
// @brief
//
//=============================================
bool CEnvMessage::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "messagesound"))
	{
		m_msgSound = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "messagevolume"))
	{
		m_msgVolume = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "messageattenuation"))
	{
		Int32 atten = SDL_atoi(kv.value);
		switch(atten)
		{
		case ATTEN_MEDIUM:
			m_msgAttenuation = ATTN_STATIC;
			break;
		case ATTEN_LARGE:
			m_msgAttenuation = ATTN_NORM;
			break;
		case ATTEN_GLOBAL:
			m_msgAttenuation = ATTN_NONE;
			break;
		case ATTEN_SMALL:
		default:
			m_msgAttenuation = ATTN_IDLE;
			break;
		}
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
void CEnvMessage::Precache( void )
{
	if(m_msgSound != NO_STRING_VALUE)
		gd_engfuncs.pfnPrecacheSound(gd_engfuncs.pfnGetString(m_msgSound));
}

//=============================================
// @brief
//
//=============================================
bool CEnvMessage::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	// Make sure it's valid
	if(m_pFields->message == NO_STRING_VALUE)
	{
		Util::WarnEmptyEntity(m_pEdict);
		return false;
	}

	// Make sure volume is valid
	if(m_msgVolume <= 0)
		m_msgVolume = VOL_NORM;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CEnvMessage::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	if(HasSpawnFlag(FL_ALL_PLAYERS))
	{
		Util::ShowMessageAllPlayers(gd_engfuncs.pfnGetString(m_pFields->message));
	}
	else
	{
		CBaseEntity* pPlayer;
		if(pActivator && pActivator->IsPlayer())
			pPlayer = pActivator;
		else
			pPlayer = Util::GetHostPlayer();

		if(pPlayer)
			Util::ShowMessage(gd_engfuncs.pfnGetString(m_pFields->message), pPlayer);
	}	

	if(m_msgSound != NO_STRING_VALUE)
	{
		Util::EmitAmbientSound(m_pState->origin,
			m_msgSound,
			m_msgVolume, 
			m_msgAttenuation,
			PITCH_NORM,
			SND_FL_NONE,
			this);
	}

	if(HasSpawnFlag(FL_ONLY_ONCE))
	{
		Util::RemoveEntity(m_pEdict);
		return;
	}

	UseTargets(this, USE_TOGGLE, 0);
}