/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "grenade.h"
#include "envexplosion.h"
#include "weapons_shared.h"
#include "ai_sounds.h"

// Model file path
const Char CGrenade::MODEL_FILENAME[] = "models/grenade.mdl";

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(grenade, CGrenade);

//=============================================
// @brief
//
//=============================================
CGrenade::CGrenade( edict_t* pedict ):
	CAnimatingEntity(pedict),
	m_nextDmgTime(0),
	m_explodeTime(0),
	m_emittedNPCSound(false),
	m_damageAmount(0),
	m_damageRadius(0)
{
}

//=============================================
// @brief
//
//=============================================
CGrenade::~CGrenade( void )
{
}

//=============================================
// @brief
//
//=============================================
void CGrenade::Precache( void )
{
	Util::PrecacheFixedNbSounds("weapons/grenade_impact%d.wav", 3);
	m_pFields->modelname = gd_engfuncs.pfnAllocString(MODEL_FILENAME);

	CAnimatingEntity::Precache();
}

//=============================================
// @brief
//
//=============================================
bool CGrenade::Spawn( void )
{
	if(!CAnimatingEntity::Spawn())
		return false;

	m_pState->movetype = MOVETYPE_BOUNCE;
	m_pState->solid = SOLID_BBOX;

	gd_engfuncs.pfnSetMinsMaxs(m_pEdict, ZERO_VECTOR, ZERO_VECTOR);

	return true;
}

//=============================================
// @brief
//
//=============================================
void CGrenade::BounceTouch( CBaseEntity* pOther )
{
	// Remove this object if it's stuck in the sky
	if(gd_tracefuncs.pfnPointContents(m_pState->origin, nullptr) == CONTENTS_SKY)
	{
		Util::RemoveEntity(this);
		return;
	}

	// Don't hit our spawner
	if(pOther == GetOwner())
		return;

	if(!m_explodeTime && m_explodeDelay)
		SetExplodeTime(m_explodeDelay);

	// Do damage to impacted entity if we have an owner
	if(m_pState->owner != NO_ENTITY_INDEX 
		&& m_nextDmgTime <= g_pGameVars->time 
		&& m_pState->velocity.Length() > 100)
	{
		CBaseEntity* pOwner = GetOwner();
		if(pOwner)
		{
			if(!g_pGameVars->globaltrace.noHit() 
				&& g_pGameVars->globaltrace.hitentity != NO_ENTITY_INDEX 
				&& g_pGameVars->globaltrace.hitentity != WORLDSPAWN_ENTITY_INDEX)
			{
				Vector direction = m_pState->velocity;
				direction.Normalize();

				gMultiDamage.Clear();
				pOther->TraceAttack(pOwner, 1, direction, g_pGameVars->globaltrace, DMG_SLASH);
				gMultiDamage.ApplyDamage(this, m_attacker, 0);
			}

			m_nextDmgTime = g_pGameVars->time + 1.0;
		}
	}

	Vector testVelocity = m_pState->velocity * 0.5;
	if(!m_emittedNPCSound && testVelocity.Length() <= 60)
	{
		gAISounds.AddSound(this, AI_SOUND_DANGER, m_damageRadius, VOL_NORM, 0.3);
		m_emittedNPCSound = true;
	}

	if(m_pState->flags & FL_ONGROUND)
	{
		// Reduce velocity by 20% as we're sliding
		Math::VectorScale(m_pState->velocity, 0.8, m_pState->velocity);
	}
	else
	{
		// Play bounce sound
		BounceSound();
	}
}

//=============================================
// @brief
//
//=============================================
void CGrenade::SlideTouch( CBaseEntity* pOther )
{
	// Remove this object if it's stuck in the sky
	if(gd_tracefuncs.pfnPointContents(m_pState->origin, nullptr) == CONTENTS_SKY)
	{
		Util::RemoveEntity(this);
		return;
	}

	// Don't hit our spawner
	if(pOther == GetOwner())
		return;

	if(m_pState->flags & FL_ONGROUND)
	{
		// Reduce velocity by 5% as we're sliding
		Math::VectorScale(m_pState->velocity, 0.8, m_pState->velocity);
	}
	else
	{
		// Play bounce sound
		BounceSound();
	}
}

//=============================================
// @brief
//
//=============================================
void CGrenade::BounceSound( void )
{
	Util::PlayRandomEntitySound(this, "weapons/grenade_impact%d.wav", 3, SND_CHAN_VOICE, 0.25);
}

//=============================================
// @brief
//
//=============================================
void CGrenade::ExplodeTouch( CBaseEntity* pOther )
{
	// Don't explode if hitting skybox
	if(gd_tracefuncs.pfnPointContents(m_pState->origin, nullptr) == CONTENTS_SKY)
	{
		Util::RemoveEntity(this);
		return;
	}

	Explode();
}

//=============================================
// @brief
//
//=============================================
void CGrenade::Explode( void )
{
	// Create the explosion
	CEnvExplosion::CreateEnvExplosion(m_pState->origin, m_pState->angles, 
		m_damageRadius, m_damageAmount, true, m_attacker, this);

	// Disable visibility and collisions
	m_pState->effects |= EF_NODRAW;
	m_pState->movetype = MOVETYPE_NONE;
	m_pState->solid = SOLID_NOT;

	SetThink(&CBaseEntity::RemoveThink);
	m_pState->nextthink = g_pGameVars->time + 1.0;
}

//=============================================
// @brief
//
//=============================================
void CGrenade::DangerSoundThink( void )
{
	gAISounds.AddSound(this, AI_SOUND_DANGER, m_damageRadius, VOL_NORM, 0.3);
	m_pState->nextthink = g_pGameVars->time + 0.2;

	if(m_pState->waterlevel > WATERLEVEL_NONE)
		m_pState->velocity = m_pState->velocity * 0.5;
}

//=============================================
// @brief
//
//=============================================
void CGrenade::TumbleThink( void )
{
	// Remove this object if it's stuck in the sky
	if(gd_tracefuncs.pfnPointContents(m_pState->origin, nullptr) == CONTENTS_SKY)
	{
		Util::RemoveEntity(this);
		return;
	}

	m_pState->nextthink = g_pGameVars->time + 0.1;

	if(!m_explodeTime || (m_explodeTime - 1) < g_pGameVars->time)
		gAISounds.AddSound(this, AI_SOUND_DANGER, m_damageRadius, VOL_NORM, 0.1);

	if(m_explodeTime && m_explodeTime <= g_pGameVars->time)
		SetThink(&CGrenade::Explode);
	else
		SetThink(&CGrenade::TumbleThink);
}

//=============================================
// @brief
//
//=============================================
void CGrenade::SetExplodeTime( Float explodeTime )
{
	m_explodeTime = g_pGameVars->time + explodeTime;
}

//=============================================
// @brief
//
//=============================================
void CGrenade::SetExplodeDelay( Float delay )
{
	m_explodeDelay = delay;
}

//=============================================
// @brief
//
//=============================================
void CGrenade::SetDamageAmount( Float dmgAmount )
{
	m_damageAmount = dmgAmount;
}

//=============================================
// @brief
//
//=============================================
void CGrenade::SetDamageRadius( Float dmgRadius )
{
	m_damageRadius = dmgRadius;
}

//=============================================
// @brief
//
//=============================================
void CGrenade::SetDamageTime( Float dmgtime )
{
	m_nextDmgTime = g_pGameVars->time + dmgtime;
}

//=============================================
// @brief
//
//=============================================
void CGrenade::SetAttacker( CBaseEntity* pAttacker )
{
	m_attacker = pAttacker;
}

//=============================================
// @brief
//
//=============================================
CGrenade* CGrenade::CreateTimed( CBaseEntity* pOwner, const Vector& origin, const Vector& velocity, Float time, Float radius, Float damage, bool contactDelayCountdown )
{
	CGrenade* pGrenade = reinterpret_cast<CGrenade*>(CBaseEntity::CreateEntity("grenade", pOwner));
	if(!pGrenade->Spawn())
	{
		Util::RemoveEntity(pGrenade);
		return nullptr;
	}

	pGrenade->SetOrigin(origin);
	pGrenade->SetVelocity(velocity);

	Vector angles = Math::VectorToAngles(velocity);
	pGrenade->SetAngles(angles);

	pGrenade->SetOwner(pOwner);
	pGrenade->SetAttacker(pOwner);
	pGrenade->SetTouch(&CGrenade::BounceTouch);

	if(!contactDelayCountdown)
		pGrenade->SetExplodeTime(time);
	else
		pGrenade->SetExplodeDelay(time);

	pGrenade->SetThink(&CGrenade::TumbleThink);
	
	if(time < 0.1)
	{
		// Think immediately if it's too low
		pGrenade->SetNextThinkTime(g_pGameVars->time);
		pGrenade->SetVelocity(ZERO_VECTOR);
	}
	else
	{
		// Set next think to 0.1s
		pGrenade->SetNextThink(0.1);
	}

	pGrenade->SetModel(W_OBJECTS_MODEL_FILENAME, false);

	pGrenade->SetDamageAmount(damage);
	pGrenade->SetDamageRadius(radius);

	pGrenade->SetGravity(0.5);
	pGrenade->SetFriction(0.8);

	pGrenade->SetBodyGroup(WMODEL_BODY_BASE, WMODEL_GRENADE_PRIMED);

	return pGrenade;
}

//=============================================
// @brief
//
//=============================================
CGrenade* CGrenade::CreateContact( CBaseEntity* pOwner, const Vector& origin, const Vector& velocity, Float radius, Float damage )
{
	CGrenade* pGrenade = reinterpret_cast<CGrenade*>(CBaseEntity::CreateEntity("grenade", pOwner));
	if(!pGrenade->Spawn())
	{
		Util::RemoveEntity(pGrenade);
		return nullptr;
	}

	pGrenade->SetGravity(0.5);
	pGrenade->SetOrigin(origin);
	pGrenade->SetVelocity(velocity);
	pGrenade->SetAttacker(pOwner);

	Vector angles = Math::VectorToAngles(velocity);
	pGrenade->SetAngles(angles);

	pGrenade->SetThink(&CGrenade::DangerSoundThink);
	pGrenade->SetNextThink(g_pGameVars->time);

	Vector avelocity = pGrenade->GetAngularVelocity();
	avelocity.z = Common::RandomFloat(-100, -500);
	pGrenade->SetAngularVelocity(avelocity);

	pGrenade->SetTouch(&CGrenade::ExplodeTouch);

	pGrenade->SetDamageAmount(damage);
	pGrenade->SetDamageRadius(radius);

	return pGrenade;
}