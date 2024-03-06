/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envbeamfollow.h"
#include "cl_entity.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_beamfollow, CEnvBeamFollow);

//=============================================
// @brief
//
//=============================================
CEnvBeamFollow::CEnvBeamFollow( edict_t* pedict ):
	CPointEntity(pedict),
	m_spriteModelName(NO_STRING_VALUE),
	m_life(0),
	m_width(0),
	m_beamNoise(0),
	m_attachment(NO_POSITION)
{
}

//=============================================
// @brief
//
//=============================================
CEnvBeamFollow::~CEnvBeamFollow( void )
{
}

//=============================================
// @brief Spawns the entity
//
//=============================================
bool CEnvBeamFollow::Spawn( void )
{
	// Check for model
	if(m_spriteModelName == NO_STRING_VALUE)
	{
		Util::WarnEmptyEntity(m_pEdict);
		return false;
	}

	if(!CPointEntity::Spawn())
		return false;

	if(m_pFields->target == NO_STRING_VALUE
		|| m_pFields->targetname == NO_STRING_VALUE)
	{
		Util::WarnEmptyEntity(m_pEdict);
		Util::RemoveEntity(this);
		return false;
	}

	if(m_attachment != NO_POSITION && m_attachment < 0 || m_attachment >= MAX_ATTACHMENTS)
	{
		Util::EntityConPrintf(m_pEdict, "Attachment index %d is invalid.\n", m_attachment);
		Util::RemoveEntity(this);
		return false;
	}

	return true;
}

//=============================================
// @brief Performs precache functions
//
//=============================================
void CEnvBeamFollow::Precache( void )
{
	CPointEntity::Precache();

	if(m_spriteModelName != NO_STRING_VALUE)
		gd_engfuncs.pfnPrecacheModel(gd_engfuncs.pfnGetString(m_spriteModelName));
}

//=============================================
// @brief Calls for classes and their children
//
//=============================================
void CEnvBeamFollow::DeclareSaveFields( void )
{
	CPointEntity::DeclareSaveFields();

	DeclareSaveField(DEFINE_DATA_FIELD(CEnvBeamFollow, m_spriteModelName, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvBeamFollow, m_life, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvBeamFollow, m_width, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvBeamFollow, m_beamNoise, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD_FLAGS(CEnvBeamFollow, m_attachment, EFIELD_INT32, EFIELD_SAVE_ALWAYS));
}

//=============================================
// @brief Manages keyvalues
//
//=============================================
bool CEnvBeamFollow::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "life"))
	{
		m_life = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "width"))
	{
		m_width = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "beamnoise"))
	{
		m_beamNoise = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "texture"))
	{
		m_spriteModelName = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "attachment"))
	{
		m_attachment = SDL_atoi(kv.value);
		return true;
	}
	else 
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief Calls use function
//
//=============================================
void CEnvBeamFollow::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	if(m_pFields->target == NO_STRING_VALUE)
		return;

	const Char* pstrTarget = gd_engfuncs.pfnGetString(m_pFields->target);
	edict_t* pEdict = Util::FindEntityByTargetName(nullptr, pstrTarget);
	if(!pEdict)
	{
		Util::EntityConPrintf(m_pEdict, "Cannot find target '%s'.\n", pstrTarget);
		return;
	}

	CBaseEntity* pEntity = CBaseEntity::GetClass(pEdict);
	if(!pEntity)
	{
		Util::EntityConPrintf(m_pEdict, "Target '%s' is not a valid entity.\n", pstrTarget);
		return;
	}

	if(!pEntity->HasModelName())
	{
		Util::EntityConPrintf(m_pEdict, "Target '%s' is not an entity with a valid model.\n", pstrTarget);
		return;
	}

	if(pEntity->GetEffectFlags() & EF_NODRAW)
	{
		Util::EntityConPrintf(m_pEdict, "Target '%s' has effect flag EF_NODRAW.\n", pstrTarget);
		return;
	}

	Util::CreateBeamFollow(pEntity, m_attachment, gd_engfuncs.pfnGetString(m_spriteModelName), 
		m_life, m_width, m_beamNoise, m_pState->renderamt/255.0f, 
		m_pState->rendercolor.x, m_pState->rendercolor.y, m_pState->rendercolor.z);
}



