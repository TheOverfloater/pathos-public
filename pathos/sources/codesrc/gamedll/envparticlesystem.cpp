/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envparticlesystem.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_particle_system, CEnvParticleSystem);

//=============================================
// @brief
//
//=============================================
CEnvParticleSystem::CEnvParticleSystem( edict_t* pedict ):
	CPointEntity(pedict),
	m_isActive(false),
	m_wasSent(false)
{
}

//=============================================
// @brief
//
//=============================================
CEnvParticleSystem::~CEnvParticleSystem( void )
{
}

//=============================================
// @brief
//
//=============================================
void CEnvParticleSystem::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvParticleSystem, m_isActive, EFIELD_BOOLEAN));
}

//=============================================
// @brief
//
//=============================================
bool CEnvParticleSystem::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	if(m_pFields->targetname == NO_STRING_VALUE || HasSpawnFlag(FL_START_ON))
		m_isActive = true;

	// Warn about empty particles
	if(m_pFields->message == NO_STRING_VALUE)
	{
		Util::WarnEmptyEntity(m_pEdict);
		Util::RemoveEntity(this);
		return true;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CEnvParticleSystem::Precache( void )
{
	gd_engfuncs.pfnPrecacheParticleScript(gd_engfuncs.pfnGetString(m_pFields->message), (part_script_type_t)m_pState->frags);
}

//=============================================
// @brief
//
//=============================================
bool CEnvParticleSystem::Restore( void )
{
	// Reset this
	m_wasSent = false;
	return true;
}

//=============================================
// @brief
//
//=============================================
void CEnvParticleSystem::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	bool prevstate = m_isActive;
	if(!HasSpawnFlag(FL_NOT_TOGGLED))
	{
		switch(useMode)
		{
		case USE_ON:
			m_isActive = true;
			break;
		case USE_OFF:
			m_isActive = false;
			break;
		case USE_TOGGLE:
			m_isActive = !m_isActive;
			break;
		}
	}
	else if(!m_isActive)
	{
		// Just set to true
		m_isActive = true;
	}

	if(prevstate != m_isActive)
	{
		m_wasSent = false;
		SendInitMessage(nullptr);
	}
}

//=============================================
// @brief
//
//=============================================
void CEnvParticleSystem::SendInitMessage( const CBaseEntity* pPlayer )
{
	if(!m_isActive && pPlayer)
		return;
	else if(m_isActive && m_wasSent)
		return;
	
	entindex_t targetEntityIndex = GetTargetEntityIndex();

	if(!m_isActive)
	{
		if(pPlayer)
			gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.removeparticlesystem, nullptr, pPlayer->GetEdict());
		else
			gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.removeparticlesystem, nullptr, nullptr);

		gd_engfuncs.pfnMsgWriteInt32(GetEntityIndex());
		gd_engfuncs.pfnMsgWriteInt32(targetEntityIndex);
		gd_engfuncs.pfnMsgWriteByte(TRUE);
		gd_engfuncs.pfnUserMessageEnd();
	}
	else
	{
		Vector forward;
		Math::AngleVectors(m_pState->angles, &forward, nullptr, nullptr);

		if(pPlayer)
			gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.createparticlesystem, nullptr, pPlayer->GetEdict());
		else
			gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.createparticlesystem, nullptr, nullptr);

		for(Uint32 i = 0; i < 3; i++)
			gd_engfuncs.pfnMsgWriteFloat(m_pState->origin[i]);

		for(Uint32 i = 0; i < 3; i++)
			gd_engfuncs.pfnMsgWriteSmallFloat(forward[i]*360.0f);

		Int32 attachmentIndex;
		Int32 attachmentFlags;
		if(targetEntityIndex == GetEntityIndex())
		{
			attachmentIndex = 0;
			attachmentFlags = PARTICLE_ATTACH_NONE;
		}
		else
		{
			attachmentIndex = 0;
			attachmentFlags = PARTICLE_ATTACH_TO_PARENT;

			switch(m_pState->skin)
			{
			case ATTACHMODE_ENTITY_ORIGIN:
				break;
			case ATTACHMODE_MODEL_ATTACHMENT:
				attachmentIndex = m_pState->body;
				attachmentFlags |= PARTICLE_ATTACH_TO_ATTACHMENT;
				break;
			case ATTACHMODE_VECTOR_FROM_ATTACHMENTS:
				attachmentIndex = m_pState->body;
				attachmentFlags |= PARTICLE_ATTACH_ATTACHMENT_VECTOR;
				break;
			}
		}

		gd_engfuncs.pfnMsgWriteByte(m_pState->frags);
		gd_engfuncs.pfnMsgWriteString(gd_engfuncs.pfnGetString(m_pFields->message));
		gd_engfuncs.pfnMsgWriteInt32(GetEntityIndex());
		gd_engfuncs.pfnMsgWriteInt32(targetEntityIndex);
		gd_engfuncs.pfnMsgWriteByte(attachmentIndex);
		gd_engfuncs.pfnMsgWriteInt16(NO_POSITION);
		gd_engfuncs.pfnMsgWriteByte(attachmentFlags);
		gd_engfuncs.pfnUserMessageEnd();
	}

	m_wasSent = true;

	if(m_isActive && HasSpawnFlag(FL_REMOVE_ON_FIRE))
		Util::RemoveEntity(this);
}

//=============================================
// @brief
//
//=============================================
entindex_t CEnvParticleSystem::GetTargetEntityIndex( void )
{
	if(m_pFields->target == NO_STRING_VALUE)
		return GetEntityIndex();

	const Char* pstrEntityName = gd_engfuncs.pfnGetString(m_pFields->target);
	edict_t* pedict = Util::FindEntityByTargetName(nullptr, pstrEntityName);
	if(!pedict)
	{
		Util::EntityConPrintf(m_pEdict, "Particle system target entity '%s' not found.\n", pstrEntityName);
		return GetEntityIndex();
	}

	if(!pedict->state.modelindex)
	{
		Util::EntityConPrintf(m_pEdict, "Particle system target entity '%s' has no model set.\n", pstrEntityName);
		return GetEntityIndex();
	}

	return pedict->entindex;
}