/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef AI_WANDERNPC_H
#define AI_WANDERNPC_H

#include "ai_talknpc.h"

//=============================================
// Wander npc specific tasks
//=============================================
enum wandernpc_tasks_t
{
	AI_WANDERNPC_TASK_FIND_DEST = LAST_TALKNPC_TASK + 1,
	AI_WANDERNPC_TASK_GET_NODE_IDEAL_YAW,
	AI_WANDERNPC_TASK_RANDOM_WAIT,
	AI_WANDERNPC_TASK_BEGIN_REST,
	AI_WANDERNPC_TASK_DONE_RESTING,
	AI_WANDERNPC_TASK_FACE_PLAYER,
	AI_WANDERNPC_TASK_SET_TARGET,
	AI_WANDERNPC_TASK_UNSET_TARGET,
	AI_WANDERNPC_TASK_CONVERSE_PLAYER,
	AI_WANDERNPC_TASK_REST,

	// Must be last
	LAST_WANDERNPC_TASK
};

//=============================================
// Wander npc specific schedules
//=============================================
enum wandernpc_schedules_t
{
	AI_WANDERNPC_SCHED_WANDER = LAST_TALKNPC_SCHEDULE + 1,
	AI_WANDERNPC_SCHED_SIT,
	AI_WANDERNPC_SCHED_FIND_SITTING_SPOT,
	AI_WANDERNPC_SCHED_WALK_TO_WINDOW,
	AI_WANDERNPC_SCHED_FIND_PLAYER,

	LAST_WANDERNPC_SCHEDULE
};

//=============================================
//
//=============================================
class CWanderNPC : public CTalkNPC
{
public:
	// Speed at which tiredness fades
	static const Float WANDERNPC_REST_SPEED;
	// Time after which we'll try to find the player to talk to him
	static const Float WANDERNPC_PLAYER_ABSENCE_TIME_TRESHOLD;
	// Amount of tiredness after which we'll try to sit down to rest
	static const Float WANDERNPC_TIREDNESS_TRESHOLD;
	// Maximum tiredness we're willing to take
	static const Float WANDERNPC_TIREDNESS_LIMIT;
	// Speed at which the NPC tires
	static const Float WANDERNPC_TIRE_SPEED;
	// Distance at which the player is considered to be "close"
	static const Float WANDERNPC_PLAYER_CLOSE_DISTANCE;
	// Minimum distance between node destinations for wander npcs
	static const Float WANDERNPC_MIN_DESTINATION_DISTANCE;
	// Minimum distance to a wander spot
	static const Float WANDERNPC_MIN_WANDER_DISTANCE;
	// Maximum distance to a wander spot
	static const Float WANDERNPC_MAX_WANDER_DISTANCE;
	// Max retries before we give up on finding a destination
	static const Uint32 WANDERNPC_MAX_RETRIES;

public:
	explicit CWanderNPC( edict_t* pedict );
	virtual ~CWanderNPC( void );

public:
	// Declares save-restore fields
	virtual void DeclareSaveFields( void ) override;
	// Manages keyvalues
	virtual bool KeyValue( const keyvalue_t& kv ) override;

public:
	// Initializes the NPC
	virtual void InitNPC( void ) override;
	// Runs AI code
	virtual void RunAI( void ) override;
	// Performs pre-schedule think functions
	virtual void PreScheduleThink( void ) override;
	// Tells if this is a wandering NPC
	virtual bool IsWanderNPC( void ) override { return true; }
	// Returns the wander node used
	virtual Int32 GetWanderNode( void ) override;

	// Returns the ideal schedule
	virtual const CAISchedule* GetSchedule( void ) override;
	// Returns a schedule by it's index
	virtual const CAISchedule* GetScheduleByIndex( Int32 scheduleIndex ) override;

	// Sets the NPC state
	virtual void SetNPCState( npcstate_t state ) override;
	// Cleans up the current scripted_sequence
	virtual void CleanupScriptedSequence( void ) override;

	// Starts a task
	virtual void StartTask( const ai_task_t* pTask ) override;
	// Runs a task
	virtual void RunTask( const ai_task_t* pTask ) override;

	// Tells if talking npc can answer
	virtual bool CanAnswer( void ) override;

	// Sets wander state
	virtual void SetWanderState( bool state ) override;
	// Returns the wander state
	virtual bool GetWanderState( void ) override;

public:
	// Finds a wander destination
	bool FindWanderDestination( wandernpc_dest_type_t type, Uint32 numTries );
	// Tells if a node is available for wandering
	bool IsWanderNodeAvailable( Int32 nodeIndex ) const;

protected:
	// Time until we wander again
	Double m_nextWanderTime;
	// Last time we saw the player
	Double m_wanderLastPlayerSightTime;
	// Tiredness factor
	Float m_tirednessFactor;

	// Node region name(if any)
	string_t m_nodeRegionName;
	// Destination node index
	Int32 m_wanderDestinationNodeIndex;
	// Last player origin we checked at
	Vector m_lastWanderPlayerOrigin;

	// TRUE if resting
	bool m_isResting;
	// TRUE if npc should wander
	bool m_shouldWander;
};
#endif //AI_WANDERNPC_H