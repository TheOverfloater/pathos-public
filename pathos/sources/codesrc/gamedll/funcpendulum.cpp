/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "funcpendulum.h"

// Default speed for pendulum
const Float CFuncPendulum::DEFAULT_SPEED = 100;

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(func_pendulum, CFuncPendulum);

//=============================================
// @brief
//
//=============================================
CFuncPendulum::CFuncPendulum( edict_t* pedict ):
	CBaseEntity(pedict),
	m_acceleration(0),
	m_distance(0),
	m_time(0),
	m_dampening(0),
	m_maxSpeed(0),
	m_dampSpeed(0),
	m_damage(0)
{
}

//=============================================
// @brief
//
//=============================================
CFuncPendulum::~CFuncPendulum( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CFuncPendulum::Spawn( void )
{
	// Set axis direction
	Util::SetAxisDirection(*m_pState, m_pState->spawnflags, FL_ROTATE_Z, FL_ROTATE_X);

	if(HasSpawnFlag(FL_NOT_SOLID))
		m_pState->solid = SOLID_NOT;
	else
		m_pState->solid = SOLID_BSP;

	m_pState->movetype = MOVETYPE_PUSH;
	if(!SetModel(m_pFields->modelname))
		return false;

	if(!m_distance)
	{
		Util::EntityConPrintf(m_pEdict, "No distance set.\n");
		return true;
	}

	if(!m_pState->speed)
		m_pState->speed = DEFAULT_SPEED;

	m_acceleration = (m_pState->speed*m_pState->speed)/(2*SDL_fabs(m_distance));
	m_maxSpeed = m_pState->speed;
	m_start = m_pState->angles;
	Math::VectorMA(m_pState->angles, m_distance*0.5, m_pState->movedir, m_center);

	if(HasSpawnFlag(FL_START_ON))
		m_pState->flags |= FL_INITIALIZE;

	// Reset speed to zero
	m_pState->speed = 0;

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CFuncPendulum::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "distance"))
	{
		m_distance = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "damp"))
	{
		m_dampening = SDL_atof(kv.value) * 0.001;
		return true;
	}
	else if(!qstrcmp(kv.keyname, "dmg"))
	{
		m_damage = SDL_atof(kv.value);
		return true;
	}
	else
		return CBaseEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
void CFuncPendulum::DeclareSaveFields( void )
{
	// Call base class to handle it's own
	CBaseEntity::DeclareSaveFields();

	DeclareSaveField(DEFINE_DATA_FIELD(CFuncPendulum, m_acceleration, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncPendulum, m_distance, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncPendulum, m_time, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncPendulum, m_dampening, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncPendulum, m_maxSpeed, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncPendulum, m_dampSpeed, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncPendulum, m_damage, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncPendulum, m_center, EFIELD_VECTOR));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncPendulum, m_start, EFIELD_VECTOR));
}

//=============================================
// @brief
//
//=============================================
void CFuncPendulum::CallUse( CBaseEntity* pacticator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	if(!m_pState->speed)
	{
		// Start the pendulum;
		m_time = g_pGameVars->time;
		m_dampSpeed = m_maxSpeed;
		m_pState->flags |= FL_ALWAYSTHINK;

		SetThink(&CFuncPendulum::SwingThink);
		m_pState->nextthink = m_pState->ltime + 0.1;
	}
	else if(HasSpawnFlag(FL_AUTO_RETURN))
	{
		// Get axis delta
		Float delta = Util::GetAxisDelta(m_pState->spawnflags, m_pState->angles, m_start, FL_ROTATE_Z, FL_ROTATE_X);
		Math::VectorScale(m_pState->movedir, -m_maxSpeed, m_pState->avelocity);

		SetThink(&CFuncPendulum::StopThink);
		m_pState->nextthink = m_pState->ltime + (delta/m_maxSpeed);
		m_pState->flags &= ~FL_ALWAYSTHINK;
	}
	else
	{
		ClearThinkFunctions();

		// Clear velocity
		m_pState->avelocity.Clear();
		m_pState->flags &= ~FL_ALWAYSTHINK;
	}
}

//=============================================
// @brief
//
//=============================================
void CFuncPendulum::StopThink( void )
{
	m_pState->angles = m_start;
	m_pState->speed = 0;
	m_pState->avelocity.Clear();

	ClearThinkFunctions();
}

//=============================================
// @brief
//
//=============================================
void CFuncPendulum::SwingThink( void )
{
	Float axisdelta = Util::GetAxisDelta(m_pState->spawnflags, m_pState->angles, m_center, FL_ROTATE_Z, FL_ROTATE_X);
	Float timedelta = g_pGameVars->time - m_time;
	m_time = g_pGameVars->time;

	if(axisdelta > 0 && m_acceleration > 0)
		m_pState->speed -= m_acceleration * timedelta;
	else
		m_pState->speed += m_acceleration * timedelta;

	// Clamp velocity
	m_pState->speed = clamp(m_pState->speed, -m_maxSpeed, m_maxSpeed);
	Math::VectorScale(m_pState->movedir, m_pState->speed, m_pState->avelocity);

	m_pState->nextthink = m_pState->ltime + 0.1;

	// Add dampening if any
	if(m_dampening)
	{
		m_dampSpeed -= m_dampening * m_dampSpeed * timedelta;
		if(m_dampSpeed < 30)
		{
			m_pState->angles = m_center;
			m_pState->speed = 0;
			m_pState->avelocity.Clear();

			ClearThinkFunctions();
		}
		else
		{
			// Clamp speed
			m_pState->speed = clamp(m_pState->speed, -m_dampSpeed, m_dampSpeed);
		}
	}
}

//=============================================
// @brief
//
//=============================================
void CFuncPendulum::CallBlocked( CBaseEntity* pOther )
{
	m_time = g_pGameVars->time;
}

//=============================================
// @brief
//
//=============================================
void CFuncPendulum::CallTouch( CBaseEntity* pOther )
{
	if(m_damage <= 0)
		return;

	if(pOther->GetTakeDamage() == TAKEDAMAGE_NO)
		return;

	Float damage = m_damage * m_pState->speed * 0.01;
	damage = SDL_fabs(damage);

	// Apply damage to other
	pOther->TakeDamage(this, this, damage, DMG_CRUSH);

	// Apply velocity to other
	Vector velocity = (pOther->GetOrigin() - GetBrushModelCenter()).Normalize() * damage;
	pOther->SetVelocity(velocity);
}

//=============================================
// @brief
//
//=============================================
void CFuncPendulum::InitEntity( void )
{
	// Used for delayed initialization
	CallUse(this, this, USE_TOGGLE, 0);
}