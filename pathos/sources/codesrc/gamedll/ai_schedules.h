/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef AI_SCHEDULES_H
#define AI_SCHEDULES_H

enum ai_schedules_t
{
	AI_SCHED_NONE = 0,
	AI_SCHED_IDLE_STAND,
	AI_SCHED_IDLE_WALK,
	AI_SCHED_ALERT_FACE,
	AI_SCHED_ALERT_SMALL_FLINCH,
	AI_SCHED_ALERT_BIG_FLINCH,
	AI_SCHED_ALERT_STAND,
	AI_SCHED_INVESTIGATE_SOUND,
	AI_SCHED_COMBAT_FACE,
	AI_SCHED_COMBAT_STAND,
	AI_SCHED_CHASE_ENEMY,
	AI_SCHED_CHASE_ENEMY_FAILED,
	AI_SCHED_VICTORY_DANCE,
	AI_SCHED_SMALL_FLINCH,
	AI_SCHED_TAKE_COVER_FROM_ENEMY,
	AI_SCHED_TAKE_COVER_FROM_BEST_SOUND,
	AI_SCHED_TAKE_COVER_FROM_BEST_SOUND_WITH_COWER,
	AI_SCHED_TAKE_COVER_FROM_ORIGIN,
	AI_SCHED_COWER,
	AI_SCHED_MELEE_ATTACK1,
	AI_SCHED_MELEE_ATTACK2,
	AI_SCHED_RANGE_ATTACK1,
	AI_SCHED_RANGE_ATTACK2,
	AI_SCHED_SPECIAL_ATTACK1,
	AI_SCHED_SPECIAL_ATTACK2,
	AI_SCHED_STANDOFF,
	AI_SCHED_ARM_WEAPON,
	AI_SCHED_RELOAD,
	AI_SCHED_HIDE_RELOAD,
	AI_SCHED_GUARD,
	AI_SCHED_AMBUSH,
	AI_SCHED_DIE,
	AI_SCHED_WAIT_FOR_TRIGGER,
	AI_SCHED_FOLLOW,
	AI_SCHED_SLEEP,
	AI_SCHED_WAKE,
	AI_SCHED_AISCRIPT,
	AI_SCHED_FAIL,
	AI_SCHED_MOVE_OUT_OF_WAY,
	AI_SCHED_FIND_ENEMIES,
	AI_SCHED_DODGE_ENEMY,
	AI_SCHED_BECOME_ALERT,
	AI_SCHED_INSPECT_ENEMY_CORPSE,
	AI_SCHED_ESTABLISH_LINE_OF_FIRE,
	AI_SCHED_ELOF_FAIL,
	AI_SCHED_SWEEP,
	AI_SCHED_WAIT_FACE_ENEMY,
	AI_SCHED_RUN_FROM_NPC_PULLER,
	AI_SCHED_HOVER_BY_NPC_PULLER,
	AI_SCHED_FACE_ENEMY,

	// Must be last
	LAST_BASENPC_SCHEDULE
};

extern const CAISchedule scheduleFail;
extern const CAISchedule scheduleCombatFail;
extern const CAISchedule scheduleIdleStand;
extern const CAISchedule scheduleIdleWalk;
extern const CAISchedule scheduleAmbush;
extern const CAISchedule scheduleActiveIdle;
extern const CAISchedule scheduleBecomeAlert;
extern const CAISchedule scheduleAlertFace;
extern const CAISchedule scheduleAlertSmallFlinch;
extern const CAISchedule scheduleAlertStand;
extern const CAISchedule scheduleInvestigateSound;
extern const CAISchedule scheduleCombatStand;
extern const CAISchedule scheduleCombatFace;
extern const CAISchedule scheduleStandoff;
extern const CAISchedule scheduleArmWeapon;
extern const CAISchedule scheduleReload;
extern const CAISchedule scheduleHideReload;
extern const CAISchedule scheduleRangeAttack1;
extern const CAISchedule scheduleRangeAttack2;
extern const CAISchedule scheduleMeleeAttack1;
extern const CAISchedule scheduleMeleeAttack2;
extern const CAISchedule scheduleSpecialAttack1;
extern const CAISchedule scheduleSpecialAttack2;
extern const CAISchedule scheduleChaseEnemy;
extern const CAISchedule scheduleChaseEnemyFailed;
extern const CAISchedule scheduleSmallFlinch;
extern const CAISchedule scheduleDie;
extern const CAISchedule scheduleVictoryDance;
extern const CAISchedule scheduleError;
extern const CAISchedule scheduleScriptedWalk;
extern const CAISchedule scheduleScriptedWalkNoFace;
extern const CAISchedule scheduleScriptedRun;
extern const CAISchedule scheduleScriptedRunNoFace;
extern const CAISchedule scheduleWaitForScript;
extern const CAISchedule scheduleFaceScript;
extern const CAISchedule scheduleScriptedFacePlayer;
extern const CAISchedule scheduleCower;
extern const CAISchedule scheduleTakeCoverFromOrigin;
extern const CAISchedule scheduleTakeCoverFromBestSound;
extern const CAISchedule scheduleTakeCoverFromBestSoundWithCower;
extern const CAISchedule scheduleTakeCoverFromEnemy;
extern const CAISchedule scheduleMoveOutOfWay;
extern const CAISchedule scheduleGrabbed;
extern const CAISchedule scheduleFindEnemies;
extern const CAISchedule scheduleDodgeEnemy;

#endif //AI_SCHEDULES_H