/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "triggervacuum.h"
#include "ai_nodegraph.h"
#include "npctesthull.h"

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(trigger_vacuum, CTriggerVacuum);

//=============================================
// @brief
//
//=============================================
CTriggerVacuum::CTriggerVacuum( edict_t* pedict ):
	CBaseEntity(pedict),
	m_isActive(false),
	m_pTestHullNPC(nullptr),
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
CTriggerVacuum::~CTriggerVacuum( void )
{
}

//=============================================
// @brief
//
//=============================================
void CTriggerVacuum::DeclareSaveFields( void )
{
	// Declare parent's fields
	CBaseEntity::DeclareSaveFields();

	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerVacuum, m_isActive, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerVacuum, m_pTestHullNPC, EFIELD_ENTPOINTER));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerVacuum, m_beginTime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerVacuum, m_shutdownTime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerVacuum, m_fullForceTime, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CTriggerVacuum, m_forceFadeOutTime, EFIELD_FLOAT));
}

//=============================================
// @brief
//
//=============================================
bool CTriggerVacuum::KeyValue( const keyvalue_t& kv )
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
bool CTriggerVacuum::Spawn( void )
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

	// Create test hull NPC
	m_pTestHullNPC = reinterpret_cast<CNPCTestHull*>(CBaseEntity::CreateEntity("npc_testhull", this));
	if(!m_pTestHullNPC)
	{
		Util::EntityConDPrintf(m_pEdict, "Failed to create 'npc_testhull' entity.\n");
		Util::RemoveEntity(m_pEdict);
		return false;
	}

	// Make the entity dormant
	m_pTestHullNPC->MakeInert();

	// Initialize entity
	m_pState->flags |= FL_INITIALIZE;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CTriggerVacuum::InitEntity( void )
{
	trace_t tr;
	Util::TraceLine(m_pState->origin, m_pState->origin - Vector(0, 0, 16384), true, false, nullptr, tr);
	if(tr.noHit())
		return;

	SetOrigin(tr.endpos);
}

//=============================================
// @brief
//
//=============================================
void CTriggerVacuum::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
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
		SetThink(&CTriggerVacuum::PullThink);
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

			m_shutdownTime = 0;
			m_beginTime = 0;
		}
	}
}

//=============================================
// @brief
//
//=============================================
void CTriggerVacuum::PullThink( void )
{
	// For now only affect the player
	CBaseEntity* pPlayer = Util::GetHostPlayer();
	if(!pPlayer)
		return;

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
			return;
		}
	}

	// Get player position
	Vector playerPosition = pPlayer->GetNavigablePosition();
	Vector pullPosition = GetPullPosition(pPlayer, playerPosition);

	Vector pullVector = (pullPosition - playerPosition).Normalize() * m_pState->speed * pullForce;
	if(pPlayer->GetFlags() & FL_BASEVELOCITY)
		Math::VectorAdd(pullVector, pPlayer->GetBaseVelocity(), pullVector);

	pPlayer->SetBaseVelocity(pullVector);
	pPlayer->SetFlags(FL_BASEVELOCITY);

	// Set next time so we think again next frame
	m_pState->nextthink = m_pState->ltime + 0.1;
}

//=============================================
// @brief
//
//=============================================
Vector CTriggerVacuum::GetPullPosition( CBaseEntity* pPlayer, const Vector& playerPosition )
{
	// Set up test hull NPC
	m_pTestHullNPC->MakeSolid();

	if(pPlayer->GetFlags() & FL_DUCKING)
		m_pTestHullNPC->SetMinsMaxs(SMALL_NPC_HULL_MIN, SMALL_NPC_HULL_MAX);
	else
		m_pTestHullNPC->SetMinsMaxs(VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	// See if we have a straight line to the player
	if(m_pTestHullNPC->CheckLocalMove(m_pState->origin, playerPosition, pPlayer) > LOCAL_MOVE_RESULT_FAILURE)
	{
		m_pTestHullNPC->MakeInert();
		return m_pState->origin;
	}

	Uint64 nodeTypeBits = Util::GetNodeTypeForNPC(m_pTestHullNPC);
	node_hull_types_t hullType = Util::GetNodeHullForNPC(m_pTestHullNPC);

	// Try to find the air nodes that can navigate to the player
	Int32 playerNodeIndex = NO_POSITION;
	Int32 myNodeIndex = gNodeGraph.GetNearestNode(m_pState->origin, nodeTypeBits, m_pTestHullNPC, pPlayer);
	if(myNodeIndex != NO_POSITION)
		playerNodeIndex = gNodeGraph.GetNearestNode(playerPosition, nodeTypeBits, m_pTestHullNPC, pPlayer);

	if(myNodeIndex == NO_POSITION || playerNodeIndex == NO_POSITION)
	{
		m_pTestHullNPC->MakeInert();
		return m_pState->origin;
	}

	static Int32 nodeIndexes[MAX_ROUTE_POINTS];
	Uint32 numNodes = gNodeGraph.GetShortestPath(myNodeIndex, playerNodeIndex, hullType, CBaseNPC::AI_CAP_NONE, nodeIndexes);
	if(!numNodes)
	{
		m_pTestHullNPC->MakeInert();
		return m_pState->origin;
	}

	CArray<Vector> pointsArray;
	bool showRoute = (gd_engfuncs.pfnGetCvarFloatValue(NODE_DEBUG_CVAR_NAME) >= 1) ? true : false;
	if(showRoute)
		pointsArray.push_back(m_pState->origin);

	// Find the last node we can navigate to
	Vector idealPullPosition = m_pState->origin;
	Int32 i;
	for(i = (Int32)numNodes - 1; i >= 0; i--)
	{
		const CAINodeGraph::node_t* pNode = gNodeGraph.GetNode(nodeIndexes[i]);
		if(!pNode)
		{
			// Shouldn't happen
			idealPullPosition = m_pState->origin;
			break;
		}

		if(m_pTestHullNPC->CheckLocalMove(pNode->origin, playerPosition, pPlayer) <= LOCAL_MOVE_RESULT_FAILURE)
		{
			if(i == ((Int32)numNodes - 1))
				idealPullPosition = pNode->origin;

			break;
		}

		idealPullPosition = pNode->origin;
	}

	if(showRoute)
	{
		for(Int32 j = 0; j < i; j++)
		{
			const CAINodeGraph::node_t* pNode = gNodeGraph.GetNode(nodeIndexes[i]);
			if(!pNode)
			{
				// Shouldn't happen
				idealPullPosition = m_pState->origin;
				break;
			}

			pointsArray.push_back(pNode->origin);
		}

		pointsArray.push_back(idealPullPosition);
		pointsArray.push_back(playerPosition);

		gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.nodedebug, nullptr, nullptr);
		gd_engfuncs.pfnMsgWriteByte(NODE_DEBUG_WAYPOINT);
		gd_engfuncs.pfnMsgWriteByte(WAYPOINT_NORMAL);
		gd_engfuncs.pfnMsgWriteInt32(m_pState->entindex);
		gd_engfuncs.pfnMsgWriteUint16(pointsArray.size());
		for(i = 0; i < (Int32)pointsArray.size(); i++)
		{
			gd_engfuncs.pfnMsgWriteFloat(pointsArray[i].x);
			gd_engfuncs.pfnMsgWriteFloat(pointsArray[i].y);
			gd_engfuncs.pfnMsgWriteFloat(pointsArray[i].z);
		}
		gd_engfuncs.pfnUserMessageEnd();
	}

	m_pTestHullNPC->MakeInert();
	return idealPullPosition;
}