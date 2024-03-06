/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "sparkshower.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(spark_shower, CSparkShower);

//=============================================
// @brief
//
//=============================================
CSparkShower::CSparkShower( edict_t* pedict ):
	CBaseEntity(pedict)
{
}

//=============================================
// @brief
//
//=============================================
CSparkShower::~CSparkShower( void )
{
}

//=============================================
// @brief
//
//=============================================
void CSparkShower::Precache( void )
{
	gd_engfuncs.pfnPrecacheModel(NULL_SPRITE_FILENAME);
}

//=============================================
// @brief
//
//=============================================
bool CSparkShower::Spawn( void )
{
	if(!CBaseEntity::Spawn())
		return false;

	// Set empty sprite texture
	if(!SetModel(NULL_SPRITE_FILENAME, false))
		return false;

	Vector forward;
	Math::AngleVectors(m_pState->angles, &forward);

	m_pState->velocity = Common::RandomFloat(200, 300)*forward;

	for(Uint32 i = 0; i < 2; i++)
		m_pState->velocity[i] += Common::RandomFloat(-100, 100);

	if(m_pState->velocity[2] >= 0)
		m_pState->velocity[2] += 200;
	else
		m_pState->velocity[2] -= 200;

	m_pState->solid = SOLID_NOT;
	m_pState->movetype = MOVETYPE_BOUNCE;
	m_pState->gravity = 0.5;
	m_pState->effects |= EF_NODRAW;
	m_pState->speed = Common::RandomFloat(0.5, 1.5);
	m_pState->angles.Clear();

	SetThink(&CSparkShower::SparkThink);
	m_pState->nextthink = g_pGameVars->time + 0.1;

	gd_engfuncs.pfnSetMinsMaxs(m_pEdict, ZERO_VECTOR, ZERO_VECTOR);

	return true;
}

//=============================================
// @brief
//
//=============================================
void CSparkShower::CallTouch( CBaseEntity* pOther )
{
	Float velscale = (m_pState->flags & FL_ONGROUND) ? 0.1 : 0.6;
	Math::VectorScale(m_pState->velocity, velscale, m_pState->velocity);

	if((m_pState->velocity[0]*m_pState->velocity[0]+m_pState->velocity[1]*m_pState->velocity[1]) < 10)
		m_pState->speed = 0;
}

//=============================================
// @brief
//
//=============================================
void CSparkShower::SparkThink( void )
{
	Util::CreateSparks(m_pState->origin);

	m_pState->flags &= ~FL_ONGROUND;

	m_pState->speed -= 0.1;
	if(m_pState->speed <= 0)
		SetThink(&CBaseEntity::RemoveThink);
		
	m_pState->nextthink = g_pGameVars->time + 0.1;
}