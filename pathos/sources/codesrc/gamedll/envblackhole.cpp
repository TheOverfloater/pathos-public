/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "envblackhole.h"
#include "screenfade.h"
#include "blackhole_shared.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(env_blackhole, CEnvBlackHole);

//=============================================
// @brief
//
//=============================================
CEnvBlackHole::CEnvBlackHole( edict_t* pedict ):
	CPointEntity(pedict),
	m_isActive(false),
	m_pullStrength(0),
	m_rotationSpeed(0),
	m_growthTime(0),
	m_shrinkTime(0),
	m_lifeTime(0)
{
}

//=============================================
// @brief
//
//=============================================
CEnvBlackHole::~CEnvBlackHole( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CEnvBlackHole::Spawn( void )
{
	if(!CPointEntity::Spawn())
		return false;

	// So FL_ALWAYSTHINK works
	m_pState->movetype = MOVETYPE_PUSH;
	
	if(m_pFields->targetname == NO_STRING_VALUE || HasSpawnFlag(FL_START_ON))
		m_isActive = true;
	else
		m_isActive = false;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CEnvBlackHole::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	bool desiredState = false;
	switch(useMode)
	{
	case USE_OFF:
		desiredState = false;
		break;
	case USE_ON:
		desiredState = true;
		break;
	case USE_TOGGLE:
	default:
		if(m_isActive)
			desiredState = false;
		else
			desiredState = true;
		break;
	}

	if(desiredState == m_isActive)
		return;

	m_isActive = desiredState;
	if(m_isActive)
	{
		// Spawn the black hole
		m_spawnTime = g_pGameVars->time;
		SendInitMessage(nullptr);

		if(m_pullStrength > 0)
		{
			SetThink(&CEnvBlackHole::SuckThink);
			m_pState->nextthink = m_pState->ltime + 0.1;
			m_pState->flags |= FL_ALWAYSTHINK;
		}
		else if(m_lifeTime != -1)
		{
			SetThink(&CEnvBlackHole::DieThink);
			m_pState->nextthink = g_pGameVars->time + m_lifeTime;
		}
	}
	else
	{
		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.blackhole, nullptr, nullptr);
		gd_engfuncs.pfnMsgWriteByte(MSG_BLACKHOLE_KILL);
		gd_engfuncs.pfnMsgWriteInt16(m_pEdict->entindex);
		gd_engfuncs.pfnUserMessageEnd();

		if(m_pullStrength > 0)
		{
			SetThink(nullptr);
			m_pState->nextthink = 0;
			m_pState->flags &= ~FL_ALWAYSTHINK;
		}
	}
}

//=============================================
// @brief
//
//=============================================
void CEnvBlackHole::DeclareSaveFields( void )
{
	// Call base class to do it first
	CPointEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvBlackHole, m_isActive, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvBlackHole, m_pullStrength, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvBlackHole, m_rotationSpeed, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvBlackHole, m_growthTime, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvBlackHole, m_shrinkTime, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvBlackHole, m_lifeTime, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CEnvBlackHole, m_spawnTime, EFIELD_TIME));
}

//=============================================
// @brief
//
//=============================================
bool CEnvBlackHole::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "pullstrength"))
	{
		m_pullStrength = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "rotationspeed"))
	{
		m_rotationSpeed = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "growthtime"))
	{
		m_growthTime = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "shrinktime"))
	{
		m_shrinkTime = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "lifetime"))
	{
		m_lifeTime = SDL_atof(kv.value);
		return true;
	}
	else
		return CPointEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
void CEnvBlackHole::SendInitMessage( const CBaseEntity* pPlayer )
{
	if(!m_isActive)
		return;

	if(pPlayer)
		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.blackhole, nullptr, pPlayer->GetEdict());
	else
		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.blackhole, nullptr, nullptr);

	Float adjustedGrowthTime = m_growthTime - ((Float)g_pGameVars->time - (Float)m_spawnTime);
	if(adjustedGrowthTime < 0)
		adjustedGrowthTime = 0;

	gd_engfuncs.pfnMsgWriteByte(MSG_BLACKHOLE_SPAWN);
	gd_engfuncs.pfnMsgWriteInt16(m_pEdict->entindex);
	gd_engfuncs.pfnMsgWriteFloat(m_pState->origin.x);
	gd_engfuncs.pfnMsgWriteFloat(m_pState->origin.y);
	gd_engfuncs.pfnMsgWriteFloat(m_pState->origin.z);
	gd_engfuncs.pfnMsgWriteFloat(m_lifeTime);
	gd_engfuncs.pfnMsgWriteFloat(m_pState->scale);
	gd_engfuncs.pfnMsgWriteFloat(m_pullStrength);
	gd_engfuncs.pfnMsgWriteFloat(m_rotationSpeed);
	gd_engfuncs.pfnMsgWriteSmallFloat(adjustedGrowthTime);
	gd_engfuncs.pfnMsgWriteSmallFloat(m_shrinkTime);
	gd_engfuncs.pfnUserMessageEnd();
}

//=============================================
// @brief
//
//=============================================
void CEnvBlackHole::SuckThink( void )
{
	if(m_lifeTime != -1)
	{
		if(m_spawnTime + m_lifeTime < g_pGameVars->time)
		{
			SetThink(nullptr);
			m_pState->nextthink = 0;
			m_pState->flags &= ~FL_ALWAYSTHINK;
			return;
		}
	}

	// Scale of pull strength
	Float scale = m_pState->scale;

	// Calculate growth period
	if(m_growthTime && g_pGameVars->time < (m_spawnTime + m_growthTime))
	{
		Float growthFactor = (g_pGameVars->time - m_spawnTime) / m_growthTime;
		if(growthFactor > 1.0)
			growthFactor = 1.0;
		else if(growthFactor > 1.0)
			growthFactor = 1.0;

		scale *= growthFactor;
	}

	// Calculate shrinking
	if(m_lifeTime != -1 && m_shrinkTime)
	{
		Double shrinkBeginTime = (m_spawnTime + m_lifeTime - m_shrinkTime);
		if(shrinkBeginTime < g_pGameVars->time)
		{
			Float shrinkFactor = (g_pGameVars->time - shrinkBeginTime) / m_shrinkTime;
			if(shrinkFactor < 0)
				shrinkFactor = 0;
			else if(shrinkFactor > 1.0)
				shrinkFactor = 1.0;

			scale *= (1.0 - shrinkFactor);
		}
	}

	Float fullRadius = BLACK_HOLE_SIZE*m_pState->scale*scale;
	Float radiusSquared = fullRadius*fullRadius;

	Vector mins, maxs;
	for(Uint32 i = 0; i < 3; i++)
	{
		mins[i] = m_pState->origin[i] - fullRadius;
		maxs[i] = m_pState->origin[i] + fullRadius;
	}

	edict_t* pedict = nullptr;
	while(true)
	{
		pedict = Util::FindEntityInBBox(pedict, mins, maxs);
		if(!pedict)
			break;

		CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
		if(!pEntity->CanBlackHolePull())
			continue;

		// Get entity's origin
		Vector entityOrigin = pEntity->IsBrushModel() ? pEntity->GetCenter() : pEntity->GetOrigin();

		// Use inverse square radius
		Vector direction = m_pState->origin - entityOrigin;
		Float distance = Math::DotProduct(direction, direction);
		Float attenuation = ((distance/radiusSquared) - 1.0) * -1.0;
		attenuation = clamp(attenuation, 0.0, 1.0);

		// Calculate strength of pull by black hole
		Float pullStrength = attenuation*BLACK_HOLE_SUCK_SPEED*m_pullStrength;
		direction.Normalize();

		Vector entityVelocity = pEntity->GetVelocity();
		Vector velocityDirection = entityVelocity;
		velocityDirection.Normalize();

		Vector awayDirection = velocityDirection - direction;
		awayDirection.Normalize();

		// Remove from velocity slowly the direction pointing away from the hole
		entityVelocity = entityVelocity - awayDirection*pullStrength*g_pGameVars->frametime;

		if(m_rotationSpeed)
		{
			Vector right, up;
			Math::GetUpRight(direction, up, right);

			Float orbitAtten = radiusSquared/(distance*sqrt(distance));
			orbitAtten = clamp(orbitAtten, 0.0, 1.0);

			awayDirection = velocityDirection - right;
			awayDirection.Normalize();

			// Calculate orbit strength
			Float orbitStrength = BLACK_HOLE_SUCK_SPEED * m_rotationSpeed * (1.0 - orbitAtten);

			// Orbit direction should pull towards the black hole the closer we are
			entityVelocity = entityVelocity + right * orbitStrength * g_pGameVars->frametime;
		}

		// Add velocity moving towards black hole
		entityVelocity = entityVelocity + direction*pullStrength*g_pGameVars->frametime;

		// Set final speed in entity and remove onground flag
		pEntity->SetVelocity(entityVelocity);
		pEntity->RemoveFlags(FL_ONGROUND);

		if( sqrt(distance) < BLACK_HOLE_KILL_DISTANCE*scale )
		{
			if(pEntity->IsNPC() || pEntity->IsPlayer())
			{
				pEntity->TakeDamage( this, this, 50000, (DMG_CRUSH|DMG_BLACKHOLE) );

				if(pEntity->IsPlayer() && !(pEntity->GetFlags() & FL_GODMODE))
					pEntity->SetControlEnable(false);
			}
			else if( pEntity->GetMoveType() != MOVETYPE_FOLLOW )
			{
				// Remove this non-NPC entity
				Util::RemoveEntity(pedict);
			}
		}
	}

	m_pState->nextthink = m_pState->ltime + 0.1;
}

//=============================================
// @brief
//
//=============================================
void CEnvBlackHole::DieThink( void )
{
	SetThink(nullptr);
	m_pState->nextthink = 0;
}