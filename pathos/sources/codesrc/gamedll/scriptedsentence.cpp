/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "scriptedsentence.h"
#include "ai_basenpc.h"
#include "sentencesfile.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(scripted_sentence, CScriptedSentence);

//=============================================
// @brief
//
//=============================================
CScriptedSentence::CScriptedSentence( edict_t* pedict ):
	CDelayEntity(pedict),
	m_sentenceName(NO_STRING_VALUE),
	m_npcName(NO_STRING_VALUE),
	m_listenerName(NO_STRING_VALUE),
	m_radius(0),
	m_repeatRate(0),
	m_attenuation(0),
	m_attenuationSetting(0),
	m_volume(0),
	m_isActive(false),
	m_duration(0),
	m_beginTime(0)
{
}

//=============================================
// @brief
//
//=============================================
CScriptedSentence::~CScriptedSentence( void )
{
}

//=============================================
// @brief
//
//=============================================
void CScriptedSentence::DeclareSaveFields( void )
{
	// Call base class to do it first
	CDelayEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CScriptedSentence, m_sentenceName, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CScriptedSentence, m_npcName, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CScriptedSentence, m_listenerName, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CScriptedSentence, m_radius, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CScriptedSentence, m_repeatRate, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CScriptedSentence, m_attenuation, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CScriptedSentence, m_volume, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CScriptedSentence, m_isActive, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CScriptedSentence, m_duration, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CScriptedSentence, m_beginTime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CScriptedSentence, m_targetEntity, EFIELD_EHANDLE));
	DeclareSaveField(DEFINE_DATA_FIELD(CScriptedSentence, m_listenerEntity, EFIELD_EHANDLE));
}

//=============================================
// @brief
//
//=============================================
bool CScriptedSentence::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "sentence"))
	{
		m_sentenceName = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "entity"))
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
	else if(!qstrcmp(kv.keyname, "listener"))
	{
		m_listenerName = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "duration"))
	{
		m_duration = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "radius"))
	{
		m_radius = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "refire"))
	{
		m_repeatRate = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "attenuation"))
	{
		m_attenuationSetting = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "volume"))
	{
		m_volume = SDL_atof(kv.value);
		return true;
	}
	else
		return CDelayEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CScriptedSentence::Spawn( void )
{
	if(!CDelayEntity::Spawn())
		return false;

	m_pState->solid = SOLID_NOT;
	m_pState->movetype = MOVETYPE_NONE;
	m_pState->effects |= EF_NODRAW;

	switch(m_attenuationSetting)
	{
		case RADIUS_MEDIUM:
			m_attenuation = ATTN_STATIC;
			break;
		case RADIUS_LARGE:
			m_attenuation = ATTN_NORM;
			break;
		case RADIUS_X_LARGE:
			m_attenuation = ATTN_LARGE;
			break;
		case RADIUS_NONE:
			m_attenuation = ATTN_NONE;
			break;
		case RADIUS_SMALL:
		default:
			m_attenuation = ATTN_IDLE;
			break;
	}

	if(m_volume <= 0)
		m_volume = VOL_NORM;

	// Set this to active by default
	m_isActive = true;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CScriptedSentence::Precache( void )
{
	if(g_pSentencesFile)
	{
		const Char* pstrSentence = gd_engfuncs.pfnGetString(m_sentenceName);
		if(pstrSentence)
		{
			const Char* pstrEntryName = g_pSentencesFile->GetSentence(pstrSentence, nullptr);
			if(pstrEntryName && qstrlen(pstrEntryName) > 0)
				gd_engfuncs.pfnPrecacheSound(pstrEntryName);
		}
	}

	// Call base class to precache
	CDelayEntity::Precache();
}

//=============================================
// @brief
//
//=============================================
void CScriptedSentence::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	if(!m_isActive)
		return;

	SetThink(&CScriptedSentence::FindThink);
	m_pState->nextthink = g_pGameVars->time;
}

//=============================================
// @brief
//
//=============================================
void CScriptedSentence::SendInitMessage( const CBaseEntity* pPlayer )
{
	if(!m_beginTime || m_beginTime + m_duration <= g_pGameVars->time || !m_targetEntity)
	{
		if(m_targetEntity)
			m_targetEntity.reset();

		if(m_beginTime)
			m_beginTime = 0;

		return;
	}

	bool subOnlyRadius = HasSpawnFlag(FL_SUB_ONLY_IN_RADIUS) ? true : false;
	bool isConcurrent = (!HasSpawnFlag(FL_CONCURRENT)) ? false : true;
	const Char* pstrSentenceName = gd_engfuncs.pfnGetString(m_sentenceName);
	Double timeOffset = g_pGameVars->time - m_beginTime;

	m_targetEntity->PlayScriptedSentence(pstrSentenceName, m_duration, m_volume / 10.0f, m_attenuation, timeOffset, subOnlyRadius, isConcurrent, m_listenerEntity);
}

//=============================================
// @brief
//
//=============================================
void CScriptedSentence::FindThink( void )
{
	CBaseEntity* pEntity = FindNPC();
	if(pEntity)
	{
		StartSentence(pEntity);
		if(HasSpawnFlag(FL_ONLY_ONCE))
			Util::RemoveEntity(this);

		SetThink(&CScriptedSentence::DelayThink);
		m_pState->nextthink = g_pGameVars->time + m_duration + m_repeatRate;
		m_isActive = false;
	}
	else
	{
		SetThink(&CScriptedSentence::FindThink);
		m_pState->nextthink = g_pGameVars->time + m_repeatRate + 0.5;
	}
}

//=============================================
// @brief
//
//=============================================
void CScriptedSentence::DelayThink( void )
{
	m_isActive = true;
	if(!HasTargetName())
		m_pState->nextthink = g_pGameVars->time + 0.1;

	SetThink(&CScriptedSentence::FindThink);
}

//=============================================
// @brief
//
//=============================================
CBaseEntity* CScriptedSentence::FindNPC( void )
{
	// Try finding by targetname first
	edict_t* pedict = nullptr;
	while(true)
	{
		pedict = Util::FindEntityByTargetName(pedict, gd_engfuncs.pfnGetString(m_npcName));
		if(!pedict)
			break;

		if(Util::IsNullEntity(pedict))
			continue;

		CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
		if(pEntity->IsNPC() && IsAcceptableSpeaker(pEntity))
			return pEntity;
	}

	// If not found, look for classname
	pedict = nullptr;
	while(true)
	{
		pedict = Util::FindEntityByClassname(pedict, gd_engfuncs.pfnGetString(m_npcName));
		if(!pedict)
			break;

		if(Util::IsNullEntity(pedict))
			continue;

		CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
		if(pEntity->IsNPC() && IsAcceptableSpeaker(pEntity))
			return pEntity;
	}

	return nullptr;
}

//=============================================
// @brief
//
//=============================================
bool CScriptedSentence::IsAcceptableSpeaker( CBaseEntity* pEntity ) const
{
	if(!pEntity)
		return false;

	if(HasSpawnFlag(FL_ONLY_FOLLOWERS))
	{
		CBaseEntity* pNPCFollowTarget = pEntity->GetTargetEntity();
		if(!pNPCFollowTarget || !pNPCFollowTarget->IsPlayer())
			return false;
	}

	bool canOverride = HasSpawnFlag(FL_INTERRUPT_SPEECH) ? true : false;
	return pEntity->CanPlaySentence(canOverride);
}

//=============================================
// @brief
//
//=============================================
bool CScriptedSentence::StartSentence( CBaseEntity* pTarget )
{
	if(!pTarget)
	{
		Util::EntityConPrintf(m_pEdict, "Entity '%s' not found.\n", gd_engfuncs.pfnGetString(m_npcName));
		return false;
	}
	
	if(m_listenerName != NO_STRING_VALUE)
	{
		const Char* pstrListenerName = gd_engfuncs.pfnGetString(m_listenerName);
		if(!qstrcmp(pstrListenerName, "player"))
		{
			// Listener is host player always
			m_listenerEntity = Util::GetHostPlayer();
		}
		else
		{
			edict_t* pedict = nullptr;
			while(true)
			{
				pedict = Util::FindEntityByTargetName(pedict, pstrListenerName);
				if(!pedict)
					break;

				if(Util::IsNullEntity(pedict))
					continue;

				m_listenerEntity = CBaseEntity::GetClass(pedict);
				break;
			}
		}
	}

	m_targetEntity = pTarget;
	m_beginTime = g_pGameVars->time;

	bool isConcurrent = HasSpawnFlag(FL_CONCURRENT) ? true : false;
	bool subOnlyRadius = HasSpawnFlag(FL_SUB_ONLY_IN_RADIUS) ? true : false;
	pTarget->PlayScriptedSentence(gd_engfuncs.pfnGetString(m_sentenceName), m_duration, m_volume / 10.0f, m_attenuation, 0, subOnlyRadius, isConcurrent, m_listenerEntity);
	return true;
}