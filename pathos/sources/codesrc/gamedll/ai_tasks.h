/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef AI_TASKS_H
#define AI_TASKS_H

struct ai_task_t
{
	ai_task_t():
		task(0),
		param(0)
		{
		}
	ai_task_t(Int32 _task, Double _param):
		task(_task),
		param(_param)
		{
		}

	Int32 task;
	Double param;
};

#define AITASK(task, param) ai_task_t(task, param)

enum ai_tasks_t
{
	AI_TASK_INVALID = 0,
	AI_TASK_WAIT,
	AI_TASK_WAIT_FACE_ENEMY,
	AI_TASK_WAIT_PVS,
	AI_TASK_SUGGEST_STATE,
	AI_TASK_WALK_TO_TARGET,
	AI_TASK_RUN_TO_TARGET,
	AI_TASK_MOVE_TO_TARGET_RANGE,
	AI_TASK_GET_PATH_TO_ENEMY,
	AI_TASK_GET_PATH_TO_ENEMY_LKP,
	AI_TASK_GET_PATH_TO_ENEMY_CORPSE,
	AI_TASK_GET_PATH_TO_LEADER,
	AI_TASK_GET_PATH_TO_SPOT,
	AI_TASK_GET_PATH_TO_TARGET,
	AI_TASK_GET_PATH_TO_HINT_NODE,
	AI_TASK_GET_PATH_TO_LAST_POSITION,
	AI_TASK_GET_PATH_TO_BEST_SOUND,
	AI_TASK_GET_CLEAR_PATH,
	AI_TASK_RUN_PATH,
	AI_TASK_WALK_PATH,
	AI_TASK_RUN_TO_ATTACK_RANGE,
	AI_TASK_CLEAR_MOVE_WAIT,
	AI_TASK_STORE_LAST_POSITION,
	AI_TASK_CLEAR_LAST_POSITION,
	AI_TASK_PLAY_ACTIVE_IDLE,
	AI_TASK_FIND_HINT_NODE,
	AI_TASK_CLEAR_HINT_NODE,
	AI_TASK_FLINCH,
	AI_TASK_FACE_IDEAL,
	AI_TASK_FACE_ROUTE,
	AI_TASK_FACE_ENEMY,
	AI_TASK_FACE_HINT_NODE,
	AI_TASK_FACE_TARGET,
	AI_TASK_FACE_LAST_POSITION,
	AI_TASK_RANGE_ATTACK1,
	AI_TASK_RANGE_ATTACK2,
	AI_TASK_MELEE_ATTACK1,
	AI_TASK_MELEE_ATTACK2,
	AI_TASK_SPECIAL_ATTACK1,
	AI_TASK_SPECIAL_ATTACK2,
	AI_TASK_RELOAD,
	AI_TASK_RANGE_ATTACK1_NO_TURN,
	AI_TASK_RANGE_ATTACK2_NO_TURN,
	AI_TASK_MELEE_ATTACK1_NO_TURN,
	AI_TASK_MELEE_ATTACK2_NO_TURN,
	AI_TASK_RELOAD_NO_TURN,
	AI_TASK_CROUCH,
	AI_TASK_STAND,
	AI_TASK_GUARD,
	AI_TASK_STEP_LEFT,
	AI_TASK_STEP_RIGHT,
	AI_TASK_STEP_FORWARD,
	AI_TASK_STEP_BACK,
	AI_TASK_DODGE_LEFT,
	AI_TASK_DODGE_RIGHT,
	AI_TASK_SET_ACTIVITY,
	AI_TASK_SET_SCHEDULE,
	AI_TASK_SET_FAIL_SCHEDULE,
	AI_TASK_CLEAR_FAIL_SCHEDULE,
	AI_TASK_PLAY_SEQUENCE,
	AI_TASK_PLAY_SEQUENCE_FACE_ENEMY,
	AI_TASK_PLAY_SEQUENCE_FACE_TARGET,
	AI_TASK_SOUND_IDLE,
	AI_TASK_SOUND_PAIN,
	AI_TASK_SOUND_DIE,
	AI_TASK_FIND_COVER_FROM_BEST_SOUND,
	AI_TASK_FIND_COVER_FROM_ENEMY,
	AI_TASK_FIND_RELOAD_COVER_SPOT,
	AI_TASK_FIND_LATERAL_COVER_FROM_ENEMY,
	AI_TASK_FIND_NODE_COVER_FROM_ENEMY,
	AI_TASK_FIND_NEAR_NODE_COVER_FROM_ENEMY,
	AI_TASK_FIND_FAR_NODE_COVER_FROM_ENEMY,
	AI_TASK_FIND_COVER_FROM_ORIGIN,
	AI_TASK_DIE,
	AI_TASK_DIE_LAND,
	AI_TASK_WAIT_DIE_LAND,
	AI_TASK_WAIT_FOR_SCRIPT,
	AI_TASK_PLAY_SCRIPT,
	AI_TASK_ENABLE_SCRIPT,
	AI_TASK_PLANT_ON_SCRIPT,
	AI_TASK_FACE_SCRIPT,
	AI_TASK_WAIT_RANDOM,
	AI_TASK_WAIT_INDEFINITE,
	AI_TASK_STOP_MOVING,
	AI_TASK_TURN_LEFT,
	AI_TASK_TURN_RIGHT,
	AI_TASK_REMEMBER,
	AI_TASK_FORGET,
	AI_TASK_WAIT_FOR_MOVEMENT,
	AI_TASK_CLEAR_BLOCKED_PATH,
	AI_TASK_CLEAR_BLOCK_STATUS,
	AI_TASK_FIND_ENEMY_SEARCH_SPOT,
	AI_TASK_WAIT_BLEND,
	AI_TASK_FACE_PLAYER,
	AI_TASK_WAIT_FACE_ENEMY_INDEFINITE,
	AI_TASK_FACE_BEST_SOUND,
	AI_TASK_DODGE_ENEMY,
	AI_TASK_ALERT_SOUND,
	AI_TASK_SET_CHECK_BEST_SOUND,
	AI_TASK_SET_NEXT_SCHEDULE,
	AI_TASK_ATTACK_REACTION_DELAY,
	AI_TASK_GET_PATH_AWAY_FROM_NPCPULLER,
	AI_TASK_NPCPULLER_HOVER,
	AI_TASK_FACE_TOSS_DIR,

	// Must be last
	LAST_BASENPC_TASK
};
#endif //AI_TASKS_H