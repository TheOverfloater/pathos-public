/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "ai_patrolnpc.h"
#include "ai_nodegraph.h"
#include "player.h"

// Max number of patrol failures before the patrol history is flushed
const Uint32 CPatrolNPC::MAX_PATROL_FAILURES = 2;
// Minimum patrol distance
const Float CPatrolNPC::MIN_PATROL_DISTANCE = 1024;
// Minimum visible patrol distance
const Float CPatrolNPC::MIN_VISIBLE_PATROL_DISTANCE = 512;
// Default patrol radius
const Float CPatrolNPC::DEFAULT_PATROL_RADIUS = 4096;
// Maximum distance a destination can have from the player's position
const Float CPatrolNPC::MAX_PATROL_DEST_PLAYER_DISTANCE = 2048;
// Minimum distance from a budge attempt we'll accept
const Float CPatrolNPC::MIN_BUDGE_MOVE_DISTANCE = 64;
// Distance to which we'll try to budge out of a stuck spot
const Float CPatrolNPC::BUDGE_MOVE_DISTANCE = 256;

//==========================================================================
//
// SCHEDULES FOR CPATROLNPC CLASS
//
//==========================================================================

//=============================================
// @brief Patrol
//
//=============================================
ai_task_t taskListSchedulePatrolNPCPatrol[] = 
{
	AITASK(AI_TASK_SET_FAIL_SCHEDULE,			(Float)AI_PATROLNPC_SCHED_PATROL_FAIL),
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_PATROLNPC_TASK_FIND_DEST,			0),
	AITASK(AI_TASK_FACE_IDEAL,					0),
	AITASK(AI_TASK_WALK_PATH,					0),
	AITASK(AI_TASK_WAIT_FOR_MOVEMENT,			0),
	AITASK(AI_PATROLNPC_TASK_PATROL_DONE,		0),
	AITASK(AI_TASK_TURN_LEFT,					180),
	AITASK(AI_TASK_SET_ACTIVITY,				(Float)ACT_IDLE),
	AITASK(AI_TASK_WAIT,						3),
	AITASK(AI_TASK_TURN_LEFT,					180),
	AITASK(AI_TASK_SET_ACTIVITY,				(Float)ACT_IDLE),
	AITASK(AI_TASK_WAIT,						2)
};

const CAISchedule schedulePatrolNPCPatrol(
	// Task list
	taskListSchedulePatrolNPCPatrol, 
	// Number of tasks
	PT_ARRAYSIZE(taskListSchedulePatrolNPCPatrol),
	// AI interrupt mask
	AI_COND_DANGEROUS_ENEMY_CLOSE |
	AI_COND_NEW_ENEMY |
	AI_COND_SEE_FEAR |
	AI_COND_IN_DANGER |
	AI_COND_LIGHT_DAMAGE |
	AI_COND_BLOCKING_PATH |
	AI_COND_HEAVY_DAMAGE |
	AI_COND_SUSPICIOUS |
	AI_COND_HEAR_SOUND,
	// Sound mask
	AI_SOUND_DANGER |
	AI_SOUND_COMBAT |
	AI_SOUND_PLAYER, 
	// Name
	"Patrol"
);

//=============================================
// @brief Patrol fail
//
//=============================================
ai_task_t taskListSchedulePatrolNPCPatrolFail[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_PATROLNPC_TASK_MOVE_BUDGE,		0),
	AITASK(AI_TASK_WALK_PATH,					0),
	AITASK(AI_TASK_WAIT_FOR_MOVEMENT,			0),
	AITASK(AI_TASK_SET_ACTIVITY,				(Float)ACT_IDLE)
};

const CAISchedule schedulePatrolNPCPatrolFail(
	// Task list
	taskListSchedulePatrolNPCPatrolFail, 
	// Number of tasks
	PT_ARRAYSIZE(taskListSchedulePatrolNPCPatrolFail),
	// AI interrupt mask
	AI_COND_DANGEROUS_ENEMY_CLOSE |
	AI_COND_NEW_ENEMY |
	AI_COND_SEE_FEAR |
	AI_COND_IN_DANGER |
	AI_COND_LIGHT_DAMAGE |
	AI_COND_BLOCKING_PATH |
	AI_COND_HEAVY_DAMAGE |
	AI_COND_SUSPICIOUS |
	AI_COND_HEAR_SOUND,
	// Sound mask
	AI_SOUND_DANGER |
	AI_SOUND_COMBAT |
	AI_SOUND_PLAYER, 
	// Name
	"Patrol Fail"
);

//=============================================
// @brief Patrol prompt
//
//=============================================
ai_task_t taskListSchedulePatrolNPCPatrolPrompt[] = 
{
	AITASK(AI_PATROLNPC_TASK_PATROL_FLUSH,		0)
};

const CAISchedule schedulePatrolNPCPatrolPrompt(
	// Task list
	taskListSchedulePatrolNPCPatrolPrompt, 
	// Number of tasks
	PT_ARRAYSIZE(taskListSchedulePatrolNPCPatrolPrompt),
	// AI interrupt mask
	AI_COND_SCHEDULE_DONE |
	AI_COND_HEAR_SOUND,
	// Sound mask
	AI_SOUND_DANGER |
	AI_SOUND_COMBAT |
	AI_SOUND_PLAYER, 
	// Name
	"Patrol Prompt"
);

//==========================================================================
//
// SCHEDULES FOR CPATROLNPC CLASS
//
//==========================================================================

//=============================================
// @brief Constructor
//
//=============================================
CPatrolNPC::CPatrolNPC( edict_t* pedict ):
	CSquadNPC(pedict),
	m_shouldPatrol(false),
	m_numPatrolHistory(0),
	m_numPatrolErrorPrompts(0),
	m_lastPatrolNodeIndex(NO_POSITION),
	m_myLastPatrolNodeIndex(NO_POSITION),
	m_numPatrolFailures(0),
	m_nextPatrolTime(0),
	m_patrolRadius(0),
	m_nodeRegionName(NO_STRING_VALUE)
{
}

//=============================================
// @brief Destructor
//
//=============================================
CPatrolNPC::~CPatrolNPC( void )
{
}

//=============================================
// @brief Declares save-restore fields
//
//=============================================
void CPatrolNPC::DeclareSaveFields( void )
{
	CSquadNPC::DeclareSaveFields();

	DeclareSaveField(DEFINE_DATA_FIELD(CPatrolNPC, m_shouldPatrol, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD_ARRAY(CPatrolNPC, m_patrolHistoryArray, EFIELD_VECTOR, MAX_PATROL_HISTORY));
	DeclareSaveField(DEFINE_DATA_FIELD(CPatrolNPC, m_numPatrolHistory, EFIELD_UINT32));
	DeclareSaveField(DEFINE_DATA_FIELD_ARRAY(CPatrolNPC, m_squadMembersWithErrorsArray, EFIELD_EHANDLE, MAX_SQUAD_MEMBERS-1));
	DeclareSaveField(DEFINE_DATA_FIELD(CPatrolNPC, m_numPatrolErrorPrompts, EFIELD_UINT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CPatrolNPC, m_lastPatrolNodeIndex, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CPatrolNPC, m_myLastPatrolNodeIndex, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CPatrolNPC, m_numPatrolFailures, EFIELD_UINT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CPatrolNPC, m_nextPatrolTime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CPatrolNPC, m_patrolRadius, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CPatrolNPC, m_nodeRegionName, EFIELD_STRING));
}

//=============================================
// @brief Manages keyvalues
//
//=============================================
bool CPatrolNPC::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "patrol"))
	{
		m_shouldPatrol = (SDL_atoi(kv.value) == 1) ? true : false;
		return true;
	}
	else if(!qstrcmp(kv.keyname, "region"))
	{
		m_nodeRegionName = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "patrolradius"))
	{
		m_patrolRadius = SDL_atof(kv.value);
		return true;
	}
	else
		return CSquadNPC::KeyValue(kv);
}

//=============================================
// @brief Initializes the NPC
//
//=============================================
void CPatrolNPC::InitNPC( void )
{
#if 0
	// For debugging
	if(CPlayerEntity::IsUsingCheatCommand())
		m_shouldPatrol = true;
#endif

	if(m_patrolRadius <= 0)
		m_patrolRadius = DEFAULT_PATROL_RADIUS;

	if(ShouldPatrol())
		m_nextPatrolTime = g_pGameVars->time + Common::RandomFloat(1, 4);

	CSquadNPC::InitNPC();
}

//=============================================
// @brief Validates whether an intended patrol destination should be visited
//
//=============================================
bool CPatrolNPC::ValidatePatrolDestination( const Vector& destPosition )
{
	if(gSkillData.GetSkillLevel() != SKILL_EASY && Common::RandomLong(0, 2) != 2)
	{
		CBaseEntity* pPlayer = Util::GetHostPlayer();
		if(pPlayer)
		{
			Float distance = (pPlayer->GetNavigablePosition() - destPosition).Length2D();
			if(distance > MAX_PATROL_DEST_PLAYER_DISTANCE)
				return false;
		}
	}

	// Get eye position
	Vector eyesPosition = destPosition + m_pState->view_offset;

	// Check each already visited position
	for(Uint32 i = 0; i < MAX_PATROL_HISTORY; i++)
	{
		if(m_patrolHistoryArray[i].IsZero())
			continue;

		// Make sure it's not too near any already visited position
		Float distance = (m_patrolHistoryArray[i] - destPosition).Length();
		if(distance < GetMinimumPatrolDistance())
			return false;

		// Check if it's visible from another patrol position
		if(distance < GetMinimumVisiblePatrolDistance())
		{
			trace_t tr;

			Vector positionLookOrigin = m_patrolHistoryArray[i]+m_pState->view_offset;
			Util::TraceLine(positionLookOrigin, eyesPosition, true, false, m_pEdict, tr);
			if(tr.noHit())
				return false;
		}
	}

	return true;
}

//=============================================
// @brief Returns the ideal schedule
//
//=============================================
const CAISchedule* CPatrolNPC::GetSchedule( void )
{
	switch(m_npcState)
	{
	case NPC_STATE_ALERT:
	case NPC_STATE_IDLE:
		{
			if(ShouldPatrol() && m_nextPatrolTime <= g_pGameVars->time && !CheckConditions(AI_COND_BLOCKING_PATH|AI_COND_HEAR_SOUND))
			{
				// Set time now, so NPC won't try to patrol every frame in case of a failure
				m_nextPatrolTime = g_pGameVars->time + Common::RandomFloat(2, 3);
				return GetScheduleByIndex(AI_PATROLNPC_SCHED_PATROL);
			}
		}
		break;
	}

	return CSquadNPC::GetSchedule();
}

//=============================================
// @brief Returns a schedule by it's index
//
//=============================================
const CAISchedule* CPatrolNPC::GetScheduleByIndex( Int32 scheduleIndex )
{
	switch(scheduleIndex)
	{
	case AI_PATROLNPC_SCHED_PATROL:
		{
			return &schedulePatrolNPCPatrol;
		}
		break;
	case AI_PATROLNPC_SCHED_PATROL_FAIL:
		{
			m_numPatrolFailures++;
			if(m_numPatrolFailures >= MAX_PATROL_FAILURES)
			{
				if(m_myLastPatrolNodeIndex != NO_POSITION)
				{
					CBaseEntity* pSquadLeader = GetSquadLeader();
					if(pSquadLeader)
					{
						const CAINodeGraph::node_t* pNode = gNodeGraph.GetNode(m_myLastPatrolNodeIndex);
						if(pNode)
						{
							pSquadLeader->PushPatrolDestination(pNode->origin);
							pSquadLeader->PatrolErrorPrompt(this);
						}
					}
				}

				// Reset this
				m_numPatrolFailures = 0;
			}

			return &schedulePatrolNPCPatrolFail;
		}
		break;
	case AI_PATROLNPC_SCHED_PATROL_PROMPT:
		{
			return &schedulePatrolNPCPatrolPrompt;
		}
		break;
	}

	return CSquadNPC::GetScheduleByIndex(scheduleIndex);
}
//=============================================
// @brief Cleans up the current scripted_sequence
//
//=============================================
void CPatrolNPC::CleanupScriptedSequence( void )
{
	// Patrol again after five to ten seconds
	m_nextPatrolTime = g_pGameVars->time + Common::RandomFloat(5, 10);

	CSquadNPC::CleanupScriptedSequence();
}

//=============================================
// @brief Starts a task
//
//=============================================
void CPatrolNPC::StartTask( const ai_task_t* pTask )
{
	switch(pTask->task)
	{
	case AI_PATROLNPC_TASK_FIND_DEST:
		{
			if(BuildPatrolPath(GetMinimumPatrolDistance(), m_patrolRadius))
				SetTaskCompleted();
			else
				SetTaskFailed(false);
		}
		break;
	case AI_PATROLNPC_TASK_PATROL_DONE:
		{
			if(m_myLastPatrolNodeIndex == NO_POSITION)
			{
				SetTaskFailed();
				break;
			}

			const CAINodeGraph::node_t* pNode = gNodeGraph.GetNode(m_myLastPatrolNodeIndex);
			if(!pNode)
			{
				SetTaskFailed();
				break;
			}

			CBaseEntity* pSquadLeader = GetSquadLeader();
			if(!pSquadLeader)
			{
				SetTaskFailed();
				break;
			}

			// Have squad leader remember our patrol destination
			pSquadLeader->PushPatrolDestination(pNode->origin);

			// Patrol again in 10 to 20 seconds
			m_nextPatrolTime = g_pGameVars->time + Common::RandomFloat(10, 20);
			SetTaskCompleted();
		}
		break;
	case AI_PATROLNPC_TASK_PATROL_FLUSH:
		{
			CBaseEntity* pSquadLeader = GetSquadLeader();
			if(!pSquadLeader)
			{
				SetTaskFailed();
				break;
			}

			pSquadLeader->PatrolErrorPrompt(this);
			SetTaskCompleted();
		}
		break;
	case AI_PATROLNPC_TASK_MOVE_BUDGE:
		{
			// Try to find an angle out of this stuck position
			for(Float yaw = 0; yaw < 360; yaw += 20)
			{
				// Build move direction
				Vector angles;
				angles[YAW] = yaw;

				Vector forward;
				Math::AngleVectors(angles, &forward);

				// Move the distance
				Float moveDistance = 0;
				Vector finalPosition;
				WalkMoveTrace(m_pState->origin, forward, finalPosition, BUDGE_MOVE_DISTANCE, moveDistance);
				if(moveDistance < MIN_BUDGE_MOVE_DISTANCE)
					continue;

				// See if a node is reachable from here
				if(gNodeGraph.GetNearestNode(finalPosition, this) == NO_POSITION)
					continue;

				// Try to move there
				if(MoveToLocation(ACT_WALK, 0, finalPosition))
				{
					SetTaskCompleted();
					return;
				}
			}

			// Try to find a node then
			if(FindClearNode(nullptr))
				SetTaskCompleted();
			else
				SetTaskFailed(false);
		}
		break;
	default:
		CSquadNPC::StartTask(pTask);
		break;
	}
}

//=============================================
// @brief Set by an NPC if this NPC is blocking them
//
//=============================================
void CPatrolNPC::SetPathBlocked( CBaseEntity* pBlockedEntity, const Vector& destination )
{
	// Patrol again after five to ten seconds
	m_nextPatrolTime = g_pGameVars->time + Common::RandomFloat(5, 10);

	CSquadNPC::SetPathBlocked(pBlockedEntity, destination);
}

//=============================================
// @brief Sets the NPC state
//
//=============================================
void CPatrolNPC::SetNPCState( npcstate_t state )
{
	// If switching from combat to idle/alert, wait for a while before patrolling
	if(m_npcState == NPC_STATE_COMBAT && (state == NPC_STATE_IDLE || state == NPC_STATE_ALERT))
		m_nextPatrolTime = g_pGameVars->time + Common::RandomFloat(5, 10);

	CSquadNPC::SetNPCState(state);
}

//=============================================
// @brief Returns the minimum patrol distance for non-visible destinations
//
//=============================================
Float CPatrolNPC::GetMinimumPatrolDistance( void ) const
{
	return MIN_PATROL_DISTANCE;
}

//=============================================
// @brief Returns the minimum patrol distance for visible destinations
//
//=============================================
Float CPatrolNPC::GetMinimumVisiblePatrolDistance( void ) const
{
	return MIN_VISIBLE_PATROL_DISTANCE;
}

//=============================================
// @brief Pushes a patrol destination to the history stack
//
//=============================================
void CPatrolNPC::PushPatrolDestination( const Vector& destPosition )
{
	if(m_numPatrolHistory >= MAX_PATROL_HISTORY)
		m_numPatrolHistory = 0;

	m_patrolHistoryArray[m_numPatrolHistory] = destPosition;
	m_numPatrolHistory++;
}

//=============================================
// @brief Clears patrol history stack
//
//=============================================
void CPatrolNPC::ClearPatrolHistory( void )
{
	for(Uint32 i = 0; i < MAX_PATROL_HISTORY; i++)
		m_patrolHistoryArray[i].Clear();

	m_numPatrolHistory = 0;

	for(Uint32 i = 0; i < (MAX_SQUAD_MEMBERS-1); i++)
		m_squadMembersWithErrorsArray[i].reset();

	m_numPatrolErrorPrompts = 0;
}

//=============================================
// @brief Prompts a patrol error
//
//=============================================
void CPatrolNPC::PatrolErrorPrompt( CBaseEntity* pEntity )
{
	if(pEntity->GetSquadLeader() != this)
	{
		Util::EntityConDPrintf(m_pEdict, "%s called by non-squad member.\n", __FUNCTION__);
		return;
	}

	for(Uint32 i = 0; i < m_numPatrolErrorPrompts; i++)
	{
		if(m_squadMembersWithErrorsArray[i] == const_cast<const CBaseEntity*>(pEntity))
			return;
	}

	if((m_numPatrolErrorPrompts+1) >= GetNbSquadMembers() || m_numPatrolErrorPrompts >= MAX_SQUAD_MEMBERS)
	{
		ClearPatrolHistory();
		return;
	}

	m_squadMembersWithErrorsArray[m_numPatrolErrorPrompts] = pEntity;
	m_numPatrolErrorPrompts++;
}

//=============================================
// @brief Builds a patrol path
//
//=============================================
bool CPatrolNPC::BuildPatrolPath( Float minDistance, Float maxDistance )
{
	if(!gNodeGraph.IsNodeGraphValid())
	{
		Util::EntityConPrintf(m_pEdict, "%s - Node graph unavailable.\n", __FUNCTION__);
		return false;
	}

	if(maxDistance <= 0)
	{
		Util::EntityConPrintf(m_pEdict, "%s - Max distance was zero.\n", __FUNCTION__);
		return false;
	}

	CBaseEntity* pSquadLeader = GetSquadLeader();
	if(!pSquadLeader)
	{
		Util::EntityConPrintf(m_pEdict, "%s - No squad leader.\n", __FUNCTION__);
		return false;
	}

	// Last patrol node index
	Int32 squadLastPatrolNodeIndex = pSquadLeader->GetLastPatrolNodeIndex();

	// Reset idle search node index if needed
	if(g_lastActiveIdleSearchNodeIndex > gNodeGraph.GetNumNodes())
		g_lastActiveIdleSearchNodeIndex = 0;

	// Make sure min distance is valid
	Float _minDistance = minDistance;
	if(_minDistance > 0.5*maxDistance)
		_minDistance = 0.5*maxDistance;

	// Get node type
	Uint64 nodeType = Util::GetNodeTypeForNPC(this);

	// Get node hull
	node_hull_types_t hullType = Util::GetNodeHullForNPC(this);

	// Get the capability index we'll use
	CAINodeGraph::capability_indexes_t capIndex = gNodeGraph.GetCapabilityIndex(m_capabilityBits);

	// Find nearest node
	Int32 myNode = gNodeGraph.GetNearestNode(m_pState->origin, nodeType, this);
	if(myNode == NO_POSITION)
	{
		Util::EntityConDPrintf(m_pEdict, "%s - No nearest node.\n", __FUNCTION__);
		return false;
	}

	Vector eyesPosition = GetEyePosition();
	
	Int32 lastClosestNodeIndex = NO_POSITION;
	Float lastClosestDistance = maxDistance;

	Int32 numNodes = gNodeGraph.GetNumNodes();
	if(g_lastActiveIdleSearchNodeIndex >= numNodes)
		g_lastActiveIdleSearchNodeIndex = 0;

	for(Int32 i = g_lastActiveIdleSearchNodeIndex; i < numNodes; i++)
	{
		// Make sure someone else hasn't already chosen this node
		if(i == m_myLastPatrolNodeIndex || i == squadLastPatrolNodeIndex)
			continue;

		g_lastActiveIdleSearchNodeIndex = i + 1;
		const CAINodeGraph::node_t* pNode = gNodeGraph.GetNode(i);
		if(!pNode)
			continue;

		// Make sure distance is valid
		Float distance = (m_pState->origin - pNode->origin).Length();
		if(distance < _minDistance || distance > maxDistance)
			continue;

		// Check for node regions
		if(m_nodeRegionName != NO_STRING_VALUE)
		{
			if(qstrcmp(gd_engfuncs.pfnGetString(m_nodeRegionName), pNode->noderegionname))
				continue;
		}

		// Make sure it's reachable
		if(gNodeGraph.GetNextNodeInRoute(myNode, i, hullType, capIndex) == myNode)
			continue;

		// Make sure it's not visible from my current position
		trace_t tr;
		Vector nodeLookOffset = pNode->origin + m_pState->view_offset;
		Util::TraceLine(eyesPosition, nodeLookOffset, true, false, m_pEdict, tr);
		if(tr.noHit())
			continue;

		// See if squad leader validates this position
		if(!pSquadLeader->ValidatePatrolDestination(pNode->origin))
			continue;

		if(distance < lastClosestDistance && CheckRoute(m_pState->origin, pNode->origin))
		{
			lastClosestNodeIndex = i;
			lastClosestDistance = distance;
		}
	}

	if(lastClosestNodeIndex == NO_POSITION)
	{
		pSquadLeader->ClearPatrolHistory();
		return false;
	}

	// Update this
	g_lastActiveIdleSearchNodeIndex = lastClosestNodeIndex + 1;

	// Flush patrol history if we've gone over all the nodes
	if(squadLastPatrolNodeIndex >= gNodeGraph.GetNumNodes())
		pSquadLeader->ClearPatrolHistory();

	// If our node index is larger than the last visited by any other
	// squad member, update it in the squad leader
	if(squadLastPatrolNodeIndex < lastClosestNodeIndex)
		pSquadLeader->SetLastPatrolNodeIndex(lastClosestNodeIndex);

	m_myLastPatrolNodeIndex = lastClosestNodeIndex;

	const CAINodeGraph::node_t* pNode = gNodeGraph.GetNode(m_myLastPatrolNodeIndex);
	if(!pNode)
	{
		Util::EntityConPrintf(m_pEdict, "%s - Failed to get closest node.\n", __FUNCTION__);
		return false;
	}

	if(MoveToLocation(ACT_WALK, 2, pNode->origin))
	{
		// We can go there
		return true;
	}
	else
	{
		// Tell squad leader we had an error
		pSquadLeader->PatrolErrorPrompt(this);
		return false;
	}
}

//=============================================
// @brief Returns the last node index a squad member chose to visit
//
//=============================================
void CPatrolNPC::SetLastPatrolNodeIndex( Int32 nodeIndex )
{
	m_lastPatrolNodeIndex = nodeIndex;
}

//=============================================
// @brief Returns the last node index a squad member chose to visit
//
//=============================================
Int32 CPatrolNPC::GetLastPatrolNodeIndex( void )
{
	return m_lastPatrolNodeIndex;
}

//=============================================
// @brief Tells if NPC should patrol(so non-patrolling 
// npcs that still inherit from this class can override it)
//
//=============================================
bool CPatrolNPC::ShouldPatrol( void )
{
	return m_shouldPatrol;
}