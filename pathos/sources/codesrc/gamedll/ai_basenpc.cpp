/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

//
// I'd like to thank Valve for the AI code in the Half-Life SDK,
// which I used as a reference while writing this implementation
//

#include "includes.h"
#include "gd_includes.h"
#include "ai_basenpc.h"
#include "sentencesfile.h"
#include "envfog.h"
#include "flex_shared.h"
#include "flexmanager.h"
#include "game.h"
#include "player.h"
#include "envfog.h"
#include "ai_nodegraph.h"
#include "funcdoor.h"
#include "gib.h"
#include "cplane.h"
#include "textures_shared.h"
#include "timedamage.h"
#include "materialdefs.h"

// Delay between npc thinking
const Float CBaseNPC::NPC_THINK_TIME = 0.1f;
// Head turn speed on yaw
const Float CBaseNPC::NPC_HEAD_TURN_YAW_SPEED = 70;
// Head turn speed on pitch
const Float CBaseNPC::NPC_HEAD_TURN_PITCH_SPEED = 40;
// Maximum looking distance
const Float CBaseNPC::NPC_DEFAULT_MAX_LOOK_DISTANCE = 4096.0f;
// Maximum firing distance
const Float CBaseNPC::NPC_DEFAULT_MAX_FIRING_DISTANCE = 2048.0f;

// Default hearing sensitivity
const Float CBaseNPC::NPC_DEFAULT_HEARING_SENSITIVITY = 0.1f;
// Hearing lean awareness gain
const Float CBaseNPC::NPC_HEAR_LEAN_AWARENESS_GAIN = 0.5f;
// Lean awareness timeout
const Float CBaseNPC::NPC_LEANAWARENESS_TIMEOUT = 2;
// NPC step size
const Float CBaseNPC::NPC_STEP_SIZE = 16.0f;
// Maximum danger exposure time
const Float CBaseNPC::NPC_MAX_DANGER_TIME = 1.0f;
// Minimum enemy distance
const Float CBaseNPC::NPC_MINIMUM_ENEMY_DISTANCE = 128.0f;
// Enemy intercept distance
const Float CBaseNPC::NPC_ENEMY_INTERCEPT_DISTANCE = 64.0f;
// Enemy combat state timeout
const Float CBaseNPC::NPC_COMBATSTATE_TIMEOUT = 35.0f;
// Distance beyond which we'll try to cut corners
const Float CBaseNPC::NPC_CORNER_CUT_MIN_DIST = 2.0f;
// Enemy update distance
const Float CBaseNPC::NPC_ENEMY_UPDATE_DISTANCE = 64.0f;
// Triangulation maximum height
const Float CBaseNPC::NPC_TRIANGULATION_MAX_HEIGHT = 72.0f;
// Triangulation minimum x size
const Float CBaseNPC::NPC_TRIANGULATION_MIN_SIZE_X = 24.0f;
// Triangulation maximum x size
const Float CBaseNPC::NPC_TRIANGULATION_MAX_SIZE_X = 48.0f;
// Door search radius when looking for double doors
const Float CBaseNPC::NPC_DOOR_SEARCH_RADIUS = 256;
// Minimum localmove check distance
const Float CBaseNPC::MIN_LOCALMOVE_CHECK_DIST = 32;
// Maximum distance the NPC can simplify paths in
const Float CBaseNPC::NPC_MAX_SIMPLIFY_DISTANCE = 2048;
// Maximum traces an NPC can do while simplifying routes
const Uint32 CBaseNPC::NPC_MAX_SIMPLIFY_TRACES = 6;
// Distance between corner cut checks
const Float CBaseNPC::NPC_SIMPLIFICATION_FIX_DISTANCE = 40;
// NPC error glow aura color
const Vector CBaseNPC::NPC_ERROR_GLOW_AURA_COLOR = Vector(255, 0, 0);
// Default max distance for navigation
const Float CBaseNPC::NPC_MAX_NAVIGATION_DISTANCE = 4096;
// Dangerous enemy minimum distance
const Float CBaseNPC::NPC_DANGEROUS_ENEMY_MIN_DISTANCE = 180;
// Dangerous enemy minimum cover distance
const Float CBaseNPC::NPC_DANGEROUS_ENEMY_MIN_COVER_DISTANCE = 512;
// Enemy search distance
const Float CBaseNPC::NPC_MAX_ENEMY_SEARCH_DISTANCE = 1024;
// Number of lateral cover checks
const Float CBaseNPC::NPC_LATERAL_COVER_CHECK_NUM = 6;
// Lateral cover check distance
const Float CBaseNPC::NPC_LATERAL_COVER_CHECK_DISTANCE = 32;
// NPC follow walk distance
const Float CBaseNPC::NPC_FOLLOW_WALK_DISTANCE = 200;
// NPC follow run distance
const Float CBaseNPC::NPC_FOLLOW_RUN_DISTANCE = 300;
// NPC default cover distance
const Float CBaseNPC::NPC_DEFAULT_COVER_DISTANCE = 2048;
// NPC reload cover distance
const Float CBaseNPC::NPC_RELOAD_COVER_DISTANCE = 512;
// Best sound cover minimum distance
const Float CBaseNPC::NPC_COVER_BESTSOUND_MIN_DISTANCE = 64;
// Best sound cover maximum distance
const Float CBaseNPC::NPC_COVER_BESTSOUND_MAX_DISTANCE = 1024;
// Best sound cover optimal distance
const Float CBaseNPC::NPC_COVER_BESTSOUND_OPTIMAL_DISTANCE = 400;
// NPC move wait time
const Float CBaseNPC::NPC_DEFAULT_MOVE_WAIT_TIME = 2;
// Dodge min distance
const Float CBaseNPC::NPC_DODGE_MIN_DISTANCE = 128;
// Dodge max distance
const Float CBaseNPC::NPC_DODGE_MAX_DISTANCE = 512;
// Minimum health value for gibbing
const Float CBaseNPC::NPC_GIB_HEALTH_VALUE = -30;
// Bullet gib damage treshold
const Float CBaseNPC::NPC_BULLETGIB_DMG_TRESHOLD = 70;
// Bullet gibbing minimum health treshold
const Float CBaseNPC::NPC_BULLETGIB_MIN_HEALTH = 30;
// Light damage treshold
const Float CBaseNPC::NPC_LIGHT_DAMAGE_TRESHOLD = 1;
// Heavy damage treshold
const Float CBaseNPC::NPC_HEAVY_DAMAGE_TRESHOLD = 30;
// Default lean awareness time
const Float CBaseNPC::NPC_DEFAULT_LEAN_AWARE_TIME = 1.0f;
// Script move minimum distance
const Float CBaseNPC::NPC_SCRIPT_MOVE_MIN_DIST = 8.0f;
// NPC gun sound radius
const Float CBaseNPC::NPC_GUN_SOUND_RADIUS = 384.0f;
// Minimum size of an enemy the NPC will kick
const Float CBaseNPC::NPC_ENEMY_MIN_KICK_SIZE = 24.0f;
// NPC firing angle treshold
const Float CBaseNPC::NPC_FIRING_ANGLE_TRESHOLD = 0.5f;
// Max localmove height diff in start and end
const Float CBaseNPC::NPC_MAX_LOCALMOVE_HEIGHT_DIFF = 1024.0f;
// Distance at which we can be decapitated
const Float CBaseNPC::NPC_DECAP_MAX_DISTANCE = 512.0f;
// Distance at which we can be gibbed by a bullet
const Float CBaseNPC::NPC_BULLETGIB_MAX_DISTANCE = 512.0f;
// Number of coverage checks
const Uint32 CBaseNPC::NPC_NUM_COVERAGE_CHECKS = 6;
// Max number of schedule changes per think
const Uint32 CBaseNPC::NPC_MAX_SCHEDULE_CHANGES = 4;
// Max number of tasks executed
const Uint32 CBaseNPC::NPC_MAX_TASK_EXECUTIONS = 8;
// Navigability minimum distance change
const Float CBaseNPC::NAVIGABILITY_CHECK_MIN_DISTANCE_CHANGE = 64;

// AI state names
static const Char* AI_STATE_NAMES[NB_AI_STATES] = 
{
	"None",
	"Idle",
	"Combat",
	"Alert",
	"Script",
	"Dead"
};

// Last cover search node
Int32 CBaseNPC::g_lastCoverSearchNodeIndex = 0;
// Last active idle search node
Int32 CBaseNPC::g_lastActiveIdleSearchNodeIndex = 0;

// Door group bits
const Uint32 CBaseNPC::AI_CAP_DOORS_GROUP_BITS[] = {AI_CAP_USE, AI_CAP_AUTO_OPEN_DOORS, AI_CAP_OPEN_DOORS};
// Door group bitset
const CBitSet CBaseNPC::AI_CAP_GROUP_DOORS(AI_CAP_BITS_COUNT, AI_CAP_DOORS_GROUP_BITS, PT_ARRAYSIZE(AI_CAP_DOORS_GROUP_BITS));

//=============================================
// @brief Constructor
//
//=============================================
CBaseNPC::CBaseNPC( edict_t* pedict ):
	CAnimatingEntity(pedict),
	m_npcLastThinkTime(0),
	m_thinkIntervalTime(0),
	m_idealHeadYaw(0),
	m_headYaw(0),
	m_idealHeadPitch(0),
	m_headPitch(0),
	m_dontDropWeapons(false),
	m_currentActivity(ACT_RESET),
	m_idealActivity(0),
	m_lastActivityTime(0),
	m_taskStatus(0),
	m_pSchedule(nullptr),
	m_scheduleTaskIndex(0),
	m_failureScheduleIndex(AI_SCHED_NONE),
	m_nextScheduleIndex(AI_SCHED_NONE),
	m_npcState(0),
	m_idealNPCState(0),
	m_currentScheduleIndex(0),
	m_lookDistance(0),
	m_firingDistance(0),
	m_aiConditionBits(AI_COND_BITSET_SIZE),
	m_memoryBits(AI_MEMORY_BITSET_SIZE),
	m_capabilityBits(AI_CAP_BITS_COUNT),
	m_disabledCapabilityBits(AI_CAP_BITS_COUNT),
	m_damageBits(0),
	m_lastDamageAmount(0),
	m_enemyBodyTarget(BODYTARGET_CENTER),
	m_fieldOfView(0),
	m_waitFinishedTime(0),
	m_moveWaitFinishTime(0),
	m_lastHitGroup(0),
	m_movementGoal(0),
	m_routePointIndex(0),
	m_moveWaitTime(0),
	m_firstNodeIndex(0),
	m_canCutCorners(false),
	m_shortcutPathIndex(0),
	m_movementActivity(0),
	m_distanceTravelled(0),
	m_soundTypes(0),
	m_hintNodeIndex(0),
	m_maximumHealth(0),
	m_ammoLoaded(0),
	m_clipSize(0),
	m_bloodColor(0),
	m_talkTime(0),
	m_pBestSound(nullptr),
	m_valuesParsed(false),
	m_forceSkillCvar(FORCE_SKILL_OFF),
	m_lastClearNode(0),
	m_lastEnemySightTime(0),
	m_deathDamageBits(0),
	m_deathExplodeTime(0),
	m_deathMode(DEATH_NORMAL),
	m_lastDangerDistance(0),
	m_dangerExposure(0),
	m_isSeekingCover(false),
	m_activeFlexState(0),
	m_flexScriptDuration(0),
	m_flexScriptFlags(0),
	m_scriptState(AI_SCRIPT_STATE_NONE),
	m_pScriptedSequence(nullptr),
	m_updateYaw(false),
	m_checkSoundWasSet(false),
	m_triggerTarget1(NO_STRING_VALUE),
	m_triggerCondition1(AI_TRIGGER_NONE),
	m_triggerTarget2(NO_STRING_VALUE),
	m_triggerCondition2(AI_TRIGGER_NONE)
{
}

//=============================================
// @brief Destructor
//
//=============================================
CBaseNPC::~CBaseNPC( void )
{
}

//=============================================
// @brief Declares saved variables for the class
//
//=============================================
void CBaseNPC::DeclareSaveFields( void )
{
	// Call base class to do it first
	CAnimatingEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_enemy, EFIELD_EHANDLE));
	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_enemyLastKnownPosition, EFIELD_COORD));
	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_enemyLastKnownAngles, EFIELD_VECTOR));

	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_targetEntity, EFIELD_EHANDLE));

	// Unfortunately I can't solve this in a for loop
	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_backedUpEnemies[0].lastknownorigin, EFIELD_COORD));
	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_backedUpEnemies[0].lastknownangles, EFIELD_VECTOR));
	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_backedUpEnemies[0].lastsighttime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_backedUpEnemies[0].enemy, EFIELD_EHANDLE));

	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_backedUpEnemies[1].lastknownorigin, EFIELD_COORD));
	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_backedUpEnemies[1].lastknownangles, EFIELD_VECTOR));
	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_backedUpEnemies[1].lastsighttime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_backedUpEnemies[1].enemy, EFIELD_EHANDLE));

	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_backedUpEnemies[2].lastknownorigin, EFIELD_COORD));
	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_backedUpEnemies[2].lastknownangles, EFIELD_VECTOR));
	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_backedUpEnemies[2].lastsighttime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_backedUpEnemies[2].enemy, EFIELD_EHANDLE));

	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_backedUpEnemies[3].lastknownorigin, EFIELD_COORD));
	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_backedUpEnemies[3].lastknownangles, EFIELD_VECTOR));
	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_backedUpEnemies[3].lastsighttime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_backedUpEnemies[3].enemy, EFIELD_EHANDLE));

	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_fieldOfView, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_waitFinishedTime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_moveWaitFinishTime, EFIELD_TIME));

	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_currentActivity, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_idealActivity, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_lastHitGroup, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_npcState, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_idealNPCState, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_taskStatus, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_currentScheduleIndex, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_aiConditionBits, EFIELD_CBITSET));
	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_lastPosition, EFIELD_COORD));
	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_hintNodeIndex, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_maximumHealth, EFIELD_FLOAT));

	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_ammoLoaded, EFIELD_UINT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_clipSize, EFIELD_UINT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_capabilityBits, EFIELD_CBITSET));
	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_disabledCapabilityBits, EFIELD_CBITSET));
	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_memoryBits, EFIELD_CBITSET));
	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_damageBits, EFIELD_UINT64));

	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_bloodColor, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_failureScheduleIndex, EFIELD_INT32));

	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_lookDistance, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_firingDistance, EFIELD_FLOAT));

	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_triggerCondition1, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_triggerTarget1, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_triggerCondition2, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_triggerTarget2, EFIELD_STRING));

	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_scriptState, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_scriptEntity, EFIELD_EHANDLE));
	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_blockedNPC, EFIELD_EHANDLE));
	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_valuesParsed, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_forceSkillCvar, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_pScriptedSequence, EFIELD_ENTPOINTER));

	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_lastEnemySightTime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_deathMode, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_dontDropWeapons, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CBaseNPC, m_talkTime, EFIELD_TIME));
}

//=============================================
// @brief Manages keyvalues
//
// @param kv Keyvalue data
// @return TRUE if keyvalue as managed, false otherwise
//=============================================
bool CBaseNPC::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "TriggerTarget"))
	{
		m_triggerTarget1 = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "TriggerCondition"))
	{
		m_triggerCondition1 = SDL_atoi(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "TriggerTarget2"))
	{
		m_triggerTarget2 = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "TriggerCondition2"))
	{
		m_triggerCondition2 = SDL_atoi(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "nodropweapons"))
	{
		m_dontDropWeapons = (SDL_atoi(kv.value) == 1) ? true : false;
		return true;
	}
	else if(!qstrcmp(kv.keyname, "forceskill"))
	{
		Int32 value = SDL_atoi(kv.value);
		switch(value)
		{
		case FORCE_SKILL_OFF:
		case FORCE_SKILL_EASY:
		case FORCE_SKILL_NORMAL:
		case FORCE_SKILL_HARD:
		case FORCE_SKILL_LESS_THAN_HARD:
			m_forceSkillCvar = value;
			break;
		default:
			Util::EntityConPrintf(m_pEdict, "Invalid keyvalue '%s' for '%s'.\n", kv.value, kv.keyname);
			m_forceSkillCvar = FORCE_SKILL_OFF;
			break;
		}

		return true;
	}

	else
		return CAnimatingEntity::KeyValue(kv);
}

//=============================================
// @brief Restores the entity after a save load
//
// @return Result of save-restore attempt
//=============================================
bool CBaseNPC::Restore( void )
{
	if(!CAnimatingEntity::Restore())
		return false;

	// Reset the conditions if we've got no enemy
	if(!m_enemy)
		m_aiConditionBits.reset();

	if(m_pState->deadstate == DEADSTATE_DEAD)
		m_pState->forcehull = HULL_POINT;

	// Clear these so they don't get stuck
	if(m_damageBits & DMG_UNUSED2)
		m_damageBits &= ~DMG_UNUSED2;

	// Clear these so they don't get stuck
	if(m_damageBits & DMG_UNUSED3)
		m_damageBits &= ~DMG_UNUSED3;

	// Clear these so they don't get stuck
	if(m_damageBits & DMG_EXPLOSION)
		m_damageBits &= ~DMG_EXPLOSION;

	// Reset flex stuff
	ResetNPC();

	return true;
}

//=============================================
// @brief Performs precache functions
//
// @return Hearing sensitivity
//=============================================
void CBaseNPC::Precache( void )
{
	// Call base class to precache
	CAnimatingEntity::Precache();

	Util::PrecacheFixedNbSounds("common/bodydrop_light%d.wav", 2);
	Util::PrecacheFixedNbSounds("common/bodydrop_heavy%d.wav", 2);
}

//=============================================
// @brief Returns the hearing sensitvity of the NPC
//
// @return Hearing sensitivity
//=============================================
Float CBaseNPC::GetHearingSensitivity( void )
{
	return NPC_DEFAULT_HEARING_SENSITIVITY;
}

//=============================================
// @brief Sets ideal head angles
//
// @param pitch Ideal pitch value
// @param yaw Ideal yaw value
//=============================================
void CBaseNPC::SetIdealHeadAngles( Float pitch, Float yaw )
{
	if(!HasCapability(AI_CAP_TURN_HEAD))
		return;

	if(!SetIdealHeadYaw(yaw))
	{
		m_idealHeadPitch = 0;
		return;
	}

	Float _pitch = pitch;
	if(_pitch < 0)
		_pitch += 360;

	if(_pitch > 180)
	{
		m_idealHeadPitch = _pitch - 360;
		// Limit to 30 degrees
		if(m_idealHeadPitch < -30)
			m_idealHeadPitch = -30;
	}
	else if(_pitch > 30)
	{
		// Limit to 30 degrees
		m_idealHeadPitch = 30;
	}
	else
	{
		// Pitch is valid
		m_idealHeadPitch = _pitch;
	}
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::SetIdealHeadYaw( Float yaw )
{
	if(!HasCapability(AI_CAP_TURN_HEAD))
		return false;

	Float _yaw = yaw;
	if(_yaw < 0)
		_yaw += 360;

	if(_yaw > 180)
	{
		m_idealHeadYaw = _yaw - 360;
		if(m_idealHeadYaw < -60 && m_idealHeadYaw > -120)
		{
			// Limit to sixty degrees
			m_idealHeadYaw = -60;
		}
		else if(m_idealHeadYaw < -120)
		{
			m_idealHeadYaw = 0;
			return false;
		}
	}
	else if(_yaw > 60 && _yaw < 120)
	{
		// Limit to 60 degrees
		m_idealHeadYaw = 60;
	}
	else if(_yaw > 120)
	{
		m_idealHeadYaw = 0;
		return false;
	}
	else
		m_idealHeadYaw = _yaw;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::UpdateHeadControllers( void )
{
	// Apply head turning on yaw
	if(HasCapability(AI_CAP_TURN_HEAD))
	{
		Float currentyaw = Math::AngleMod(m_headYaw);
		if(currentyaw != m_idealHeadYaw)
		{
			Float moveamount = m_idealHeadYaw - currentyaw;
			Float turnspeed = NPC_HEAD_TURN_YAW_SPEED * m_thinkIntervalTime;

			if(m_idealHeadYaw > currentyaw)
			{
				if(moveamount >= 180)
					moveamount = moveamount - 360;
			}
			else if(moveamount <= -180)
				moveamount = moveamount + 360;

			if(moveamount > 0)
			{
				if(moveamount > turnspeed)
					moveamount = turnspeed;
			}
			else if(moveamount < -turnspeed)
				moveamount = -turnspeed;

			// Don't adjust if it's too small
			if(SDL_fabs(moveamount) > 1)
				m_headYaw = Math::AngleMod(currentyaw + moveamount);
		}

		SetBoneController(0, m_headYaw);
	}

	// Apply head turning on pitch
	if(HasCapability(AI_CAP_TURN_HEAD_PITCH))
	{
		Float currentpitch = Math::AngleMod(m_headPitch);
		if(currentpitch != m_idealHeadPitch)
		{
			Float moveamount = m_idealHeadPitch - currentpitch;
			Float turnspeed = NPC_HEAD_TURN_PITCH_SPEED * m_thinkIntervalTime;

			if(m_idealHeadPitch > currentpitch)
			{
				if(moveamount >= 180)
					moveamount = moveamount - 360;
			}
			else if(moveamount <= -180)
				moveamount = moveamount + 360;

			if(moveamount > 0)
			{
				if(moveamount > turnspeed)
					moveamount = turnspeed;
			}
			else if(moveamount < -turnspeed)
				moveamount = -turnspeed;

			// Don't adjust if it's too small
			if(SDL_fabs(moveamount) > 1)
				m_headPitch = Math::AngleMod(currentpitch + moveamount);
		}

		SetBoneController(1, m_headPitch);
	}
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::UpdateIdleAnimation( void )
{
	// Don't switch animations if dead or scripting
	if(m_npcState == NPC_STATE_SCRIPT 
		|| m_npcState == NPC_STATE_DEAD)
		return;

	// Only switch anims if idle and animation has done playing
	if(m_currentActivity != ACT_IDLE || !m_isSequenceFinished)
		return;

	Int32 sequenceIndex;
	if(m_isSequenceLooped)
		sequenceIndex = FindActivity(m_currentActivity);
	else
		sequenceIndex = FindHeaviestActivity(m_currentActivity);

	if(sequenceIndex != NO_SEQUENCE_VALUE)
	{
		m_pState->sequence = sequenceIndex;
		ResetSequenceInfo();
	}
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::IsMovementComplete( void ) const
{
	return (m_movementGoal == MOVE_GOAL_NONE) ? true : false;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::UpdateBestSound( void )
{
	// Determine which sound is the best sound
	ai_sound_t* pbestsound = nullptr;
	Float bestsounddist = 0;

	// Get eye/head position
	Vector headposition = GetEyePosition();

	m_soundsList.begin();
	while(!m_soundsList.end())
	{
		ai_sound_t& snd = m_soundsList.get();
		Float distance = (headposition-snd.position).Length();
		if(!pbestsound || distance < bestsounddist)
		{
			pbestsound = &snd;
			bestsounddist = distance;
		}

		m_soundsList.next();
	}

	m_pBestSound = pbestsound;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::HearSounds( void )
{
	// Clear sound condition
	ClearCondition(AI_COND_HEAR_SOUND);
	// Clear sound list
	m_soundTypes = 0;

	Uint64 soundmask = GetSoundMask();
	if(!soundmask)
		return;

	// Update existing sounds list
	if(!m_soundsList.empty())
	{
		m_soundsList.begin();
		while(m_soundsList.end())
		{
			const ai_sound_t& snd = m_soundsList.get();
			if(snd.life < g_pGameVars->time)
			{
				m_soundsList.remove(m_soundsList.get_link());
				m_soundsList.next();
				continue;
			}

			m_soundsList.next();
		}
	}

	if(m_pSchedule)
	{
		Uint64 scheduleMask = m_pSchedule->GetSoundMask();
		if(m_npcState == NPC_STATE_SCRIPT)
		{
			if(m_pScriptedSequence && m_pScriptedSequence->CanInterrupt() && m_pScriptedSequence->HasSpawnFlag(CScriptedSequence::FL_SOUNDS_CAN_INTERRUPT))
				scheduleMask |= m_pScriptedSequence->GetInterruptSoundMask(); // Use dedicated sound mask
			else
				scheduleMask |= (AI_SOUND_PLAYER|AI_SOUND_COMBAT); // Always add these so AI triggers work
		}

		soundmask = soundmask & scheduleMask;
	}

	Int32 extendedMask = soundmask;
	if(m_npcState == NPC_STATE_COMBAT)
		extendedMask |= AI_SOUND_PLAYER;

	// Get list of sounds
	CLinkedList<ai_sound_t> soundsList;
	Vector headPosition = GetEyePosition();

	gAISounds.GetSoundList(headPosition, soundsList, extendedMask, GetHearingSensitivity());

	if(!soundsList.empty())
	{
		soundsList.begin();
		while(!soundsList.end())
		{
			ai_sound_t& curSound = soundsList.get();

			// Do not listen to sounds from self
			if(curSound.emitter == const_cast<const CBaseNPC*>(this))
			{
				soundsList.next();
				continue;
			}

			ai_sound_t* pNPCSound = nullptr;

			if(!m_soundsList.empty())
			{
				m_soundsList.begin();
				while(!m_soundsList.end())
				{
					ai_sound_t& soundOnNPC = m_soundsList.get();
					if(soundOnNPC.identifier == curSound.identifier)
					{
						pNPCSound = &soundOnNPC;
						break;
					}

					m_soundsList.next();
				}
			}

			if(!pNPCSound)
				pNPCSound = &m_soundsList.add(ai_sound_t())->_val;

			// Fill in values from source
			(*pNPCSound) = curSound;

			// Go to next one
			soundsList.next();
		}
	}

	// Update existing sounds
	if(m_soundsList.empty())
		return;

	m_soundsList.begin();
	while(!m_soundsList.end())
	{
		ai_sound_t& sound = m_soundsList.get();
		if(sound.life < g_pGameVars->time)
		{
			m_soundsList.remove(m_soundsList.get_link());
			continue;
		}

		// Process this sound
		ProcessHeardSound(sound, soundmask);
		m_soundsList.next();
	}

	// Update best sound
	UpdateBestSound();
}

//=============================================
// @brief Processes a sound heard
//
//=============================================
bool CBaseNPC::ProcessHeardSound( ai_sound_t& sound, Uint64 soundMask )
{
	// If enemy, and not seen, then update position
	if(sound.emitter == m_enemy 
		&& !(sound.emitter->GetFlags() & FL_NOTARGET) 
		&& !CheckCondition(AI_COND_SEE_ENEMY))
	{
		const Vector& soundPosition = sound.emitter->GetNavigablePosition();
		if((m_enemyLastKnownPosition - soundPosition).Length2D() > NPC_ENEMY_UPDATE_DISTANCE)
			SetCondition(AI_COND_HEARD_ENEMY_NEW_POSITION);

		m_enemyLastKnownPosition = soundPosition;
		m_enemyLastKnownAngles = sound.emitter->GetAngles();
	}

	// Don't add sounds not under our schedule mask
	if(m_pSchedule && !(soundMask & sound.typeflags))
		return false;

	// Don't listen to sounds from notarget players
	if(sound.emitter && sound.emitter->GetFlags() & FL_NOTARGET)
		return false;

	// Mark that we've heard a sound
	SetCondition(AI_COND_HEAR_SOUND);
	// Set typeflags
	m_soundTypes |= sound.typeflags;

	// Update lean awareness if it's a combat sound
	if (sound.typeflags & (AI_SOUND_COMBAT | AI_SOUND_PLAYER) && sound.emitter && sound.emitter->IsPlayer())
	{
		enemyawareness_t* pawareness = GetEnemyPartialAwarenessInfo(sound.emitter);
		assert(pawareness != nullptr);

		pawareness->awareness += NPC_HEAR_LEAN_AWARENESS_GAIN * m_thinkIntervalTime;
		pawareness->lastsighttime = g_pGameVars->time;
		if (pawareness->awareness > 1.0)
			pawareness->awareness = 1.0;
	}
	return true;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::PerformMovement( Double animInterval )
{
	// Don't move if we don't have a valid route
	if(IsRouteClear())
	{
		// If we still have a goal, then refresh it
		if(m_movementGoal == MOVE_GOAL_NONE || !RefreshRoute())
		{
			Util::EntityConDPrintf(m_pEdict, "NPC tried to move with no route set.\n");
			SetTaskFailed();
			return;
		}
	}

	// If we're waiting, then don't run movement
	if(m_moveWaitFinishTime > g_pGameVars->time)
		return;

	// Get current point and get direction
	route_point_t& currentPoint = m_routePointsArray[m_routePointIndex];
	Vector moveDirection = (currentPoint.position - m_pState->origin);
	Float distanceToPoint = moveDirection.Length();
	moveDirection.Normalize();

	// Manage yaw related stuff
	SetIdealYaw(currentPoint.position);

	// Use distance to waypoint as max dist, because otherwise
	// we night not detect anything blocking us in time
	Float moveDistance = 0;

	// Check our movement
	localmove_t moveResult = CheckLocalMove(m_pState->origin, currentPoint.position, m_movementGoalEntity, &moveDistance);

	// If we reached the target, then stop
	if(moveResult == LOCAL_MOVE_REACHED_TARGET 
		&& moveDistance < NPC_STEP_SIZE)
	{
		StopMovement();
		ClearRoute();
		return;
	}

	// Check if we can continue
	Vector vectorToTarget = moveDirection * distanceToPoint;
	if(!CheckMoveResult(moveResult, moveDistance, vectorToTarget))
		return;

	// Try and advance the route index
	if(!CheckAdvanceRoute(distanceToPoint, m_movementGoalEntity))
	{
		StopMovement();
		SetTaskFailed();
		return;
	}

	// If waiting for a door and such, then stop
	if(m_moveWaitFinishTime > g_pGameVars->time)
	{
		StopMovement();
		return;
	}

	// Recalculate distanceToPoint and moveDirection, as it might've changed since the last check
	currentPoint = m_routePointsArray[m_routePointIndex];
	moveDirection = (currentPoint.position - m_pState->origin);
	distanceToPoint = moveDirection.Length2D();
	moveDirection.Normalize();

	// We need to set this again
	SetIdealYaw(currentPoint.position);

	// Actually move the NPC
	ExecuteMovement(m_movementGoalEntity, moveDirection, animInterval, distanceToPoint);

	// If movement is completed, then stop and clear route
	if(IsMovementComplete())
	{
		StopMovement();
		ClearRoute();
	}
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::CheckAdvanceRoute( Float distanceToPoint, CBaseEntity* pTargetEntity )
{
	trace_t tr;
	while(true)
	{
		// Make sure the height is not too big
		const route_point_t& point = m_routePointsArray[m_routePointIndex];
		if(point.type == MF_NONE)
			break;

		Float pointDistance2D = (point.position - m_pState->origin).Length2D();
		Float heightDifference = SDL_fabs(point.position.z - m_pState->origin.z);
		if(heightDifference > NPC_TRIANGULATION_MAX_HEIGHT)
		{
			if(pointDistance2D > NPC_CORNER_CUT_MIN_DIST)
				break;// Unreachable from here

			Util::TraceLine(m_pState->origin, point.position, false, false, m_pEdict, tr);
			if(!tr.noHit() && (!m_movementGoalEntity || tr.hitentity != m_movementGoalEntity->GetEntityIndex()))
			{
				// Didn't reach the target
				return false;
			}
		}
		else
		{
			// Check if we've reached this waypoint
			if(pointDistance2D > NPC_CORNER_CUT_MIN_DIST)
				break;
		}

		// Try and advance the route index
		advance_result_t result = AdvanceRoute(pointDistance2D);
		switch(result)
		{
		case ADVANCE_RESULT_FAILED:
			{
				// Abort movement
				return false;
			}
			break;
		case ADVANCE_RESULT_REACHED_GOAL:
			{
				MovementComplete();
				return true;
			}
			break;
		case ADVANCE_RESULT_SUCCESS:
			// Do nothing, keep trying to advance
			break;
		}

		// Return true if we have to wait
		if(m_moveWaitFinishTime > g_pGameVars->time)
			return true;
	}

	// See if we can cut corners from here, but only if we're not heading right at the target
	if((m_routePointIndex+1) != MAX_ROUTE_POINTS 
		&& m_routePointsArray[m_routePointIndex].type != MF_NONE
		&& !(m_routePointsArray[m_routePointIndex].type & MF_IS_GOAL))
	{
		// Keep track of traceline counts to avoid slowing down the game
		Uint32 traceCounter = 0;
		Int32 lastValidPathIndex = NO_POSITION;

		for(Int32 i = m_routePointIndex + 1; i < MAX_ROUTE_POINTS; i++)
		{
			route_point_t& nextPoint = m_routePointsArray[i];

			// Don't simplify the end
			if(nextPoint.type == MF_NONE)
				break;
			
			// Make sure we don't simplify something we shouldn't
			if(!ShouldSimplifyRoute(m_pState->origin, nextPoint.position, nextPoint.type))
				continue;

			// Only within a maximum distance
			if((m_pState->origin-nextPoint.position).Length() > NPC_MAX_SIMPLIFY_DISTANCE)
				continue;

			Vector testStart = m_pState->origin + Vector(0, 0, NPC_STEP_SIZE);
			Vector testEnd = nextPoint.position + Vector(0, 0, NPC_STEP_SIZE);
			
			Util::TraceHull(testStart, testEnd, true, false, false, HULL_POINT, m_pEdict, tr);
			traceCounter++;

			if(tr.noHit() || pTargetEntity && tr.hitentity != NO_ENTITY_INDEX && pTargetEntity->GetEntityIndex() == tr.hitentity)
			{
				localmove_t moveResult = CheckLocalMove(m_pState->origin, nextPoint.position, pTargetEntity);
				if(moveResult > LOCAL_MOVE_RESULT_FAILURE)
					lastValidPathIndex = i;
			}

			if(traceCounter == NPC_MAX_SIMPLIFY_TRACES)
				break;
		}

		if(lastValidPathIndex != NO_POSITION &&  lastValidPathIndex != m_shortcutPathIndex)
		{
			m_lastCornerCutOrigin = m_pState->origin;
			m_shortcutPathIndex = lastValidPathIndex;
			m_canCutCorners = true;
		}
		else if(m_canCutCorners && lastValidPathIndex == NO_POSITION)
		{
			m_lastCornerCutOrigin.Clear();
			m_shortcutPathIndex = NO_POSITION;
			m_canCutCorners = false;
		}

		if(m_canCutCorners && m_shortcutPathIndex != NO_POSITION)
		{
			Float distance = (m_lastCornerCutOrigin-m_pState->origin).Length2D();
			if(distance >= NPC_SIMPLIFICATION_FIX_DISTANCE)
			{
				// Advance till
				while(m_routePointIndex < m_shortcutPathIndex)
				{
					advance_result_t result = AdvanceRoute(distance);
					if(result == ADVANCE_RESULT_FAILED
						|| result == ADVANCE_RESULT_REACHED_GOAL)
						break;
				}

				m_canCutCorners = false;
				m_shortcutPathIndex = NO_POSITION;
			}
		}
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::ExecuteMovement( CBaseEntity* pTargetEntity, const Vector& direction, Double animInterval, Float checkDistance )
{
	if(GetIdealActivity() != m_movementActivity)
		SetIdealActivity(m_movementActivity);

	Float totalDistance = m_groundSpeed * animInterval;
	totalDistance = clamp(totalDistance, 0, checkDistance);
	totalDistance *= m_pState->framerate;

	// Get reference to current point
	Vector endPosition = m_pState->origin + direction * totalDistance;
	// According to ReHLDS, the old MoveToOrigin relied on idealyaw
	// to calculate movement vectors... why? This caused issues when
	// AdvanceRoute changed the destination node in another direction
	// but calculate it here just in case
	Float moveYaw = Util::VectorToYaw(direction);

	// Do the movement
	while(totalDistance > 0.001)
	{
		// Clamp step distance to the max step size
		Float stepDistance = clamp(totalDistance, 0, NPC_STEP_SIZE);
		gd_engfuncs.pfnMoveToOrigin(m_pEdict, endPosition, moveYaw, stepDistance, MOVE_NORMAL);
		totalDistance -= stepDistance;
	}
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::InitNPC( void )
{
	// Set basic properties
	m_pState->takedamage = TAKEDAMAGE_YES;
	m_pState->idealyaw = m_pState->angles[YAW];
	m_pState->maxhealth = m_pState->health;
	m_pState->deadstate = DEADSTATE_NONE;
	m_pState->flags |= FL_NPC;

	if(HasSpawnFlag(FL_NPC_USE_NPC_CLIP))
		m_pState->flags |= FL_NPC_CLIP;

	// Set basic AI states
	m_idealNPCState = NPC_STATE_IDLE;
	m_deathMode = DEATH_NORMAL;
	m_hintNodeIndex = NO_POSITION;

	SetIdealActivity(ACT_IDLE);
	SetEyePosition();

	InitBoneControllers();

	// TODO: Think up a better way to spread think times out!
	SetThink(&CBaseNPC::NPCInitThink);
	m_pState->nextthink = g_pGameVars->time + Common::RandomFloat(0.1, 1.0);

	SetUse(&CBaseNPC::NPCUse);
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::InitDeadNPC( void )
{
	InitBoneControllers();

	m_pState->frame = 0;
	ResetSequenceInfo();
	m_pState->framerate = 0;
	m_pState->forcehull = HULL_POINT;

	m_pState->maxhealth = m_pState->health;
	m_pState->deadstate = DEADSTATE_DEAD;
	m_pState->takedamage = TAKEDAMAGE_YES;

	// Make the NPC truly dead
	BecomeDead(true);

	// Make sure the bbox is of valid size
	SetSequenceBox(false);
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::BecomeDead( bool startedDead )
{
	// Give this corpse half it's original max health
	m_pState->health = m_pState->maxhealth * 0.5f;
	m_pState->maxhealth = 5;

	m_pState->movetype = MOVETYPE_TOSS;
	m_pState->solid = SOLID_SLIDEBOX;
	m_pState->flags |= FL_DEAD;
	m_pState->forcehull = HULL_POINT;

	// Reset these so the corpse drops
	m_pState->flags &= ~FL_ONGROUND;
	m_pState->groundent = NO_ENTITY_INDEX;

	if(!startedDead)
	{
		// Nudge any groundents on us
		Util::FixGroundEntities(this, true);
	}

	// So dead expression is set
	UpdateExpressions();

	// Bullets, melee, crush and slash create decals
	if(m_damageBits & (DMG_BULLET|DMG_MELEE|DMG_AXE|DMG_SLASH|DMG_CRUSH))
	{
		Vector traceStart = m_pState->origin + Vector(0, 0, 8);
		Vector traceEnd = m_pState->origin - Vector(0, 0, 32);

		trace_t tr;
		Util::TraceLine(traceStart, traceEnd, true, false, m_pEdict, tr);
		if(!tr.noHit())
		{
			CString decalName;
			switch(Common::RandomLong(0, 1))
			{
			case 0: 
				decalName = "bloodbigsplat";
				break;
			case 1: 
				decalName = "bloodbigsplat2";
				break;
			}

			// Create on client
			Int32 decalFlags = (FL_DECAL_SPECIFIC_TEXTURE);
			Util::CreateGenericDecal(tr.endpos, &tr.plane.normal, decalName.c_str(), decalFlags, NO_ENTITY_INDEX, 0, 0, 20);

			// Add to save-restore
			gd_engfuncs.pfnAddSavedDecal(tr.endpos, tr.plane.normal, tr.hitentity, decalName.c_str(), decalFlags);
		}
	}
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::ShouldGibNPC( gibbing_t gibFlag ) const
{
	if(gibFlag == GIB_NORMAL && m_pState->health < NPC_GIB_HEALTH_VALUE || gibFlag == GIB_ALWAYS)
		return true;
	else
		return false;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::CallGibNPC( void )
{
	m_pState->takedamage = TAKEDAMAGE_NO;
	m_pState->solid = SOLID_NOT;
	m_pState->effects |= EF_NODRAW;
	m_pState->deadstate = DEADSTATE_DEAD;

	// Check AI triggers
	CheckAITriggers();
	// Gib the npc
	GibNPC();
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::Killed( CBaseEntity* pAttacker, gibbing_t gibbing, deathmode_t deathMode )
{
	if(HasMemory(AI_MEMORY_KILLED))
	{
		// Gib if we can
		if(ShouldGibNPC(gibbing))
			CallGibNPC();

		return;
	}

	// Signal our attacker that he killed us
	if(pAttacker)
		pAttacker->OnNPCKilled(this);

	// Remember that we got killed
	SetMemory(AI_MEMORY_KILLED);
	m_deathMode = deathMode;

	// Shut up the weapon channel
	Util::EmitEntitySound(this, NULL_SOUND_FILENAME, SND_CHAN_WEAPON);
	m_idealNPCState = NPC_STATE_DEAD;

	// This is for AI triggers
	SetCondition(AI_COND_LIGHT_DAMAGE);

	// Tell our owner that we died
	if(m_pState->owner != NO_ENTITY_INDEX)
	{
		CBaseEntity* pOwner = GetOwner();
		if(pOwner)
			pOwner->ChildDeathNotice(this);
	}

	// Remember this
	m_deathDamageBits = m_damageBits;

	if(ShouldGibNPC(gibbing))
	{
		CallGibNPC();
		return;
	}
	else if(m_pState->flags & FL_NPC)
	{
		SetTouch(nullptr);
		BecomeDead(false);
	}

	// Remember enemy
	m_enemy = pAttacker;
	m_killer = pAttacker;

	if(m_enemy && m_enemy->IsPlayer())
		m_enemy->SetNPCAwareness(0, this, 0, false);

	// Explode in a few seconds
	if(deathMode == DEATH_NORMAL
		&& (m_damageBits & DMG_UNUSED2) 
		&& (m_damageBits & DMG_UNUSED3))
	{
		m_deathMode = DEATH_EXPLODE;
		m_deathExplodeTime = g_pGameVars->time + Common::RandomFloat(1, 3);
	}
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::TakeHealth( Float amount, Int32 damageFlags )
{
	if(HasSpawnFlag(FL_NPC_IMMORTAL) || m_pState->takedamage != TAKEDAMAGE_YES)
		return false;

	// Clear out healed damage bit
	m_damageBits &= ~(damageFlags & ~DMG_TIMEBASED);

	return CBaseEntity::TakeHealth(amount, damageFlags);
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::TakeDamage( CBaseEntity* pInflictor, CBaseEntity* pAttacker, Float amount, Int32 damageFlags )
{
	if(!pInflictor || !pAttacker)
		return false;

	// Don't take damage if you're an immortal
	if(HasSpawnFlag(FL_NPC_IMMORTAL))
		return false;

	// If takedamage is not set, then don't take any damage
	if(m_pState->takedamage != TAKEDAMAGE_YES)
		return false;

	// If we took damage, forget that we were in cover
	ClearMemory(AI_MEMORY_IN_COVER);

	// Play damage sound if hit by a bullet, and damage is substantial
	if(amount >= NPC_LIGHT_DAMAGE_TRESHOLD && (damageFlags & DMG_BULLET))
	{
		// Play sound on player if the player hit us
		CBaseEntity* pSoundEntity = pAttacker->IsPlayer() ? pAttacker : this;

		CString soundName;
		soundName << "impact/bullet_hit_flesh" << (Int32)Common::RandomLong(1, 2) << ".wav";

		Util::EmitEntitySound(pSoundEntity, soundName.c_str(), SND_CHAN_BODY);
	}

	// If not alive, run the function for corpses
	if(!IsAlive())
		return TakeDamageDead(pInflictor, pAttacker, amount, damageFlags);

	// Get distance to enemy
	Float shooterDistance = (pInflictor->GetCenter() - GetCenter()).Length();

	// If insta-decapitated, then change amount to our full health
	Float _dmgAmount = amount;
	Int32 hitgroup = gMultiDamage.GetHitGroupForEntity(this);
	if(shooterDistance < NPC_DECAP_MAX_DISTANCE && hitgroup == HITGROUP_HEAD 
		&& (damageFlags & DMG_INSTANTDECAP) && CanBeInstantlyDecapitated())
		_dmgAmount = m_pState->health;

	m_damageBits |= damageFlags;

	if(damageFlags & DMG_UNUSED2)
		AddClearDamage(DMG_UNUSED2, 1.0);

	if(damageFlags & DMG_UNUSED3)
		AddClearDamage(DMG_UNUSED3, 1.0);

	if(damageFlags & DMG_EXPLOSION)
		AddClearDamage(DMG_EXPLOSION, 1.0);

	// Grab the direction from the inflictor
	Vector dmgDirection;
	if(pInflictor)
	{
		// Change attack dir to use centers
		dmgDirection = (pInflictor->GetCenter() - Vector(0, 0, 8) - GetCenter()).Normalize();
		// Alter in multidamage also
		gMultiDamage.SetAttackDirection(dmgDirection); 
	}

	// Deal the damage to the NPC
	Float prevHealth = m_pState->health;
	m_pState->health -= _dmgAmount;

	// Don't let scripted NPCs die if specific flags are set
	if(m_npcState == NPC_STATE_SCRIPT)
	{
		if(m_pScriptedSequence && m_pScriptedSequence->HasSpawnFlag(CScriptedSequence::FL_OVERRIDE_STATE|CScriptedSequence::FL_NO_INTERRUPTIONS))
		{
			SetCondition(AI_COND_LIGHT_DAMAGE);
			return false;
		}
	}

	// If we're dead, then manage death
	if(m_pState->health <= 0)
	{
		// True if the NPC was decapitated
		bool wasDecapitated = false;

		// If hit by a gibbing or insta-decap bullet in the head/helmet, then decapitate the NPC
		if((!(damageFlags & DMG_BUCKSHOT) || shooterDistance < NPC_DECAP_MAX_DISTANCE) 
			&& (hitgroup == HITGROUP_HEAD || hitgroup == HITGROUP_HELMET) 
			&& (damageFlags & (DMG_BULLETGIB|DMG_INSTANTDECAP|DMG_AXE)))
		{
			Decapitate((damageFlags & DMG_AXE) ? true : false);
			wasDecapitated = true;
		}
		
		// Gibbing and death mode depend on damage types
		gibbing_t gibFlag = GIB_NORMAL;
		deathmode_t deathMode = DEATH_NORMAL;

		if(shooterDistance < NPC_BULLETGIB_MAX_DISTANCE 
			&& ShouldDamageGibNPC(_dmgAmount, prevHealth, damageFlags, wasDecapitated))
			gibFlag = GIB_ALWAYS;
		else if((damageFlags & DMG_NEVERGIB) || wasDecapitated)
			gibFlag = GIB_NEVER;
		else
			gibFlag = GIB_NORMAL;

		// If we got decapitated, then set the death mode
		if(wasDecapitated && gibFlag != GIB_ALWAYS)
			deathMode = DEATH_DECAPITATED;

		// Kill the NPC
		Killed(pAttacker, gibFlag, deathMode);
		return false;
	}
	else if(m_pState->deadstate == DEADSTATE_NONE)
	{
		// Play a pain sound
		EmitPainSound();
	}

	// React to the damage
	if(pAttacker && (pAttacker->IsNPC() || pAttacker->IsPlayer()))
	{
		if(GetRelationship(pAttacker) > R_NONE)
		{
			if(pInflictor)
			{
				if(!m_enemy || pInflictor == m_enemy || !CheckCondition(AI_COND_SEE_ENEMY))
					m_enemyLastKnownPosition = pInflictor->GetOrigin();
			}
			else
			{
				// Just set an arbitrary position
				m_enemyLastKnownPosition = m_pState->origin + gMultiDamage.GetAttackDirection() * 64;
			}

			// Push this enemy instantly
			if(!(pAttacker->GetFlags() & FL_NOTARGET))
				PushEnemy(pAttacker, m_enemyLastKnownPosition, pInflictor->GetAngles(), g_pGameVars->time);

			SetIdealYaw(m_enemyLastKnownPosition);
		}

		// Set damage conditions
		if(_dmgAmount >= NPC_LIGHT_DAMAGE_TRESHOLD)
			SetCondition(AI_COND_LIGHT_DAMAGE);

		if(_dmgAmount >= NPC_HEAVY_DAMAGE_TRESHOLD)
			SetCondition(AI_COND_HEAVY_DAMAGE);
	}

	return true;
}

//=============================================
// @brief Tells if the NPC should be gibbed
//
//=============================================
bool CBaseNPC::ShouldDamageGibNPC( Float damageAmount, Float prevHealth, Int32 dmgFlags, bool wasDecapitated )
{
	// Always gib enemies in DMG_BULLETGIB type if we cross the treshold for gibbing damage,
	// and we're at a minimum health, or at a one out of three random chance. Also never gib 
	// if we already got decapitated
	if(((dmgFlags & DMG_BULLETGIB) && damageAmount >= NPC_BULLETGIB_DMG_TRESHOLD 
		&& (prevHealth <= NPC_BULLETGIB_MIN_HEALTH || Common::RandomLong(0, 2) == 1) 
		&& !wasDecapitated || (dmgFlags & DMG_ALWAYSGIB)) && !(dmgFlags & DMG_NEVERGIB))
		return true;
	else
		return false;
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::TakeDamageDead( CBaseEntity* pInflictor, CBaseEntity* pAttacker, Float amount, Int32 damageFlags )
{
	// Grab the direction from the inflictor
	Vector dmgDirection;
	if(pInflictor)
	{
		// Change attack dir to use centers
		dmgDirection = (pInflictor->GetCenter() - Vector(0, 0, 8) - GetCenter()).Normalize();
		// Alter in multidamage also
		gMultiDamage.SetAttackDirection(dmgDirection); 
	}

	// Destroy the corpse if enough damage was dealt
	if(damageFlags & DMG_GIB_CORPSE)
	{
		if(m_pState->health <= amount)
		{
			m_pState->health = -50;
			Killed(pAttacker, GIB_ALWAYS, DEATH_NORMAL);
			return false;
		}

		m_pState->health -= amount * 0.1f;
	}

	return true;
}

//=============================================
// @brief Checks clear damage list for any dmg bit that needs to be cleared
//
//=============================================
void CBaseNPC::ProcessClearDamageList( void )
{
	if(m_damageClearList.empty())
		return;

	m_damageClearList.begin();
	while(!m_damageClearList.end())
	{
		const cleardamage_t& dmg = m_damageClearList.get();
		if(dmg.time <= g_pGameVars->time)
		{
			m_damageBits &= ~dmg.dmgbit;
			m_damageClearList.remove(m_damageClearList.get_link());
			m_damageClearList.next();
			continue;
		}

		m_damageClearList.next();
	}
}

//=============================================
// @brief Adds a new damage bit that needs delayed clearing
//
//=============================================
void CBaseNPC::AddClearDamage( Int32 dmgbit, Float delay )
{
	while(!m_damageClearList.end())
	{
		cleardamage_t& dmg = m_damageClearList.get();
		if(dmg.dmgbit & dmgbit)
		{
			dmg.time = g_pGameVars->time + delay;
			return;
		}

		m_damageClearList.next();
	}

	cleardamage_t dmg;
	dmg.dmgbit = dmgbit;
	dmg.time = g_pGameVars->time + delay;
	m_damageClearList.add(dmg);
}

//=============================================
// @brief
//
//=============================================
Float CBaseNPC::GetHitgroupDmgMultiplier( Int32 hitgroup )
{
	Float dmgMultiplier = 1.0;
	switch(hitgroup)
	{
	case HITGROUP_HEAD:
		{
			dmgMultiplier = GetSkillCVarValue(g_skillcvars.skillNPCDmgMultiplierHead);
		}
		break;
	case HITGROUP_CHEST:
		{
			dmgMultiplier = GetSkillCVarValue(g_skillcvars.skillNPCDmgMultiplierChest);
		}
		break;
	case HITGROUP_STOMACH:
		{
			dmgMultiplier = GetSkillCVarValue(g_skillcvars.skillNPCDmgMultiplierStomach);
		}
		break;
	case HITGROUP_LEFT_ARM:
	case HITGROUP_RIGHT_ARM:
		{
			dmgMultiplier = GetSkillCVarValue(g_skillcvars.skillNPCDmgMultiplierArm);
		}
		break;
	case HITGROUP_LEFT_LEG:
	case HITGROUP_RIGHT_LEG:
		{
			dmgMultiplier = GetSkillCVarValue(g_skillcvars.skillNPCDmgMultiplierLeg);
		}
		break;
	case HITGROUP_GENERIC:
	default:
		break;
	}

	return dmgMultiplier;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::TraceAttack( CBaseEntity* pAttacker, Float damage, const Vector& direction, trace_t& tr, Int32 damageFlags )
{
	// Do not take damage from allies
	if(pAttacker && (GetRelationship(pAttacker) == R_ALLY || GetRelationship(pAttacker) == R_NONE && pAttacker != this && pAttacker->IsNPC()))
		return;

	Float _dmgAmount = damage;

	if(m_pState->takedamage == TAKEDAMAGE_YES)
	{
		// Pop lean awareness to full if we got damaged
		if (pAttacker && pAttacker->IsPlayer())
		{
			enemyawareness_t* pawareness = GetEnemyPartialAwarenessInfo(pAttacker);
			assert(pawareness != nullptr);

			pawareness->lastsighttime = g_pGameVars->time;
			if (pawareness->awareness < 1.0)
				pawareness->awareness = 1.0;
		}

		m_lastHitGroup = tr.hitgroup;
		if(_dmgAmount >= 1.0)
			SpawnBloodDecals(damage, direction, tr, damageFlags);

		if(tr.hitgroup == HITGROUP_HEAD)
		{
			Vector traceEndPosition = tr.endpos + direction * 256;

			trace_t bloodtr;
			Util::TraceLine(tr.endpos, traceEndPosition, true, false, m_pEdict, bloodtr);
			if(!bloodtr.noHit())
			{
				Util::CreateGenericDecal(bloodtr.endpos, &bloodtr.plane.normal, "brains", FL_DECAL_VBM);
				gd_engfuncs.pfnAddSavedDecal(bloodtr.endpos, bloodtr.plane.normal, bloodtr.hitentity, "brains", FL_DECAL_VBM);
			}
		}

		// Calculate final damage
		_dmgAmount *= GetHitgroupDmgMultiplier(tr.hitgroup);

		if(!(damageFlags & DMG_MELEE) && _dmgAmount >= 1.0)
			Util::SpawnBloodParticles(tr, GetBloodColor(), false);

		gMultiDamage.AddDamage(this, _dmgAmount, damageFlags);
	}

	// Spawn blood decal on NPC if you can
	if(GetBloodColor() != BLOOD_NONE && _dmgAmount >= 1)
		Util::CreateVBMDecal(tr.endpos, tr.plane.normal, "shot_human", m_pEdict, FL_DECAL_NORMAL_PERMISSIVE);
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::DecapitateNPC( bool spawnGib, Int32 bodyGroup, Int32 bodyNumber )
{
	// Play the splatter sound
	CString soundFile;
	soundFile << "impact/gib_0" << (Int32)Common::RandomLong(1, 2) << ".wav";
	Util::EmitAmbientSound(GetCenter(), soundFile.c_str());

	// Spawn head gib if specified
	if(spawnGib)
		CGib::SpawnHeadGib(this, nullptr, -200, 200);

	// Set the decapitated bodygroup
	SetBodyGroup(bodyGroup, bodyNumber);

	// Shut the voice channel up
	Util::EmitEntitySound(this, NULL_SOUND_FILENAME, SND_CHAN_VOICE);

	// Create a fountain of blood
	Vector boneOrigin;
	if(!GetBonePosition(HEAD_BONE_NAME, boneOrigin))
	{
		Util::EntityConDPrintf(m_pEdict, "%s - No bone named '%s'.\n", __FUNCTION__, HEAD_BONE_NAME);
		boneOrigin = GetEyePosition();
	}

	Util::CreateParticles("blood_effects_decap.txt", boneOrigin, Vector(0, 0, 1), PART_SCRIPT_CLUSTER);
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::ResetNPC( void )
{
	// Reset flex states
	m_activeFlexState = FLEX_AISTATE_NONE;
	m_flexScriptDuration = g_pGameVars->time + 2;

	for(Uint32 i = 0; i < MAX_CONTROLLERS; i++)
		SetBoneController(i, 0);
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::NPCInitThink( void )
{
	StartNPC();
	InitSquad();
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::StartNPC( void )
{
	// See if range attacks exists as animations
	if(CAnimatingEntity::FindActivity(ACT_RANGE_ATTACK1) != NO_SEQUENCE_VALUE)
		SetCapability(AI_CAP_RANGE_ATTACK1);

	if(CAnimatingEntity::FindActivity(ACT_RANGE_ATTACK2) != NO_SEQUENCE_VALUE)
		SetCapability(AI_CAP_RANGE_ATTACK2);

	// See if range attacks exists as animations
	if(CAnimatingEntity::FindActivity(ACT_MELEE_ATTACK1) != NO_SEQUENCE_VALUE)
		SetCapability(AI_CAP_MELEE_ATTACK1);

	if(CAnimatingEntity::FindActivity(ACT_MELEE_ATTACK2) != NO_SEQUENCE_VALUE)
		SetCapability(AI_CAP_MELEE_ATTACK2);

	// See if range attacks exists as animations
	if(CAnimatingEntity::FindActivity(ACT_SPECIAL_ATTACK1) != NO_SEQUENCE_VALUE)
		SetCapability(AI_CAP_SPECIAL_ATTACK1);

	if(CAnimatingEntity::FindActivity(ACT_SPECIAL_ATTACK2) != NO_SEQUENCE_VALUE)
		SetCapability(AI_CAP_SPECIAL_ATTACK2);

	// Check for medkit pickups
	if(CAnimatingEntity::FindActivity(ACT_USE_MEDKIT) != NO_SEQUENCE_VALUE)
		SetCapability(AI_CAP_USE_MEDKITS);

	// Check for flex support
	if(GetModelFlags() & STUDIO_MF_HAS_FLEXES)
		SetCapability(AI_CAP_EXPRESSIONS);

	if(m_pState->movetype != MOVETYPE_FLY 
		&& m_pState->movetype != MOVETYPE_NONE 
		&& !HasSpawnFlag(FL_NPC_FALL_TO_GROUND))
	{
		if(!HasSpawnFlag(FL_NPC_DONT_FALL))
		{
			// Raise the NPC off the floor, then drop him
			m_pState->origin.z += 1.0f;
			gd_engfuncs.pfnDropToFloor(m_pEdict);

			if(!gd_engfuncs.pfnWalkMove(m_pEdict, 0, 0, WALKMOVE_NORMAL))
			{
				Util::EntityConPrintf(m_pEdict, "Entity stuck in brush geometry at %.2f %.2f %.2f.\n", m_pState->origin.x, m_pState->origin.y, m_pState->origin.z);

				// Set a glow aura with a red color
				m_pState->renderfx = RenderFx_GlowAura;
				m_pState->rendercolor = NPC_ERROR_GLOW_AURA_COLOR;
			}
		}
	}
	else if(m_pState->movetype != MOVETYPE_NONE)
	{
		// Remove this if we're a flying monster, or not on ground
		m_pState->flags &= ~FL_ONGROUND;
	}

	// See if the NPC has a target
	if(HasTarget())
	{
		const Char* pstrTarget = GetTarget();
		edict_t* pedict = Util::FindEntityByTargetName(nullptr, pstrTarget);
		if(pedict && !Util::IsNullEntity(pedict))
		{
			m_goalEntity = CBaseEntity::GetClass(pedict);

			SetIdealYaw(m_goalEntity->GetOrigin());

			if(m_pState->movetype == MOVETYPE_FLY)
				m_movementActivity = ACT_FLY;
			else
				m_movementActivity = ACT_WALK;

			if(!RefreshRoute())
				Util::EntityConPrintf(m_pEdict, "Couldn't refresh route to reach target '%s'.\n", pstrTarget);

			SetNPCState(NPC_STATE_IDLE);
			ChangeSchedule(GetScheduleByIndex(AI_SCHED_IDLE_WALK));
		}
		else
		{
			// Tell the level designer that the target was not valid
			Util::EntityConPrintf(m_pEdict, "Couldn't find target '%s'.\n", m_pEdict, pstrTarget);
		}
	}

	// Spread out think time
	SetThink(&CBaseNPC::CallNPCThink);
	m_pState->nextthink = g_pGameVars->time + Common::RandomFloat(0.1f, 0.4f);

	// Mark values as having been parsed
	m_valuesParsed = true;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::CallNPCThink( void )
{
	NPCThink();
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::CallNPCDeadThink( void )
{
	NPCDeadThink();
}

//=============================================
// @brief
//
//=============================================
CBitSet CBaseNPC::GetIgnoreConditions( void )
{
	CBitSet ignoreConditions(AI_COND_BITSET_SIZE);
	if(m_npcState == NPC_STATE_SCRIPT && m_pScriptedSequence)
		ignoreConditions |= m_pScriptedSequence->GetIgnoreConditions();

	if(HasMemory(AI_MEMORY_DODGE_ENEMY_FAILED))
		ignoreConditions.set(AI_COND_DANGEROUS_ENEMY_CLOSE);

	if(m_npcState == NPC_STATE_SCRIPT)
	{
		if(m_triggerCondition1 == AI_TRIGGER_TAKEDAMAGE || m_triggerCondition2 == AI_TRIGGER_TAKEDAMAGE)
		{
			ignoreConditions.reset(AI_COND_LIGHT_DAMAGE);
			ignoreConditions.reset(AI_COND_HEAVY_DAMAGE);
		}

		if(m_triggerCondition1 == AI_TRIGGER_HEAR_WORLD || m_triggerCondition2 == AI_TRIGGER_HEAR_WORLD
			|| m_triggerCondition1 == AI_TRIGGER_HEAR_PLAYER || m_triggerCondition2 == AI_TRIGGER_HEAR_PLAYER
			|| m_triggerCondition1 == AI_TRIGGER_HEAR_COMBAT || m_triggerCondition2 == AI_TRIGGER_HEAR_COMBAT)
			ignoreConditions.reset(AI_COND_HEAR_SOUND);
	}

	return ignoreConditions;
}

//=============================================
// @brief
//
//=============================================
CBitSet CBaseNPC::GetScheduleChangeKeptConditions( void )
{
	CBitSet result(AI_COND_BITSET_SIZE);
	return result; 
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::SetEyePosition( void )
{
	const cache_model_t* pmodel = gd_engfuncs.pfnGetModel(m_pState->modelindex);
	if(!pmodel)
	{
		Util::EntityConPrintf(m_pEdict, "No model available.\n");
		return;
	}

	if(pmodel->type != MOD_VBM)
	{
		Util::EntityConPrintf(m_pEdict, "Not a valid VBM model.\n");
		return;
	}

	const vbmcache_t* pvbmcache = pmodel->getVBMCache();
	if(!pvbmcache)
	{
		Util::EntityConPrintf(m_pEdict, "Not a valid VBM model.\n");
		return;
	}
	
	m_pState->view_offset = pvbmcache->pstudiohdr->eyeposition;
	if(m_pState->view_offset.IsZero())
		Util::EntityConPrintf(m_pEdict, "Model has no view offset.\n");
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::HandleAnimationEvent( const mstudioevent_t* pevent )
{
	switch(pevent->event)
	{
	case NPC_AE_DEAD:
		if(m_npcState == NPC_STATE_SCRIPT)
		{
			m_pState->deadstate = DEADSTATE_DYING;
			m_pState->health = 0;
		}
		break;
	case NPC_AE_NOT_DEAD:
		if(m_npcState == NPC_STATE_SCRIPT)
		{
			m_pState->deadstate = DEADSTATE_NONE;
			m_pState->health = m_pState->maxhealth;
		}
		break;
	case NPC_AE_SOUND:
		{
			Util::EmitEntitySound(this, pevent->options, SND_CHAN_BODY, VOL_NORM, ATTN_IDLE);
		}
		break;
	case NPC_AE_SOUND_VOICE:
		{
			Util::EmitEntitySound(this, pevent->options, SND_CHAN_VOICE, VOL_NORM, ATTN_IDLE, GetVoicePitch());
		}
		break;
	case NPC_AE_SENTENCE_RANDOM1:
		{
			if(!g_pSentencesFile)
				return;

			if(Common::RandomLong(0, 2) == 0)
			{
				Float duration = 0;
				const Char* pstrSentence = g_pSentencesFile->GetRandomSentence(pevent->options, &duration);
				if(pstrSentence)
				{
					Util::EmitEntitySound(this, pstrSentence, SND_CHAN_VOICE, VOL_NORM, ATTN_NORM, GetVoicePitch());
					m_talkTime = g_pGameVars->time + duration;
				}
			}
		}
		break;
	case NPC_AE_SENTENCE:
		{
			if(!g_pSentencesFile)
				return;

			Float duration = 0;
			const Char* pstrSentence = g_pSentencesFile->GetRandomSentence(pevent->options, &duration);
			if(pstrSentence)
			{
				Util::EmitEntitySound(this, pstrSentence, SND_CHAN_VOICE, VOL_NORM, ATTN_NORM, GetVoicePitch());
				m_talkTime = g_pGameVars->time + duration;
			}
		}
		break;
	case NPC_AE_FIRE_EVENT:
		{
			Util::FireTargets(pevent->options, this, this, USE_TOGGLE, 0);
		}
		break;
	case NPC_AE_NO_INTERRUPT:
		{
			if(m_pScriptedSequence)
				m_pScriptedSequence->SetAllowInterrupt(false);
		}
		break;
	case NPC_AE_CAN_INTERRUPT:
		{
			if(m_pScriptedSequence)
				m_pScriptedSequence->SetAllowInterrupt(true);
		}
		break;
	case NPC_AE_BODYDROP_HEAVY:
		{
			if(m_pState->flags & FL_ONGROUND)
			{
				switch(Common::RandomLong(0, 1))
				{
				case 0:
					Util::EmitEntitySound(this, "common/bodydrop_heavy1.wav", SND_CHAN_BODY);
					break;
				case 1:
					Util::EmitEntitySound(this, "common/bodydrop_heavy2.wav", SND_CHAN_BODY);
					break;
				}
			}
		}
		break;
	case NPC_AE_BODYDROP_LIGHT:
		{
			if(m_pState->flags & FL_ONGROUND)
			{
				switch(Common::RandomLong(0, 1))
				{
				case 0:
					Util::EmitEntitySound(this, "common/bodydrop_light1.wav", SND_CHAN_BODY);
					break;
				case 1:
					Util::EmitEntitySound(this, "common/bodydrop_light2.wav", SND_CHAN_BODY);
					break;
				}
			}
		}
		break;
	case NPC_AE_SWISH_SOUND:
		{
			Util::EmitEntitySound(this, "common/hit_miss.wav", SND_CHAN_BODY);
		}
		break;
	case NPC_AE_STEP:
		{
			FootStep();
		}
		break;
	case NPC_AE_SETBODYGROUP:
		{
			const Char* pstrSemicolon = qstrstr(pevent->options, ";");
			if(!pstrSemicolon)
			{
				Util::EntityConPrintf(m_pEdict, "Missing ';' in event '%d' options '%s'.\n", pevent->event, pevent->options);
				return;
			}

			// Extract bodygroup name
			Uint32 bodyGrpLength = pstrSemicolon - pevent->options;
			CString bodygroupname(pevent->options, bodyGrpLength);

			// Extract the submodel name
			Uint32 stringLength = qstrlen(pevent->options);
			Int32 submodelLength = stringLength - bodyGrpLength - 1;
			if(submodelLength <= 0)
			{
				Util::EntityConPrintf(m_pEdict, "Missing submodel part after ';' in event '%d' options '%s'.\n", pevent->event, pevent->options);
				return;
			}

			CString submodelname(pstrSemicolon+1, submodelLength);

			// Find bodygroup
			Int32 bodyGroupIndex = GetBodyGroupIndexByName(bodygroupname.c_str());
			if(bodyGroupIndex == NO_POSITION)
			{
				Util::EntityConPrintf(m_pEdict, "Bodygroup '%s' not found for event '%d'.\n", bodygroupname.c_str(), pevent->event);
				return;
			}

			// Find submodel
			Int32 submodelIndex = GetSubmodelIndexByName(bodyGroupIndex, submodelname.c_str());
			if(submodelIndex == NO_POSITION)
			{
				Util::EntityConPrintf(m_pEdict, "Submodel '%s' not found for bodygroup '%s', event '%d'.\n", submodelname.c_str(), bodygroupname.c_str(), pevent->event);
				return;
			}

			SetBodyGroup(bodyGroupIndex, submodelIndex);
		}
		break;
	default:
		{
			Util::EntityConDPrintf(m_pEdict, "Unhandled animation event with id '%d'.\n", pevent->event);
		}
		break;
	}
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::FootStep( void )
{
	Vector startPosition = m_pState->origin + Vector(0, 0, 8);
	Vector endPosition = m_pState->origin - Vector(0, 0, 16);

	trace_t tr;
	Util::TraceLine(startPosition, endPosition, true, false, m_pEdict, tr);
	if(tr.noHit() || tr.allSolid() || tr.startSolid())
		return;

	const Char* pstrTextureName = gd_tracefuncs.pfnTraceTexture(tr.hitentity, tr.endpos+tr.plane.normal, tr.endpos-tr.plane.normal);
	if(!pstrTextureName || !qstrcmp(pstrTextureName, "black"))
		return;

	const en_material_t* pMaterial = gd_engfuncs.pfnGetMapTextureMaterialScript(pstrTextureName);
	if(!pMaterial)
		return;

	Float volume;
	if(m_currentActivity == ACT_WALK)
		volume = 0.5;
	else
		volume = VOL_NORM;

	PlayFootStepSound(pMaterial->materialname.c_str(), volume);
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::CleanupScriptedSequence( void )
{
	// Remember this for later
	CScriptedSequence* pScript = m_pScriptedSequence;
	if(!pScript)
	{
		Util::EntityConPrintf(m_pEdict, "Scripted sequence pointer is null!\n");
		return;
	}

	// Clear pointers
	m_pScriptedSequence = nullptr;
	m_targetEntity.reset();
	m_goalEntity.reset();
	
	// Restore movetype
	m_pState->movetype = MOVETYPE_STEP;

	// On script as well
	pScript->ClearTargetEntity();

	if(m_pState->deadstate == DEADSTATE_DYING)
	{
		m_pState->health = 0;
		m_pState->framerate = 0;
		
		SetNPCState(NPC_STATE_DEAD);
		m_pState->deadstate = DEADSTATE_DEAD;

		SetSequenceBox(false);

		if(pScript->HasSpawnFlag(CScriptedSequence::FL_LEAVE_CORPSE))
		{
			SetUse(nullptr);
			SetThink(nullptr);
			SetThink(nullptr);
		}
		else
		{
			// Fade the corpse out
			FadeBeginThink();
		}

		// Stop animating
		StopAnimation();

		m_pState->movetype = MOVETYPE_NONE;
		m_pState->effects |= EF_NOINTERP;

		return;
	}

	// Move entity if we played an animation
	if(pScript->HasPlaySequence())
	{
		if(!pScript->HasSpawnFlag(CScriptedSequence::FL_NO_SCRIPT_MOVEMENT))
		{
			// Get position of Bip 01
			Vector bonePosition;
			if(!GetBonePosition(ROOT_BONE_NAME, bonePosition))
			{
				Util::EntityConDPrintf(m_pEdict, "%s - No bone named '%s'.\n", __FUNCTION__, ROOT_BONE_NAME); 
				bonePosition = pScript->GetOrigin();
			}

			// Sew new position if the move is big enough
			if((m_pState->origin - bonePosition).Length2D() > NPC_SCRIPT_MOVE_MIN_DIST)
			{
				m_pState->origin[0] = bonePosition[0];
				m_pState->origin[1] = bonePosition[1];
			}

			// Set ideal yaw to current angles
			m_pState->idealyaw = m_pState->angles[YAW];
		}

		// Reset animation
		m_currentActivity = ACT_RESET;
	}

	// Nudge the NPC to make sure it doesn't get stuck in the ground
	if(!HasSpawnFlag(FL_NPC_DONT_FALL) && !pScript->HasSpawnFlag(CScriptedSequence::FL_NO_SCRIPT_MOVEMENT))
		GroundEntityNudge();

	// Link it back to the world
	gd_engfuncs.pfnSetOrigin(m_pEdict, m_pEdict->state.origin);

	// Clear enemy
	m_enemy.reset();

	if(m_pState->health > 0)
	{
		// Go back to idle after script ends
		m_idealNPCState = NPC_STATE_IDLE;
	}
	else
	{
		// NPC was killed mid-sequence, so kill him now
		m_idealNPCState = NPC_STATE_DEAD;

		// Trigger any AI conditions
		SetCondition(AI_COND_LIGHT_DAMAGE);
		m_pState->deadstate = DEADSTATE_DYING;
		CheckAITriggers();
		m_pState->deadstate = DEADSTATE_NONE;
	}

	// Remove wait for script flag
	if(HasSpawnFlag(FL_NPC_WAIT_FOR_SCRIPT))
		RemoveSpawnFlag(FL_NPC_WAIT_FOR_SCRIPT);
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::ExitScriptedSequence( void )
{
	if(m_pState->deadstate == DEADSTATE_DYING)
	{
		m_idealNPCState = NPC_STATE_DEAD;
		return;
	}

	if(m_pScriptedSequence)
		m_pScriptedSequence->CancelScript();
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::NPCUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	// Move NPC to alert state
	m_idealNPCState = NPC_STATE_ALERT;
}

//=============================================
// @brief Spawns particles when NPC is gibbed
//
//=============================================
void CBaseNPC::SpawnGibbedParticles( void )
{
	if(m_deathMode == DEATH_EXPLODE)
		Util::CreateParticles("engine_gib_explode_cluster.txt", GetCenter(), ZERO_VECTOR, PART_SCRIPT_CLUSTER);
	else
		Util::CreateParticles("engine_gib_cluster.txt", GetCenter(), ZERO_VECTOR, PART_SCRIPT_CLUSTER);
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::GibNPC( void )
{
	// Play the splatter sound
	CString soundFile;
	soundFile << "impact/gib_0" << (Int32)Common::RandomLong(1, 2) << ".wav";
	Util::EmitAmbientSound(GetCenter(), soundFile.c_str());

	Float chestMinvel;
	Float chestMaxvel;
	Float headMinvel;
	Float headMaxvel;
	Float randomMinvel;
	Float randomMaxvel;

	Vector myCenter;
	Vector* pCenter = nullptr;
	if(m_deathDamageBits & (DMG_UNUSED2|DMG_UNUSED3|DMG_EXPLOSION))
	{
		// Make gibs scatter all over
		chestMinvel = 80;
		chestMaxvel = 160;
		headMinvel = 60;
		headMaxvel = 250;
		randomMinvel = 150;
		randomMaxvel = 250;

		myCenter = GetCenter();
		pCenter = &myCenter;
	}
	else
	{
		// Set default values
		chestMinvel = -50;
		chestMaxvel = 50;
		headMinvel = -200;
		headMaxvel = 200;
		randomMinvel = 300;
		randomMaxvel = 400;
	}

	// Spawn one-piece gibs
	CGib::SpawnHeadGib(this, pCenter, headMinvel, headMaxvel);
	CGib::SpawnChestGib(this, pCenter, chestMinvel, chestMaxvel);
	CGib::SpawnRandomGibs(this, Common::RandomLong(4, 8), pCenter, randomMinvel, randomMaxvel);

	if(m_enemy && m_enemy->IsPlayer())
		m_enemy->SetNPCAwareness(0, this, 0, false);

	// Spawn particles for gibbing
	SpawnGibbedParticles();

	// Shut the voice channel
	Util::StopEntitySounds(this, SND_CHAN_VOICE);

	SetThink(&CBaseEntity::RemoveThink);
	m_pState->nextthink = g_pGameVars->time;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::OnGibSpawnCallback( CBaseEntity* pGib )
{
	if(m_deathDamageBits & DMG_UNUSED2)
	{
		Vector direction = -pGib->GetVelocity();
		direction.Normalize();

		Util::CreateParticles("plasmaimpact_cluster.txt", pGib->GetOrigin(), direction, PART_SCRIPT_CLUSTER, pGib->GetEdict(), 0, pGib->GetEntityIndex(), NO_POSITION, PARTICLE_ATTACH_TO_PARENT);
		
		Int32 lightystyle = (Common::RandomLong(0, 1) == 1) ? LS_FLICKER_A : LS_FLICKER_B;
		Util::CreateDynamicLight(pGib->GetOrigin(), 128, 255, 50, 0, 7, 64, 5, (FL_DLIGHT_NOSHADOWS|FL_DLIGHT_FOLLOW_ENTITY|FL_DLIGHT_USE_LIGHTSTYLES), pGib->GetEntityIndex(), NO_POSITION, lightystyle);
	}
	else if(m_deathDamageBits & DMG_EXPLOSION && Common::RandomLong(0, 1) == 1)
	{
		Util::CreateParticles("engine_gib_flames_cluster.txt", pGib->GetOrigin(), Vector(0, 0, 1), PART_SCRIPT_CLUSTER, pGib->GetEdict(), 0, pGib->GetEntityIndex(), NO_POSITION, PARTICLE_ATTACH_TO_PARENT);

		Int32 lightystyle = (Common::RandomLong(0, 1) == 1) ? LS_FLICKER_A : LS_FLICKER_B;
		Util::CreateDynamicLight(pGib->GetOrigin(), 128, 255, 192, 64, 10, 64, 6, (FL_DLIGHT_NOSHADOWS|FL_DLIGHT_FOLLOW_ENTITY|FL_DLIGHT_USE_LIGHTSTYLES), pGib->GetEntityIndex(), NO_POSITION, lightystyle);

		// Create bbox for prolonged dmg
		Float burnDmg = gSkillData.GetSkillCVarSetting(g_skillcvars.skillExplodeGibBurnDmg);
		Float dmgDelay = gSkillData.GetSkillCVarSetting(g_skillcvars.skillExplodeGibDmgDelay);
		Float dmgRadius = gSkillData.GetSkillCVarSetting(g_skillcvars.skillExplodeGibDmgRadius);

		Vector mins(-dmgRadius, -dmgRadius, -dmgRadius);
		Vector maxs(dmgRadius, dmgRadius, dmgRadius);

		CTimeDamage::CreateTimeDamageBox(m_killer, pGib->GetOrigin(), mins, maxs, DMG_BURN, dmgDelay, burnDmg, 5, pGib);
	}
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::GroundEntityNudge( bool noExceptions )
{
	if(!noExceptions && m_pState->movetype != MOVETYPE_FLY)
	{
		if(!m_valuesParsed || m_npcState == NPC_STATE_SCRIPT 
			|| HasSpawnFlag(FL_NPC_DONT_FALL) 
			|| HasSpawnFlag(FL_NPC_WAIT_FOR_SCRIPT))
			return;
	}

	// Move the NPC up a bit, then drop him
	Vector preNudgeOrigin = m_pState->origin;
	m_pState->flags &= ~FL_ONGROUND;
	m_pState->origin.z += 4;

	if(!gd_engfuncs.pfnDropToFloor(m_pEdict))
	{
		// If nudge fails, re-set the previous origin
		gd_engfuncs.pfnSetOrigin(m_pEdict, preNudgeOrigin);
	}
}

//=============================================
// @brief
//
//=============================================
Float CBaseNPC::GetYawDifference( void )
{
	Float currentYaw = Math::AngleMod(m_pState->angles[1]);
	if(currentYaw != m_pState->idealyaw)
		return Util::AngleDistance(m_pState->idealyaw, currentYaw);
	else
		return 0;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::ChangeYaw( Double timeInterval )
{
	if(!m_pState->yawspeed)
		return;

	if(!IsMoving() && !m_updateYaw)
		return;

	Float currentYaw = Math::AngleMod(m_pState->angles[YAW]);
	Float idealYaw = m_pState->idealyaw;
	if(currentYaw != idealYaw)
	{
		Float yawSpeed = m_pState->yawspeed * timeInterval;
		Float yawMove = idealYaw - currentYaw;

		if(idealYaw > currentYaw)
		{
			if(yawMove >= 180.0f)
				yawMove -= 360.0f;
		}
		else if(yawMove <= -180.0f)
			yawMove += 360.0f;

		if(yawMove > 0)
		{
			if(yawMove > yawSpeed)
				yawMove = yawSpeed;
		}
		else if(yawMove < -yawSpeed)
			yawMove = -yawSpeed;

		// Set the yaw angle
		m_pState->angles[YAW] = Math::AngleMod(currentYaw+yawMove);

		// Turn head in desired direction if we can turn heads
		if(HasCapability(AI_CAP_TURN_HEAD))
		{
			Float headYaw = m_pState->idealyaw - m_pState->angles[YAW];
			if(headYaw > 180.0f)
				headYaw -= 360.0f;
			else if(headYaw < -180.0f)
				headYaw += 360.0f;

			SetIdealHeadYaw(headYaw);
		}
	}
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::IsFacingIdealYaw( void )
{
	return (SDL_fabs(GetYawDifference()) < 1.0f) ? true : false;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::SetTurnActivity( void )
{
	// Get difference in yaw-ideal yaw
	Float yawDifference = GetYawDifference();

	if(yawDifference <= -45 && FindActivity(ACT_TURN_RIGHT) != NO_SEQUENCE_VALUE)
		SetIdealActivity(ACT_TURN_RIGHT);
	else if(yawDifference > 45 && FindActivity(ACT_TURN_LEFT) != NO_SEQUENCE_VALUE)
		SetIdealActivity(ACT_TURN_LEFT);
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::SetIdealYaw( const Vector& targetVector, bool isPositionVector )
{
	if(isPositionVector)
	{
		if(m_movementActivity == ACT_STRAFE_LEFT || m_movementActivity == ACT_STRAFE_RIGHT)
		{
			Vector projVector;
			if(m_movementActivity == ACT_STRAFE_LEFT)
			{
				projVector[0] = -targetVector[1];
				projVector[1] = targetVector[0];
			}
			else
			{
				projVector[0] = targetVector[1];
				projVector[1] = targetVector[0];
			}

			m_pState->idealyaw = Util::VectorToYaw(projVector - m_pState->origin);
		}
		else
		{
			// Other animations are handled simpler
			m_pState->idealyaw = Util::VectorToYaw(targetVector - m_pState->origin);
		}
	}
	else
	{
		// The vector supplied is already a direction vector
		m_pState->idealyaw = Util::VectorToYaw(targetVector);
	}
}

//=============================================
// @brief
//
//=============================================
Float CBaseNPC::VectorToYaw( const Vector& direction ) const
{
	if(direction.IsZero())
		return m_pState->angles[YAW];
	else
		return Util::VectorToYaw(direction);
}

//=============================================
// @brief
//
//=============================================
Float CBaseNPC::VectorToPitch( const Vector& direction ) const
{
	if(direction.IsZero())
		return m_pState->angles[PITCH];
	else
		return Util::VectorToPitch(direction);
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::ClearCondition( Uint32 conditionBit )
{
	m_aiConditionBits.reset(conditionBit);
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::ClearConditions( const CBitSet& conditionBitSet )
{
	m_aiConditionBits &= ~conditionBitSet;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::SetCondition( Uint32 conditionBit )
{
	m_aiConditionBits.set(conditionBit);
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::SetConditions( const CBitSet& conditionBitSet )
{
	m_aiConditionBits |= conditionBitSet;
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::CheckCondition( Uint32 conditionBit ) const
{
	return m_aiConditionBits.test(conditionBit);
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::CheckConditions( const CBitSet& conditionBitSet ) const
{
	return (m_aiConditionBits & conditionBitSet).any() ? true : false;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::SetMemory( Uint32 memoryBit )
{
	m_memoryBits.set(memoryBit);
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::ClearMemory( Uint32 memoryBit )
{
	m_memoryBits.reset(memoryBit);
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::HasMemory( Uint32 memoryBit ) const
{
	return (m_memoryBits.test(memoryBit)) ? true : false;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::SetCapability( Uint32 capabilityBit )
{
	m_capabilityBits.set(capabilityBit);
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::SetCapabilities( const CBitSet& capabilityBitSet )
{
	m_capabilityBits |= capabilityBitSet;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::DisableCapability( Uint32 capabilityBit )
{
	m_disabledCapabilityBits.set(capabilityBit);
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::RemoveCapability( Uint32 capabilityBit )
{
	m_capabilityBits.reset(capabilityBit);
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::HasCapability( Uint32 capabilityBit ) const
{
	return (m_capabilityBits.test(capabilityBit) && !m_disabledCapabilityBits.test(capabilityBit)) ? true : false;
}

//=============================================
// @brief
//
//=============================================
Int32 CBaseNPC::GetRelationship( CBaseEntity* pOther )
{
	Int32 myClassification = GetClassification();
	assert(myClassification >= 0 && myClassification < NB_CLASSIFICATIONS);
	Int32 enemyClassification = pOther->GetClassification();
	assert(enemyClassification >= 0 && enemyClassification < NB_CLASSIFICATIONS);

	return NPC_RELATIONS_TABLE[myClassification][enemyClassification];
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::UpdateDistances( void )
{
	Int32 endDistance = CEnvFog::GetFogEndDistance();
	if(endDistance > 0)
	{
		// These fractions should depend on fog end distance
		Float minVisibilityFraction;
		Float minFiringFractionEdgeCase;
		Float minFiringFractionGeneric;
		if(endDistance < 2000)
		{
			minVisibilityFraction = 0.9;
			minFiringFractionEdgeCase = 0.6;
			minFiringFractionGeneric = 0.7;
		}
		else
		{
			minVisibilityFraction = 0.9;
			minFiringFractionEdgeCase = 0.8;
			minFiringFractionGeneric = 0.8;
		}

		// Put visibility at very edge
		m_lookDistance = endDistance * minVisibilityFraction;

		// For very small or very large firing cones, set up shorter distance
		Uint32 firingCone = GetFiringCone(false);
		if(firingCone <= 1 || firingCone >= 7)
			m_firingDistance = endDistance * minFiringFractionEdgeCase;
		else
			m_firingDistance = endDistance * minFiringFractionGeneric;
	}
	else
	{
		m_lookDistance = NPC_DEFAULT_MAX_LOOK_DISTANCE;
		m_firingDistance = NPC_DEFAULT_MAX_FIRING_DISTANCE;
	}
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::RunSenses( void )
{
	// Don't run if dead or not in any valid state
	if(m_npcState == NPC_STATE_NONE || m_npcState == NPC_STATE_DEAD)
		return;

	// Don't run if we do not have a valid target, and the client is not in the PVS, and we are not in combat or script mode
	if(!m_targetEntity && !gd_engfuncs.pfnFindClientInPVS(m_pEdict) && m_npcState != NPC_STATE_COMBAT && m_npcState != NPC_STATE_SCRIPT)
		return;

	// Try to look for targets
	Look();

	// Listen for audible sounds
	HearSounds();

	// Clear ignore conditions
	ClearConditions(GetIgnoreConditions());

	// Look for enemies
	GetNextEnemy();
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::RunAI( void )
{
	// Play idle sound at random
	if((m_npcState == NPC_STATE_IDLE || m_npcState == NPC_STATE_ALERT) 
		&& Common::RandomLong(0, 99) == 0 && !HasSpawnFlag(FL_NPC_GAG))
		EmitIdleSound();

	if(m_npcState != NPC_STATE_DEAD && m_pState->deadstate != DEADSTATE_DYING)
	{
		if(gd_engfuncs.pfnFindClientInPVS(m_pEdict) != nullptr || m_npcState == NPC_STATE_COMBAT)
		{
			// Make the NPC sense things
			RunSenses();

			if(m_enemy)
			{
				// If enemy is not hull, update it
				CheckEnemy();
			}
		}

		UpdateExpressions();

		// Check ammo always
		CheckAmmo();
	}

	// Check for AI triggers
	CheckAITriggers();

	// Run pre-schedule thinking
	PreScheduleThink();

	// Maintain running schedules
	MaintainSchedule();

	// Clear hurt conditions so AI doesn't get stuck
	ClearCondition(AI_COND_LIGHT_DAMAGE);
	ClearCondition(AI_COND_HEAVY_DAMAGE);
	ClearCondition(AI_COND_HEARD_ENEMY_NEW_POSITION);
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::CheckAITriggers( void )
{
	// Check trigger 1
	if(m_triggerCondition1 != AI_TRIGGER_NONE 
		&& m_triggerTarget1 != NO_STRING_VALUE 
		&& CheckAITrigger(m_triggerCondition1))
	{
		Util::EntityConDPrintf(m_pEdict, "Firing AI trigger target 1.\n");
		Util::FireTargets(gd_engfuncs.pfnGetString(m_triggerTarget1), this, this, USE_TOGGLE, 0);

		// Clear this trigger
		m_triggerTarget1 = NO_STRING_VALUE;
		m_triggerCondition1 = AI_TRIGGER_NONE;

		// If trigger target 2 uses the same target, then clear that too
		if(m_triggerCondition2 != AI_TRIGGER_NONE 
			&& m_triggerTarget2 != NO_STRING_VALUE
			&& !qstrcmp(gd_engfuncs.pfnGetString(m_triggerTarget1), gd_engfuncs.pfnGetString(m_triggerTarget2)))
		{
			m_triggerTarget2 = NO_STRING_VALUE;
			m_triggerCondition2 = AI_TRIGGER_NONE;
		}
	}

	// Check trigger 2
	if(m_triggerCondition2 != AI_TRIGGER_NONE 
		&& m_triggerTarget2 != NO_STRING_VALUE 
		&& CheckAITrigger(m_triggerCondition2))
	{
		Util::EntityConDPrintf(m_pEdict, "Firing AI trigger target 2.\n");
		Util::FireTargets(gd_engfuncs.pfnGetString(m_triggerTarget2), this, this, USE_TOGGLE, 0);

		// Clear this trigger
		m_triggerTarget2 = NO_STRING_VALUE;
		m_triggerCondition2 = AI_TRIGGER_NONE;

		// If trigger target 1 uses the same target, then clear that too
		if(m_triggerCondition1 != AI_TRIGGER_NONE 
			&& m_triggerTarget1 != NO_STRING_VALUE
			&& !qstrcmp(gd_engfuncs.pfnGetString(m_triggerTarget2), gd_engfuncs.pfnGetString(m_triggerTarget1)))
		{
			m_triggerTarget1 = NO_STRING_VALUE;
			m_triggerCondition1 = AI_TRIGGER_NONE;
		}
	}
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::SetAITriggerCondition( Int32 conditionIndex, Int32 condition, const Char* pstrTarget )
{
	switch(conditionIndex)
	{
	case 1:
		{
			if(conditionIndex != AI_TRIGGER_NONE && pstrTarget && qstrlen(pstrTarget))
			{
				m_triggerTarget2 = gd_engfuncs.pfnAllocString(pstrTarget);
				m_triggerCondition2 = condition;
			}
			else
			{
				m_triggerTarget2 = NO_STRING_VALUE;
				m_triggerCondition2 = AI_TRIGGER_NONE;
			}
		}
		break;
	default:
	case 0:
		{
			if(condition != AI_TRIGGER_NONE && pstrTarget && qstrlen(pstrTarget))
			{
				m_triggerTarget1 = gd_engfuncs.pfnAllocString(pstrTarget);
				m_triggerCondition1 = condition;
			}
			else
			{
				m_triggerTarget1 = NO_STRING_VALUE;
				m_triggerCondition1 = AI_TRIGGER_NONE;
			}
		}
		break;
	}
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::CheckAITrigger( Int32 triggerCondition )
{
	switch(triggerCondition)
	{
	case AI_TRIGGER_SEE_PLAYER_ANGRY_AT_PLAYER:
		{
			if(m_enemy && m_enemy->IsPlayer() && CheckCondition(AI_COND_SEE_ENEMY))
				return true;
		}
		break;
	case AI_TRIGGER_TAKEDAMAGE:
		{
			if(CheckCondition(AI_COND_LIGHT_DAMAGE) || CheckCondition(AI_COND_HEAVY_DAMAGE))
				return true;
		}
		break;
	case AI_TRIGGER_HALF_HEALTH:
		{
			if(IsAlive() && m_pState->health <= (m_pState->maxhealth*0.5))
				return true;
		}
		break;
	case AI_TRIGGER_DEATH:
		{
			if(m_pState->deadstate != DEADSTATE_NONE)
				return true;
		}
		break;
	case AI_TRIGGER_HEAR_WORLD:
		{
			if(CheckCondition(AI_COND_HEAR_SOUND) && (m_soundTypes & AI_SOUND_WORLD))
				return true;
		}
		break;
	case AI_TRIGGER_HEAR_PLAYER:
		{
			if(CheckCondition(AI_COND_HEAR_SOUND) && (m_soundTypes & AI_SOUND_PLAYER))
				return true;
		}
		break;
	case AI_TRIGGER_HEAR_COMBAT:
		{
			if(CheckCondition(AI_COND_HEAR_SOUND) && (m_soundTypes & AI_SOUND_COMBAT))
				return true;
		}
		break;
	case AI_TRIGGER_SEE_PLAYER_UNCONDITIONAL:
		{
			if(CheckCondition(AI_COND_SEE_CLIENT))
				return true;
		}
		break;
	case AI_TRIGGER_SEE_PLAYER_NOT_IN_COMBAT:
		{
			if(CheckCondition(AI_COND_SEE_CLIENT)
				&& m_npcState != NPC_STATE_COMBAT
				&& m_npcState != NPC_STATE_SCRIPT)
				return true;
		}
		break;
	case AI_TRIGGER_SEE_ENEMY:
		{
			if(m_enemy && CheckCondition(AI_COND_SEE_ENEMY))
				return true;
		}
		break;
	}

	return false;
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::CanPlaySequence( bool disregardState, script_interrupt_level_t interruptLevel )
{
	// Don't play sequences ifn ot alive or already posessed
	if(m_pScriptedSequence || !IsAlive())
		return false;

	// If forced, just go into it
	if(disregardState)
		return true;

	if(m_npcState == NPC_STATE_NONE
		|| m_npcState == NPC_STATE_IDLE
		|| m_idealNPCState == NPC_STATE_IDLE)
		return true;

	if(m_npcState == NPC_STATE_ALERT 
		&& interruptLevel >= SCRIPT_INTERRUPT_BY_NAME)
		return true;

	return false;
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::CanPlaySentence( bool disregardState )
{
	return IsAlive() ? true : false;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::PlaySentence( const Char* pstrSentenceName, Float duration, Float volume, Float attenuation, Float timeOffset, bool subtitleOnlyInRadius, CBaseEntity* pPlayer )
{
	if(!IsAlive() || !g_pSentencesFile)
		return;

	Float sentduration = 0;
	if(pstrSentenceName && qstrlen(pstrSentenceName))
	{
		Uint64 sndFlags = SND_FL_NONE;
		if(subtitleOnlyInRadius)
			sndFlags |= SND_FL_SUB_ONLY_RADIUS;
		
		if(pstrSentenceName[0] == '!')
		{
			// Just play the sentence specified
			const Char* pstrName = g_pSentencesFile->GetSentence(pstrSentenceName, &sentduration);
			if(pstrName)
				Util::EmitEntitySound(this, pstrName, SND_CHAN_VOICE, volume, attenuation, GetVoicePitch(), sndFlags, pPlayer, timeOffset);
		}
		else
		{
			// Get sentence from group
			const Char* pstrRandomSentence = g_pSentencesFile->GetRandomSentence(pstrSentenceName, &sentduration);
			if(pstrRandomSentence)
				Util::EmitEntitySound(this, pstrRandomSentence, SND_CHAN_VOICE, volume, attenuation, GetVoicePitch(), sndFlags, pPlayer, timeOffset);
		}
	}

	// Make sure the duration is set before checking if pstrSentenceName is valid
	Float _duration = (duration < sentduration) ? sentduration : duration;
	if(_duration > 0)
		m_talkTime = g_pGameVars->time + _duration;
	else
		m_talkTime = g_pGameVars->time + 3;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::PlayScriptedSentence( const Char* pstrSentenceName, Float duration, Float volume, Float attenuation, Float timeOffset, bool subtitleOnlyInRadius, bool isConcurrent, CBaseEntity* pListener, CBaseEntity* pPlayer )
{
	PlaySentence(pstrSentenceName, duration, volume, attenuation, timeOffset, subtitleOnlyInRadius, pPlayer);

	if(HasCapability(AI_CAP_EXPRESSIONS) && pstrSentenceName && qstrlen(pstrSentenceName) > 0)
		PlayFlexScript(pstrSentenceName);
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::StopSentence( void )
{
	Util::EmitEntitySound(this, NULL_SOUND_FILENAME, SND_CHAN_VOICE);
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::CorpseFallThink( void )
{
	if(m_pState->flags & FL_ONGROUND)
	{
		SetThink(nullptr);

		SetSequenceBox(false);
		gd_engfuncs.pfnSetOrigin(m_pEdict, m_pState->origin);
	}
	else
	{
		// Think again in 0.1s
		m_pState->nextthink = g_pGameVars->time + 0.1;
	}
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::SetNPCState( npcstate_t state )
{
	// Drop any enemies when going to idle
	if(state == NPC_STATE_IDLE && m_enemy)
	{
		m_enemy.reset();
		Util::EntityConPrintf(m_pEdict, "Enemy was stripped when changing to idle AI state.\n");
	}

	m_npcState = state;
	m_idealNPCState = state;

	if(m_npcState == NPC_STATE_COMBAT && state != NPC_STATE_COMBAT)
		m_lastEnemySeekPosition.Clear();
}

//=============================================
// @brief
//
//=============================================
npcstate_t CBaseNPC::GetIdealNPCState( void )
{
	CBitSet conditions = GetScheduleFlags();

	switch(m_npcState)
	{
	case NPC_STATE_IDLE:
		{
			if(conditions.test(AI_COND_NEW_ENEMY))
			{
				// We have a new enemy, so switch to combat
				m_idealNPCState = NPC_STATE_COMBAT;
			}
			else if(conditions.test(AI_COND_LIGHT_DAMAGE) || conditions.test(AI_COND_HEAVY_DAMAGE))
			{
				// We got hit, so face the direction of the attack
				SetIdealYaw(-m_lastAttackVector, false);
				m_idealNPCState = NPC_STATE_ALERT;
			}
			else if(conditions.test(AI_COND_HEAR_SOUND))
			{
				if(m_pBestSound)
				{
					SetIdealYaw(m_pBestSound->position);
					if(m_pBestSound->typeflags & (AI_SOUND_DANGER|AI_SOUND_COMBAT))
						m_idealNPCState = NPC_STATE_ALERT;
				}
			}
		}
		break;
	case NPC_STATE_ALERT:
		{
			if(conditions.test(AI_COND_NEW_ENEMY) || conditions.test(AI_COND_SEE_ENEMY))
			{
				// We have a new enemy, so switch to combat
				m_idealNPCState = NPC_STATE_COMBAT;
			}
			else if(conditions.test(AI_COND_HEAR_SOUND))
			{
				m_idealNPCState = NPC_STATE_ALERT;

				if(m_pBestSound && m_pBestSound->emitter != const_cast<const CBaseNPC*>(this))
					SetIdealYaw(m_pBestSound->position);
			}
		}
		break;
	case NPC_STATE_COMBAT:
		{
			// Clear this condition when in combat
			if(CheckCondition(AI_COND_BLOCKING_PATH))
				ClearCondition(AI_COND_BLOCKING_PATH);

			// This shouldn't happen
			if(!m_enemy)
			{
				m_idealNPCState = NPC_STATE_ALERT;
				Util::EntityConPrintf(m_pEdict, "Combat state with no enemy.\n");
			}
		}
		break;
	case NPC_STATE_DEAD:
		{
			// Keep this state from now on
			m_idealNPCState = NPC_STATE_DEAD;
		}
		break;
	case NPC_STATE_SCRIPT:
		{
			if(!m_pScriptedSequence || conditions.test(AI_COND_TASK_FAILED) || (conditions & m_pScriptedSequence->GetScriptBreakingAIConditions()).any())
				ExitScriptedSequence();
		}
		break;
	}

	// If we were previously in danger and now we're idle, clear it
	if(m_idealNPCState == NPC_STATE_IDLE && CheckCondition(AI_COND_IN_DANGER))
		ClearCondition(AI_COND_IN_DANGER);

	return (npcstate_t)m_idealNPCState;
}

//=============================================
// @brief
//
//=============================================
npcstate_t CBaseNPC::GetNPCState( void ) 
{ 
	return (npcstate_t)m_npcState; 
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::UpdatePartialAwareness( enemyawareness_t* pAwarenessinfo, Uint64 sightBits )
{
	assert(pAwarenessinfo != nullptr);
	assert(pAwarenessinfo->entity != nullptr);

	CBaseEntity* pEntity = pAwarenessinfo->entity;
	if (!pEntity->IsPlayer())
	{
		assert(false);
		return;
	}

	if(sightBits & AI_SIGHTED_PLAYER_LEAN)
	{
		if(pAwarenessinfo->awareness < 1.0)
		{
			Vector playerDir = (pEntity->GetEyePosition() - m_pState->origin).Normalize();

			Vector forward;
			Math::AngleVectors(m_pState->angles, &forward);
			Float dp = Math::DotProduct(playerDir, forward);
			dp = clamp(dp, 0.0, 1.0);

			if(dp > 0.5)
			{
				pAwarenessinfo->awareness += m_thinkIntervalTime*(1.0f/GetLeanAwarenessTime())*dp;
				if(pAwarenessinfo->awareness > 1.0)
					pAwarenessinfo->awareness = 1.0;

				pAwarenessinfo->lastsighttime = g_pGameVars->time;
			}

			// If not aware, set this as npc awareness
			if(!IsAwareOf(pEntity))
				pEntity->SetNPCAwareness(pAwarenessinfo->awareness, this, NPC_LEANAWARENESS_TIMEOUT, true);
		}
		else if(sightBits & AI_SIGHTED_PLAYER_FULL)
		{
			// Set lean awareness to full
			pAwarenessinfo->awareness = 1.0;
			pAwarenessinfo->lastsighttime = g_pGameVars->time;
			pEntity->SetNPCAwareness(1.0, this, NPC_COMBATSTATE_TIMEOUT, false);
		}
	}
}

//=============================================
// @brief
//
//=============================================
CBaseNPC::enemyawareness_t* CBaseNPC::GetEnemyPartialAwarenessInfo( CBaseEntity* pEntity )
{
	if (!m_enemyPartialAwarenessList.empty())
	{
		m_enemyPartialAwarenessList.begin();
		while (!m_enemyPartialAwarenessList.end())
		{
			enemyawareness_t& awareness = m_enemyPartialAwarenessList.get();
			if (!awareness.entity)
			{
				m_enemyPartialAwarenessList.remove(m_enemyPartialAwarenessList.get_link());
				m_enemyPartialAwarenessList.next();
				continue;
			}

			if (awareness.entity.get() == pEntity->GetEdict())
				return &awareness;

			m_enemyPartialAwarenessList.next();
		}
	}

	enemyawareness_t& awareness = m_enemyPartialAwarenessList.add({})->_val;
	awareness.entity = pEntity;

	return &awareness;
}

//=============================================
// @brief
//
//=============================================
CBaseNPC::enemyawareness_t* CBaseNPC::GetEnemyAwarenessInfo(CBaseEntity* pEntity)
{
	return nullptr;
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::ShouldSeeNPC( Uint64 sightBits, CBaseEntity* pEntity, enemyawareness_t* pPartialAwareness, enemyawareness_t* pEnemyAwareness )
{
	if(pEntity->GetEffectFlags() & EF_NODRAW)
		return false;

	if(pEntity->IsPlayer())
	{
		if(sightBits & AI_SIGHTED_PLAYER_FULL)
		{
			if(HasSpawnFlag(FL_NPC_WAIT_TILL_SEEN))
			{
				if(!pEntity->IsInView(this))
				{
					// Not in view yet
					return false;
				}
				else
				{
					// Remove flag
					RemoveSpawnFlag(FL_NPC_WAIT_TILL_SEEN);
				}
			}

			// We should see the player
			return true;
		}
		else if(sightBits & AI_SIGHTED_PLAYER_LEAN)
		{
			// Make fully visible if lean awareness is full
			return (pPartialAwareness && pPartialAwareness->awareness == 1.0) ? true : false;
		}
	}
	else if(sightBits & AI_SIGHTED_NPC)
	{
		// Saw an NPC, become instantly aware
		return true;
	}

	return false;
}

//=============================================
// @brief
//
//=============================================
Uint64 CBaseNPC::GetNPCVisibilityBits( CBaseEntity* pEntity, bool checkGlass, enemyawareness_t** pAwarenessPtr )
{
	trace_t tr;
	Uint64 sightBits = 0;
	// Get eye position
	Vector eyePosition = GetEyePosition();
	// Check first without adding lean
	Vector otherEyePosition = pEntity->GetEyePosition(false);

	if(pEntity->IsPlayer())
	{
		// Do special checks for glass
		if(checkGlass)
		{
			Util::TraceLine(eyePosition, otherEyePosition, true, false, false, false, m_pEdict, tr);

			if(!tr.noHit() && tr.hitentity != NO_ENTITY_INDEX)
			{
				edict_t* pedict = gd_engfuncs.pfnGetEdictByIndex(tr.hitentity);
				CBaseEntity* pHitEntity = CBaseEntity::GetClass(pedict);

				if(pHitEntity->IsFuncBreakableEntity())
				{
					// If there's no other opaque occluder, then we saw the player
					Util::TraceLine(eyePosition, otherEyePosition, true, false, true, false, m_pEdict, tr);
					if(!tr.noHit() && tr.hitentity == pEntity->GetEntityIndex())
						sightBits |= AI_SIGHTED_PLAYER_FULL;
				}
			}
		}
		else
		{
			// Do a check ignoring glass objects
			Util::TraceLine(eyePosition, otherEyePosition, true, false, true, false, m_pEdict, tr);
		}

		if(!checkGlass && !tr.noHit())
		{
			edict_t* pedict = gd_engfuncs.pfnGetEdictByIndex(tr.hitentity);
			CBaseEntity* pHitEntity = CBaseEntity::GetClass(pedict);

			rendermode_t renderMode = pHitEntity->GetRenderMode();
			if(renderMode != RENDER_NORMAL && (renderMode & RENDERMODE_BITMASK) != RENDER_TRANSCOLOR)
				Util::TraceLine(tr.endpos, otherEyePosition, true, false, true, false, pedict, tr);
		}

		if(tr.noHit())
		{
			// Player is fully visible
			sightBits |= AI_SIGHTED_PLAYER_FULL;
		}
		else
		{
			// Check entity visibility with lean added
			otherEyePosition = pEntity->GetEyePosition(true);

			// Do special checks for glass
			if(checkGlass)
			{
				Util::TraceLine(eyePosition, otherEyePosition, true, false, false, false, m_pEdict, tr);
				if(!tr.noHit() && tr.hitentity != NO_ENTITY_INDEX)
				{
					edict_t* pedict = gd_engfuncs.pfnGetEdictByIndex(tr.hitentity);
					CBaseEntity* pHitEntity = CBaseEntity::GetClass(pedict);

					if(pHitEntity->IsFuncBreakableEntity())
					{
						// If there's no other opaque occluder, then we saw the player
						Util::TraceLine(eyePosition, otherEyePosition, true, false, true, false, m_pEdict, tr);
					}
				}
			}
			else
			{
				// Do a check ignoring glass objects
				Util::TraceLine(eyePosition, otherEyePosition, true, false, true, false, m_pEdict, tr);
			}

			if(!checkGlass && !tr.noHit())
			{
				edict_t* pedict = gd_engfuncs.pfnGetEdictByIndex(tr.hitentity);
				CBaseEntity* pHitEntity = CBaseEntity::GetClass(pedict);

				rendermode_t renderMode = pHitEntity->GetRenderMode();
				if(renderMode != RENDER_NORMAL && (renderMode & RENDERMODE_BITMASK) != RENDER_TRANSCOLOR)
					Util::TraceLine(tr.endpos, otherEyePosition, true, false, true, false, pedict, tr);
			}

			if((tr.noHit() || tr.hitentity == pEntity->GetEntityIndex()) && !tr.allSolid() && !tr.startSolid())
			{
				// Mark as being visible leaning only
				sightBits |= AI_SIGHTED_PLAYER_LEAN;

				enemyawareness_t* pAwareness = nullptr;
				if (pAwarenessPtr)
				{
					if (!(*pAwarenessPtr))
						(*pAwarenessPtr) = GetEnemyPartialAwarenessInfo(pEntity);

					pAwareness = (*pAwarenessPtr);
					if (pAwareness->awareness >= 1.0)
						sightBits |= AI_SIGHTED_PLAYER_FULL;
				}
			}
		}
	}
	else
	{
		if(checkGlass)
		{
			Util::TraceLine(eyePosition, otherEyePosition, true, false, false, false, m_pEdict, tr);

			if(!tr.noHit() && tr.hitentity != NO_ENTITY_INDEX)
			{
				edict_t* pedict = gd_engfuncs.pfnGetEdictByIndex(tr.hitentity);
				CBaseEntity* pHitEntity = CBaseEntity::GetClass(pedict);

				// If there's no other opaque occluder, then we saw the player
				if(pHitEntity->IsFuncBreakableEntity())
					Util::TraceLine(eyePosition, otherEyePosition, true, false, true, false, m_pEdict, tr);
			}
		}
		else
		{
			Util::TraceLine(eyePosition, otherEyePosition, true, false, false, false, m_pEdict, tr);
		}

		if(!checkGlass && !tr.noHit())
		{
			edict_t* pedict = gd_engfuncs.pfnGetEdictByIndex(tr.hitentity);
			CBaseEntity* pHitEntity = CBaseEntity::GetClass(pedict);

			rendermode_t renderMode = pHitEntity->GetRenderMode();
			if(renderMode != RENDER_NORMAL && (renderMode & 255) != RENDER_TRANSCOLOR)
				Util::TraceLine(tr.endpos, otherEyePosition, true, false, true, false, pedict, tr);
		}

		if((tr.noHit() || tr.hitentity == pEntity->GetEntityIndex()) && !tr.allSolid() && !tr.startSolid())
			sightBits |= AI_SIGHTED_NPC;
	}

	return sightBits;
}

//=============================================
// @brief Returns an entity we can kick
//
//=============================================
CBaseEntity* CBaseNPC::GetKickEntity( Float checkDistance )
{
	trace_t tr;

	Vector forward;
	Math::AngleVectors(m_pState->angles, &forward);

	Vector startPosition = m_pState->origin;
	startPosition.z += m_pState->size.z * 0.5;

	Vector endPosition = startPosition + forward*checkDistance;

	Util::TraceHull(startPosition, endPosition, false, false, HULL_SMALL, m_pEdict, tr);
	if(!tr.noHit() && tr.hitentity != NO_ENTITY_INDEX)
	{
		CBaseEntity* pEntity = Util::GetEntityFromTrace(tr);
		if(pEntity && (pEntity->IsNPC() || pEntity->IsPlayer() || pEntity->IsFuncBreakableEntity()))
			return pEntity;
	}

	return nullptr;
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::IsInView( CBaseEntity* pOther ) const
{
	return Util::IsInViewCone(GetEyePosition(), m_pState->angles, m_fieldOfView, pOther->GetCenter());
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::IsInView( const Vector& position ) const
{
	return Util::IsInViewCone(GetEyePosition(), m_pState->angles, m_fieldOfView, position);
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::Look( void )
{
	// Clear any previous conditions related to seeing
	ClearCondition(AI_COND_SEE_HATE);
	ClearCondition(AI_COND_SEE_DISLIKE);
	ClearCondition(AI_COND_SEE_ENEMY);
	ClearCondition(AI_COND_SEE_FEAR);
	ClearCondition(AI_COND_SEE_NEMESIS);
	ClearCondition(AI_COND_SEE_CLIENT);
	ClearCondition(AI_COND_SEE_HOSTILE_NPC);

	// Sighted enemy AI conditions
	CBitSet sightConditions(AI_COND_BITSET_SIZE);
	// Sighted enemy bits
	Uint64 sightBits = 0;

	// Clear previous list contents
	if(!m_sightedHostileNPCsList.empty())
		m_sightedHostileNPCsList.clear();

	if(!m_sightedFriendlyNPCsList.empty())
		m_sightedFriendlyNPCsList.clear();

	if (!m_partiallySightedHostileNPCsList.empty())
		m_partiallySightedHostileNPCsList.clear();

	// Only see enemies if prisoner flag is not set
	if(!HasSpawnFlag(FL_NPC_PRISONER))
	{
		Vector mins;
		Vector maxs;

		Vector eyePosition = GetEyePosition();
		for(Uint32 i = 0; i < 3; i++)
		{
			mins[i] = eyePosition[i] - m_lookDistance;
			maxs[i] = eyePosition[i] + m_lookDistance;
		}

		edict_t* pedict = nullptr;
		while(true)
		{
			pedict = Util::FindEntityInBBox(pedict, mins, maxs);
			if(!pedict)
				break;

			if(Util::IsNullEntity(pedict))
				continue;

			// Only bother with player or NPC entities
			CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
			if(!pEntity || pEntity == this || !pEntity->IsNPC() && !pEntity->IsPlayer() 
				|| (pEntity->GetFlags() & EF_NODRAW) || !pEntity->HasModelName())
				continue;

			// Don't bother with prisoner NPCs, or dead ones
			if((pEntity->HasSpawnFlag(FL_NPC_PRISONER) && !IsEnemyOf(pEntity)) || !pEntity->IsAlive())
				continue;

			// Don't see through water boundaries
			if(m_pState->waterlevel == WATERLEVEL_FULL && pEntity->GetWaterLevel() != WATERLEVEL_FULL
				|| m_pState->waterlevel != WATERLEVEL_FULL && pEntity->GetWaterLevel() == WATERLEVEL_FULL)
				continue;

			// Don't bother with notarget clients, or friendlies
			Int32 relationship = GetRelationship(pEntity);
			if(pEntity->GetFlags() & FL_NOTARGET)
				continue;

			// Don't bother if outside view
			if(!IsInView(pEntity))
				continue;

			// We can't rely on bbox alone
			Float linearDistance = (pEntity->GetEyePosition() - GetEyePosition()).Length();
			if(linearDistance > m_lookDistance)
				continue;

			// Get visibility bits
			enemyawareness_t* pPartialAwareness = nullptr;
			Uint64 visibilityBits = GetNPCVisibilityBits(pEntity, false, &pPartialAwareness);
			if ((visibilityBits & AI_SIGHTED_PLAYER_LEAN))
			{
				// Add to partially aware NPC list
				m_partiallySightedHostileNPCsList.add(pEntity);
				
				// Update awareness stats
				UpdatePartialAwareness(pPartialAwareness, visibilityBits);
			}

			// Let child classes update this
			enemyawareness_t* pEnemyAwareness = nullptr;
			if(relationship != R_ALLY && relationship != R_NONE && relationship != R_FRIEND)
			{
				// Only call this on non-friendlies
				UpdateAwareness(pEntity, pPartialAwareness, &pEnemyAwareness, visibilityBits);

				sightBits |= visibilityBits;
				if(!ShouldSeeNPC(visibilityBits, pEntity, pPartialAwareness, pEnemyAwareness))
					continue;
			}
			else
			{
				// Call non-derived function for friendlies
				if(!CBaseNPC::ShouldSeeNPC(visibilityBits, pEntity, pPartialAwareness, pEnemyAwareness))
					continue;
			}

			// Add to linked list
			if(relationship != R_ALLY && relationship != R_NONE && relationship != R_FRIEND)
			{
				m_sightedHostileNPCsList.add(pEntity);
				sightConditions.set(AI_COND_SEE_HOSTILE_NPC);
			}
			else
				m_sightedFriendlyNPCsList.add(pEntity);

			// Update sighting time for enemy on squad leader
			CBaseEntity* pLeader = GetSquadLeader();
			if(pLeader && pLeader != this && pLeader->GetEnemy() == pEntity)
				pLeader->SetLastEnemySightTime(g_pGameVars->time);

			if (pEntity->IsPlayer())
			{
				// Remember if we saw a player
				sightConditions.set(AI_COND_SEE_CLIENT);
			}
			
			// Check if we saw our enemy
			if(pEntity == m_enemy)
				sightConditions.set(AI_COND_SEE_ENEMY);

			// Add flags based on relation
			switch(relationship)
			{
			case R_NEMESIS:
				sightConditions.set(AI_COND_SEE_NEMESIS);
				break;
			case R_HATE:
				sightConditions.set(AI_COND_SEE_HATE);
				break;
			case R_DISLIKE:
				sightConditions.set(AI_COND_SEE_DISLIKE);
				break;
			case R_ALLY:
				break;
			default:
				{
					if (GetClassification() != CLASS_NONE && pEntity->GetClassification() != CLASS_NONE)
						Util::EntityConPrintf(m_pEdict, "Couldn't assess '%s'.\n", pEntity->GetClassName());
				}
				break;
			}
		}
	}

	// Set the conditions based on what we saw
	SetConditions(sightConditions);

	// See if we can clear any awareness infos from the entity
	if (!m_enemyPartialAwarenessList.empty())
	{
		m_enemyPartialAwarenessList.begin();
		while (!m_enemyPartialAwarenessList.end())
		{
			enemyawareness_t& awareness = m_enemyPartialAwarenessList.get();
			if (!awareness.entity)
			{
				m_enemyPartialAwarenessList.remove(m_enemyPartialAwarenessList.get_link());
				m_enemyPartialAwarenessList.next();
				continue;
			}

			if (!IsAwareOf(awareness.entity) && (awareness.lastsighttime + NPC_LEANAWARENESS_TIMEOUT) < g_pGameVars->time)
			{
				if (awareness.entity->IsPlayer())
					awareness.entity->SetNPCAwareness(0, this, 0, false);

				m_enemyPartialAwarenessList.remove(m_enemyPartialAwarenessList.get_link());
			}

			m_enemyPartialAwarenessList.next();
		}
	}
}

//=============================================
// @brief
//
//=============================================
Float CBaseNPC::GetFiringCoverage( const Vector& shootOrigin, const Vector& targetPosition, const Vector& firingCone )
{
	trace_t tr;
	Util::TraceLine(shootOrigin, targetPosition, true, true, true, m_pEdict, tr);

	// If the center doesn't hit, we won't bother
	if(!tr.noHit())
	{
		CBaseEntity* pHit = Util::GetEntityFromTrace(tr);
		if(pHit && !pHit->IsFuncBreakableEntity())
			return 0;
	}

	// Determine how much of the firing cone is not blocked
	// by any nearby occluders
	Float coverage = 1.0f/5.0f;
	Vector direction = (targetPosition - shootOrigin).Normalize();
	Float distance = (targetPosition - shootOrigin).Length();

	// Precalculate offsets
	const Vector offsets[] = { 
		Vector(firingCone[0], 0, 0),
		Vector(-firingCone[0], 0, 0),
		Vector(0, firingCone[1], 0),
		Vector(0, -firingCone[1], 0)
	};

	for(int i = 0; i < 4; i++)
	{
		// Estimate offset position
		Vector vecDest = shootOrigin + (direction + offsets[i])*distance;
		Util::TraceLine(shootOrigin, targetPosition, true, true, true, m_pEdict, tr);

		// Determine how far we travelled
		Double traceDist = tr.fraction * distance;
		if(tr.fraction == 1.0 || traceDist > (distance*0.75))
		{
			coverage += 1.0f/5.0f;
			continue;
		}
		else if( tr.hitentity != NO_ENTITY_INDEX )
		{
			CBaseEntity* pHit = Util::GetEntityFromTrace(tr);
			if(pHit && pHit->IsFuncBreakableEntity())
			{
				coverage += 1.0f/5.0f;
				continue;
			}
		}
	}

	return coverage;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::ClearRoute( void )
{
	// Begin a new route
	NewRoute();

	m_movementGoal = MOVE_GOAL_NONE;
	m_movementActivity = ACT_IDLE;

	if(IsMoving())
		SetIdealActivity(m_movementActivity);

	ClearMemory(AI_MEMORY_MOVE_FAILED);
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::NewRoute( void )
{
	// Clear previous routes
	for(Uint32 i = 0; i < MAX_ROUTE_POINTS; i++)
		m_routePointsArray[i] = route_point_t();

	m_movementGoalEntity.reset();
	m_lastCornerCutOrigin.Clear();
	m_canCutCorners = false;
	m_routePointIndex = 0;
	m_shortcutPathIndex = 0;
	m_firstNodeIndex = NO_POSITION;
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::IsRouteClear( void ) const
{
	return(m_routePointsArray[m_routePointIndex].type == MF_NONE || m_movementGoal == MOVE_GOAL_NONE) ? true : false;
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::RefreshRoute( void )
{
	// Set us up for a new route
	NewRoute();

	bool result = false;
	switch(m_movementGoal)
	{
	case MOVE_GOAL_PATH_CORNER:
		{
			if(m_goalEntity)
			{
				CBaseEntity* pPath = m_goalEntity;

				Uint32 numpoints = 0;
				while(true)
				{
					route_point_t& pt = m_routePointsArray[numpoints];
					numpoints++;

					pt.type = MF_TO_PATH_CORNER;
					pt.position = pPath->GetOrigin();
					pt.nodeindex = NO_POSITION;
				}
			}

			// Set return code as true
			result = true;
		}
		break;
	case MOVE_GOAL_ENEMY:
		{
			result = BuildRoute(m_enemyLastKnownPosition, MF_TO_ENEMY, m_enemy);
		}
		break;
	case MOVE_GOAL_LOCATION:
		{
			result = BuildRoute(m_movementGoalPosition, MF_TO_LOCATION, nullptr);
		}
		break;
	case MOVE_GOAL_TARGET_ENT:
		{
			if(m_targetEntity)
				result = BuildRoute(m_targetEntity->GetNavigablePosition(), MF_TO_TARGETENT, m_targetEntity);
		}
		break;
	case MOVE_GOAL_NODE:
		{
			result = BuildNodeRoute(m_movementGoalPosition);
		}
		break;
	}

	return result;
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::ShouldSimplifyRoute( const Vector& currentPosition, const Vector& testPosition, Int32 routeTypeBits )
{
	// DO NOT allow cutting corners around stairs, that can fuck up shit bad
	if( SDL_fabs(currentPosition.z - testPosition.z) > NPC_STEP_SIZE )
		return false;

	// Remove goal flag
	Int32 _routeTypeBits = routeTypeBits & ~MF_IS_GOAL;

	// Don't simplify detour paths, path corners or triangulated paths
	if (_routeTypeBits & (MF_TO_PATH_CORNER|MF_DONT_SIMPLIFY|MF_DETOUR_PATH|MF_TO_DETOUR|MF_CIRCLE_PATH))
		return false;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::SimplifyRoute( CBaseEntity* pTargetEntity )
{
	// Make sure we have enough room for everything
	static route_point_t resultArray[MAX_ROUTE_POINTS];

	// Count the points we have
	Uint32 pointCount = 0;
	for(Int32 i = m_routePointIndex; i < MAX_ROUTE_POINTS; i++)
	{
		// Check if we reached the end
		if(m_routePointsArray[i].type == MF_NONE)
			break;

		pointCount++;

		// Check if we hit the goal, but only after count was raised
		if(m_routePointsArray[i].type & MF_IS_GOAL)
			break;
	}

	// Return if we have nothing to simplify
	if(pointCount < 2)
		return;

	// Start simplifying the route
	Uint32 outCount = 0;
	Vector startPosition = m_pState->origin;

	Uint32 i = 0;
	for(; i < pointCount-1 && outCount < (MAX_ROUTE_POINTS-1); i++)
	{
		const route_point_t& pt1 = m_routePointsArray[m_routePointIndex+i];

		// Don't eliminate specific route point types
		if(!ShouldSimplifyRoute(startPosition, pt1.position, pt1.type))
		{
			resultArray[outCount] = pt1;
			startPosition = pt1.position;
			outCount++;
			continue;
		}

		// Check move to the route point ahead
		const route_point_t& pt2 = m_routePointsArray[m_routePointIndex+i+1];
		localmove_t moveResult = CheckLocalMove(startPosition, pt2.position, pTargetEntity);
		if(moveResult > LOCAL_MOVE_RESULT_FAILURE)
		{
			// There is a valid shortcut here, so skip
			continue;
		}
		else
		{
			resultArray[outCount] = pt1;
			startPosition = pt1.position;
			outCount++;
		}
	}

	resultArray[outCount] = m_routePointsArray[m_routePointIndex+i];
	resultArray[outCount+1].type = MF_NONE;
	outCount++;

	// Copy the simplified route
	for(i = 0; i < MAX_ROUTE_POINTS && i < outCount; i++)
		m_routePointsArray[i] = resultArray[i];

	// Terminate the route
	if(i < MAX_ROUTE_POINTS)
		m_routePointsArray[i].type = MF_NONE;
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::MoveToLocation( activity_t moveActivity, Float waitTime, const Vector& goalPosition )
{
	m_movementActivity = moveActivity;
	m_moveWaitTime = waitTime;
	m_movementGoal = MOVE_GOAL_LOCATION;
	m_movementGoalPosition = goalPosition;

	return RefreshRoute();
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::MoveToNode( activity_t moveActivity, Float waitTime, const Vector& goalPosition )
{
	m_movementActivity = moveActivity;
	m_moveWaitTime = waitTime;
	m_movementGoal = MOVE_GOAL_NODE;
	m_movementGoalPosition = goalPosition;

	return RefreshRoute();
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::MoveToTarget( activity_t moveActivity, Float waitTime )
{
	m_movementActivity = moveActivity;
	m_moveWaitTime = waitTime;
	m_movementGoal = MOVE_GOAL_TARGET_ENT;
	
	return RefreshRoute();
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::CheckAttacks( CBaseEntity* pTargetEntity, Float distance )
{
	if(pTargetEntity->GetFlags() & FL_NOTARGET)
		return;

	Vector forward;
	Math::AngleVectors(m_pState->angles, &forward);

	Vector centerOffset = pTargetEntity->GetCenter() - pTargetEntity->GetNavigablePosition();
	Vector enemyCenter = m_enemyLastKnownPosition + centerOffset;

	Vector dirToEnemy = (enemyCenter - GetCenter());
	dirToEnemy[2] = 0;
	dirToEnemy.Normalize();

	forward[2] = 0;
	forward.Normalize();

	Float dp = Math::DotProduct(dirToEnemy, forward);

	// Check range attack 1
	if(HasCapability(AI_CAP_RANGE_ATTACK1) || HasCapability(AI_CAP_RANGE_ATTACK2))
	{
		// Check for friendly fire
		if(!CheckFriendlyFire())
			SetCondition(AI_COND_FRIENDLY_FIRE);

		// Do a cheap check first
		if(FavorRangedAttacks(pTargetEntity)
			&& IsInView(pTargetEntity->GetEyePosition()) 
			&& IsEnemyShootable((*pTargetEntity), false, true))
			SetCondition(AI_COND_SHOOT_VECTOR_VALID);

		if(HasCapability(AI_CAP_RANGE_ATTACK1))
		{
			if(CheckRangeAttack1(dp, distance))
				SetCondition(AI_COND_CAN_RANGE_ATTACK1);
		}

		// Check range attack 2
		if(HasCapability(AI_CAP_RANGE_ATTACK2))
		{
			if(CheckRangeAttack2(dp, distance))
				SetCondition(AI_COND_CAN_RANGE_ATTACK2);
		}
	}

	// Check melee attack 1
	if(HasCapability(AI_CAP_MELEE_ATTACK1))
	{
		if(CheckMeleeAttack1(dp, distance))
			SetCondition(AI_COND_CAN_MELEE_ATTACK1);
	}

	// Check melee attack 2
	if(HasCapability(AI_CAP_MELEE_ATTACK2))
	{
		if(CheckMeleeAttack2(dp, distance))
			SetCondition(AI_COND_CAN_MELEE_ATTACK2);
	}

	// Check special attack 1
	if(HasCapability(AI_CAP_SPECIAL_ATTACK1))
	{
		if(CheckSpecialAttack1(dp, distance))
			SetCondition(AI_COND_CAN_SPECIAL_ATTACK1);
	}

	// Check special attack 2
	if(HasCapability(AI_CAP_SPECIAL_ATTACK2))
	{
		if(CheckSpecialAttack2(dp, distance))
			SetCondition(AI_COND_CAN_SPECIAL_ATTACK2);
	}
}

//=============================================
// @brief Checks if we are likely to shoot a friendly NPC
//
//=============================================
bool CBaseNPC::CheckFriendlyFire( void )
{
	if(!m_enemy)
		return true;

	Vector gunPosition = GetGunPosition();
	Vector enemyTargetPosition = GetIdealEnemyBodyTarget(gunPosition);

	CBaseEntity* pSquadLeader = GetSquadLeader();
	if(!pSquadLeader)
	{
		Util::EntityConDPrintf(m_pEdict, "%s - Squad leader was null.\n", __FUNCTION__);
		return true;
	}

	// Go through the visible npcs list
	m_sightedFriendlyNPCsList.begin();
	while(!m_sightedFriendlyNPCsList.end())
	{
		CBaseEntity* pEntity = m_sightedFriendlyNPCsList.get();
		m_sightedFriendlyNPCsList.next();

		// Only if entity is valid or still alive
		if(!pEntity || !pEntity->IsAlive())
			continue;

		// Only check for non-hostile entities
		Int32 relation = GetRelationship(pEntity);
		if(relation != R_ALLY && relation != R_NONE && relation != R_FRIEND)
			continue;

		trace_t tr;
		gd_tracefuncs.pfnTraceModel(pEntity->GetEntityIndex(), gunPosition, enemyTargetPosition, HULL_POINT, FL_TRACE_NORMAL, tr);
		if(!tr.noHit())
			return false;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::CanCheckAttacks( void ) const
{
	if(CheckCondition(AI_COND_SEE_ENEMY) && !CheckCondition(AI_COND_ENEMY_TOO_FAR))
		return true;
	else
		return false;
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::CanRangeAttack( void ) const
{
	if(HasCapability(AI_CAP_RANGE_ATTACK1) || HasCapability(AI_CAP_RANGE_ATTACK2))
		return true;
	else
		return false;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::ForgetPlayer( CBaseEntity* pPlayer )
{
	assert(pPlayer != nullptr);

	// Clear player info
	pPlayer->SetNPCAwareness(0.0, this, NPC_COMBATSTATE_TIMEOUT, false);

	if(pPlayer == m_enemy)
	{
		SetCondition(AI_COND_ENEMY_DEAD);
		ClearCondition(AI_COND_SEE_ENEMY);
		ClearCondition(AI_COND_ENEMY_OCCLUDED);
		ClearCondition(AI_COND_ENEMY_NAVIGABLE);
	}
	else
	{
		for(Uint32 i = 0; i < MAX_BACKED_UP_ENEMIES; i++)
		{
			if(m_backedUpEnemies[i].enemy == reinterpret_cast<const CBaseEntity*>(pPlayer))
			{
				m_backedUpEnemies[i].lastknownangles.Clear();
				m_backedUpEnemies[i].lastknownorigin.Clear();
				m_backedUpEnemies[i].lastsighttime = 0;
				m_backedUpEnemies[i].enemy.reset();
			}
		}
	}

	if (!m_enemyPartialAwarenessList.empty())
	{
		m_enemyPartialAwarenessList.begin();
		while (!m_enemyPartialAwarenessList.end())
		{
			enemyawareness_t& awareness = m_enemyPartialAwarenessList.get();
			if (!awareness.entity)
			{
				m_enemyPartialAwarenessList.remove(m_enemyPartialAwarenessList.get_link());
				m_enemyPartialAwarenessList.next();
				continue;
			}

			if (awareness.entity.get() == pPlayer->GetEdict())
				m_enemyPartialAwarenessList.remove(m_enemyPartialAwarenessList.get_link());

			m_enemyPartialAwarenessList.next();
		}
	}

	m_lastEnemySightTime = 0;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::StartDangerCheck( const Vector& dangerPosition, CBaseEntity* pDangerEntity, bool coverPosition )
{
	m_dangerEntity = pDangerEntity;
	m_dangerOrigin = dangerPosition;
	m_isSeekingCover = coverPosition;

	// Get distance to danger
	m_lastDangerDistance = (m_pState->origin - dangerPosition).Length();
	// Reset this
	m_dangerExposure = 0;

	// Remember this
	SetMemory(AI_MEMORY_CHECKING_DANGERS);
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::ExamineDangers( void )
{
	// Only if we're taking cover, and we're not in danger
	if(!HasMemory(AI_MEMORY_CHECKING_DANGERS) || CheckCondition(AI_COND_IN_DANGER))
		return;

	// If we have an enemy, prefer him
	CBaseEntity* pDangerEntity = NULL;
	if(m_enemy && m_dangerEntity != m_enemy)
		pDangerEntity = m_enemy;
	else
		pDangerEntity = m_dangerEntity;

	// If it's an entity, then update the info
	if(pDangerEntity)
		m_dangerOrigin = pDangerEntity->GetOrigin();

	// Determine current distance and calculate exposure
	Float dangerExposure = 0;
	Float dangerDistance = (m_pState->origin - m_dangerOrigin).Length();

	// If we're getting closer, then the exposure poses danger
	Float flDiff = dangerDistance - m_lastDangerDistance;
	if(flDiff < 0)
	{
		if(dangerDistance < 1024)
			dangerExposure += 0.5;
		else
			dangerExposure += 0.25;
	}
	else if(flDiff > 0)
	{
		dangerExposure -= 0.1;
	}

	// Double if he's facing us
	if(pDangerEntity && pDangerEntity->IsAlive())
	{
		// Determine if the danger source is facing us, and how much
		Vector dangerDir = (m_pState->origin - m_dangerOrigin).Normalize();

		Vector forward;
		Math::AngleVectors(pDangerEntity->GetAngles(), &forward);

		// If our enemy is looking right at us, then get worried
		Float flDot = Math::DotProduct(forward, dangerDir);
		if(flDot > 0.2)
		{
			trace_t tr;
			Util::TraceLine(pDangerEntity->GetEyePosition(true), GetEyePosition(), true, false, m_pEdict, tr);
			if(tr.noHit())
			{
				if(dangerDistance < 1024)
					dangerExposure += 0.25 + (1.0 - flDot) * 0.25;
				else
					dangerExposure += 0.25;
			}
			else
			{
				// Enemy cannot see us, so reduce danger exposure
				dangerExposure -= 0.1;
			}
		}
		else if(flDot < -0.2)
		{
			// Enemy is not facing us, so reduce danger exposure
			dangerExposure -= 0.1;
		}
	}

	// Reduce this when not seeking cover
	if(!m_isSeekingCover)
		dangerExposure *= 0.5;

	if(!dangerExposure)
		return;

	m_dangerExposure += dangerExposure * m_thinkIntervalTime;
	if(m_dangerExposure < 0)
		m_dangerExposure = 0;

	if(m_dangerExposure >= NPC_MAX_DANGER_TIME)
	{
		ClearMemory(AI_MEMORY_CHECKING_DANGERS);
		SetCondition(AI_COND_IN_DANGER);
	}
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::FindCoverWithBestDistance( const Vector& threatPosition, Float minDistance, float maxDistance, Float optimalDistance, Vector& outPosition )
{
	Float traveledDist = 0;
	Float bestDistance = 0;
	Vector bestCoverSpot;

	// Determine ideal escape vector
	Vector threatAngle;
	Vector vecFromThreat = m_pState->origin - Vector(threatPosition.x, threatPosition.y, m_pState->origin.z);
	Float distToThreat = vecFromThreat.Length();
	vecFromThreat.Normalize();

	// Ideal direction is directly away from the threat
	threatAngle = Math::VectorToAngles(vecFromThreat);

	// Begin searching in circles
	Vector forward;
	for(Int32 i = 0; i < 36; i++)
	{
		Math::AngleVectors(threatAngle, &forward);

		// Don't go running into it if we're far
		if(Math::DotProduct(vecFromThreat, forward) > 0.5 && distToThreat > 128)
		{
			threatAngle[YAW] += 10;
			continue;
		}

		Vector arriveSpot;
		WalkMoveTrace( m_pState->origin, forward, arriveSpot, maxDistance, traveledDist );
		if(traveledDist > bestDistance)
		{
			bestCoverSpot = arriveSpot;
			bestDistance = traveledDist;
		}

		if(traveledDist >= optimalDistance && CheckLocalMove(m_pState->origin, arriveSpot, nullptr, nullptr) > LOCAL_MOVE_RESULT_FAILURE)
			break;

		// Search by ten degrees
		threatAngle[YAW] += 10;
	}

	if(bestDistance > minDistance)
	{
		outPosition = bestCoverSpot;
		return true;
	}
	else
	{
		outPosition.Clear();
		return false;
	}
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::FindDodgeCover( const Vector& threatPosition, Float minDistance, float maxDistance, Vector& outPosition )
{
	Float traveledDist = 0;
	Float bestDistance = 0;
	Vector bestCoverSpot = Vector(0, 0, 0);

	// Determine ideal escape vector
	Vector threatAngle;
	Vector vecFromThreat = m_pState->origin - Vector(threatPosition.x, threatPosition.y, m_pState->origin.z);
	Float distToThreat = vecFromThreat.Length();
	vecFromThreat.Normalize();

	// Ideal direction is directly away from the threat
	threatAngle = Math::VectorToAngles(vecFromThreat);

	// Begin searching in circles
	Vector vRight;
	for(Int32 i = 0; i < 36; i++)
	{
		Vector arriveSpot;
		Math::AngleVectors(threatAngle, nullptr, &vRight);

		// Try one side first
		{
			// Don't go running into it if we're far
			if(Math::DotProduct(vecFromThreat, vRight) > 0.5 && distToThreat > 128)
			{
				threatAngle[YAW] += 10;
				continue;
			}

			WalkMoveTrace( m_pState->origin, vRight, arriveSpot, maxDistance, traveledDist );
			if(traveledDist > bestDistance)
			{
				bestCoverSpot = arriveSpot;
				bestDistance = traveledDist;
			}
		}

		// Try the other next
		{
			// Don't go running into it if we're far
			if(Math::DotProduct(vecFromThreat, -vRight) > 0.5 && distToThreat > 128)
			{
				threatAngle[YAW] += 10;
				continue;
			}

			WalkMoveTrace( m_pState->origin, vRight, arriveSpot, maxDistance, traveledDist );
			if(traveledDist > bestDistance)
			{
				bestCoverSpot = arriveSpot;
				bestDistance = traveledDist;
			}
		}

		if(traveledDist >= 64 && CheckLocalMove(m_pState->origin, arriveSpot, nullptr, nullptr) > LOCAL_MOVE_RESULT_FAILURE)
			break;

		// Search by ten degrees
		threatAngle[YAW] += 10;
	}

	if(bestDistance > minDistance)
	{
		outPosition = bestCoverSpot;
		return true;
	}
	else
	{
		outPosition.Clear();
		return false;
	}
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::FindCover( const Vector& threatPosition, const Vector& viewOffset, Float minDistance, Float maxDistance, CBaseEntity* pThreatEntity )
{
	if(!gNodeGraph.IsNodeGraphValid())
	{
		gd_engfuncs.pfnCon_Printf("AI Graph not initialized.\n");
		return false;
	}

	// Make sure max distance is valid
	Float _maxDistance = maxDistance;
	if(!_maxDistance)
		_maxDistance = NPC_MAX_NAVIGATION_DISTANCE;

	// Set min distance also if not valid
	Float _minDistance = minDistance;
	if(_minDistance > _maxDistance * 0.5)
		_minDistance = _maxDistance * 0.5;

	// Get the hull type for this NPC
	node_hull_types_t hullType = Util::GetNodeHullForNPC(this);
	// Get the node type
	Uint64 nodeType = Util::GetNodeTypeForNPC(this);
	// Get the node we're near to
	Int32 startNode = gNodeGraph.GetNearestNode(m_pState->origin, nodeType, this);
	if(startNode == NO_POSITION)
	{
		Util::EntityConDPrintf(m_pEdict, "%s - No nearest node.\n", __FUNCTION__);
		return false;
	}

	// Get threat's node
	Int32 threatNode = gNodeGraph.GetNearestNode(threatPosition, pThreatEntity ? pThreatEntity : this);
	if(threatNode == NO_POSITION)
		threatNode = startNode;

	// Get the looker's offset
	Vector lookerEyePosition = threatPosition + viewOffset;

	// If possible, factor in the threat
	if(pThreatEntity && pThreatEntity->IsNPCDangerous())
	{
		// Check if we're outdoors
		Vector skyOffset = m_pState->origin + Vector(0, 0, 4096);

		trace_t tr;
		Util::TraceLine(m_pState->origin, skyOffset, true, false, m_pEdict, tr);

		bool isThreatOutdoors = (gd_tracefuncs.pfnPointContents(tr.endpos, nullptr, false) == CONTENTS_SKY) ? true : false;

		Int32 nodeIndex = NO_POSITION;
		for(Int32 i = 0; i < gNodeGraph.GetNumNodes(); i++)
		{
			// Get the node number
			nodeIndex = (g_lastCoverSearchNodeIndex + i) % gNodeGraph.GetNumNodes();
			const CAINodeGraph::node_t* pNode = gNodeGraph.GetNode(nodeIndex);
			if(!pNode)
				continue;

			Float distance = (m_pState->origin - pNode->origin).Length();
			if(distance < _minDistance)
				continue;

			Vector testPosition = pNode->origin + m_pState->view_offset;
			Util::TraceLine(testPosition, lookerEyePosition, true, false, m_pEdict, tr);
			if(tr.noHit())
			{
				// Avoid nodes in view of our threat
				if(pThreatEntity->IsInView(pNode->origin))
					continue;
			}

			skyOffset = pNode->origin + Vector(0, 0, 4096);
			Util::TraceLine(pNode->origin, skyOffset, true, false, m_pEdict, tr);
			bool isNodeOutdoors = gd_tracefuncs.pfnPointContents(tr.endpos, nullptr, false) == CONTENTS_SKY ? true : false;
			if(isNodeOutdoors && isThreatOutdoors || !isNodeOutdoors && !isThreatOutdoors)
				continue;

			if(startNode == threatNode)
			{
				if(ValidateCover(pNode->origin) && gNodeGraph.IsValidCoverPath(startNode, nodeIndex, hullType, m_capabilityBits, this, pThreatEntity) && MoveToLocation(ACT_RUN, 0, pNode->origin))
					return true;
			}
			else
			{
				Float distanceToCover = gNodeGraph.GetPathLength(startNode, nodeIndex, hullType, m_capabilityBits);
				Float threatDistance = gNodeGraph.GetPathLength(threatNode, nodeIndex, hullType, m_capabilityBits);

				if(distanceToCover <= threatDistance)
				{
					if(ValidateCover(pNode->origin) && gNodeGraph.IsValidCoverPath(startNode, nodeIndex, hullType, m_capabilityBits, this, pThreatEntity) && MoveToLocation(ACT_RUN, 0, pNode->origin))
						return true;
				}
			}
		}

		// Update last node
		if(nodeIndex != NO_POSITION)
			g_lastCoverSearchNodeIndex = nodeIndex + 1;
	}

	// Just look for a node
	Int32 nodeIndex = NO_POSITION;
	for(Int32 i = 0; i < gNodeGraph.GetNumNodes(); i++)
	{
		// Get the node number
		nodeIndex = (g_lastCoverSearchNodeIndex + i) % gNodeGraph.GetNumNodes();
		const CAINodeGraph::node_t* pNode = gNodeGraph.GetNode(nodeIndex);
		if(!pNode)
			continue;

		Float distance = (m_pState->origin - pNode->origin).Length();
		if(distance < _minDistance || distance > _maxDistance)
			continue;

		// See if the node will be blocked
		Vector testPosition = pNode->origin + m_pState->view_offset;

		trace_t tr;
		Util::TraceLine(testPosition, lookerEyePosition, true, false, m_pEdict, tr);
		if(tr.noHit())
		{
			// See if our threat sees it
			if(pThreatEntity && pThreatEntity->IsInView(pNode->origin))
				continue;
		}

		Float distanceToCover = gNodeGraph.GetPathLength(startNode, nodeIndex, hullType, m_capabilityBits);
		Float threatDistance = gNodeGraph.GetPathLength(threatNode, nodeIndex, hullType, m_capabilityBits);

		if(distanceToCover <= threatDistance)
		{
			if(ValidateCover(pNode->origin) && gNodeGraph.IsValidCoverPath(startNode, nodeIndex, hullType, m_capabilityBits, this, nullptr) && MoveToLocation(ACT_RUN, 0, pNode->origin))
				return true;
		}
	}

	// Update last node
	if(nodeIndex != NO_POSITION)
		g_lastCoverSearchNodeIndex = nodeIndex + 1;

	return false;
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::FindLateralCover( const Vector& threatPosition, const Vector& viewOffset )
{
	trace_t tr;

	Vector forward, right;
	Math::AngleVectors(m_pState->angles, &forward, &right);

	Vector stepRight = right * NPC_LATERAL_COVER_CHECK_DISTANCE;

	Vector testLeft, testRight;
	testLeft = testRight = m_pState->origin;
	Vector threatEyes = threatPosition + viewOffset;

	for(Int32 i = 0; i < NPC_LATERAL_COVER_CHECK_NUM; i++)
	{
		Math::VectorSubtract(testLeft, stepRight, testLeft);
		Math::VectorAdd(testRight, stepRight, testRight);

		// Try left
		Vector myEyesTest = testLeft + m_pState->view_offset;
		Util::TraceLine(threatEyes, myEyesTest, true, false, true, m_pEdict, tr);
		if(!tr.noHit() && ValidateCover(testLeft) 
			&& CheckLocalMove(m_pState->origin, testLeft, nullptr) > LOCAL_MOVE_RESULT_FAILURE 
			&& MoveToLocation(ACT_RUN, 0, testLeft))
		{
			// Cover is valid
			return true;
		}

		// Try right
		myEyesTest = testRight + m_pState->view_offset;
		Util::TraceLine(threatEyes, myEyesTest, true, false, true, m_pEdict, tr);
		if(!tr.noHit() && ValidateCover(testRight) 
			&& CheckLocalMove(m_pState->origin, testRight, nullptr) > LOCAL_MOVE_RESULT_FAILURE 
			&& MoveToLocation(ACT_RUN, 0, testRight))
		{
			// Cover is valid
			return true;
		}
	}

	return false;
}

//=============================================
// @brief
//
//=============================================
Vector CBaseNPC::GetIdealEnemyBodyTarget( const Vector& shootOrigin )
{
	Vector enemyBodyTarget;
	switch(m_enemyBodyTarget)
	{
	case BODYTARGET_CENTER:
		enemyBodyTarget = m_enemy->GetBodyTarget(shootOrigin);
		break;
	case BODYTARGET_HEAD:
		enemyBodyTarget = m_enemy->GetEyePosition();
		break;
	case BODYTARGET_LEGS:
		enemyBodyTarget = m_enemy->GetNavigablePosition() * 0.75 + m_enemy->GetBodyTarget(shootOrigin) * 0.25;
	}

	return enemyBodyTarget;
}

//=============================================
// @brief
//
//=============================================
Vector CBaseNPC::GetShootVector( const Vector& shootOrigin )
{
	Vector shootForward;
	if(m_enemy)
	{
		// Position our firing direction at the enemy's last known position
		Vector enemyBodyTarget = GetIdealEnemyBodyTarget(shootOrigin);
		Vector enemyBodyTargetOffset = enemyBodyTarget - m_enemy->GetNavigablePosition();
		Vector enemyLKPPosition = m_enemyLastKnownPosition + enemyBodyTargetOffset;
		shootForward = (enemyLKPPosition - shootOrigin).Normalize();
	}
	else
	{
		// Firing direction is directly forward if no enemy
		Math::AngleVectors(m_pState->angles, &shootForward);
	}

	return shootForward;
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::ValidateCover( const Vector& coverPosition )
{
	return true;
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::IsInDanger( void )
{
	// If our enemy is dangerous, it means we're in danger
	if(m_enemy->IsNPCDangerous())
		return true;

	// If one of our backed up enemies is dangerous
	for(Uint32 i = 0; i < MAX_BACKED_UP_ENEMIES; i++)
	{
		if(m_backedUpEnemies[i].enemy && m_backedUpEnemies[i].enemy->IsNPCDangerous())
			return true;
	}

	return false;
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::GetLateralShootingPosition( const Vector& enemyPosition )
{
	// Don't bother checking if they're too far out
	if((enemyPosition - m_pState->origin).Length() > m_firingDistance)
		return false;

	trace_t tr;
	Vector direction = (enemyPosition - m_pState->origin).Normalize();
	Vector angles = Math::VectorToAngles(direction);

	Vector right;
	Math::AngleVectors(angles, nullptr, &right);

	// Try finding a crouching position first
	if(CanCrouch())
	{
		for(Float distance = 64; distance >= 512; distance += 64)
		{
			Vector testPosition = m_pState->origin + right*distance;
			Vector testViewPosition = testPosition + (GetGunPosition(STANCE_CROUCHING) - m_pState->origin);

			// can I see where I want to be from there?
			Util::TraceLine(testViewPosition, enemyPosition, true, false, m_pEdict, tr);
			if( tr.noHit() && !tr.startSolid() && !tr.allSolid() )
			{
				localmove_t localMoveResult = CheckLocalMove(m_pState->origin, testPosition, nullptr, nullptr);
				if( localMoveResult > LOCAL_MOVE_RESULT_FAILURE )
				{
					if(BuildRoute(testPosition, MF_TO_LOCATION, nullptr))
					{
						m_movementGoalPosition = testPosition;
						return true;
					}
				}
			}

			// Look to the left then
			testPosition = m_pState->origin - right*distance;
			testViewPosition = testPosition + (GetGunPosition(STANCE_CROUCHING) - m_pState->origin);

			// can I see where I want to be from there?
			Util::TraceLine(testViewPosition, enemyPosition, true, false, m_pEdict, tr);
			if( tr.noHit() && !tr.startSolid() && !tr.allSolid() )
			{
				localmove_t localMoveResult = CheckLocalMove(m_pState->origin, testPosition, nullptr, nullptr);
				if( localMoveResult > LOCAL_MOVE_RESULT_FAILURE )
				{
					if(BuildRoute(testPosition, MF_TO_LOCATION, nullptr))
					{
						m_movementGoalPosition = testPosition;
						return true;
					}
				}
			}
		}
	}

	for(Float distance = 64; distance >= 512; distance += 64)
	{
		Vector testPosition = m_pState->origin + right*distance;
		Vector testViewPosition = testPosition + (GetGunPosition(STANCE_STANDING) - m_pState->origin);

		// can I see where I want to be from there?
		Util::TraceLine(testViewPosition, enemyPosition, true, false, m_pEdict, tr);
		if( tr.noHit() && !tr.startSolid() && !tr.allSolid() )
		{
			localmove_t localMoveResult = CheckLocalMove(m_pState->origin, testPosition, nullptr, nullptr);
			if( localMoveResult > LOCAL_MOVE_RESULT_FAILURE )
			{
				if(BuildRoute(testPosition, MF_TO_LOCATION, nullptr))
				{
					m_movementGoalPosition = testPosition;
					return true;
				}
			}
		}

		// Look to the left then
		testPosition = m_pState->origin - right*distance;
		testViewPosition = testPosition + (GetGunPosition(STANCE_STANDING) - m_pState->origin);

		// can I see where I want to be from there?
		Util::TraceLine(testViewPosition, enemyPosition, true, false, m_pEdict, tr);
		if( tr.noHit() && !tr.startSolid() && !tr.allSolid() )
		{
			localmove_t localMoveResult = CheckLocalMove(m_pState->origin, testPosition, nullptr, nullptr);
			if( localMoveResult > LOCAL_MOVE_RESULT_FAILURE )
			{
				if(BuildRoute(testPosition, MF_TO_LOCATION, nullptr))
				{
					m_movementGoalPosition = testPosition;
					return true;
				}
			}
		}
	}

	return false;
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::GetClosestShootingPosition( const Vector& enemyPosition )
{
	trace_t tr;

	Vector walkPosition;
	Vector threatDirection = (enemyPosition-m_pState->origin).Normalize();
	Vector threatAngle = Math::VectorToAngles(threatDirection);

	Vector right, forward;
	Math::AngleVectors(threatAngle, &forward, &right);

	// Get distance to enemy
	Float distance = (enemyPosition - m_pState->origin).Length2D();

	// Try going as far as you can
	for(Int32 i = 0; i < 2; i++)
	{
		for(Float fl = 0; fl < 0.5; fl += 0.1)
		{
			// Try to find a position from forward to side from where we can shoot him
			Vector vSide = (i == 0) ? right : -right;
			Vector vecDir = forward * (1.0 - fl) + vSide * fl;

			Float walkDistance = 0;
			WalkMoveTrace(m_pState->origin, vecDir, walkPosition, distance, walkDistance);
			if(walkDistance > 16)
			{
				// Test the new position
				Vector testPosition = walkPosition + (GetGunPosition(STANCE_STANDING) - m_pState->origin);
				
				Util::TraceLine(testPosition, enemyPosition, true, false, m_pEdict, tr);
				if(tr.noHit() && !tr.allSolid() && !tr.startSolid())
				{
					if(BuildRoute(walkPosition, MF_TO_LOCATION, NULL))
					{
						m_movementGoalPosition = walkPosition;
						return true;
					}
				}
			}
		}
	}

	return false;
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::BuildNearestVisibleRoute( const Vector& destination, const Vector& viewOffset, Float minDistance, Float maxDistance )
{
	if(!gNodeGraph.IsNodeGraphValid())
	{
		gd_engfuncs.pfnCon_Printf("AI Graph not initialized.\n");
		return false;
	}

	Float _maxDistance = maxDistance;
	if(!_maxDistance)
		_maxDistance = NPC_MAX_NAVIGATION_DISTANCE;

	Float _minDistance = minDistance;
	if(_minDistance > maxDistance * 0.5)
		_minDistance = maxDistance * 0.5;

	// Get the hull type for this NPC
	node_hull_types_t hullType = Util::GetNodeHullForNPC(this);
	// Get the node type
	Uint64 nodeType = Util::GetNodeTypeForNPC(this);
	// Get the node we're near to
	Int32 startNode = gNodeGraph.GetNearestNode(m_pState->origin, nodeType, this);
	if(startNode == NO_POSITION)
	{
		Util::EntityConDPrintf(m_pEdict, "%s - No nearest node.\n", __FUNCTION__);
		return false;
	}

	m_firstNodeIndex = startNode;
	Vector lookerOffset = destination + viewOffset;

	// Closest node index
	Int32 closestNodeIndex = NO_POSITION;

	// Get the capability index we'll use
	CAINodeGraph::capability_indexes_t capIndex = gNodeGraph.GetCapabilityIndex(m_capabilityBits);

	// Favor a crouching position
	if(CanCrouch())
	{
		for(Int32 i = 0; i < gNodeGraph.GetNumNodes(); i++)
		{
			// Get the node number
			Int32 nodeIndex = (g_lastCoverSearchNodeIndex + i) % gNodeGraph.GetNumNodes();
			const CAINodeGraph::node_t* pNode = gNodeGraph.GetNode(nodeIndex);
			if(!pNode)
				continue;

			if(gNodeGraph.GetNextNodeInRoute(startNode, nodeIndex, hullType, capIndex) != startNode)
			{
				// See if we can get there
				Float distance = (destination - pNode->origin).Length();
				if(distance > _minDistance && distance < _maxDistance)
				{
					// Check if we can see our target
					Vector testPosition = pNode->origin + (GetGunPosition(STANCE_CROUCHING) - m_pState->origin);
					
					trace_t tr;
					Util::TraceLine(testPosition, lookerOffset, true, false, m_pEdict, tr);
					if(tr.noHit() && !tr.startSolid() && CheckRoute(m_pState->origin, pNode->origin, nullptr))
					{
						closestNodeIndex = nodeIndex;
						_maxDistance = distance;
					}
				}
			}
		}
	}

	if(closestNodeIndex != NO_POSITION)
	{
		for(Int32 i = 0; i < gNodeGraph.GetNumNodes(); i++)
		{
			// Get the node number
			Int32 nodeIndex = (g_lastCoverSearchNodeIndex + i) % gNodeGraph.GetNumNodes();
			const CAINodeGraph::node_t* pNode = gNodeGraph.GetNode(nodeIndex);
			if(!pNode)
				continue;

			if(gNodeGraph.GetNextNodeInRoute(startNode, nodeIndex, hullType, capIndex) != startNode)
			{
				// See if we can get there
				Float distance = (destination - pNode->origin).Length();
				if(distance > _minDistance && distance < _maxDistance)
				{
					// Check if we can see our target
					Vector testPosition = pNode->origin + (GetGunPosition(STANCE_CROUCHING) - m_pState->origin);

					trace_t tr;
					Util::TraceLine(testPosition, lookerOffset, true, false, m_pEdict, tr);
					if(tr.noHit() && !tr.startSolid() && CheckRoute(m_pState->origin, pNode->origin, nullptr))
					{
						closestNodeIndex = nodeIndex;
						_maxDistance = distance;
					}
				}
			}
		}
	}

	if(closestNodeIndex != NO_POSITION)
	{
		// Update this
		g_lastCoverSearchNodeIndex = closestNodeIndex + 1;

		const CAINodeGraph::node_t* pNode = gNodeGraph.GetNode(closestNodeIndex);
		if(!pNode)
			return false;

		if(BuildRoute(pNode->origin, MF_TO_LOCATION, nullptr))
		{
			m_movementGoalPosition = pNode->origin;
			return true;
		}
	}

	return false;
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::BuildNearestRoute( const Vector& destination, Float minDistance, Float maxDistance )
{
	if(!gNodeGraph.IsNodeGraphValid())
	{
		gd_engfuncs.pfnCon_Printf("AI Graph not initialized.\n");
		return false;
	}

	Float _maxDistance = maxDistance;
	if(!_maxDistance)
		_maxDistance = NPC_MAX_NAVIGATION_DISTANCE;

	Float _minDistance = minDistance;
	if(_minDistance > maxDistance * 0.5)
		_minDistance = maxDistance * 0.5;

	// Get the hull type for this NPC
	node_hull_types_t hullType = Util::GetNodeHullForNPC(this);
	// Get the node type
	Uint64 nodeType = Util::GetNodeTypeForNPC(this);
	// Get the node we're near to
	Int32 startNode = gNodeGraph.GetNearestNode(m_pState->origin, nodeType, this);
	if(startNode == NO_POSITION)
	{
		Util::EntityConDPrintf(m_pEdict, "%s - No nearest node.\n", __FUNCTION__);
		return false;
	}

	m_firstNodeIndex = startNode;
	Int32 nearestNodeIndex = NO_POSITION;

	// Get the capability index we'll use
	CAINodeGraph::capability_indexes_t capIndex = gNodeGraph.GetCapabilityIndex(m_capabilityBits);

	for(Int32 i = 0; i < gNodeGraph.GetNumNodes(); i++)
	{
		// Get the node number
		Int32 nodeIndex = (g_lastCoverSearchNodeIndex + i) % gNodeGraph.GetNumNodes();
		const CAINodeGraph::node_t* pNode = gNodeGraph.GetNode(nodeIndex);
		if(!pNode)
			continue;

		if(gNodeGraph.GetNextNodeInRoute(startNode, nodeIndex, hullType, capIndex) != startNode)
		{
			// See if we can get there
			Float distance = (destination - pNode->origin).Length();
			if(distance > _minDistance && distance < _maxDistance)
			{
				if(BuildRoute(pNode->origin, MF_TO_LOCATION, nullptr))
				{
					_maxDistance = distance;
					nearestNodeIndex = i;
				}
			}
		}
	}

	if(nearestNodeIndex != NO_POSITION)
	{
		// Update last node
		g_lastCoverSearchNodeIndex = nearestNodeIndex + 1;

		const CAINodeGraph::node_t* pNode = gNodeGraph.GetNode(nearestNodeIndex);
		if(!pNode)
			return false;

		if(BuildRoute(pNode->origin, MF_TO_LOCATION, nullptr))
		{
			m_movementGoalPosition = pNode->origin;
			return true;
		}
	}

	return false;
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::IsEnemyShootable( CBaseEntity& enemy, bool ignoreGlass, bool ignoreBreakables, stance_t stance )
{
	// Don't shoot invisible
	if(!enemy.IsVisible() && enemy.GetSolidity() == SOLID_NOT)
		return false;

	// Try hitting center first
	m_enemyBodyTarget = BODYTARGET_CENTER;
	Vector gunPosition = GetGunPosition(stance);
	Vector targetPosition = enemy.GetBodyTarget(gunPosition);

	if(IsEnemyBodyTargetShootable(enemy, ignoreGlass, ignoreBreakables, gunPosition, targetPosition))
		return true;

	// Try hitting head
	m_enemyBodyTarget = BODYTARGET_HEAD;
	targetPosition = enemy.GetEyePosition();

	if(IsEnemyBodyTargetShootable(enemy, ignoreGlass, ignoreBreakables, gunPosition, targetPosition))
		return true;

	m_enemyBodyTarget = BODYTARGET_LEGS;
	Vector feetPosition =  enemy.GetNavigablePosition();
	Vector bodyCenter = enemy.GetBodyTarget(gunPosition);
	targetPosition = feetPosition * 0.75 + bodyCenter * 0.25;

	return IsEnemyBodyTargetShootable(enemy, ignoreGlass, ignoreBreakables, gunPosition, targetPosition);
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::CheckMaterialPenetration( CBaseEntity* pHitEntity, const Vector& gunPosition, const Vector& hitPosition, const Vector& hitNormal, Vector& outPosition )
{
	// We check for texture itself to make sure TX_FL_NO_PENETRATION is not set
	const Char* pstrTextureName = Util::TraceTexture(pHitEntity->GetEntityIndex(), hitPosition, hitNormal);
	if(!pstrTextureName)
		return false;

	const en_material_t* pmaterial = gd_engfuncs.pfnGetMapTextureMaterialScript(pstrTextureName);
	if(!pmaterial)
		return false;

	// Check if penetration is allowed
	if(pmaterial->flags & TX_FL_NO_PENETRATION)
		return false;

	CString materialName;
	if(pHitEntity->GetRenderMode() != RENDER_NORMAL
		&& (pHitEntity->GetRenderMode() & RENDERMODE_BITMASK) != RENDER_TRANSALPHA)
	{
		// All transparents are glass
		materialName = GLASS_MATERIAL_TYPE_NAME;
	}
	else
	{
		// Take from material script file
		materialName = pmaterial->materialname;
	}

	// get material definition
	const CMaterialDefinitions::materialdefinition_t* pdefinition = gMaterialDefinitions.GetDefinitionForMaterial(materialName.c_str());
	if(!pdefinition)
		return false;

	// Check for bullet type
	Int32 bulletType = GetBulletType();
	if(bulletType < 0 || bulletType >= NB_BULLET_TYPES)
		return false;

	CMaterialDefinitions::penetration_t penetrationInfo = pdefinition->penetrationinfos[bulletType];
	if(penetrationInfo.penetrationchance > 2)
		return false;

	Vector startPosition = hitPosition;
	Vector direction = startPosition - gunPosition;
	direction.Normalize();

	trace_t tr;
	Float depthOfPenetration = 0;

	for(Float distance = 4.0f; distance <= penetrationInfo.penetrationdepth; distance += 4.0f)
	{
		tr = trace_t();
		Vector testPosition = startPosition + direction*distance;

		gd_tracefuncs.pfnTraceModel(pHitEntity->GetEntityIndex(), testPosition, startPosition, HULL_POINT, (FL_TRACE_NORMAL|FL_TRACE_NO_NPCS), tr);
		if(!tr.startSolid() && !tr.allSolid() && !tr.noHit())
		{
			depthOfPenetration = (startPosition - tr.endpos).Length();
			break;
		}
	}

	if(depthOfPenetration > penetrationInfo.penetrationdepth)
		return false;
					
	Int32 contentsAtExit = gd_tracefuncs.pfnPointContents(tr.endpos, nullptr, false);
	if(contentsAtExit != CONTENTS_EMPTY && contentsAtExit != CONTENTS_WATER
		&& contentsAtExit != CONTENTS_SLIME && contentsAtExit != CONTENTS_LAVA)
		return false;

	outPosition = tr.endpos;
	return true;
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::IsEnemyBodyTargetShootable( CBaseEntity& enemy, bool ignoreGlass, bool ignoreBreakables, const Vector& gunPosition, const Vector& enemyBodyTarget )
{
	trace_t tr;
	Util::TraceLine(gunPosition, enemyBodyTarget, true, false, ignoreGlass, false, m_pEdict, tr);

	if(!tr.noHit() && (ignoreBreakables || !ignoreGlass))
	{
		while(true)
		{
			CBaseEntity* pEntity = Util::GetEntityFromTrace(tr);
			if(!pEntity)
				break;

			// Check separately for penetrable glass
			if(!ignoreGlass && pEntity->GetRenderMode() != RENDER_NORMAL
				&& (pEntity->GetRenderMode() & RENDERMODE_BITMASK) != RENDER_TRANSALPHA
				&& pEntity->GetRenderAmount() > 0)
			{
				Vector startPosition;
				if(!CheckMaterialPenetration(pEntity, gunPosition, tr.endpos, tr.plane.normal, startPosition))
					break;

				// Reset trace
				tr = trace_t();
				Util::TraceLine(startPosition, enemyBodyTarget, true, false, ignoreGlass, false, pEntity->GetEdict(), tr);
				if(tr.noHit())
					break;
			}
			else
			{
				if(!pEntity->IsFuncBreakableEntity())
				{
					tr = trace_t();
					break;
				}

				Util::TraceLine(tr.endpos, enemyBodyTarget, true, false, ignoreGlass, false, pEntity->GetEdict(), tr);
				if(tr.noHit())
					break;
			}
		}
	}

	if(tr.noHit())
		return true;
	else
		return false;
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::CheckFiringWithStance( CBaseEntity& enemy, const Vector& firingCone, bool& shouldStand )
{
	// Do a cheap check first
	if(!IsEnemyShootable(enemy, false, true, STANCE_STANDING))
		return false;

	// Determine how many of the shots might be successful when crouching
	Vector gunCrouchPosition = GetGunPosition(STANCE_CROUCHING);
	Vector targetCrouching = GetIdealEnemyBodyTarget(gunCrouchPosition);
	Float crouchingCoverage = GetFiringCoverage(gunCrouchPosition, targetCrouching, firingCone);
	Float totalCoverage = crouchingCoverage;

	// Don't bother checking if we're good
	if(crouchingCoverage != 1.0)
	{
		// Determine how many of the shots might be successful when standing
		Vector targetStandingPosition = GetGunPosition(STANCE_STANDING);
		Vector targetStanding = GetIdealEnemyBodyTarget(targetStandingPosition);
		Float standingCoverage = GetFiringCoverage(targetStandingPosition, targetStanding, firingCone);
		totalCoverage = (totalCoverage + standingCoverage) / 2.0f;

		// See if we should stand or crouch
		if( crouchingCoverage >= standingCoverage )
			shouldStand = false;
		else
			shouldStand = true;
	}
	else
		shouldStand = false;

	return (totalCoverage > 0) ? true : false;
}

//=============================================
// @brief
//
//=============================================
CBaseEntity* CBaseNPC::GetBestVisibleEnemy( void )
{
	static CBaseEntity* categorizedEnemies[NUM_ENEMY_RELATIONS];
	static Float enemyDistances[NUM_ENEMY_RELATIONS];
	static bool enemyShootable[NUM_ENEMY_RELATIONS];

	// Make sure these arrays are always reset
	memset(categorizedEnemies, 0, sizeof(categorizedEnemies));
	memset(enemyDistances, 0, sizeof(enemyDistances));
	memset(enemyShootable, 0, sizeof(enemyShootable));

	// Build a list of enemies based on dislike types
	m_sightedHostileNPCsList.begin();
	while(!m_sightedHostileNPCsList.end())
	{
		CBaseEntity* pEnemy = m_sightedHostileNPCsList.get();
		Int32 enemyRelation = GetRelationship( pEnemy );

		if (pEnemy->IsAlive() && enemyRelation != R_NONE && enemyRelation != R_ALLY && enemyRelation != R_FRIEND)
		{
			// Get origin we can test with and get distance
			Vector enemyOrigin = pEnemy->GetNavigablePosition();
			Float enemyDist = ( enemyOrigin - m_pState->origin ).Length();

			// Only check if we have ranged attacks
			bool isShootable = false;
			if(CanRangeAttack() && FavorRangedAttacks(pEnemy))
				isShootable = IsEnemyShootable( (*pEnemy), true, false );

			//Insert this one if the category has no enemies, or it's closer than the last one
			if(!categorizedEnemies[enemyRelation] 
			|| enemyDist <= enemyDistances[enemyRelation] 
			|| (!enemyShootable[enemyRelation] && isShootable))
			{
				enemyDistances[enemyRelation] = enemyDist;
				categorizedEnemies[enemyRelation] = pEnemy;
				enemyShootable[enemyRelation] = isShootable;
			}
		}

		m_sightedHostileNPCsList.next();
	}

	// Get the best enemy from the three categories
	// considering least to most hated
	bool lastShootable = false;
	bool lastReachable = false;
	CBaseEntity* lastBestEnemy = nullptr;
	for(int i = 0; i < NUM_ENEMY_RELATIONS; i++)
	{
		if( !categorizedEnemies[i] )
			continue;

		CBaseEntity* bestEnemy = categorizedEnemies[i];
		Vector vecOrigin = bestEnemy->GetNavigablePosition();
		bool isShootable = enemyShootable[i];

		if(lastBestEnemy)
		{
			// We already have a potential enemy, but if this guy is more hated, first make sure
			// it can actually be reached or shot at
			if( CanRangeAttack() && FavorRangedAttacks(bestEnemy) 
				&& (isShootable == lastShootable || !lastShootable && isShootable) )
			{
				// Choose an enemy we can shoot
				bool isReachable = false;
				if(SDL_fabs(m_pState->origin.z - vecOrigin.z) < VEC_HUMAN_HULL_MAX[2])
				{
					// Only do localmove tests on entities around our height for performance reasons
					localmove_t moveResult = CheckLocalMove( m_pState->origin, vecOrigin, bestEnemy, nullptr);
					if( moveResult > LOCAL_MOVE_RESULT_FAILURE )
						isReachable = true;
				}

				if (!FavorDirectlyReachableEnemies() || isReachable)
				{
					lastBestEnemy = bestEnemy;
					lastShootable = isShootable;
					lastReachable = isReachable;
				}
			}
			else if(abs(m_pState->origin.z - vecOrigin.z) < NPC_TRIANGULATION_MAX_HEIGHT)
			{
				// Only do localmove tests on entities around our height for performance reasons
				localmove_t moveResult = CheckLocalMove( m_pState->origin, vecOrigin, bestEnemy, nullptr);
				if( moveResult > LOCAL_MOVE_RESULT_FAILURE || !lastReachable )
				{
					lastBestEnemy = bestEnemy;
					lastShootable = isShootable;
					lastReachable = true;
				}
			}
		}
		else
		{
			// No enemy yet, so just try to pick the first guy
			lastBestEnemy = bestEnemy;
			lastShootable = isShootable;
			lastReachable = false;

			// Only do localmove tests on entities around our height for performance reasons
			if(SDL_fabs(m_pState->origin.z - vecOrigin.z) < VEC_HUMAN_HULL_MAX[2])
			{
				localmove_t moveResult = CheckLocalMove( m_pState->origin, vecOrigin, bestEnemy, NULL);
				if( moveResult > LOCAL_MOVE_RESULT_FAILURE )
					lastReachable = true;
			}
		}
	}

	return lastBestEnemy;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::PreScheduleThink( void )
{
	// Examine any dangers
	ExamineDangers();
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::IsEnemyOf( CBaseEntity* pNPC ) const
{
	if(m_enemy == pNPC)
		return true;
	else
		return false;
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::CheckEnemy( void )
{
	// Tells if enemy's last known position was updated
	bool updatedEnemy = false;

	// Check visibility on the enemy
	Uint64 visibilityBits = GetNPCVisibilityBits(m_enemy, false);
	if(visibilityBits == AI_SIGHTED_NOTHING)
		SetCondition(AI_COND_ENEMY_OCCLUDED);
	else
		ClearCondition(AI_COND_ENEMY_OCCLUDED);

	// Check if enemy died
	if(!m_enemy->IsAlive())
	{
		SetCondition(AI_COND_ENEMY_DEAD);
		ClearCondition(AI_COND_SEE_ENEMY);
		ClearCondition(AI_COND_ENEMY_OCCLUDED);
		ClearCondition(AI_COND_ENEMY_NAVIGABLE);
		return false;
	}

	// Get distance to enemy's origin
	Vector enemyPosition = m_enemy->GetNavigablePosition();
	Vector myNavigablePosition = GetNavigablePosition();
	Float enemyDistance1 = (enemyPosition-myNavigablePosition).Length();

	// Get distance to enemy head or feet
	Vector enemySize = m_enemy->GetSize();
	enemyPosition.z += enemySize.z * 0.5;

	Float enemyDistance2 = (enemyPosition - myNavigablePosition).Length();
	if(enemyDistance2 < enemyDistance1)
	{
		// Distance to head is used
		enemyDistance1 = enemyDistance2;
	}
	else
	{
		// Get distance to feet
		enemyPosition.z -= enemySize.z;
		enemyDistance2 = (enemyPosition-myNavigablePosition).Length();
		if(enemyDistance2 < enemyDistance1)
			enemyDistance1 = enemyDistance2;
	}

	if(!CheckCondition(AI_COND_ENEMY_OCCLUDED) && !(m_enemy->GetFlags() & FL_NOTARGET)
		&& (!CEnvFog::GetFogEndDistance() || enemyDistance1 < m_lookDistance) &&
		(CheckCondition(AI_COND_SEE_ENEMY) || enemyDistance1 <= NPC_MINIMUM_ENEMY_DISTANCE))
	{
		// Update enemy's last known position
		m_enemyLastKnownAngles = m_enemy->GetAngles();
		m_enemyLastKnownPosition = m_enemy->GetNavigablePosition();
		updatedEnemy = true;
		
		// Update enemy sighting on squad leader
		CBaseEntity* pLeader = GetSquadLeader();
		if(pLeader && pLeader != this && pLeader->GetEnemy() == m_enemy)
			pLeader->SetLastEnemySightTime(g_pGameVars->time);

		// Update awareness about player
		if (m_enemy->IsPlayer())
			m_enemy->SetNPCAwareness(1.0, this, NPC_COMBATSTATE_TIMEOUT, false);

		// We've found the enemy, so clear this
		ClearCondition(AI_COND_ENEMY_NOT_FOUND);

		// Do extra stuff if enemy is visible
		if(CheckCondition(AI_COND_SEE_ENEMY))
		{
			// Check if enemy is facing me
			if(m_enemy->IsInView(this))
				SetCondition(AI_COND_ENEMY_FACING_ME);
			else
				ClearCondition(AI_COND_ENEMY_FACING_ME);

			// Trail the enemy's position a bit
			if(!m_enemy->GetVelocity().IsZero())
				m_enemyLastKnownPosition -= m_enemy->GetVelocity() * Common::RandomFloat(-0.05, 0);
		}
	}
	else
	{
		// Check if we've heard our enemy
		if(m_pBestSound && CheckCondition(AI_COND_HEAR_SOUND) 
			&& m_pBestSound->emitter == m_enemy)
		{
			// Update enemy's last known position if we heard a sound from him
			m_enemyLastKnownAngles = m_enemy->GetAngles();
			m_enemyLastKnownPosition = m_enemy->GetNavigablePosition();
			updatedEnemy = true;

			// Update awareness about player
			if(m_enemy->IsPlayer())
				m_enemy->SetNPCAwareness(1.0, this, NPC_COMBATSTATE_TIMEOUT, false);

			// We've found the enemy, so clear this
			ClearCondition(AI_COND_ENEMY_NOT_FOUND);
		}

		if(!updatedEnemy)
		{
			if(!CheckCondition(AI_COND_SEE_ENEMY))
			{
				// See if we can expire the player awareness
				if(m_enemy->IsPlayer() && !HasSpawnFlag(FL_NPC_DONT_FORGET_PLAYER))
				{
					CBaseEntity* pSquadLeader = GetSquadLeader();
					if(pSquadLeader && (pSquadLeader == this || pSquadLeader->GetEnemy() != m_enemy))
					{
						if((m_lastEnemySightTime + NPC_COMBATSTATE_TIMEOUT) < g_pGameVars->time)
						{
							ForgetPlayer(m_enemy);
							return false;
						}
					}
				}

				// Check for missing enemy if not timed out yet
				if(!CheckCondition(AI_COND_ENEMY_NOT_FOUND))
				{
					Float enemyLKPDistance = (m_enemyLastKnownPosition - m_pState->origin).Length2D();
					if(enemyLKPDistance < NPC_STEP_SIZE)
					{
						Vector enemyLookOrigin = (m_enemy->GetEyePosition() - m_enemy->GetNavigablePosition()) + m_enemyLastKnownPosition;
						if((m_enemyLastKnownPosition.z - m_pState->origin.z) < NPC_ENEMY_INTERCEPT_DISTANCE 
							|| Util::CheckTraceLine(GetEyePosition(), enemyLookOrigin, true, true, m_pEdict))
						{
							// Enemy not found, so start looking around
							SetCondition(AI_COND_ENEMY_NOT_FOUND);
						}
					}
				}
			}
			else
			{
				// Enemy is here, so clear this condition
				ClearCondition(AI_COND_ENEMY_NOT_FOUND);
			}
		}
	}

	// Set AI_COND_ENEMY_TOO_FAR based on distance to enemy
	if(enemyDistance1 > m_firingDistance)
		SetCondition(AI_COND_ENEMY_TOO_FAR);
	else
		ClearCondition(AI_COND_ENEMY_TOO_FAR);

	// See if enemy is navigable
	if(ShouldCheckEnemyNavigability())
	{
		Float distanceChange = (m_enemyLastKnownPosition - m_lastNavigabilityCheckPosition).Length();
		if(distanceChange > NAVIGABILITY_CHECK_MIN_DISTANCE_CHANGE)
		{
			if(CheckRoute(m_pState->origin, m_enemyLastKnownPosition, m_enemy))
				SetCondition(AI_COND_ENEMY_NAVIGABLE);
			else
				ClearCondition(AI_COND_ENEMY_NAVIGABLE);

			m_lastNavigabilityCheckPosition = m_enemyLastKnownPosition;
		}
	}

	// Clear previous attack conditions
	ClearCondition(AI_COND_CAN_RANGE_ATTACK1);
	ClearCondition(AI_COND_CAN_RANGE_ATTACK2);
	ClearCondition(AI_COND_CAN_MELEE_ATTACK1);
	ClearCondition(AI_COND_CAN_MELEE_ATTACK2);
	ClearCondition(AI_COND_CAN_SPECIAL_ATTACK1);
	ClearCondition(AI_COND_CAN_SPECIAL_ATTACK2);
	ClearCondition(AI_COND_FRIENDLY_FIRE);
	ClearCondition(AI_COND_SHOOT_VECTOR_VALID);

	// Check attacks opportunistically
	if(CanCheckAttacks())
		CheckAttacks(m_enemy, enemyDistance1);

	// If movement goal is enemy, try to get direct path
	if(m_movementGoal == MOVE_GOAL_ENEMY)
	{
		if(m_enemy && !(m_routePointsArray[m_routePointIndex].type & (MF_IS_GOAL|MF_DONT_SIMPLIFY)))
		{
			if(SDL_fabs(m_enemyLastKnownPosition.z - m_pState->origin.z) < NPC_STEP_SIZE 
				&& (m_enemyLastKnownPosition - m_pState->origin).Length() > NPC_CORNER_CUT_MIN_DIST)
			{
				localmove_t result = CheckLocalMove(m_pState->origin, m_enemyLastKnownPosition, m_enemy);
				if(result > LOCAL_MOVE_RESULT_FAILURE)
				{
					RefreshRoute();
					return updatedEnemy;
				}
			}
		}

		// Update routing if enemy moved a significant distance
		if((m_movementGoalPosition - m_enemyLastKnownPosition).Length2D() > NPC_ENEMY_UPDATE_DISTANCE)
		{
			if(UpdateRoute(m_enemy, m_enemyLastKnownPosition))
			{
				ShowRoute(false, MAX_ROUTE_POINTS, m_enemyLastKnownPosition);
				return updatedEnemy;
			}
		}

		// Update enemy location
		if(m_routePointsArray[m_routePointIndex].type & MF_IS_GOAL)
		{
			m_routePointsArray[m_routePointIndex].position = m_enemyLastKnownPosition;
			ShowRoute(false, MAX_ROUTE_POINTS, m_enemyLastKnownPosition);
		}
	}

	return updatedEnemy;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::PushEnemy( CBaseEntity* pEnemy, const Vector& lastPosition, const Vector& lastAngles, Double lastSightTime )
{
	// Ignore if null, or if it's the main enemy
	if(!pEnemy)
		return;

	assert(pEnemy->IsNPC() || pEnemy->IsPlayer() || pEnemy->IsFuncBreakableEntity());

	if(!pEnemy->IsNPC() && !pEnemy->IsPlayer() && !pEnemy->IsFuncBreakableEntity())
		return;

	if(pEnemy == m_enemy)
	{
		// If it's a player, update last sight time on leader
		CBaseEntity* pLeader = GetSquadLeader();
		if(pLeader && pLeader != this && pLeader->GetEnemy() == pEnemy)
			pLeader->SetLastEnemySightTime(lastSightTime);

		return;
	}

	// Make sure it's not already present
	Int32 insertPosition = NO_POSITION;
	for(Int32 i = 0; i < MAX_BACKED_UP_ENEMIES; i++)
	{
		if(m_backedUpEnemies[i].enemy == reinterpret_cast<const CBaseEntity*>(pEnemy))
		{
			// If it's a player, update last sight time on leader
			CBaseEntity* pLeader = GetSquadLeader();
			if (pLeader && pLeader != this && pLeader->GetEnemy() == pEnemy)
				pLeader->SetLastEnemySightTime(lastSightTime);

			return;
		}

		if(insertPosition == NO_POSITION 
			&& !m_backedUpEnemies[i].enemy)
			insertPosition = i;
	}

	if(insertPosition == NO_POSITION)
		return;

	enemy_info_t& newEnemy = m_backedUpEnemies[insertPosition];
	newEnemy.enemy = pEnemy;
	newEnemy.lastknownangles = lastAngles;
	newEnemy.lastknownorigin = lastPosition;
	newEnemy.lastsighttime = lastSightTime;

	// If it's a player, update last sight time on leader
	CBaseEntity* pLeader = GetSquadLeader();
	if(pLeader && pLeader != this && pLeader->GetEnemy() == pEnemy)
		pLeader->SetLastEnemySightTime(g_pGameVars->time);
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::PopEnemy( void )
{
	// Look through the enemies we backed up
	for(Int32 i = MAX_BACKED_UP_ENEMIES - 1; i >= 0; i--)
	{
		if(m_backedUpEnemies[i].enemy)
		{
			if(!HasSpawnFlag(FL_NPC_DONT_FORGET_PLAYER))
			{
				// Forget player if it's been too long
				if(m_backedUpEnemies[i].enemy->IsPlayer() 
					&& (m_backedUpEnemies[i].lastsighttime + NPC_COMBATSTATE_TIMEOUT) < g_pGameVars->time)
				{
					m_backedUpEnemies[i].enemy.reset();
					m_backedUpEnemies[i].lastknownangles.Clear();
					m_backedUpEnemies[i].lastknownorigin.Clear();
					m_backedUpEnemies[i].lastsighttime = 0;
					continue;
				}
			}

			if(m_backedUpEnemies[i].enemy->IsAlive() || m_backedUpEnemies[i].enemy->IsFuncBreakableEntity())
			{
				m_enemy = m_backedUpEnemies[i].enemy;
				m_enemyLastKnownPosition = m_backedUpEnemies[i].lastknownorigin;
				m_enemyLastKnownAngles = m_backedUpEnemies[i].lastknownangles;
				m_lastEnemySightTime = m_backedUpEnemies[i].lastsighttime;
				return true;
			}
			else
			{
				// Otherwise clear the enemy
				m_backedUpEnemies[i].enemy.reset();
				m_backedUpEnemies[i].lastknownangles.Clear();
				m_backedUpEnemies[i].lastknownorigin.Clear();
				m_backedUpEnemies[i].lastsighttime = 0;
			}
		}
	}

	return false;
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::GetNextEnemy( void )
{
	/// Clear this always
	ClearCondition(AI_COND_DANGEROUS_ENEMY_CLOSE);

	if(m_dangerousEnemy)
		m_dangerousEnemy.reset();

	// Pointer to enemy
	CBaseEntity* pNewEnemy = nullptr;

	if(CheckCondition(AI_COND_SEE_HATE) 
		|| CheckCondition(AI_COND_SEE_DISLIKE) 
		|| CheckCondition(AI_COND_SEE_NEMESIS))
	{
		// Get the most optimal best enemy
		pNewEnemy = GetBestVisibleEnemy();

		if(pNewEnemy != m_enemy && pNewEnemy && m_pSchedule)
		{
			if(m_pSchedule->GetInterruptMask().test(AI_COND_NEW_ENEMY))
			{
				// Remember our old enemy's position and angles
				if(m_enemy)
					PushEnemy(m_enemy, m_enemyLastKnownPosition, m_enemyLastKnownAngles, g_pGameVars->time);

				SetCondition(AI_COND_NEW_ENEMY);

				m_enemy = pNewEnemy;
				m_enemyLastKnownPosition = m_enemy->GetNavigablePosition();
				m_enemyLastKnownAngles = m_enemy->GetAngles();
				m_lastEnemySightTime = g_pGameVars->time;
			}

			// Push enemy's owner if he is a valid enemy
			CBaseEntity* pEnemyOwner = pNewEnemy->GetOwner();
			if(pEnemyOwner && pEnemyOwner->IsNPC() && GetRelationship(pEnemyOwner) != R_NONE)
				PushEnemy(pEnemyOwner, m_enemyLastKnownPosition, m_enemyLastKnownAngles, g_pGameVars->time);
		}
	}

	// Try and dig out an old enemy
	if(!m_enemy && PopEnemy())
	{
		if(m_pSchedule && m_pSchedule->GetInterruptMask().test(AI_COND_NEW_ENEMY))
			SetCondition(AI_COND_NEW_ENEMY);
	}

	// Tell if we have an enemy or not
	return m_enemy ? true : false;
}

//=============================================
// @brief
//
//=============================================
CBaseEntity* CBaseNPC::DropItem( weaponid_t weaponId, Uint32 attachmentIndex, bool wasGibbed )
{
	if(weaponId <= WEAPON_NONE || WEAPON_NONE >= NUM_WEAPONS)
		return nullptr;

	if(m_dontDropWeapons)
		return nullptr;

	Vector gunPosition;
	GetAttachment(attachmentIndex, gunPosition);

	Vector angles(m_pState->angles);
	angles[PITCH] = angles[ROLL] = 0;

	CString weaponName = WEAPON_ENTITY_NAMES[weaponId];
	if(weaponName.empty())
	{
		Util::EntityConPrintf(m_pEdict, "No classname specified.\n");
		return false;
	}

	CBaseEntity* pEntity = CBaseEntity::CreateEntity(weaponName.c_str(), gunPosition, angles, this);
	if(!pEntity)
	{
		Util::EntityConPrintf(m_pEdict, "'%s' is not a valid classname.\n", weaponName.c_str());
		return nullptr;
	}

	Vector velocity;
	Vector angularVelocity;

	if(wasGibbed)
	{
		for(Uint32 i = 0; i < 2; i++)
			velocity[i] = Common::RandomFloat(-100, 100);
		velocity[2] = Common::RandomFloat(200, 300);
					
		angularVelocity = Vector(0, Common::RandomFloat(200, 400), 0);

		pEntity->SetVelocity(velocity);
		pEntity->SetAngularVelocity(angularVelocity);
	}
	else
	{
		velocity = m_pState->velocity;
		angularVelocity = Vector(0, Common::RandomFloat(0, 100), 0);
	}

	pEntity->SetVelocity(velocity);
	pEntity->SetAngularVelocity(angularVelocity);

	if(!pEntity->Spawn())
	{
		Util::RemoveEntity(pEntity);
		return nullptr;
	}

	return pEntity;
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::IsAwareOf( CBaseEntity* pNPC ) const
{
	if(!pNPC)
		return false;

	if(m_enemy == pNPC)
		return true;

	for(Uint32 i = 0; i < MAX_BACKED_UP_ENEMIES; i++)
	{
		if(m_backedUpEnemies[i].enemy == pNPC)
			return true;
	}

	return false;
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::IsFacing( CBaseEntity* pEntity, const Vector& checkPosition )
{
	Vector direction = (checkPosition - pEntity->GetOrigin());
	direction[2] = 0;
	direction.Normalize();

	Vector angle = pEntity->GetAngles();
	angle[PITCH] = 0;

	Vector forward;
	Math::AngleVectors(angle, &forward);

	if(Math::DotProduct(forward, direction) > 0.96)
		return true;
	else
		return false;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::SetIdealActivity( Int32 activity )
{
	m_idealActivity = activity;
}

//=============================================
// @brief
//
//=============================================
Int32 CBaseNPC::GetIdealActivity( void )
{
	return m_idealActivity;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::SetActivity( Int32 activity )
{
	if(!IsMoving())
	{
		// For specific animations, wait until blend time's done to change
		if(activity == ACT_IDLE || activity == ACT_IDLE_ANGRY || activity == ACT_FOCUS)
		{
			if((g_pGameVars->time - m_lastActivityTime) < VBM_SEQ_BLEND_TIME)
				return;
		}
	}

	// Reset blending
	for(Uint32 i = 0; i < MAX_BLENDING; i++)
		SetBlending(i, 0);

	// Remember last sequence
	Int32 prevSequence = m_pState->sequence;
	// Don't use ACT_RESET, just set it to ACT_IDLE
	Int32 newSequence = FindActivity((activity == ACT_RESET) ? ACT_IDLE : activity);

	// Set the animation
	if(newSequence != NO_SEQUENCE_VALUE)
	{
		if(prevSequence != newSequence || !m_isSequenceLooped)
		{
			// Don't reset between walk and run
			if(m_currentActivity != ACT_WALK && m_currentActivity != ACT_WALK_HURT && m_currentActivity != ACT_RUN && m_currentActivity != ACT_RUN_HURT
				|| activity != ACT_WALK && activity != ACT_WALK_HURT && activity != ACT_RUN && activity != ACT_RUN_HURT)
				m_pState->frame = 0;
		}

		m_pState->sequence = newSequence;
		ResetSequenceInfo();
		SetYawSpeed();

		// Set last time we changed our sequence
		m_lastActivityTime = g_pGameVars->time;

		// If activity is an attack one, then set blending
		if(m_enemy && HasCapability(AI_CAP_ATTACK_BLEND_SEQ)
			&& (activity == ACT_RANGE_ATTACK1 || activity == ACT_RANGE_ATTACK2
			|| activity == ACT_MELEE_ATTACK1 || activity == ACT_MELEE_ATTACK2
			|| activity == ACT_SPECIAL_ATTACK1 || activity == ACT_SPECIAL_ATTACK2))
		{
			Vector shootOrigin = GetGunPosition();
			Vector shootDirection = GetShootVector(shootOrigin);

			Vector directionAngle = Math::VectorToAngles(shootDirection);
			SetBlending(0, directionAngle.x);
		}
		else
		{
			SetBlending(0, 0);
		}
	}
	else
	{
		Util::EntityConPrintf(m_pEdict, "No sequence for activity %s.\n", ACTIVITYMAP[activity].name);
		m_pState->sequence = 0;
	}

	// Set these
	m_currentActivity = activity;
	m_idealActivity = m_currentActivity;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::SetSequenceByName( const Char* pstrName )
{
	// Look it up by name
	Int32 newSequence = FindSequence(pstrName);
	// Remember last sequence
	Int32 prevSequence = m_pState->sequence;

	// Set the animation
	if(newSequence != NO_SEQUENCE_VALUE)
	{
		if(prevSequence != newSequence || !m_isSequenceLooped)
			m_pState->frame = 0;

		m_pState->sequence = newSequence;
		ResetSequenceInfo();
		SetYawSpeed();

		// Set last time we changed our sequence
		m_lastActivityTime = g_pGameVars->time;
	}
	else
	{
		Util::EntityConPrintf(m_pEdict, "No sequence for name %s.\n", pstrName);
		m_pState->sequence = 0;
	}
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::WalkMoveTrace( const Vector& origin, const Vector& moveDirection, Vector& outPosition, Float moveDistance, Float& distanceMoved, bool noNPCs )
{
	Vector originalPosition = m_pState->origin;
	Vector startPosition = origin;
	Vector endPosition = startPosition + moveDirection * moveDistance;

	Float yaw = Util::VectorToYaw(endPosition-startPosition);
	Float distance = (endPosition-startPosition).Length2D();

	// Save these for after-test restore
	Uint64 savedFlags = m_pState->flags;
	entindex_t savedGroundEntity = m_pState->groundent;

	// Assume we'll make it
	bool traceResult = true;

	// Move NPC to the start of the move
	gd_engfuncs.pfnSetOrigin(m_pEdict, startPosition);

	// Make sure we're on the floor
	if(!(m_pState->flags & (FL_FLY|FL_SWIM)))
		gd_engfuncs.pfnDropToFloor(m_pEdict);

	// Perform the movement
	Vector lastValidPosition = m_pState->origin;
	for(distanceMoved = 0; distanceMoved < distance; distanceMoved += NPC_STEP_SIZE)
	{
		Float stepSize = NPC_STEP_SIZE;
		if((distanceMoved+NPC_STEP_SIZE) > (distance-1))
			stepSize = (distance-distanceMoved) - 1;

		if(!gd_engfuncs.pfnWalkMove(m_pEdict, yaw, stepSize, noNPCs ? WALKMOVE_NO_NPCS : WALKMOVE_CHECKONLY))
		{
			// Set the result to false
			traceResult = false;

			if(g_pGameVars->globaltrace.hitentity != NO_ENTITY_INDEX)
			{
				CBaseEntity* pHitEntity = Util::GetEntityFromTrace(g_pGameVars->globaltrace);
				if(pHitEntity && (pHitEntity->IsFuncDoorEntity() || pHitEntity->IsNPC() && pHitEntity != m_blockedNPC))
				{
					CEntityStateStack saveStack;

					// Save the original always
					saveStack.SaveEntity(pHitEntity);

					// Look up any additional entities
					if(pHitEntity->IsFuncDoorEntity())
					{
						CArray<CBaseEntity*> linkEntitiesArray;
						Util::FindLinkEntities(pHitEntity, linkEntitiesArray, nullptr);

						for(Uint32 i = 0; i < linkEntitiesArray.size(); i++)
							saveStack.SaveEntity(linkEntitiesArray[i]);
					}

					// Do another trace
					for(; distanceMoved < distance; distanceMoved += NPC_STEP_SIZE)
					{
						stepSize = NPC_STEP_SIZE;
						if((distanceMoved+NPC_STEP_SIZE) > (distance-1))
							stepSize = (distance-distanceMoved) - 1;

						if(!gd_engfuncs.pfnWalkMove(m_pEdict, yaw, stepSize, WALKMOVE_CHECKONLY))
						{
							traceResult = false;
							break;
						}
					}
				}
			}

			break;
		}

		// Set last valid position
		lastValidPosition = m_pState->origin;
	}

	// Set output position
	outPosition = lastValidPosition;
	// Since we've actually moved the NPC, move him back
	gd_engfuncs.pfnSetOrigin(m_pEdict, originalPosition);

	// Restore original state
	m_pState->flags = savedFlags;
	m_pState->groundent = savedGroundEntity;

	return traceResult;
}

//=============================================
// @brief
//
//=============================================
localmove_t CBaseNPC::CheckLocalMove( const Vector startPosition, const Vector& endPosition, const CBaseEntity* pTargetEntity, Float* pDistance, bool isInitial )
{
	// Don't waste resources on objects significantly higher than us
	if((startPosition.z - endPosition.z) > NPC_MAX_LOCALMOVE_HEIGHT_DIFF)
		return LOCAL_MOVE_INVALID_NO_TRIANGULATION;

	// Save these for after-test restore
	Uint64 savedFlags = m_pState->flags;
	entindex_t savedGroundEntity = m_pState->groundent;

	// Move NPC to start position and drop him to the floor
	Vector moveStart = m_pState->origin;
	gd_engfuncs.pfnSetOrigin(m_pEdict, startPosition);

	// Drop entity to floor
	if(!(m_pState->flags & (FL_FLY|FL_SWIM)))
		gd_engfuncs.pfnDropToFloor(m_pEdict);

	// Make sure it's a valid position, not something inside a solid
	if(!IsPositionNavigable(endPosition))
	{
		m_pState->flags = savedFlags;
		m_pState->groundent = savedGroundEntity;

		gd_engfuncs.pfnSetOrigin(m_pEdict, moveStart);
		return LOCAL_MOVE_INVALID_NO_TRIANGULATION;
	}

	// Reset blocker entity
	m_blockerEntity.reset();
	
	Float yaw = Util::VectorToYaw(endPosition - startPosition);
	Float distance = (endPosition - startPosition).Length2D();

	// Result of move
	localmove_t moveResult = LOCAL_MOVE_VALID;
	// Total distance moved
	Float distanceMoved = 0;

	if(distance > 0)
	{
		// Perform the movement
		for(; distanceMoved < distance; distanceMoved += NPC_STEP_SIZE)
		{
			Float stepSize = NPC_STEP_SIZE;
			if((distanceMoved+NPC_STEP_SIZE) > (distance-1))
				stepSize = (distance-distanceMoved) - 1;

			if(!gd_engfuncs.pfnWalkMove(m_pEdict, yaw, stepSize, WALKMOVE_CHECKONLY))
			{
				// Set distance if available
				if(pDistance)
					(*pDistance) = distanceMoved;

				// Retrieve hit entity if possible
				if(g_pGameVars->globaltrace.hitentity != NO_ENTITY_INDEX)
					m_blockerEntity = Util::GetEntityFromTrace(g_pGameVars->globaltrace);

				if(m_blockerEntity && pTargetEntity && pTargetEntity == m_blockerEntity)
				{
					// We hit the target entity, so it was a success
					moveResult = LOCAL_MOVE_REACHED_TARGET;
					break;
				}
				else if(m_npcState == NPC_STATE_SCRIPT && m_blockerEntity && m_blockerEntity->IsPlayer() && isInitial)
				{
					// See if we can nudge the player
					CEntityStateStack saveStack;
					saveStack.SaveEntity(m_blockerEntity);

					for(; distanceMoved < distance; distanceMoved += NPC_STEP_SIZE)
					{
						stepSize = NPC_STEP_SIZE;
						if((distanceMoved+NPC_STEP_SIZE) > (distance-1))
							stepSize = (distance-distanceMoved) - 1;

						if(!gd_engfuncs.pfnWalkMove(m_pEdict, yaw, stepSize, WALKMOVE_CHECKONLY))
						{
							CBaseEntity* pSecondBlocker = nullptr;
							if(g_pGameVars->globaltrace.hitentity != NO_ENTITY_INDEX)
								pSecondBlocker = Util::GetEntityFromTrace(g_pGameVars->globaltrace);

							if(pSecondBlocker && pTargetEntity && pTargetEntity == pSecondBlocker)
							{
								// We hit the target entity, so it was a success
								moveResult = LOCAL_MOVE_REACHED_TARGET;
								m_blockerEntity.reset();
							
							}
							else
							{
								// Can't get there either way
								moveResult = LOCAL_MOVE_INVALID;
							}

							// Don't do any more
							break;
						}
					}

					// Don't do any more
					break;
				}
				else
				{
					// Can't get there either way
					moveResult = LOCAL_MOVE_INVALID;
					break;
				}
			}
		}
	}

	// Since we've actually moved the NPC, move him back
	Vector moveEnd = m_pState->origin;
	gd_engfuncs.pfnSetOrigin(m_pEdict, moveStart);

	if(!(m_pState->flags & (FL_FLY|FL_SWIM)) && (!pTargetEntity || pTargetEntity->GetFlags() & FL_ONGROUND))
	{
		// We can move to a spot under the NPC, but not to it. In this case, don't triangulate
		if(SDL_fabs(endPosition.z - moveEnd.z) > NPC_TRIANGULATION_MAX_HEIGHT)
			moveResult = LOCAL_MOVE_INVALID_NO_TRIANGULATION;
	}

	// Set distance travelled
	m_distanceTravelled = distanceMoved;

	// Restore original states
	m_pState->flags = savedFlags;
	m_pState->groundent = savedGroundEntity;

	return moveResult;
}

//=============================================
// @brief
//
//=============================================
Double CBaseNPC::OpenDoor( CBaseEntity* pDoorEntity )
{
	if(!pDoorEntity || !pDoorEntity->IsFuncDoorEntity())
		return g_pGameVars->time;

	if(pDoorEntity->HasSpawnFlag(CFuncDoor::FL_NO_NPCS)
		|| pDoorEntity->GetToggleState() == TS_AT_TOP
		|| pDoorEntity->GetToggleState() == TS_GOING_UP)
		return g_pGameVars->time;

	// Trigger the door to open
	pDoorEntity->CallUse(this, this, USE_ON, 0);

	// Get travel time
	Double openTime = pDoorEntity->GetNextThinkTime() - pDoorEntity->GetLocalTime();

	// See if a trigger_multiple is targeting this door, and if so, then
	// set it to wait for it's given time
	if(pDoorEntity->HasTargetName())
	{
		const Char* pstrTargetName = pDoorEntity->GetTargetName();

		edict_t* pEdict = nullptr;
		while(true)
		{
			pEdict = Util::FindEntityByTarget(pEdict, pstrTargetName);
			if(!pEdict)
				break;

			CBaseEntity* pEntity = CBaseEntity::GetClass(pEdict);
			if(pEntity->IsTriggerMultipleEntity())
			{
				pEntity->SetTriggerWait();
				break;
			}
		}
	}

	// Get the entities relevant to this door
	CArray<CBaseEntity*> linkEntityArray;
	Util::FindLinkEntities(pDoorEntity, linkEntityArray, this);

	// If we have something, trigger those entities
	if(!linkEntityArray.empty())
	{
		for(Uint32 i = 0; i < linkEntityArray.size(); i++)
			linkEntityArray[i]->CallUse(this, this, USE_ON, 0);
	}

	return g_pGameVars->time + openTime;
}

//=============================================
// @brief
//
//=============================================
CBaseNPC::advance_result_t CBaseNPC::AdvanceRoute( Float distance )
{
	if(m_routePointIndex >= MAX_ROUTE_POINTS - 1)
	{
		if(RefreshRoute())
		{
			// Successfully refreshed the route
			return ADVANCE_RESULT_SUCCESS;
		}
		else
		{
			Util::EntityConDPrintf(m_pEdict, "Can't refresh route.\n");
			return ADVANCE_RESULT_FAILED;
		}
	}

	route_point_t& routePoint = m_routePointsArray[m_routePointIndex];
	if(routePoint.type & MF_IS_GOAL)
	{
		// If distance is smaller than corner cut, then set movement as completed
		if(distance < NPC_CORNER_CUT_MIN_DIST)
			return ADVANCE_RESULT_REACHED_GOAL;
		else
			return ADVANCE_RESULT_FAILED;
	}

	// If we just passed a path corner, advance goal entity
	if(m_goalEntity && (routePoint.type & MF_NOT_TO_MASK) == MF_TO_PATH_CORNER)
		m_goalEntity = m_goalEntity->GetNextTarget();

	// Get next route point
	route_point_t& nextRoutePoint = m_routePointsArray[m_routePointIndex+1];
	if((routePoint.type & MF_TO_NODE) && (nextRoutePoint.type & MF_TO_NODE) 
		&& routePoint.nodeindex != NO_POSITION)
	{
		// See if we have any link entities
		Uint32 numlinkentities = 0;
		CAINodeGraph::link_entity_t* plinkentities = nullptr;
		if(gNodeGraph.GetNodeLinkEntities(routePoint.nodeindex, nextRoutePoint.nodeindex, plinkentities, numlinkentities))
		{
			// Get our hull type
			Int32 hullType = Util::GetNodeHullForNPC(this);

			// Go through all the linkents
			for(Uint32 i = 0; i < numlinkentities; i++)
			{
				CAINodeGraph::link_entity_t& linkentity = plinkentities[i];
				if(linkentity.hulltype > hullType 
					|| linkentity.entityindex == NO_ENTITY_INDEX)
					continue;

				if(gNodeGraph.HandleLinkEntity(routePoint.nodeindex, linkentity.entityindex, m_capabilityBits, CAINodeGraph::GRAPH_QUERY_DYNAMIC, this))
				{
					// If linkent has become null/invalid, delete the link
					if(Util::IsNullEntity(linkentity.entityindex))
					{
						linkentity.entityindex = NO_ENTITY_INDEX; 
						continue;
					}
					else
					{
						// If it's a door, open it
						CBaseEntity* plinkentity = Util::GetEntityByIndex(linkentity.entityindex);
						if(plinkentity->IsFuncDoorEntity())
						{
							// Only doors for now
							Double waitTime = OpenDoor(plinkentity);
							if(waitTime > m_moveWaitFinishTime)
								m_moveWaitFinishTime = waitTime;
						}
					}
				}
				else
				{
					// Couldn't handle linkent, so fail
					Util::EntityConDPrintf(m_pEdict, "Abort advance route due to unhandled linkent.\n");
					return ADVANCE_RESULT_FAILED;
				}
			}
		}
	}

	// Advance route
	m_routePointIndex++;

	// Show debug
	ShowRoute(false, MAX_ROUTE_POINTS, m_movementGoalPosition);
	return ADVANCE_RESULT_SUCCESS;
}

//=============================================
// @brief
//
//=============================================
CBaseNPC::movegoal_t CBaseNPC::ClassifyRoute( Uint64 moveFlags )
{
	if(moveFlags & MF_TO_TARGETENT)
		return MOVE_GOAL_TARGET_ENT;
	else if(moveFlags & MF_TO_ENEMY)
		return MOVE_GOAL_ENEMY;
	else if(moveFlags & MF_TO_PATH_CORNER)
		return MOVE_GOAL_PATH_CORNER;
	else if(moveFlags & MF_TO_NODE)
		return MOVE_GOAL_NODE;
	else if(moveFlags & MF_TO_LOCATION)
		return MOVE_GOAL_LOCATION;
	else
		return MOVE_GOAL_NONE;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::MovementComplete( void )
{
	switch(m_taskStatus)
	{
	case TASK_STATUS_NEW:
	case TASK_STATUS_RUNNING:
		m_taskStatus = TASK_STATUS_RUNNING_TASK;
		break;
	case TASK_STATUS_RUNNING_MOVEMENT:
		SetTaskCompleted();
		break;
	case TASK_STATUS_RUNNING_TASK:
		Util::EntityConPrintf(m_pEdict, "Movement completed twice.\n");
		break;
	case TASK_STATUS_COMPLETE:
		break;
	}

	m_movementGoal = MOVE_GOAL_NONE;
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::UpdateRoute( CBaseEntity* pTargetEntity, const Vector& destination )
{
	// For holding temp node indexes
	static Int32 nodeIndexes[MAX_ROUTE_POINTS];

	// If following an enemy, skip updates if he's not visible
	if(m_movementGoalEntity && m_movementGoalEntity == m_enemy
		&& !(CheckCondition(AI_COND_SEE_ENEMY) || !CheckCondition(AI_COND_ENEMY_OCCLUDED)))
		return true;

	// Find the last node
	Int32 lastPointIndex = NO_POSITION;
	for(Int32 i = m_routePointIndex; i < MAX_ROUTE_POINTS; i++)
	{
		if(m_routePointsArray[i].type == MF_NONE)
			break;

		lastPointIndex = i;
	}

	if(lastPointIndex == NO_POSITION)
		return false;

	// Try going straight for them
	route_point_t& lastPoint = m_routePointsArray[lastPointIndex];
	if(lastPoint.type & MF_IS_GOAL)
	{
		localmove_t moveResult = CheckLocalMove(m_pState->origin, destination, m_movementGoalEntity);
		if(moveResult > LOCAL_MOVE_RESULT_FAILURE)
		{
			if(m_enemy && m_enemy == reinterpret_cast<const CBaseEntity*>(pTargetEntity))
			{
				m_enemyLastKnownPosition = destination;
				m_enemyLastKnownAngles = pTargetEntity->GetAngles();
			}

			lastPoint.position = destination;
			m_movementGoalPosition = destination;
			return true;
		}
	}

	// Fail if we can't refresh
	if(lastPointIndex >= (MAX_ROUTE_POINTS-1))
		return false;

	if(!gNodeGraph.IsNodeGraphValid())
	{
		gd_engfuncs.pfnCon_Printf("AI Graph not initialized.\n");
		return false;
	}

	// the "destination" variable should already be coming from GetNavigableOrigin
	// if the destination is an NPC
	Int32 destNode = gNodeGraph.GetNearestNode( destination, this, pTargetEntity );
	if(destNode == NO_POSITION)
		return false;

	Int32 srcNode;
	if(lastPoint.nodeindex == NO_POSITION)
	{
		srcNode = gNodeGraph.GetNearestNode(lastPoint.position, this, pTargetEntity);
		if(srcNode == NO_POSITION)
			return false;
	}
	else
	{
		// Just use the already present node index
		srcNode = lastPoint.nodeindex;
	}

	// If target is still reachable by last node, only update destination
	if(srcNode == destNode)
	{
		m_routePointsArray[lastPointIndex].position = destination;
		m_movementGoalPosition = destination;
		return true;
	}

	// Get the hull type for this NPC
	node_hull_types_t hullType = Util::GetNodeHullForNPC(this);
	Int32 numNodes = gNodeGraph.GetShortestPath(srcNode, destNode, hullType, m_capabilityBits, nodeIndexes, this, pTargetEntity);
	if(numNodes <= 0)
		return false;

	// Make sure we don't copy more than can fit
	if((lastPointIndex+numNodes+1) >= MAX_ROUTE_POINTS)
		numNodes = (MAX_ROUTE_POINTS - lastPointIndex - 1);

	// Save flags from goal
	Int32 goalFlags = lastPoint.type;
	for(Int32 i = 0; i < numNodes; i++)
	{
		route_point_t& insert = m_routePointsArray[lastPointIndex];
		lastPointIndex++;

		const CAINodeGraph::node_t* pnode = gNodeGraph.GetNode(nodeIndexes[i]);
		if(!pnode)
			return false;

		insert.position = pnode->origin;
		insert.nodeindex = nodeIndexes[i];
		insert.type = MF_TO_NODE;
	}

	if(lastPointIndex < MAX_ROUTE_POINTS)
	{
		route_point_t& insert = m_routePointsArray[lastPointIndex];
		lastPointIndex++;

		insert.position = destination;
		insert.nodeindex = destNode;
		insert.type = goalFlags;

		if(lastPointIndex < MAX_ROUTE_POINTS)
			m_routePointsArray[lastPointIndex].type = MF_NONE;
	}

	// Set this always
	m_movementGoalPosition = destination;
	return true;
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::BuildRoute( const Vector& destination, Uint64 moveFlags, CBaseEntity* pTargetEntity )
{
	// Begin a new route
	NewRoute();
	
	// Make sure it's a valid position, not something inside a solid
	if(!IsPositionNavigable(destination))
	{
		route_point_t& firstPoint = m_routePointsArray[0];
		firstPoint.position = destination;
		firstPoint.type = moveFlags | MF_IS_GOAL;
		firstPoint.nodeindex = NO_POSITION;
		m_routePointsArray[1].type = MF_NONE;
		ShowRoute(false, MAX_ROUTE_POINTS, destination);
		return false;
	}

	// Set movement goal and entity
	m_movementGoal = ClassifyRoute(moveFlags);
	m_movementGoalEntity = pTargetEntity;

	// Set first route point always
	route_point_t& firstPoint = m_routePointsArray[0];
	firstPoint.position = destination;
	firstPoint.type = moveFlags | MF_IS_GOAL;
	firstPoint.nodeindex = NO_POSITION;

	// Check if we can move to this point straight away,
	// but only if it's not too far up or down from us
	Vector apexPosition;
	Float moveDistance = -1;
	localmove_t moveResult = LOCAL_MOVE_INVALID_NO_TRIANGULATION;
	if(!(moveFlags & MF_TO_ENEMY) || SDL_fabs(m_pState->origin.z - destination.z) < NPC_TRIANGULATION_MAX_HEIGHT)
		moveResult = CheckLocalMove(m_pState->origin, destination, pTargetEntity, &moveDistance, true);

	if(moveResult > LOCAL_MOVE_RESULT_FAILURE)
	{
		// Set last point to none
		m_routePointsArray[1].type = MF_NONE;

		// We can go there straight
		ShowRoute(false, MAX_ROUTE_POINTS, destination);
		return true;
	}
	else if(moveResult != LOCAL_MOVE_INVALID_NO_TRIANGULATION && AttemptTriangulation(m_pState->origin, destination, moveDistance, pTargetEntity, &apexPosition))
	{
		// Set first position to the apex position
		firstPoint.position = apexPosition;
		firstPoint.type = (moveFlags | MF_TO_DETOUR);
		firstPoint.nodeindex = NO_POSITION;

		// Set destination
		route_point_t& destPoint = m_routePointsArray[1];
		destPoint.position = destination;
		destPoint.type = (moveFlags | MF_IS_GOAL);
		destPoint.nodeindex = NO_POSITION;

		// Set last point to none
		m_routePointsArray[2].type = MF_NONE;

		ShowRoute(false, MAX_ROUTE_POINTS, destination);
		return true;
	}
	else if(BuildNodeRoute(destination, pTargetEntity))
	{
		m_movementGoalPosition = destination;
		SimplifyRoute(pTargetEntity);
		ShowRoute(false, MAX_ROUTE_POINTS, destination);
		return true;
	}

	// HACK: Teleport the NPC if he's stuck on a scripted_sequence, so
	// we don't end up locking the game
	if(m_npcState == NPC_STATE_SCRIPT 
		&& m_blockerEntity 
		&& m_blockerEntity->IsBrushModel() 
		&& !m_blockerEntity->IsMoving() 
		&& !m_distanceTravelled)
	{
		Vector savedOrigin = m_pState->origin;
		gd_engfuncs.pfnSetOrigin(m_pEdict, destination);

		// Make sure it's a valid position
		if(!gd_engfuncs.pfnWalkMove(m_pEdict, 0, 0, WALKMOVE_CHECKONLY))
		{
			if(g_pGameVars->globaltrace.hitentity != NO_ENTITY_INDEX)
			{
				CBaseEntity* pHitEntity = Util::GetEntityFromTrace(g_pGameVars->globaltrace);
				if(pHitEntity && (pHitEntity->IsNPC() || pHitEntity->IsPlayer()))
				{
					// Failed, try again later
					gd_engfuncs.pfnSetOrigin(m_pEdict, savedOrigin);
				}
			}
		}

		// Try to nudge the NPC
		GroundEntityNudge();
	}

	return false;
}

//=============================================
// @brief Tells if a position is navigable
//
//=============================================
bool CBaseNPC::IsPositionNavigable( const Vector& position )
{
	Uint64 savedFlags = m_pState->flags;
	entindex_t savedGroundEntity = m_pState->groundent;

	// Remember original position
	Vector moveStart = m_pState->origin;
	gd_engfuncs.pfnSetOrigin(m_pEdict, position);

	// See result of moving in said location
	bool checkResult = gd_engfuncs.pfnWalkMove(m_pEdict, 0, 0, WALKMOVE_NO_NPCS);
	if(!checkResult)
	{
		// If the previous failed, try lifting NPC off the ground
		Vector checkPosition = position + Vector(0, 0, 4);
		gd_engfuncs.pfnSetOrigin(m_pEdict, position);

		if(!(m_pState->flags & (FL_FLY|FL_SWIM)))
			gd_engfuncs.pfnDropToFloor(m_pEdict);

		checkResult = gd_engfuncs.pfnWalkMove(m_pEdict, 0, 0, WALKMOVE_NO_NPCS);
	}

	// Always restore original position
	gd_engfuncs.pfnSetOrigin(m_pEdict, moveStart);

	// Restore original state
	m_pState->flags = savedFlags;
	m_pState->groundent = savedGroundEntity;

	return checkResult;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::InsertRoutePoint( const Vector& position, Int32 nodeIndex, Uint64 moveFlags )
{
	Uint64 typeFlags = (moveFlags | (m_routePointsArray[m_routePointIndex].type & ~MF_NOT_TO_MASK));
	typeFlags &= ~(MF_TO_ENEMY|MF_IS_GOAL);

	for(Int32 i = MAX_ROUTE_POINTS-1; i > m_routePointIndex; i--)
		m_routePointsArray[i] = m_routePointsArray[i-1];

	route_point_t& insert = m_routePointsArray[m_routePointIndex];
	insert.position = position;
	insert.nodeindex = nodeIndex;
	insert.type = typeFlags;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::InsertRoutePoint( Int32 insertIndex, const Vector& position, Int32 nodeIndex, Uint64 moveFlags )
{
	// Make sure the position is valid
	if(insertIndex >= MAX_ROUTE_POINTS)
		return;

	Uint64 typeFlags = (moveFlags | (m_routePointsArray[insertIndex].type & ~MF_NOT_TO_MASK));
	typeFlags &= ~(MF_TO_ENEMY|MF_IS_GOAL);

	for(Int32 i = MAX_ROUTE_POINTS-1; i > insertIndex; i--)
		m_routePointsArray[i] = m_routePointsArray[i-1];

	route_point_t& insert = m_routePointsArray[insertIndex];
	insert.position = position;
	insert.nodeindex = nodeIndex;
	insert.type = typeFlags;
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::CheckRoute( const Vector& startPosition, const Vector& endPosition, CBaseEntity* pTargetEntity )
{
	Float distanceTravelled = 0;
	Vector apexPosition;

	// Make sure it's a valid position, not something inside a solid
	if(!IsPositionNavigable(endPosition))
		return false;

	localmove_t moveResult = CheckLocalMove(startPosition, endPosition, pTargetEntity, &distanceTravelled);
	if(moveResult > LOCAL_MOVE_RESULT_FAILURE)
	{
		// We can go in a straight line
		return true;
	}
	else if(moveResult != LOCAL_MOVE_INVALID_NO_TRIANGULATION && AttemptTriangulation(startPosition, endPosition, distanceTravelled, pTargetEntity, &apexPosition, true))
	{
		// Triangulation was successful
		return true;
	}
	else if(CheckNodeRoute(startPosition, endPosition, pTargetEntity))
	{
		// A valid node route is available
		return true;
	}
	
	// No way to the target
	return false;
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::IsMoving( void ) const
{
	if(m_currentActivity == ACT_WALK || m_currentActivity == ACT_RUN
		|| m_currentActivity == ACT_WALK_HURT || m_currentActivity == ACT_RUN_HURT)
		return true;
	else
		return false;
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::BuildNodeRoute( const Vector& destination, CBaseEntity* pTargetEntity )
{
	static Int32 routeNodes[MAX_ROUTE_POINTS];

	if(!gNodeGraph.IsNodeGraphValid())
	{
		gd_engfuncs.pfnCon_Printf("AI Graph not initialized.\n");
		return false;
	}

	// Get start node
	Uint64 nodeTypeBits = Util::GetNodeTypeForNPC(this);
	Int32 startNode = gNodeGraph.GetNearestNode(m_pState->origin, nodeTypeBits, this, pTargetEntity);
	if(startNode == NO_POSITION)
		return false;

	// Get end node
	Int32 endNode = gNodeGraph.GetNearestNode(destination, nodeTypeBits, this, pTargetEntity);
	if(endNode == NO_POSITION)
		return false;

	CNodeIgnoreList startIgnoreList;

	Int32 originalStartNode = startNode;
	Int32 numNodes = 0;
	node_hull_types_t hullType = Util::GetNodeHullForNPC(this);
	while(true)
	{
		numNodes = gNodeGraph.GetShortestPath(startNode, endNode, hullType, m_capabilityBits, routeNodes, this, nullptr);
		if(numNodes)
			break;

		startIgnoreList.AddNode(startNode);
		startNode = gNodeGraph.GetNearestNode(m_pState->origin, nodeTypeBits, this, pTargetEntity, -1.0f, &startIgnoreList);
		if(startNode == NO_POSITION)
			break;
	}

	if(!numNodes)
	{
		const CAINodeGraph::node_t* pStartNode = gNodeGraph.GetNode(originalStartNode);
		const CAINodeGraph::node_t* pEndNode = gNodeGraph.GetNode(endNode);

		if(pStartNode && pEndNode)
		{
			CArray<Vector> points;
			points.push_back(pStartNode->origin);
			points.push_back(pEndNode->origin);

			ShowRoute(&points[0], points.size());
		}

		Util::EntityConDPrintf(m_pEdict, "No path from node %d to %d.\n", originalStartNode, endNode);
		return false;
	}

	// Do not allow overflow
	if(numNodes > MAX_ROUTE_POINTS)
		numNodes = MAX_ROUTE_POINTS;

	// Final number of nodes
	Uint32 finalNumNodes = 0;

	// Copy nodes from result array
	for(Int32 i = 0; i < numNodes; i++)
	{
		const CAINodeGraph::node_t* pNode = gNodeGraph.GetNode(routeNodes[i]);

		route_point_t& point = m_routePointsArray[finalNumNodes];
		finalNumNodes++;

		point.position = pNode->origin;
		point.nodeindex = routeNodes[i];
		point.type = MF_TO_NODE;
	}

	if(numNodes < MAX_ROUTE_POINTS)
	{
		route_point_t& point = m_routePointsArray[finalNumNodes];
		finalNumNodes++;

		point.position = destination;
		point.nodeindex = endNode;
		point.type = MF_IS_GOAL;

		if(finalNumNodes < MAX_ROUTE_POINTS)
			m_routePointsArray[finalNumNodes].type = MF_NONE;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::FindRandomSearchSpot( void )
{
	const Float maxCheckDistance = 256;
	const Float maxChecksWithDistance = 4;
	const Float maxTotalChecks = 16;

	Float traveledDist = 0;
	Float largestDistance = 0;
	Vector bestSearchSpot;

	Uint32 numChecks = 0;
	while(true)
	{
		// Define a search angle randomly
		Vector searchAngle(0, Common::RandomLong(0, 36)*10, 0);
	
		Vector forward;
		Math::AngleVectors(searchAngle, &forward);

		// Try one side first
		Vector arriveSpot;
		WalkMoveTrace( m_pState->origin, forward, arriveSpot, NPC_MAX_ENEMY_SEARCH_DISTANCE, traveledDist );
		if(traveledDist > largestDistance)
		{
			bestSearchSpot = arriveSpot;
			largestDistance = traveledDist;
		}

		// Try the other next
		WalkMoveTrace( m_pState->origin, forward, arriveSpot, NPC_MAX_ENEMY_SEARCH_DISTANCE, traveledDist );
		if(traveledDist > largestDistance)
		{
			bestSearchSpot = arriveSpot;
			largestDistance = traveledDist;
		}

		if(largestDistance > maxCheckDistance 
			|| numChecks >= maxChecksWithDistance && largestDistance != 0 
			|| numChecks >= maxTotalChecks)
			break;

		numChecks++;
	}

	if(!largestDistance || !MoveToLocation(ACT_RUN, 0, bestSearchSpot))
	{
		m_lastEnemySeekPosition = bestSearchSpot;
		return false;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::FindUnseenNode( void )
{
	if(!gNodeGraph.IsNodeGraphValid())
	{
		gd_engfuncs.pfnCon_Printf("AI Graph not initialized.\n");
		return false;
	}

	// Make sure we're in the limit
	if(g_lastActiveIdleSearchNodeIndex >= gNodeGraph.GetNumNodes())
		g_lastActiveIdleSearchNodeIndex = 0;

	Vector eyesPosition = m_pState->origin + m_pState->view_offset;

	// Last best node we had
	Int32 lastBestNode = NO_POSITION;

	for(Int32 i = g_lastActiveIdleSearchNodeIndex; i < gNodeGraph.GetNumNodes(); i++)
	{
		const CAINodeGraph::node_t* pNode = gNodeGraph.GetNode(i);
		g_lastActiveIdleSearchNodeIndex = i + 1;

		// Randomly find a position near the player
		Vector enemyPosition = m_enemyLastKnownPosition;
		Float lkpDistance = (enemyPosition - pNode->origin).Length();
		if(lkpDistance > NPC_MAX_ENEMY_SEARCH_DISTANCE)
			continue;

		Vector nodeLook = pNode->origin + m_pState->view_offset;

		trace_t tr;
		Util::TraceLine(eyesPosition, nodeLook, true, false, true, m_pEdict, tr);

		if(!tr.noHit())
		{
			if(!CheckRoute(GetNavigablePosition(), pNode->origin, m_enemy))
				continue;

			if(m_lastEnemySeekPosition.IsZero())
			{
				lastBestNode = i;
				break;
			}
			else
			{
				Vector lastLook = m_lastEnemySeekPosition + m_pState->view_offset;
				Util::TraceLine(lastLook, nodeLook, true, false, true, m_pEdict, tr);

				if(!tr.noHit())
				{
					lastBestNode = i;
					break;
				}
			}
		}
	}

	if(lastBestNode != NO_POSITION)
	{
		const CAINodeGraph::node_t* pNode = gNodeGraph.GetNode(lastBestNode);
		if(MoveToLocation(ACT_RUN, NPC_DEFAULT_MOVE_WAIT_TIME, pNode->origin))
		{
			m_lastEnemySeekPosition = pNode->origin;
			return true;
		}
	}

	return false;
}

//=============================================
// @brief
//
//=============================================
Int32 CBaseNPC::FindHintNode( void )
{
	if(!gNodeGraph.IsNodeGraphValid())
	{
		gd_engfuncs.pfnCon_Printf("AI Graph not initialized.\n");
		return NO_POSITION;
	}

	for(Int32 i = 0; i < gNodeGraph.GetNumNodes(); i++)
	{
		Int32 nodeIndex = (i + g_lastActiveIdleSearchNodeIndex) % gNodeGraph.GetNumNodes();
		
		const CAINodeGraph::node_t* pNode = gNodeGraph.GetNode(nodeIndex);
		if(!pNode->hinttype)
			continue;

		if(!ValidateHintType(pNode->hinttype))
			continue;

		if(!pNode->hintactivity || FindActivity(pNode->hintactivity) == NO_SEQUENCE_VALUE)
			continue;

		Vector myLookOffset = m_pState->origin + m_pState->view_offset;
		Vector nodeLookOffset = pNode->origin + m_pState->view_offset;

		trace_t tr;
		Util::TraceLine(myLookOffset, nodeLookOffset, true, false, true, m_pEdict, tr);
		if(!tr.noHit())
			continue;

		g_lastActiveIdleSearchNodeIndex = nodeIndex + 1;
		return nodeIndex;
	}

	g_lastActiveIdleSearchNodeIndex = 0;
	return NO_POSITION;
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::FindClearNode( CBaseEntity* pBlockedEntity )
{
	if(!gNodeGraph.IsNodeGraphValid())
	{
		gd_engfuncs.pfnCon_Printf("AI Graph not initialized.\n");
		return false;
	}

	// Clear the last node if we're over the number
	Int32 numNodes = gNodeGraph.GetNumNodes();
	if(m_lastClearNode >= numNodes)
		m_lastClearNode = 0;

	Int32 lastClosestNodeIndex = NO_POSITION;
	Float lastClosestDistance = 0;

	for(Int32 i = 0; i < numNodes; i++)
	{
		Int32 nodeIndex = (i + m_lastClearNode) % gNodeGraph.GetNumNodes();
		
		const CAINodeGraph::node_t* pNode = gNodeGraph.GetNode(nodeIndex);
		if(!pNode->hinttype)
			continue;

		if(pBlockedEntity)
		{
			if(m_lastClearNode != i && IsFacing(pBlockedEntity, pNode->origin))
			{
				Float distance = (pNode->origin-m_pState->origin).Length();
				if(lastClosestNodeIndex == NO_POSITION || distance < lastClosestDistance)
				{
					lastClosestDistance = distance;
					lastClosestNodeIndex = nodeIndex;
				}
			}
		}
		else
		{
			Float distance = (pNode->origin-m_pState->origin).Length();
			if(lastClosestNodeIndex == NO_POSITION || distance < lastClosestDistance)
			{
				lastClosestDistance = distance;
				lastClosestNodeIndex = nodeIndex;
			}
		}
	}

	if(lastClosestNodeIndex != NO_POSITION)
	{
		m_lastClearNode = lastClosestNodeIndex;

		const CAINodeGraph::node_t* pNode = gNodeGraph.GetNode(lastClosestNodeIndex);
		if(!pNode->hinttype)
			return false;

		if(MoveToLocation(ACT_RUN, NPC_DEFAULT_MOVE_WAIT_TIME, pNode->origin))
			return true;
		else
			return false;
	}
	else
	{
		return false;
	}
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::ValidateHintType( Int32 hintType )
{
	return false;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::ReportAIState( void )
{
	CString msg;
	msg << "Entity " << GetEntityIndex();
	if(HasTargetName())
		msg << "(" << GetTargetName() << ")";

	msg << " - ";

	if(m_npcState < NB_AI_STATES)
		msg << "AI State: " << AI_STATE_NAMES[m_npcState];

	Int32 i = 0;
	while(ACTIVITYMAP[i].type)
	{
		if(ACTIVITYMAP[i].type == m_currentActivity)
		{
			msg << ", Activity: " << ACTIVITYMAP[i].name << ", ";
			break;
		}

		i++;
	}

	if(m_pSchedule)
	{
		const Char* pstrName = m_pSchedule->GetName();
		if(!pstrName || !qstrlen(pstrName))
			msg << "Unknown schedule, ";
		else
			msg << "Schedule '" << pstrName << "', ";

		msg << " Task is: " << m_pSchedule->GetTaskByIndex(m_scheduleTaskIndex).task << ", ";
	}
	else
	{
		// Shouldn't happen(unless we're dead?)
		msg << "No schedule, ";
	}

	if(m_enemy)
	{
		msg << "Enemy is " << m_enemy->GetClassName();
		if(m_enemy->HasTargetName())
			msg << "(" << m_enemy->GetTargetName() << ")";
		msg << ", ";
	}
	else
		msg << "No enemy, ";

	msg << "Health: " << m_pState->health << ", ";
	msg << "Yaw speed: " << m_pState->yawspeed << ", ";

	if(HasSpawnFlag(FL_NPC_PRISONER))
		msg << " Prisoner, ";
	if(HasSpawnFlag(FL_NPC_IDLE))
		msg << " Pre-idle, ";

	msg << "\n";

	Util::EntityConPrintf(m_pEdict, msg.c_str());
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::CheckNodeRoute( const Vector& startPosition, const Vector& endPosition, CBaseEntity* pTargetEntity )
{
	static Int32 routeNodes[MAX_ROUTE_POINTS];

	if(!gNodeGraph.IsNodeGraphValid())
	{
		gd_engfuncs.pfnCon_Printf("AI Graph not initialized.\n");
		return false;
	}

	Int32 startNode = gNodeGraph.GetNearestNode(startPosition, this, pTargetEntity);
	if(startNode == NO_POSITION)
		return false;

	Int32 endNode = gNodeGraph.GetNearestNode(endPosition, this, pTargetEntity);
	if(endNode == NO_POSITION)
		return false;

	node_hull_types_t nodeHull = Util::GetNodeHullForNPC(this);

	if(gNodeGraph.GetShortestPath(startNode, endNode, nodeHull, m_capabilityBits, nullptr, this, pTargetEntity) > 0)
		return true;
	else
		return false;
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::BuildNodeDetourRoute( const Vector& destination, CBaseEntity* pBlocker, CBaseEntity* pTargetEntity )
{
	if(!gNodeGraph.IsNodeGraphValid())
	{
		gd_engfuncs.pfnCon_Printf("AI Graph not initialized.\n");
		return false;
	}

	if(!pBlocker)
		return false;

	// Only check for NPCs
	if(!pBlocker->IsNPC() && !pBlocker->IsFuncBreakableEntity() && !pBlocker->IsFuncDoorEntity())
		return false;

	// Array to hold path nodes
	static Int32 nodeIndexesArray[MAX_ROUTE_POINTS];

	// Make sure we haven't tried already, or we could end up in an endless loop
	Int32 routePointIndex = m_routePointIndex;
	for(Int32 i = routePointIndex; i < MAX_ROUTE_POINTS; i++)
	{
		route_point_t& point = m_routePointsArray[i];
		
		// Break if we reached the end
		if(!point.type)
			break;

		// If any of the waypoints have a detour flag,
		// then don't try again
		if(point.type & MF_DETOUR_PATH)
			return false;
	}

	Vector blockerMins = pBlocker->GetAbsMins();
	Vector blockerMaxs = pBlocker->GetAbsMaxs();

	// Add in our mins/maxs as well
	Math::VectorAdd(blockerMins, m_pState->mins, blockerMins);
	Math::VectorAdd(blockerMaxs, m_pState->mins, blockerMaxs);

	// See which points of the route are in the blocker's vicinity
	for(Int32 i = routePointIndex; i < MAX_ROUTE_POINTS; i++)
	{
		route_point_t& point = m_routePointsArray[i];
		if(!point.type)
			break;

		if(!Math::PointInMinsMaxs(point.position, blockerMins, blockerMaxs))
			break;

		// If goal is in the blocker's vicinity, we can't build a detour
		if(point.type & MF_IS_GOAL)
			return false;

		routePointIndex = i;
	}

	// Find any nodes touched by the blocker
	CNodeIgnoreList ignoreList;
	for(Int32 i = 0; i < gNodeGraph.GetNumNodes(); i++)
	{
		const CAINodeGraph::node_t* pNodeCheck = gNodeGraph.GetNode(i);
		if(Math::PointInMinsMaxs(pNodeCheck->origin, blockerMins, blockerMaxs))
			ignoreList.AddNode(pNodeCheck->index);
	}

	// Get start node
	Uint64 nodeTypeBits = Util::GetNodeTypeForNPC(this);
	Int32 startNode = gNodeGraph.GetNearestNode(m_pState->origin, nodeTypeBits, this, pTargetEntity, -1.0f, &ignoreList);
	if(startNode == NO_POSITION)
		return false;

	// Get end node
	Int32 endNode = gNodeGraph.GetNearestNode(destination, nodeTypeBits, this, pTargetEntity, -1.0f, &ignoreList);
	if(endNode == NO_POSITION)
		return false;

	node_hull_types_t hullType = Util::GetNodeHullForNPC(this);
	Int32 numNodes = gNodeGraph.GetShortestPath(startNode, endNode, hullType, m_capabilityBits, nodeIndexesArray, this, nullptr, &ignoreList);
	if(!numNodes)
	{
		Util::EntityConDPrintf(m_pEdict, "No path from node %d to %d.\n", startNode, endNode);
		return false;
	}

	// Do an extended check, to see if the destination is actually accessible from the last node
	const CAINodeGraph::node_t* pNode = gNodeGraph.GetNode(nodeIndexesArray[numNodes-1]);
	if((pNode->origin - destination).Length() < NPC_CORNER_CUT_MIN_DIST)
	{
		if(numNodes < 2)
			return false;

		pNode = gNodeGraph.GetNode(nodeIndexesArray[numNodes-2]);
	}

	if(pNode)
	{
		localmove_t moveresult = CheckLocalMove(pNode->origin, destination, pTargetEntity);
		if(moveresult < LOCAL_MOVE_RESULT_FAILURE)
			return false;
	}

	// Build an entirely new route
	static route_point_t routePointsArray[MAX_ROUTE_POINTS];

	Int32 pointIndex = 0;
	for(Int32 i = 0; i < numNodes; i++)
	{
		route_point_t& point = routePointsArray[pointIndex];
		pointIndex++;

		const CAINodeGraph::node_t* pNodeCheck = gNodeGraph.GetNode(nodeIndexesArray[i]);

		point.nodeindex = nodeIndexesArray[i];
		point.position = pNodeCheck->origin;
		point.type = (MF_TO_NODE|MF_DETOUR_PATH);
	}

	// Find last position
	Int32 lastPointIndex = 0;
	for(Int32 i = 0; i < MAX_ROUTE_POINTS; i++)
	{
		route_point_t& point = m_routePointsArray[i];
		if(!point.type)
			break;

		lastPointIndex++;

		if(point.type & MF_IS_GOAL)
			break;
	}

	// Add the rest of the nodes in
	Int32 numAdd = (lastPointIndex - routePointIndex) + numNodes;
	if(numAdd >= MAX_ROUTE_POINTS)
		return false;

	for(Int32 i = routePointIndex; i < lastPointIndex; i++)
	{
		route_point_t& point = routePointsArray[pointIndex];
		pointIndex++;

		point = m_routePointsArray[routePointIndex];
	}

	for(Int32 i = 0; i < pointIndex; i++)
		m_routePointsArray[i] = routePointsArray[i];

	// Set last point
	if(pointIndex < MAX_ROUTE_POINTS)
		m_routePointsArray[pointIndex].type = MF_NONE;

	m_routePointIndex = 0;
	m_firstNodeIndex = nodeIndexesArray[0];

	// Show debug info
	ShowRoute(false, MAX_ROUTE_POINTS, m_movementGoalPosition);
	ShowRoute(true, m_routePointIndex+numNodes, ZERO_VECTOR);

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::AttemptTriangulation( const Vector& startPosition, const Vector& endPosition, Float distance, CBaseEntity* pTargetEntity, Vector* pApexPosition, bool isTest )
{
	// Use a minimum of 24, because CheckLocalMove uses that size
	Float sizeX = m_pState->size.x;
	sizeX = clamp(sizeX, NPC_TRIANGULATION_MIN_SIZE_X, NPC_TRIANGULATION_MAX_SIZE_X);
	Float sizeZ = m_pState->size.z;

	Vector forwardDirection = (endPosition-startPosition);
	forwardDirection[2] = 0;
	forwardDirection.Normalize();

	Vector upDirection = Vector(0, 0, 1); // This is fixed
	
	Vector rightDirection;
	Math::CrossProduct(forwardDirection, upDirection, rightDirection);

	// Make sure distance isn't too short
	Float travelledDistance = distance;
	Float destinationDistance = (endPosition-startPosition).Length();
	if((travelledDistance/destinationDistance) < 0.3)
		travelledDistance = destinationDistance * 0.5;

	// Calculate test positions for triangulation
	Vector testRight = m_pState->origin + (forwardDirection * (travelledDistance+sizeX)) + rightDirection * (sizeX*3);
	Vector testLeft = m_pState->origin + (forwardDirection * (travelledDistance+sizeX)) - rightDirection * (sizeX*3);

	// Do extra calculations for movetype_fly
	Vector testUp, testDown;
	if(m_pState->movetype == MOVETYPE_FLY)
	{
		testUp = m_pState->origin + (forwardDirection*travelledDistance) + (upDirection*sizeZ*3);
		testDown = m_pState->origin + (forwardDirection*travelledDistance) - (upDirection*sizeZ*3);
	}

	// Far side depends on isTest parameter
	Vector destinationPosition;
	if(isTest)
		destinationPosition = endPosition;
	else
		destinationPosition = m_routePointsArray[m_routePointIndex].position;

	Math::VectorScale(rightDirection, sizeX*2, rightDirection);
	if(m_pState->movetype == MOVETYPE_FLY)
		Math::VectorScale(upDirection, sizeZ*2, upDirection);

	for(Uint32 i = 0; i < 8; i++)
	{
		// Try right first
		localmove_t moveResult = CheckLocalMove(m_pState->origin, testRight, pTargetEntity);
		if(moveResult > LOCAL_MOVE_RESULT_FAILURE)
		{
			// See if we can make it to the far side
			moveResult = CheckLocalMove(testRight, destinationPosition, pTargetEntity);
			if(moveResult > LOCAL_MOVE_RESULT_FAILURE)
			{
				if(pApexPosition)
					(*pApexPosition) = testRight;

				// Triangulation was successful
				return true;
			}
		}

		// Test left next
		moveResult = CheckLocalMove(m_pState->origin, testLeft, pTargetEntity);
		if(moveResult > LOCAL_MOVE_RESULT_FAILURE)
		{
			// See if we can make it to the far side
			moveResult = CheckLocalMove(testLeft, destinationPosition, pTargetEntity);
			if(moveResult > LOCAL_MOVE_RESULT_FAILURE)
			{
				if(pApexPosition)
					(*pApexPosition) = testLeft;

				// Triangulation was successful
				return true;
			}
		}

		// Do extra checks for flying npcs
		if(m_pState->movetype == MOVETYPE_FLY)
		{
			// Try up first
			moveResult = CheckLocalMove(m_pState->origin, testUp, pTargetEntity);
			if(moveResult > LOCAL_MOVE_RESULT_FAILURE)
			{
				// See if we can make it to the far side
				moveResult = CheckLocalMove(testUp, destinationPosition, pTargetEntity);
				if(moveResult > LOCAL_MOVE_RESULT_FAILURE)
				{
					if(pApexPosition)
						(*pApexPosition) = testUp;

					// Triangulation was successful
					return true;
				}
			}

			// Test down next
			moveResult = CheckLocalMove(m_pState->origin, testDown, pTargetEntity);
			if(moveResult > LOCAL_MOVE_RESULT_FAILURE)
			{
				// See if we can make it to the far side
				moveResult = CheckLocalMove(testDown, destinationPosition, pTargetEntity);
				if(moveResult > LOCAL_MOVE_RESULT_FAILURE)
				{
					if(pApexPosition)
						(*pApexPosition) = testDown;

					// Triangulation was successful
					return true;
				}
			}
		}

		Math::VectorAdd(testRight, rightDirection, testRight);
		Math::VectorSubtract(testLeft, rightDirection, testLeft);

		// Do extra checks for flying npcs
		if(m_pState->movetype == MOVETYPE_FLY)
		{
			Math::VectorAdd(testUp, upDirection, testUp);
			Math::VectorSubtract(testDown, upDirection, testDown);
		}
	}

	return false;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::ShowRoute( bool isDetour, Uint32 maxPath, const Vector& destination )
{
	// Only send msgs if we actually have the cvar enabled
	if(gd_engfuncs.pfnGetCvarFloatValue(NODE_DEBUG_CVAR_NAME) < 1)
		return;

	CArray<Vector> pointsArray;

	// Add current position
	pointsArray.push_back(m_pState->origin);

	// Count the number of points
	Uint32 i = m_routePointIndex;
	for(; i < maxPath; i++)
	{
		if(!m_routePointsArray[i].type)
			break;

		pointsArray.push_back(m_routePointsArray[i].position);
	}

	// Add the destination if it's not a detour and doesn't match last point
	if(!isDetour && m_routePointsArray[i].position != destination)
		pointsArray.push_back(destination);

	gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.nodedebug, nullptr, nullptr);
	gd_engfuncs.pfnMsgWriteByte(NODE_DEBUG_WAYPOINT);
	gd_engfuncs.pfnMsgWriteByte(isDetour ? WAYPOINT_DETOUR : WAYPOINT_NORMAL);
	gd_engfuncs.pfnMsgWriteInt32(m_pState->entindex);
	gd_engfuncs.pfnMsgWriteUint16(pointsArray.size());
	for(i = 0; i < pointsArray.size(); i++)
	{
		gd_engfuncs.pfnMsgWriteFloat(pointsArray[i].x);
		gd_engfuncs.pfnMsgWriteFloat(pointsArray[i].y);
		gd_engfuncs.pfnMsgWriteFloat(pointsArray[i].z);
	}
	gd_engfuncs.pfnUserMessageEnd();
}

//=============================================
// @brief Shows the current route
//
//=============================================
void CBaseNPC::ShowRoute( const Vector* pPoints, Uint32 numPoints )
{
	// Only send msgs if we actually have the cvar enabled
	if(gd_engfuncs.pfnGetCvarFloatValue(NODE_DEBUG_CVAR_NAME) < 1)
		return;

	gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.nodedebug, nullptr, nullptr);
	gd_engfuncs.pfnMsgWriteByte(NODE_DEBUG_WAYPOINT);
	gd_engfuncs.pfnMsgWriteByte(WAYPOINT_NORMAL);
	gd_engfuncs.pfnMsgWriteInt32(m_pState->entindex);
	gd_engfuncs.pfnMsgWriteUint16(numPoints);
	for(Uint32 i = 0; i < numPoints; i++)
	{
		gd_engfuncs.pfnMsgWriteFloat(pPoints[i].x);
		gd_engfuncs.pfnMsgWriteFloat(pPoints[i].y);
		gd_engfuncs.pfnMsgWriteFloat(pPoints[i].z);
	}
	gd_engfuncs.pfnUserMessageEnd();
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::ShowErrorPath( const Vector& startPosition, const Vector& endPosition )
{
	// Only send msgs if we actually have the cvar enabled
	if(gd_engfuncs.pfnGetCvarFloatValue(NODE_DEBUG_CVAR_NAME) < 1)
		return;

	gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.nodedebug, nullptr, nullptr);
	gd_engfuncs.pfnMsgWriteByte(NODE_DEBUG_WAYPOINT);
	gd_engfuncs.pfnMsgWriteByte(WAYPOINT_ERROR);
	gd_engfuncs.pfnMsgWriteInt32(m_pState->entindex);
	gd_engfuncs.pfnMsgWriteUint16(2);
	gd_engfuncs.pfnMsgWriteFloat(startPosition.x);
	gd_engfuncs.pfnMsgWriteFloat(startPosition.y);
	gd_engfuncs.pfnMsgWriteFloat(startPosition.z);
	gd_engfuncs.pfnMsgWriteFloat(endPosition.x);
	gd_engfuncs.pfnMsgWriteFloat(endPosition.y);
	gd_engfuncs.pfnMsgWriteFloat(endPosition.z);
	gd_engfuncs.pfnUserMessageEnd();
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::SetPathBlocked( CBaseEntity* pBlockedEntity, const Vector& destination )
{
	// Don't allow pushing if this flag's set
	if(HasSpawnFlag(FL_NPC_NO_PUSHING))
		return;

	// Do not allow in combat
	if(m_npcState == NPC_STATE_COMBAT)
		return;

	// If we already are blocking someone, don't change
	if(m_blockedNPC && CheckCondition(AI_COND_BLOCKING_PATH))
		return;

	m_blockedNPC = pBlockedEntity;
	m_blockedNPCDestination = destination;
	SetCondition(AI_COND_BLOCKING_PATH);
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::NudgePlayer( CBaseEntity* pPlayer ) const
{
	// Determine the move direction
	const route_point_t& point = m_routePointsArray[m_routePointIndex];
	Vector moveDirection = (point.position-m_pState->origin);
	moveDirection[2] = 0;
	moveDirection.Normalize();

	// Get the right vector
	Vector upDirection(0, 0, 1);
	Vector rightDirection;
	Math::CrossProduct(moveDirection, upDirection, rightDirection);

	Int32 forwardSign = 0;
	Int32 directionSign = 1;

	// Determine hull type
	hull_types_t hullType = (pPlayer->GetFlags() & FL_DUCKING) ? HULL_SMALL : HULL_HUMAN;

	Vector blockerOrigin = pPlayer->GetOrigin();
	Vector checkPosition = blockerOrigin + rightDirection * 64;

	// Check rightwards first
	trace_t tr;
	Util::TraceHull(blockerOrigin, checkPosition, false, false, false, hullType, pPlayer->GetEdict(), tr);
	if(tr.noHit() || tr.startSolid())
	{
		// Determine if going the opposite would just take the player inwards
		Vector testPosition = (blockerOrigin - rightDirection * 32);
		Vector testDirection = (testPosition - m_pState->origin);
		testDirection[2] = 0;
		testDirection.Normalize();

		Vector blockerDirection = (blockerOrigin - m_pState->origin);
		blockerDirection[2] = 0;
		blockerDirection.Normalize();

		if(Math::DotProduct(moveDirection, blockerDirection) > Math::DotProduct(moveDirection, testDirection))
		{
			// Store previous fraction
			Double rightFraction = tr.fraction;

			checkPosition = blockerOrigin - rightDirection * 64;
			Util::TraceHull(blockerOrigin, checkPosition, false, false, false, hullType, pPlayer->GetEdict(), tr);
			if(tr.noHit() && !tr.startSolid() || tr.fraction > rightFraction)
				directionSign = -1;
		}
	}

	// Push the player forward if there's not enough space either way
	if(tr.fraction < 0.25)
	{
		checkPosition = blockerOrigin + moveDirection * 32;
		Util::TraceHull(blockerOrigin, checkPosition, false, false, false, hullType, pPlayer->GetEdict(), tr);

		if(tr.fraction > 0.25)
			forwardSign = 1;
		else
			forwardSign = -1;
	}

	// Give the player a bit of a push
	Vector newVelocity = pPlayer->GetVelocity() + rightDirection * directionSign * 320 + moveDirection * forwardSign * 320;
	pPlayer->SetVelocity(newVelocity);
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::AttemptCircleAround( bool isRetry, const Vector& destination, CBaseEntity* pBlocker, CBaseEntity* pTargetEntity )
{
	bool fullCircle = false;
	bool isSuccessful = false;

	Float movedDistance = 0;
	Float sideMoveDistance = 0;

	// Get current destination
	route_point_t& point = m_routePointsArray[m_routePointIndex];

	// Get distance to target
	Float distance = (m_pState->origin - destination).Length2D();
	if( distance < 32 )
	{
		if(!isRetry && !(point.type & MF_IS_GOAL))
		{
			// Try skipping to the next spot if current one isn't the goal
			const route_point_t& nextPoint = m_routePointsArray[m_routePointIndex + 1];
			return AttemptCircleAround(true, nextPoint.position, pBlocker, pTargetEntity);
		}
		else
		{
			// Invalid
			return false;
		}
	}

	// Forward vector to destination
	Vector forwardDirection = ( destination - m_pState->origin );
	forwardDirection[2] = 0;
	forwardDirection = forwardDirection.Normalize();

	// Get right vector
	Vector rightDirection;
	Vector upDirection(0, 0, 1);
	Math::CrossProduct(forwardDirection, upDirection, rightDirection);

	// See where we end up if we go right
	Vector vecEndSide, vecEndForward, vecEndBack;
	WalkMoveTrace(m_pState->origin, rightDirection, vecEndSide, 128, sideMoveDistance);

	if( sideMoveDistance >= 16 )
	{
		// See how far get if we go forward from here
		WalkMoveTrace(vecEndSide, forwardDirection, vecEndForward, distance, movedDistance);

		if(movedDistance >= 32)
		{
			// Check if we can reach the destination from here
			if(CheckLocalMove(vecEndForward, destination, pTargetEntity, nullptr) < LOCAL_MOVE_RESULT_FAILURE)
			{
				// Try moving back to the same line
				WalkMoveTrace(vecEndForward, -rightDirection, vecEndBack, sideMoveDistance, movedDistance);
				if(movedDistance >= 16)
				{
					if(CheckLocalMove(vecEndBack, destination, pTargetEntity, nullptr) > LOCAL_MOVE_RESULT_FAILURE)
					{
						isSuccessful = true;
						fullCircle = true;
					}
				}
			}
			else
			{
				// We can get there
				isSuccessful = true;
			}
		}
	}

	// If we failed, try the other way around
	if(!isSuccessful)
	{
		// Try going left first
		WalkMoveTrace(m_pState->origin, -rightDirection, vecEndSide, 128, sideMoveDistance);

		if(sideMoveDistance >= 16)
		{
			// See how far get if we go forward from here
			WalkMoveTrace(vecEndSide, forwardDirection, vecEndForward, distance, movedDistance);

			if(movedDistance >= 32)
			{
				// Check if we can reach the destination from here
				if(CheckLocalMove(vecEndForward, destination, pTargetEntity, nullptr) < LOCAL_MOVE_RESULT_FAILURE)
				{
					// Try moving back to the same line
					WalkMoveTrace(vecEndForward, rightDirection, vecEndBack, sideMoveDistance, movedDistance);
					if(movedDistance >= 16)
					{
						if(CheckLocalMove(vecEndBack, destination, pTargetEntity, nullptr) > LOCAL_MOVE_RESULT_FAILURE)
						{
							isSuccessful = true;
							fullCircle = true;
						}
					}
				}
				else
				{
					// We can get there
					isSuccessful = true;
				}
			}
		}
	}

	// Insert the waypoints we've collected
	if(isSuccessful)
	{
		if(!isRetry)
		{
			// Mark the destination also as a circle around path
			point.type = (MF_DETOUR_PATH|MF_CIRCLE_PATH);
			// Just insert it into the stack
			InsertRoutePoint( m_routePointIndex, vecEndSide, NO_POSITION, (MF_DETOUR_PATH|MF_CIRCLE_PATH) );
		}
		else
		{
			// Skip the current index
			point.position = vecEndSide;
			point.type = (MF_DETOUR_PATH|MF_CIRCLE_PATH);
			point.nodeindex = NO_POSITION;
		}

		InsertRoutePoint( m_routePointIndex+1, vecEndForward, NO_POSITION, (MF_DETOUR_PATH|MF_CIRCLE_PATH) );

		if(fullCircle)
			InsertRoutePoint( m_routePointIndex+2, vecEndBack, NO_POSITION, (MF_DETOUR_PATH|MF_CIRCLE_PATH) );

		return true;
	}
	else if(!isRetry && !(point.type & MF_IS_GOAL) )
	{
		// Try skipping to the next spot if current one isn't the goal
		point = m_routePointsArray[m_routePointIndex + 1];
		return AttemptCircleAround(true, point.position, pBlocker, pTargetEntity);
	}

	return false;
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::AttemptShiftDestination( CBaseEntity* pTargetEntity )
{
	route_point_t& point = m_routePointsArray[m_routePointIndex];
	if(!point.type)
		return false;

	// Get the direction to the destination
	Vector destination = point.position;

	// Make sure the node we'll be checking is valid
	Int32 nextNode = m_routePointIndex+1;
	for(; nextNode < MAX_ROUTE_POINTS; nextNode++)
	{
		const route_point_t& nextPoint = m_routePointsArray[nextNode];
		Vector nextPosition = nextPoint.position;

		Float pathLength = (destination - nextPosition).Length();
		if(pathLength)
			break;
	}

	// Get the direction to the destination
	const route_point_t& nextPoint = m_routePointsArray[nextNode];
	Vector vecDir = (nextPoint.position - destination).Normalize(); 
	Float distance = (nextPoint.position - destination).Length2D();

	// Test if we can get there
	Float moveDistance = NPC_STEP_SIZE;
	while( moveDistance < distance )
	{
		Vector testPosition = destination + vecDir * NPC_STEP_SIZE;
		if(CheckLocalMove(m_pState->origin, testPosition, pTargetEntity) > LOCAL_MOVE_RESULT_FAILURE)
		{
			point.position = testPosition;
			return true;
		}

		moveDistance += NPC_STEP_SIZE;
	}

	return false;
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::HandleBlockage( CBaseEntity* pBlocker, CBaseEntity* pTargetEntity, Float moveDistance )
{
	// Get reference to destination
	const route_point_t& currentPoint = m_routePointsArray[m_routePointIndex];

	// Handle entities we can manipulate here
	if(pBlocker)
	{
		// If it's a player, and we're scripted, push him aside
		if(pBlocker->IsPlayer() && !pBlocker->IsMoving()
			&& m_npcState == NPC_STATE_SCRIPT
			&& moveDistance < MIN_LOCALMOVE_CHECK_DIST)
		{
			// Try to nudge the player
			NudgePlayer(pBlocker);
			// Move forward if the distance is big enough
			return (moveDistance > MIN_LOCALMOVE_CHECK_DIST*0.25) ? true : false;
		}

		// If it's a door, try opening it
		if(pBlocker->IsFuncDoorEntity() 
			&& !pBlocker->HasSpawnFlag(CFuncDoor::FL_NO_NPCS)
			&& !pBlocker->IsLockedByMaster())
		{
			if(pBlocker->GetToggleState() != TS_AT_TOP)
			{
				if(pBlocker->HasTargetName())
				{
					// Trigger the door
					const Char* pstrTargetName = pBlocker->GetTargetName();
					Util::FireTargets(pstrTargetName, this, this, USE_TOGGLE, 0);

					// Look for any trigger_multiples targeting this entity
					edict_t* pEdict = nullptr;
					while(true)
					{
						pEdict = Util::FindEntityByTarget(pEdict, pstrTargetName);
						if(!pEdict)
							break;

						CBaseEntity* pEntity = CBaseEntity::GetClass(pEdict);
						if(pEntity->IsTriggerMultipleEntity())
						{
							pEntity->SetTriggerWait();
							break;
						}
					}
				}
				else
				{
					// Non-linked door
					pBlocker->CallUse(this, this, USE_TOGGLE, 0);
				}

				m_moveWaitFinishTime = g_pGameVars->time + m_moveWaitTime;
				return true;
			}
		}

		// Try shifting the destination a bit
		if(pBlocker->IsNPC() 
			&& !(currentPoint.type & MF_IS_GOAL) 
			&& AttemptShiftDestination(pTargetEntity))
		{
			ShowRoute(false, MAX_ROUTE_POINTS, m_movementGoalPosition);
			return true;
		}
	}

	// Try to triangulate around it first
	Vector apexPosition;
	if(!(currentPoint.type & MF_TO_DETOUR) 
		&& AttemptTriangulation(m_pState->origin, currentPoint.position, moveDistance, pTargetEntity, &apexPosition))
	{
		InsertRoutePoint(apexPosition, NO_POSITION, MF_TO_DETOUR);
		SimplifyRoute(pTargetEntity);

		ShowRoute(false, MAX_ROUTE_POINTS, m_movementGoalPosition);
		return true;
	}

	// Try circling around it, if it's an NPC, but only if we couldn't triangulate
	if(pBlocker && pBlocker->IsNPC() && !pBlocker->IsMoving()
		&& !(currentPoint.type & MF_CIRCLE_PATH)
		&& AttemptCircleAround(false, currentPoint.position, pBlocker, pTargetEntity))
	{
		ShowRoute(false, MAX_ROUTE_POINTS, m_movementGoalPosition);
		return true;
	}

	// Try building a detour around our blocker
	if(BuildNodeDetourRoute(currentPoint.position, pBlocker, pTargetEntity))
		return true;

	// Try to manage the blocker, if he's a valid entity
	if(pBlocker && pBlocker->IsNPC() && !(pBlocker->HasSpawnFlag(FL_NPC_NO_PUSHING)))
	{
		// Tell our friendly that he's in the way, then wait for a bit
		if( !HasMemory(AI_MEMORY_MOVE_FAILED)  && m_npcState != NPC_STATE_SCRIPT && m_npcState != NPC_STATE_COMBAT )
		{
			// Neither we nor the blocker are in combat/script modes, so tell him and wait
			if( !pBlocker->IsPlayer() && pBlocker->IsAlive() && pBlocker->GetClassification() == GetClassification() 
				&& pBlocker->GetNPCState() != NPC_STATE_SCRIPT 
				&& pBlocker->GetNPCState() != NPC_STATE_COMBAT )
			{
				pBlocker->SetPathBlocked( this, currentPoint.position );
				SetMemory( AI_MEMORY_MOVE_FAILED );

				// Wait for a second
				m_moveWaitFinishTime = g_pGameVars->time + m_moveWaitTime;
				return TRUE;
			}
		}
		else if( !pBlocker->IsPlayer() && pBlocker->IsAlive() 
			&& pBlocker->GetClassification() == GetClassification() 
			&& pBlocker->GetNPCState() != NPC_STATE_SCRIPT 
			&& pBlocker->GetNPCState() != NPC_STATE_COMBAT )
		{
			// We're in combat mode or script mode, so tell the friendly, but don't wait for him
			pBlocker->SetPathBlocked( this, currentPoint.position );
			SetMemory( AI_MEMORY_MOVE_FAILED );
		}
	}

	// Any other entities
	if( pBlocker )
	{
#if 0
		// If it's a breakable, shoot it
		if(pBlocker->IsFuncBreakableEntity())
		{
			// TODO doesn't work
			PushEnemy(pBlocker, pBlocker->GetCenter(), pBlocker->GetAngles());
			return true;
		}
#endif
		// Wait for any moving entity
		if( pBlocker->IsMoving() 
			&& !CheckCondition(AI_COND_BLOCKING_PATH) 
			&& m_npcState != NPC_STATE_SCRIPT 
			&& m_npcState != NPC_STATE_COMBAT )
		{
			// No, Wait for a second
			m_moveWaitFinishTime = g_pGameVars->time + m_moveWaitTime;
			return true;
		}
	}

	return false;
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::CheckMoveResult( localmove_t moveResult, Float moveDistance, const Vector& vectorToTarget )
{
	// Check if there's an error at all
	if(moveResult > LOCAL_MOVE_RESULT_FAILURE)
		return true;

	// Get the blocking entity
	CBaseEntity* pBlocker = nullptr;
	edict_t* pBlockerEdict = nullptr;
	if(g_pGameVars->globaltrace.hitentity != NO_ENTITY_INDEX)
	{
		pBlockerEdict = gd_engfuncs.pfnGetEdictByIndex(g_pGameVars->globaltrace.hitentity);
		if(pBlockerEdict && !Util::IsNullEntity(pBlockerEdict))
			pBlocker = CBaseEntity::GetClass(pBlockerEdict);
	}

	// If we're still far from the npc, wait before failing
	if(pBlocker && (pBlocker->IsNPC() || pBlocker->IsPlayer() && m_npcState == NPC_STATE_SCRIPT)
		&& moveDistance > (MIN_LOCALMOVE_CHECK_DIST*0.5))
		return true;

	// Call blocked function
	if(pBlocker && pBlockerEdict && !pBlocker->IsWorldSpawn())
	{
		DispatchBlocked(m_pEdict, pBlockerEdict);
		Util::EntityConDPrintf(m_pEdict, "Attempting to process '%s'.\n", pBlocker->GetClassName());
	}

	// Tell the debugger where we failed
	Vector endPosition;
	Math::VectorMA(m_pState->origin, g_pGameVars->globaltrace.fraction, vectorToTarget, endPosition);
	ShowErrorPath(m_pState->origin, endPosition);

	// Try handling the blocking entity somehow
	if(HandleBlockage(pBlocker, m_movementGoalEntity, moveDistance))
		return true;

	// StopMovement the NPC
	StopMovement();

	if(m_moveWaitTime && !HasMemory(AI_MEMORY_MOVE_FAILED))
	{
		// Try refreshing the route
		RefreshRoute();

		if(IsRouteClear())
		{
			// Fail our current task
			SetTaskFailed();
		}
		else
		{
			SetMemory(AI_MEMORY_MOVE_FAILED);
			m_moveWaitFinishTime = g_pGameVars->time + 0.1;
		}
	}
	else
	{
		SetTaskFailed();
		Util::EntityConDPrintf(m_pEdict, "NPC failed to move.\n");
	}

	return false;
}

//=============================================
// @brief
//
//=============================================
flexaistates_t CBaseNPC::GetFlexAIState( void )
{
	if(m_npcState == NPC_STATE_COMBAT
		&& m_enemy && m_enemy->IsNPCDangerous())
	{
		// Make NPC act terrified
		return FLEX_AISTATE_AFRAID;
	}

	switch(m_npcState)
	{
	case NPC_STATE_ALERT:
	case NPC_STATE_SCRIPT:
	case NPC_STATE_IDLE:
		return FLEX_AISTATE_IDLE;
	case NPC_STATE_COMBAT:
		return FLEX_AISTATE_COMBAT;
	case NPC_STATE_DEAD:
		return FLEX_AISTATE_DEAD;
	default:
		return FLEX_AISTATE_NONE;
	}
}

//=============================================
// @brief
//
//=============================================
flextypes_t CBaseNPC::GetFlexNPCType( void )
{
	return FLEX_NPC_TYPE_HUMAN;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::UpdateExpressions( void )
{
	// Only do this if we have expression abilities
	if(!HasCapability(AI_CAP_EXPRESSIONS))
		return;

	// Probably after a reload, wait a bit
	if(m_activeFlexState == FLEX_AISTATE_NONE
		&& m_flexScriptDuration > g_pGameVars->time)
		return;

	// Scripts are handled specially
	if(m_activeFlexState == FLEX_AISTATE_SCRIPT)
	{
		// Wait until another script cancels this one
		if(m_flexScriptFlags & (FLEX_FLAG_STAY|FLEX_FLAG_LOOP))
			return;

		// Don't let state changes mangle this
		if(m_flexScriptDuration >= g_pGameVars->time)
			return;
	}

	// Switch scripts if our AI state has changed
	if(GetFlexAIState() != m_activeFlexState)
	{
		SetIdealExpression();
		return;
	}

	// Don't update if on a loop
	if(m_flexScriptFlags & FLEX_FLAG_LOOP)
		return;

	// Get another ideal script if this one expired
	if(m_flexScriptDuration < g_pGameVars->time)
	{
		SetIdealExpression();
		return;
	}
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::SetIdealExpression( void )
{
	if(!g_pFlexManager)
		return;

	// Get NPC type and AI state
	flexaistates_t aistate = GetFlexAIState(); 
	if(aistate == FLEX_AISTATE_NONE)
		return;

	flextypes_t npctype = GetFlexNPCType();

	// Get the script name
	const Char* pstrscript = g_pFlexManager->GetAIScript(npctype, aistate);
	if(!pstrscript || !qstrlen(pstrscript))
	{
		// Don't try again for a while
		m_flexScriptDuration = g_pGameVars->time + 5;
		Util::EntityConPrintf(m_pEdict, "NPC has no valid flex scripts for AI state %d.\n", (Int32)aistate);
		return;
	}

	const flexscript_t* pscript = g_pFlexManager->LoadScript(pstrscript);
	if(!pscript)
	{
		// Don't try again for a while
		m_flexScriptDuration = g_pGameVars->time + 5;
		Util::EntityConPrintf(m_pEdict, "%s.\n", g_pFlexManager->GetError());
		return;
	}

	gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.setentityflexscript, nullptr, nullptr);
		gd_engfuncs.pfnMsgWriteInt16(m_pEdict->entindex);
		gd_engfuncs.pfnMsgWriteString(pstrscript);
	gd_engfuncs.pfnUserMessageEnd();

	// Set relevant info
	m_flexScriptDuration = g_pGameVars->time + pscript->duration;
	m_flexScriptFlags = pscript->flags;

	m_activeFlexState = aistate;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::PlayFlexScript( const Char* pstrSentenceName )
{
	if(!g_pFlexManager)
		return;

	// Only do this if we have expression abilities
	if(!HasCapability(AI_CAP_EXPRESSIONS))
		return;

	if(!pstrSentenceName || !qstrlen(pstrSentenceName))
		return;

	const Char* pstrscript = g_pFlexManager->GetSentenceScript(pstrSentenceName[0] == '!' ? pstrSentenceName+1 : pstrSentenceName);
	if(!pstrscript)
	{
		Util::EntityConPrintf(m_pEdict, "NPC has no flex script for sentence '%s'.\n", pstrSentenceName);
		return;
	}

	const flexscript_t* pscript = g_pFlexManager->LoadScript(pstrscript);
	if(!pscript)
	{
		// Don't try again for a while
		m_flexScriptDuration = g_pGameVars->time + 5;
		Util::EntityConPrintf(m_pEdict, "%s.\n", g_pFlexManager->GetError());
		return;
	}

	gd_engfuncs.pfnUserMessageBegin(MSG_ALL, g_usermsgs.setentityflexscript, nullptr, nullptr);
		gd_engfuncs.pfnMsgWriteInt16(m_pEdict->entindex);
		gd_engfuncs.pfnMsgWriteString(pstrscript);
	gd_engfuncs.pfnUserMessageEnd();

	// Set relevant info
	m_flexScriptDuration = g_pGameVars->time + pscript->duration;
	m_flexScriptFlags = pscript->flags;

	m_activeFlexState = FLEX_AISTATE_SCRIPT;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::NPCDeadThink( void )
{
	// Set the interval for thinking
	m_thinkIntervalTime = g_pGameVars->time - m_npcLastThinkTime;
	// Set the current time
	m_npcLastThinkTime = g_pGameVars->time;

	// Set next think time
	m_pState->nextthink = g_pGameVars->time + NPC_THINK_TIME;

	// Check for gibbing from explode death mode
	if(m_deathMode == DEATH_EXPLODE 
		&& m_deathExplodeTime <= g_pGameVars->time)
	{
		CallGibNPC();
		return;
	}
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::NPCThink( void )
{
	// Set the interval for thinking
	m_thinkIntervalTime = g_pGameVars->time - m_npcLastThinkTime;
	// Set the current time
	m_npcLastThinkTime = g_pGameVars->time;

	// Set onground condition
	if(m_pState->flags & FL_ONGROUND)
		ClearCondition(AI_COND_NOT_ONGROUND);
	else
		SetCondition(AI_COND_NOT_ONGROUND);

	// Check for gibbing from explode death mode
	if(m_deathMode == DEATH_EXPLODE 
		&& m_deathExplodeTime <= g_pGameVars->time)
	{
		CallGibNPC();
		return;
	}

	// Clear damage bits that need clearing
	ProcessClearDamageList();

	// Update AI awareness distances
	UpdateDistances();

	// Perform AI functions
	RunAI();

	// Advance frame and get animation interval time
	Double animInterval = FrameAdvance(0);

	// Set next think time
	m_pState->nextthink = g_pGameVars->time + NPC_THINK_TIME;

	// Update head controllers
	UpdateHeadControllers();

	// Update idle animation state
	UpdateIdleAnimation();

	// Manage any animation events
	ManageAnimationEvents();
	if(m_npcState != NPC_STATE_DEAD && m_pState->deadstate != DEADSTATE_DYING)
	{
		if(!IsMovementComplete())
		{
			// Perform any movement
			PerformMovement(animInterval);
		}
		else if(!IsTaskRunning() && !IsTaskComplete())
		{
			// Warn about errors with AI state
			Util::EntityConDPrintf(m_pEdict, "Schedule stalled.\n");
		}
	}

	// Perform yaw changes
	ChangeYaw(animInterval);
}
 
//=============================================
// @brief TRUE if a black hole can pull this entity
//
//=============================================
bool CBaseNPC::CanBlackHolePull( void ) const
{
	if(m_npcState == NPC_STATE_SCRIPT)
		return false;
	else
		return true;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::StopAnimation( void )
{
	m_pState->framerate = 0;
}

//=============================================
// @brief
//
//=============================================
Float CBaseNPC::GetCoverDistance( void )
{
	return NPC_DEFAULT_COVER_DISTANCE;
}

//=============================================
// @brief
//
//=============================================
activity_t CBaseNPC::GetFlinchActivity( void )
{
	activity_t flinchActivity = ACT_RESET;
	switch(m_lastHitGroup)
	{
	case HITGROUP_HEAD:
		flinchActivity = ACT_FLINCH_HEAD;
		break;
	case HITGROUP_STOMACH:
		flinchActivity = ACT_FLINCH_STOMACH;
		break;
	case HITGROUP_LEFT_ARM:
		flinchActivity = ACT_FLINCH_LEFTARM;
		break;
	case HITGROUP_RIGHT_ARM:
		flinchActivity = ACT_FLINCH_RIGHTARM;
		break;
	case HITGROUP_LEFT_LEG:
		flinchActivity = ACT_FLINCH_LEFTLEG;
		break;
	case HITGROUP_RIGHT_LEG:
		flinchActivity = ACT_FLINCH_RIGHTLEG;
		break;
	default:
		flinchActivity = ACT_SMALL_FLINCH;
		break;
	}

	// Default to ACT_SMALL_FLINCH if activity is not available
	if(FindActivity(flinchActivity) == NO_SEQUENCE_VALUE)
		flinchActivity = ACT_SMALL_FLINCH;

	return flinchActivity;
}

//=============================================
// @brief
//
//=============================================
activity_t CBaseNPC::GetDeathActivity( void )
{
	if(m_pState->deadstate != DEADSTATE_NONE)
		return (activity_t)GetIdealActivity();

	// Manage special death modes
	if(m_deathMode != DEATH_NORMAL 
		&& m_deathMode != DEATH_EXPLODE)
	{
		activity_t activity;
		switch(m_deathMode)
		{
		case DEATH_DECAPITATED:
			activity = ACT_DIE_HEADSHOT;
			break;
		default:
			Util::EntityConPrintf(m_pEdict, "Unhandled special death mode '%d'.\n", (Int32)m_deathMode);
			activity = ACT_RESET;
			break;
		}

		if(FindActivity(activity) == NO_SEQUENCE_VALUE)
			activity = ACT_DIESIMPLE;

		return activity;
	}

	bool triedDirection = false;
	activity_t deathActivity = ACT_DIESIMPLE;

	Vector forward;
	Math::AngleVectors(m_pState->angles, &forward);
	Float dp = Math::DotProduct(forward, gMultiDamage.GetAttackDirection() * -1);

	if(m_damageBits & DMG_EXPLOSION || m_lastHitGroup == HITGROUP_GENERIC)
	{
		// We tried to get an animation based
		// on an attack direction
		triedDirection = true;

		if(dp > 0.3)
			deathActivity = ACT_DIEFORWARD;
		else if(dp <= -0.3)
			deathActivity = ACT_DIEBACKWARD;
	}
	else
	{
		switch(m_lastHitGroup)
		{
		case HITGROUP_HEAD:
			deathActivity = ACT_DIE_HEADSHOT;
			break;
		case HITGROUP_STOMACH:
			deathActivity = ACT_DIE_GUTSHOT;
			break;
		}
	}

	if(FindActivity(deathActivity) == NO_SEQUENCE_VALUE)
	{
		if(triedDirection)
		{
			// Default to simple death if all else fails
			deathActivity = ACT_DIESIMPLE;
		}
		else
		{
			// Get a death animation based on attack dir
			if(dp > 0.3)
				deathActivity = ACT_DIEFORWARD;
			else if(dp <= -0.3)
				deathActivity = ACT_DIEBACKWARD;

			if(FindActivity(deathActivity) == NO_SEQUENCE_VALUE)
				deathActivity = ACT_DIESIMPLE;
		}
	}

	// See if we have space to die forward/backwards
	if(deathActivity == ACT_DIEFORWARD || deathActivity == ACT_DIEBACKWARD)
	{
		Vector endPosition;
		if(deathActivity == ACT_DIEFORWARD)
			endPosition = m_pState->origin + forward*64;
		else
			endPosition = m_pState->origin - forward*64;

		trace_t tr;
		Util::TraceLine(m_pState->origin, endPosition, false, false, m_pEdict, tr);
		if(!tr.noHit())
			deathActivity = ACT_DIESIMPLE;
	}

	return deathActivity;
}

//=============================================
// @brief
//
//=============================================
activity_t CBaseNPC::GetStoppedActivity( void )
{
	return ACT_IDLE;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::StopMovement( void )
{
	SetIdealActivity(GetStoppedActivity());
	// Force think immediately
	m_pState->nextthink = g_pGameVars->time;
}

//=============================================
// @brief
//
//=============================================
Float CBaseNPC::GetLeanAwarenessTime( void )
{
	return NPC_DEFAULT_LEAN_AWARE_TIME;
}

//=============================================
// @brief
//
//=============================================
scriptstate_t CBaseNPC::GetScriptState( void ) const
{
	return (scriptstate_t)m_scriptState;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::SetGoalEntity( CBaseEntity* pEntity )
{
	m_goalEntity = pEntity;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::SetScriptEntity( CScriptedSequence* pScriptEntity )
{
	m_pScriptedSequence = pScriptEntity;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::SetTargetEntity( CBaseEntity* pEntity )
{
	m_targetEntity = pEntity;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::SetScriptState( scriptstate_t state )
{
	m_scriptState = state;
}

//=============================================
// @brief
//
//=============================================
CScriptedSequence* CBaseNPC::GetScriptedSequence( void )
{
	return m_pScriptedSequence; 
}

//=============================================
// @brief
//
//=============================================
CBaseEntity* CBaseNPC::GetEnemy( void ) const 
{
	if(!m_enemy)
		return nullptr;
	else
		return m_enemy.operator->(); 
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::SetIdealNPCState( npcstate_t state )
{
	m_idealNPCState = state;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::SetLastActivityTime( Double time )
{
	m_lastActivityTime = time;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::SetCurrentActivity( activity_t activity )
{
	m_currentActivity = (activity_t)activity;
}

//=============================================
// @brief
//
//=============================================
CBaseEntity* CBaseNPC::GetTargetEntity( void ) 
{ 
	return m_targetEntity; 
}

//=============================================
// @brief Sets the enemy's last known position and angles
//
//=============================================
void CBaseNPC::SetEnemyInfo( const Vector& enemyLKP, const Vector& enemyLKA )
{
	m_enemyLastKnownPosition = enemyLKP;
	m_enemyLastKnownAngles = enemyLKA;
}

//=============================================
// @brief Gets the enemy information
//
//=============================================
void CBaseNPC::GetEnemyInfo( Vector& enemyLKP, Vector& enemyLKA, Double& enemyLST )
{
	enemyLKP = m_enemyLastKnownPosition;
	enemyLKA = m_enemyLastKnownAngles;
	enemyLST = m_lastEnemySightTime;
}

//=============================================
// @brief Sets the current enemy
//
//=============================================
void CBaseNPC::SetEnemy( CBaseEntity* pEnemy )
{
	assert(pEnemy->IsNPC() || pEnemy->IsPlayer() || pEnemy->IsFuncBreakableEntity());

	if(!pEnemy->IsNPC() && !pEnemy->IsPlayer() && !pEnemy->IsFuncBreakableEntity())
		return;

	m_enemy = pEnemy;
}

//=============================================
// @brief Sets the last time the enemy was sighted
//
//=============================================
void CBaseNPC::SetLastEnemySightTime( Double time )
{
	m_lastEnemySightTime = time;
}

//=============================================
// @brief Sets the last enemy sight time
//
//=============================================
Double CBaseNPC::GetLastEnemySightTime( void )
{
	return m_lastEnemySightTime;
}

//=============================================
// @brief Returns the voice pitch
//
//=============================================
Uint32 CBaseNPC::GetVoicePitch( void )
{
	return PITCH_NORM;
}

//=============================================
// @brief Tells if weapons can be dropped
//
//=============================================
bool CBaseNPC::CanDropWeapons( void ) const
{
	return m_dontDropWeapons ? false : true;
}

//=============================================
// @brief Sets whether npc can drop weapons
//
//=============================================
void CBaseNPC::SetCanDropWeapons( bool canDrop )
{
	m_dontDropWeapons = canDrop ? false : true;
}

//=============================================
// @brief Fires a weapon
//
//=============================================
void CBaseNPC::FireWeapon( Uint32 numShots, const Char* soundPattern, Uint32 numSounds, Uint32* ptrAmmoLoaded )
{
	bullet_types_t bulletType = GetBulletType();
	if(bulletType == BULLET_NONE)
	{
		Util::EntityConPrintf(m_pEdict, "%s - GetBulletType returned BULLET_NONE", __FUNCTION__);
		return;
	}

	Vector shootOrigin = GetGunPosition();
	Vector shootDirection = GetShootVector(shootOrigin);

	m_pState->effects |= EF_MUZZLEFLASH;

	Vector up, right;
	Math::GetUpRight(shootDirection, up, right);

	Uint32 firingConeId = GetFiringCone(true);
	Vector firingCone = Weapon_GetConeSize(firingConeId);

	FireBullets(numShots, shootOrigin, shootDirection, right, up, firingCone, NPC_DEFAULT_MAX_FIRING_DISTANCE, bulletType, 4, 0, this);

	if(numSounds > 1)
		Util::PlayRandomEntitySound(this, soundPattern, numSounds, SND_CHAN_WEAPON, VOL_NORM, ATTN_GUNFIRE);
	else
		Util::EmitEntitySound(this, soundPattern, SND_CHAN_WEAPON, VOL_NORM, ATTN_GUNFIRE);

	gAISounds.AddSound(AI_SOUND_COMBAT, m_pState->origin, NPC_GUN_SOUND_RADIUS, VOL_NORM, 0.3);

	if(ptrAmmoLoaded && (*ptrAmmoLoaded) > 0)
		(*ptrAmmoLoaded)--;
}

//=============================================
// @brief Returns blood color setting
//
//=============================================
bloodcolor_t CBaseNPC::GetBloodColor( void ) 
{ 
	return (bloodcolor_t)m_bloodColor; 
}

//=============================================
// @brief
//
//=============================================
Vector CBaseNPC::GetBodyTarget( const Vector& targetingPosition )
{
	return (GetCenter()*0.5) + (GetEyePosition()*0.5);
}

//=============================================
// @brief Tells if entity can be auto-aimed
//
//=============================================
bool CBaseNPC::IsAutoAimable( CBaseEntity* pAimerEntity )
{
	return IsAlive() ? true : false;
}

//=============================================
// @brief Calculates coverage for a position
//
//=============================================
Float CBaseNPC::CalculateCoverage( const Vector& lookOrigin, const Vector& lookOffset, const Vector& enemyEyePosition )
{
	trace_t tr;
	Float coverage = 0;

	for(Uint32 i = 0; i < NPC_NUM_COVERAGE_CHECKS; i++)
	{
		Vector testPosition = lookOrigin + lookOffset*((Float)i/(Float)NPC_NUM_COVERAGE_CHECKS);
		Util::TraceLine(testPosition, enemyEyePosition, true, false, m_pEdict, tr);
		if(!tr.noHit())
			coverage += 1.0f/(Float)NPC_NUM_COVERAGE_CHECKS;
	}

	return coverage;
}

//=============================================
// @brief Picks a proper reload schedule
//
//=============================================
const CAISchedule* CBaseNPC::GetReloadSchedule( void )
{
	if(!CheckCondition(AI_COND_IN_DANGER) && m_enemy)
	{
		// See if our coverage warrants actually hiding
		Vector enemyEyePosition = m_enemy->GetEyePosition();
		Float coverage = CalculateCoverage(m_pState->origin, m_pState->view_offset, enemyEyePosition);
		
		if(coverage < 0.5)
			return GetScheduleByIndex(AI_SCHED_HIDE_RELOAD);
		else
			return GetScheduleByIndex(AI_SCHED_RELOAD);
	}
	else
	{
		return GetScheduleByIndex(AI_SCHED_RELOAD);
	}
}

//=============================================
// @brief Returns the length of the route we are taking
//
//=============================================
Float CBaseNPC::GetRouteLength( void )
{
	Float pathLength = 0;
	Vector prevPosition = GetNavigablePosition();
	for(Int32 i = m_routePointIndex; i < MAX_ROUTE_POINTS; i++)
	{
		// Check if we reached the end
		if(!m_routePointsArray[i].type)
			break;

		pathLength += (m_routePointsArray[i].position - prevPosition).Length();

		// Check if we hit the goal, but only after count was raised
		if(m_routePointsArray[i].type & MF_IS_GOAL)
			break;

		// Set for next
		prevPosition = m_routePointsArray[i].position;
	}

	return pathLength;
}

//=============================================
// @brief Returns the reaction time
//
//=============================================
Float CBaseNPC::GetReactionTime( void ) 
{ 
	return GetSkillCVarValue(g_skillcvars.skillNPCReactionTime);
}

//=============================================
// @brief Return skill cvar value
//
//=============================================
Float CBaseNPC::GetSkillCVarValue( Int32 skillcvar ) const
{
	return gSkillData.GetSkillCVarSetting(skillcvar, (force_skillcvar_t)m_forceSkillCvar);
}

//=============================================
// @brief Return skill cvar value
//
//=============================================
Uint32 CBaseNPC::GetFogAttenuatedFiringCone( Uint32 coneIndex )
{
	if(!m_enemy)
		return coneIndex;

	if(coneIndex >= 11)
		return coneIndex;

	Int32 endDistance = CEnvFog::GetFogEndDistance();
	if(endDistance <= 0)
		return coneIndex;

	Vector enemyPosition = m_enemy->GetNavigablePosition();
	Vector myEyePosition = GetEyePosition();

	Uint32 add;
	if(coneIndex < 3)
		add = 2;
	else
		add = 1;

	Uint32 _coneIndex = coneIndex;
	Float distance = (enemyPosition - myEyePosition).Length();
	Float ratio = distance / (Float)endDistance;
	if(ratio > 0.4 && ratio < 0.6)
		_coneIndex += add;
	else if(ratio >= 0.6 && ratio < 0.8)
		_coneIndex += add * 2;
	else
		_coneIndex += add * 2.5;

	_coneIndex = clamp(_coneIndex, 0, 11);
	return _coneIndex;
}

//=============================================
// @brief Sets the NPC to be pulled by a trigger_pullnpc
//
//=============================================
void CBaseNPC::SetNPCPuller( CBaseEntity* pPuller, const Vector& pullPosition )
{
	CBaseEntity* pLastPuller = m_npcPullerEntity;
	m_npcPullerEntity = pPuller;

	if(m_npcPullerEntity)
	{
		m_npcPullerPosition = pullPosition;

		if(pLastPuller != m_npcPullerEntity)
		{
			// Force a reset and set to run away from puller
			ClearSchedule();
			ChangeSchedule(GetScheduleByIndex(AI_SCHED_RUN_FROM_NPC_PULLER));
		}
	}
}
