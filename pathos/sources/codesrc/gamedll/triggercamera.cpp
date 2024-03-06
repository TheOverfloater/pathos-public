/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "triggercamera.h"
#include "player.h"
#include "pathcorner.h"

// Default angular speed
const Float CTriggerCamera::DEFAULT_ANGULAR_SPEED = 40;

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(trigger_camera, CTriggerCamera);

//=============================================
// @brief
//
//=============================================
CTriggerCamera::CTriggerCamera( edict_t* pedict ):
	CDelayEntity(pedict),
	m_pPlayerEntity(nullptr),
	m_pPathEntity(nullptr),
	m_sPathEntityName(NO_STRING_VALUE),
	m_waitTime(0),
	m_returnTime(0),
	m_stopTime(0),
	m_moveDistance(0),
	m_targetSpeed(0),
	m_initialSpeed(0),
	m_acceleration(0),
	m_deceleration(0),
	m_angularSpeed(0),
	m_isActive(false)
{
}

//=============================================
// @brief
//
//=============================================
CTriggerCamera::~CTriggerCamera( void )
{
}

//=============================================
// @brief
//
//=============================================
void CTriggerCamera::DeclareSaveFields( void )
{
	// Call base class to do it first
	CDelayEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerCamera, m_pPlayerEntity, EFIELD_ENTPOINTER));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerCamera, m_targetEntity, EFIELD_EHANDLE));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerCamera, m_pPathEntity, EFIELD_ENTPOINTER));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerCamera, m_sPathEntityName, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerCamera, m_waitTime, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerCamera, m_returnTime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerCamera, m_stopTime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerCamera, m_moveDistance, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerCamera, m_targetSpeed, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerCamera, m_initialSpeed, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerCamera, m_acceleration, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerCamera, m_deceleration, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerCamera, m_isActive, EFIELD_BOOLEAN));
}

//=============================================
// @brief
//
//=============================================
bool CTriggerCamera::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "wait"))
	{
		m_waitTime = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "moveto"))
	{
		m_sPathEntityName = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "acceleration"))
	{
		m_acceleration = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "deceleration"))
	{
		m_deceleration = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "angularspeed"))
	{
		m_angularSpeed = SDL_atof(kv.value);
		return true;
	}
	else
		return CDelayEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
void CTriggerCamera::Precache( void )
{
	gd_engfuncs.pfnPrecacheModel(NULL_SPRITE_FILENAME);
}

//=============================================
// @brief
//
//=============================================
bool CTriggerCamera::Spawn( void )
{
	if(!CDelayEntity::Spawn())
		return false;

	// Set empty sprite texture
	if(!SetModel(NULL_SPRITE_FILENAME, false))
		return false;

	m_pState->movetype = MOVETYPE_NOCLIP;
	m_pState->solid = SOLID_NOT;

	m_initialSpeed = m_pState->speed;
	if(m_acceleration < 1)
		m_acceleration = 0;
	if(m_deceleration < 1)
		m_deceleration = 0;

	if(m_angularSpeed <= 0)
		m_angularSpeed = DEFAULT_ANGULAR_SPEED;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CTriggerCamera::SendInitMessage( const CBaseEntity* pPlayer )
{
	if(!m_isActive || !m_pPlayerEntity)
		return;

	m_pPlayerEntity->SetViewEntity(this);
}

//=============================================
// @brief
//
//=============================================
void CTriggerCamera::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	if(!ShouldToggle(useMode, m_isActive))
		return;

	if(m_isActive)
	{
		if(m_pPlayerEntity)
		{
			m_pPlayerEntity->SetViewEntity(nullptr);

			if(HasSpawnFlag(FL_PLAYER_TAKECONTROL))
				m_pPlayerEntity->SetControlEnable(false);
		}

		m_pState->effects &= ~EF_NOVIS;
		m_returnTime = 0;
		m_isActive = false;
		return;
	}

	CPlayerEntity* pPlayer;
	if(pActivator && pActivator->IsPlayer())
		pPlayer = reinterpret_cast<CPlayerEntity*>(pActivator);
	else
		pPlayer = reinterpret_cast<CPlayerEntity*>(Util::GetHostPlayer());

	m_pPlayerEntity = pPlayer;
	m_returnTime = g_pGameVars->time + m_waitTime;
	m_pState->speed = m_initialSpeed;
	m_targetSpeed = m_initialSpeed;

	if(HasSpawnFlag(FL_PLAYER_TARGET))
	{
		m_targetEntity = m_pPlayerEntity;
	}
	else
	{
		edict_t* pEdict = Util::FindEntityByTargetName(nullptr, gd_engfuncs.pfnGetString(m_pFields->target));
		if(pEdict)
			m_targetEntity = CDelayEntity::GetClass(pEdict);
		else
			m_targetEntity = (CDelayEntity*)nullptr;
	}

	// Disable player control if set
	if(HasSpawnFlag(FL_PLAYER_TAKECONTROL))
		m_pPlayerEntity->SetControlEnable(false);

	if(m_sPathEntityName != NO_STRING_VALUE)
	{
		edict_t* pEdict = Util::FindEntityByTargetName(nullptr, gd_engfuncs.pfnGetString(m_sPathEntityName));
		if(pEdict)
			m_pPathEntity = CDelayEntity::GetClass(pEdict);
		else
			m_pPathEntity = nullptr;
	}
	else
	{
		m_pPathEntity = nullptr;
	}

	m_stopTime = g_pGameVars->time;
	if(m_pPathEntity)
	{
		if(m_pPathEntity->GetSpeed())
			m_targetSpeed = m_pPathEntity->GetSpeed();

		m_stopTime += m_pPathEntity->GetDelay();
	}

	if(HasSpawnFlag(FL_PLAYER_POSITION))
	{
		Vector origin = m_pPlayerEntity->GetEyePosition();
		gd_engfuncs.pfnSetOrigin(m_pEdict, origin);

		Vector angles = m_pPlayerEntity->GetViewAngles();
		angles[0] = -angles[0];
		angles[2] = 0;
		m_pState->angles = angles;

		m_pState->velocity = m_pPlayerEntity->GetVelocity();
	}
	else
	{
		m_pState->velocity.Clear();
	}

	if(HasSpawnFlag(FL_LOCKON) && m_targetEntity)
	{
		m_pState->angles = Math::VectorToAngles((m_targetEntity->GetOrigin() - m_pState->origin).Normalize());
		m_pState->angles.x = -m_pState->angles.x;
	}

	m_pPlayerEntity->SetViewEntity(this);
	
	SetThink(&CTriggerCamera::FollowTarget);
	m_pState->nextthink = g_pGameVars->time;

	// Set NOVIS flag
	m_pState->effects |= EF_NOVIS;
	
	m_moveDistance = 0;
	m_isActive = true;

	Move();
}

//=============================================
// @brief
//
//=============================================
void CTriggerCamera::Move( void )
{
	if(!m_pPathEntity)
		return;

	// Subtract speed
	m_moveDistance -= m_pState->speed * g_pGameVars->frametime;
	if(m_moveDistance <= 0)
	{
		// Fire pass target
		if(m_pPathEntity->HasMessage())
		{
			const Char* pstrMessage = m_pPathEntity->GetMessage();
			Util::FireTargets(pstrMessage, this, this, USE_TOGGLE, 0);
			if(m_pPathEntity->HasSpawnFlag(CPathCorner::FL_FIRE_ONCE))
				m_pPathEntity->SetMessage(nullptr);
		}

		// Go to next target
		m_pPathEntity = m_pPathEntity->GetNextTarget();

		if(!m_pPathEntity)
		{
			// Clear velocity
			m_pState->velocity.Clear();
		}
		else
		{
			Int32 pathspeed = m_pPathEntity->GetSpeed();
			if(pathspeed > 0)
				m_targetSpeed = pathspeed;

			Vector delta = m_pPathEntity->GetOrigin() - m_pState->origin;
			m_moveDistance = delta.Length();
			m_pState->movedir = delta;
			m_pState->movedir.Normalize();

			m_stopTime = g_pGameVars->time + m_pPathEntity->GetDelay();
		}
	}

	if(m_acceleration && m_deceleration)
	{
		if(m_stopTime > g_pGameVars->time)
			m_pState->speed = Util::Approach(0, m_pState->speed, m_deceleration * g_pGameVars->frametime);
		else 
			m_pState->speed = Util::Approach(m_targetSpeed, m_pState->speed, m_acceleration * g_pGameVars->frametime);

		Float fraction = 2*g_pGameVars->frametime;
		m_pState->velocity = ((m_pState->movedir*m_pState->speed)*fraction)+(m_pState->velocity*(1.0-fraction));
	}
	else
	{
		m_pState->speed = m_initialSpeed;
		m_pState->velocity = m_pState->movedir * m_pState->speed;
	}
}

//=============================================
// @brief
//
//=============================================
void CTriggerCamera::FollowTarget( void )
{
	if(!m_pPlayerEntity)
		return;

	if(!m_targetEntity)
	{
		if(m_returnTime < g_pGameVars->time)
		{
			if(m_pPlayerEntity->IsAlive())
			{
				if(HasSpawnFlag(FL_PLAYER_TAKECONTROL))
					m_pPlayerEntity->SetControlEnable(false);

				m_pPlayerEntity->SetViewEntity(nullptr);
			}

			UseTargets(this, USE_TOGGLE, 0);
			m_pState->avelocity.Clear();
			m_isActive = false;
			m_pState->effects &= ~EF_NOVIS;
			return;
		}

		m_pState->nextthink = g_pGameVars->time;
		return;
	}

	if(m_returnTime < g_pGameVars->time)
	{
		if(m_pPlayerEntity->IsAlive())
		{
			if(HasSpawnFlag(FL_PLAYER_TAKECONTROL))
				m_pPlayerEntity->SetControlEnable(false);

			m_pPlayerEntity->SetViewEntity(nullptr);
		}

		UseTargets(this, USE_TOGGLE, 0);
		m_pState->avelocity.Clear();
		m_isActive = false;
		m_pState->effects &= ~EF_NOVIS;
		return;
	}

	Vector targetDirection = (m_targetEntity->GetOrigin() - m_pState->origin).Normalize();
	Vector vecDestAngles = Math::VectorToAngles(targetDirection);
	vecDestAngles.x *= -1;

	if(m_pState->angles.y > 360)
		m_pState->angles.y -= 360;

	if(m_pState->angles.y < 0)
		m_pState->angles.y += 360;

	Float dx = vecDestAngles.x - m_pState->angles.x;
	Float dy = vecDestAngles.y - m_pState->angles.y;

	if(dx < -180) dx += 360;
	if(dx > 180) dx -= 360;

	if(dy < -180) dy += 360;
	if(dy > 180) dy -= 360;

	m_pState->avelocity.x = dx*m_angularSpeed*g_pGameVars->frametime;
	m_pState->avelocity.y = dy*m_angularSpeed*g_pGameVars->frametime;

	if(!HasSpawnFlag(FL_PLAYER_TAKECONTROL))
	{
		Math::VectorScale(m_pState->velocity, 0.8, m_pState->velocity);
		if(m_pState->velocity.Length() < 10)
			m_pState->velocity.Clear();
	}

	m_pState->nextthink = g_pGameVars->time;
	Move();
}