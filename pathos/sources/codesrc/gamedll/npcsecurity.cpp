/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "npcsecurity.h"
#include "ai_nodegraph.h"
#include "weapons_shared.h"
#include "ai_common.h"

// Glock clip size
const Uint32 CNPCSecurity::NPC_GLOCK_CLIP_SIZE = 17;
// Desert eagle clip size
const Uint32 CNPCSecurity::NPC_DEAGLE_CLIP_SIZE = 7;
// TRG42 clip size
const Uint32 CNPCSecurity::NPC_TRG42_CLIP_SIZE = 5;
// View offset for npc
const Vector CNPCSecurity::NPC_VIEW_OFFSET = Vector(0, 0, 50);
// Attachment for weapon
const Uint32 CNPCSecurity::NPC_WEAPON_ATTACHMENT_INDEX = 0;
// Yaw speed for npc
const Uint32 CNPCSecurity::NPC_YAW_SPEED = 180;
// Gun position offset
const Vector CNPCSecurity::NPC_GUN_POSITION_OFFSET = Vector(0, 0, 60);

// Model name for the npc
const Char CNPCSecurity::NPC_MODEL_NAME[] = "models/security.mdl";
// Pain sound pattern
const Char CNPCSecurity::NPC_PAIN_SOUND_PATTERN[] = "security/se_pain%d.wav";
// Number of pain sounds
const Uint32 CNPCSecurity::NPC_NB_PAIN_SOUNDS = 3;
// Death sound pattern
const Char CNPCSecurity::NPC_DEATH_SOUND_PATTERN[] = "security/se_die%d.wav";
// Number of death sounds
const Uint32 CNPCSecurity::NPC_NB_DEATH_SOUNDS = 3;
// Sentence prefix for npc
const Char CNPCSecurity::NPC_SENTENCE_PREFIX[] = "SE";

// Bodygroup name for heads
const Char CNPCSecurity::NPC_BODYGROUP_HEADS_NAME[] = "heads";
// Submodel name for head 1
const Char CNPCSecurity::NPC_SUBMODEL_HEAD1_NAME[] = "security_head1_reference";
// Submodel name for head 2
const Char CNPCSecurity::NPC_SUBMODEL_HEAD2_NAME[] = "security_head2_reference";

// Bodygroup name for guns
const Char CNPCSecurity::NPC_BODYGROUP_WEAPONS_NAME[] = "weapons";
// Submodel name for holstered glock
const Char CNPCSecurity::NPC_SUBMODEL_WEAPON_GLOCK_HOLSTERED_NAME[] = "glock_holster_reference";
// Submodel name for holstered desert eagle
const Char CNPCSecurity::NPC_SUBMODEL_WEAPON_DEAGLE_HOLSTERED_NAME[] = "blank";
// Submodel name for holstered glock
const Char CNPCSecurity::NPC_SUBMODEL_WEAPON_GLOCK_UNHOLSTERED_NAME[] = "glock_drawn_reference";
// Submodel name for holstered desert eagle
const Char CNPCSecurity::NPC_SUBMODEL_WEAPON_DEAGLE_UNHOLSTERED_NAME[] = "blank";
// Submodel name for TRG 42
const Char CNPCSecurity::NPC_SUBMODEL_WEAPON_TRG42_NAME[] = "blank";
// Submodel name for blank weapon
const Char CNPCSecurity::NPC_SUBMODEL_WEAPON_BLANK_NAME[] = "blank";

//==========================================================================
//
// SCHEDULES FOR CNPCSECURITY CLASS
//
//==========================================================================

//=============================================
// @brief Draw Weapon
//
//=============================================
ai_task_t taskListScheduleDrawWeapon[] = 
{
	AITASK(AI_TASK_STOP_MOVING,					0),
	AITASK(AI_TASK_FACE_ENEMY,					0),
	AITASK(AI_TASK_PLAY_SEQUENCE_FACE_ENEMY,	(Float)ACT_ARM),
};

const CAISchedule scheduleDrawWeapon(
	// Task list
	taskListScheduleDrawWeapon, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleDrawWeapon),
	// AI interrupt mask
	AI_COND_SCHEDULE_DONE,
	// Sound mask
	AI_SOUND_NONE, 
	// Name
	"Draw Weapon"
);

//==========================================================================
//
// SCHEDULES FOR CNPCSECURITY CLASS
//
//==========================================================================

LINK_ENTITY_TO_CLASS( npc_security, CNPCSecurity );

//=============================================
// @brief Constructor
//
//=============================================
CNPCSecurity::CNPCSecurity( edict_t* pedict ):
	CWanderNPC(pedict),
	m_isGunDrawn(false),
	m_isHostile(false),
	m_headSetting(0),
	m_nextPainTime(0),
	m_headBodyGroupIndex(NO_POSITION),
	m_head1SubmodelIndex(NO_POSITION),
	m_head2SubmodelIndex(NO_POSITION),
	m_weaponsBodyGroupIndex(NO_POSITION),
	m_weaponGlockHolsteredSubmodelIndex(NO_POSITION),
	m_weaponDEagleHolsteredSubmodelIndex(NO_POSITION),
	m_weaponGlockUnholsteredSubmodelIndex(NO_POSITION),
	m_weaponDEagleUnholsteredSubmodelIndex(NO_POSITION),
	m_weaponTRG42SubmodelIndex(NO_POSITION),
	m_weaponBlankSubmodelIndex(NO_POSITION)
{
}

//=============================================
// @brief Destructor
//
//=============================================
CNPCSecurity::~CNPCSecurity( void )
{
}

//=============================================
// @brief Declares save-restore fields
//
//=============================================
void CNPCSecurity::DeclareSaveFields( void )
{
	CWanderNPC::DeclareSaveFields();

	DeclareSaveField(DEFINE_DATA_FIELD(CNPCSecurity, m_isGunDrawn, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CNPCSecurity, m_isHostile, EFIELD_BOOLEAN));
}

//=============================================
// @brief Spawns the entity
//
//=============================================
bool CNPCSecurity::Spawn( void )
{
	// Set modelname
	m_pFields->modelname = gd_engfuncs.pfnAllocString(NPC_MODEL_NAME);

	if(!CWanderNPC::Spawn())
		return false;

	gd_engfuncs.pfnSetMinsMaxs(m_pEdict, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	m_pState->solid = SOLID_SLIDEBOX;
	m_pState->movetype = MOVETYPE_STEP;
	m_pState->health = GetSkillCVarValue(g_skillcvars.skillSecurityHealth);
	m_pState->view_offset = NPC_VIEW_OFFSET;

	m_bloodColor = BLOOD_RED;
	m_fieldOfView = VIEW_FIELD_WIDE;
	m_npcState = NPC_STATE_NONE;

	if(!m_pState->weapons)
		m_pState->weapons = NPC_WEAPON_GLOCK;

	Int32 weaponSubmodelToSet;
	if(m_pState->weapons & NPC_WEAPON_DEAGLE)
	{
		m_clipSize = NPC_DEAGLE_CLIP_SIZE;
		weaponSubmodelToSet = m_weaponGlockHolsteredSubmodelIndex;
		m_isGunDrawn = false;
	}
	else if(m_pState->weapons & NPC_WEAPON_TRG42)
	{
		m_clipSize = NPC_TRG42_CLIP_SIZE;
		weaponSubmodelToSet = m_weaponTRG42SubmodelIndex;
		m_isGunDrawn = true;
	}
	else
	{
		m_clipSize = NPC_GLOCK_CLIP_SIZE;
		weaponSubmodelToSet = m_weaponGlockHolsteredSubmodelIndex;
		m_isGunDrawn = false;
	}

	m_ammoLoaded = m_clipSize;

	if(m_weaponsBodyGroupIndex != NO_POSITION && weaponSubmodelToSet != NO_POSITION)
		SetBodyGroup(m_weaponsBodyGroupIndex, weaponSubmodelToSet);

	if(m_headBodyGroupIndex != NO_POSITION)
	{
		if(m_headSetting == 0)
			m_headSetting = Common::RandomLong(0, 1);

		Int32 headSubmodelToSet = NO_POSITION;
		switch(m_headSetting)
		{
		case 1:
			headSubmodelToSet = m_head2SubmodelIndex;
			break;
		case 0:
		default:
			headSubmodelToSet = m_head1SubmodelIndex;
			break;
		}

		SetBodyGroup(m_headBodyGroupIndex, headSubmodelToSet);
	}

	if(m_isHostile)
		SetMemory(AI_MEMORY_PROVOKED);

	// Set capabilities
	m_capabilityBits |= (AI_CAP_SQUAD|AI_CAP_HEAR|AI_CAP_TURN_HEAD|AI_CAP_TURN_HEAD_PITCH|AI_CAP_GROUP_DOORS|AI_CAP_ATTACK_BLEND_SEQ);

	// Initialize the NPC
	InitNPC();

	// Set use function
	SetUse(&CTalkNPC::FollowerUse);

	return true;
}

//=============================================
// @brief Performs precache functions
//
//=============================================
void CNPCSecurity::Precache( void )
{
	CWanderNPC::Precache();

	Util::PrecacheFixedNbSounds(NPC_PAIN_SOUND_PATTERN, NPC_NB_PAIN_SOUNDS);
	Util::PrecacheFixedNbSounds(NPC_DEATH_SOUND_PATTERN, NPC_NB_DEATH_SOUNDS);

	gd_engfuncs.pfnPrecacheSound(NPC_DEAGLE_FIRING_SOUND);
	gd_engfuncs.pfnPrecacheSound(NPC_GLOCK_FIRING_SOUND);
	gd_engfuncs.pfnPrecacheSound(NPC_GLOCK_RELOAD_SOUND);
	gd_engfuncs.pfnPrecacheSound(NPC_TRG42_FIRING_SOUND);

	// Precache and set up talking stuff
	InitTalkingNPC();
}

//=============================================
// @brief Initializes talknpc data
//
//=============================================
void CNPCSecurity::InitTalkingNPC( void )
{
	// Set sentence group names
	SetSentenceGroups(NPC_SENTENCE_PREFIX);

	// Call after setting groups
	CWanderNPC::InitTalkingNPC();
}

//=============================================
// @brief Manages a keyvalue
//
//=============================================
bool CNPCSecurity::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "hostile"))
	{
		m_isHostile = (SDL_atoi(kv.value) == 1) ? true : false;
		return true;
	}
	else if(!qstrcmp(kv.keyname, "hostile"))
	{
		m_headSetting = SDL_atoi(kv.value);
		return true;
	}
	else
		return CWanderNPC::KeyValue(kv);
}

//=============================================
// @brief Returns entity property flags
//
//=============================================
Int32 CNPCSecurity::GetEntityFlags( void )
{
	Int32 entityFlags = CWanderNPC::GetEntityFlags();
	if(!m_isHostile)
		entityFlags |= FL_ENTITY_PLAYER_USABLE;

	return entityFlags;
}

//=============================================
// @brief Sets extra model info after setting the model
//
//=============================================
void CNPCSecurity::PostModelSet( void )
{
	m_headBodyGroupIndex = GetBodyGroupIndexByName(NPC_BODYGROUP_HEADS_NAME);
	if(m_headBodyGroupIndex != NO_POSITION)
	{
		m_head1SubmodelIndex = GetSubmodelIndexByName(m_headBodyGroupIndex, NPC_SUBMODEL_HEAD1_NAME);
		m_head2SubmodelIndex = GetSubmodelIndexByName(m_headBodyGroupIndex, NPC_SUBMODEL_HEAD2_NAME);
	}

	m_weaponsBodyGroupIndex = GetBodyGroupIndexByName(NPC_BODYGROUP_WEAPONS_NAME);
	if(m_weaponsBodyGroupIndex != NO_POSITION)
	{
		m_weaponGlockHolsteredSubmodelIndex = GetSubmodelIndexByName(m_weaponsBodyGroupIndex, NPC_SUBMODEL_WEAPON_GLOCK_HOLSTERED_NAME);
		m_weaponDEagleHolsteredSubmodelIndex = GetSubmodelIndexByName(m_weaponsBodyGroupIndex, NPC_SUBMODEL_WEAPON_DEAGLE_HOLSTERED_NAME);
		m_weaponGlockUnholsteredSubmodelIndex = GetSubmodelIndexByName(m_weaponsBodyGroupIndex, NPC_SUBMODEL_WEAPON_GLOCK_UNHOLSTERED_NAME);
		m_weaponDEagleUnholsteredSubmodelIndex = GetSubmodelIndexByName(m_weaponsBodyGroupIndex, NPC_SUBMODEL_WEAPON_DEAGLE_UNHOLSTERED_NAME);
		m_weaponTRG42SubmodelIndex = GetSubmodelIndexByName(m_weaponsBodyGroupIndex, NPC_SUBMODEL_WEAPON_TRG42_NAME);
		m_weaponBlankSubmodelIndex = GetSubmodelIndexByName(m_weaponsBodyGroupIndex, NPC_SUBMODEL_WEAPON_BLANK_NAME);
	}
}

//=============================================
// @brief Returns the classification
//
//=============================================
Int32 CNPCSecurity::GetClassification( void ) const
{
	if(!m_isHostile)
		return CLASS_HUMAN_FRIENDLY;
	else
		return CLASS_HUMAN_HOSTILE;
}

//=============================================
// @brief Handles animation events
//
//=============================================
void CNPCSecurity::HandleAnimationEvent( const mstudioevent_t* pevent )
{
	switch(pevent->event)
	{
	case NPC_AE_DRAW:
		{
			if(m_weaponsBodyGroupIndex != NO_POSITION)
			{
				Int32 submodelToSet;
				if(m_pState->weapons & NPC_WEAPON_DEAGLE)
					submodelToSet = m_weaponDEagleUnholsteredSubmodelIndex;
				else
					submodelToSet = m_weaponGlockUnholsteredSubmodelIndex;

				if(submodelToSet != NO_POSITION)
					SetBodyGroup(m_weaponsBodyGroupIndex, submodelToSet);
			}

			m_isGunDrawn = true;
		}
		break;
	case NPC_AE_RELOAD:
		{
			Util::EmitEntitySound(this, NPC_GLOCK_RELOAD_SOUND, SND_CHAN_WEAPON);
			m_ammoLoaded = m_clipSize;
			ClearConditions(AI_COND_NO_AMMO_LOADED);
		}
		break;
	case NPC_AE_SHOOT:
		{
			const Char* pstrSoundPattern = nullptr;
			if(m_pState->weapons & NPC_WEAPON_DEAGLE)
				pstrSoundPattern = NPC_DEAGLE_FIRING_SOUND;
			else if(m_pState->weapons & NPC_WEAPON_TRG42)
				pstrSoundPattern = NPC_TRG42_FIRING_SOUND;
			else
				pstrSoundPattern = NPC_GLOCK_FIRING_SOUND;

			FireWeapon(1, pstrSoundPattern, 1, &m_ammoLoaded);
		}
		break;
	default:
		CWanderNPC::HandleAnimationEvent(pevent);
		break;
	}
}

//=============================================
// @brief Returns a sequence for an activity type
//
//=============================================
Int32 CNPCSecurity::FindActivity( Int32 activity )
{
	switch(activity)
	{
	case ACT_RANGE_ATTACK1:
		{
			if(m_pState->weapons & NPC_WEAPON_TRG42)
				return FindSequence("fire_trg42");
			else if(m_pState->weapons & NPC_WEAPON_DEAGLE)
				return FindSequence("fire_deagle");
			else
				return FindSequence("fire_glock");
		}
		break;
	default:
		return CWanderNPC::FindActivity(activity);
		break;
	}
}

//=============================================
// @brief Makes the entity take on damage
//
//=============================================
bool CNPCSecurity::TakeDamage( CBaseEntity* pInflictor, CBaseEntity* pAttacker, Float amount, Int32 damageFlags )
{
	// Do not take damage from allies
	if(pAttacker && pAttacker != this && (GetRelationship(pAttacker) == R_ALLY || GetRelationship(pAttacker) == R_NONE))
		return false;

	bool result = false;
	if(m_isHostile)
		result = CSquadNPC::TakeDamage(pInflictor, pAttacker, amount, damageFlags);
	else
		result = CWanderNPC::TakeDamage(pInflictor, pAttacker, amount, damageFlags);

	if(!IsAlive() || m_pState->deadstate == DEADSTATE_DYING || m_isHostile || !pAttacker || !pAttacker->IsPlayer())
		return result;

	if(!m_enemy)
	{
		if(HasMemory(AI_MEMORY_SUSPICIOUS) && IsFacing(pAttacker, m_pState->origin))
		{
			PlaySentence("SE_MAD", 0, VOL_NORM, ATTN_NORM, 0, true);
			SetMemory(AI_MEMORY_PROVOKED);
			StopFollowing(true);
		}
		else
		{
			PlaySentence("SE_SHOT", 0, VOL_NORM, ATTN_NORM, 0, true);
			SetMemory(AI_MEMORY_SUSPICIOUS);
		}
	}
	else if(!m_enemy->IsPlayer() && m_pState->deadstate == DEADSTATE_NONE)
	{
		PlaySentence("SE_SHOT", 0, VOL_NORM, ATTN_NORM, 0, true);
	}

	return result;
}

//=============================================
// @brief Manages getting killed
//
//=============================================
void CNPCSecurity::Killed( CBaseEntity* pAttacker, gibbing_t gibbing, deathmode_t deathMode )
{
	if(m_weaponsBodyGroupIndex != NO_POSITION && m_weaponBlankSubmodelIndex != NO_POSITION
		&& GetBodyGroupValue(m_weaponsBodyGroupIndex) != m_weaponBlankSubmodelIndex)
	{
		// Set blank bodygroup
		SetBodyGroup(m_weaponsBodyGroupIndex, m_weaponBlankSubmodelIndex);

		// Spawn weapon
		if(!m_dontDropWeapons)
		{
			weaponid_t weaponid = WEAPON_GLOCK;
			DropItem(weaponid, NPC_WEAPON_ATTACHMENT_INDEX, false);
		}
	}

	SetUse(nullptr);

	if(m_isHostile)
		CSquadNPC::Killed(pAttacker, gibbing, deathMode);
	else
		CWanderNPC::Killed(pAttacker, gibbing, deathMode);
}

//=============================================
// @brief Sets the ideal yaw speed
//
//=============================================
void CNPCSecurity::SetYawSpeed( void )
{
	m_pState->yawspeed = NPC_YAW_SPEED;
}

//=============================================
// @brief Returns the sound mask for the NPC
//
//=============================================
Uint64 CNPCSecurity::GetSoundMask( void )
{
	return (AI_SOUND_WORLD|AI_SOUND_COMBAT|AI_SOUND_DANGER|AI_SOUND_PLAYER);
}

//=============================================
// @brief Runs a task
//
//=============================================
void CNPCSecurity::RunTask( const ai_task_t* pTask )
{
	switch(pTask->task)
	{
	case AI_TASK_RANGE_ATTACK1:
		{
			if(m_enemy && m_enemy->IsPlayer() || m_isHostile)
				m_pState->framerate = 1.25f;

			CWanderNPC::RunTask(pTask);
		}
		break;
	default:
		CWanderNPC::RunTask(pTask);
		break;
	}
}

//=============================================
// @brief Returns the ideal schedule
//
//=============================================
const CAISchedule* CNPCSecurity::GetSchedule( void )
{
	// Run away from danger sounds
	if(CheckConditions(AI_COND_HEAR_SOUND) && m_pBestSound && (m_pBestSound->typeflags & AI_SOUND_DANGER))
		return GetScheduleByIndex(AI_SCHED_TAKE_COVER_FROM_BEST_SOUND);

	// Comment on dead enemies
	if(CheckConditions(AI_COND_ENEMY_DEAD) && CanSpeak())
		PlaySentence("SE_KILL", 0, VOL_NORM, ATTN_NORM, 0, true);

	// Dodge any dangerous enemies
	if(m_dangerousEnemy && CheckConditions(AI_COND_DANGEROUS_ENEMY_CLOSE) && !HasMemory(AI_MEMORY_DODGE_ENEMY_FAILED))
		return GetScheduleByIndex(AI_SCHED_DODGE_ENEMY);

	switch(m_npcState)
	{
	case NPC_STATE_COMBAT:
		{
			if(CheckConditions(AI_COND_ENEMY_DEAD))
			{
				// Let base class managed dead enemy
				return CWanderNPC::GetSchedule();
			}
			else if(CheckConditions(AI_COND_NO_AMMO_LOADED))
			{
				// Reload where you stand
				return GetScheduleByIndex(AI_SCHED_RELOAD);
			}
			else if(!m_isGunDrawn)
			{
				// Draw your gun
				return GetScheduleByIndex(AI_SCHED_ARM_WEAPON);
			}
			else if(CheckConditions(AI_COND_HEAVY_DAMAGE))
			{
				// Take cover from enemy is hurt too much
				return GetScheduleByIndex(AI_SCHED_TAKE_COVER_FROM_ENEMY);
			}
		}
		break;
	case NPC_STATE_ALERT:
	case NPC_STATE_IDLE:
		{
			if(!HasSpawnFlag(FL_NPC_IDLE) && !m_enemy && IsFollowing())
			{
				if(!m_targetEntity->IsAlive())
				{
					StopFollowing(false);
					break;
				}
				else if(!CheckConditions(AI_COND_BLOCKING_PATH))
				{
					// Face the target and follow him
					return GetScheduleByIndex(AI_TALKNPC_SCHED_TARGET_FACE);
				}
			}
		}
		break;
	}

	return CWanderNPC::GetSchedule();
}

//=============================================
// @brief Returns a schedule by it's index
//
//=============================================
const CAISchedule* CNPCSecurity::GetScheduleByIndex( Int32 scheduleIndex )
{
	switch(scheduleIndex)
	{
	case AI_SCHED_ARM_WEAPON:
		{
			if(m_enemy)
				return &scheduleDrawWeapon;
			else if(m_npcState == NPC_STATE_COMBAT)
				return &scheduleCombatFail;
			else
				return &scheduleFail;
		}
		break;
	case AI_TALKNPC_SCHED_TARGET_CHASE:
		{
			return &scheduleFollow;
		}
		break;
	}

	return CWanderNPC::GetScheduleByIndex(scheduleIndex);
}

//=============================================
// @brief Checks if we can do range attack 1
//
//=============================================
bool CNPCSecurity::CheckRangeAttack1( Float dp, Float distance )
{
	if(CheckConditions(AI_COND_ENEMY_OCCLUDED))
		return false;

	if(dp < NPC_FIRING_ANGLE_TRESHOLD || CheckConditions(AI_COND_FRIENDLY_FIRE) || distance > m_firingDistance)
		return false;

	return CheckConditions(AI_COND_SHOOT_VECTOR_VALID);
}

//=============================================
// @brief Checks the ammo capacity
//
//=============================================
void CNPCSecurity::CheckAmmo( void )
{
	if(!m_ammoLoaded)
		SetConditions(AI_COND_NO_AMMO_LOADED);
}

//=============================================
// @brief Returns the gun position
//
//=============================================
Vector CNPCSecurity::GetGunPosition( stance_t stance )
{
	return m_pState->origin + Vector(0, 0, 60);
}

//=============================================
// @brief Plays pain sounds
//
//=============================================
void CNPCSecurity::EmitPainSound( void )
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
void CNPCSecurity::EmitDeathSound( void )
{
	Util::PlayRandomEntitySound(this, NPC_DEATH_SOUND_PATTERN, NPC_NB_DEATH_SOUNDS, SND_CHAN_VOICE, VOL_NORM, ATTN_NORM, GetVoicePitch());
}

//=============================================
// @brief Plays alert sounds
//
//=============================================
void CNPCSecurity::EmitAlertSound( void )
{
	if(!m_enemy || !CanSpeak())
		return;

	PlaySentence("SE_ATTACK", 0, VOL_NORM, ATTN_IDLE, 0, true);
}

//=============================================
// @brief Returns the firing cone used
//
//=============================================
const Uint32 CNPCSecurity::GetFiringCone( bool attenuateByFog )
{
	Int32 skillCvar;
	if(m_pState->weapons & NPC_WEAPON_DEAGLE)
		skillCvar = g_skillcvars.skillSecurityDeagleConeSize;
	else if(m_pState->weapons & NPC_WEAPON_GLOCK)
		skillCvar = g_skillcvars.skillSecurityGlockConeSize;
	else
		skillCvar = g_skillcvars.skillSecurityTRG42ConeSize;

	Uint32 coneIndex = (Int32)GetSkillCVarValue(skillCvar);
	if(attenuateByFog)
		coneIndex = GetFogAttenuatedFiringCone(coneIndex);

	return coneIndex;
}

//=============================================
// @brief Return bullet type used by NPC
//
//=============================================
bullet_types_t CNPCSecurity::GetBulletType( void )
{
	bullet_types_t bulletType = BULLET_NPC_9MM;
	return bulletType;
}