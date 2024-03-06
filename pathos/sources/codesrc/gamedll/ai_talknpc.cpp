/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "ai_talknpc.h"
#include "cplane.h"
#include "sentencesfile.h"

// Wait time until we can talk again
Double CTalkNPC::g_talkWaitTime = 0;

// Ideal yaw task yaw speed
const Float CTalkNPC::IDEALYAW_TASK_YAWSPEED = 60;
// Maximum distance for staring
const Float CTalkNPC::STARE_MAX_DIST = 128;
// Minimum treshold for idealyaw difference
const Float CTalkNPC::IDEALYAW_DIFF_TRESHOLD = 10;
// Minimum talk range
const Float CTalkNPC::MAXIMUM_TALK_RANGE = 512;
// Minimum push speed
const Float CTalkNPC::MINIMUM_PUSH_SPEED = 50;
// Mortal health treshold on player
const Float CTalkNPC::PLAYER_MORTAL_HEALTH_TRESHOLD = 0.125;
// Medium health treshold on player
const Float CTalkNPC::PLAYER_MEDIUM_HEALTH_TRESHOLD = 0.25;
// Light health treshold on player
const Float CTalkNPC::PLAYER_LIGHT_HEALTH_TRESHOLD = 0.5;
// Maximum follower distance before FOLLO_TARGET_TOOFAR is set
const Float CTalkNPC::MAX_FOLLOW_DISTANCE = 128;
// Default follow range
const Float CTalkNPC::DEFAULT_FOLLOW_RANGE = 128;
// Sentence group name postifxes
const Char* CTalkNPC::SENTENCE_GROUP_POSTFIXES[CTalkNPC::NB_TALKNPC_GROUPS] = {
	"_ANSWER",
	"_QUESTION",
	"_IDLE",
	"_STARE",
	"_OK",
	"_WAIT",
	"_STOP",
	"_SCARED",
	"_HELLO",
	"_PHELLO",
	"_PIDLE",
	"_PQUEST",
	"_CUREA",
	"_CUREB",
	"_CUREC",
	"_WOUND",
	"_MORTAL",
	"_POK",
	"_CONVERSE",
	"_FINDPLAYER",
};

//==========================================================================
//
// SCHEDULES FOR CTALKNPC CLASS
//
//==========================================================================

//=============================================
// @brief Idle respone
//
//=============================================
ai_task_t taskListScheduleIdleResponse[] = 
{
	AITASK(AI_TASK_SET_ACTIVITY,				(Float)ACT_IDLE),
	AITASK(AI_TASK_WAIT,						0.5),
	AITASK(AI_TALKNPC_TASK_EYECONTACT,			0),
	AITASK(AI_TALKNPC_TASK_RESPOND,				0),
	AITASK(AI_TALKNPC_TASK_IDEALYAW,			0),
	AITASK(AI_TASK_FACE_IDEAL,					0),
	AITASK(AI_TASK_SET_ACTIVITY,				(Float)ACT_IDLE),
	AITASK(AI_TALKNPC_TASK_EYECONTACT,			0)
};

const CAISchedule scheduleIdleResponse(
	// Task list
	taskListScheduleIdleResponse, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleIdleResponse),
	// AI interrupt mask
	AI_COND_NEW_ENEMY |
	AI_COND_IN_DANGER |
	AI_COND_LIGHT_DAMAGE |
	AI_COND_FOLLOW_TARGET_TOO_FAR |
	AI_COND_BLOCKING_PATH |
	AI_COND_HEAVY_DAMAGE,
	// Sound mask
	AI_SOUND_NONE, 
	// Name
	"Idle Response"
);

//=============================================
// @brief Idle speak
//
//=============================================
ai_task_t taskListScheduleIdleSpeak[] = 
{
	AITASK(AI_TALKNPC_TASK_SPEAK,				0),
	AITASK(AI_TALKNPC_TASK_IDEALYAW,			0),
	AITASK(AI_TASK_FACE_IDEAL,					0),
	AITASK(AI_TASK_SET_ACTIVITY,				(Float)ACT_IDLE),
	AITASK(AI_TALKNPC_TASK_EYECONTACT,			0),
	AITASK(AI_TASK_WAIT_RANDOM,					0.5)
};

const CAISchedule scheduleIdleSpeak(
	// Task list
	taskListScheduleIdleSpeak, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleIdleSpeak),
	// AI interrupt mask
	AI_COND_NEW_ENEMY |
	AI_COND_IN_DANGER |
	AI_COND_LIGHT_DAMAGE |
	AI_COND_FOLLOW_TARGET_TOO_FAR |
	AI_COND_BLOCKING_PATH |
	AI_COND_HEAVY_DAMAGE,
	// Sound mask
	AI_SOUND_NONE, 
	// Name
	"Idle Speak"
);

//=============================================
// @brief Idle speak wait
//
//=============================================
ai_task_t taskListScheduleIdleSpeakWait[] = 
{
	AITASK(AI_TALKNPC_TASK_SPEAK,				0),
	AITASK(AI_TALKNPC_TASK_EYECONTACT,			0),
	AITASK(AI_TASK_WAIT,						2)
};

const CAISchedule scheduleIdleSpeakWait(
	// Task list
	taskListScheduleIdleSpeakWait, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleIdleSpeakWait),
	// AI interrupt mask
	AI_COND_NEW_ENEMY |
	AI_COND_IN_DANGER |
	AI_COND_LIGHT_DAMAGE |
	AI_COND_FOLLOW_TARGET_TOO_FAR |
	AI_COND_BLOCKING_PATH |
	AI_COND_HEAVY_DAMAGE,
	// Sound mask
	AI_SOUND_NONE, 
	// Name
	"Idle Speak Wait"
);

//=============================================
// @brief Idle hello
//
//=============================================
ai_task_t taskListScheduleIdleHello[] = 
{
	AITASK(AI_TASK_FACE_IDEAL,					0),
	AITASK(AI_TASK_SET_ACTIVITY,				(Float)ACT_IDLE),
	AITASK(AI_TALKNPC_TASK_SAY_HELLO,			0),
	AITASK(AI_TALKNPC_TASK_EYECONTACT,			0),
	AITASK(AI_TASK_WAIT,						0.5),
	AITASK(AI_TASK_FACE_IDEAL,					0),
	AITASK(AI_TASK_SET_ACTIVITY,				(Float)ACT_IDLE),
	AITASK(AI_TALKNPC_TASK_SAY_HELLO,			0),
	AITASK(AI_TALKNPC_TASK_EYECONTACT,			0),
	AITASK(AI_TASK_WAIT,						0.5),
	AITASK(AI_TASK_FACE_IDEAL,					0),
	AITASK(AI_TASK_SET_ACTIVITY,				(Float)ACT_IDLE),
	AITASK(AI_TALKNPC_TASK_SAY_HELLO,			0),
	AITASK(AI_TALKNPC_TASK_EYECONTACT,			0),
	AITASK(AI_TASK_WAIT,						0.5),
	AITASK(AI_TASK_FACE_IDEAL,					0),
	AITASK(AI_TASK_SET_ACTIVITY,				(Float)ACT_IDLE),
	AITASK(AI_TALKNPC_TASK_SAY_HELLO,			0),
	AITASK(AI_TALKNPC_TASK_EYECONTACT,			0),
	AITASK(AI_TASK_WAIT,						0.5)
};

const CAISchedule scheduleIdleHello(
	// Task list
	taskListScheduleIdleHello, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleIdleHello),
	// AI interrupt mask
	AI_COND_NEW_ENEMY |
	AI_COND_IN_DANGER |
	AI_COND_LIGHT_DAMAGE |
	AI_COND_FOLLOW_TARGET_TOO_FAR |
	AI_COND_BLOCKING_PATH |
	AI_COND_HEAVY_DAMAGE |
	AI_COND_HEAR_SOUND |
	AI_COND_PROVOKED,
	// Sound mask
	AI_SOUND_COMBAT |
	AI_SOUND_DANGER, 
	// Name
	"Idle Hello"
);

//=============================================
// @brief Idle say stop shooting
//
//=============================================
ai_task_t taskListScheduleSayStopShooting[] = 
{
	AITASK(AI_TALKNPC_TASK_SAY_STOPSHOOTING,	0)
};

const CAISchedule scheduleSayStopShooting(
	// Task list
	taskListScheduleSayStopShooting, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleSayStopShooting),
	// AI interrupt mask
	AI_COND_NEW_ENEMY |
	AI_COND_IN_DANGER |
	AI_COND_LIGHT_DAMAGE |
	AI_COND_HEAVY_DAMAGE,
	// Sound mask
	AI_SOUND_NONE, 
	// Name
	"Say Stop Shooting"
);

//=============================================
// @brief Idle watch player
//
//=============================================
ai_task_t taskListScheduleIdleWatchPlayer[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_SET_ACTIVITY,				(Float)ACT_IDLE),
	AITASK(AI_TALKNPC_TASK_LOOK_AT_PLAYER,		6)
};

const CAISchedule scheduleIdleWatchPlayer(
	// Task list
	taskListScheduleIdleWatchPlayer, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleIdleWatchPlayer),
	// AI interrupt mask
	AI_COND_NEW_ENEMY |
	AI_COND_IN_DANGER |
	AI_COND_LIGHT_DAMAGE |
	AI_COND_HEAVY_DAMAGE |
	AI_COND_HEAR_SOUND |
	AI_COND_CLIENT_UNSEEN |
	AI_COND_FOLLOW_TARGET_TOO_FAR |
	AI_COND_BLOCKING_PATH |
	AI_COND_PROVOKED,
	// Sound mask
	AI_SOUND_COMBAT |
	AI_SOUND_DANGER, 
	// Name
	"Idle Watch Player"
);

//=============================================
// @brief Idle watch player stare
//
//=============================================
ai_task_t taskListScheduleIdleWatchPlayerStare[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_SET_ACTIVITY,				(Float)ACT_IDLE),
	AITASK(AI_TALKNPC_TASK_PLAYER_STARE,		6),
	AITASK(AI_TALKNPC_TASK_STARE,				0),
	AITASK(AI_TALKNPC_TASK_IDEALYAW,			0),
	AITASK(AI_TASK_FACE_IDEAL,					0),
	AITASK(AI_TALKNPC_TASK_EYECONTACT,			0)
};

const CAISchedule scheduleIdleWatchPlayerStare(
	// Task list
	taskListScheduleIdleWatchPlayerStare, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleIdleWatchPlayerStare),
	// AI interrupt mask
	AI_COND_NEW_ENEMY |
	AI_COND_IN_DANGER |
	AI_COND_LIGHT_DAMAGE |
	AI_COND_HEAVY_DAMAGE |
	AI_COND_HEAR_SOUND |
	AI_COND_CLIENT_UNSEEN |
	AI_COND_FOLLOW_TARGET_TOO_FAR |
	AI_COND_BLOCKING_PATH |
	AI_COND_PROVOKED,
	// Sound mask
	AI_SOUND_COMBAT |
	AI_SOUND_DANGER, 
	// Name
	"Idle Watch Player Stare"
);

//=============================================
// @brief Idle eye contact
//
//=============================================
ai_task_t taskListScheduleIdleEyeContact[] = 
{
	AITASK(AI_TALKNPC_TASK_IDEALYAW,			0),
	AITASK(AI_TASK_FACE_IDEAL,					0),
	AITASK(AI_TASK_SET_ACTIVITY,				(Float)ACT_IDLE),
	AITASK(AI_TALKNPC_TASK_EYECONTACT,			0)
};

const CAISchedule scheduleIdleEyeContact(
	// Task list
	taskListScheduleIdleEyeContact, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleIdleEyeContact),
	// AI interrupt mask
	AI_COND_NEW_ENEMY |
	AI_COND_IN_DANGER |
	AI_COND_LIGHT_DAMAGE |
	AI_COND_HEAVY_DAMAGE |
	AI_COND_FOLLOW_TARGET_TOO_FAR |
	AI_COND_BLOCKING_PATH |
	AI_COND_HEAR_SOUND,
	// Sound mask
	AI_SOUND_DANGER, 
	// Name
	"Idle Eye Contact"
);

//=============================================
// @brief Idle Stand
//
//=============================================
ai_task_t taskListScheduleTalkNPCIdleStand[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_SET_ACTIVITY,				(Float)ACT_IDLE),
	AITASK(AI_TASK_WAIT,						2),
	AITASK(AI_TALKNPC_TASK_HEADRESET,			0),
};

const CAISchedule scheduleTalkNPCIdleStand(
	// Task list
	taskListScheduleTalkNPCIdleStand, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleTalkNPCIdleStand),
	// AI interrupt mask
	AI_COND_DANGEROUS_ENEMY_CLOSE |
	AI_COND_NEW_ENEMY |
	AI_COND_LIGHT_DAMAGE |
	AI_COND_HEAVY_DAMAGE |
	AI_COND_BLOCKING_PATH |
	AI_COND_HEAR_SOUND |
	AI_COND_PROVOKED,
	// Sound mask
	AI_SOUND_DANGER,
	// Name
	"TalkNPC Idle Stand"
);

//=============================================
// @brief Follow
//
//=============================================
ai_task_t taskListScheduleFollow[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_MOVE_TO_TARGET_RANGE,		CTalkNPC::DEFAULT_FOLLOW_RANGE),
	AITASK(AI_TASK_SET_SCHEDULE,				(Float)AI_TALKNPC_SCHED_TARGET_FACE),
};

const CAISchedule scheduleFollow(
	// Task list
	taskListScheduleFollow, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleFollow),
	// AI interrupt mask
	AI_COND_DANGEROUS_ENEMY_CLOSE |
	AI_COND_NEW_ENEMY |
	AI_COND_LIGHT_DAMAGE |
	AI_COND_HEAVY_DAMAGE |
	AI_COND_BLOCKING_PATH |
	AI_COND_HEAR_SOUND |
	AI_COND_PROVOKED,
	// Sound mask
	AI_SOUND_DANGER, 
	// Name
	"Follow"
);

//=============================================
// @brief Face target schedule
//
//=============================================
ai_task_t taskListScheduleFaceTarget[] = 
{
	AITASK(AI_TASK_SET_ACTIVITY,				(Float)ACT_IDLE),
	AITASK(AI_TASK_FACE_TARGET,					0),
	AITASK(AI_TASK_SET_ACTIVITY,				(Float)ACT_IDLE),
	AITASK(AI_TASK_SET_SCHEDULE,				(Float)AI_TALKNPC_SCHED_TARGET_CHASE),
};

const CAISchedule scheduleFaceTarget(
	// Task list
	taskListScheduleFaceTarget, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleFaceTarget),
	// AI interrupt mask
	AI_COND_DANGEROUS_ENEMY_CLOSE |
	AI_COND_NEW_ENEMY |
	AI_COND_LIGHT_DAMAGE |
	AI_COND_HEAVY_DAMAGE |
	AI_COND_BLOCKING_PATH |
	AI_COND_HEAR_SOUND |
	AI_COND_PROVOKED,
	// Sound mask
	AI_SOUND_DANGER, 
	// Name
	"Face Target"
);

//==========================================================================
//
// SCHEDULES FOR CTALKNPC CLASS
//
//==========================================================================

//=============================================
// @brief Constructor
//
//=============================================
CTalkNPC::CTalkNPC( edict_t* pedict ):
	CSquadNPC(pedict),
	m_saidSentencesBits(0),
	m_nbTalksInitiated(0),
	m_voicePitch(PITCH_NORM),
	m_nextUseTime(0),
	m_useSentenceGroup(NO_STRING_VALUE),
	m_unFollowSentenceGroup(NO_STRING_VALUE)
{
}

//=============================================
// @brief Destructor
//
//=============================================
CTalkNPC::~CTalkNPC( void )
{
}

//=============================================
// @brief Performs precache functions
//
//=============================================
void CTalkNPC::Precache( void )
{
	if(m_useSentenceGroup != NO_STRING_VALUE)
		m_sentenceGroupNames[TALKNPC_GRP_USE] = gd_engfuncs.pfnGetString(m_useSentenceGroup);

	if(m_unFollowSentenceGroup != NO_STRING_VALUE)
		m_sentenceGroupNames[TALKNPC_GRP_USE] = gd_engfuncs.pfnGetString(m_unFollowSentenceGroup);

	CSquadNPC::Precache();
}

//=============================================
// @brief Declares save-restore fields
//
//=============================================
void CTalkNPC::DeclareSaveFields( void )
{
	CSquadNPC::DeclareSaveFields();

	DeclareSaveField(DEFINE_DATA_FIELD(CTalkNPC, m_saidSentencesBits, EFIELD_UINT64));
	DeclareSaveField(DEFINE_DATA_FIELD(CTalkNPC, m_nbTalksInitiated, EFIELD_UINT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CTalkNPC, m_voicePitch, EFIELD_UINT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CTalkNPC, m_nextUseTime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CTalkNPC, m_useSentenceGroup, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CTalkNPC, m_unFollowSentenceGroup, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CTalkNPC, m_talkTargetEntity, EFIELD_EHANDLE));
	DeclareSaveField(DEFINE_DATA_FIELD(CTalkNPC, m_nextFollowTarget, EFIELD_EHANDLE));
}

//=============================================
// @brief Manages keyvalues
//
//=============================================
bool CTalkNPC::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "UseSentence"))
	{
		m_useSentenceGroup = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "UnUseSentence"))
	{
		m_unFollowSentenceGroup = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else
		return CSquadNPC::KeyValue(kv);
}

//=============================================
// @brief Manages taking damage
//
//=============================================
bool CTalkNPC::TakeDamage( CBaseEntity* pInflictor, CBaseEntity* pAttacker, Float amount, Int32 damageFlags )
{
	if(pAttacker && (GetRelationship(pAttacker) == R_ALLY || GetRelationship(pAttacker) == R_NONE) && pAttacker != this)
		return false;

	if(IsAlive())
	{
		// If it's the player, have others complain
		if(pAttacker && pAttacker->IsPlayer())
		{
			CBaseEntity* pFriend = FindNearestFriend();
			if(pFriend && pFriend->IsAlive())
				pFriend->ChangeSchedule(&scheduleSayStopShooting);
		}
	}

	return CBaseNPC::TakeDamage(pInflictor, pAttacker, amount, damageFlags);
}

//=============================================
// @brief Handles damage calculation for a hitscan
//
//=============================================
void CTalkNPC::TraceAttack( CBaseEntity* pAttacker, Float damage, const Vector& direction, trace_t& tr, Int32 damageFlags )
{
	if(pAttacker && (GetRelationship(pAttacker) == R_ALLY || GetRelationship(pAttacker) == R_NONE) && pAttacker != this)
		return;

	CSquadNPC::TraceAttack(pAttacker, damage, direction, tr, damageFlags);
}

//=============================================
// @brief Calls touch function
//
//=============================================
void CTalkNPC::CallTouch( CBaseEntity* pOther )
{
	if(!pOther->IsPlayer())
		return;

	if(HasMemory(AI_MEMORY_PROVOKED) || IsTalking() || m_npcState == NPC_STATE_SCRIPT)
		return;

	CBaseEntity* pGroundEntity = GetGroundEntity();
	if(pGroundEntity && pGroundEntity->IsMoving())
		return;

	Vector playerVelocity = pOther->GetVelocity();
	Float speed = SDL_fabs(playerVelocity.x) + SDL_fabs(playerVelocity.y);
	if(speed > MINIMUM_PUSH_SPEED)
	{
		Vector forward;
		Math::AngleVectors(pOther->GetAngles(), &forward);

		// Set an arbitrary destination based on angles
		Vector playerOrigin = pOther->GetOrigin();
		Vector moveDestination = playerOrigin + forward*64;

		SetPathBlocked(pOther, moveDestination);
		SetIdealYaw(playerOrigin);
	}
}

//=============================================
// @brief Manages getting killed
//
//=============================================
void CTalkNPC::Killed( CBaseEntity* pAttacker, gibbing_t gibbing, deathmode_t deathMode )
{
	// Immortals can't be killed
	if(HasSpawnFlag(CBaseNPC::FL_NPC_IMMORTAL))
		return;

	if(pAttacker && pAttacker->IsPlayer())
	{
		AlertFriends();
		LimitFollowers(pAttacker, 0);
	}

	m_targetEntity.reset();

	StopTalking();
	SetUse(nullptr);

	CSquadNPC::Killed(pAttacker, gibbing, deathMode);
}

//=============================================
// @brief Gets the relationship with the other entity
//
//=============================================
Int32 CTalkNPC::GetRelationship( CBaseEntity* pOther )
{
	if(pOther && pOther->IsPlayer() && HasMemory(AI_MEMORY_PROVOKED))
		return R_HATE;
	else
		return CSquadNPC::GetRelationship(pOther);
}

//=============================================
// @brief Tells if the NPC can play a scripted_sentence
//
//=============================================
bool CTalkNPC::CanPlaySentence( bool disregardState )
{
	if(disregardState)
		return CSquadNPC::CanPlaySentence(disregardState);
	else
		return CanSpeak();
}

//=============================================
// @brief Plays a sentence
//
//=============================================
void CTalkNPC::PlaySentence( const Char* pstrSentenceName, Float duration, Float volume, Float attenuation, Float timeOffset, bool subtitleOnlyInRadius, CBaseEntity* pPlayer )
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

		g_talkWaitTime = g_pGameVars->time + duration + Common::RandomFloat(2, 8);
	}

	// Make sure the duration is set before checking if pstrSentenceName is valid
	Float _duration = (duration < sentduration) ? sentduration : duration;
	if(_duration > 0)
		m_talkTime = g_pGameVars->time + _duration;
	else
		m_talkTime = g_pGameVars->time + 3;
}

//=============================================
// @brief Plays a scripted_sentence
//
//=============================================
void CTalkNPC::PlayScriptedSentence( const Char* pstrSentenceName, Float duration, Float volume, Float attenuation, Float timeOffset, bool subtitleOnlyInRadius, bool isConcurrent, CBaseEntity* pListener, CBaseEntity* pPlayer )
{
	// Make sure this is cleared
	ClearConditions(AI_COND_BLOCKING_PATH);

	m_nextUseTime = g_pGameVars->time + duration;
	m_talkTargetEntity = pListener;

	// Make us face the talk target
	if(m_talkTargetEntity && m_npcState != NPC_STATE_SCRIPT)
		SetIdealYaw(m_talkTargetEntity->GetEyePosition());

	CSquadNPC::PlayScriptedSentence(pstrSentenceName, duration, volume, attenuation, timeOffset, subtitleOnlyInRadius, isConcurrent, pListener, pPlayer);
}

//=============================================
// @brief Returns the conditions to ignore
//
//=============================================
Uint64 CTalkNPC::GetIgnoreConditions( void )
{
	Uint64 ignoreConditions = CSquadNPC::GetIgnoreConditions();
	if(m_talkTime >= g_pGameVars->time)
		ignoreConditions |= AI_COND_BLOCKING_PATH;

	return ignoreConditions;
}

//=============================================
// @brief Returns a schedule by it's index
//
//=============================================
const CAISchedule* CTalkNPC::GetScheduleByIndex( Int32 scheduleIndex )
{
	switch(scheduleIndex)
	{
	case AI_TALKNPC_SCHED_TARGET_CHASE:
		{
			return &scheduleFollow;
		}
		break;
	case AI_TALKNPC_SCHED_TARGET_FACE:
		{
			if(Common::RandomLong(0, 99) < 2)
				return &scheduleIdleSpeakWait;
			else
				return GetIdleStandSchedule(scheduleIndex);
		}
		break;
	case AI_SCHED_IDLE_STAND:
		{
			if(!(m_saidSentencesBits & TALKNPC_SAID_HELLO_PLAYER))
				return &scheduleIdleHello;

			if(!(m_saidSentencesBits & TALKNPC_SAID_DAMAGE_LIGHT) && m_pState->health <= (m_pState->maxhealth * 0.75))
			{
				PlaySentence(m_sentenceGroupNames[TALKNPC_GRP_WOUND].c_str(), 0, VOL_NORM, ATTN_IDLE, 0, true);
				m_saidSentencesBits |= TALKNPC_SAID_DAMAGE_LIGHT;
				return GetIdleStandSchedule(scheduleIndex);
			}
			else if(!(m_saidSentencesBits & TALKNPC_SAID_DAMAGE_HEAVY) && m_pState->health <= (m_pState->maxhealth * 0.5))
			{
				PlaySentence(m_sentenceGroupNames[TALKNPC_GRP_MORTAL].c_str(), 0, VOL_NORM, ATTN_IDLE, 0, true);
				m_saidSentencesBits |= TALKNPC_SAID_DAMAGE_HEAVY;
				return GetIdleStandSchedule(scheduleIndex);
			}
			else if(CanSpeak() && Common::RandomLong(0, m_nbTalksInitiated*2) == 0)
			{
				return &scheduleIdleSpeak;
			}
			else if(!IsTalking() && CheckConditions(AI_COND_SEE_CLIENT) && Common::RandomLong(0, 6) == 0)
			{
				CBaseEntity* pPlayer = Util::GetHostPlayer();
				if(pPlayer)
				{
					Vector playerOrigin = pPlayer->GetOrigin();
					Float distance = (playerOrigin - m_pState->origin).Length2D();
					if(distance < STARE_MAX_DIST)
					{
						Vector forward;
						Math::AngleVectors(pPlayer->GetAngles(), &forward);

						if(Util::DotPoints(playerOrigin, m_pState->origin, forward) >= m_fieldOfView)
							return &scheduleIdleWatchPlayerStare;
					}

					return &scheduleIdleWatchPlayer;
				}
			}

			if(IsTalking())
				return &scheduleIdleEyeContact;
			else
				return GetIdleStandSchedule(scheduleIndex);
		}
		break;
	default:
		return CSquadNPC::GetScheduleByIndex(scheduleIndex);
		break;
	}
}

//=============================================
// @brief Starts a task
//
//=============================================
void CTalkNPC::StartTask( const ai_task_t* pTask )
{
	switch(pTask->task)
	{
	case AI_TALKNPC_TASK_PLAYER_STARE:
	case AI_TALKNPC_TASK_EYECONTACT:
	case AI_TALKNPC_TASK_LOOK_AT_PLAYER:
		break;
	case AI_TALKNPC_TASK_HEADRESET:
		{
			if(m_talkTime && m_talkTime < g_pGameVars->time)
			{
				m_talkTargetEntity.reset();
				m_talkTime = 0;
			}
			SetTaskCompleted();
		}
		break;
	case AI_TALKNPC_TASK_FACE_PLAYER:
		{
			m_updateYaw = true;
		}
		break;
	case AI_TALKNPC_TASK_SPEAK:
		{
			IdleSpeak();
			SetTaskCompleted();
		}
		break;
	case AI_TALKNPC_TASK_RESPOND:
		{
			IdleRespond();
			SetTaskCompleted();
		}
		break;
	case AI_TALKNPC_TASK_SAY_HELLO:
		{
			IdleHello();
			SetTaskCompleted();
		}
		break;
	case AI_TALKNPC_TASK_STARE:
		{
			IdleStare();
			SetTaskCompleted();
		}
		break;
	case AI_TALKNPC_TASK_IDEALYAW:
		{
			if(m_talkTargetEntity)
			{
				m_pState->yawspeed = IDEALYAW_TASK_YAWSPEED;
				Vector vectorToTarget = (m_talkTargetEntity->GetOrigin() - m_pState->origin);
				vectorToTarget.Normalize();

				Float yaw = Util::VectorToYaw(vectorToTarget) - m_pState->angles[YAW];
				if(yaw > 180.0f)
					yaw -= 360.0f;
				else if(yaw < -180.0f)
					yaw += 360.0f;

				if(yaw < 0)
					m_pState->idealyaw = _max(yaw+45.0f, 0) + m_pState->angles[YAW];
				else
					m_pState->idealyaw = _min(yaw-45.0f, 0) + m_pState->angles[YAW];
			}

			SetTaskCompleted();
		}
		break;
	case AI_TALKNPC_TASK_SAY_STOPSHOOTING:
		{
			PlaySentence(m_sentenceGroupNames[TALKNPC_GRP_NOSHOOT].c_str(), 0, VOL_NORM, ATTN_NORM, 0, true);
			SetTaskCompleted();
		}
		break;
	case AI_TALKNPC_TASK_CANT_FOLLOW:
		{
			StopFollowing(false);
			PlaySentence(m_sentenceGroupNames[TALKNPC_GRP_STOP].c_str(), 0, VOL_NORM, ATTN_NORM, 0, true);
			SetTaskCompleted();
		}
		break;
	case AI_TASK_PLAY_SCRIPT:
		{
			if(m_talkTime <= g_pGameVars->time)
			{
				m_talkTargetEntity.reset();
				m_talkTime = 0;
			}
			CSquadNPC::StartTask(pTask);
		}
		break;
	default:
		CSquadNPC::StartTask(pTask);
		break;
	}
}

//=============================================
// @brief Runs a task
//
//=============================================
void CTalkNPC::RunTask( const ai_task_t* pTask )
{
	switch(pTask->task)
	{
	case AI_TALKNPC_TASK_PLAYER_STARE:
	case AI_TALKNPC_TASK_LOOK_AT_PLAYER:
		{
			if(m_npcState == NPC_STATE_IDLE && !IsMoving() && !IsTalking())
			{
				CBaseEntity* pPlayer = Util::GetHostPlayer();
				if(pPlayer)
				{
					Vector eyePosition = pPlayer->GetEyePosition(true);
					IdleHeadTurn(&eyePosition);
				}
			}
			else
			{
				SetTaskFailed();
				return;
			}

			if(pTask->task == AI_TALKNPC_TASK_PLAYER_STARE)
			{
				CBaseEntity* pPlayer = Util::GetHostPlayer();
				if(!pPlayer)
				{
					SetTaskFailed();
					return;
				}

				// If player is too far, then fail
				const Vector& playerOrigin = pPlayer->GetOrigin();
				if((playerOrigin - m_pState->origin).Length2D() > STARE_MAX_DIST)
				{
					SetTaskFailed();
					return;
				}

				Vector forward;
				Math::AngleVectors(m_pState->angles, &forward);

				if(Util::DotPoints(playerOrigin, m_pState->origin, forward) < m_fieldOfView)
				{
					SetTaskFailed();
					return;
				}
			}

			if(g_pGameVars->time > m_waitFinishedTime)
				SetTaskCompleted();
		}
		break;
	case AI_TALKNPC_TASK_FACE_PLAYER:
		{
			CBaseEntity* pPlayer = Util::GetHostPlayer();
			if(!pPlayer)
			{
				SetTaskFailed();
				return;
			}

			SetIdealYaw(pPlayer->GetOrigin());

			Vector eyePosition = pPlayer->GetEyePosition(true);
			IdleHeadTurn(&eyePosition);
			if(g_pGameVars->time > m_waitFinishedTime && GetYawDifference() < IDEALYAW_DIFF_TRESHOLD)
				SetTaskCompleted();
		}
		break;
	case AI_TALKNPC_TASK_EYECONTACT:
		{
			if(!IsMoving() && IsTalking() && m_talkTargetEntity)
			{
				Vector eyePosition = m_talkTargetEntity->GetEyePosition(true);
				IdleHeadTurn(&eyePosition);
			}
			else
				SetTaskCompleted();
		}
		break;
	case AI_TASK_WAIT_FOR_MOVEMENT:
		{
			if(IsTalking() && m_talkTargetEntity)
			{
				Vector eyePosition = m_talkTargetEntity->GetEyePosition(true);
				IdleHeadTurn(&eyePosition);
			}
			else
			{
				IdleHeadTurn(nullptr);
				IdleHello();

				// Randomly try some idle speech
				if(Common::RandomLong(0, m_nbTalksInitiated*20) == 0)
					IdleSpeak();
			}

			CSquadNPC::RunTask(pTask);
			if(IsTaskComplete())
				IdleHeadTurn(nullptr);
		}
		break;
	default:
		{
			if(IsTalking() && m_talkTargetEntity)
			{
				Vector eyePosition = m_talkTargetEntity->GetEyePosition(true);
				IdleHeadTurn(&eyePosition);
			}
			else
				SetIdealHeadAngles(0, 0);

			CSquadNPC::RunTask(pTask);
		}
		break;
	}
}

//=============================================
// @brief Handles an animation event
//
//=============================================
void CTalkNPC::HandleAnimationEvent( const mstudioevent_t* pevent )
{
	switch(pevent->event)
	{
	case EVENT_SCRIPT_SENTENCE_RND:
		{
			if(Common::RandomLong(0, 99) < 75)
				return;

			PlaySentence(pevent->options, 0, VOL_NORM, ATTN_IDLE, 0, true);
		}
		break;
	case EVENT_SCRIPT_SENTENCE:
		{
			PlaySentence(pevent->options, 0, VOL_NORM, ATTN_IDLE, 0, true);
		}
		break;
	default:
		{
			CSquadNPC::HandleAnimationEvent(pevent);
		}
		break;
	}
}

//=============================================
// @brief Return the stopped activity
//
//=============================================
activity_t CTalkNPC::GetStoppedActivity( void )
{
	if(IsTalking() && m_talkTargetEntity)
		return ACT_FOCUS;
	else
		return CSquadNPC::GetStoppedActivity();
}

//=============================================
// @brief Returns the ideal activity
//
//=============================================
Int32 CTalkNPC::GetIdealActivity( void )
{
	if(m_npcState == NPC_STATE_SCRIPT)
		return CSquadNPC::GetIdealActivity();

	switch(m_idealActivity)
	{
	case ACT_IDLE:
		{
			if(IsTalking() && m_talkTargetEntity)
				return ACT_FOCUS;
			else
				return CSquadNPC::GetIdealActivity();
		}
		break;
	default:
		return CSquadNPC::GetIdealActivity();
		break;
	}
}

//=============================================
// @brief Returns the view position
//
//=============================================
Vector CTalkNPC::GetEyePosition( bool addlean ) const
{
	Vector bonePosition;
	if(!gd_engfuncs.pfnGetBonePositionByName(m_pEdict, EYE_CENTER_BONE_NAME, bonePosition))
		bonePosition = CSquadNPC::GetEyePosition(addlean);

	return bonePosition;
}

//=============================================
// @brief Initializes talknpc data
//
//=============================================
void CTalkNPC::InitTalkingNPC( void )
{
	for(Uint32 i = 0; i < NB_TALKNPC_GROUPS; i++)
	{
		if(m_sentenceGroupNames[i].empty())
			continue;

		if(g_pSentencesFile)
			g_pSentencesFile->PrecacheGroup(m_sentenceGroupNames[i].c_str());
	}
}

//=============================================
// @brief Finds the nearest NPC
//
//=============================================
CBaseEntity* CTalkNPC::FindNearestFriend( void )
{
	trace_t tr;

	Float lastClosestDistance = -1;
	CBaseEntity* pLastNearestFriend = nullptr;
	CBaseEntity* pFriend = nullptr;

	Vector startPosition = m_pState->origin;
	startPosition.z = m_pState->absmax.z;

	while(true)
	{
		pFriend = EnumerateFriends(pFriend, false);
		if(!pFriend)
			break;

		if(pFriend->GetNPCState() == NPC_STATE_SCRIPT)
			continue;

		Vector checkPosition = pFriend->GetOrigin();
		checkPosition.z = pFriend->GetAbsMaxs().z;

		Float distance = (checkPosition-startPosition).Length();
		if(distance < MAXIMUM_TALK_RANGE && (lastClosestDistance == -1 || lastClosestDistance < distance))
		{
			Util::TraceLine(startPosition, checkPosition, true, false, m_pEdict, tr);
			if(tr.noHit())
			{
				pLastNearestFriend = pFriend;
				lastClosestDistance = distance;
			}
		}
	}

	return pLastNearestFriend;
}

//=============================================
// @brief Finds the player
//
//=============================================
CBaseEntity* CTalkNPC::FindPlayer( void )
{
	CBaseEntity* pPlayer = Util::GetHostPlayer();
	if(!pPlayer)
		return nullptr;

	Vector myOrigin = GetCenter();
	Vector playerOrigin = pPlayer->GetCenter();

	Float distance = (myOrigin-playerOrigin).Length();
	if(distance > MAXIMUM_TALK_RANGE)
		return nullptr;

	trace_t tr;
	Util::TraceLine(myOrigin, playerOrigin, true, false, m_pEdict, tr);
	if(tr.noHit())
		return pPlayer;
	else
		return nullptr;
}

//=============================================
// @brief Returns distance to target
//
//=============================================
Float CTalkNPC::GetTargetDistance( void )
{
	if(!m_targetEntity || !m_targetEntity->IsAlive())
		return 1E6;
	else
		return (m_targetEntity->GetOrigin()-m_pState->origin).Length();
}

//=============================================
// @brief Stops the NPC from talking
//
//=============================================
void CTalkNPC::StopTalking( void )
{
	StopSentence();
}

//=============================================
// @brief Returns the voice pitch
//
//=============================================
Uint32 CTalkNPC::GetVoicePitch( void )
{
	return m_voicePitch + Common::RandomFloat(0, 3);
}

//=============================================
// @brief Manages an idle response
//
//=============================================
void CTalkNPC::IdleRespond( void )
{
	PlaySentence(m_sentenceGroupNames[TALKNPC_GRP_ANSWER].c_str(), 0, VOL_NORM, ATTN_NORM, 0, true);
}

//=============================================
// @brief Manages idle speech
//
//=============================================
void CTalkNPC::IdleSpeak( void )
{
	if(!CanSpeak())
		return;

	if(m_targetEntity && m_targetEntity->IsPlayer() && m_targetEntity->IsAlive())
	{
		m_talkTargetEntity = m_targetEntity;

		if(m_targetEntity->GetHealth() <= m_targetEntity->GetMaxHealth() * PLAYER_MORTAL_HEALTH_TRESHOLD
			&& !(m_saidSentencesBits & TALKNPC_SAID_DAMAGE_HEAVY))
		{
			PlaySentence(m_sentenceGroupNames[TALKNPC_GRP_PLHURT3].c_str(), 0, VOL_NORM, ATTN_IDLE, 0, true);
			m_saidSentencesBits |= TALKNPC_SAID_DAMAGE_HEAVY;
			return;
		}
		else if(m_targetEntity->GetHealth() <= m_targetEntity->GetMaxHealth() * PLAYER_MEDIUM_HEALTH_TRESHOLD
			&& !(m_saidSentencesBits & TALKNPC_SAID_DAMAGE_MEDIUM))
		{
			PlaySentence(m_sentenceGroupNames[TALKNPC_GRP_PLHURT2].c_str(), 0, VOL_NORM, ATTN_IDLE, 0, true);
			m_saidSentencesBits |= TALKNPC_SAID_DAMAGE_MEDIUM;
			return;
		}
		else if(m_targetEntity->GetHealth() <= m_targetEntity->GetMaxHealth() * PLAYER_LIGHT_HEALTH_TRESHOLD
			&& !(m_saidSentencesBits & TALKNPC_SAID_DAMAGE_LIGHT))
		{
			PlaySentence(m_sentenceGroupNames[TALKNPC_GRP_PLHURT1].c_str(), 0, VOL_NORM, ATTN_IDLE, 0, true);
			m_saidSentencesBits |= TALKNPC_SAID_DAMAGE_LIGHT;
			return;
		}
	}

	// Look for a friend to talk to
	CBaseEntity* pFriend = FindNearestFriend();
	if(pFriend && !pFriend->IsMoving() && Common::RandomLong(0, 99) < 75 
		&& pFriend->CanAnswer()  && pFriend->GetNPCState() != NPC_STATE_SCRIPT)
	{
		Int32 groupIndex = HasSpawnFlag(FL_NPC_IDLE) ? TALKNPC_GRP_PQUESTION : TALKNPC_GRP_QUESTION;
		PlaySentence(m_sentenceGroupNames[groupIndex].c_str(), 0, VOL_NORM, ATTN_NORM, 0, true);

		// Set friend to respond back
		Float delay = Common::RandomFloat(0.5, 1.5);
		pFriend->SetAnswerQuestion(this, m_talkTime + delay);
		m_talkTargetEntity = pFriend;

		m_nbTalksInitiated++;
		return;
	}

	if(Common::RandomLong(0, 1))
	{
		CBaseEntity* pPlayer = FindPlayer();
		if(pPlayer && pPlayer->CanAnswer())
		{
			Int32 groupIndex = HasSpawnFlag(FL_NPC_IDLE) ? TALKNPC_GRP_PIDLE : TALKNPC_GRP_IDLE;
			PlaySentence(m_sentenceGroupNames[groupIndex].c_str(), 0, VOL_NORM, ATTN_NORM, 0, true);
			m_talkTargetEntity = pPlayer;
			m_nbTalksInitiated++;
			return;
		}
	}

	// Try again in 3 seconds
	m_talkTime = g_pGameVars->time + 3;
	g_talkWaitTime = 0;
}

//=============================================
// @brief Tells if talking npc can answer
//
//=============================================
bool CTalkNPC::CanAnswer( void )
{
	if(m_talkTime >= g_pGameVars->time || !CanSpeak())
		return false;
	else
		return true;
}

//=============================================
// @brief Manages idle stare
//
//=============================================
void CTalkNPC::IdleStare( void )
{
	if(!CanSpeak())
		return;

	PlaySentence(m_sentenceGroupNames[TALKNPC_GRP_STARE].c_str(), 0, VOL_NORM, ATTN_NORM, 0, true);

	// Talk target is always the player in this case
	m_talkTargetEntity = Util::GetHostPlayer();
}

//=============================================
// @brief Manages idle hello
//
//=============================================
void CTalkNPC::IdleHello( void )
{
	if(!CanSpeak() || (m_saidSentencesBits & TALKNPC_SAID_HELLO_PLAYER))
		return;

	CBaseEntity* pPlayer = FindPlayer();
	if(!pPlayer)
		return;

	if(!IsInView(pPlayer) || !GetNPCVisibilityBits(pPlayer))
		return;

	m_talkTargetEntity = pPlayer;

	Int32 groupIndex = HasSpawnFlag(FL_NPC_IDLE) ? TALKNPC_GRP_PHELLO : TALKNPC_GRP_HELLO;
	PlaySentence(m_sentenceGroupNames[groupIndex].c_str(), 0, VOL_NORM, ATTN_IDLE, 0, true);
	m_saidSentencesBits |= TALKNPC_SAID_HELLO_PLAYER;
}

//=============================================
// @brief Turns head while idling
//
//=============================================
void CTalkNPC::IdleHeadTurn( const Vector* pTarget )
{
	if(!(m_capabilityBits & AI_CAP_TURN_HEAD))
		return;

	if(!pTarget)
	{
		SetIdealHeadAngles(0, 0);
		return;
	}

	Vector bonePosition = GetEyePosition();
	Float yaw = Util::VectorToYaw((*pTarget) - bonePosition) - m_pState->angles[YAW];
	if(yaw > 180.0f)
		yaw -= 360.0f;
	else if(yaw < -180.0f)
		yaw += 360.0f;

	Float pitch = Util::VectorToPitch((*pTarget) - bonePosition) - m_pState->angles[PITCH];
	if(yaw > 180.0f)
		yaw -= 360.0f;
	else if(yaw < -180.0f)
		yaw += 360.0f;

	SetIdealHeadAngles(pitch, yaw);
}

//=============================================
// @brief Tells if it's okay to speak
//
//=============================================
bool CTalkNPC::CanSpeak( void )
{
	if(!IsAlive() 
		|| g_talkWaitTime >= g_pGameVars->time
		|| m_talkTime > g_pGameVars->time
		|| HasSpawnFlag(FL_NPC_GAG) 
		|| Util::IsNullEntity(gd_engfuncs.pfnFindClientInPVS(m_pEdict))
		|| m_enemy && GetNPCVisibilityBits(m_enemy) != 0)
		return false;
	else
		return true;
}

//=============================================
// @brief Finds a friendly from the list
//
//=============================================
CBaseEntity* CTalkNPC::EnumerateFriends( CBaseEntity* pPrevious, bool trace )
{
	trace_t tr;

	Int32 startIndex = (pPrevious) ? pPrevious->GetEntityIndex() : 0;

	const Vector& eyePosition = GetEyePosition();
	for(Int32 i = startIndex + 1; i < g_pGameVars->numentities; i++)
	{
		edict_t* pedict = gd_engfuncs.pfnGetEdictByIndex(i);
		if(!pedict || Util::IsNullEntity(pedict))
			continue;

		CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
		if(!pEntity)
			continue;

		if(!pEntity->IsNPC() || !pEntity->IsAlive() 
			|| !pEntity->IsTalkNPC() || pEntity == this
			|| pEntity->GetClassification() != GetClassification())
			continue;

		if(trace)
		{
			Util::TraceLine(eyePosition, pEntity->GetEyePosition(), true, false, m_pEdict, tr);
			if(!tr.noHit())
				continue;
		}

		return pEntity;
	}
	
	return nullptr;
}

//=============================================
// @brief Alerts friends
//
//=============================================
void CTalkNPC::AlertFriends( void )
{
	CBaseEntity* pFriend = nullptr;

	while(true)
	{
		pFriend = EnumerateFriends(pFriend, true);
		if(!pFriend)
			break;

		pFriend->SetMemory(AI_MEMORY_PROVOKED);
	}
}

//=============================================
// @brief Tells if the NPC is talking
//
//=============================================
bool CTalkNPC::IsTalking( void ) const
{
	return (m_talkTime > g_pGameVars->time) ? true : false;
}

//=============================================
// @brief Sets talking duration
//
//=============================================
void CTalkNPC::SetTalkTime( Float duration )
{
	m_talkTime = g_pGameVars->time + duration;
}

//=============================================
// @brief Tells if NPC can follow the player
//
//=============================================
bool CTalkNPC::CanFollow( void ) const
{
	if(m_npcState == NPC_STATE_SCRIPT && m_pScriptedSequence && !m_pScriptedSequence->CanInterrupt() || !IsAlive())
		return false;
	else
		return (IsFollowing()) ? false : true;
}

//=============================================
// @brief Tells if NPC is following a player
//
//=============================================
bool CTalkNPC::IsFollowing( void ) const
{
	return (m_targetEntity) ? true : false;
}

//=============================================
// @brief Stops following the target
//
//=============================================
void CTalkNPC::StopFollowing( bool clearSchedule )
{
	if(!IsFollowing())
		return;

	if(!HasMemory(AI_MEMORY_PROVOKED) && m_targetEntity->IsPlayer())
	{
		PlaySentence(m_sentenceGroupNames[TALKNPC_GRP_UNUSE].c_str(), 0, VOL_NORM, ATTN_NORM, 0, true);
		m_talkTargetEntity = m_targetEntity;
	}

	if(m_movementGoal == MOVE_GOAL_TARGET_ENT)
		ClearRoute();

	m_targetEntity.reset();

	if(clearSchedule)
		ClearSchedule();

	if(m_enemy)
		m_idealNPCState = NPC_STATE_COMBAT;
}

//=============================================
// @brief Starts following a target
//
//=============================================
void CTalkNPC::StartFollowing( CBaseEntity* pTarget )
{
	if(m_pScriptedSequence)
		m_pScriptedSequence->CancelScript();

	if(m_enemy)
		m_idealNPCState = NPC_STATE_ALERT;

	m_targetEntity = pTarget;
	PlaySentence(m_sentenceGroupNames[TALKNPC_GRP_USE].c_str(), 0, VOL_NORM, ATTN_IDLE, 0, true);

	ClearConditions(AI_COND_BLOCKING_PATH);
	ClearSchedule();
}

//=============================================
// @brief Limits followers to a number
//
//=============================================
void CTalkNPC::LimitFollowers( const CBaseEntity* pPlayer, Uint32 maxFollowers )
{
	Uint32 followerCount = 0;
	CBaseEntity* pFriend = nullptr;

	while(true)
	{
		pFriend = EnumerateFriends(pFriend, true);
		if(!pFriend)
			break;

		if(pFriend->GetTargetEntity() == pPlayer)
		{
			// Raise count
			followerCount++;

			if(followerCount > maxFollowers)
				pFriend->StopFollowing(true);
		}
	}
}

//=============================================
// @brief Use function for following
//
//=============================================
void CTalkNPC::FollowerUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	if(m_npcState == NPC_STATE_SCRIPT || m_nextUseTime > g_pGameVars->time 
		|| !pCaller || !pCaller->IsPlayer())
		return;

	if(HasSpawnFlag(FL_NPC_IDLE))
	{
		m_talkTargetEntity = pCaller;
		DeclineFollowing();
	}
	else if(CanFollow())
	{
		if(!HasMemory(AI_MEMORY_PROVOKED))
		{
			StartFollowing(pCaller);
			m_saidSentencesBits = TALKNPC_SAID_HELLO_PLAYER;
		}
	}
	else
		StopFollowing(true);
}

//=============================================
// @brief Sets the follow target for this NPC
//
//=============================================
void CTalkNPC::SetFollowTarget( CBaseEntity* pTarget )
{
	if(m_npcState == NPC_STATE_SCRIPT || m_pScriptedSequence)
	{
		m_nextFollowTarget = pTarget;
		return;
	}

	if(m_enemy)
		m_idealNPCState = NPC_STATE_ALERT;

	m_targetEntity = pTarget;
	m_talkTargetEntity = pTarget;

	ClearConditions(AI_COND_BLOCKING_PATH);
	ClearSchedule();
}

//=============================================
// @brief Manages a healer call
//
//=============================================
void CTalkNPC::NPCHealerCall( CBaseEntity* pCaller )
{
	if(m_nextUseTime > g_pGameVars->time)
		return;

	if(m_pScriptedSequence || !IsAlive())
		return;

	m_targetEntity = pCaller;
	m_talkTargetEntity = pCaller;

	ClearSchedule();
}

//=============================================
// @brief Performs pre-schedule think functions
//
//=============================================
void CTalkNPC::PreScheduleThink( void )
{
	if(m_nextFollowTarget)
	{
		if(m_npcState != NPC_STATE_SCRIPT && !m_pScriptedSequence 
			&& m_npcState != NPC_STATE_DEAD && m_npcState != NPC_STATE_COMBAT)
		{
			SetFollowTarget(m_nextFollowTarget);
			m_nextFollowTarget.reset();
		}
	}

	if(m_targetEntity && !CheckConditions(AI_COND_FOLLOW_TARGET_TOO_FAR) && m_npcState != NPC_STATE_SCRIPT)
	{
		Float distance = (m_targetEntity->GetOrigin() - m_pState->origin).Length2D();
		if(distance > MAX_FOLLOW_DISTANCE)
			SetConditions(AI_COND_FOLLOW_TARGET_TOO_FAR);
	}

	if(!CheckConditions(AI_COND_SEE_CLIENT))
		SetConditions(AI_COND_CLIENT_UNSEEN);

	CSquadNPC::PreScheduleThink();
}

//=============================================
// @brief Declines following a target
//
//=============================================
void CTalkNPC::DeclineFollowing( void )
{
	PlaySentence(m_sentenceGroupNames[TALKNPC_GRP_DECLINE].c_str(), 0, VOL_NORM, ATTN_IDLE, 0, true);
}

//=============================================
// @brief Sets to answer a question
//
//=============================================
void CTalkNPC::SetAnswerQuestion( CBaseEntity* pSpeaker, Double talkTime )
{
	if(!m_pScriptedSequence)
		ChangeSchedule(&scheduleIdleResponse);

	m_talkTargetEntity = pSpeaker;
	m_talkTime = talkTime;
}

//=============================================
// @brief Sets the time until someone else can talk
//
//=============================================
void CTalkNPC::SetTalkWaitTime( Double talkWaitTime )
{
	g_talkWaitTime = talkWaitTime;
}

//=============================================
// @brief Sets the time until someone else can talk
//
//=============================================
void CTalkNPC::SetSentenceGroups( const Char* pstrPrefix )
{
	if(!pstrPrefix)
	{
		Util::EntityConPrintf(m_pEdict, "No prefix set for '%s'.\n", __FUNCTION__);
		return;
	}

	for(Uint32 i = 0; i < NB_TALKNPC_GROUPS; i++)
	{
		CString grpName;
		grpName << pstrPrefix << SENTENCE_GROUP_POSTFIXES[i];
		m_sentenceGroupNames[i] = grpName.c_str();
	}
}

//=============================================
// @brief Returns the schedule for idle stand
//
//=============================================
const CAISchedule* CTalkNPC::GetIdleStandSchedule( Int32 scheduleIndex )
{
	if(scheduleIndex == AI_TALKNPC_SCHED_TARGET_FACE)
		return &scheduleFaceTarget;
	else if(scheduleIndex == AI_SCHED_IDLE_STAND)
		return &scheduleTalkNPCIdleStand;
	else
		return &scheduleIdleStand;
}

//=============================================
// @brief Resets talk time
//
//=============================================
void CTalkNPC::ResetTalkTime( void )
{
	g_talkWaitTime = 0;
}