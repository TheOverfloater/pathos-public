/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envlaser.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_laser, CEnvLaser);

//=============================================
// @brief
//
//=============================================
CEnvLaser::CEnvLaser( edict_t* pedict ):
	CBeam(pedict),
	m_spriteModelName(NO_STRING_VALUE),
	m_laserTarget(NO_STRING_VALUE)
{
}

//=============================================
// @brief
//
//=============================================
CEnvLaser::~CEnvLaser( void )
{
}

//=============================================
// @brief Spawns the entity
//
//=============================================
bool CEnvLaser::Spawn( void )
{
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
	m_pState->rendertype = RT_BEAM;
	m_pState->iuser2 |= FL_BEAM_NO_FADE;

	BeamInitPoints(m_pState->origin, m_pState->origin);

	if(!m_sprite && m_spriteModelName != NO_STRING_VALUE)
	{
		CEnvSprite* pSprite = CEnvSprite::CreateSprite(gd_engfuncs.pfnGetString(m_spriteModelName), m_pState->origin, true);
		pSprite->SetTransparency(RENDER_TRANSGLOW, m_pState->rendercolor.x, m_pState->rendercolor.y, m_pState->rendercolor.z, m_pState->renderamt, m_pState->renderfx);
		m_sprite = pSprite;
	}

	if(m_pFields->targetname != NO_STRING_VALUE && !HasSpawnFlag(FL_START_ON))
		TurnOff();
	else
		TurnOn();

	if(!m_pState->fuser1)
		m_pState->fuser1 = 10.0;

	return true;
}

//=============================================
// @brief Performs precache functions
//
//=============================================
void CEnvLaser::Precache( void )
{
	CBeam::Precache();

	if(m_spriteModelName != NO_STRING_VALUE)
		gd_engfuncs.pfnPrecacheModel(gd_engfuncs.pfnGetString(m_spriteModelName));
}

//=============================================
// @brief Calls for classes and their children
//
//=============================================
void CEnvLaser::DeclareSaveFields( void )
{
	CBeam::DeclareSaveFields();

	DeclareSaveField(DEFINE_DATA_FIELD(CEnvLaser, m_sprite, EFIELD_EHANDLE));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvLaser, m_spriteModelName, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvLaser, m_firePosition, EFIELD_COORD));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvLaser, m_laserTarget, EFIELD_STRING));
}

//=============================================
// @brief Manages keyvalues
//
//=============================================
bool CEnvLaser::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "LaserTarget"))
	{
		m_laserTarget = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "width"))
	{
		SetBeamWidth(SDL_atof(kv.value));
		return true;
	}
	else if(!qstrcmp(kv.keyname, "NoiseAmplitude"))
	{
		SetBeamAmplitude(SDL_atof(kv.value));
		return true;
	}
	else if(!qstrcmp(kv.keyname, "TextureScroll"))
	{
		SetBeamScrollRate(SDL_atof(kv.value));
		return true;
	}
	else if(!qstrcmp(kv.keyname, "NoiseSpeed"))
	{
		SetBeamNoiseSpeed(SDL_atof(kv.value));
		return true;
	}
	else if(!qstrcmp(kv.keyname, "EndSprite"))
	{
		m_spriteModelName = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "texture"))
	{
		m_pFields->modelname = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "framestart"))
	{
		m_pState->frame = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "damage"))
	{
		m_beamDamage = SDL_atof(kv.value);
		return true;
	}
	else 
		return CBeam::KeyValue(kv);
}

//=============================================
// @brief Calls use function
//
//=============================================
void CEnvLaser::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	bool isActive = IsOn();
	if(!ShouldToggle(useMode, isActive))
		return;

	if(isActive)
		TurnOff();
	else
		TurnOn();
}

//=============================================
// @brief Turns the laser on
//
//=============================================
void CEnvLaser::TurnOn( void )
{
	m_pState->effects &= ~EF_NODRAW;
	CBaseEntity* pSpriteEntity = m_sprite;
	if(pSpriteEntity)
	{
		CEnvSprite* pSprite = reinterpret_cast<CEnvSprite*>(pSpriteEntity);
		pSprite->TurnOn();
	}

	m_dmgTime = g_pGameVars->time;

	SetThink(&CEnvLaser::StrikeThink);
	m_pState->nextthink = g_pGameVars->time;
}

//=============================================
// @brief Turns the laser off
//
//=============================================
void CEnvLaser::TurnOff( void )
{
	CBaseEntity* pSpriteEntity = m_sprite;
	if(pSpriteEntity)
	{
		CEnvSprite* pSprite = reinterpret_cast<CEnvSprite*>(pSpriteEntity);
		pSprite->TurnOff();
	}

	m_pState->effects |= EF_NODRAW;
	m_pState->nextthink = 0;
	SetThink(nullptr);
}

//=============================================
// @brief Tells if the laser is on
//
//=============================================
bool CEnvLaser::IsOn( void )
{
	return (m_pState->effects & EF_NODRAW) ? false : true;
}

//=============================================
// @brief Fires the laser at a given point
//
//=============================================
void CEnvLaser::FireAtPoint( trace_t& tr )
{
	SetBeamEndPosition(tr.endpos);
	if(m_sprite)
	{
		CBaseEntity* pSpriteEntity = m_sprite;
		CEnvSprite* pSprite = reinterpret_cast<CEnvSprite*>(pSpriteEntity);
		pSprite->SetOrigin(tr.endpos);
	}

	BeamDamage(tr);
	BeamSparks(m_pState->origin, tr.endpos);
}

//=============================================
// @brief Called when striking
//
//=============================================
void CEnvLaser::StrikeThink( void )
{
	CBaseEntity* pEndEntity = GetRandomTargetName(gd_engfuncs.pfnGetString(m_laserTarget));
	if(pEndEntity)
		m_firePosition = pEndEntity->GetOrigin();

	trace_t tr;
	Util::TraceLine(m_pState->origin, m_firePosition, false, true, nullptr, tr);
	FireAtPoint(tr);

	SetThink(&CEnvLaser::StrikeThink);
	m_pState->nextthink = g_pGameVars->time + 0.1;
}