/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "ai_wandernpc.h"
#include "ai_nodegraph.h"

// Speed at which tiredness fades
const Float CWanderNPC::WANDERNPC_REST_SPEED = 0.4;
// Time after which we'll try to find the player to talk to him
const Float CWanderNPC::WANDERNPC_PLAYER_ABSENCE_TIME_TRESHOLD = 50.0f;
// Amount of tiredness after which we'll try to sit down to rest
const Float CWanderNPC::WANDERNPC_TIREDNESS_TRESHOLD = 10.0f;
// Maximum tiredness we're willing to take
const Float CWanderNPC::WANDERNPC_TIREDNESS_LIMIT = 40.0f;
// Speed at which the NPC tires
const Float CWanderNPC::WANDERNPC_TIRE_SPEED = 1.0f/30.0f;
// Distance at which the player is considered to be "close"
const Float CWanderNPC::WANDERNPC_PLAYER_CLOSE_DISTANCE = 128;
// Minimum distance between node destinations for wander npcs
const Float CWanderNPC::WANDERNPC_MIN_DESTINATION_DISTANCE = 256.0f;
// Minimum distance to a wander spot
const Float CWanderNPC::WANDERNPC_MIN_WANDER_DISTANCE = 256.0f;
// Maximum distance to a wander spot
const Float CWanderNPC::WANDERNPC_MAX_WANDER_DISTANCE = 4096.0f;
// Max retries for finding a destination
const Uint32 CWanderNPC::WANDERNPC_MAX_RETRIES = 2;

//==========================================================================
//
// SCHEDULES FOR CWANDERNPC CLASS
//
//==========================================================================

//=============================================
// @brief Wander
//
//=============================================
ai_task_t taskListScheduleWanderNPCWander[] = 
{
	AITASK(AI_TASK_STOP_MOVING,							0),
	AITASK(AI_WANDERNPC_TASK_FIND_DEST,					(Float)WANDER_NPC_DEST_WANDER),
	AITASK(AI_TASK_WALK_PATH,							0),
	AITASK(AI_TASK_WAIT_FOR_MOVEMENT,					0),
	AITASK(AI_TASK_SET_ACTIVITY,						(Float)ACT_IDLE),
	AITASK(AI_TASK_WAIT,								4)
};

const CAISchedule scheduleWanderNPCWander(
	// Task list
	taskListScheduleWanderNPCWander, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleWanderNPCWander),
	// AI interrupt mask
	AI_COND_DANGEROUS_ENEMY_CLOSE |
	AI_COND_NEW_ENEMY |
	AI_COND_SEE_FEAR |
	AI_COND_LIGHT_DAMAGE |
	AI_COND_HEAVY_DAMAGE |
	AI_COND_BLOCKING_PATH |
	AI_COND_HEAR_SOUND,
	// Sound mask
	AI_SOUND_DANGER |
	AI_SOUND_COMBAT, 
	// Name
	"Wander"
);

//=============================================
// @brief Find sitting spot
//
//=============================================
ai_task_t taskListScheduleWanderNPCFindSittingSpot[] = 
{
	AITASK(AI_TASK_STOP_MOVING,							0),
	AITASK(AI_WANDERNPC_TASK_FIND_DEST,					(Float)WANDER_NPC_DEST_SIT),
	AITASK(AI_TASK_WALK_PATH,							0),
	AITASK(AI_TASK_WAIT_FOR_MOVEMENT,					0),
	AITASK(AI_TASK_WAIT,								0.3),
	AITASK(AI_TASK_SET_SCHEDULE,						(Float)AI_WANDERNPC_SCHED_SIT)
};

const CAISchedule scheduleWanderNPCFindSittingSpot(
	// Task list
	taskListScheduleWanderNPCFindSittingSpot, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleWanderNPCFindSittingSpot),
	// AI interrupt mask
	AI_COND_DANGEROUS_ENEMY_CLOSE |
	AI_COND_NEW_ENEMY |
	AI_COND_SEE_FEAR |
	AI_COND_LIGHT_DAMAGE |
	AI_COND_HEAVY_DAMAGE |
	AI_COND_BLOCKING_PATH |
	AI_COND_HEAR_SOUND,
	// Sound mask
	AI_SOUND_DANGER |
	AI_SOUND_COMBAT, 
	// Name
	"Find sitting spot"
);

//=============================================
// @brief Walk to window
//
//=============================================
ai_task_t taskListScheduleWanderNPCWalkToWindow[] = 
{
	AITASK(AI_TASK_STOP_MOVING,							0),
	AITASK(AI_WANDERNPC_TASK_FIND_DEST,					(Float)WANDER_NPC_DEST_WINDOW),
	AITASK(AI_TASK_WALK_PATH,							0),
	AITASK(AI_TASK_WAIT_FOR_MOVEMENT,					0),
	AITASK(AI_TASK_WAIT,								0.1),
	AITASK(AI_WANDERNPC_TASK_GET_NODE_IDEAL_YAW,		0),
	AITASK(AI_TASK_FACE_IDEAL,							0),
	AITASK(AI_TASK_SET_ACTIVITY,						(Float)ACT_IDLE),
	AITASK(AI_WANDERNPC_TASK_RANDOM_WAIT,				0)
};

const CAISchedule scheduleWanderNPCWalkToWindow(
	// Task list
	taskListScheduleWanderNPCWalkToWindow, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleWanderNPCWalkToWindow),
	// AI interrupt mask
	AI_COND_DANGEROUS_ENEMY_CLOSE |
	AI_COND_NEW_ENEMY |
	AI_COND_SEE_FEAR |
	AI_COND_LIGHT_DAMAGE |
	AI_COND_HEAVY_DAMAGE |
	AI_COND_BLOCKING_PATH |
	AI_COND_HEAR_SOUND,
	// Sound mask
	AI_SOUND_DANGER |
	AI_SOUND_COMBAT, 
	// Name
	"Walk to window"
);

//=============================================
// @brief Sit
//
//=============================================
ai_task_t taskListScheduleWanderNPCSit[] = 
{
	AITASK(AI_TASK_STOP_MOVING,							0),
	AITASK(AI_WANDERNPC_TASK_GET_NODE_IDEAL_YAW,		(Float)AI_WANDERNPC_SCHED_SIT),
	AITASK(AI_TASK_FACE_IDEAL,							0),
	AITASK(AI_TASK_PLAY_SEQUENCE,						(Float)ACT_CROUCH),
	AITASK(AI_WANDERNPC_TASK_BEGIN_REST,				0),
	AITASK(AI_TASK_SET_ACTIVITY,						(Float)ACT_CROUCH_IDLE),
	AITASK(AI_WANDERNPC_TASK_REST,						0),
	AITASK(AI_WANDERNPC_TASK_DONE_RESTING,				0),
	AITASK(AI_TASK_PLAY_SEQUENCE,						(Float)ACT_STAND)
};

const CAISchedule scheduleWanderNPCSit(
	// Task list
	taskListScheduleWanderNPCSit, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleWanderNPCSit),
	// AI interrupt mask
	AI_COND_SCHEDULE_DONE,
	// Sound mask
	AI_SOUND_NONE, 
	// Name
	"Sit"
);

//=============================================
// @brief Find player
//
//=============================================
ai_task_t taskListScheduleWanderNPCFindPlayer[] = 
{
	AITASK(AI_TASK_STOP_MOVING,							0),
	AITASK(AI_WANDERNPC_TASK_SET_TARGET,				0),
	AITASK(AI_TASK_MOVE_TO_TARGET_RANGE,				0),
	AITASK(AI_WANDERNPC_TASK_UNSET_TARGET,				0),
	AITASK(AI_TASK_WAIT,								0.5),
	AITASK(AI_WANDERNPC_TASK_FACE_PLAYER,				0),
	AITASK(AI_TASK_FACE_IDEAL,							0),
	AITASK(AI_TASK_SET_ACTIVITY,						(Float)ACT_IDLE),
	AITASK(AI_WANDERNPC_TASK_CONVERSE_PLAYER,			0),
	AITASK(AI_TASK_WAIT,								5)
};

const CAISchedule scheduleWanderNPCFindPlayer(
	// Task list
	taskListScheduleWanderNPCFindPlayer, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleWanderNPCFindPlayer),
	// AI interrupt mask
	AI_COND_DANGEROUS_ENEMY_CLOSE |
	AI_COND_NEW_ENEMY |
	AI_COND_SEE_FEAR |
	AI_COND_LIGHT_DAMAGE |
	AI_COND_HEAVY_DAMAGE |
	AI_COND_BLOCKING_PATH |
	AI_COND_HEAR_SOUND,
	// Sound mask
	AI_SOUND_DANGER |
	AI_SOUND_COMBAT, 
	// Name
	"Find Player"
);

//==========================================================================
//
// SCHEDULES FOR CWANDERNPC CLASS
//
//==========================================================================

//=============================================
// @brief Constructor
//
//=============================================
CWanderNPC::CWanderNPC( edict_t* pedict ):
	CTalkNPC(pedict),
	m_nextWanderTime(0),
	m_wanderLastPlayerSightTime(0),
	m_tirednessFactor(0),
	m_nodeRegionName(NO_STRING_VALUE),
	m_wanderDestinationNodeIndex(NO_POSITION),
	m_isResting(false),
	m_shouldWander(false)
{
}

//=============================================
// @brief Destructor
//
//=============================================
CWanderNPC::~CWanderNPC( void )
{
}

//=============================================
// @brief Declares save-restore fields
//
//=============================================
void CWanderNPC::DeclareSaveFields( void )
{
	CTalkNPC::DeclareSaveFields();

	DeclareSaveField(DEFINE_DATA_FIELD(CWanderNPC, m_nextWanderTime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CWanderNPC, m_wanderLastPlayerSightTime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CWanderNPC, m_tirednessFactor, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CWanderNPC, m_wanderDestinationNodeIndex, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CWanderNPC, m_nodeRegionName, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CWanderNPC, m_shouldWander, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CWanderNPC, m_isResting, EFIELD_BOOLEAN));
}

//=============================================
// @brief Manages keyvalues
//
//=============================================
bool CWanderNPC::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "wander"))
	{
		m_shouldWander = (SDL_atoi(kv.value) == 1) ? true : false;
		return true;
	}
	else if(!qstrcmp(kv.keyname, "region"))
	{
		m_nodeRegionName = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else
		return CTalkNPC::KeyValue(kv);
}

//=============================================
// @brief Initializes the NPC
//
//=============================================
void CWanderNPC::InitNPC( void )
{
	m_nextWanderTime = g_pGameVars->time + Common::RandomFloat(30, 70);
	m_lastPlayerSightTime = g_pGameVars->time;

	m_tirednessFactor = 0;
	m_wanderDestinationNodeIndex = NO_POSITION;

	CTalkNPC::InitNPC();
}

//=============================================
// @brief Returns the ideal schedule
//
//=============================================
const CAISchedule* CWanderNPC::GetSchedule( void )
{
	switch(m_npcState)
	{
	case NPC_STATE_ALERT:
	case NPC_STATE_IDLE:
		{
			if(m_isResting)
			{
				// Most likely a save-restore
				return GetScheduleByIndex(AI_WANDERNPC_SCHED_SIT);
			}
			else if(m_shouldWander && (m_nextWanderTime < g_pGameVars->time) 
				&& Common::RandomLong(0, 3) == 1 && !CheckConditions(AI_COND_PLAYER_CLOSE))
			{
				if(!CheckConditions(AI_COND_SEE_CLIENT) && (g_pGameVars->time - m_wanderLastPlayerSightTime) > WANDERNPC_PLAYER_ABSENCE_TIME_TRESHOLD)
				{
					Float playerSeekChance = (g_pGameVars->time - m_wanderLastPlayerSightTime)/WANDERNPC_PLAYER_ABSENCE_TIME_TRESHOLD;
					if(Common::RandomFloat(0, 4) <= playerSeekChance)
					{
						m_nextWanderTime = g_pGameVars->time + Common::RandomLong(20, 40);
						return GetScheduleByIndex(AI_WANDERNPC_SCHED_FIND_PLAYER);
					}
				}
				
				// Opportunistically rest a bit
				if(m_tirednessFactor > WANDERNPC_TIREDNESS_TRESHOLD)
				{
					Float tirednessFactor = (m_tirednessFactor-WANDERNPC_TIREDNESS_TRESHOLD)/WANDERNPC_TIREDNESS_LIMIT;
					if(Common::RandomFloat(0, 1) >= (1.0f - tirednessFactor))
					{
						m_nextWanderTime = g_pGameVars->time + Common::RandomLong(20, 40);
						return GetScheduleByIndex(AI_WANDERNPC_SCHED_FIND_SITTING_SPOT);
					}
				}

				// Wander anywhere you want
				m_nextWanderTime = g_pGameVars->time + Common::RandomLong(20, 40);
				if(Common::RandomLong(0, 3) == 1)
					return GetScheduleByIndex(AI_WANDERNPC_SCHED_WALK_TO_WINDOW);
				else
					return GetScheduleByIndex(AI_WANDERNPC_SCHED_WANDER);
			}
		}
		break;
	}

	return CTalkNPC::GetSchedule();
}

//=============================================
// @brief Returns a schedule by it's index
//
//=============================================
const CAISchedule* CWanderNPC::GetScheduleByIndex( Int32 scheduleIndex )
{
	switch(scheduleIndex)
	{
	case AI_WANDERNPC_SCHED_SIT:
		{
			return &scheduleWanderNPCSit;
		}
		break;
	case AI_WANDERNPC_SCHED_FIND_SITTING_SPOT:
		{
			return &scheduleWanderNPCFindSittingSpot;
		}
		break;
	case AI_WANDERNPC_SCHED_WALK_TO_WINDOW:
		{
			return &scheduleWanderNPCWalkToWindow;
		}
		break;
	case AI_WANDERNPC_SCHED_WANDER:
		{
			return &scheduleWanderNPCWander;
		}
		break;
	case AI_WANDERNPC_SCHED_FIND_PLAYER:
		{
			return &scheduleWanderNPCFindPlayer;
		}
		break;
	}

	return CTalkNPC::GetScheduleByIndex(scheduleIndex);
}

//=============================================
// @brief Runs AI code
//
//=============================================
void CWanderNPC::RunAI( void )
{
	if(!m_isResting)
	{
		m_tirednessFactor += WANDERNPC_TIRE_SPEED*m_thinkIntervalTime;

		const Float maxTiredness = (WANDERNPC_TIREDNESS_TRESHOLD+WANDERNPC_TIREDNESS_LIMIT);
		if(m_tirednessFactor > maxTiredness)
			m_tirednessFactor = maxTiredness;
	}

	if(CheckConditions(AI_COND_SEE_CLIENT))
		m_wanderLastPlayerSightTime = g_pGameVars->time;

	CTalkNPC::RunAI();
}

//=============================================
// @brief Performs pre-schedule think functions
//
//=============================================
void CWanderNPC::PreScheduleThink( void )
{
	CBaseEntity* pPlayer = Util::GetHostPlayer();
	if(pPlayer)
	{
		Float playerDistance = (pPlayer->GetOrigin()-m_pState->origin).Length2D();
		if(playerDistance < WANDERNPC_PLAYER_CLOSE_DISTANCE)
			SetConditions(AI_COND_PLAYER_CLOSE);
		else
			ClearConditions(AI_COND_PLAYER_CLOSE);
	}

	CTalkNPC::PreScheduleThink();
}

//=============================================
// @brief Sets the NPC state
//
//=============================================
void CWanderNPC::SetNPCState( npcstate_t state )
{
	if((m_npcState == NPC_STATE_COMBAT || m_npcState == NPC_STATE_SCRIPT)
		&& (state == NPC_STATE_IDLE || state == NPC_STATE_ALERT))
		m_nextWanderTime = g_pGameVars->time + Common::RandomFloat(10, 20);

	CTalkNPC::SetNPCState(state);
}

//=============================================
// @brief Cleans up the current scripted_sequence
//
//=============================================
void CWanderNPC::CleanupScriptedSequence( void )
{
	m_nextWanderTime = g_pGameVars->time + 1;
	m_isResting = false;

	CTalkNPC::CleanupScriptedSequence();
}

//=============================================
// @brief Starts a task
//
//=============================================
void CWanderNPC::StartTask( const ai_task_t* pTask )
{
	switch(pTask->task)
	{
	case AI_WANDERNPC_TASK_REST:
		break;
	case AI_WANDERNPC_TASK_CONVERSE_PLAYER:
		{
			CBaseEntity* pPlayer = Util::GetHostPlayer();
			if(!pPlayer)
			{
				SetTaskFailed();
				break;
			}

			PlaySentence(m_sentenceGroupNames[TALKNPC_GRP_CONVERSE].c_str(), 0, VOL_NORM, ATTN_NORM, 0, true);

			m_talkTargetEntity = pPlayer;
			SetTaskCompleted();
		}
		break;
	case AI_WANDERNPC_TASK_SET_TARGET:
		{
			CBaseEntity* pPlayer = Util::GetHostPlayer();
			if(!pPlayer)
			{
				SetTaskFailed();
				break;
			}

			m_targetEntity = pPlayer;
			SetTaskCompleted();
		}
		break;
	case AI_WANDERNPC_TASK_UNSET_TARGET:
		{
			m_targetEntity.reset();
			SetTaskCompleted();
		}
		break;
	case AI_WANDERNPC_TASK_FIND_DEST:
		{
			wandernpc_dest_type_t type = (wandernpc_dest_type_t)(Int32)pTask->param;
			if(FindWanderDestination(type, 0))
				SetTaskCompleted();
			else
				SetTaskFailed(false);
		}
		break;
	case AI_WANDERNPC_TASK_GET_NODE_IDEAL_YAW:
		{
			if(m_wanderDestinationNodeIndex == NO_POSITION)
			{
				SetTaskFailed();
				break;
			}

			const CAINodeGraph::node_t* pNode = gNodeGraph.GetNode(m_wanderDestinationNodeIndex);
			if(!pNode)
			{
				SetTaskFailed();
				break;
			}

			m_pState->idealyaw = pNode->hintyaw;
			SetTaskCompleted();
		}
		break;
	case AI_WANDERNPC_TASK_RANDOM_WAIT:
		{
			m_waitFinishedTime = g_pGameVars->time + Common::RandomFloat(15, 40);
		}
		break;
	case AI_WANDERNPC_TASK_BEGIN_REST:
		{
			m_isResting = true;
			SetTaskCompleted();
		}
		break;
	case AI_WANDERNPC_TASK_DONE_RESTING:
		{
			m_isResting = false;
			SetTaskCompleted();
		}
		break;
	case AI_WANDERNPC_TASK_FACE_PLAYER:
		{
			CBaseEntity* pPlayer = Util::GetHostPlayer();
			if(!pPlayer)
			{
				SetTaskFailed();
				break;
			}

			m_updateYaw = true;
			SetIdealYaw(pPlayer->GetOrigin());
			SetTaskCompleted();
		}
		break;
	default:
		CTalkNPC::StartTask(pTask);
		break;
	}
}

//=============================================
// @brief Runs a task
//
//=============================================
void CWanderNPC::RunTask( const ai_task_t* pTask )
{
	switch(pTask->task)
	{
	case AI_WANDERNPC_TASK_REST:
		{
			m_tirednessFactor -= WANDERNPC_REST_SPEED * m_thinkIntervalTime;
			if(m_tirednessFactor <= 0)
			{
				m_tirednessFactor = 0;
				SetTaskCompleted();
			}
		}
		break;
	case AI_WANDERNPC_TASK_RANDOM_WAIT:
		{
			if(m_waitFinishedTime <= g_pGameVars->time)
				SetTaskCompleted();
		}
		break;
	default:
		CTalkNPC::RunTask(pTask);
		break;
	}
}

//=============================================
// @brief Tells if talking npc can answer
//
//=============================================
bool CWanderNPC::CanAnswer( void )
{
	if(m_isResting)
		return false;
	else
		return CTalkNPC::CanAnswer();
}

//=============================================
// @brief Finds a wander destination
//
//=============================================
bool CWanderNPC::FindWanderDestination( wandernpc_dest_type_t type, Uint32 numTries )
{
	if(!gNodeGraph.IsNodeGraphValid())
	{
		Util::EntityConPrintf(m_pEdict, "Node graph not ready.\n");
		return false;
	}

	if(g_lastActiveIdleSearchNodeIndex >= gNodeGraph.GetNumNodes())
		g_lastActiveIdleSearchNodeIndex = 0;

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
		Util::EntityConPrintf(m_pEdict, "%s - No nearest node.\n", __FUNCTION__);
		return false;
	}

	// Get eye position
	Vector eyesPosition = GetEyePosition();

	for(Int32 i = g_lastActiveIdleSearchNodeIndex; i < gNodeGraph.GetNumNodes(); i++)
	{
		const CAINodeGraph::node_t* pNode = gNodeGraph.GetNode(i);
		if(!pNode)
			continue;

		switch(type)
		{
		case WANDER_NPC_DEST_WANDER:
			{
				if(pNode->hinttype != NODE_HINT_NONE)
					continue;
			}
			break;
		case WANDER_NPC_DEST_WINDOW:
			{
				if(pNode->hinttype != NODE_HINT_WINDOW_SPOT)
					continue;
			}
			break;
		case WANDER_NPC_DEST_SIT:
			{
				if(pNode->hinttype != NODE_HINT_SITTING_SPOT)
					continue;
			}
			break;
		default:
			{
				Util::EntityConPrintf(m_pEdict, "Unknown type %d specified.\n", (Int32)type);
				return false;
			}
			break;
		}

		// Make sure someone else isn't targeting this node
		if(!IsWanderNodeAvailable(i))
			continue;

		// Make sure we can actually go there
		if(gNodeGraph.GetNextNodeInRoute(myNode, i, hullType, capIndex) == myNode)
			continue;

		// Check distance
		Float distance = (m_pState->origin - pNode->origin).Length();
		if(distance < WANDERNPC_MIN_WANDER_DISTANCE || distance > WANDERNPC_MAX_WANDER_DISTANCE)
			continue;

		// Only look up visible spots when looking to rest
		if(type != WANDER_NPC_DEST_SIT)
		{
			trace_t tr;
			Vector nodeEyesPosition = pNode->origin + m_pState->view_offset;
			Util::TraceLine(eyesPosition, nodeEyesPosition, true, false, m_pEdict, tr);
			if(tr.noHit())
				continue;
		}

		if(MoveToLocation(ACT_WALK, 0, pNode->origin))
		{
			g_lastActiveIdleSearchNodeIndex = i + 1;
			m_wanderDestinationNodeIndex = i;
			return true;
		}
	}

	// Fail if we exhausted our retries
	if(numTries >= WANDERNPC_MAX_RETRIES)
		return false;

	// Try finding again
	return FindWanderDestination(type, ++numTries);
}

//=============================================
// @brief Sets wander state
//
//=============================================
void CWanderNPC::SetWanderState( bool state )
{
	m_shouldWander = state;
}

//=============================================
// @brief Returns the wander state
//
//=============================================
bool CWanderNPC::GetWanderState( void )
{
	return m_shouldWander;
}

//=============================================
// @brief Tells if a node is available for wandering
//
//=============================================
bool CWanderNPC::IsWanderNodeAvailable( Int32 nodeIndex ) const
{
	if(nodeIndex == NO_POSITION)
		return false;

	const CAINodeGraph::node_t* pNode = gNodeGraph.GetNode(nodeIndex);
	if(!pNode)
		return false;

	edict_t* pedict = nullptr;
	for(Int32 i = 1; i < g_pGameVars->numentities; i++)
	{
		pedict = gd_engfuncs.pfnGetEdictByIndex(i);
		if(Util::IsNullEntity(pedict))
			continue;

		CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
		if(!pEntity || !pEntity->IsNPC() || !pEntity->IsWanderNPC() 
			|| pEntity->GetClassification() != GetClassification())
			continue;

		if(pEntity->GetWanderNode() == nodeIndex)
			return false;

		Float distance = (pNode->origin - pEntity->GetOrigin()).Length();
		if(distance < WANDERNPC_MIN_DESTINATION_DISTANCE)
			return false;
	}

	return true;
}

//=============================================
// @brief Returns the wander node used
//
//=============================================
Int32 CWanderNPC::GetWanderNode( void )
{
	return m_wanderDestinationNodeIndex;
}
