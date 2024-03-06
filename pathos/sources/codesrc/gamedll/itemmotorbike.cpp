/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "itemmotorbike.h"
#include "envdlight.h"
#include "gamedialouge.h"
#include "player.h"

// Trashed bike animation name
const Char CItemMotorBike::BIKE_ANIMATION_TRASHED_NAME[] = "trashed";
// Enter bike animation name
const Char CItemMotorBike::BIKE_ANIMATION_ENTER_NAME[] = "enter";
// Exit bike animation name
const Char CItemMotorBike::BIKE_ANIMATION_LEAVE_NAME[] = "exit";
// Motorbike model name
const Char CItemMotorBike::BIKE_MODELNAME[] = "models/motorbike.mdl";

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(item_motorbike, CItemMotorBike);

//=============================================
// @brief
//
//=============================================
CItemMotorBike::CItemMotorBike( edict_t* pedict ):
	CAnimatingEntity(pedict),
	m_isTrashed(false),
	m_pPlayer(nullptr)
{
}

//=============================================
// @brief
//
//=============================================
CItemMotorBike::~CItemMotorBike( void )
{
}

//=============================================
// @brief
//
//=============================================
void CItemMotorBike::DeclareSaveFields( void )
{
	// Call base class to do it first
	CAnimatingEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CItemMotorBike, m_isTrashed, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CItemMotorBike, m_pPlayer, EFIELD_ENTPOINTER));
}

//=============================================
// @brief
//
//=============================================
bool CItemMotorBike::Spawn( void )
{
	// Set modelname
	m_pFields->modelname = gd_engfuncs.pfnAllocString(BIKE_MODELNAME);

	// Takes care of precaching and setting the model
	if(!CAnimatingEntity::Spawn())
		return false;

	gd_engfuncs.pfnSetMinsMaxs(m_pEdict, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	m_pState->solid = SOLID_SLIDEBOX;
	m_pState->movetype = MOVETYPE_TOSS;

	m_pState->renderfx = RenderFx_GlowAura;
	m_pState->rendercolor = WEAPON_GLOW_COLOR;

	SetUse(&CItemMotorBike::UseBike);

	return true;
}

//=============================================
// @brief
//
//=============================================
void CItemMotorBike::Precache( void )
{
	CAnimatingEntity::Precache();

	gd_engfuncs.pfnPrecacheSound("bike/bike_accelerate.wav");
	gd_engfuncs.pfnPrecacheSound("bike/bike_bump1.wav");
	gd_engfuncs.pfnPrecacheSound("bike/bike_bump2.wav");
	gd_engfuncs.pfnPrecacheSound("bike/bike_crash1.wav");
	gd_engfuncs.pfnPrecacheSound("bike/bike_crash2.wav");
	gd_engfuncs.pfnPrecacheSound("bike/bike_crash3.wav");
	gd_engfuncs.pfnPrecacheSound("bike/bike_crash4.wav");
	gd_engfuncs.pfnPrecacheSound("bike/bike_accelerate_begin.wav");
	gd_engfuncs.pfnPrecacheSound("bike/bike_burning.wav");
	gd_engfuncs.pfnPrecacheSound("bike/bike_crash_fatal.wav");
	gd_engfuncs.pfnPrecacheSound("bike/bike_decelerate.wav");
	gd_engfuncs.pfnPrecacheSound("bike/bike_exit.wav");
	gd_engfuncs.pfnPrecacheSound("bike/bike_idle_loop.wav");
	gd_engfuncs.pfnPrecacheSound("bike/bike_skid.wav");
	gd_engfuncs.pfnPrecacheSound("bike/bike_start.wav");

	gd_engfuncs.pfnPrecacheParticleScript("torchfire.txt", PART_SCRIPT_SYSTEM);
	gd_engfuncs.pfnPrecacheParticleScript("torchsmoke.txt", PART_SCRIPT_SYSTEM);

	Util::PrecacheEntity("env_dlight");
}

//=============================================
// @brief
//
//=============================================
void CItemMotorBike::UseBike( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	if(m_isTrashed)
		return;

	if(pActivator && pActivator->IsPlayer() && pCaller && pCaller->IsPlayer())
	{
		PlayerEnter(pCaller);
	}
	else if(useMode == USE_OFF)
	{
		m_pState->sequence = FindSequence(BIKE_ANIMATION_TRASHED_NAME);
		if(m_pState->sequence == NO_SEQUENCE_VALUE)
			m_pState->sequence = 0;

		Util::EmitEntitySound( this, "bike/bike_burning.wav", SND_CHAN_ITEM);

		Vector origin = m_pState->origin+Vector(0, 0, 8);
		Util::CreateParticles("torchfire.txt", origin, Vector(0, 0, 1), PART_SCRIPT_SYSTEM);
		Util::CreateParticles("torchsmoke.txt", origin, Vector(0, 0, 1), PART_SCRIPT_SYSTEM);

		CEnvDLight::SpawnLight(m_pState->origin+Vector(0, 0, 16), Vector(250, 192, 20), 300);
		m_isTrashed = TRUE;
	}
}

//=============================================
// @brief
//
//=============================================
void CItemMotorBike::TraceAttack( CBaseEntity* pAttacker, Float damage, const Vector& direction, trace_t& tr, Int32 damageFlags )
{
	Util::Ricochet( tr.endpos, tr.plane.normal, true );
	Util::CreateVBMDecal( tr.endpos, tr.plane.normal, "shot_metal", m_pEdict, FL_DECAL_NORMAL_PERMISSIVE);
}

//=============================================
// @brief
//
//=============================================
void CItemMotorBike::PlayerEnter( CBaseEntity* pEntity )
{
	if(!pEntity || !pEntity->IsPlayer())
		return;

	m_pPlayer = pEntity;
	m_pPlayer->EnterBike(this);
	m_pState->solid = SOLID_NOT;

	SetUse( nullptr );
}

//=============================================
// @brief
//
//=============================================
void CItemMotorBike::PlayerLeave( void )
{
	m_pState->effects &= ~(EF_NODRAW|EF_ALWAYS_SEND);
	m_pState->solid = SOLID_BBOX;
	m_pState->movetype = MOVETYPE_TOSS;
	m_pState->aiment = m_pState->owner = NO_ENTITY_INDEX;

	gd_engfuncs.pfnSetOrigin(m_pEdict, m_pPlayer->GetOrigin()+Vector(0, 0, VEC_HULL_MIN[2]));

	// set glow
	m_pState->renderfx = RenderFx_GlowAura;

	SetUse( &CItemMotorBike::UseBike );
}

//=============================================
// @brief
//
//=============================================
void CItemMotorBike::SetFollow( void )
{
	m_pState->movetype = MOVETYPE_FOLLOW;
	m_pState->aiment = m_pPlayer->GetEntityIndex();
	m_pState->owner = m_pPlayer->GetEntityIndex();
	m_pState->effects |= (EF_NODRAW|EF_ALWAYS_SEND);

	// kill glow
	m_pState->renderfx = RenderFx_None;
}

//=============================================
// @brief
//
//=============================================
bool CItemMotorBike::Restore( void )
{
	if(!CAnimatingEntity::Restore())
		return false;

	if(m_isTrashed)
	{
		Util::EmitEntitySound( this, "bike/bike_burning.wav", SND_CHAN_ITEM);

		Vector origin = m_pState->origin+Vector(0, 0, 8);
		Util::CreateParticles("torchfire.txt", origin, Vector(0, 0, 1), PART_SCRIPT_SYSTEM);
		Util::CreateParticles("torchsmoke.txt", origin, Vector(0, 0, 1), PART_SCRIPT_SYSTEM);
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
Double CItemMotorBike::GetLeaveTime( void )
{
	return GetSequenceTime(BIKE_ANIMATION_LEAVE_NAME);
}

//=============================================
// @brief
//
//=============================================
Double CItemMotorBike::GetEnterTime( void )
{
	return GetSequenceTime(BIKE_ANIMATION_ENTER_NAME);
}