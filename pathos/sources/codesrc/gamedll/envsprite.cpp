/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envsprite.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_sprite, CEnvSprite);

//=============================================
// @brief
//
//=============================================
CEnvSprite::CEnvSprite( edict_t* pedict ):
	CBaseEntity(pedict),
	m_lastAnimTime(0),
	m_dieTime(0),
	m_maxFrame(0),
	m_angularFOV(0),
	m_attachEntity(NO_STRING_VALUE),
	m_attachmentIndex(0)
{
}

//=============================================
// @brief
//
//=============================================
CEnvSprite::~CEnvSprite( void )
{
}

//=============================================
// @brief
//
//=============================================
void CEnvSprite::Precache( void )
{
	Int32 modelindex = gd_engfuncs.pfnPrecacheModel(gd_engfuncs.pfnGetString(m_pFields->modelname));
	if(modelindex != NO_PRECACHE)
		m_maxFrame = gd_engfuncs.pfnGetModelFrameCount(modelindex) - 1;
}

//=============================================
// @brief
//
//=============================================
bool CEnvSprite::Spawn( void )
{
	// Remove immediately if it has no targetname
	// Means it's completely static
	if(!HasSpawnFlag(FL_KEEP_SERVER) && m_pFields->targetname == NO_STRING_VALUE)
	{
		Util::RemoveEntity(this);
		return true;
	}

	// Check for model
	if(m_pFields->modelname == NO_STRING_VALUE)
	{
		Util::WarnEmptyEntity(m_pEdict);
		return false;
	}

	if(!CBaseEntity::Spawn())
		return false;

	if(!SetModel(m_pFields->modelname, false))
		return false;

	m_pState->solid = SOLID_NOT;
	m_pState->movetype = MOVETYPE_NONE;

	m_pState->effects = EF_NONE;
	m_pState->frame = 0;

	if(m_pFields->targetname != NO_STRING_VALUE && !HasSpawnFlag(FL_START_ON))
		TurnOff();
	else
		TurnOn();

	if(!HasSpawnFlag(FL_KEEP_ANGLES) && m_pState->renderfx != RenderFx_AngularSprite)
	{
		if(m_pState->angles[1] != m_pState->angles[2])
		{
			m_pState->angles[2] = m_pState->angles[1];
			m_pState->angles[1] = 0;
		}
	}

	if(m_attachEntity != NO_STRING_VALUE)
		m_pState->flags |= FL_INITIALIZE;

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CEnvSprite::Restore( void )
{
	if(!CBaseEntity::Restore())
		return false;

	if(m_pState->aiment != NO_ENTITY_INDEX)
	{
		CBaseEntity* pAiment = GetAiment();
		if(pAiment)
			SetAttachment(pAiment, m_pState->body);
	}

	if(m_pState->aiment == NO_ENTITY_INDEX)
	{
		m_pState->skin = 0;
		m_pState->body = 0;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CEnvSprite::DeclareSaveFields( void )
{
	CBaseEntity::DeclareSaveFields();

	DeclareSaveField(DEFINE_DATA_FIELD(CEnvSprite, m_lastAnimTime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvSprite, m_dieTime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvSprite, m_angularFOV, EFIELD_UINT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvSprite, m_attachEntity, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvSprite, m_attachmentIndex, EFIELD_UINT32));
}

//=============================================
// @brief
//
//=============================================
bool CEnvSprite::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "ang_fov"))
	{
		m_angularFOV = SDL_atoi(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "attachentity"))
	{
		m_attachEntity = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "attachment"))
	{
		m_attachmentIndex = SDL_atoi(kv.value);
		return true;
	}
	else 
		return CBaseEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
void CEnvSprite::InitEntity( void )
{
	if(m_attachEntity == NO_STRING_VALUE)
		return;

	const Char* pstrEntityName = gd_engfuncs.pfnGetString(m_attachEntity);
	edict_t* pedict = Util::FindEntityByTargetName(nullptr, pstrEntityName);
	if(!pedict || Util::IsNullEntity(pedict))
	{
		gd_engfuncs.pfnCon_Printf("%s - Couldn't find attachment entity '%s'.\n", __FUNCTION__, pstrEntityName);
		Util::RemoveEntity(this);
		return;
	}

	m_pState->aiment = pedict->entindex;
	m_pState->body = m_attachmentIndex;
	m_pState->movetype = MOVETYPE_FOLLOW;
}

//=============================================
// @brief
//
//=============================================
void CEnvSprite::PreAddPacket( void )
{
	m_pState->fuser1 = m_angularFOV;
}

//=============================================
// @brief
//
//=============================================
Int32 CEnvSprite::GetEntityFlags( void )
{
	Int32 flags = CBaseEntity::GetEntityFlags() & ~FL_ENTITY_TRANSITION;
	if(HasSpawnFlag(FL_TEMPORARY))
		flags |= FL_ENTITY_TRANSITION;

	return flags;
}

//=============================================
// @brief
//
//=============================================
void CEnvSprite::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	bool isOn = (m_pState->effects & EF_NODRAW) ? false : true;
	if(ShouldToggle(useMode, isOn))
	{
		if(isOn)
			TurnOff();
		else
			TurnOn();
	}
}

//=============================================
// @brief
//
//=============================================
void CEnvSprite::AnimateThink( void )
{
	Double delta = g_pGameVars->time - m_lastAnimTime;
	Animate(m_pState->framerate * delta);

	m_pState->nextthink = g_pGameVars->time + 0.1;
	m_lastAnimTime = g_pGameVars->time;
}

//=============================================
// @brief
//
//=============================================
void CEnvSprite::ExpandThink( void )
{
	Double delta = g_pGameVars->time - m_lastAnimTime;
	m_pState->scale += m_pState->speed * delta;
	m_pState->renderamt -= m_pState->health * delta;

	if(m_pState->renderamt <= 0)
	{
		m_pState->renderamt = 0;
		ClearThinkFunctions();

		Util::RemoveEntity(this);
	}
	else
	{
		m_pState->nextthink = g_pGameVars->time + 0.1;
		m_lastAnimTime = g_pGameVars->time;
	}
}

//=============================================
// @brief
//
//=============================================
void CEnvSprite::AnimateOnceThink( void )
{
	if(m_dieTime <= g_pGameVars->time)
	{
		Util::RemoveEntity(this);
		return;
	}
	
	// Run normal animation code
	AnimateThink();
	m_pState->nextthink = g_pGameVars->time + 0.1;
}

//=============================================
// @brief
//
//=============================================
void CEnvSprite::Animate( Float frameadd )
{
	m_pState->frame += frameadd;
	if(m_pState->frame > m_maxFrame)
	{
		if(HasSpawnFlag(FL_PLAY_ONCE))
		{
			// Just turn it off
			TurnOff();
		}
		else if(m_maxFrame > 0)
		{
			Int32 intval = SDL_floor(m_pState->frame/m_maxFrame);
			m_pState->frame -= (m_pState->frame - intval);
		}
	}
}

//=============================================
// @brief
//
//=============================================
void CEnvSprite::AnimateAndDie( Float framerate )
{
	SetThink(&CEnvSprite::AnimateOnceThink);
	m_pState->framerate = framerate;
	m_dieTime = g_pGameVars->time + (m_maxFrame / framerate);
	m_pState->nextthink = g_pGameVars->time + 0.1;
}

//=============================================
// @brief
//
//=============================================
void CEnvSprite::Expand( Float scalespeed, Float fadespeed )
{
	m_pState->speed = scalespeed;
	m_pState->health = fadespeed;

	SetThink(&CEnvSprite::ExpandThink);
	m_pState->nextthink = g_pGameVars->time + 0.1;
	m_lastAnimTime = g_pGameVars->time;
}

//=============================================
// @brief
//
//=============================================
void CEnvSprite::SpriteInit( const Char* pstrSpriteName, const Vector& origin )
{
	m_pFields->modelname = gd_engfuncs.pfnAllocString(pstrSpriteName);
	m_pState->origin = origin;
	m_pState->spawnflags |= FL_KEEP_SERVER;

	// Call spawn function
	Spawn();
}

//=============================================
// @brief
//
//=============================================
void CEnvSprite::SetAttachment( CBaseEntity* pEntity, Int32 attachment )
{
	if(!pEntity)
		return;

	m_pState->aiment = pEntity->GetEntityIndex();
	m_pState->body = attachment + 1;
	m_pState->movetype = MOVETYPE_FOLLOW;
}

//=============================================
// @brief
//
//=============================================
void CEnvSprite::SetTransparency( rendermode_t rendermode, Int32 r, Int32 g, Int32 b, Int32 alpha, Int32 renderfx )
{
	m_pState->rendermode = rendermode;
	m_pState->rendercolor.x = r;
	m_pState->rendercolor.y = g;
	m_pState->rendercolor.z = b;
	m_pState->renderamt = alpha;
	m_pState->renderfx = renderfx;
}

//=============================================
// @brief
//
//=============================================
void CEnvSprite::SetTexture( Int32 spritemodel )
{
	const cache_model_t* pmodel = gd_engfuncs.pfnGetModel(spritemodel);
	if(!pmodel)
		return;

	SetModel(pmodel->name.c_str(), false);
}

//=============================================
// @brief
//
//=============================================
void CEnvSprite::TurnOff( void )
{
	m_pState->effects |= EF_NODRAW;
	m_pState->nextthink = 0;
}

//=============================================
// @brief
//
//=============================================
void CEnvSprite::TurnOn( void )
{
	m_pState->effects &= ~EF_NODRAW;
	m_pState->frame = 0;

	if((m_pState->framerate > 0 && m_maxFrame > 1) || HasSpawnFlag(FL_PLAY_ONCE))
	{
		SetThink(&CEnvSprite::AnimateThink);
		m_pState->nextthink = g_pGameVars->time + 0.1;
		m_lastAnimTime = g_pGameVars->time;
	}
}

//=============================================
// @brief
//
//=============================================
Float CEnvSprite::GetMaxFrame( void ) const
{
	return m_maxFrame;
}

//=============================================
// @brief
//
//=============================================
CEnvSprite* CEnvSprite::CreateSprite( const Char* pstrSpriteName, const Vector& origin, bool animate )
{
	CEnvSprite* pEntity = reinterpret_cast<CEnvSprite*>(CBaseEntity::CreateEntity("env_sprite", origin, ZERO_VECTOR, nullptr));
	pEntity->SpriteInit(pstrSpriteName, origin);

	pEntity->SetMoveType(MOVETYPE_NOCLIP);
	pEntity->SetSolidity(SOLID_NOT);

	if(animate)
		pEntity->TurnOn();

	return pEntity;
}