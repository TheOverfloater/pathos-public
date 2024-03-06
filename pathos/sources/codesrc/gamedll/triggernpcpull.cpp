/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "triggernpcpull.h"
#include "ai_nodegraph.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(trigger_npcpull, CTriggerNPCPull);

//=============================================
// @brief
//
//=============================================
CTriggerNPCPull::CTriggerNPCPull( edict_t* pedict ):
	CBaseEntity(pedict),
	m_isActive(false),
	m_beginTime(0),
	m_shutdownTime(0),
	m_fullForceTime(0),
	m_forceFadeOutTime(0)
{
}

//=============================================
// @brief
//
//=============================================
CTriggerNPCPull::~CTriggerNPCPull( void )
{
}

//=============================================
// @brief
//
//=============================================
void CTriggerNPCPull::DeclareSaveFields( void )
{
	// Declare parent's fields
	CBaseEntity::DeclareSaveFields();

	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerNPCPull, m_isActive, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerNPCPull, m_pullBBoxMins, EFIELD_COORD));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerNPCPull, m_pullBBoxMaxs, EFIELD_COORD));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerNPCPull, m_finalPullPosition, EFIELD_COORD));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerNPCPull, m_beginTime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerNPCPull, m_shutdownTime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerNPCPull, m_fullForceTime, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerNPCPull, m_forceFadeOutTime, EFIELD_FLOAT));
}

//=============================================
// @brief
//
//=============================================
bool CTriggerNPCPull::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "fullforcetime"))
	{
		m_fullForceTime = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "fadeouttime"))
	{
		m_forceFadeOutTime = SDL_atof(kv.value);
		return true;
	}
	else
		return CBaseEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CTriggerNPCPull::Spawn( void )
{
	if(!CBaseEntity::Spawn())
		return false;

	m_pState->effects |= EF_NODRAW;
	m_pState->solid = SOLID_NOT;
	m_pState->movetype = MOVETYPE_PUSH; // So FL_ALWAYSTHINK works

	if(!m_pState->speed)
	{
		Util::EntityConDPrintf(m_pEdict, "No pull speed set for entity.\n");
		Util::RemoveEntity(m_pEdict);
		return false;
	}

	if(m_pFields->target == NO_STRING_VALUE)
	{
		Util::EntityConPrintf(m_pEdict, "No bounding box entity specified in 'Target'.\n");
		Util::RemoveEntity(m_pEdict);
		return false;
	}

	if(m_pFields->netname == NO_STRING_VALUE)
	{
		Util::EntityConPrintf(m_pEdict, "No final pull position entity specified.\n");
		Util::RemoveEntity(m_pEdict);
		return false;
	}

	// Initialize entity
	m_pState->flags |= FL_INITIALIZE;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CTriggerNPCPull::InitEntity( void )
{
	// Find brushmodel that defines our mins/maxs for pulling
	const Char* pstrTargetEntity = gd_engfuncs.pfnGetString(m_pFields->target);
	edict_t* pEdict = Util::FindEntityByTargetName(nullptr, pstrTargetEntity);
	if(!pEdict || Util::IsNullEntity(pEdict))
	{
		Util::EntityConPrintf(m_pEdict, "Bounding box entity '%s' not found.\n", pstrTargetEntity);
		Util::RemoveEntity(m_pEdict);
		return;
	}

	CBaseEntity* pEntity = CBaseEntity::GetClass(pEdict);

	m_pullBBoxMins = pEntity->GetAbsMins();
	m_pullBBoxMaxs = pEntity->GetAbsMaxs();

	// Remove the entity
	pEntity->FlagForRemoval();

	// Find point entity that defines the final pull position
	const Char* pstrFinalPositionEntity = gd_engfuncs.pfnGetString(m_pFields->netname);
	pEdict = Util::FindEntityByTargetName(nullptr, pstrFinalPositionEntity);
	if(!pEdict || Util::IsNullEntity(pEdict))
	{
		Util::EntityConPrintf(m_pEdict, "Final pull position entity '%s' not found.\n", pstrTargetEntity);
		Util::RemoveEntity(m_pEdict);
		return;
	}

	m_finalPullPosition = pEdict->state.origin;
}

//=============================================
// @brief
//
//=============================================
void CTriggerNPCPull::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	bool desiredStatus = false;
	switch(useMode)
	{
	case USE_OFF:
		desiredStatus = false;
		break;
	case USE_ON:
		desiredStatus = true;
		break;
	case USE_TOGGLE:
	default:
		if(m_isActive)
			desiredStatus = false;
		else
			desiredStatus = true;
		break;
	}

	if(desiredStatus == m_isActive)
		return;

	m_isActive = desiredStatus;
	if(desiredStatus)
	{
		m_pState->flags |= FL_ALWAYSTHINK;
		m_pState->nextthink = m_pState->ltime + 0.1;
		SetThink(&CTriggerNPCPull::PullThink);
		m_beginTime = g_pGameVars->time;
		m_shutdownTime = 0;
	}
	else
	{
		if(m_forceFadeOutTime)
		{
			m_shutdownTime = g_pGameVars->time;
			m_beginTime = 0;
		}
		else
		{
			m_pState->flags &= ~FL_ALWAYSTHINK;
			m_pState->nextthink = 0;
			SetThink(nullptr);
			ClearNPCs();

			m_shutdownTime = 0;
			m_beginTime = 0;
		}
	}
}
//=============================================
// @brief
//
//=============================================
void CTriggerNPCPull::ClearNPCs( void )
{
	if(m_pulledNPCsList.empty())
		return;

	// Reset NPCs that were not pulled
	m_pulledNPCsList.begin();
	while(!m_pulledNPCsList.end())
	{
		pullednpc_t& npc = m_pulledNPCsList.get();
		npc.pentity->SetNPCPuller(nullptr, ZERO_VECTOR);

		m_pulledNPCsList.remove(m_pulledNPCsList.get_link());
		m_pulledNPCsList.next();
	}
}

//=============================================
// @brief
//
//=============================================
void CTriggerNPCPull::PullThink( void )
{
	Float pullForce = 1.0;
	if(m_beginTime && m_fullForceTime > 0)
	{
		if(m_beginTime + m_fullForceTime > g_pGameVars->time)
		{
			// Calculate pull force
			pullForce = (g_pGameVars->time - m_beginTime) / m_fullForceTime;
		}
		else
		{
			// Stop attenuation
			m_beginTime = 0;
		}
	}
	else if(m_shutdownTime && m_forceFadeOutTime > 0)
	{
		if(m_shutdownTime + m_forceFadeOutTime > g_pGameVars->time)
		{
			// Calculate pull force
			pullForce = (g_pGameVars->time - m_shutdownTime) / m_forceFadeOutTime;
			pullForce = 1.0 - pullForce;
		}
		else
		{
			m_pState->flags &= ~FL_ALWAYSTHINK;
			m_pState->nextthink = 0;
			SetThink(nullptr);

			m_shutdownTime = 0;
			m_beginTime = 0;
			ClearNPCs();
			return;
		}
	}

	// Reset pull list
	m_pulledNPCsList.begin();
	while(!m_pulledNPCsList.end())
	{
		pullednpc_t& npc = m_pulledNPCsList.get();
		npc.waspulled = false;
		m_pulledNPCsList.next();
	}

	edict_t* pEdict = nullptr;
	while(true)
	{
		pEdict = Util::FindEntityInBBox(pEdict, m_pullBBoxMins, m_pullBBoxMaxs);
		if(!pEdict || Util::IsNullEntity(pEdict))
			break;

		CBaseEntity* pEntity = CBaseEntity::GetClass(pEdict);
		if(!pEntity->IsNPC())
			continue;

		// Get player position
		Vector pullVector, pullPosition;
		if(GetPullVector(pEntity, pullVector, pullPosition))
		{
			Math::VectorScale(pullVector, pullForce, pullVector);
			if(pullVector.z > 0 || pullForce == 1.0)
				pEntity->RemoveFlags(FL_ONGROUND);

			Math::VectorAdd(pullVector, pEntity->GetVelocity(), pullVector);
			pEntity->SetVelocity(pullVector);

			// Any npc in the bbox is considered as pulled
			m_pulledNPCsList.begin();
			while(!m_pulledNPCsList.end())
			{
				pullednpc_t& npc = m_pulledNPCsList.get();
				if(npc.pentity == pEntity)
				{
					pEntity->SetNPCPuller(this, pullPosition);
					npc.waspulled = true;
					break;
				}

				m_pulledNPCsList.next();
			}

			if(m_pulledNPCsList.end())
			{
				pullednpc_t newpull;
				newpull.pentity = pEntity;
				newpull.waspulled = true;
				pEntity->SetNPCPuller(this, pullPosition);

				m_pulledNPCsList.add(newpull);
			}
		}
	}

	// Reset NPCs that were not pulled
	m_pulledNPCsList.begin();
	while(!m_pulledNPCsList.end())
	{
		pullednpc_t& npc = m_pulledNPCsList.get();
		if(!npc.waspulled)
		{
			npc.pentity->SetNPCPuller(nullptr, ZERO_VECTOR);
			m_pulledNPCsList.remove(m_pulledNPCsList.get_link());
			m_pulledNPCsList.next();
			continue;
		}

		m_pulledNPCsList.next();
	}

	// Set next time so we think again next frame
	m_pState->nextthink = m_pState->ltime + 0.1;
}

//=============================================
// @brief
//
//=============================================
bool CTriggerNPCPull::GetPullVector( CBaseEntity* pEntity, Vector& outVector, Vector& outPullPosition )
{
	Vector npcPosition = pEntity->GetOrigin();

	// Determine npc hull to use
	Vector size;
	Math::VectorSubtract(pEntity->GetMaxs(), pEntity->GetMins(), size);

	// Rely on server logic for hull type
	hull_types_t hulltype = size[2] < 36.0f ? HULL_SMALL : HULL_HUMAN;
	if(hulltype == HULL_HUMAN)
		npcPosition.z += SDL_fabs(VEC_HULL_MIN.z);
	else
		npcPosition.z += SDL_fabs(VEC_DUCK_HULL_MIN.z);

	Vector pullPosition;

	// See if final pull position is reachable
	trace_t tr;
	Util::TraceHull(npcPosition, m_finalPullPosition, true, false, hulltype, nullptr, tr);
	if(tr.noHit() && !tr.allSolid() && !tr.startSolid())
		pullPosition = m_finalPullPosition;
	else
		pullPosition = m_pState->origin;

	// See if NPC is directly traceable towards us
	tr = trace_t();
	Util::TraceHull(npcPosition, pullPosition, true, false, hulltype, nullptr, tr);

	if(tr.noHit() && !tr.startSolid() && !tr.allSolid())
	{
		outVector = (pullPosition - npcPosition).Normalize() * m_pState->speed;
		outPullPosition = pullPosition;
		return true;
	}

	// Second, see if going on the same plane as the npc is standing on reaches some distance
	Vector flatPullPosition = pullPosition;
	flatPullPosition.z = npcPosition.z;

	tr = trace_t();
	Util::TraceHull(npcPosition, flatPullPosition, true, false, hulltype, nullptr, tr);
	if(tr.fraction != 0.0 && (tr.endpos - npcPosition).Length() > 16)
	{
		// Keep pulling the NPC towards us until he reaches the blocking object
		outVector = (flatPullPosition - npcPosition).Normalize() * m_pState->speed + Vector(0, 0, 1) * m_pState->speed * 0.75;
		outPullPosition = flatPullPosition;
		return true;
	}

	// NPC is being directly blocked by an object, so try to pull him upwards
	Float checkDistance = 16;
	Float maxDistance = 512;
	while(checkDistance <= maxDistance)
	{
		Vector testPosition = npcPosition + Vector(0, 0, 1) * checkDistance;
		checkDistance += 16.0f;

		flatPullPosition = pullPosition;
		flatPullPosition.z = testPosition.z;

		tr = trace_t();
		Util::TraceHull(testPosition, flatPullPosition, true, false, hulltype, nullptr, tr);
		if(tr.noHit() && !tr.startSolid() && !tr.allSolid())
		{
			outVector = Vector(0, 0, 1) * (m_pState->speed*2);
			outPullPosition = flatPullPosition;
			return true;
		}
	}

	// Keep pulling the NPC towards us until he reaches the blocking object
	outVector = (pullPosition - npcPosition).Normalize() * m_pState->speed;
	outPullPosition = pullPosition;
	return true;
}

//=============================================
// @brief Removes a pulled NPC from the list
//
//=============================================
void CTriggerNPCPull::RemovePulledNPC( CBaseEntity* pNPC ) 
{
	// Reset NPCs that were not pulled
	m_pulledNPCsList.begin();
	while(!m_pulledNPCsList.end())
	{
		pullednpc_t& npc = m_pulledNPCsList.get();
		if(npc.pentity == pNPC)
		{
			npc.pentity->SetNPCPuller(nullptr, ZERO_VECTOR);
			m_pulledNPCsList.remove(m_pulledNPCsList.get_link());
			break;
		}

		m_pulledNPCsList.next();
	}
}