/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envexplosion.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_explosion, CEnvExplosion);

//=============================================
// @brief
//
//=============================================
CEnvExplosion::CEnvExplosion( edict_t* pedict ):
	CPointEntity(pedict),
	m_magnitude(0),
	m_dmgRadius(0),
	m_dmgAmount(0)
{
}

//=============================================
// @brief
//
//=============================================
CEnvExplosion::~CEnvExplosion( void )
{
}

//=============================================
// @brief
//
//=============================================
void CEnvExplosion::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvExplosion, m_magnitude, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvExplosion, m_dmgRadius, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvExplosion, m_dmgAmount, EFIELD_FLOAT));
}

//=============================================
// @brief
//
//=============================================
bool CEnvExplosion::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	// Set damage and radius based on Half-Life's values
	// if it wasn't set before
	if((!m_dmgRadius || !m_dmgAmount) && m_magnitude > 0)
	{
		m_dmgRadius = m_magnitude * 2.5;
		m_dmgAmount = m_magnitude;
	}

	// Make sure attacker is set
	if(!m_attacker)
		m_attacker = this;

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CEnvExplosion::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "iMagnitude"))
	{
		m_magnitude = SDL_atoi(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "radius"))
	{
		m_dmgRadius = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "dmg"))
	{
		m_dmgAmount = SDL_atof(kv.value);
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
void CEnvExplosion::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	trace_t tr;

	// Try to find hit position
	for(Uint32 i = 0; i < 3; i++)
	{
		// Get offset
		Vector offset(i == 2 ? 32 : 0,
			i == 1 ? 32 : 0,
			i == 0 ? 32 : 0);

		Vector tracePos1 = m_pState->origin + offset;
		Vector tracePos2 = m_pState->origin - offset;

		Util::TraceLine(tracePos1, tracePos2, true, false, nullptr, tr);
		if(!tr.allSolid() && !tr.startSolid() && !tr.noHit())
			break;

		Util::TraceLine(tracePos2, tracePos1, true, false, nullptr, tr);
		if(!tr.allSolid() && !tr.startSolid() && !tr.noHit())
			break;
	}

	if(!m_magnitude)
		m_magnitude = m_dmgAmount;

	Vector explosionDir;
	Vector explosionPosition;
	if(!tr.noHit())
	{
		// Get the position of the explosion
		explosionPosition = tr.endpos + (tr.plane.normal*(m_magnitude-24)*0.4);
		explosionDir = tr.plane.normal;

		if(!HasSpawnFlag(FL_NO_DECAL))
		{
			// Spawn decal
			Util::CreateGenericDecal(tr.endpos, &tr.plane.normal, "scorch", FL_DECAL_NONE);
		}
	}
	else
	{
		explosionPosition = m_pState->origin;
		Math::AngleVectors(m_pState->angles, &explosionDir);
	}

	// Create sound
	Util::ExplosionSound(m_pState->origin);

	// Create shake
	if(!HasSpawnFlag(FL_NO_SHAKE))
		Util::ScreenShake(m_pState->origin, 8, 3, 1, m_dmgRadius * 2, true);

	// Create dynlight if set
	if(!HasSpawnFlag(FL_NO_DYNLIGHT))
		Util::CreateDynamicLight(explosionPosition, Common::RandomFloat(300, 512), 255, 192, 30, 0.3, -100, 0, FL_DLIGHT_NOSHADOWS);

	// Do damage if set
	if(!HasSpawnFlag(FL_NO_DAMAGE))
	{
		CBaseEntity* pInflictor = m_inflictor ? m_inflictor : this;
		RadiusDamage(explosionPosition, pInflictor, m_attacker, m_dmgAmount, m_dmgRadius, CLASS_UNDEFINED, DMG_EXPLOSION);
	}

	// Spawn smoke if set
	if(!HasSpawnFlag(FL_NO_SMOKE))
	{
		SetThink(&CEnvExplosion::SmokeThink);
		m_pState->nextthink = g_pGameVars->time + 0.5;
	}
	else if(!HasSpawnFlag(FL_REPEATABLE))
	{
		SetThink(&CBaseEntity::RemoveThink);
		m_pState->nextthink = g_pGameVars->time + 0.1;
	}

	// Spawn fireball if needed
	if(!HasSpawnFlag(FL_NO_FIREBALL))
		Util::CreateParticles("explosion_cluster.txt", explosionPosition, explosionDir, PART_SCRIPT_CLUSTER);

	if(!HasSpawnFlag(FL_NO_SPARKS))
	{
		Uint32 numsparks = Common::RandomLong(0, 3);
		if(numsparks > 0)
		{
			// Get angles from normal(or from forward)
			Vector angles = Math::VectorToAngles(explosionDir);

			for(Uint32 i = 0; i < numsparks; i++)
			{
				CBaseEntity* pSpark = CBaseEntity::CreateEntity("spark_shower", m_pState->origin, angles, nullptr);
				if(!pSpark->Spawn())
				{
					Util::RemoveEntity(pSpark);
					break;
				}
			}
		}
	}
}

//=============================================
// @brief
//
//=============================================
void CEnvExplosion::SmokeThink( void )
{
	// Spawn smoke at origin
	Util::CreateParticles("engine_explosion_smoke.txt", m_pState->origin, ZERO_VECTOR, PART_SCRIPT_SYSTEM);

	// Remove entity if not repeatable
	if(!HasSpawnFlag(FL_REPEATABLE))
	{
		SetThink(&CBaseEntity::RemoveThink);
		m_pState->nextthink = g_pGameVars->time + 0.1;
	}
}

//=============================================
// @brief
//
//=============================================
void CEnvExplosion::SetMagnitude( Int32 magnitude )
{
	m_magnitude = magnitude;
}

//=============================================
// @brief
//
//=============================================
void CEnvExplosion::SetDamageAmount( Float dmgamount )
{
	m_dmgAmount = dmgamount;
}

//=============================================
// @brief
//
//=============================================
void CEnvExplosion::SetDamageRadius( Float radius )
{
	m_dmgRadius = radius;
}

//=============================================
// @brief
//
//=============================================
void CEnvExplosion::SetAttacker( CBaseEntity* pAttacker )
{
	m_attacker = pAttacker;
}

//=============================================
// @brief
//
//=============================================
void CEnvExplosion::SetInflictor( CBaseEntity* pInflictor )
{
	m_inflictor = pInflictor;
}

//=============================================
// @brief
//
//=============================================
void CEnvExplosion::CreateEnvExplosion( const Vector& origin, const Vector& angles, Int32 magnitude, bool dodamage, CBaseEntity* pAttacker, CBaseEntity* pInflictor )
{
	CEnvExplosion* pExplosion = reinterpret_cast<CEnvExplosion*>(CBaseEntity::CreateEntity("env_explosion", origin, angles, nullptr));
	if(!pExplosion)
		return;

	if(!dodamage)
		pExplosion->SetSpawnFlag(FL_NO_DAMAGE);

	pExplosion->SetMagnitude(magnitude);
	pExplosion->SetAttacker(pAttacker);
	pExplosion->SetInflictor(pInflictor);

	if(!pExplosion->Spawn())
	{
		Util::RemoveEntity(pExplosion);
		return;
	}

	// Do the explosion
	pExplosion->CallUse(nullptr, nullptr, USE_TOGGLE, 0);
}

//=============================================
// @brief
//
//=============================================
void CEnvExplosion::CreateEnvExplosion( const Vector& origin, const Vector& angles, Float radius, Float dmgamount, bool dodamage, CBaseEntity* pAttacker, CBaseEntity* pInflictor )
{
	CEnvExplosion* pExplosion = reinterpret_cast<CEnvExplosion*>(CBaseEntity::CreateEntity("env_explosion", origin, angles, nullptr));
	if(!pExplosion)
		return;

	if(!dodamage)
		pExplosion->SetSpawnFlag(FL_NO_DAMAGE);

	pExplosion->SetDamageRadius(radius);
	pExplosion->SetDamageAmount(dmgamount);
	pExplosion->SetAttacker(pAttacker);
	pExplosion->SetInflictor(pInflictor);

	if(!pExplosion->Spawn())
	{
		Util::RemoveEntity(pExplosion);
		return;
	}

	// Do the explosion
	pExplosion->CallUse(nullptr, nullptr, USE_TOGGLE, 0);
}