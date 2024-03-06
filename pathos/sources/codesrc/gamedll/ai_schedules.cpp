/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "ai_basenpc.h"
#include "envfog.h"
#include "ai_schedule.h"

//==========================================================================
//
// SCHEDULES FOR CBASENPC CLASS
//
//==========================================================================

//=============================================
// @brief Generic failure schedule
//
//=============================================
ai_task_t taskListScheduleFail[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_SET_ACTIVITY,				(Float)ACT_IDLE),
	AITASK(AI_TASK_WAIT,						0.5f),
	AITASK(AI_TASK_WAIT_PVS,					0)
};

const CAISchedule scheduleFail(
	// Task list
	taskListScheduleFail, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleFail),
	// AI interrupt mask
	AI_COND_DANGEROUS_ENEMY_CLOSE |
	AI_COND_CAN_ATTACK,
	// Sound mask
	AI_SOUND_NONE, 
	// Name
	"Fail"
);

//=============================================
// @brief Generic failure schedule
//
//=============================================
ai_task_t taskListScheduleCombatFail[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_SET_ACTIVITY,				(Float)ACT_IDLE),
	AITASK(AI_TASK_FACE_ENEMY,					0),
	AITASK(AI_TASK_WAIT,						0.5f),
	AITASK(AI_TASK_WAIT_PVS,					0)
};

const CAISchedule scheduleCombatFail(
	// Task list
	taskListScheduleCombatFail, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleCombatFail),
	// AI interrupt mask
	AI_COND_DANGEROUS_ENEMY_CLOSE |
	AI_COND_CAN_ATTACK,
	// Sound mask
	AI_SOUND_NONE, 
	// Name
	"Combat Fail"
);

//=============================================
// @brief Idle stand schedule
//
//=============================================
ai_task_t taskListScheduleIdleStand[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_SET_ACTIVITY,				(Float)ACT_IDLE),
	AITASK(AI_TASK_WAIT,						(Float)5)
};

const CAISchedule scheduleIdleStand(
	// Task list
	taskListScheduleIdleStand, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleIdleStand),
	// AI interrupt mask
	AI_COND_NEW_ENEMY | 
	AI_COND_SEE_DISLIKE | 
	AI_COND_SEE_FEAR | 
	AI_COND_LIGHT_DAMAGE |
	AI_COND_HEAVY_DAMAGE | 
	AI_COND_HEAR_SOUND | 
	AI_COND_FOLLOW_TARGET_TOO_FAR |
	AI_COND_BLOCKING_PATH | 
	AI_COND_PROVOKED,
	// Sound mask
	AI_SOUND_COMBAT | 
	AI_SOUND_WORLD | 
	AI_SOUND_PLAYER | 
	AI_SOUND_DANGER, 
	// Name
	"Idle Stand"
);

//=============================================
// @brief Idle walk schedule
//
//=============================================
ai_task_t taskListScheduleIdleWalk[] = 
{
	AITASK(AI_TASK_WALK_PATH,					0),
	AITASK(AI_TASK_WAIT_FOR_MOVEMENT,			0)
};

const CAISchedule scheduleIdleWalk(
	// Task list
	taskListScheduleIdleWalk, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleIdleWalk),
	// AI interrupt mask
	AI_COND_NEW_ENEMY | 
	AI_COND_LIGHT_DAMAGE |
	AI_COND_HEAVY_DAMAGE | 
	AI_COND_HEAR_SOUND | 
	AI_COND_FOLLOW_TARGET_TOO_FAR |
	AI_COND_BLOCKING_PATH | 
	AI_COND_PROVOKED,
	// Sound mask
	AI_SOUND_COMBAT |  
	AI_SOUND_DANGER, 
	// Name
	"Idle Walk"
);

//=============================================
// @brief Ambush schedule
//
//=============================================
ai_task_t taskListScheduleAmbush[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_SET_ACTIVITY,				(Float)ACT_IDLE),
	AITASK(AI_TASK_WAIT_INDEFINITE,				0)
};

const CAISchedule scheduleAmbush(
	// Task list
	taskListScheduleAmbush, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleAmbush),
	// AI interrupt mask
	AI_COND_NEW_ENEMY | 
	AI_COND_SEE_ENEMY |
	AI_COND_LIGHT_DAMAGE |
	AI_COND_HEAVY_DAMAGE | 
	AI_COND_PROVOKED,
	// Sound mask
	AI_SOUND_NONE, 
	// Name
	"Ambush"
);

//=============================================
// @brief Active idle schedule
//
//=============================================
ai_task_t taskListScheduleActiveIdle[] = 
{
	AITASK(AI_TASK_FIND_HINT_NODE,				0),
	AITASK(AI_TASK_GET_PATH_TO_HINT_NODE,		0),
	AITASK(AI_TASK_STORE_LAST_POSITION,			0),
	AITASK(AI_TASK_WALK_PATH,					0),
	AITASK(AI_TASK_WAIT_FOR_MOVEMENT,			0),
	AITASK(AI_TASK_FACE_HINT_NODE,				0),
	AITASK(AI_TASK_PLAY_ACTIVE_IDLE,			0),
	AITASK(AI_TASK_GET_PATH_TO_LAST_POSITION,	0),
	AITASK(AI_TASK_WALK_PATH,					0),
	AITASK(AI_TASK_WAIT_FOR_MOVEMENT,			0),
	AITASK(AI_TASK_CLEAR_LAST_POSITION,			0),
	AITASK(AI_TASK_CLEAR_HINT_NODE,				0)
};

const CAISchedule scheduleActiveIdle(
	// Task list
	taskListScheduleActiveIdle,
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleActiveIdle),
	// AI interrupt mask
	AI_COND_NEW_ENEMY | 
	AI_COND_LIGHT_DAMAGE |
	AI_COND_HEAVY_DAMAGE | 
	AI_COND_HEAR_SOUND | 
	AI_COND_BLOCKING_PATH | 
	AI_COND_PROVOKED,
	// Sound mask
	AI_SOUND_COMBAT | 
	AI_SOUND_WORLD | 
	AI_SOUND_PLAYER | 
	AI_SOUND_DANGER, 
	// Name
	"Active Idle"
);

//=============================================
// @brief Become alert schedule
//
//=============================================
ai_task_t taskListScheduleBecomeAlert[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_SET_ACTIVITY,				(Float)ACT_IDLE),
	AITASK(AI_TASK_ALERT_SOUND,					0),
	AITASK(AI_TASK_FACE_IDEAL,					0)
};

const CAISchedule scheduleBecomeAlert(
	// Task list
	taskListScheduleBecomeAlert, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleBecomeAlert),
	// AI interrupt mask
	AI_COND_NONE,
	// Sound mask
	AI_SOUND_NONE, 
	// Name
	"Become Alert"
);

//=============================================
// @brief Alert face schedule
//
//=============================================
ai_task_t taskListScheduleAlertFace[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_SET_ACTIVITY,				(Float)ACT_IDLE),
	AITASK(AI_TASK_FACE_IDEAL,					0)
};

const CAISchedule scheduleAlertFace(
	// Task list
	taskListScheduleAlertFace, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleAlertFace),
	// AI interrupt mask
	AI_COND_NEW_ENEMY |
	AI_COND_SEE_FEAR |
	AI_COND_LIGHT_DAMAGE |
	AI_COND_HEAVY_DAMAGE |
	AI_COND_PROVOKED |
	AI_COND_FOLLOW_TARGET_TOO_FAR,
	// Sound mask
	AI_SOUND_NONE, 
	// Name
	"Alert Face"
);

//=============================================
// @brief Alert small flinch schedule
//
//=============================================
ai_task_t taskListScheduleAlertSmallFlinch[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_REMEMBER,					(Float)AI_MEMORY_FLINCHED),
	AITASK(AI_TASK_FLINCH,						0),
	AITASK(AI_TASK_SET_SCHEDULE,				(Float)AI_SCHED_ALERT_FACE)
};

const CAISchedule scheduleAlertSmallFlinch(
	// Task list
	taskListScheduleAlertSmallFlinch, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleAlertSmallFlinch),
	// AI interrupt mask
	AI_COND_NONE,
	// Sound mask
	AI_SOUND_NONE, 
	// Name
	"Alert Small Flinch"
);

//=============================================
// @brief Alert stand schedule
//
//=============================================
ai_task_t taskListScheduleAlertStand[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_SET_ACTIVITY,				(Float)ACT_IDLE),
	AITASK(AI_TASK_WAIT,						(Float)20),
	AITASK(AI_TASK_SUGGEST_STATE,				(Float)NPC_STATE_IDLE)
};

const CAISchedule scheduleAlertStand(
	// Task list
	taskListScheduleAlertStand, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleAlertStand),
	// AI interrupt mask
	AI_COND_NEW_ENEMY | 
	AI_COND_SEE_ENEMY |
	AI_COND_SEE_FEAR | 
	AI_COND_LIGHT_DAMAGE |
	AI_COND_HEAVY_DAMAGE | 
	AI_COND_HEAR_SOUND | 
	AI_COND_FOLLOW_TARGET_TOO_FAR |
	AI_COND_BLOCKING_PATH | 
	AI_COND_PROVOKED,
	// Sound mask
	AI_SOUND_COMBAT | 
	AI_SOUND_WORLD | 
	AI_SOUND_PLAYER | 
	AI_SOUND_DANGER, 
	// Name
	"Alert Stand"
);

//=============================================
// @brief Investigate sound schedule
//
//=============================================
ai_task_t taskListScheduleInvestigateSound[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_STORE_LAST_POSITION,			0),
	AITASK(AI_TASK_SET_CHECK_BEST_SOUND,		0),
	AITASK(AI_TASK_FACE_BEST_SOUND,				0),
	AITASK(AI_TASK_GET_PATH_TO_BEST_SOUND,		0),
	AITASK(AI_TASK_FACE_IDEAL,					0),
	AITASK(AI_TASK_RUN_PATH,					0),
	AITASK(AI_TASK_WAIT_FOR_MOVEMENT,			0),
	AITASK(AI_TASK_SET_ACTIVITY,				(Float)ACT_IDLE)
};

const CAISchedule scheduleInvestigateSound(
	// Task list
	taskListScheduleInvestigateSound, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleInvestigateSound),
	// AI interrupt mask
	AI_COND_NEW_ENEMY | 
	AI_COND_SEE_FEAR | 
	AI_COND_LIGHT_DAMAGE |
	AI_COND_HEAVY_DAMAGE | 
	AI_COND_BLOCKING_PATH | 
	AI_COND_PROVOKED,
	// Sound mask
	AI_SOUND_NONE, 
	// Name
	"Investigate Sound"
);

//=============================================
// @brief Combat stand schedule
//
//=============================================
ai_task_t taskListScheduleCombatStand[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_SET_ACTIVITY,				(Float)ACT_IDLE),
	AITASK(AI_TASK_WAIT_INDEFINITE,				0)
};

const CAISchedule scheduleCombatStand(
	// Task list
	taskListScheduleCombatStand, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleCombatStand),
	// AI interrupt mask
	AI_COND_NEW_ENEMY | 
	AI_COND_ENEMY_DEAD |
	AI_COND_SEE_ENEMY | 
	AI_COND_LIGHT_DAMAGE |
	AI_COND_HEAVY_DAMAGE | 
	AI_COND_CAN_ATTACK,
	// Sound mask
	AI_SOUND_NONE, 
	// Name
	"Combat Stand"
);

//=============================================
// @brief Combat stand schedule
//
//=============================================
ai_task_t taskListScheduleCombatFace[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_SET_ACTIVITY,				(Float)ACT_IDLE),
	AITASK(AI_TASK_FACE_ENEMY,					0)
};

const CAISchedule scheduleCombatFace(
	// Task list
	taskListScheduleCombatFace, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleCombatFace),
	// AI interrupt mask
	AI_COND_CAN_ATTACK |
	AI_COND_NEW_ENEMY |
	AI_COND_SEE_ENEMY |
	AI_COND_ENEMY_DEAD,
	// Sound mask
	AI_SOUND_NONE, 
	// Name
	"Combat Face"
);

//=============================================
// @brief Standoff schedule
//
//=============================================
ai_task_t taskListScheduleStandoff[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_SET_ACTIVITY,				(Float)ACT_IDLE),
	AITASK(AI_TASK_WAIT_FACE_ENEMY,				2)
};

const CAISchedule scheduleStandoff(
	// Task list
	taskListScheduleStandoff, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleStandoff),
	// AI interrupt mask
	AI_COND_CAN_RANGE_ATTACK1 | 
	AI_COND_CAN_RANGE_ATTACK2 |
	AI_COND_CAN_SPECIAL_ATTACK1 | 
	AI_COND_CAN_SPECIAL_ATTACK2 |
	AI_COND_ENEMY_DEAD | 
	AI_COND_NEW_ENEMY |
	AI_COND_SEE_ENEMY | 
	AI_COND_HEAR_SOUND,
	// Sound mask
	AI_SOUND_DANGER, 
	// Name
	"Standoff"
);

//=============================================
// @brief Arm weapon schedule
//
//=============================================
ai_task_t taskListScheduleArmWeapon[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_PLAY_SEQUENCE,				(Float)ACT_ARM)
};

const CAISchedule scheduleArmWeapon(
	// Task list
	taskListScheduleArmWeapon, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleArmWeapon),
	// AI interrupt mask
	AI_COND_NONE,
	// Sound mask
	AI_SOUND_NONE, 
	// Name
	"Arm Weapon"
);

//=============================================
// @brief Reload weapon schedule
//
//=============================================
ai_task_t taskListScheduleReload[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_PLAY_SEQUENCE,				(Float)ACT_RELOAD),
	AITASK(AI_TASK_SET_ACTIVITY,				(Float)ACT_IDLE)
};

const CAISchedule scheduleReload(
	// Task list
	taskListScheduleReload, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleReload),
	// AI interrupt mask
	AI_COND_HEAVY_DAMAGE,
	// Sound mask
	AI_SOUND_NONE, 
	// Name
	"Reload"
);

//=============================================
// @brief Hide reload weapon schedule
//
//=============================================
ai_task_t taskListScheduleHideReload[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_SET_FAIL_SCHEDULE,			(Float)AI_SCHED_RELOAD),
	AITASK(AI_TASK_FIND_RELOAD_COVER_SPOT,		0),
	AITASK(AI_TASK_RUN_PATH,					0),
	AITASK(AI_TASK_WAIT_FOR_MOVEMENT,			0),
	AITASK(AI_TASK_REMEMBER,					(Float)AI_MEMORY_IN_COVER),
	AITASK(AI_TASK_FACE_ENEMY,					0),
	AITASK(AI_TASK_PLAY_SEQUENCE,				(Float)ACT_RELOAD),
	AITASK(AI_TASK_SET_ACTIVITY,				(Float)ACT_IDLE)
};

const CAISchedule scheduleHideReload(
	// Task list
	taskListScheduleHideReload, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleHideReload),
	// AI interrupt mask
	AI_COND_DANGEROUS_ENEMY_CLOSE |
	AI_COND_HEAVY_DAMAGE |
	AI_COND_IN_DANGER |
	AI_COND_HEAR_SOUND,
	// Sound mask
	AI_SOUND_DANGER, 
	// Name
	"Hide Reload"
);

//=============================================
// @brief Range Attack 1 schedule
//
//=============================================
ai_task_t taskListScheduleRangeAttack1[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_FACE_ENEMY,					0),
	AITASK(AI_TASK_RANGE_ATTACK1,				0)
};

const CAISchedule scheduleRangeAttack1(
	// Task list
	taskListScheduleRangeAttack1, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleRangeAttack1),
	// AI interrupt mask
	AI_COND_NEW_ENEMY |
	AI_COND_ENEMY_DEAD |
	AI_COND_LIGHT_DAMAGE |
	AI_COND_HEAVY_DAMAGE |
	AI_COND_ENEMY_OCCLUDED |
	AI_COND_NO_AMMO_LOADED |
	AI_COND_HEAR_SOUND |
	AI_COND_FRIENDLY_FIRE,
	// Sound mask
	AI_SOUND_DANGER, 
	// Name
	"Range Attack 1"
);

//=============================================
// @brief Range Attack 2 schedule
//
//=============================================
ai_task_t taskListScheduleRangeAttack2[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_FACE_ENEMY,					0),
	AITASK(AI_TASK_RANGE_ATTACK2,				0)
};

const CAISchedule scheduleRangeAttack2(
	// Task list
	taskListScheduleRangeAttack2, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleRangeAttack2),
	// AI interrupt mask
	AI_COND_NEW_ENEMY |
	AI_COND_ENEMY_DEAD |
	AI_COND_LIGHT_DAMAGE |
	AI_COND_HEAVY_DAMAGE |
	AI_COND_ENEMY_OCCLUDED |
	AI_COND_NO_AMMO_LOADED |
	AI_COND_HEAR_SOUND |
	AI_COND_FRIENDLY_FIRE,
	// Sound mask
	AI_SOUND_DANGER, 
	// Name
	"Range Attack 2"
);

//=============================================
// @brief Melee Attack 1 schedule
//
//=============================================
ai_task_t taskListScheduleMeleeAttack1[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_FACE_ENEMY,					0),
	AITASK(AI_TASK_MELEE_ATTACK1,				0)
};

const CAISchedule scheduleMeleeAttack1(
	// Task list
	taskListScheduleMeleeAttack1, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleMeleeAttack1),
	// AI interrupt mask
	AI_COND_NEW_ENEMY |
	AI_COND_ENEMY_DEAD |
	AI_COND_LIGHT_DAMAGE |
	AI_COND_HEAVY_DAMAGE |
	AI_COND_ENEMY_OCCLUDED,
	// AI inverse interrupt mask
	AI_COND_CAN_MELEE_ATTACK1,
	// Sound mask
	AI_SOUND_NONE, 
	// Name
	"Melee Attack 1"
);

//=============================================
// @brief Melee Attack 1 schedule
//
//=============================================
ai_task_t taskListScheduleMeleeAttack2[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_FACE_ENEMY,					0),
	AITASK(AI_TASK_MELEE_ATTACK2,				0)
};

const CAISchedule scheduleMeleeAttack2(
	// Task list
	taskListScheduleMeleeAttack2, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleMeleeAttack2),
	// AI interrupt mask
	AI_COND_NEW_ENEMY |
	AI_COND_ENEMY_DEAD |
	AI_COND_LIGHT_DAMAGE |
	AI_COND_HEAVY_DAMAGE |
	AI_COND_ENEMY_OCCLUDED,
	// AI inverse interrupt mask
	AI_COND_CAN_MELEE_ATTACK2,
	// Sound mask
	AI_SOUND_NONE, 
	// Name
	"Melee Attack 2"
);

//=============================================
// @brief Range Attack 1 schedule
//
//=============================================
ai_task_t taskListScheduleSpecialAttack1[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_FACE_ENEMY,					0),
	AITASK(AI_TASK_SPECIAL_ATTACK1,				0)
};

const CAISchedule scheduleSpecialAttack1(
	// Task list
	taskListScheduleSpecialAttack1, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleSpecialAttack1),
	// AI interrupt mask
	AI_COND_NEW_ENEMY |
	AI_COND_ENEMY_DEAD |
	AI_COND_LIGHT_DAMAGE |
	AI_COND_HEAVY_DAMAGE |
	AI_COND_ENEMY_OCCLUDED |
	AI_COND_NO_AMMO_LOADED |
	AI_COND_HEAR_SOUND,
	// Sound mask
	AI_SOUND_DANGER, 
	// Name
	"Special Attack 1"
);

//=============================================
// @brief Special Attack 2 schedule
//
//=============================================
ai_task_t taskListScheduleSpecialAttack2[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_FACE_ENEMY,					0),
	AITASK(AI_TASK_SPECIAL_ATTACK2,				0)
};

const CAISchedule scheduleSpecialAttack2(
	// Task list
	taskListScheduleSpecialAttack2, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleSpecialAttack2),
	// AI interrupt mask
	AI_COND_NEW_ENEMY |
	AI_COND_ENEMY_DEAD |
	AI_COND_LIGHT_DAMAGE |
	AI_COND_HEAVY_DAMAGE |
	AI_COND_ENEMY_OCCLUDED |
	AI_COND_NO_AMMO_LOADED |
	AI_COND_HEAR_SOUND,
	// Sound mask
	AI_SOUND_DANGER, 
	// Name
	"Special Attack 2"
);

//=============================================
// @brief Chase Enemy schedule
//
//=============================================
ai_task_t taskListScheduleChaseEnemy[] = 
{
	AITASK(AI_TASK_SET_FAIL_SCHEDULE,			(Float)AI_SCHED_CHASE_ENEMY_FAILED),
	AITASK(AI_TASK_GET_PATH_TO_ENEMY,			0),
	AITASK(AI_TASK_RUN_PATH,					0),
	AITASK(AI_TASK_WAIT_FOR_MOVEMENT,			0),
	AITASK(AI_TASK_FACE_ENEMY,					0)
};

const CAISchedule scheduleChaseEnemy(
	// Task list
	taskListScheduleChaseEnemy, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleChaseEnemy),
	// AI interrupt mask
	AI_COND_NEW_ENEMY |
	AI_COND_CAN_ATTACK |
	AI_COND_TASK_FAILED |
	AI_COND_HEARD_ENEMY_NEW_POSITION |
	AI_COND_HEAR_SOUND,
	// Sound mask
	AI_SOUND_DANGER, 
	// Name
	"Chase Enemy"
);

//=============================================
// @brief Chase Enemy Failed schedule
//
//=============================================
ai_task_t taskListScheduleChaseEnemyFailed[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_WAIT,						0.5),
	AITASK(AI_TASK_FIND_COVER_FROM_ENEMY,		0),
	AITASK(AI_TASK_RUN_PATH,					0),
	AITASK(AI_TASK_WAIT_FOR_MOVEMENT,			0),
	AITASK(AI_TASK_REMEMBER,					(Float)AI_MEMORY_IN_COVER),
	AITASK(AI_TASK_FACE_ENEMY,					0),
	AITASK(AI_TASK_WAIT,						0.1)

};

const CAISchedule scheduleChaseEnemyFailed(
	// Task list
	taskListScheduleChaseEnemyFailed, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleChaseEnemyFailed),
	// AI interrupt mask
	AI_COND_NEW_ENEMY |
	AI_COND_CAN_ATTACK |
	AI_COND_HEAR_SOUND,
	// Sound mask
	AI_SOUND_DANGER, 
	// Name
	"Chase Enemy Failed"
);

//=============================================
// @brief Small Flinch schedule
//
//=============================================
ai_task_t taskListScheduleSmallFlinch[] = 
{
	AITASK(AI_TASK_REMEMBER,					(Float)AI_MEMORY_FLINCHED),
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_FLINCH,						0),
	AITASK(AI_TASK_FACE_IDEAL,					0)
};

const CAISchedule scheduleSmallFlinch(
	// Task list
	taskListScheduleSmallFlinch, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleSmallFlinch),
	// AI interrupt mask
	AI_COND_NONE,
	// Sound mask
	AI_SOUND_NONE, 
	// Name
	"Small flinch"
);

//=============================================
// @brief Death schedule
//
//=============================================
ai_task_t taskListScheduleDie[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_SOUND_DIE,					0),
	AITASK(AI_TASK_DIE,							0)
};

const CAISchedule scheduleDie(
	// Task list
	taskListScheduleDie, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleDie),
	// AI interrupt mask
	AI_COND_NONE,
	// Sound mask
	AI_SOUND_NONE, 
	// Name
	"Death"
);

//=============================================
// @brief Victory dance schedule
//
//=============================================
ai_task_t taskListScheduleVictoryDance[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_PLAY_SEQUENCE,				(Float)ACT_VICTORY_DANCE)
};

const CAISchedule scheduleVictoryDance(
	// Task list
	taskListScheduleVictoryDance, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleVictoryDance),
	// AI interrupt mask
	AI_COND_NONE,
	// Sound mask
	AI_SOUND_NONE, 
	// Name
	"Victory Dance"
);

//=============================================
// @brief Error schedule
//
//=============================================
ai_task_t taskListScheduleError[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_WAIT_INDEFINITE,				0)
};

const CAISchedule scheduleError(
	// Task list
	taskListScheduleError, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleError),
	// AI interrupt mask
	AI_COND_NONE,
	// Sound mask
	AI_SOUND_NONE, 
	// Name
	"Error"
);

//=============================================
// @brief Walk to script schedule
//
//=============================================
ai_task_t taskListScheduleScriptedWalk[] = 
{
	AITASK(AI_TASK_WALK_TO_TARGET,				0),
	AITASK(AI_TASK_WAIT_FOR_MOVEMENT,			0),
	AITASK(AI_TASK_PLANT_ON_SCRIPT,				0),
	AITASK(AI_TASK_FACE_SCRIPT,					0),
	AITASK(AI_TASK_FACE_IDEAL,					0),
	AITASK(AI_TASK_WAIT_BLEND,					VBM_SEQ_BLEND_TIME),
	AITASK(AI_TASK_ENABLE_SCRIPT,				0),
	AITASK(AI_TASK_WAIT_FOR_SCRIPT,				0),
	AITASK(AI_TASK_PLAY_SCRIPT,					0)
};

const CAISchedule scheduleScriptedWalk(
	// Task list
	taskListScheduleScriptedWalk, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleScriptedWalk),
	// AI interrupt mask
	AI_SCRIPT_BREAK_CONDITIONS,
	// Sound mask
	AI_SOUND_NONE, 
	// Name
	"Walk to Script"
);

//=============================================
// @brief Walk to script without turning
//
//=============================================
ai_task_t taskListScheduleScriptedWalkNoFace[] = 
{
	AITASK(AI_TASK_WALK_TO_TARGET,				0),
	AITASK(AI_TASK_WAIT_FOR_MOVEMENT,			0),
	AITASK(AI_TASK_PLANT_ON_SCRIPT,				0),
	AITASK(AI_TASK_WAIT_BLEND,					VBM_SEQ_BLEND_TIME),
	AITASK(AI_TASK_ENABLE_SCRIPT,				0),
	AITASK(AI_TASK_WAIT_FOR_SCRIPT,				0),
	AITASK(AI_TASK_PLAY_SCRIPT,					0)
};

const CAISchedule scheduleScriptedWalkNoFace(
	// Task list
	taskListScheduleScriptedWalkNoFace, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleScriptedWalkNoFace),
	// AI interrupt mask
	AI_SCRIPT_BREAK_CONDITIONS,
	// Sound mask
	AI_SOUND_NONE, 
	// Name
	"Walk to Script(no turning)"
);

//=============================================
// @brief Run to script schedule
//
//=============================================
ai_task_t taskListScheduleScriptedRun[] = 
{
	AITASK(AI_TASK_RUN_TO_TARGET,				0),
	AITASK(AI_TASK_WAIT_FOR_MOVEMENT,			0),
	AITASK(AI_TASK_PLANT_ON_SCRIPT,				0),
	AITASK(AI_TASK_FACE_SCRIPT,					0),
	AITASK(AI_TASK_FACE_IDEAL,					0),
	AITASK(AI_TASK_WAIT_BLEND,					VBM_SEQ_BLEND_TIME),
	AITASK(AI_TASK_ENABLE_SCRIPT,				0),
	AITASK(AI_TASK_WAIT_FOR_SCRIPT,				0),
	AITASK(AI_TASK_PLAY_SCRIPT,					0)
};

const CAISchedule scheduleScriptedRun(
	// Task list
	taskListScheduleScriptedRun, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleScriptedRun),
	// AI interrupt mask
	AI_SCRIPT_BREAK_CONDITIONS,
	// Sound mask
	AI_SOUND_NONE, 
	// Name
	"Run to Script"
);

//=============================================
// @brief Run to script schedule without turning
//
//=============================================
ai_task_t taskListScheduleScriptedRunNoFace[] = 
{
	AITASK(AI_TASK_RUN_TO_TARGET,				0),
	AITASK(AI_TASK_WAIT_FOR_MOVEMENT,			0),
	AITASK(AI_TASK_PLANT_ON_SCRIPT,				0),
	AITASK(AI_TASK_WAIT_BLEND,					VBM_SEQ_BLEND_TIME),
	AITASK(AI_TASK_ENABLE_SCRIPT,				0),
	AITASK(AI_TASK_WAIT_FOR_SCRIPT,				0),
	AITASK(AI_TASK_PLAY_SCRIPT,					0)
};

const CAISchedule scheduleScriptedRunNoFace(
	// Task list
	taskListScheduleScriptedRunNoFace, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleScriptedRunNoFace),
	// AI interrupt mask
	AI_SCRIPT_BREAK_CONDITIONS,
	// Sound mask
	AI_SOUND_NONE, 
	// Name
	"Run to Script(no turning)"
);

//=============================================
// @brief Wait for script schedule
//
//=============================================
ai_task_t taskListScheduleWaitForScript[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_WAIT_FOR_SCRIPT,				0),
	AITASK(AI_TASK_PLAY_SCRIPT,					0)
};

const CAISchedule scheduleWaitForScript(
	// Task list
	taskListScheduleWaitForScript, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleWaitForScript),
	// AI interrupt mask
	AI_SCRIPT_BREAK_CONDITIONS,
	// Sound mask
	AI_SOUND_NONE, 
	// Name
	"Wait for Script"
);

//=============================================
// @brief Face script schedule
//
//=============================================
ai_task_t taskListScheduleFaceScript[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_FACE_SCRIPT,					0),
	AITASK(AI_TASK_FACE_IDEAL,					0),
	AITASK(AI_TASK_WAIT_BLEND,					VBM_SEQ_BLEND_TIME),
	AITASK(AI_TASK_WAIT_FOR_SCRIPT,				0),
	AITASK(AI_TASK_PLAY_SCRIPT,					0)
};

const CAISchedule scheduleFaceScript(
	// Task list
	taskListScheduleFaceScript, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleFaceScript),
	// AI interrupt mask
	AI_SCRIPT_BREAK_CONDITIONS,
	// Sound mask
	AI_SOUND_NONE, 
	// Name
	"Face Script"
);

//=============================================
// @brief Scripted face player
//
//=============================================
ai_task_t taskListScheduleScriptedFacePlayer[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_FACE_PLAYER,					0),
	AITASK(AI_TASK_WAIT_BLEND,					VBM_SEQ_BLEND_TIME),
	AITASK(AI_TASK_WAIT_FOR_SCRIPT,				0),
	AITASK(AI_TASK_PLAY_SCRIPT,					0)
};

const CAISchedule scheduleScriptedFacePlayer(
	// Task list
	taskListScheduleScriptedFacePlayer, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleScriptedFacePlayer),
	// AI interrupt mask
	AI_SCRIPT_BREAK_CONDITIONS,
	// Sound mask
	AI_SOUND_NONE, 
	// Name
	"Scripted Face Player"
);

//=============================================
// @brief Cower
//
//=============================================
ai_task_t taskListScheduleCower[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_PLAY_SEQUENCE,				(Float)ACT_COWER)
};

const CAISchedule scheduleCower(
	// Task list
	taskListScheduleCower, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleCower),
	// AI interrupt mask
	AI_COND_NONE,
	// Sound mask
	AI_SOUND_NONE, 
	// Name
	"Cower"
);

//=============================================
// @brief Take cover from our current position
//
//=============================================
ai_task_t taskListScheduleTakeCoverFromOrigin[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_FIND_COVER_FROM_ORIGIN,		0),
	AITASK(AI_TASK_RUN_PATH,					0),
	AITASK(AI_TASK_WAIT_FOR_MOVEMENT,			0),
	AITASK(AI_TASK_REMEMBER,					(Float)AI_MEMORY_IN_COVER),
	AITASK(AI_TASK_TURN_LEFT,					180)
};

const CAISchedule scheduleTakeCoverFromOrigin(
	// Task list
	taskListScheduleTakeCoverFromOrigin, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleTakeCoverFromOrigin),
	// AI interrupt mask
	AI_COND_NEW_ENEMY |
	AI_COND_IN_DANGER,
	// Sound mask
	AI_SOUND_NONE, 
	// Name
	"Take Cover from Origin"
);

//=============================================
// @brief Take cover from best sound
//
//=============================================
ai_task_t taskListScheduleTakeCoverFromBestSound[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_FIND_COVER_FROM_BEST_SOUND,	0),
	AITASK(AI_TASK_RUN_PATH,					0),
	AITASK(AI_TASK_WAIT_FOR_MOVEMENT,			0),
	AITASK(AI_TASK_REMEMBER,					(Float)AI_MEMORY_IN_COVER),
	AITASK(AI_TASK_TURN_LEFT,					180)
};

const CAISchedule scheduleTakeCoverFromBestSound(
	// Task list
	taskListScheduleTakeCoverFromBestSound, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleTakeCoverFromBestSound),
	// AI interrupt mask
	AI_COND_NEW_ENEMY |
	AI_COND_IN_DANGER,
	// Sound mask
	AI_SOUND_NONE, 
	// Name
	"Take Cover from Best Sound"
);

//=============================================
// @brief Take cover from best sound
//
//=============================================
ai_task_t taskListScheduleTakeCoverFromBestSoundWithCower[] = 
{
	AITASK(AI_TASK_SET_FAIL_SCHEDULE,			(Float)AI_SCHED_COWER),
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_FIND_COVER_FROM_BEST_SOUND,	0),
	AITASK(AI_TASK_RUN_PATH,					0),
	AITASK(AI_TASK_WAIT_FOR_MOVEMENT,			0),
	AITASK(AI_TASK_REMEMBER,					(Float)AI_MEMORY_IN_COVER),
	AITASK(AI_TASK_TURN_LEFT,					180)
};

const CAISchedule scheduleTakeCoverFromBestSoundWithCower(
	// Task list
	taskListScheduleTakeCoverFromBestSoundWithCower, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleTakeCoverFromBestSoundWithCower),
	// AI interrupt mask
	AI_COND_IN_DANGER,
	// Sound mask
	AI_SOUND_NONE, 
	// Name
	"Take Cover from Best Sound with Cower"
);

//=============================================
// @brief Take cover from enemy
//
//=============================================
ai_task_t taskListScheduleTakeCoverFromEnemy[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_WAIT,						0.2f),
	AITASK(AI_TASK_FIND_COVER_FROM_ENEMY,		0),
	AITASK(AI_TASK_RUN_PATH,					0),
	AITASK(AI_TASK_WAIT_FOR_MOVEMENT,			0),
	AITASK(AI_TASK_REMEMBER,					(Float)AI_MEMORY_IN_COVER),
	AITASK(AI_TASK_FACE_ENEMY,					0),
	AITASK(AI_TASK_WAIT,						1)
};

const CAISchedule scheduleTakeCoverFromEnemy(
	// Task list
	taskListScheduleTakeCoverFromEnemy, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleTakeCoverFromEnemy),
	// AI interrupt mask
	AI_COND_NEW_ENEMY |
	AI_COND_IN_DANGER,
	// Sound mask
	AI_SOUND_NONE, 
	// Name
	"Take Cover from Enemy"
);

//=============================================
// @brief Move out of another's way
//
//=============================================
ai_task_t taskListScheduleMoveOutOfWay[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_WAIT,						0.2f),
	AITASK(AI_TASK_CLEAR_BLOCKED_PATH,			0),
	AITASK(AI_TASK_RUN_PATH,					0),
	AITASK(AI_TASK_WAIT_FOR_MOVEMENT,			0),
	AITASK(AI_TASK_CLEAR_BLOCK_STATUS,			0)
};

const CAISchedule scheduleMoveOutOfWay(
	// Task list
	taskListScheduleMoveOutOfWay, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleMoveOutOfWay),
	// AI interrupt mask
	AI_COND_NEW_ENEMY |
	AI_COND_CAN_ATTACK |
	AI_COND_FOLLOW_TARGET_TOO_FAR |
	AI_COND_HEAR_SOUND,
	// Sound mask
	AI_SOUND_DANGER, 
	// Name
	"Move Out Of Way"
);

//=============================================
// @brief Find enemies
//
//=============================================
ai_task_t taskListScheduleFindEnemies[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_TURN_LEFT,					180.0f),
	AITASK(AI_TASK_WAIT,						1),
	AITASK(AI_TASK_FIND_ENEMY_SEARCH_SPOT,		0),
	AITASK(AI_TASK_RUN_PATH,					0),
	AITASK(AI_TASK_WAIT_FOR_MOVEMENT,			0),
	AITASK(AI_TASK_TURN_LEFT,					180),
	AITASK(AI_TASK_SET_ACTIVITY,				(Float)ACT_IDLE),
	AITASK(AI_TASK_WAIT,						1)
};

const CAISchedule scheduleFindEnemies(
	// Task list
	taskListScheduleFindEnemies, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleFindEnemies),
	// AI interrupt mask
	AI_COND_NEW_ENEMY |
	AI_COND_SEE_ENEMY |
	AI_COND_CAN_ATTACK |
	AI_COND_HEAR_SOUND,
	// Sound mask
	AI_SOUND_DANGER, 
	// Name
	"Find Enemies"
);

//=============================================
// @brief Dodge enemy
//
//=============================================
ai_task_t taskListScheduleDodgeEnemy[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_DODGE_ENEMY,					0),
	AITASK(AI_TASK_RUN_PATH,					0),
	AITASK(AI_TASK_WAIT_FOR_MOVEMENT,			0),
	AITASK(AI_TASK_FACE_ENEMY,					0)
};

const CAISchedule scheduleDodgeEnemy(
	// Task list
	taskListScheduleDodgeEnemy, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleDodgeEnemy),
	// AI interrupt mask
	AI_COND_NEW_ENEMY |
	AI_COND_IN_DANGER,
	// Sound mask
	AI_SOUND_NONE, 
	// Name
	"Dodge Enemy"
);

//=============================================
// @brief Inspect enemy corpse
//
//=============================================
ai_task_t taskListScheduleInspectEnemyCorpse[] = 
{
	AITASK(AI_TASK_SET_FAIL_SCHEDULE,			(Float)AI_SCHED_FAIL),
	AITASK(AI_TASK_SET_ACTIVITY,				(Float)ACT_IDLE),
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_FACE_ENEMY,					0),
	AITASK(AI_TASK_WAIT,						1.5f),
	AITASK(AI_TASK_GET_PATH_TO_ENEMY_CORPSE,	0),
	AITASK(AI_TASK_WALK_PATH,					0),
	AITASK(AI_TASK_WAIT_FOR_MOVEMENT,			0),
	AITASK(AI_TASK_FACE_ENEMY,					0),
	AITASK(AI_TASK_PLAY_SEQUENCE,				(Float)ACT_VICTORY_DANCE)
};

const CAISchedule scheduleInspectEnemyCorpse(
	// Task list
	taskListScheduleInspectEnemyCorpse, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleInspectEnemyCorpse),
	// AI interrupt mask
	AI_COND_DANGEROUS_ENEMY_CLOSE |
	AI_COND_NEW_ENEMY |
	AI_COND_LIGHT_DAMAGE |
	AI_COND_HEAVY_DAMAGE,
	// Sound mask
	AI_SOUND_NONE, 
	// Name
	"Inspect Enemy Corpse"
);

//=============================================
// @brief Establish line of fire
//
//=============================================
ai_task_t taskListScheduleEstablishLineOfFire[] = 
{
	AITASK(AI_TASK_SET_FAIL_SCHEDULE,						(Float)AI_SCHED_ELOF_FAIL),
	AITASK(AI_TASK_GET_PATH_TO_ENEMY,						0),
	AITASK(AI_TASK_RUN_PATH,								0),
	AITASK(AI_TASK_WAIT_FOR_MOVEMENT,						0),
	AITASK(AI_TASK_FACE_ENEMY,								0)
};

const CAISchedule scheduleEstablishLineOfFire(
	// Task list
	taskListScheduleEstablishLineOfFire, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleEstablishLineOfFire),
	// AI interrupt mask
	AI_COND_DANGEROUS_ENEMY_CLOSE |
	AI_COND_NEW_ENEMY |
	AI_COND_ENEMY_DEAD |
	AI_COND_CAN_ATTACK |
	AI_COND_IN_DANGER |
	AI_COND_HEARD_ENEMY_NEW_POSITION |
	AI_COND_HEAR_SOUND |
	AI_COND_SHOOT_VECTOR_VALID,
	// Inverse interrupt mask
	AI_COND_NONE,
	// Special interrupt schedule
	AI_SCHED_FACE_ENEMY,
	// Special interrupt mask
	AI_COND_SHOOT_VECTOR_VALID,
	// Special interrupt exception mask
	AI_COND_DANGEROUS_ENEMY_CLOSE |
	AI_COND_NEW_ENEMY |
	AI_COND_ENEMY_DEAD |
	AI_COND_IN_DANGER |
	AI_COND_HEARD_ENEMY_NEW_POSITION |
	AI_COND_HEAR_SOUND,
	// Special interrupt requirement mask
	AI_COND_NONE,
	// Sound mask
	AI_SOUND_DANGER, 
	// Name
	"Establish Line Of Fire"
);

//=============================================
// @brief Establish line of fire
//
//=============================================
ai_task_t taskListScheduleFaceEnemy[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_FACE_ENEMY,					0)
};

const CAISchedule scheduleFaceEnemy(
	// Task list
	taskListScheduleFaceEnemy, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleFaceEnemy),
	// AI interrupt mask
	AI_COND_DANGEROUS_ENEMY_CLOSE |
	AI_COND_NEW_ENEMY |
	AI_COND_ENEMY_DEAD |
	AI_COND_CAN_ATTACK |
	AI_COND_IN_DANGER |
	AI_COND_HEARD_ENEMY_NEW_POSITION |
	AI_COND_HEAR_SOUND,
	// Sound mask
	AI_SOUND_DANGER, 
	// Name
	"Face Enemy"
);

//=============================================
// @brief Sweep
//
//=============================================
ai_task_t taskListScheduleSweep[] = 
{
	AITASK(AI_TASK_TURN_LEFT,					(Float)180.0f),
	AITASK(AI_TASK_SET_ACTIVITY,				(Float)ACT_IDLE),
	AITASK(AI_TASK_WAIT,						0.5f),
	AITASK(AI_TASK_TURN_LEFT,					(Float)180.0f),
	AITASK(AI_TASK_SET_ACTIVITY,				(Float)ACT_IDLE),
	AITASK(AI_TASK_WAIT,						0.5f)
};

const CAISchedule scheduleSweep(
	// Task list
	taskListScheduleSweep, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleSweep),
	// AI interrupt mask
	AI_COND_DANGEROUS_ENEMY_CLOSE |
	AI_COND_NEW_ENEMY |
	AI_COND_LIGHT_DAMAGE |
	AI_COND_HEAVY_DAMAGE |
	AI_COND_CAN_ATTACK |
	AI_COND_HEAR_SOUND,
	// Sound mask
	AI_SOUND_DANGER |
	AI_SOUND_WORLD |
	AI_SOUND_PLAYER,
	// Name
	"Sweep"
);

//=============================================
// @brief Wait in cover
//
//=============================================
ai_task_t taskListScheduleWaitFaceEnemy[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_SET_ACTIVITY,				(Float)ACT_IDLE),
	AITASK(AI_TASK_WAIT_FACE_ENEMY,				1.0f)
};

const CAISchedule scheduleWaitFaceEnemy(
	// Task list
	taskListScheduleWaitFaceEnemy, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleWaitFaceEnemy),
	// AI interrupt mask
	AI_COND_DANGEROUS_ENEMY_CLOSE |
	AI_COND_NEW_ENEMY | 
	AI_COND_HEAR_SOUND | 
	AI_COND_CAN_ATTACK,
	// Sound mask 
	AI_SOUND_DANGER, 
	// Name
	"Wait Face Enemy"
);

//=============================================
// @brief Run away from NPC Puller
//
//=============================================
ai_task_t taskListScheduleRunFromNPCPuller[] = 
{
	AITASK(AI_TASK_STOP_MOVING,						0),
	AITASK(AI_TASK_WAIT,							0.2f),
	AITASK(AI_TASK_GET_PATH_AWAY_FROM_NPCPULLER,	0),
	AITASK(AI_TASK_RUN_PATH,						0),
	AITASK(AI_TASK_WAIT_FOR_MOVEMENT,				0)
};

const CAISchedule scheduleRunFromNPCPuller(
	// Task list
	taskListScheduleRunFromNPCPuller, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleRunFromNPCPuller),
	// AI interrupt mask
	AI_COND_SCHEDULE_DONE |
	AI_COND_HEAVY_DAMAGE |
	AI_COND_NOT_ONGROUND,
	// Sound mask
	AI_SOUND_NONE, 
	// Name
	"Run from NPC puller"
);

//=============================================
// @brief Hover by NPC Puller
//
//=============================================
ai_task_t taskListScheduleHoverByNPCPuller[] = 
{
	AITASK(AI_TASK_SET_ACTIVITY,					ACT_HOVER),
	AITASK(AI_TASK_STOP_MOVING,						0),
	AITASK(AI_TASK_NPCPULLER_HOVER,					0)
};

const CAISchedule scheduleHoverByNPCPuller(
	// Task list
	taskListScheduleHoverByNPCPuller, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleHoverByNPCPuller),
	// AI interrupt mask
	AI_COND_SCHEDULE_DONE |
	AI_COND_HEAVY_DAMAGE,
	// Inverse condition mask
	AI_COND_NONE,
	// Sound mask
	AI_SOUND_NONE, 
	// Name
	"Hover by NPC puller"
);

//==========================================================================
//
// SCHEDULES FOR CBASENPC CLASS
//
//==========================================================================

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::HasActiveSchedule( void ) const
{
	return (m_pSchedule != nullptr) ? true : false;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::ClearSchedule( void )
{
	m_taskStatus = TASK_STATUS_NEW;
	m_pSchedule = nullptr;
	m_scheduleTaskIndex = 0;
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::IsScheduleDone( void )
{
	if(!m_pSchedule)
	{
		Util::EntityConPrintf(m_pEdict, "%s - No current schedule!\n", __FUNCTION__);
		return true;
	}

	if(m_scheduleTaskIndex == (Int32)m_pSchedule->GetNumTasks())
		return true;
	else
		return false;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::ChangeSchedule( const CAISchedule* pNewSchedule )
{
	if(!pNewSchedule)
	{
		Util::EntityConPrintf(m_pEdict, "%s - Schedule was null\n", __FUNCTION__);
		return;
	}

	// Save some conditions in specific cases
	Uint64 saveConditions = 0;
	if(m_npcState == NPC_STATE_COMBAT)
		saveConditions = (m_aiConditionBits & (AI_COND_ENEMY_NAVIGABLE|AI_COND_ENEMY_NOT_FOUND|AI_COND_FRIENDLY_FIRE
		|AI_COND_CAN_RANGE_ATTACK1|AI_COND_CAN_RANGE_ATTACK2|AI_COND_CAN_MELEE_ATTACK1|AI_COND_CAN_MELEE_ATTACK2
		|AI_COND_CAN_SPECIAL_ATTACK1|AI_COND_CAN_SPECIAL_ATTACK2));

	// Save any conditions to be saved from custom entities
	saveConditions |= (m_aiConditionBits & GetScheduleChangeKeptConditions());

	m_pSchedule = pNewSchedule;
	m_scheduleTaskIndex = 0;
	m_taskStatus = TASK_STATUS_NEW;
	m_aiConditionBits = saveConditions;
	m_failureScheduleIndex = AI_SCHED_NONE;
	m_nextScheduleIndex = AI_SCHED_NONE;

	Uint64 interruptMask = m_pSchedule->GetInterruptMask();
	if(m_npcState == NPC_STATE_SCRIPT && m_pScriptedSequence)
		interruptMask &= ~(m_pScriptedSequence->GetIgnoreConditions());

	Uint64 soundMask = m_pSchedule->GetSoundMask();
	if(m_npcState == NPC_STATE_SCRIPT && m_pScriptedSequence)
		soundMask |= m_pScriptedSequence->GetInterruptSoundMask();

	if(interruptMask & AI_COND_HEAR_SOUND && !soundMask)
		Util::EntityConPrintf(m_pEdict, "AI_COND_HEAR_SOUND set, but no sound mask for schedule '%s'.\n", m_pSchedule->GetName());
	else if(soundMask && !(interruptMask & AI_COND_HEAR_SOUND))
		Util::EntityConPrintf(m_pEdict, "Sound mask set, but no AI_COND_HEAR_SOUND set for schedule '%s'.\n", m_pSchedule->GetName());
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::BeginNextScheduledTask( void )
{
	if(!m_pSchedule)
	{
		Util::EntityConPrintf(m_pEdict, "%s - No current schedule!\n", __FUNCTION__);
		return;
	}

	m_taskStatus = TASK_STATUS_NEW;
	m_scheduleTaskIndex++;

	if(IsScheduleDone())
		SetConditions(AI_COND_SCHEDULE_DONE);
}

//=============================================
// @brief
//
//=============================================
Uint64 CBaseNPC::GetScheduleFlags( void ) const
{
	if(!m_pSchedule)
		return 0;
	else
		return m_aiConditionBits & m_pSchedule->GetInterruptMask();
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::IsScheduleValid( void )
{
	if(!m_pSchedule)
		return false;

	Uint64 interruptMask = m_pSchedule->GetInterruptMask();
	interruptMask |= (AI_COND_SCHEDULE_DONE|AI_COND_TASK_FAILED);

	Uint64 aiConditions = (m_aiConditionBits & ~GetIgnoreConditions());
	if(aiConditions & interruptMask)
	{
		if(CheckConditions(AI_COND_TASK_FAILED) && m_failureScheduleIndex == AI_SCHED_NONE)
		{
			// Tell the engine that our schedule has failed
			Util::EntityConDPrintf(m_pEdict, "Schedule '%s' failed.\n", m_pSchedule->GetName());
		}
		else if(!((interruptMask & m_aiConditionBits) & AI_COND_SCHEDULE_DONE))
		{
			Uint64 bitsPresentOfMask = (interruptMask & m_aiConditionBits);

			CString msg;
			msg << "Schedule " << m_pSchedule->GetName() << " interrupted by ";

			bool first = true;
			CString conditionName;
			if(bitsPresentOfMask & AI_COND_NO_AMMO_LOADED)
			{
				conditionName = "AI_COND_NO_AMMO_LOADED";
				first = false;
			}

			if(bitsPresentOfMask & AI_COND_SEE_HATE)
			{
				if(!first)
					conditionName << ", ";
				else
					first = false;

				conditionName << "AI_COND_SEE_HATE";
			}

			if(bitsPresentOfMask & AI_COND_SEE_FEAR)
			{
				if(!first)
					conditionName << ", ";
				else
					first = false;

				conditionName << "AI_COND_SEE_FEAR";
			}

			if(bitsPresentOfMask & AI_COND_SEE_DISLIKE)
			{
				if(!first)
					conditionName << ", ";
				else
					first = false;

				conditionName << "AI_COND_SEE_DISLIKE";
			}

			if(bitsPresentOfMask & AI_COND_SEE_ENEMY)
			{
				if(!first)
					conditionName << ", ";
				else
					first = false;

				conditionName << "AI_COND_SEE_ENEMY";
			}

			if(bitsPresentOfMask & AI_COND_ENEMY_OCCLUDED)
			{
				if(!first)
					conditionName << ", ";
				else
					first = false;

				conditionName << "AI_COND_ENEMY_OCCLUDED";
			}

			if(bitsPresentOfMask & AI_COND_SMELL_FOOD)
			{
				if(!first)
					conditionName << ", ";
				else
					first = false;

				conditionName << "AI_COND_SMELL_FOOD";
			}

			if(bitsPresentOfMask & AI_COND_ENEMY_TOO_FAR)
			{
				if(!first)
					conditionName << ", ";
				else
					first = false;

				conditionName << "AI_COND_ENEMY_TOO_FAR";
			}

			if(bitsPresentOfMask & AI_COND_LIGHT_DAMAGE)
			{
				if(!first)
					conditionName << ", ";
				else
					first = false;

				conditionName << "AI_COND_LIGHT_DAMAGE";
			}

			if(bitsPresentOfMask & AI_COND_HEAVY_DAMAGE)
			{
				if(!first)
					conditionName << ", ";
				else
					first = false;

				conditionName << "AI_COND_HEAVY_DAMAGE";
			}

			if(bitsPresentOfMask & AI_COND_CAN_RANGE_ATTACK1)
			{
				if(!first)
					conditionName << ", ";
				else
					first = false;

				conditionName << "AI_COND_CAN_RANGE_ATTACK1";
			}

			if(bitsPresentOfMask & AI_COND_CAN_RANGE_ATTACK2)
			{
				if(!first)
					conditionName << ", ";
				else
					first = false;

				conditionName << "AI_COND_CAN_RANGE_ATTACK2";
			}

			if(bitsPresentOfMask & AI_COND_CAN_MELEE_ATTACK1)
			{
				if(!first)
					conditionName << ", ";
				else
					first = false;

				conditionName << "AI_COND_CAN_MELEE_ATTACK1";
			}

			if(bitsPresentOfMask & AI_COND_CAN_MELEE_ATTACK2)
			{
				if(!first)
					conditionName << ", ";
				else
					first = false;

				conditionName << "AI_COND_CAN_MELEE_ATTACK2";
			}

			if(bitsPresentOfMask & AI_COND_CAN_SPECIAL_ATTACK1)
			{
				if(!first)
					conditionName << ", ";
				else
					first = false;

				conditionName << "AI_COND_CAN_SPECIAL_ATTACK1";
			}

			if(bitsPresentOfMask & AI_COND_CAN_SPECIAL_ATTACK2)
			{
				if(!first)
					conditionName << ", ";
				else
					first = false;

				conditionName << "AI_COND_CAN_SPECIAL_ATTACK2";
			}

			if(bitsPresentOfMask & AI_COND_FOLLOW_TARGET_TOO_FAR)
			{
				if(!first)
					conditionName << ", ";
				else
					first = false;

				conditionName << "AI_COND_FOLLOW_TARGET_TOO_FAR";
			}

			if(bitsPresentOfMask & AI_COND_PROVOKED)
			{
				if(!first)
					conditionName << ", ";
				else
					first = false;

				conditionName << "AI_COND_PROVOKED";
			}

			if(bitsPresentOfMask & AI_COND_NEW_ENEMY)
			{
				if(!first)
					conditionName << ", ";
				else
					first = false;

				conditionName << "AI_COND_NEW_ENEMY";
			}

			if(bitsPresentOfMask & AI_COND_HEAR_SOUND)
			{
				if(!first)
					conditionName << ", ";
				else
					first = false;

				conditionName << "AI_COND_HEAR_SOUND";
			}

			if(bitsPresentOfMask & AI_COND_SMELL)
			{
				if(!first)
					conditionName << ", ";
				else
					first = false;

				conditionName << "AI_COND_SMELL";
			}

			if(bitsPresentOfMask & AI_COND_ENEMY_FACING_ME)
			{
				if(!first)
					conditionName << ", ";
				else
					first = false;

				conditionName << "AI_COND_ENEMY_FACING_ME";
			}

			if(bitsPresentOfMask & AI_COND_ENEMY_DEAD)
			{
				if(!first)
					conditionName << ", ";
				else
					first = false;

				conditionName << "AI_COND_ENEMY_DEAD";
			}

			if(bitsPresentOfMask & AI_COND_SEE_CLIENT)
			{
				if(!first)
					conditionName << ", ";
				else
					first = false;

				conditionName << "AI_COND_SEE_CLIENT";
			}

			if(bitsPresentOfMask & AI_COND_SEE_NEMESIS)
			{
				if(!first)
					conditionName << ", ";
				else
					first = false;

				conditionName << "AI_COND_SEE_NEMESIS";
			}

			if(bitsPresentOfMask & AI_COND_ENEMY_NOT_FOUND)
			{
				if(!first)
					conditionName << ", ";
				else
					first = false;

				conditionName << "AI_COND_ENEMY_NOT_FOUND";
			}

			if(bitsPresentOfMask & AI_COND_ENEMY_UNREACHABLE)
			{
				if(!first)
					conditionName << ", ";
				else
					first = false;

				conditionName << "AI_COND_ENEMY_UNREACHABLE";
			}

			if(bitsPresentOfMask & AI_COND_IN_DANGER)
			{
				if(!first)
					conditionName << ", ";
				else
					first = false;

				conditionName << "AI_COND_IN_DANGER";
			}

			if(bitsPresentOfMask & AI_COND_DANGEROUS_ENEMY_CLOSE)
			{
				if(!first)
					conditionName << ", ";
				else
					first = false;

				conditionName << "AI_COND_DANGEROUS_ENEMY_CLOSE";
			}

			if(bitsPresentOfMask & AI_COND_BLOCKING_PATH)
			{
				if(!first)
					conditionName << ", ";
				else
					first = false;

				conditionName << "AI_COND_BLOCKING_PATH";
			}

			if(bitsPresentOfMask & AI_COND_TASK_FAILED)
			{
				if(!first)
					conditionName << ", ";
				else
					first = false;

				conditionName << "AI_COND_TASK_FAILED";
			}

			if(bitsPresentOfMask & AI_COND_CLIENT_UNSEEN)
			{
				if(!first)
					conditionName << ", ";
				else
					first = false;

				conditionName << "AI_COND_CLIENT_UNSEEN";
			}

			if(bitsPresentOfMask & AI_COND_PLAYER_CLOSE)
			{
				if(!first)
					conditionName << ", ";
				else
					first = false;

				conditionName << "AI_COND_PLAYER_CLOSE";
			}

			if(bitsPresentOfMask & AI_COND_HEARD_ENEMY_NEW_POSITION)
			{
				if(!first)
					conditionName << ", ";
				else
					first = false;

				conditionName << "AI_COND_HEARD_ENEMY_NEW_POSITION";
			}

			if(bitsPresentOfMask & AI_COND_SEE_HOSTILE_NPC)
			{
				if(!first)
					conditionName << ", ";
				else
					first = false;

				conditionName << "AI_COND_SEE_HOSTILE_NPC";
			}

			if(bitsPresentOfMask & AI_COND_RESERVED1)
			{
				if(!first)
					conditionName << ", ";
				else
					first = false;

				conditionName << "AI_COND_RESERVED1";
			}

			if(bitsPresentOfMask & AI_COND_RESERVED2)
			{
				if(!first)
					conditionName << ", ";
				else
					first = false;

				conditionName << "AI_COND_RESERVED2";
			}

			if(bitsPresentOfMask & AI_COND_RESERVED1)
			{
				if(!first)
					conditionName << ", ";
				else
					first = false;

				conditionName << "AI_COND_RESERVED1";
			}

			if(bitsPresentOfMask & AI_COND_RESERVED2)
			{
				if(!first)
					conditionName << ", ";
				else
					first = false;

				conditionName << "AI_COND_RESERVED2";
			}

			if(bitsPresentOfMask & AI_COND_RESERVED3)
			{
				if(!first)
					conditionName << ", ";
				else
					first = false;

				conditionName << "AI_COND_RESERVED3";
			}

			if(bitsPresentOfMask & AI_COND_RESERVED4)
			{
				if(!first)
					conditionName << ", ";
				else
					first = false;

				conditionName << "AI_COND_RESERVED4";
			}

			if(conditionName.empty())
				conditionName = "Unknown condition";

			msg << " " << conditionName << "\n";

			// Tell the console what interrupted the schedule
			Util::EntityConDPrintf(m_pEdict, msg.c_str());
		}

		return false;
	}

	// Allow inverse interrupt mask to also interrupt
	Uint64 inverseInterruptMask = m_pSchedule->GetInverseInterruptMask();
	Uint64 setBitsOfMask = (aiConditions & inverseInterruptMask);
		
	if(setBitsOfMask != inverseInterruptMask)
	{
		Uint64 missingBitsOfMask = inverseInterruptMask & ~setBitsOfMask;
		
		CString msg;
		msg << "Schedule " << m_pSchedule->GetName() << " interrupted by non-set AI condition ";

		bool first = true;
		CString conditionName;
		if(missingBitsOfMask & AI_COND_NO_AMMO_LOADED)
		{
			conditionName = "AI_COND_NO_AMMO_LOADED";
			first = false;
		}

		if(missingBitsOfMask & AI_COND_SEE_HATE)
		{
			if(!first)
				conditionName << ", ";
			else
				first = false;

			conditionName << "AI_COND_SEE_HATE";
		}

		if(missingBitsOfMask & AI_COND_SEE_FEAR)
		{
			if(!first)
				conditionName << ", ";
			else
				first = false;

			conditionName << "AI_COND_SEE_FEAR";
		}

		if(missingBitsOfMask & AI_COND_SEE_DISLIKE)
		{
			if(!first)
				conditionName << ", ";
			else
				first = false;

			conditionName << "AI_COND_SEE_DISLIKE";
		}

		if(missingBitsOfMask & AI_COND_SEE_ENEMY)
		{
			if(!first)
				conditionName << ", ";
			else
				first = false;

			conditionName << "AI_COND_SEE_ENEMY";
		}

		if(missingBitsOfMask & AI_COND_ENEMY_OCCLUDED)
		{
			if(!first)
				conditionName << ", ";
			else
				first = false;

			conditionName << "AI_COND_ENEMY_OCCLUDED";
		}

		if(missingBitsOfMask & AI_COND_SMELL_FOOD)
		{
			if(!first)
				conditionName << ", ";
			else
				first = false;

			conditionName << "AI_COND_SMELL_FOOD";
		}

		if(missingBitsOfMask & AI_COND_ENEMY_TOO_FAR)
		{
			if(!first)
				conditionName << ", ";
			else
				first = false;

			conditionName << "AI_COND_ENEMY_TOO_FAR";
		}

		if(missingBitsOfMask & AI_COND_LIGHT_DAMAGE)
		{
			if(!first)
				conditionName << ", ";
			else
				first = false;

			conditionName << "AI_COND_LIGHT_DAMAGE";
		}

		if(missingBitsOfMask & AI_COND_HEAVY_DAMAGE)
		{
			if(!first)
				conditionName << ", ";
			else
				first = false;

			conditionName << "AI_COND_HEAVY_DAMAGE";
		}

		if(missingBitsOfMask & AI_COND_CAN_RANGE_ATTACK1)
		{
			if(!first)
				conditionName << ", ";
			else
				first = false;

			conditionName << "AI_COND_CAN_RANGE_ATTACK1";
		}

		if(missingBitsOfMask & AI_COND_CAN_RANGE_ATTACK2)
		{
			if(!first)
				conditionName << ", ";
			else
				first = false;

			conditionName << "AI_COND_CAN_RANGE_ATTACK2";
		}

		if(missingBitsOfMask & AI_COND_CAN_MELEE_ATTACK1)
		{
			if(!first)
				conditionName << ", ";
			else
				first = false;

			conditionName << "AI_COND_CAN_MELEE_ATTACK1";
		}

		if(missingBitsOfMask & AI_COND_CAN_MELEE_ATTACK2)
		{
			if(!first)
				conditionName << ", ";
			else
				first = false;

			conditionName << "AI_COND_CAN_MELEE_ATTACK2";
		}

		if(missingBitsOfMask & AI_COND_CAN_SPECIAL_ATTACK1)
		{
			if(!first)
				conditionName << ", ";
			else
				first = false;

			conditionName << "AI_COND_CAN_SPECIAL_ATTACK1";
		}

		if(missingBitsOfMask & AI_COND_CAN_SPECIAL_ATTACK2)
		{
			if(!first)
				conditionName << ", ";
			else
				first = false;

			conditionName << "AI_COND_CAN_SPECIAL_ATTACK2";
		}

		if(missingBitsOfMask & AI_COND_FOLLOW_TARGET_TOO_FAR)
		{
			if(!first)
				conditionName << ", ";
			else
				first = false;

			conditionName << "AI_COND_FOLLOW_TARGET_TOO_FAR";
		}

		if(missingBitsOfMask & AI_COND_PROVOKED)
		{
			if(!first)
				conditionName << ", ";
			else
				first = false;

			conditionName << "AI_COND_PROVOKED";
		}

		if(missingBitsOfMask & AI_COND_NEW_ENEMY)
		{
			if(!first)
				conditionName << ", ";
			else
				first = false;

			conditionName << "AI_COND_NEW_ENEMY";
		}

		if(missingBitsOfMask & AI_COND_HEAR_SOUND)
		{
			if(!first)
				conditionName << ", ";
			else
				first = false;

			conditionName << "AI_COND_HEAR_SOUND";
		}

		if(missingBitsOfMask & AI_COND_SMELL)
		{
			if(!first)
				conditionName << ", ";
			else
				first = false;

			conditionName << "AI_COND_SMELL";
		}

		if(missingBitsOfMask & AI_COND_ENEMY_FACING_ME)
		{
			if(!first)
				conditionName << ", ";
			else
				first = false;

			conditionName << "AI_COND_ENEMY_FACING_ME";
		}

		if(missingBitsOfMask & AI_COND_ENEMY_DEAD)
		{
			if(!first)
				conditionName << ", ";
			else
				first = false;

			conditionName << "AI_COND_ENEMY_DEAD";
		}

		if(missingBitsOfMask & AI_COND_SEE_CLIENT)
		{
			if(!first)
				conditionName << ", ";
			else
				first = false;

			conditionName << "AI_COND_SEE_CLIENT";
		}

		if(missingBitsOfMask & AI_COND_SEE_NEMESIS)
		{
			if(!first)
				conditionName << ", ";
			else
				first = false;

			conditionName << "AI_COND_SEE_NEMESIS";
		}

		if(missingBitsOfMask & AI_COND_ENEMY_NOT_FOUND)
		{
			if(!first)
				conditionName << ", ";
			else
				first = false;

			conditionName << "AI_COND_ENEMY_NOT_FOUND";
		}

		if(missingBitsOfMask & AI_COND_ENEMY_UNREACHABLE)
		{
			if(!first)
				conditionName << ", ";
			else
				first = false;

			conditionName << "AI_COND_ENEMY_UNREACHABLE";
		}

		if(missingBitsOfMask & AI_COND_IN_DANGER)
		{
			if(!first)
				conditionName << ", ";
			else
				first = false;

			conditionName << "AI_COND_IN_DANGER";
		}

		if(missingBitsOfMask & AI_COND_DANGEROUS_ENEMY_CLOSE)
		{
			if(!first)
				conditionName << ", ";
			else
				first = false;

			conditionName << "AI_COND_DANGEROUS_ENEMY_CLOSE";
		}

		if(missingBitsOfMask & AI_COND_BLOCKING_PATH)
		{
			if(!first)
				conditionName << ", ";
			else
				first = false;

			conditionName << "AI_COND_BLOCKING_PATH";
		}

		if(missingBitsOfMask & AI_COND_TASK_FAILED)
		{
			if(!first)
				conditionName << ", ";
			else
				first = false;

			conditionName << "AI_COND_TASK_FAILED";
		}

		if(missingBitsOfMask & AI_COND_CLIENT_UNSEEN)
		{
			if(!first)
				conditionName << ", ";
			else
				first = false;

			conditionName << "AI_COND_CLIENT_UNSEEN";
		}

		if(missingBitsOfMask & AI_COND_PLAYER_CLOSE)
		{
			if(!first)
				conditionName << ", ";
			else
				first = false;

			conditionName << "AI_COND_PLAYER_CLOSE";
		}

		if(missingBitsOfMask & AI_COND_CAN_ATTACK)
		{
			if(!first)
				conditionName << ", ";
			else
				first = false;

			conditionName << "AI_COND_CAN_ATTACK";
		}

		if(missingBitsOfMask & AI_COND_HEARD_ENEMY_NEW_POSITION)
		{
			if(!first)
				conditionName << ", ";
			else
				first = false;

			conditionName << "AI_COND_HEARD_ENEMY_NEW_POSITION";
		}

		if(missingBitsOfMask & AI_COND_SEE_HOSTILE_NPC)
		{
			if(!first)
				conditionName << ", ";
			else
				first = false;

			conditionName << "AI_COND_SEE_HOSTILE_NPC";
		}

		if(missingBitsOfMask & AI_COND_RESERVED1)
		{
			if(!first)
				conditionName << ", ";
			else
				first = false;

			conditionName << "AI_COND_RESERVED1";
		}

		if(missingBitsOfMask & AI_COND_RESERVED2)
		{
			if(!first)
				conditionName << ", ";
			else
				first = false;

			conditionName << "AI_COND_RESERVED2";
		}

		if(missingBitsOfMask & AI_COND_RESERVED1)
		{
			if(!first)
				conditionName << ", ";
			else
				first = false;

			conditionName << "AI_COND_RESERVED1";
		}

		if(missingBitsOfMask & AI_COND_RESERVED2)
		{
			if(!first)
				conditionName << ", ";
			else
				first = false;

			conditionName << "AI_COND_RESERVED2";
		}

		if(missingBitsOfMask & AI_COND_RESERVED3)
		{
			if(!first)
				conditionName << ", ";
			else
				first = false;

			conditionName << "AI_COND_RESERVED3";
		}

		if(missingBitsOfMask & AI_COND_RESERVED4)
		{
			if(!first)
				conditionName << ", ";
			else
				first = false;

			conditionName << "AI_COND_RESERVED4";
		}

		if(conditionName.empty())
			conditionName = "Unknown condition";

		msg << " " << conditionName << "\n";

		// Tell the AI what interrupted the schedule
		Util::EntityConDPrintf(m_pEdict, msg.c_str());
		return false;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::MaintainSchedule( void )
{
	// Number of tasks executed so far
	Uint32 numTasksExecuted = 0;

	// Tells whether we should keep trying new schedules.
	// This is needed if we keep picking the same schedule
	// over and over again, leading to an infinite loop.
	Uint32 numScheduleChanges = 0;
	while(numScheduleChanges < NPC_MAX_SCHEDULE_CHANGES)
	{
		const CAISchedule* pNewSchedule = nullptr;

		// Begin the next task if you can
		if(m_pSchedule && IsTaskComplete())
			BeginNextScheduledTask();

		if(!IsScheduleValid() || m_npcState != m_idealNPCState)
		{
			// Change the schedule
			OnScheduleChange();

			if(m_idealNPCState != NPC_STATE_DEAD && (m_idealNPCState != NPC_STATE_SCRIPT || m_idealNPCState == m_npcState))
			{
				if(m_aiConditionBits && !CheckConditions(AI_COND_SCHEDULE_DONE)
					|| m_pSchedule && (m_pSchedule->GetInterruptMask() & AI_COND_SCHEDULE_DONE)
					|| m_npcState == NPC_STATE_COMBAT && !m_enemy)
				{
					// Set ideal state
					GetIdealNPCState();
				}
			}

			if(CheckConditions(AI_COND_TASK_FAILED) && m_npcState == m_idealNPCState)
			{
				// Get the appropriate schedule if the task failed
				Int32 newScheduleIndex = (m_failureScheduleIndex != AI_SCHED_NONE) ? m_failureScheduleIndex : AI_SCHED_FAIL;
				pNewSchedule = GetScheduleByIndex(newScheduleIndex);

				// Tell that the schedule failed
				if(m_pSchedule)
					Util::EntityConDPrintf(m_pEdict, "Schedule '%s' failed at task %d.\n", m_pSchedule->GetName(), m_scheduleTaskIndex);
			}
			else if(!CheckConditions(AI_COND_TASK_FAILED)
				&& m_pSchedule && m_pSchedule->GetSpecialInterruptScheduleIndex() != AI_SCHED_NONE
				&& m_pSchedule->GetSpecialInterruptMask() != AI_COND_NONE
				&& (m_pSchedule->GetSpecialInterruptExceptionMask() == AI_COND_NONE
				|| !CheckConditions(m_pSchedule->GetSpecialInterruptExceptionMask()))
				&& (m_pSchedule->GetSpecialInterruptRequirementMask() == AI_COND_NONE
				|| CheckConditions(m_pSchedule->GetSpecialInterruptRequirementMask()))
				&& CheckConditions(m_pSchedule->GetSpecialInterruptMask())
				)
			{
				// Get new schedule
				pNewSchedule = GetScheduleByIndex(m_pSchedule->GetSpecialInterruptScheduleIndex());
				if(!pNewSchedule)
				{
					Util::EntityConPrintf(m_pEdict, "Failed to get special interrupt schedule.\n");
					pNewSchedule = GetScheduleByIndex(AI_SCHED_FAIL);
				}
			}
			else
			{
				if(m_nextScheduleIndex != AI_SCHED_NONE)
				{
					// Get next schedule marked to play
					pNewSchedule = GetScheduleByIndex(m_nextScheduleIndex);
				}
				else
				{
					// Make sure the ideal state is set
					SetNPCState((npcstate_t)m_idealNPCState);

					// SCRIPT and DEAD always take from base class
					if(m_npcState == NPC_STATE_SCRIPT || m_npcState == NPC_STATE_DEAD)
						pNewSchedule = CBaseNPC::GetSchedule();
					else
						pNewSchedule = GetScheduleMain();
				}
			}

			// Change to the new schedule
			ChangeSchedule(pNewSchedule);
			numScheduleChanges++;
		}

		if(m_taskStatus == TASK_STATUS_NEW)
		{
			const ai_task_t* pTask = GetTask();
			if(!pTask)
			{
				Util::EntityConDPrintf(m_pEdict, "%s - GetTask returned null.\n", __FUNCTION__);
				return;
			}

			TaskBegin();
			StartTask(pTask);
			numTasksExecuted++;
		}

		activity_t idealActivity = (activity_t)GetIdealActivity();
		if(m_currentActivity != idealActivity)
			SetActivity(idealActivity);

		// TASK_STATUS_FAILED_NO_RETRY should cause this to abort, but abort if numTasksExecuted is too high
		if(!IsTaskComplete() && m_taskStatus != TASK_STATUS_NEW 
			&& ( m_taskStatus != TASK_STATUS_FAILED 
			|| m_taskStatus == TASK_STATUS_FAILED 
			&& numTasksExecuted >= NPC_MAX_TASK_EXECUTIONS))
			break;
	}

	if(IsTaskRunning())
	{
		const ai_task_t* pTask = GetTask();
		if(!pTask)
		{
			Util::EntityConDPrintf(m_pEdict, "%s - GetTask returned null.\n", __FUNCTION__);
			return;
		}

		RunTask(pTask);
	}

	// Make sure ideal activity is set
	activity_t idealActivity = (activity_t)GetIdealActivity();
	if(m_currentActivity != idealActivity)
		SetActivity(idealActivity);
}

//=============================================
// @brief
//
//=============================================
const CAISchedule* CBaseNPC::GetScheduleMain( void )
{
	if(m_npcPullerEntity)
	{
		if(!CheckConditions(AI_COND_NOT_ONGROUND))
			return GetScheduleByIndex(AI_SCHED_RUN_FROM_NPC_PULLER);
		else
			return GetScheduleByIndex(AI_SCHED_HOVER_BY_NPC_PULLER);
	}
	else
	{
		// Allow NPC to pick their own schedule
		return GetSchedule();
	}
}

//=============================================
// @brief
//
//=============================================
const CAISchedule* CBaseNPC::GetSchedule( void )
{
	if(m_npcState == NPC_STATE_IDLE && CheckConditions(AI_COND_ENEMY_DEAD))
	{
		// Forget this in idle
		if(HasMemory(AI_MEMORY_DODGE_ENEMY_FAILED))
			ClearMemory(AI_MEMORY_DODGE_ENEMY_FAILED);

		// Clear enemy dead state in alert or idle
		ClearConditions(AI_COND_ENEMY_DEAD|AI_COND_ENEMY_NAVIGABLE);
	}

	// Do not bother with AI_COND_FOLLOW_TARGET_TOO_FAR if in ALERT
	if((m_npcState == NPC_STATE_ALERT || m_npcState == NPC_STATE_COMBAT)
		&& CheckConditions(AI_COND_FOLLOW_TARGET_TOO_FAR))
		ClearConditions(AI_COND_FOLLOW_TARGET_TOO_FAR);

	// Choose an appropriate schedule
	switch(m_npcState)
	{
	case NPC_STATE_NONE:
		{
			// This should never happen
			Util::EntityConPrintf(m_pEdict, "GetSchedule called while npc AI state is NPC_STATE_NONE.\n");
		}
		break;
	case NPC_STATE_IDLE:
		{
			if(CheckConditions(AI_COND_BLOCKING_PATH))
				return GetScheduleByIndex(AI_SCHED_MOVE_OUT_OF_WAY);
			else if(CheckConditions(AI_COND_HEAR_SOUND))
				return GetScheduleByIndex(AI_SCHED_ALERT_FACE);
			else if(IsRouteClear())
				return GetScheduleByIndex(AI_SCHED_IDLE_STAND);
			else
				return GetScheduleByIndex(AI_SCHED_IDLE_WALK);
		}
		break;
	case NPC_STATE_ALERT:
		{
			if(CheckConditions(AI_COND_BLOCKING_PATH))
				return GetScheduleByIndex(AI_SCHED_MOVE_OUT_OF_WAY);
			else if(CheckConditions(AI_COND_ENEMY_DEAD) && FindActivity(ACT_VICTORY_DANCE) != NO_SEQUENCE_VALUE)
				return GetScheduleByIndex(AI_SCHED_VICTORY_DANCE);
			else if(CheckConditions(AI_COND_LIGHT_DAMAGE|AI_COND_HEAVY_DAMAGE))
			{
				Float coverYaw = SDL_fabs(GetYawDifference());
				if(CheckConditions(AI_COND_HEAVY_DAMAGE) && coverYaw < (1.0 - m_fieldOfView) * 60)
					return GetScheduleByIndex(AI_SCHED_TAKE_COVER_FROM_ORIGIN);
				else
					return GetScheduleByIndex(AI_SCHED_ALERT_SMALL_FLINCH);
			}
			else if(CheckConditions(AI_COND_HEAR_SOUND))
				return GetScheduleByIndex(AI_SCHED_ALERT_FACE);
			else
				return GetScheduleByIndex(AI_SCHED_ALERT_STAND);
		}
		break;
	case NPC_STATE_COMBAT:
		{
			if(CheckConditions(AI_COND_ENEMY_DEAD))
			{
				// Clear enemy
				m_enemy.reset();

				// Find a new enemy
				if(GetEnemy())
				{
					ClearConditions(AI_COND_ENEMY_DEAD|AI_COND_ENEMY_NAVIGABLE);
					return GetSchedule();
				}
				else
				{
					SetNPCState(NPC_STATE_ALERT);
					return GetSchedule();
				}
			}
			else if(m_enemy && m_enemy->IsNPCDangerous())
			{
				if(!m_enemy->IsAwareOf(this) && CheckConditions(AI_COND_SEE_ENEMY))
					return GetScheduleByIndex(AI_SCHED_TAKE_COVER_FROM_ENEMY);
				else
					return GetScheduleByIndex(AI_SCHED_STANDOFF);
			}
			else if(CheckConditions(AI_COND_NEW_ENEMY))
				return GetScheduleByIndex(AI_SCHED_BECOME_ALERT);
			else if(CheckConditions(AI_COND_LIGHT_DAMAGE) && !HasMemory(AI_MEMORY_FLINCHED))
				return GetScheduleByIndex(AI_SCHED_SMALL_FLINCH);
			else if(!CheckConditions(AI_COND_SEE_ENEMY))
			{
				if(!CheckConditions(AI_COND_ENEMY_OCCLUDED))
					return GetScheduleByIndex(AI_SCHED_COMBAT_FACE);
				else if(m_enemy->IsNPCDangerous())
				{
					Float fogEndDistance = CEnvFog::GetFogEndDistance();
					Float minCoverDistance = fogEndDistance > 0 ? fogEndDistance : NPC_DANGEROUS_ENEMY_MIN_COVER_DISTANCE;
					Float enemyDistance = (m_pState->origin - m_enemy->GetNavigablePosition()).Length();
					
					if(enemyDistance < minCoverDistance)
						return GetScheduleByIndex(AI_SCHED_TAKE_COVER_FROM_ENEMY);
					else
						return GetScheduleByIndex(AI_SCHED_COMBAT_FACE);
				}
				else if(!CheckConditions(AI_COND_ENEMY_NOT_FOUND))
					return GetScheduleByIndex(AI_SCHED_CHASE_ENEMY);
				else
					return GetScheduleByIndex(AI_SCHED_FIND_ENEMIES);
			}
			else if(CheckConditions(AI_COND_CAN_RANGE_ATTACK1))
				return GetScheduleByIndex(AI_SCHED_RANGE_ATTACK1);
			else if(CheckConditions(AI_COND_CAN_RANGE_ATTACK2))
				return GetScheduleByIndex(AI_SCHED_RANGE_ATTACK2);
			else if(CheckConditions(AI_COND_CAN_MELEE_ATTACK1))
				return GetScheduleByIndex(AI_SCHED_MELEE_ATTACK1);
			else if(CheckConditions(AI_COND_CAN_MELEE_ATTACK2))
				return GetScheduleByIndex(AI_SCHED_MELEE_ATTACK2);
			else if(CheckConditions(AI_COND_CAN_SPECIAL_ATTACK1))
				return GetScheduleByIndex(AI_SCHED_SPECIAL_ATTACK1);
			else if(CheckConditions(AI_COND_CAN_SPECIAL_ATTACK2))
				return GetScheduleByIndex(AI_SCHED_SPECIAL_ATTACK2);
			else if(!CheckConditions(AI_COND_CAN_RANGE_ATTACK1|AI_COND_CAN_RANGE_ATTACK2))
				return GetScheduleByIndex(AI_SCHED_CHASE_ENEMY);
			else if(!IsFacingIdealYaw())
				return GetScheduleByIndex(AI_SCHED_COMBAT_FACE);
			else
				Util::EntityConDPrintf(m_pEdict, "No fitting combat schedule.\n");
		}
		break;
	case NPC_STATE_DEAD:
		{
			return GetScheduleByIndex(AI_SCHED_DIE);
		}
		break;
	case NPC_STATE_SCRIPT:
		{
			if(!m_pScriptedSequence)
			{
				Util::EntityConDPrintf(m_pEdict, "Script failed for npc.\n");
				CleanupScriptedSequence();
				return GetScheduleByIndex(AI_SCHED_IDLE_STAND);
			}
			else
				return GetScheduleByIndex(AI_SCHED_AISCRIPT);
		}
		break;
	default:
		{
			Util::EntityConPrintf(m_pEdict, "%s - Unknown AI state.\n", __FUNCTION__);
		}
		break;
	}

	return &scheduleError;
}

//=============================================
// @brief
//
//=============================================
const CAISchedule* CBaseNPC::GetScheduleByIndex( Int32 scheduleIndex )
{
	switch(scheduleIndex)
	{
	case AI_SCHED_AISCRIPT:
		{
			if(!m_pScriptedSequence)
			{
				Util::EntityConDPrintf(m_pEdict, "Script failed for npc.\n");
				CleanupScriptedSequence();
				return GetScheduleByIndex(AI_SCHED_IDLE_STAND);
			}

			Int32 moveToSetting = m_pScriptedSequence->GetMoveToSetting();
			switch(moveToSetting)
			{
			case SCRIPT_MOVETO_NO:
			case SCRIPT_MOVETO_INSTANTENOUS:
				return &scheduleWaitForScript;
			case SCRIPT_MOVETO_WALK:
				return &scheduleScriptedWalk;
			case SCRIPT_MOVETO_RUN:
				return &scheduleScriptedRun;
			case SCRIPT_MOVETO_TURN_TO_FACE:
				return &scheduleFaceScript;
			case SCRIPT_MOVETO_WALK_NO_TURN:
				return &scheduleScriptedWalkNoFace;
			case SCRIPT_MOVETO_RUN_NO_TURN:
				return &scheduleScriptedRunNoFace;
			case SCRIPT_MOVETO_TURN_TO_PLAYER:
				return &scheduleScriptedFacePlayer;
			default:
				Util::EntityConPrintf(m_pEdict, "Invalid case '%d' for scripted sequence.\n", moveToSetting);
				CleanupScriptedSequence();
				return GetScheduleByIndex(AI_SCHED_IDLE_STAND);
			}
		}
		break;
	case AI_SCHED_IDLE_STAND:
		{
			if(CanActiveIdle() && Common::RandomLong(0, 10) == 0)
				return &scheduleActiveIdle;
			else
				return GetIdleStandSchedule(scheduleIndex);
		}
		break;
	case AI_SCHED_IDLE_WALK:
		{
			return &scheduleIdleWalk;
		}
		break;
	case AI_SCHED_BECOME_ALERT:
		{
			return &scheduleBecomeAlert;
		}
		break;
	case AI_SCHED_ALERT_FACE:
		{
			return &scheduleAlertFace;
		}
		break;
	case AI_SCHED_ALERT_STAND:
		{
			return &scheduleAlertStand;
		}
		break;
	case AI_SCHED_COMBAT_STAND:
		{
			return &scheduleCombatStand;
		}
		break;
	case AI_SCHED_COMBAT_FACE:
		{
			return &scheduleCombatFace;
		}
		break;
	case AI_SCHED_CHASE_ENEMY:
		{
			return &scheduleChaseEnemy;
		}
		break;
	case AI_SCHED_CHASE_ENEMY_FAILED:
		{
			return &scheduleCombatFail;
		}
		break;
	case AI_SCHED_SMALL_FLINCH:
		{
			return &scheduleSmallFlinch;
		}
		break;
	case AI_SCHED_ALERT_SMALL_FLINCH:
		{
			return &scheduleAlertSmallFlinch;
		}
		break;
	case AI_SCHED_RELOAD:
		{
			return &scheduleReload;
		}
		break;
	case AI_SCHED_HIDE_RELOAD:
		{
			return &scheduleHideReload;
		}
		break;
	case AI_SCHED_ARM_WEAPON:
		{
			return &scheduleArmWeapon;
		}
		break;
	case AI_SCHED_STANDOFF:
		{
			return &scheduleStandoff;
		}
		break;
	case AI_SCHED_RANGE_ATTACK1:
		{
			return &scheduleRangeAttack1;
		}
		break;
	case AI_SCHED_RANGE_ATTACK2:
		{
			return &scheduleRangeAttack2;
		}
		break;
	case AI_SCHED_MELEE_ATTACK1:
		{
			return &scheduleMeleeAttack1;
		}
		break;
	case AI_SCHED_MELEE_ATTACK2:
		{
			return &scheduleMeleeAttack2;
		}
		break;
	case AI_SCHED_SPECIAL_ATTACK1:
		{
			return &scheduleSpecialAttack1;
		}
		break;
	case AI_SCHED_SPECIAL_ATTACK2:
		{
			return &scheduleSpecialAttack2;
		}
		break;
	case AI_SCHED_TAKE_COVER_FROM_BEST_SOUND:
		{
			return &scheduleTakeCoverFromBestSound;
		}
		break;
	case AI_SCHED_TAKE_COVER_FROM_BEST_SOUND_WITH_COWER:
		{
			return &scheduleTakeCoverFromBestSoundWithCower;
		}
		break;
	case AI_SCHED_TAKE_COVER_FROM_ENEMY:
		{
			return &scheduleTakeCoverFromEnemy;
		}
		break;
	case AI_SCHED_COWER:
		{
			return &scheduleCower;
		}
		break;
	case AI_SCHED_AMBUSH:
		{
			return &scheduleAmbush;
		}
		break;
	case AI_SCHED_INVESTIGATE_SOUND:
		{
			return &scheduleInvestigateSound;
		}
		break;
	case AI_SCHED_DIE:
		{
			return &scheduleDie;
		}
		break;
	case AI_SCHED_TAKE_COVER_FROM_ORIGIN:
		{
			return &scheduleTakeCoverFromOrigin;
		}
		break;
	case AI_SCHED_VICTORY_DANCE:
		{
			return &scheduleVictoryDance;
		}
		break;
	case AI_SCHED_FAIL:
		{
			if(m_npcState == NPC_STATE_COMBAT)
				return &scheduleCombatFail;
			else
				return &scheduleFail;
		}
		break;
	case AI_SCHED_MOVE_OUT_OF_WAY:
		{
			return &scheduleMoveOutOfWay;
		}
		break;
	case AI_SCHED_FIND_ENEMIES:
		{
			return &scheduleFindEnemies;
		}
		break;
	case AI_SCHED_DODGE_ENEMY:
		{
			return &scheduleDodgeEnemy;
		}
		break;
	case AI_SCHED_INSPECT_ENEMY_CORPSE:
		{
			return &scheduleInspectEnemyCorpse;
		}
		break;
	case AI_SCHED_ESTABLISH_LINE_OF_FIRE:
		{
			if(FavorRangedAttacks() 
				&& CheckConditions(AI_COND_SHOOT_VECTOR_VALID) 
				&& CheckConditions(AI_COND_SEE_ENEMY))
				return &scheduleFaceEnemy;
			else
				return &scheduleEstablishLineOfFire;
		}
		break;
	case AI_SCHED_ELOF_FAIL:
		{
			if(CheckConditions(AI_COND_CAN_RANGE_ATTACK1))
				return GetScheduleByIndex(AI_SCHED_RANGE_ATTACK1);
			else
				return GetScheduleByIndex(AI_SCHED_FAIL);
		}
		break;
	case AI_SCHED_SWEEP:
		{
			return &scheduleSweep;
		}
		break;
	case AI_SCHED_WAIT_FACE_ENEMY:
		{
			return &scheduleWaitFaceEnemy;
		}
		break;
	case AI_SCHED_RUN_FROM_NPC_PULLER:
		{
			return &scheduleRunFromNPCPuller;
		}
		break;
	case AI_SCHED_HOVER_BY_NPC_PULLER:
		{
			return &scheduleHoverByNPCPuller;
		}
		break;
	case AI_SCHED_FACE_ENEMY:
		{
			return &scheduleFaceEnemy;
		}
		break;
	default:
		{
			Util::EntityConPrintf(m_pEdict, "%s - No case for schedule index '%d'.\n", __FUNCTION__, scheduleIndex);
			return GetIdleStandSchedule(scheduleIndex);
		}
		break;
	}
}

//=============================================
// @brief Returns the schedule for idle stand
//
//=============================================
const CAISchedule* CBaseNPC::GetIdleStandSchedule( Int32 scheduleIndex )
{
	return &scheduleIdleStand;
}

//=============================================
// @brief Called when a schedule is changed
//
//=============================================
void CBaseNPC::OnScheduleChange( void )
{
	// Make sure this is cleared
	m_checkSoundWasSet = false;
}
