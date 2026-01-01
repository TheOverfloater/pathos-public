/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "npcsentry.h"
#include "weapons_shared.h"

// Model used by this NPC
const Char CNPCSentry::NPC_MODEL_NAME[] = "models/sentry.mdl";
// Retracted height
const Float CNPCSentry::NPC_RETRACTED_HEIGHT = 64;
// Deployed height
const Float CNPCSentry::NPC_DEPLOYED_HEIGHT = 64;
// Minimum pitch value
const Float CNPCSentry::NPC_MIN_PITCH_VALUE = -60;
// View offset for npc
const Vector CNPCSentry::NPC_VIEW_OFFSET = Vector(0, 0, 48);
// X size of NPC
const Float CNPCSentry::NPC_X_SIZE = 16;
// Y size of NPC
const Float CNPCSentry::NPC_Y_SIZE = 16;

LINK_ENTITY_TO_CLASS( npc_sentry, CNPCSentry );

//=============================================
// @brief Constructor
//
//=============================================
CNPCSentry::CNPCSentry( edict_t* pedict ):
	CTurretNPC(pedict)
{
}

//=============================================
// @brief Destructor
//
//=============================================
CNPCSentry::~CNPCSentry( void )
{
}

//=============================================
// @brief Spawns the entity
//
//=============================================
bool CNPCSentry::Spawn( void )
{
	m_pFields->modelname = gd_engfuncs.pfnAllocString(NPC_MODEL_NAME);
	if(!CBaseNPC::Spawn())
		return false;

	m_pState->health = GetSkillCVarValue(g_skillcvars.skillSentryHealth);

	m_maxWaitTime = MAX_FLOAT_VALUE;
	m_maxSpinTime = MAX_FLOAT_VALUE;
	m_pState->view_offset = NPC_VIEW_OFFSET;

	if(!CTurretNPC::Spawn())
		return false;

	m_retractHeight = NPC_RETRACTED_HEIGHT;
	m_deployHeight = NPC_DEPLOYED_HEIGHT;
	m_minPitch = NPC_MIN_PITCH_VALUE;

	Vector mins(-NPC_X_SIZE, -NPC_Y_SIZE, -m_retractHeight);
	Vector maxs(NPC_X_SIZE, NPC_Y_SIZE, m_retractHeight);
	gd_engfuncs.pfnSetMinsMaxs(m_pEdict, mins, maxs);

	SetTouch(&CNPCSentry::SentryTouch);
	SetThink(&CTurretNPC::InitializeThink);
	m_pState->nextthink = g_pGameVars->time + 0.3;
	return true;
}

//=============================================
// @brief Called on initialization
//
//=============================================
void CNPCSentry::InitializeTurret( void )
{
	// Drop sentry to floor
	DropToFloor();
}

//=============================================
// @brief Makes the entity take on damage
//
//=============================================
bool CNPCSentry::TakeDamage( CBaseEntity* pInflictor, CBaseEntity* pAttacker, Float amount, Int32 damageFlags )
{
	m_pState->health -= amount;
	if(m_pState->health <= 0)
	{
		m_pState->health = 0;
		m_pState->takedamage = TAKEDAMAGE_NO;
		m_damageTime = g_pGameVars->time;
		m_pState->flags &= ~FL_NPC;

		SetThink(&CNPCSentry::SentryDeath);
		SetUse(nullptr);
		m_pState->nextthink = g_pGameVars->time + 0.1;
		return false;
	}
	else if(!m_isOn)
	{
		SetThink(&CTurretNPC::DeployThink);
		SetUse(nullptr);
		m_pState->nextthink = g_pGameVars->time + 0.1;
		return true;
	}
	else
	{
		return true;
	}
}

//=============================================
// @brief Returns the classification
//
//=============================================
Int32 CNPCSentry::GetClassification( void ) const
{
	return (m_isOn || m_shouldAutoStart) ? CLASS_HUMAN_FRIENDLY : CLASS_NONE;
}

//=============================================
// @brief Called when firing
//
//=============================================
void CNPCSentry::TurretShoot( const Vector& shootOrigin, const Vector& enemyDirection )
{
	Vector up, right;
	Math::GetUpRight(enemyDirection, up, right);

	Uint32 firingConeId = GetFiringCone(true);
	Vector firingCone = Weapon_GetConeSize(firingConeId);

	FireBullets(1, shootOrigin, enemyDirection, right, up, firingCone, NPC_DEFAULT_MAX_FIRING_DISTANCE, BULLET_NPC_SIG552, 4, 0, this);

	Util::EmitEntitySound(this, NPC_SIG552_FIRING_SOUND, SND_CHAN_WEAPON, VOL_NORM, ATTN_GUNFIRE);
	gAISounds.AddSound(AI_SOUND_COMBAT, m_pState->origin, NPC_GUN_SOUND_RADIUS, VOL_NORM, 0.3);

	m_pState->effects |= EF_MUZZLEFLASH;
}

//=============================================
// @brief Called when touching the sentry
//
//=============================================
void CNPCSentry::SentryTouch( CBaseEntity* pOther )
{
	if(m_pState->health > 0 && !m_isOn && (pOther && (pOther->IsPlayer() || pOther->IsNPC())))
	{
		SetThink(&CTurretNPC::DeployThink);
		SetUse(nullptr);
		SetTouch(nullptr);
		m_pState->nextthink = g_pGameVars->time + 0.1;
	}
}

//=============================================
// @brief Called when sentry dies
//
//=============================================
void CNPCSentry::SentryDeath( void )
{
	FrameAdvance();
	m_pState->nextthink = g_pGameVars->time + 0.1;

	if(m_pState->deadstate != DEADSTATE_DEAD)
	{
		switch(Common::RandomLong(0, 2))
		{
		case 0:
			Util::EmitEntitySound(this, "turret/tu_die.wav", SND_CHAN_BODY);
			break;
		case 1:
			Util::EmitEntitySound(this, "turret/tu_die2.wav", SND_CHAN_BODY);
			break;
		case 2:
			Util::EmitEntitySound(this, "turret/tu_die3.wav", SND_CHAN_BODY);
			break;
		}

		Util::EmitEntitySound(this, "turret/tu_active2.wav", SND_CHAN_STATIC, VOL_NORM, ATTN_NORM, PITCH_NORM, SND_FL_STOP);

		SetBoneController(0, 0);
		SetBoneController(1, 0);
		SetAnimation(TURRET_ANIM_DIE);

		m_pState->solid = SOLID_NOT;
		m_pState->angles.y = Math::AngleMod(m_pState->angles.y + Common::RandomFloat(0, 120));
		m_pState->deadstate = DEADSTATE_DEAD;
	}

	EyeOff();

	if(m_damageTime + Common::RandomFloat(0, 2) > g_pGameVars->time)
		Util::CreateParticles("turretsmoke.txt", m_pState->origin, Vector(0, 0, 1), PART_SCRIPT_SYSTEM, GetEdict(), 0, m_pState->entindex, NO_POSITION, PARTICLE_ATTACH_TO_ATTACHMENT);

	if(m_damageTime + Common::RandomFloat(0, 8) > g_pGameVars->time)
	{
		Vector src;
		GetAttachment(1, src);
		Util::CreateSparks(src);
	}

	if(m_isSequenceFinished && m_damageTime + 5 < g_pGameVars->time)
	{
		m_pState->framerate = 0;
		SetThink(nullptr);
	}
}

//=============================================
// @brief Return bullet type used by NPC
//
//=============================================
bullet_types_t CNPCSentry::GetBulletType( void )
{
	return BULLET_NPC_SIG552;
}