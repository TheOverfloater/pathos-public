/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "npcclonesoldier.h"
#include "ai_nodegraph.h"
#include "weapons_shared.h"
#include "ai_common.h"
#include "sentencesfile.h"
#include "grenade.h"
#include "envfog.h"
#include "player.h"

// View offset for npc
const Vector CNPCCloneSoldier::NPC_VIEW_OFFSET = Vector(0, 0, 50);
// Yaw speed for npc
const Uint32 CNPCCloneSoldier::NPC_YAW_SPEED = 180;
// Kick distance for NPC
const Float CNPCCloneSoldier::NPC_KICK_DISTANCE = 70;
// Kick treshold distance for NPC
const Float CNPCCloneSoldier::NPC_KICK_TRESHOLD_DISTANCE = 64;
// Attachment for weapon
const Uint32 CNPCCloneSoldier::NPC_WEAPON_ATTACHMENT_INDEX = 0;
// Gun position offset when standing
const Vector CNPCCloneSoldier::NPC_GUN_POSITION_STANDING_OFFSET = Vector(0, 0, 60);
// Gun position offset when crouching
const Vector CNPCCloneSoldier::NPC_GUN_POSITION_CROUCHING_OFFSET = Vector(0, 0, 48);
// Clip size for Sig552
const Uint32 CNPCCloneSoldier::NPC_SIG552_CLIP_SIZE = 30;
// Clip size for shotgun
const Uint32 CNPCCloneSoldier::NPC_SHOTGUN_CLIP_SIZE = 8;
// Clip size for M249 SAW
const Uint32 CNPCCloneSoldier::NPC_M249_CLIP_SIZE = 50;
// Clip size for TRG42
const Uint32 CNPCCloneSoldier::NPC_TRG42_CLIP_SIZE = 5;
// Grenade explode delay
const Float CNPCCloneSoldier::NPC_GRENADE_EXPLODE_DELAY = 3.5;
// Next grenade check delay
const Float CNPCCloneSoldier::NPC_GRENADE_CHECK_DELAY = 6;
// NPC weapon sound radius
const Float CNPCCloneSoldier::NPC_WEAPON_SOUND_RADIUS = 384;
// NPC weapon sound duration
const Float CNPCCloneSoldier::NPC_WEAPON_SOUND_DURATION = 0.3;
// NPC helmet health
const Float CNPCCloneSoldier::NPC_HELMET_HEALTH = 30;
// How much of the damage the helmet takes
const Float CNPCCloneSoldier::NPC_HELMET_DMG_TAKE = 0.6f;
// How much of the damage the helmet absorbs
const Float CNPCCloneSoldier::NPC_HELMET_DMG_ABSORB = 0.2f;
// Minimum enemy distance for support type
const Float CNPCCloneSoldier::NPC_MIN_ENEMY_DISTANCE = 512;
// Maximum tactical position distance for support type
const Float CNPCCloneSoldier::NPC_MAX_TACTICALPOS_DISTANCE = 1024;
// Minimum distance at which we'll throw grenades with a visible enemy
const Float CNPCCloneSoldier::NPC_MIN_GRENADE_DISTANCE = 1024;
// Max grenades on a clone soldier
const Uint32 CNPCCloneSoldier::NPC_NUM_GRENADES = 3;
// Max look distance
const Float CNPCCloneSoldier::NPC_MAX_LOOK_DISTANCE = 4096;
// Max normal firing distance
const Float CNPCCloneSoldier::NPC_MAX_FIRING_DISTANCE = 2048;
// Max normal firing distance for shotgunner
const Float CNPCCloneSoldier::NPC_MAX_SHOTGUNNER_FIRING_DISTANCE = 1024;
// Precise firing distance
const Float CNPCCloneSoldier::NPC_PRECISE_FIRING_DISTANCE = 1024;
// Minimum ambush distance
const Float CNPCCloneSoldier::NPC_MIN_AMBUSH_DISTANCE_DISTANCE = 512;

// Model name for the npc
const Char CNPCCloneSoldier::NPC_MODEL_NAME[] = "models/replica.mdl";
// Pain sound pattern
const Char CNPCCloneSoldier::NPC_PAIN_SOUND_PATTERN[] = "replica/pain%d.wav";
// Number of pain sounds
const Uint32 CNPCCloneSoldier::NPC_NB_PAIN_SOUNDS = 3;
// Death sound pattern
const Char CNPCCloneSoldier::NPC_DEATH_SOUND_PATTERN[] = "replica/death%d.wav";
// Number of death sounds
const Uint32 CNPCCloneSoldier::NPC_NB_DEATH_SOUNDS = 3;

// Array of clone soldier sentences
const Char* CNPCCloneSoldier::NPC_SENTENCES[NUM_NPC_SENTENCES] = 
{
	"RP_GRENADE",
	"RP_ALERT",
	"RP_MONSTER",
	"RP_COVER",
	"RP_THROW",
	"RP_CHARGE",
	"RP_TAUNT"
};

// Bodygroup name for heads
const Char CNPCCloneSoldier::NPC_BODYGROUP_HEADS_NAME[] = "heads";
// Submodel name for normal head
const Char CNPCCloneSoldier::NPC_SUBMODEL_HEAD_NORMAL_NAME[] = "replica_head_normal_reference";
// Submodel name for decapitated head
const Char CNPCCloneSoldier::NPC_SUBMODEL_HEAD_DECAPITATED_NAME[] = "replica_head_decap_reference";

// Bodygroup name for weapons
const Char CNPCCloneSoldier::NPC_BODYGROUP_WEAPONS_NAME[] = "weapons";
// Submodel name for spanner weapon
const Char CNPCCloneSoldier::NPC_SUBMODEL_WEAPON_SIG552_NAME[] = "blank";
// Submodel name for spanner weapon
const Char CNPCCloneSoldier::NPC_SUBMODEL_WEAPON_SHOTGUN_NAME[] = "blank";
// Submodel name for spanner weapon
const Char CNPCCloneSoldier::NPC_SUBMODEL_WEAPON_M249_NAME[] = "blank";
// Submodel name for spanner weapon
const Char CNPCCloneSoldier::NPC_SUBMODEL_WEAPON_TRG42_NAME[] = "blank";
// Submodel name for blank weapon
const Char CNPCCloneSoldier::NPC_SUBMODEL_WEAPON_BLANK_NAME[] = "blank";

// Question asked
CNPCCloneSoldier::npc_question_types_t CNPCCloneSoldier::g_questionAsked = NPC_QUESTION_NONE;
// Time until we talk again
Double CNPCCloneSoldier::g_talkWaitTime = 0;

//==========================================================================
//
// SCHEDULES FOR CNPCCLONESOLDIER CLASS
//
//==========================================================================

//=============================================
// @brief Suppressing Fire
//
//=============================================
ai_task_t taskListScheduleCloneSoldierSuppressingFire[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_FACE_ENEMY,					0),
	AITASK(AI_TASK_RANGE_ATTACK1,				0),
	AITASK(AI_TASK_FACE_ENEMY,					0),
	AITASK(AI_TASK_RANGE_ATTACK1,				0),
	AITASK(AI_TASK_FACE_ENEMY,					0),
	AITASK(AI_TASK_RANGE_ATTACK1,				0),
	AITASK(AI_TASK_FACE_ENEMY,					0),
	AITASK(AI_TASK_RANGE_ATTACK1,				0),
	AITASK(AI_TASK_FACE_ENEMY,					0),
	AITASK(AI_TASK_RANGE_ATTACK1,				0)
};

const CAISchedule scheduleCloneSoldierSuppressingFire(
	// Task list
	taskListScheduleCloneSoldierSuppressingFire, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleCloneSoldierSuppressingFire),
	// AI interrupt mask
	AI_COND_DANGEROUS_ENEMY_CLOSE |
	AI_COND_ENEMY_DEAD |
	AI_COND_LIGHT_DAMAGE |
	AI_COND_HEAVY_DAMAGE |
	AI_COND_HEAR_SOUND |
	AI_COND_NO_AMMO_LOADED |
	AI_COND_FRIENDLY_FIRE,
	// Sound mask
	AI_SOUND_DANGER, 
	// Name
	"Suppressing Fire"
);

//=============================================
// @brief Range Attack 1
//
//=============================================
ai_task_t taskListScheduleCloneSoldierRangeAttack1[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_SET_ACTIVITY,				ACT_IDLE_ANGRY),
	AITASK(AI_TASK_ATTACK_REACTION_DELAY,		0),
	AITASK(AI_TASK_RANGE_ATTACK1,				0),
	AITASK(AI_TASK_FACE_ENEMY,					0),
	AITASK(AI_TASK_RANGE_ATTACK1,				0),
	AITASK(AI_TASK_FACE_ENEMY,					0),
	AITASK(AI_TASK_RANGE_ATTACK1,				0),
	AITASK(AI_TASK_FACE_ENEMY,					0),
	AITASK(AI_TASK_RANGE_ATTACK1,				0),
	AITASK(AI_TASK_FACE_ENEMY,					0),
	AITASK(AI_TASK_RANGE_ATTACK1,				0)
};

const CAISchedule scheduleCloneSoldierRangeAttack1(
	// Task list
	taskListScheduleCloneSoldierRangeAttack1, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleCloneSoldierRangeAttack1),
	// AI interrupt mask
	AI_COND_DANGEROUS_ENEMY_CLOSE |
	AI_COND_NEW_ENEMY |
	AI_COND_ENEMY_DEAD |
	AI_COND_HEAVY_DAMAGE |
	AI_COND_ENEMY_OCCLUDED |
	AI_COND_HEAR_SOUND |
	AI_COND_NO_AMMO_LOADED |
	AI_COND_FRIENDLY_FIRE,
	// Sound mask
	AI_SOUND_DANGER, 
	// Name
	"Range Attack 1"
);

//=============================================
// @brief Range Attack 2
//
//=============================================
ai_task_t taskListScheduleCloneSoldierRangeAttack2[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_CLONE_SOLDIER_TASK_FACE_TOSS_DIR,	0),
	AITASK(AI_TASK_PLAY_SEQUENCE,				(Float)ACT_RANGE_ATTACK2),
	AITASK(AI_TASK_SET_SCHEDULE,				(Float)AI_SCHED_WAIT_FACE_ENEMY)
};

const CAISchedule scheduleCloneSoldierRangeAttack2(
	// Task list
	taskListScheduleCloneSoldierRangeAttack2, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleCloneSoldierRangeAttack2),
	// AI interrupt mask
	AI_COND_DANGEROUS_ENEMY_CLOSE,
	// Sound mask
	AI_SOUND_NONE, 
	// Name
	"Range Attack 2"
);

//=============================================
// @brief Take Tactical Position
//
//=============================================
ai_task_t taskListScheduleCloneSoldierTakeTacticalPosition[] = 
{
	AITASK(AI_TASK_STOP_MOVING,							0),
	AITASK(AI_TASK_SET_FAIL_SCHEDULE,					(Float)AI_SCHED_ELOF_FAIL),
	AITASK(AI_CLONE_SOLDIER_TASK_GET_TACTICAL_POSITION,	0),
	AITASK(AI_CLONE_SOLDIER_TASK_SPEAK_SENTENCE,		0),
	AITASK(AI_TASK_RUN_PATH,							0),
	AITASK(AI_TASK_WAIT_FOR_MOVEMENT,					0),
	AITASK(AI_TASK_STOP_MOVING,							0),
	AITASK(AI_TASK_FACE_ENEMY,							0)
};

const CAISchedule scheduleCloneSoldierTakeTacticalPosition(
	// Task list
	taskListScheduleCloneSoldierTakeTacticalPosition, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleCloneSoldierTakeTacticalPosition),
	// AI interrupt mask
	AI_COND_DANGEROUS_ENEMY_CLOSE |
	AI_COND_NEW_ENEMY |
	AI_COND_LIGHT_DAMAGE |
	AI_COND_HEAVY_DAMAGE |
	AI_COND_CAN_RANGE_ATTACK1 |
	AI_COND_HEARD_ENEMY_NEW_POSITION |
	AI_COND_IN_DANGER,
	// Sound mask
	AI_SOUND_NONE, 
	// Name
	"Take Tactical Position"
);

//=============================================
// @brief Ambush Enemy
//
//=============================================
ai_task_t taskListScheduleCloneSoldierAmbushEnemy[] = 
{
	AITASK(AI_TASK_STOP_MOVING,								0),
	AITASK(AI_TASK_SET_FAIL_SCHEDULE,						(Float)AI_SCHED_ELOF_FAIL),
	AITASK(AI_CLONE_SOLDIER_TASK_GET_AMBUSH_PATH,			0),
	AITASK(AI_CLONE_SOLDIER_TASK_SPEAK_SENTENCE,			0),
	AITASK(AI_TASK_RUN_PATH,								0),
	AITASK(AI_TASK_WAIT_FOR_MOVEMENT,						0),
	AITASK(AI_TASK_STOP_MOVING,								0),
	AITASK(AI_TASK_FACE_ENEMY,								0)
};

const CAISchedule scheduleCloneSoldierAmbushEnemy(
	// Task list
	taskListScheduleCloneSoldierAmbushEnemy, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleCloneSoldierAmbushEnemy),
	// AI interrupt mask
	AI_COND_DANGEROUS_ENEMY_CLOSE |
	AI_COND_NEW_ENEMY |
	AI_COND_LIGHT_DAMAGE |
	AI_COND_HEAVY_DAMAGE |
	AI_COND_CAN_ATTACK |
	AI_COND_IN_DANGER |
	AI_COND_HEARD_ENEMY_NEW_POSITION |
	AI_COND_HEAR_SOUND |
	AI_COND_SHOOT_VECTOR_VALID|
	AI_COND_ENEMY_DEAD,
	// Inverse condition mask
	AI_COND_NONE,
	// Special interrupt schedule
	AI_SCHED_FACE_ENEMY,
	// Special interrupt condition mask
	AI_COND_SHOOT_VECTOR_VALID,
	// Special interrupt exception mask
	AI_COND_DANGEROUS_ENEMY_CLOSE |
	AI_COND_NEW_ENEMY |
	AI_COND_LIGHT_DAMAGE |
	AI_COND_HEAVY_DAMAGE |
	AI_COND_IN_DANGER |
	AI_COND_HEARD_ENEMY_NEW_POSITION |
	AI_COND_HEAR_SOUND |
	AI_COND_ENEMY_DEAD,
	// Special interrupt requirement mask
	AI_COND_NONE,
	// Sound mask
	AI_SOUND_DANGER, 
	// Name
	"Ambush Enemy"
);

//=============================================
// @brief Idle Sweep
//
//=============================================
ai_task_t taskListScheduleCloneSoldierIdleSweep[] = 
{
	AITASK(AI_TASK_TURN_LEFT,							180),
	AITASK(AI_TASK_WAIT,								2)
};

const CAISchedule scheduleCloneSoldierIdleSweep(
	// Task list
	taskListScheduleCloneSoldierIdleSweep, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleCloneSoldierIdleSweep),
	// AI interrupt mask
	AI_COND_DANGEROUS_ENEMY_CLOSE |
	AI_COND_NEW_ENEMY |
	AI_COND_LIGHT_DAMAGE |
	AI_COND_HEAVY_DAMAGE |
	AI_COND_CAN_ATTACK |
	AI_COND_IN_DANGER |
	AI_COND_HEAR_SOUND,
	// Sound mask
	AI_SOUND_DANGER |
	AI_SOUND_WORLD |
	AI_SOUND_PLAYER |
	AI_SOUND_COMBAT, 
	// Name
	"Idle Sweep"
);

//=============================================
// @brief Hide and Wait
//
//=============================================
ai_task_t taskListScheduleCloneSoldierHideAndWait[] = 
{
	AITASK(AI_TASK_STOP_MOVING,							0),
	AITASK(AI_TASK_FIND_COVER_FROM_ENEMY,				0),
	AITASK(AI_CLONE_SOLDIER_TASK_GET_HIDING_POSITION,	0),
	AITASK(AI_TASK_RUN_PATH,							0),
	AITASK(AI_TASK_WAIT_FOR_MOVEMENT,					0),
	AITASK(AI_TASK_STOP_MOVING,							0),
	AITASK(AI_TASK_WAIT_FACE_ENEMY_INDEFINITE,			0)
};

const CAISchedule scheduleCloneSoldierHideAndWait(
	// Task list
	taskListScheduleCloneSoldierHideAndWait, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleCloneSoldierHideAndWait),
	// AI interrupt mask
	AI_COND_DANGEROUS_ENEMY_CLOSE |
	AI_COND_NEW_ENEMY |
	AI_COND_LIGHT_DAMAGE |
	AI_COND_HEAVY_DAMAGE |
	AI_COND_CAN_ATTACK,
	// Sound mask
	AI_SOUND_NONE, 
	// Name
	"Hide and Wait"
);

//==========================================================================
//
// SCHEDULES FOR CNPCCLONESOLDIER CLASS
//
//==========================================================================

LINK_ENTITY_TO_CLASS( npc_clone_soldier, CNPCCloneSoldier );

//=============================================
// @brief Constructor
//
//=============================================
CNPCCloneSoldier::CNPCCloneSoldier( edict_t* pedict ):
	CPatrolNPC(pedict),
	m_nextPainTime(0),
	m_nextGrenadeCheckTime(0),
	m_nextSweepTime(0),
	m_helmetHealth(0),
	m_preciseDistance(0),
	m_tossGrenade(false),
	m_isStanding(false),
	m_takeAttackChance(false),
	m_isPreciseAiming(false),
	m_sentence(0),
	m_voicePitch(0),
	m_attackType(0),
	m_numGrenades(0),
	m_headsBodyGroupIndex(NO_POSITION),
	m_headNormalSubmodelIndex(NO_POSITION),
	m_headDecapitatedSubmodelIndex(NO_POSITION),
	m_weaponsBodyGroupIndex(NO_POSITION),
	m_weaponSig552SubmodelIndex(NO_POSITION),
	m_weaponShotgunSubmodelIndex(NO_POSITION),
	m_weaponM249SubmodelIndex(NO_POSITION),
	m_weaponTRG42SubmodelIndex(NO_POSITION),
	m_weaponBlankSubmodelIndex(NO_POSITION)
{
}

//=============================================
// @brief Destructor
//
//=============================================
CNPCCloneSoldier::~CNPCCloneSoldier( void )
{
}

//=============================================
// @brief Spawns the entity
//
//=============================================
bool CNPCCloneSoldier::Spawn( void )
{
	// Set modelname
	m_pFields->modelname = gd_engfuncs.pfnAllocString(NPC_MODEL_NAME);

	if(!CPatrolNPC::Spawn())
		return false;

	gd_engfuncs.pfnSetMinsMaxs(m_pEdict, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	m_pState->solid = SOLID_SLIDEBOX;
	m_pState->movetype = MOVETYPE_STEP;
	m_pState->health = GetSkillCVarValue(g_skillcvars.skillReplicaHealth);
	m_pState->view_offset = NPC_VIEW_OFFSET;

	m_bloodColor = BLOOD_RED;
	m_fieldOfView = VIEW_FIELD_WIDE;
	m_npcState = NPC_STATE_NONE;
	m_sentence = NPC_SENT_NONE;
	m_numGrenades = NPC_NUM_GRENADES;
	m_helmetHealth = NPC_HELMET_HEALTH;
	m_attackType = NPC_TYPE_SUPPORT;

	// Set capabilities
	m_capabilityBits |= (AI_CAP_SQUAD|AI_CAP_HEAR|AI_CAP_TURN_HEAD|AI_CAP_GROUP_DOORS|AI_CAP_ATTACK_BLEND_SEQ);

	if(!CPlayerEntity::IsUsingCheatCommand())
	{
		// Protect against invalid weapon states
		if(!(m_pState->weapons & (NPC_WEAPON_SHOTGUN|NPC_WEAPON_M249|NPC_WEAPON_TRG42|NPC_WEAPON_SIG552)))
			m_pState->weapons |= NPC_WEAPON_SIG552;
	}
	else
	{
		switch(Common::RandomLong(0, 3))
		{
			case 0: m_pState->weapons = NPC_WEAPON_SHOTGUN; 
				break;
			case 1: m_pState->weapons = NPC_WEAPON_SIG552; 
				break;
			case 2: m_pState->weapons = NPC_WEAPON_TRG42; 
				break;
			case 3: m_pState->weapons = NPC_WEAPON_M249;
				break;
		}

		if(Common::RandomLong(0, 1))
			m_pState->weapons |= NPC_WEAPON_GRENADE;
	}

	Int32 weaponSubmodelToSet = NO_POSITION;
	if(m_pState->weapons & NPC_WEAPON_SHOTGUN)
	{
		weaponSubmodelToSet = m_weaponShotgunSubmodelIndex;
		m_clipSize = NPC_SHOTGUN_CLIP_SIZE;
	}
	else if(m_pState->weapons & NPC_WEAPON_M249)
	{
		weaponSubmodelToSet = m_weaponM249SubmodelIndex;
		m_clipSize = NPC_M249_CLIP_SIZE;
	}
	else if(m_pState->weapons & NPC_WEAPON_TRG42)
	{
		weaponSubmodelToSet = m_weaponTRG42SubmodelIndex;
		m_clipSize = NPC_TRG42_CLIP_SIZE;
	}
	else if(m_pState->weapons & NPC_WEAPON_SIG552)
	{
		weaponSubmodelToSet = m_weaponSig552SubmodelIndex;
		m_clipSize = NPC_SIG552_CLIP_SIZE;
	}

	if(m_weaponsBodyGroupIndex != NO_POSITION && weaponSubmodelToSet != NO_POSITION)
		SetBodyGroup(m_weaponsBodyGroupIndex, weaponSubmodelToSet);

	m_ammoLoaded = m_clipSize;
	m_nextSweepTime = g_pGameVars->time + Common::RandomFloat(2, 6);
	m_voicePitch = Common::RandomLong(PITCH_NORM-5, PITCH_NORM+5);

	// Initialize the NPC
	InitNPC();

	return true;
}

//=============================================
// @brief Performs precache functions
//
//=============================================
void CNPCCloneSoldier::Precache( void )
{
	CPatrolNPC::Precache();

	Util::PrecacheFixedNbSounds(NPC_PAIN_SOUND_PATTERN, NPC_NB_PAIN_SOUNDS);
	Util::PrecacheFixedNbSounds(NPC_DEATH_SOUND_PATTERN, NPC_NB_DEATH_SOUNDS);

	gd_engfuncs.pfnPrecacheSound(NPC_SIG552_FIRING_SOUND);
	gd_engfuncs.pfnPrecacheSound(NPC_M249_FIRING_SOUND);
	gd_engfuncs.pfnPrecacheSound(NPC_SHOTGUN_FIRING_SOUND);
	gd_engfuncs.pfnPrecacheSound(NPC_TRG42_FIRING_SOUND);
	gd_engfuncs.pfnPrecacheSound(NPC_HIT_MISS_SOUND);

	if(g_pSentencesFile)
	{
		for(Uint32 i = 0; i < NUM_NPC_SENTENCES; i++)
			g_pSentencesFile->PrecacheGroup(NPC_SENTENCES[i]);
	}
}

//=============================================
// @brief Sets extra model info after setting the model
//
//=============================================
void CNPCCloneSoldier::PostModelSet( void )
{
	m_headsBodyGroupIndex = GetBodyGroupIndexByName(NPC_BODYGROUP_HEADS_NAME);
	if(m_headsBodyGroupIndex != NO_POSITION)
	{
		m_headNormalSubmodelIndex = GetSubmodelIndexByName(m_headsBodyGroupIndex, NPC_SUBMODEL_HEAD_NORMAL_NAME);
		m_headDecapitatedSubmodelIndex = GetSubmodelIndexByName(m_headsBodyGroupIndex, NPC_SUBMODEL_HEAD_DECAPITATED_NAME);
	}

	m_weaponsBodyGroupIndex = GetBodyGroupIndexByName(NPC_BODYGROUP_WEAPONS_NAME);
	if(m_headsBodyGroupIndex != NO_POSITION)
	{
		m_weaponSig552SubmodelIndex = GetSubmodelIndexByName(m_weaponsBodyGroupIndex, NPC_SUBMODEL_WEAPON_SIG552_NAME);
		m_weaponShotgunSubmodelIndex = GetSubmodelIndexByName(m_weaponsBodyGroupIndex, NPC_SUBMODEL_WEAPON_SHOTGUN_NAME);
		m_weaponM249SubmodelIndex = GetSubmodelIndexByName(m_weaponsBodyGroupIndex, NPC_SUBMODEL_WEAPON_M249_NAME);
		m_weaponTRG42SubmodelIndex = GetSubmodelIndexByName(m_weaponsBodyGroupIndex, NPC_SUBMODEL_WEAPON_TRG42_NAME);
		m_weaponBlankSubmodelIndex = GetSubmodelIndexByName(m_weaponsBodyGroupIndex, NPC_SUBMODEL_WEAPON_BLANK_NAME);
	}
}

//=============================================
// @brief Returns the classification
//
//=============================================
Int32 CNPCCloneSoldier::GetClassification( void ) const
{
	return CLASS_HUMAN_HOSTILE;
}

//=============================================
// @brief Handles animation events
//
//=============================================
void CNPCCloneSoldier::HandleAnimationEvent( const mstudioevent_t* pevent )
{
	switch(pevent->event)
	{
	case NPC_AE_CAUGHT_ENEMY:
		{
			if(CanSpeak())
			{
				PlaySentence("RP_MONSTER", 0, VOL_NORM, ATTN_NORM, 0, true);
				Spoke();
			}
		}
		break;
	case NPC_AE_DROP_GUN:
		{
			if(GetBodyGroupValue(m_weaponsBodyGroupIndex) != m_weaponBlankSubmodelIndex)
			{
				if(m_weaponsBodyGroupIndex != NO_POSITION && m_weaponBlankSubmodelIndex != NO_POSITION)
					SetBodyGroup(m_weaponsBodyGroupIndex, m_weaponBlankSubmodelIndex);

				if(!m_dontDropWeapons)
				{
					CString weaponName;
					// These weapons have been cut from the public release, so
					// weaponName remains empty

					if(!weaponName.empty())
					{
						Vector gunPosition;
						GetAttachment(NPC_WEAPON_ATTACHMENT_INDEX, gunPosition);

						Vector angles(m_pState->angles);
						angles[PITCH] = angles[ROLL] = 0;

						DropItem(weaponName.c_str(), gunPosition, angles);
					}
				}
			}
		}
		break;
	case NPC_AE_RELOAD:
		{
			m_ammoLoaded = m_clipSize;
			ClearConditions(AI_COND_NO_AMMO_LOADED);
		}
		break;
	case NPC_AE_KICK:
		{
			CBaseEntity* pKickEntity = GetKickEntity(NPC_KICK_DISTANCE);
			if(pKickEntity)
			{
				if(!pKickEntity->IsBrushModel())
				{
					Vector forward, up;
					Math::AngleVectors(m_pState->angles, &forward, nullptr, &up);

					if(pKickEntity->IsPlayer())
					{
						Vector punchAngles(15, 0, 0);
						pKickEntity->SetPunchAngle(punchAngles);
					}

					Vector hurtVelocity = pKickEntity->GetVelocity() + forward*100 + up*50;
					pKickEntity->SetVelocity(hurtVelocity);
				}

				Float hurtDamage = GetSkillCVarValue(g_skillcvars.skillReplicaKickDmg);
				if(hurtDamage > 0)
					pKickEntity->TakeDamage(this, this, hurtDamage, DMG_MELEE);
			}
		}
		break;
	case NPC_AE_BURST1:
	case NPC_AE_BURST1_PRECISE:
	case NPC_AE_BURST2:
	case NPC_AE_BURST2_PRECISE:
	case NPC_AE_BURST3:
	case NPC_AE_BURST3_PRECISE:
		{
			Uint32 numShots = 0;
			const char* pstrSound = nullptr;

			if(m_pState->weapons & NPC_WEAPON_SIG552)
			{
				pstrSound = NPC_SIG552_FIRING_SOUND;
				numShots = 1;
			}
			else if(m_pState->weapons & NPC_WEAPON_SHOTGUN)
			{
				pstrSound = NPC_SHOTGUN_FIRING_SOUND;
				numShots = GetSkillCVarValue(g_skillcvars.skillReplicaPellets);
			}
			else if(m_pState->weapons & NPC_WEAPON_M249)
			{
				pstrSound = NPC_M249_FIRING_SOUND;
				numShots = 1;
			}
			else if(m_pState->weapons & NPC_WEAPON_TRG42)
			{
				pstrSound = NPC_TRG42_FIRING_SOUND;
				numShots = 1;
			}

			if(numShots > 0 && pstrSound)
			{
				if(pevent->event == NPC_AE_BURST1_PRECISE || pevent->event == NPC_AE_BURST2_PRECISE || pevent->event == NPC_AE_BURST3_PRECISE)
					m_isPreciseAiming = true;

				FireWeapon(numShots, pstrSound, 1, &m_ammoLoaded);
				gAISounds.AddSound(this, AI_SOUND_COMBAT, NPC_WEAPON_SOUND_RADIUS, VOL_NORM, NPC_WEAPON_SOUND_DURATION);
				m_isPreciseAiming = false;
			}
		}
		break;
	case NPC_AE_GREN_TOSS:
		{
			Vector forward;
			Math::AngleVectors(m_pState->angles, &forward);

			Float grenadeRadius = GetSkillCVarValue(g_skillcvars.skillGrenadeRadius);
			Float grenadeDamage = GetSkillCVarValue(g_skillcvars.skillGrenadeDmg);

			CGrenade::CreateTimed(this, GetGunPosition(), m_grenadeTossVelocity, NPC_GRENADE_EXPLODE_DELAY, grenadeRadius, grenadeDamage, true);

			m_tossGrenade = false;
			m_nextGrenadeCheckTime = g_pGameVars->time + NPC_GRENADE_CHECK_DELAY;

			CBaseEntity* pSquadLeader = GetSquadLeader();
			if(pSquadLeader)
			{
				Uint32 numGrenades = pSquadLeader->GetNumGrenades();
				if(numGrenades > 0)
				{
					numGrenades--;
					pSquadLeader->SetNumGrenades(numGrenades);
				}
			}
		}
		break;
	default:
		CPatrolNPC::HandleAnimationEvent(pevent);
		break;
	}
}

//=============================================
// @brief Sets the ideal yaw speed
//
//=============================================
void CNPCCloneSoldier::SetYawSpeed( void )
{
	m_pState->yawspeed = NPC_YAW_SPEED;
}

//=============================================
// @brief Returns the sound mask for the NPC
//
//=============================================
Uint64 CNPCCloneSoldier::GetSoundMask( void )
{
	return (AI_SOUND_WORLD|AI_SOUND_COMBAT|AI_SOUND_DANGER|AI_SOUND_PLAYER);
}

//=============================================
// @brief Returns the ideal schedule
//
//=============================================
const CAISchedule* CNPCCloneSoldier::GetSchedule( void )
{
	// Clear old sentence
	m_sentence = NPC_SENT_NONE;

	// Run away from danger sounds
	if(CheckConditions(AI_COND_HEAR_SOUND) && m_pBestSound && (m_pBestSound->typeflags & AI_SOUND_DANGER))
		return GetScheduleByIndex(AI_SCHED_TAKE_COVER_FROM_BEST_SOUND);

	// Dodge any dangerous enemies
	if(m_dangerousEnemy && CheckConditions(AI_COND_DANGEROUS_ENEMY_CLOSE) && !HasMemory(AI_MEMORY_DODGE_ENEMY_FAILED))
		return GetScheduleByIndex(AI_SCHED_DODGE_ENEMY);

	switch(m_npcState)
	{
	case NPC_STATE_COMBAT:
		{
			if(CheckConditions(AI_COND_ENEMY_DEAD))
			{
				// Call base class to manage dead enemies
				return CPatrolNPC::GetSchedule();
			}
			else if(CheckConditions(AI_COND_NEW_ENEMY) && IsInSquad())
			{
				CBaseEntity* pSquadLeader = GetSquadLeader();
				if(pSquadLeader)
					pSquadLeader->SetEnemyEluded(false);

				if(IsSquadLeader() && IsVisibleBySquadMembers())
				{
					if(CanSpeak())
					{
						PlaySentence("RP_MONSTER", 0, VOL_NORM, ATTN_NORM, 0, true);
						Spoke();
					}

					if(!m_signalledSuppressingFire)
					{
						return GetScheduleByIndex(AI_SQUADNPC_SCHED_SIGNAL_SUPPRESSING_FIRE);
					}
				}

				return GetCombatSchedule();
			}
			else if(CheckConditions(AI_COND_NO_AMMO_LOADED))
			{
				return GetReloadSchedule();
			}
			else if(CheckConditions(AI_COND_LIGHT_DAMAGE))
			{
				if(Common::RandomLong(0, 99) <= 90 && m_enemy 
					&& (GetNavigablePosition()-m_enemy->GetNavigablePosition()).Length() < NPC_MIN_ENEMY_DISTANCE)
				{
					if(CanSpeak())
					{
						PlaySentence("RP_COVER", 0, VOL_NORM, ATTN_NORM, 0, true);
						Spoke();
					}

					return GetScheduleByIndex(AI_SCHED_TAKE_COVER_FROM_ENEMY);
				}
				else
				{
					return GetScheduleByIndex(AI_SCHED_SMALL_FLINCH);
				}
			}
			else if(CheckConditions(AI_COND_CAN_MELEE_ATTACK1))
			{
				return GetScheduleByIndex(AI_SCHED_MELEE_ATTACK1);
			}
			else if(CheckConditions(AI_COND_CAN_RANGE_ATTACK2)
				&& (!CheckConditions(AI_COND_SEE_ENEMY) 
				|| (m_pState->origin - m_enemyLastKnownPosition).Length() > NPC_MIN_GRENADE_DISTANCE))
			{
				if(CanSpeak())
				{
					PlaySentence("RP_THROW", 0, VOL_NORM, ATTN_NORM, 0, true);
					Spoke();
				}

				return GetScheduleByIndex(AI_SCHED_RANGE_ATTACK2);
			}
			else if(CheckConditions(AI_COND_CAN_RANGE_ATTACK1))
			{
				if(IsInSquad())
				{
					CBaseEntity* pSquadLeader = GetSquadLeader();
					if(pSquadLeader && pSquadLeader->HasEnemyEluded() && !CheckConditions(AI_COND_ENEMY_FACING_ME))
					{
						pSquadLeader->SetEnemyEluded(false);
						return GetScheduleByIndex(AI_SQUADNPC_SCHED_FOUND_ENEMY);
					}
				}

				return GetCombatSchedule();
			}
			else if(CheckConditions(AI_COND_ENEMY_OCCLUDED))
			{
				if(m_attackType == NPC_TYPE_OFFENSIVE)
				{
					if(CheckConditions(AI_COND_CAN_RANGE_ATTACK2))
					{
						if(CanSpeak())
						{
							PlaySentence("RP_THROW", 0, VOL_NORM, ATTN_NORM, 0, true);
							Spoke();
						}

						return GetScheduleByIndex(AI_SCHED_RANGE_ATTACK2);
					}
					else if(!CheckConditions(AI_COND_ENEMY_NOT_FOUND))
					{
						if(CanSpeak())
						{
							PlaySentence("RP_CHARGE", 0, VOL_NORM, ATTN_NORM, 0, true);
							Spoke();
						}

						return GetScheduleByIndex(AI_SCHED_ESTABLISH_LINE_OF_FIRE);
					}
					else
					{
						return GetScheduleByIndex(AI_SCHED_FIND_ENEMIES);
					}
				}
				else
				{
					// Wait out the enemy
					if(CanSpeak())
					{
						PlaySentence("RP_TAUNT", 0, VOL_NORM, ATTN_NORM, 0, true);
						Spoke();
					}

					if(!HasMemory(AI_MEMORY_SOUGHT_TACTICAL) && !CheckConditions(AI_COND_ENEMY_NOT_FOUND))
					{
						SetMemory(AI_MEMORY_SOUGHT_TACTICAL);
						return GetScheduleByIndex(AI_CLONE_SOLDIER_SCHED_TAKE_TACTICAL_POSITION);
					}
					else
					{
						CBaseEntity *pLeader = GetSquadLeader();

						if( pLeader && (g_pGameVars->time - pLeader->GetLastEnemySightTime()) > Common::RandomFloat(20, 40) )
						{
							// If the enemy's been out of sight for a while, ambush
							return GetScheduleByIndex( AI_CLONE_SOLDIER_SCHED_AMBUSH_ENEMY );
						}
						else
						{
							// Wait for enemy to try again, but do not engage
							return GetScheduleByIndex(AI_SCHED_STANDOFF);
						}
					}
				}
			}
			else if(CheckConditions(AI_COND_SEE_ENEMY) && !CheckConditions(AI_COND_CAN_RANGE_ATTACK1))
			{
				if(!m_enemy->IsNPCDangerous())
					return GetCombatSchedule();
				else
					return GetScheduleByIndex(AI_SCHED_TAKE_COVER_FROM_ENEMY);
			}
		}
		break;
	case NPC_STATE_ALERT:
	case NPC_STATE_IDLE:
		{
			if(CheckConditions(AI_COND_HEAR_SOUND))
			{
				if(m_pBestSound && (m_pBestSound->typeflags & AI_SOUND_DANGER))
					return GetScheduleByIndex(AI_SCHED_TAKE_COVER_FROM_BEST_SOUND);
				else if(m_pBestSound && (m_pBestSound->typeflags & (AI_SOUND_COMBAT|AI_SOUND_PLAYER)))
					return GetScheduleByIndex(AI_SCHED_INVESTIGATE_SOUND);
			}

			if(m_shouldPatrol && m_nextSweepTime <= g_pGameVars->time)
			{
				m_nextSweepTime = g_pGameVars->time + Common::RandomFloat(4, 6);
				return GetScheduleByIndex(AI_CLONE_SOLDIER_SCHED_IDLE_SWEEP);
			}
		}
		break;
	}

	return CPatrolNPC::GetSchedule();
}

//=============================================
// @brief Returns a schedule by it's index
//
//=============================================
const CAISchedule* CNPCCloneSoldier::GetScheduleByIndex( Int32 scheduleIndex )
{
	switch(scheduleIndex)
	{
	case AI_CLONE_SOLDIER_SCHED_HIDE_AND_WAIT:
		{
			return &scheduleCloneSoldierHideAndWait;
		}
		break;
	case AI_SCHED_TAKE_COVER_FROM_BEST_SOUND:
		{
			return &scheduleTakeCoverFromBestSoundWithCower;
		}
		break;
	case AI_CLONE_SOLDIER_SCHED_SUPPRESSING_FIRE:
		{
			return &scheduleCloneSoldierSuppressingFire;
		}
		break;
	case AI_CLONE_SOLDIER_SCHED_TAKE_TACTICAL_POSITION:
		{
			return &scheduleCloneSoldierTakeTacticalPosition;
		}
		break;
	case AI_CLONE_SOLDIER_SCHED_IDLE_SWEEP:
		{
			return &scheduleCloneSoldierIdleSweep;
		}
		break;
	case AI_CLONE_SOLDIER_SCHED_AMBUSH_ENEMY:
		{
			return &scheduleCloneSoldierAmbushEnemy;
		}
		break;
	case AI_SCHED_RANGE_ATTACK1:
		{
			return &scheduleCloneSoldierRangeAttack1;
		}
		break;
	case AI_SCHED_RANGE_ATTACK2:
		{
			return &scheduleCloneSoldierRangeAttack2;
		}
		break;
	case AI_SCHED_VICTORY_DANCE:
		{
			return CPatrolNPC::GetScheduleByIndex(AI_SCHED_INSPECT_ENEMY_CORPSE);
		}
		break;
	}

	return CPatrolNPC::GetScheduleByIndex(scheduleIndex);
}

//=============================================
// @brief Plays pain sounds
//
//=============================================
void CNPCCloneSoldier::EmitPainSound( void )
{
	if(m_nextPainTime > g_pGameVars->time)
		return;

	Util::PlayRandomEntitySound(this, NPC_PAIN_SOUND_PATTERN, NPC_NB_PAIN_SOUNDS, SND_CHAN_VOICE, VOL_NORM, ATTN_NORM, GetVoicePitch());
	m_nextPainTime = g_pGameVars->time + Common::RandomFloat(0.5, 0.75);
}

//=============================================
// @brief Plays death sounds
//
//=============================================
void CNPCCloneSoldier::EmitDeathSound( void )
{
	Util::PlayRandomEntitySound(this, NPC_DEATH_SOUND_PATTERN, NPC_NB_DEATH_SOUNDS, SND_CHAN_VOICE, VOL_NORM, ATTN_NORM, GetVoicePitch());
}

//=============================================
// @brief Declares save-restore fields
//
//=============================================
void CNPCCloneSoldier::DeclareSaveFields( void )
{
	CPatrolNPC::DeclareSaveFields();

	DeclareSaveField(DEFINE_DATA_FIELD(CNPCCloneSoldier, m_nextGrenadeCheckTime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CNPCCloneSoldier, m_nextPainTime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CNPCCloneSoldier, m_voicePitch, EFIELD_UINT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CNPCCloneSoldier, m_sentence, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CNPCCloneSoldier, m_attackType, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CNPCCloneSoldier, m_numGrenades, EFIELD_UINT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CNPCCloneSoldier, m_helmetHealth, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CNPCCloneSoldier, m_preciseDistance, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CNPCCloneSoldier, m_tacticalCoverage, EFIELD_FLOAT));
}

//=============================================
// @brief Handles damage calculation for a hitscan
//
//=============================================
void CNPCCloneSoldier::TraceAttack( CBaseEntity* pAttacker, Float damage, const Vector& direction, trace_t& tr, Int32 damageFlags )
{
	// Remember damage
	Float _damage = damage;

	if(tr.hitgroup == HITGROUP_HELMET)
	{
		if(m_helmetHealth > 0 && damageFlags & (DMG_BULLET|DMG_EXPLOSION|DMG_MELEE|DMG_SLASH))
		{
			// Determine amounts
			Float dmgAbsorbed = _damage - m_helmetHealth;
			Float finalDamageAbsorb = SDL_fabs(_min(0, dmgAbsorbed));
			_damage = finalDamageAbsorb * NPC_HELMET_DMG_ABSORB;
			
			Float dmgFullAdd = damage - finalDamageAbsorb;
			if(dmgFullAdd > 0)
			{
				// Helmet is useless
				_damage += dmgFullAdd;
				m_helmetHealth = 0;
			}
			else
			{
				// Weaken the helmet
				m_helmetHealth -= finalDamageAbsorb * NPC_HELMET_DMG_TAKE;
			}

			Util::Ricochet(tr.endpos, tr.plane.normal, true);
		}

		// Change to headshot
		tr.hitgroup = HITGROUP_HEAD;
	}

	CPatrolNPC::TraceAttack(pAttacker, _damage, direction, tr, damageFlags);
}

//=============================================
// @brief Returns a sequence for an activity type
//
//=============================================
Int32 CNPCCloneSoldier::FindActivity( Int32 activity )
{
	switch( activity )
	{
	case ACT_RANGE_ATTACK1:
		{
			if (m_pState->weapons & NPC_WEAPON_SIG552)
			{
				if ( m_enemy && (m_enemy->GetOrigin() - m_pState->origin).Length() > m_preciseDistance )
				{
					if ( m_isStanding )
						return FindSequence( "standing_sig552_precise" );
					else
						return FindSequence( "crouching_sig552_precise" );
				}
				else
				{
					if ( m_isStanding )
						return FindSequence( "standing_sig552" );
					else
						return FindSequence( "crouching_sig552" );
				}
			}
			else if (m_pState->weapons & NPC_WEAPON_M249)
				return FindSequence( "shoot_m249" );
			else if (m_pState->weapons & NPC_WEAPON_TRG42)
				return FindSequence( "fire_trg42" );
			else
				return FindSequence( "shoot_spas12" );
		}
		break;
	case ACT_RELOAD:
		{
			if (m_pState->weapons & NPC_WEAPON_SHOTGUN)
				return FindSequence( "reload_spas12" );
			else
				return FindSequence( "reload_sig552_standing" );
		}
		break;
	default:
		return CPatrolNPC::FindActivity( activity );
		break;
	}
}

//=============================================
// @brief Returns the ideal activity
//
//=============================================
Int32 CNPCCloneSoldier::GetIdealActivity( void )
{
	if(m_npcState == NPC_STATE_SCRIPT)
		return CPatrolNPC::GetIdealActivity();

	switch(m_idealActivity)
	{
	case ACT_IDLE_ANGRY:
	case ACT_IDLE:
		{
			if(m_npcState == NPC_STATE_COMBAT)
				return ACT_IDLE_ANGRY;
			else
				return CPatrolNPC::GetIdealActivity();
		}
		break;
	default:
		return CPatrolNPC::GetIdealActivity();
		break;
	}
}

//=============================================
// @brief Set by an NPC if this NPC is blocking them
//
//=============================================
void CNPCCloneSoldier::SetPathBlocked( CBaseEntity* pBlockedEntity, const Vector& destination )
{
	m_nextSweepTime = g_pGameVars->time + Common::RandomLong(5, 8);
	CPatrolNPC::SetPathBlocked(pBlockedEntity, destination);
}

//=============================================
// @brief Plays idle sounds
//
//=============================================
void CNPCCloneSoldier::EmitIdleSound( void )
{
	if(!CanSpeak())
		return;
		
	if(g_questionAsked != NPC_QUESTION_NONE || Common::RandomLong(0, 1))
	{
		if(g_questionAsked == NPC_QUESTION_NONE)
		{
			switch(Common::RandomLong(0, 2))
			{
			case 0: // Check in request
				PlaySentence("RP_CHECK", 0, VOL_NORM, ATTN_NORM, 0, true);
				g_questionAsked = NPC_QUESTION_CHECKIN;
				break;
			case 1: // Question
				PlaySentence("RP_QUEST", 0, VOL_NORM, ATTN_NORM, 0, true);
				g_questionAsked = NPC_QUESTION_NORMAL;
				break;
			case 2: // Statement
				PlaySentence("RP_IDLE", 0, VOL_NORM, ATTN_NORM, 0, true);
				g_questionAsked = NPC_QUESTION_IDLE;
				break;
			}
		}
		else
		{
			switch(g_questionAsked)
			{
			case NPC_QUESTION_CHECKIN: // Check in request
				PlaySentence("RP_CLEAR", 0, VOL_NORM, ATTN_NORM, 0, true);
				g_questionAsked = NPC_QUESTION_CHECKIN;
				break;
			case NPC_QUESTION_NORMAL: // Question
				PlaySentence("RP_ANSWER", 0, VOL_NORM, ATTN_NORM, 0, true);
				g_questionAsked = NPC_QUESTION_NORMAL;
				break;
			}

			g_questionAsked = NPC_QUESTION_NONE;
		}

		Spoke();
	}
}

//=============================================
// @brief Checks the ammo capacity
//
//=============================================
void CNPCCloneSoldier::CheckAmmo( void )
{
	if(!m_ammoLoaded)
		SetConditions(AI_COND_NO_AMMO_LOADED);
}

//=============================================
// @brief Returns the gun position
//
//=============================================
Vector CNPCCloneSoldier::GetGunPosition( stance_t stance )
{
	Vector offset;

	switch(stance)
	{
	case STANCE_ACTUAL:
		{
			if(m_isStanding)
				offset = NPC_GUN_POSITION_STANDING_OFFSET;
			else
				offset = NPC_GUN_POSITION_CROUCHING_OFFSET;
		}
		break;
	case STANCE_STANDING:
		{
			offset = NPC_GUN_POSITION_STANDING_OFFSET;
		}
		break;
	case STANCE_CROUCHING:
		{
			offset = NPC_GUN_POSITION_CROUCHING_OFFSET;
		}
		break;
	}

	return m_pState->origin +  offset;
}

//=============================================
// @brief Tells if we can check the attacks
//
//=============================================
bool CNPCCloneSoldier::CanCheckAttacks( void ) const
{
	if(!CheckConditions(AI_COND_ENEMY_TOO_FAR))
		return true;
	else
		return false;
}

//=============================================
// @brief Checks if we can do range attack 1
//
//=============================================
bool CNPCCloneSoldier::CheckRangeAttack1( Float dp, Float distance )
{
	if(CheckConditions(AI_COND_ENEMY_OCCLUDED|AI_COND_FRIENDLY_FIRE) || dp < NPC_FIRING_ANGLE_TRESHOLD || distance > m_firingDistance)
		return false;

	if(!m_enemy->IsPlayer() && distance < NPC_KICK_TRESHOLD_DISTANCE && m_enemy->GetSize().z > NPC_ENEMY_MIN_KICK_SIZE)
		return false;

	return CheckFiringWithStance((*m_enemy), Weapon_GetConeSize(GetFiringCone()), m_isStanding);
}

//=============================================
// @brief Checks if we can do range attack 2
//
//=============================================
bool CNPCCloneSoldier::CheckRangeAttack2( Float dp, Float distance )
{
	if(!(m_pState->weapons & NPC_WEAPON_GRENADE))
		return false;

	// Prevent grenade spam
	if(Common::RandomLong(0, 2) != 0)
		return false;;

	// Check if squad exhausted grenades
	CBaseEntity* pSquadLeader = GetSquadLeader();
	if(!pSquadLeader || !pSquadLeader->GetNumGrenades())
		return false;

	return CheckGrenadeToss(m_nextGrenadeCheckTime, m_tossGrenade, m_grenadeTossVelocity);
}

//=============================================
// @brief Checks if we can do melee attack 1
//
//=============================================
bool CNPCCloneSoldier::CheckMeleeAttack1( Float dp, Float distance )
{
	if(!m_enemy || m_enemy->IsNPCDangerous())
		return false;
	else if(distance <= NPC_KICK_TRESHOLD_DISTANCE && dp >= 0.7 && m_enemy->GetSize().z >= NPC_ENEMY_MIN_KICK_SIZE)
		return true;
	else
		return false;
}

//=============================================
// @brief Performs pre-schedule think functions
//
//=============================================
void CNPCCloneSoldier::PreScheduleThink( void )
{
	if(CheckConditions(AI_COND_ENEMY_OCCLUDED) 
		&& !CheckConditions(AI_COND_ENEMY_NOT_FOUND) 
		&& HasMemory(AI_MEMORY_SOUGHT_TACTICAL))
		ClearMemory(AI_MEMORY_SOUGHT_TACTICAL);

	if(m_enemy && m_npcState == NPC_STATE_COMBAT && !m_takeAttackChance && m_tacticalCoverage > 0)
	{
		Vector lookerPosition = m_enemyLastKnownPosition + m_enemy->GetViewOffset(true);
		Float coverage = CalculateCoverage(m_movementGoalPosition, m_pState->view_offset, lookerPosition);
		if(coverage < m_tacticalCoverage)
			m_takeAttackChance = true;
	}

	CPatrolNPC::PreScheduleThink();
}

//=============================================
// @brief Returns the firing cone used
//
//=============================================
const Uint32 CNPCCloneSoldier::GetFiringCone( bool attenuateByFog )
{
	Int32 skillCvar;
	if( m_pState->weapons & NPC_WEAPON_SHOTGUN)
		skillCvar = g_skillcvars.skillReplicaShotgunConeSize;
	else if( m_pState->weapons & NPC_WEAPON_SIG552)
		skillCvar = (m_isPreciseAiming ? g_skillcvars.skillReplicaSig552ConeSizePrecise : g_skillcvars.skillReplicaSig552ConeSize);
	else if( m_pState->weapons & NPC_WEAPON_TRG42)
		skillCvar = g_skillcvars.skillReplicaTRG42ConeSize;
	else
		skillCvar = g_skillcvars.skillReplicaM249ConeSize;

	Uint32 coneIndex = (Int32)GetSkillCVarValue(skillCvar);
	if(attenuateByFog)
		coneIndex = GetFogAttenuatedFiringCone(coneIndex);

	return coneIndex;
}

//=============================================
// @brief Sets task as failed
//
//=============================================
void CNPCCloneSoldier::SetTaskFailed( bool allowRetry )
{
	CPatrolNPC::SetTaskFailed(allowRetry);

	// Make sure the ignore flag is reset on both failure and completion
	const ai_task_t* pTask = GetTask();
	if(!m_takeAttackChance && pTask->task == AI_TASK_WAIT_FOR_MOVEMENT)
		m_takeAttackChance = true;
}

//=============================================
// @brief Sets task as completed
//
//=============================================
void CNPCCloneSoldier::SetTaskCompleted( void )
{
	CPatrolNPC::SetTaskCompleted();

	// Make sure the ignore flag is reset on both failure and completion
	const ai_task_t* pTask = GetTask();
	if(!m_takeAttackChance && pTask->task == AI_TASK_WAIT_FOR_MOVEMENT)
		m_takeAttackChance = true;
}

//=============================================
// @brief Starts a task
//
//=============================================
void CNPCCloneSoldier::StartTask( const ai_task_t* pTask )
{
	switch(pTask->task)
	{
	case AI_CLONE_SOLDIER_TASK_FACE_TOSS_DIR:
		{
			m_updateYaw = true;
		}
		break;
	case AI_CLONE_SOLDIER_TASK_SPEAK_SENTENCE:
		{
			SpeakSentence();
			SetTaskCompleted();
		}
		break;
	case AI_CLONE_SOLDIER_TASK_GET_AMBUSH_PATH:
		{
			// Set to this by default
			m_takeAttackChance = true;

			if(!m_enemy)
			{
				SetTaskFailed();
				return;
			}

			// Get center offset of enemy
			const Vector& enemyViewOffset = m_enemy->GetViewOffset(true);

			// Try and build the route
			bool result = BuildAmbushRoute(true, m_enemyLastKnownPosition, m_enemyLastKnownAngles, enemyViewOffset, NPC_MIN_ENEMY_DISTANCE, NPC_MAX_TACTICALPOS_DISTANCE);
			// If we failed, try again without looking for an obscured place
			if(!result)
				result = BuildAmbushRoute(true, m_enemyLastKnownPosition, m_enemyLastKnownAngles, enemyViewOffset, NPC_MIN_ENEMY_DISTANCE, NPC_MAX_TACTICALPOS_DISTANCE);

			if(!result)
			{
				if(m_capabilityBits & (AI_CAP_RANGE_ATTACK1|AI_CAP_RANGE_ATTACK2)
					&& !CheckConditions(AI_COND_CAN_RANGE_ATTACK1|AI_COND_CAN_RANGE_ATTACK2)
					&& GetLateralShootingPosition(m_enemyLastKnownPosition + enemyViewOffset))
				{
					Util::EntityConDPrintf(m_pEdict, "Found lateral shooting position.\n");
					SetTaskCompleted();
				}
				else if(m_capabilityBits & (AI_CAP_RANGE_ATTACK1|AI_CAP_RANGE_ATTACK2)
					&& !CheckConditions(AI_COND_CAN_RANGE_ATTACK1|AI_COND_CAN_RANGE_ATTACK2)
					&& GetClosestShootingPosition(m_enemyLastKnownPosition))
				{
					Util::EntityConDPrintf(m_pEdict, "Found closest shooting position.\n");
					SetTaskCompleted();
				}
				else if(BuildNearestVisibleRoute(m_enemyLastKnownPosition, enemyViewOffset, NPC_MIN_ENEMY_DISTANCE, m_firingDistance))
				{
					SetTaskCompleted();
				}
				else
				{
					// Mark enemy as not found
					if(!CheckConditions(AI_COND_SEE_ENEMY))
						SetConditions(AI_COND_ENEMY_NOT_FOUND);

					Util::EntityConDPrintf(m_pEdict, "AI_CLONE_SOLDIER_TASK_GET_AMBUSH_PATH failed.\n");
					SetTaskFailed(false);
				}
			}
			else
			{
				Util::EntityConDPrintf(m_pEdict, "Found an ambush shooting position.\n");

				StartDangerCheck(m_enemyLastKnownPosition, m_enemy, false);
				m_takeAttackChance = false;
				SetTaskCompleted();
			}
		}
		break;
	case AI_CLONE_SOLDIER_TASK_GET_TACTICAL_POSITION:
		{
			// Set to this by default
			m_takeAttackChance = true;

			if(!m_enemy)
			{
				SetTaskFailed();
				return;
			}

			// Get center offset of enemy
			const Vector& enemyViewOffset = m_enemy->GetViewOffset(true);

			// Try and find a tactical position
			bool isPartialCover = false;
			if(!BuildTacticalRoute(m_enemy, m_enemyLastKnownPosition, m_enemyLastKnownAngles, enemyViewOffset, NPC_MIN_ENEMY_DISTANCE, NPC_MAX_TACTICALPOS_DISTANCE, isPartialCover))
			{
				if(m_capabilityBits & (AI_CAP_RANGE_ATTACK1|AI_CAP_RANGE_ATTACK2)
					&& !CheckConditions(AI_COND_CAN_RANGE_ATTACK1|AI_COND_CAN_RANGE_ATTACK2)
					&& GetLateralShootingPosition(m_enemyLastKnownPosition + enemyViewOffset))
				{
					Util::EntityConDPrintf(m_pEdict, "Found lateral shooting position.\n");
					SetTaskCompleted();
				}
				else if(m_capabilityBits & (AI_CAP_RANGE_ATTACK1|AI_CAP_RANGE_ATTACK2)
					&& !CheckConditions(AI_COND_CAN_RANGE_ATTACK1|AI_COND_CAN_RANGE_ATTACK2)
					&& GetClosestShootingPosition(m_enemyLastKnownPosition))
				{
					Util::EntityConDPrintf(m_pEdict, "Found closest shooting position.\n");
					SetTaskCompleted();
				}
				else if(BuildNearestVisibleRoute(m_enemyLastKnownPosition, enemyViewOffset, NPC_MIN_ENEMY_DISTANCE, m_firingDistance))
				{
					// Try and build a nearby visible route
					SetTaskCompleted();
				}
				else
				{
					// Mark enemy as not found
					if(!CheckConditions(AI_COND_SEE_ENEMY))
						SetConditions(AI_COND_ENEMY_NOT_FOUND);

					Util::EntityConDPrintf(m_pEdict, "AI_CLONE_SOLDIER_TASK_GET_TACTICAL_POSITION failed.\n");
					SetTaskFailed(false);
				}
			}
			else
			{
				Util::EntityConDPrintf(m_pEdict, "Found a tactical shooting position.\n");

				StartDangerCheck(m_enemyLastKnownPosition, m_enemy, false);

				if(isPartialCover)
					m_takeAttackChance = false;

				SetTaskCompleted();
			}
		}
		break;
	case AI_CLONE_SOLDIER_TASK_GET_HIDING_POSITION:
		{
			// Get center offset of enemy
			const Vector& enemyViewOffset = m_enemy->GetViewOffset(true);

			if(BuildNearbyHidingPosition(m_enemy, m_enemyLastKnownPosition, enemyViewOffset, NPC_MIN_ENEMY_DISTANCE, NPC_MAX_TACTICALPOS_DISTANCE))
			{
				StartDangerCheck(m_enemyLastKnownPosition, m_enemy, false);
				SetTaskCompleted();
			}
			else
			{
				SetMemory(AI_MEMORY_HIDING_SPOT_NOT_FOUND);
				SetTaskFailed(false);
			}
		}
		break;
	default:
		CPatrolNPC::StartTask(pTask);
		break;
	}
}

//=============================================
// @brief Runs a task
//
//=============================================
void CNPCCloneSoldier::RunTask( const ai_task_t* pTask )
{
	switch(pTask->task)
	{
	case AI_CLONE_SOLDIER_TASK_FACE_TOSS_DIR:
		{
			SetIdealYaw(m_pState->origin + m_grenadeTossVelocity * 64);
			if(IsFacingIdealYaw())
				SetTaskCompleted();
		}
		break;
	default:
		{
			CPatrolNPC::RunTask(pTask);
		}
		break;
	}
}

//=============================================
// @brief Gibs the NPC
//
//=============================================
void CNPCCloneSoldier::GibNPC( void )
{
	if(GetBodyGroupValue(m_weaponsBodyGroupIndex) != m_weaponBlankSubmodelIndex)
	{
		Vector gunPosition;
		GetAttachment(NPC_WEAPON_ATTACHMENT_INDEX, gunPosition);

		if(m_weaponsBodyGroupIndex != NO_POSITION && m_weaponBlankSubmodelIndex != NO_POSITION)
			SetBodyGroup(m_weaponsBodyGroupIndex, m_weaponBlankSubmodelIndex);

		if(!m_dontDropWeapons)
		{
			CString weaponName;
			// This NPC's weapons got cut from the public release, so
			// weaponName remains empty

			if(!weaponName.empty())
			{
				Vector angles(m_pState->angles);
				angles[PITCH] = angles[ROLL] = 0;

				CBaseEntity* pEntity = DropItem(weaponName.c_str(), gunPosition, angles);
				if(pEntity)
				{
					Vector velocity;
					for(Uint32 i = 0; i < 2; i++)
						velocity[i] = Common::RandomFloat(-100, 100);
					velocity[2] = Common::RandomFloat(200, 300);
					
					Vector avelocity(0, Common::RandomFloat(200, 400), 0);

					pEntity->SetVelocity(velocity);
					pEntity->SetAngularVelocity(avelocity);
				}
			}
		}
	}

	CPatrolNPC::GibNPC();
}

//=============================================
// @brief Decapitates an NPC
//
//=============================================
void CNPCCloneSoldier::Decapitate( bool spawnHeadGib )
{
	DecapitateNPC(spawnHeadGib, m_headsBodyGroupIndex, m_headDecapitatedSubmodelIndex);
}

//=============================================
// @brief Returns the conditions to ignore
//
//=============================================
Uint64 CNPCCloneSoldier::GetIgnoreConditions( void )
{
	Uint64 ignoreConditions = CPatrolNPC::GetIgnoreConditions();
	if(CheckConditions(AI_COND_CAN_ATTACK) && !m_takeAttackChance)
		ignoreConditions |= AI_COND_CAN_ATTACK;

	return ignoreConditions;
}

//=============================================
// @brief Sets the NPC state
//
//=============================================
void CNPCCloneSoldier::SetNPCState( npcstate_t state )
{
	if(m_npcState == NPC_STATE_COMBAT && (state == NPC_STATE_IDLE || state == NPC_STATE_ALERT))
	{
		ClearMemory(AI_MEMORY_SOUGHT_TACTICAL);
		m_nextSweepTime = g_pGameVars->time + Common::RandomFloat(2, 3);
	}

	CPatrolNPC::SetNPCState(state);
}

//=============================================
// @brief Updates visibility and shooting distances
//
//=============================================
void CNPCCloneSoldier::UpdateDistances( void )
{
	Float fogEndDistance = CEnvFog::GetFogEndDistance();
	if(!fogEndDistance)
	{
		m_lookDistance = NPC_MAX_LOOK_DISTANCE;

		if(m_pState->weapons & NPC_WEAPON_SHOTGUN)
			m_firingDistance = NPC_MAX_SHOTGUNNER_FIRING_DISTANCE;
		else
			m_firingDistance = NPC_MAX_FIRING_DISTANCE;
	}
	else
	{
		m_lookDistance = fogEndDistance;

		Float fogEdgeDistance = fogEndDistance*0.9;
		if(m_pState->weapons & NPC_WEAPON_SHOTGUN)
		{
			if(fogEdgeDistance < NPC_MAX_SHOTGUNNER_FIRING_DISTANCE)
				m_firingDistance = fogEdgeDistance;
			else
				m_firingDistance = NPC_MAX_SHOTGUNNER_FIRING_DISTANCE;
		}
		else
		{
			m_firingDistance = fogEdgeDistance;
		}
	}

	// Precise distance is always the same
	m_preciseDistance = NPC_PRECISE_FIRING_DISTANCE;
}

//=============================================
// @brief Initializes squad related data
//
//=============================================
void CNPCCloneSoldier::InitSquad( void )
{
	CPatrolNPC::InitSquad();

	// Shotgunners are automatically offensive type
	if(IsInSquad() && (m_pState->weapons & NPC_WEAPON_SHOTGUN))
		m_attackType = NPC_TYPE_OFFENSIVE;
}

//=============================================
// @brief Speaks a sentence
//
//=============================================
void CNPCCloneSoldier::SpeakSentence( void )
{
	if(m_sentence == NPC_SENT_NONE || !CanSpeak())
		return;

	if(m_sentence < 0 || m_sentence >= NUM_NPC_SENTENCES)
	{
		Util::EntityConPrintf(m_pEdict, "Invalid sentence queued: %d.\n", m_sentence);
		return;
	}

	PlaySentence(NPC_SENTENCES[m_sentence], 0, VOL_NORM, ATTN_NORM, 0, true);
}

//=============================================
// @brief Builds a route to a tactical position
//
//=============================================
bool CNPCCloneSoldier::BuildTacticalRoute( CBaseEntity* pTargetEntity, const Vector& threatPosition, const Vector& threatAngles, const Vector& viewOffset, Float minDistance, Float maxDistance, bool& isPartialCover )
{
	if(!minDistance)
	{
		Util::EntityConDPrintf(m_pEdict, "No minimum distance set.\n");
		return false;
	}

	if(!maxDistance)
	{
		Util::EntityConDPrintf(m_pEdict, "No maximum distance set.\n");
		return false;
	}

	if(!gNodeGraph.IsNodeGraphValid())
	{
		Util::EntityConDPrintf(m_pEdict, "No valid node graph available.\n");
		return false;
	}

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

	Vector lookerPosition = threatPosition + viewOffset;
	Float distanceToEnemy = (lookerPosition - m_pState->origin).Length2D();

	Int32 lastBestNodeIndex = NO_POSITION;
	Float lastBestCoverage = 0;

	// Current coverage needs to be compared against best one, so we don't move for no reason
	// However keep in mind whether the enemy is totally occluded from our current position, in
	// which case current coverage would be invalid
	Vector myEyesPosition = m_pState->origin + m_pState->view_offset;
	Float currentCoverage = 0;
	if(Util::CheckTraceLine(lookerPosition, myEyesPosition))
		currentCoverage = CalculateCoverage(m_pState->origin, m_pState->view_offset, lookerPosition);

	Uint32 numNodes = gNodeGraph.GetNumNodes();
	for(Uint32 i = 0; i < numNodes; i++)
	{
		Int32 nodeIndex = (i + g_lastCoverSearchNodeIndex) % numNodes;
		
		// See if we can get there
		if(myNode != nodeIndex && gNodeGraph.GetNextNodeInRoute(myNode, nodeIndex, hullType, capIndex) == myNode)
			continue;

		const CAINodeGraph::node_t* pNode = gNodeGraph.GetNode(nodeIndex);
		if(!pNode)
			continue;

		// Don't take node if enemy is too close
		Float enemyDistanceToNode = (threatPosition - pNode->origin).Length();
		if(enemyDistanceToNode < minDistance || enemyDistanceToNode > maxDistance)
			continue;

		// Do not take nodes that are beyond the enemy's position
		Float myDistanceToNode = (m_pState->origin - pNode->origin).Length2D();
		if(myDistanceToNode > distanceToEnemy && enemyDistanceToNode < myDistanceToNode)
			continue;

		// If my distance is too big compared to the target's, don't take the node
		if(myDistanceToNode / enemyDistanceToNode > 2.5)
			continue;

		// Check if we can still hit the enemy from here
		Vector gunNodePosition = GetGunPosition(STANCE_STANDING) - m_pState->origin;
		gunNodePosition += pNode->origin;

		trace_t tr;
		Util::TraceLine(gunNodePosition, lookerPosition, true, false, m_pEdict, tr);
		if(!tr.noHit() || tr.startSolid())
			continue;

		if(IsSquadMemberInRange(pNode->origin, 64))
			continue;

		Float coverage = CalculateCoverage(pNode->origin, m_pState->view_offset, lookerPosition);
		if(coverage > lastBestCoverage || lastBestNodeIndex == NO_POSITION)
		{
			lastBestNodeIndex = nodeIndex;
			lastBestCoverage = coverage;
		}
	}

	if(lastBestNodeIndex != NO_POSITION)
	{
		g_lastCoverSearchNodeIndex = lastBestNodeIndex + 1;

		const CAINodeGraph::node_t* pNode = gNodeGraph.GetNode(lastBestNodeIndex);
		if(!pNode)
			return false;

		// Make sure it's a better coverage value than our current one
		if(lastBestCoverage > currentCoverage)
			isPartialCover = true;
		else
			isPartialCover = false;

		if(BuildRoute(pNode->origin, MF_TO_LOCATION, nullptr))
		{
			m_movementGoalPosition = pNode->origin;
			m_tacticalCoverage = lastBestCoverage;
			return true;
		}
	}

	return false;
}

//=============================================
// @brief Builds a route to an ambush position
//
//=============================================
bool CNPCCloneSoldier::BuildAmbushRoute( bool isObscured, const Vector& threatPosition, const Vector& threatAngles, const Vector& viewOffset, Float minDistance, Float maxDistance )
{
	if(!minDistance)
	{
		Util::EntityConDPrintf(m_pEdict, "No minimum distance set.\n");
		return false;
	}

	if(!maxDistance)
	{
		Util::EntityConDPrintf(m_pEdict, "No maximum distance set.\n");
		return false;
	}

	if(!gNodeGraph.IsNodeGraphValid())
	{
		Util::EntityConDPrintf(m_pEdict, "No valid node graph available.\n");
		return false;
	}

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

	Vector lookerPosition = threatPosition + viewOffset;
	Float distanceToEnemy = (lookerPosition - m_pState->origin).Length2D();

	Vector enemyForward;
	Math::AngleVectors(threatAngles, &enemyForward);
	enemyForward[ROLL] = 0;
	enemyForward.Normalize();

	Uint32 numNodes = gNodeGraph.GetNumNodes();
	for(Uint32 i = 0; i < numNodes; i++)
	{
		Int32 nodeIndex = (i + g_lastCoverSearchNodeIndex) % numNodes;
		g_lastCoverSearchNodeIndex = nodeIndex + 1;

		// See if we can get there
		if(myNode != nodeIndex && gNodeGraph.GetNextNodeInRoute(myNode, nodeIndex, hullType, capIndex) == myNode)
			continue;

		const CAINodeGraph::node_t* pNode = gNodeGraph.GetNode(nodeIndex);
		if(!pNode)
			continue;

		Float enemyDistanceToNode = (threatPosition - pNode->origin).Length();
		if(enemyDistanceToNode < minDistance || enemyDistanceToNode > maxDistance)
			continue;

		Float myDistanceToNode = (m_pState->origin - pNode->origin).Length2D();
		if(myDistanceToNode > distanceToEnemy && enemyDistanceToNode < myDistanceToNode)
			continue;

		// If my distance is too big compared to the target's, don't take the node
		if(myDistanceToNode / enemyDistanceToNode > 2.5)
			continue;

		if(isObscured)
		{
			Vector testPosition = pNode->origin + (GetCenter() - m_pState->origin) * 0.4;

			trace_t tr;
			Util::TraceLine(testPosition, lookerPosition, true, false, m_pEdict, tr);
			if(tr.noHit())
				continue;
		}

		Vector enemyDirection = (pNode->origin - threatPosition).Normalize();
		enemyDirection.Normalize();

		// See if this position is elusive enough
		if(Math::DotProduct(enemyDirection, enemyForward) > 0.25)
			continue;

		// Check if we can still hit the enemy from here
		Vector gunNodePosition = GetGunPosition(STANCE_STANDING) - m_pState->origin;
		gunNodePosition += pNode->origin;

		trace_t tr;
		Util::TraceLine(gunNodePosition, lookerPosition, true, false, m_pEdict, tr);
		if(!tr.noHit() || tr.startSolid())
			continue;

		if(BuildRoute(pNode->origin, MF_TO_LOCATION, nullptr))
		{
			m_movementGoalPosition = pNode->origin;
			return true;
		}
	}

	return false;
}

//=============================================
// @brief Finds a nearby cover position from the enemy's view
//
//=============================================
bool CNPCCloneSoldier::BuildNearbyHidingPosition( CBaseEntity* pTargetEntity, const Vector& threatPosition, const Vector& viewOffset, Float minDistance, Float maxDistance )
{
	if(!minDistance)
	{
		Util::EntityConDPrintf(m_pEdict, "No minimum distance set.\n");
		return false;
	}

	if(!maxDistance)
	{
		Util::EntityConDPrintf(m_pEdict, "No maximum distance set.\n");
		return false;
	}

	if(!gNodeGraph.IsNodeGraphValid())
	{
		Util::EntityConDPrintf(m_pEdict, "No valid node graph available.\n");
		return false;
	}

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

	Vector lookerPosition = threatPosition + viewOffset;
	Float distanceToEnemy = (lookerPosition - m_pState->origin).Length2D();

	Uint32 numNodes = gNodeGraph.GetNumNodes();
	for(Uint32 i = 0; i < numNodes; i++)
	{
		Int32 nodeIndex = (i + g_lastCoverSearchNodeIndex) % numNodes;
		g_lastCoverSearchNodeIndex = nodeIndex + 1;

		// See if we can get there
		if(myNode != nodeIndex && gNodeGraph.GetNextNodeInRoute(myNode, nodeIndex, hullType, capIndex) == myNode)
			continue;

		const CAINodeGraph::node_t* pNode = gNodeGraph.GetNode(nodeIndex);
		if(!pNode)
			continue;

		Float enemyDistanceToNode = (threatPosition - pNode->origin).Length();
		if(enemyDistanceToNode < minDistance || enemyDistanceToNode > maxDistance)
			continue;

		Float myDistanceToNode = (m_pState->origin - pNode->origin).Length2D();
		if(myDistanceToNode > distanceToEnemy && enemyDistanceToNode < myDistanceToNode)
			continue;

		// If my distance is too big compared to the target's, don't take the node
		if(myDistanceToNode / enemyDistanceToNode > 2.5)
			continue;

		Vector testPosition = pNode->origin + (GetCenter() - m_pState->origin) * 0.4;

		trace_t tr;
		Util::TraceLine(testPosition, lookerPosition, true, false, m_pEdict, tr);
		if(tr.noHit())
			continue;

		if(BuildRoute(pNode->origin, MF_TO_LOCATION, nullptr))
		{
			m_movementGoalPosition = pNode->origin;
			return true;
		}
	}

	return false;
}

//=============================================
// @brief Returns a schedule for a combat state
//
//=============================================
const CAISchedule* CNPCCloneSoldier::GetCombatSchedule( void )
{
	// Set to this by default
	m_takeAttackChance = true;

	if( m_attackType == NPC_TYPE_OFFENSIVE || CheckConditions(AI_COND_IN_DANGER) )
	{
		if(CheckConditions(AI_COND_CAN_RANGE_ATTACK1))
		{
			// Enemy is really close and can be attacked
			return GetScheduleByIndex( AI_SCHED_RANGE_ATTACK1 );
		}
		else if((m_enemy->GetNavigablePosition() - GetNavigablePosition()).Length() > NPC_MIN_AMBUSH_DISTANCE_DISTANCE)
		{
			// Enemy is not too near, so try and find an ambush spot
			if(CheckConditions(AI_COND_ENEMY_NAVIGABLE))
			{
				// Just ambush the player, preferably from behind
				m_takeAttackChance = false;
				return GetScheduleByIndex( AI_CLONE_SOLDIER_SCHED_AMBUSH_ENEMY );
			}
			else if(!HasMemory(AI_MEMORY_HIDING_SPOT_NOT_FOUND) && (m_enemy->GetNavigablePosition() - m_enemyLastKnownPosition).Length() > m_firingDistance)
			{
				// Find a cover spot and wait for the player
				return GetScheduleByIndex( AI_CLONE_SOLDIER_SCHED_HIDE_AND_WAIT );
			}
			else
			{
				// Wait for player to get into firing range
				return GetScheduleByIndex( AI_SCHED_STANDOFF );
			}
		}
		else if(CheckConditions(AI_COND_ENEMY_NAVIGABLE))
		{
			// Seek out a position where we can fire from with cover
			return GetScheduleByIndex( AI_CLONE_SOLDIER_SCHED_TAKE_TACTICAL_POSITION );
		}
		else
		{
			// Wait for enemy to appear
			return GetScheduleByIndex( AI_SCHED_STANDOFF );
		}
	}
	else if( m_attackType == NPC_TYPE_SUPPORT )
	{
		float enemyDist = (m_pState->origin - m_enemyLastKnownPosition).Length2D();
		if( CheckConditions(AI_COND_CAN_RANGE_ATTACK1) && (enemyDist < NPC_MIN_ENEMY_DISTANCE || HasMemory(AI_MEMORY_SOUGHT_TACTICAL)) )
		{
			// Enemy is really close and can be attacked, or
			// we've already sought a tactical position before
			return GetScheduleByIndex( AI_SCHED_RANGE_ATTACK1 );
		}
		else
		{
			CBaseEntity *pLeader = GetSquadLeader();

			if( pLeader && !CheckConditions(AI_COND_SEE_ENEMY) && (g_pGameVars->time - pLeader->GetLastEnemySightTime()) > Common::RandomFloat(7, 10) )
			{
				// If the enemy's been out of sight for a while, ambush
				m_takeAttackChance = false;
				return GetScheduleByIndex( AI_CLONE_SOLDIER_SCHED_AMBUSH_ENEMY );
			}
			else if(!HasMemory(AI_MEMORY_SOUGHT_TACTICAL))
			{
				// Remember that we tried to do this
				SetMemory(AI_MEMORY_SOUGHT_TACTICAL);
				m_takeAttackChance = false;

				// Try and find a path that gives us more coverage
				return GetScheduleByIndex( AI_CLONE_SOLDIER_SCHED_TAKE_TACTICAL_POSITION );
			}
			else if(CheckConditions(AI_COND_CAN_RANGE_ATTACK1))
			{
				// Enemy is really close and can be attacked, or
				// we've already sought a tactical position before
				return GetScheduleByIndex( AI_SCHED_RANGE_ATTACK1 );
			}
			else if(!HasMemory(AI_MEMORY_HIDING_SPOT_NOT_FOUND))
			{
				// Just seek out a shooting position
				return GetScheduleByIndex( AI_CLONE_SOLDIER_SCHED_HIDE_AND_WAIT );
			}
			else
			{
				// Wait for enemy to appear
				return GetScheduleByIndex( AI_SCHED_STANDOFF );
			}
		}
	}

	// Ambush the enemy
	return GetScheduleByIndex( AI_CLONE_SOLDIER_SCHED_AMBUSH_ENEMY );
}

//=============================================
// @brief Tells if it's okay to speak
//
//=============================================
bool CNPCCloneSoldier::CanSpeak( void ) const
{
	if(g_talkWaitTime > g_pGameVars->time)
		return false;

	if(m_talkTime > g_pGameVars->time)
		return false;

	if(HasSpawnFlag(FL_NPC_GAG) && m_npcState != NPC_STATE_COMBAT)
		return false;

	return true;
}

//=============================================
// @brief Called when a clone soldier just spoke
//
//=============================================
void CNPCCloneSoldier::Spoke( void )
{
	g_talkWaitTime = g_pGameVars->time + Common::RandomFloat(1.5, 2.0);
	m_sentence = NPC_SENT_NONE;
}

//=============================================
// @brief Returns the voice pitch
//
//=============================================
Uint32 CNPCCloneSoldier::GetVoicePitch( void )
{
	return m_voicePitch;
}

//=============================================
// @brief Tells how many grenades this NPC has
//
//=============================================
Uint32 CNPCCloneSoldier::GetNumGrenades( void )
{ 
	return m_numGrenades; 
}

//=============================================
// @brief Sets grenade count on NPC
//
//=============================================
void CNPCCloneSoldier::SetNumGrenades( Uint32 numGrenades ) 
{ 
	m_numGrenades = numGrenades; 
}

//=============================================
// @brief Resets talk
//
//=============================================
void CNPCCloneSoldier::Reset( void )
{
	g_talkWaitTime = 0;
	g_questionAsked = NPC_QUESTION_NONE;
}

//=============================================
// @brief Returns the reaction time
//
//=============================================
Float CNPCCloneSoldier::GetReactionTime( void ) 
{ 
	return GetSkillCVarValue(g_skillcvars.skillReplicaReactionTime);
}

//=============================================
// @brief Return bullet type used by NPC
//
//=============================================
bullet_types_t CNPCCloneSoldier::GetBulletType( void )
{
	bullet_types_t bulletType = BULLET_NONE;
	if(m_pState->weapons & NPC_WEAPON_SIG552)
		bulletType = BULLET_NPC_SIG552;
	else if(m_pState->weapons & NPC_WEAPON_SHOTGUN)
		bulletType = BULLET_NPC_SIG552;
	else if(m_pState->weapons & NPC_WEAPON_M249)
		bulletType = BULLET_NPC_M249;
	else if(m_pState->weapons & NPC_WEAPON_TRG42)
		bulletType = BULLET_NPC_TRG42;

	return bulletType;
}