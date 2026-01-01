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

// TODO: Get rid of hardcoded animations, use animation names
// TODO: Check that orientation checks are proper!

#include "includes.h"
#include "gd_includes.h"
#include "ai_turretnpc.h"
#include "envsprite.h"

// Array of turret sounds
const Char* CTurretNPC::TURRET_SOUNDS[NB_TURRET_SOUNDS] = 
{
	"turret/tu_fire1.wav",
	"turret/tu_ping.wav",
	"turret/tu_activate2.wav",
	"turret/tu_die.wav",
	"turret/tu_die2.wav",
	"turret/tu_die3.wav",
	"turret/tu_deploy.wav",
	"turret/tu_spinup.wav",
	"turret/tu_spindown.wav",
	"turret/tu_search.wav",
	"turret/tu_alert.wav"
};

// Turret glow sprite name
const Char CTurretNPC::TURRET_GLOW_SPRITE_NAME[] = "sprites/flare3.spr";
// Metal impact sound pattern
const Char CTurretNPC::TURRET_METAL_IMPACT_SOUND_PATTERN[] = "impact/metal_impact_bullet%d.wav";
// Number of Metal impact sounds
const Uint32 CTurretNPC::TURRET_NB_METAL_IMPACT_SOUNDS = 4;
// Metal impact particle script
const Char CTurretNPC::TURRET_METAL_IMPACT_PARTICLE_SCRIPT[] = "cluster_impact_metal.txt";
// Impact decal name
const Char CTurretNPC::TURRET_IMPACT_DECAL_NAME[] = "shot_metal";
// Maximum firing range of turret
const Float CTurretNPC::TURRET_MAX_RANGE = 1200;
// Default turn rate
const Float CTurretNPC::TURRET_DEFAULT_TURN_RATE = 30;
// Default max wait time
const Float CTurretNPC::TURRET_DEFAULT_MAX_WAIT_TIME = 15;

//=============================================
// @brief Constructor
//
//=============================================
CTurretNPC::CTurretNPC( edict_t* pedict ):
	CBaseNPC(pedict),
	m_maxSpinTime(0),
	m_spinState(SPIN_STATE_DOWN),
	m_pEyeGlowSprite(nullptr),
	m_eyeBrightness(0),
	m_deployHeight(0),
	m_retractHeight(0),
	m_minPitch(0),
	m_baseTurnRate(0),
	m_turnRate(0),
	m_orientation(ORIENTATION_FLOOR),
	m_isOn(false),
	m_isBerserk(false),
	m_shouldAutoStart(false),
	m_lastSightTime(0),
	m_maxWaitTime(0),
	m_searchSpeed(0),
	m_startYaw(0),
	m_pingTime(0),
	m_spinUpTime(0),
	m_damageTime(0)
{
}

//=============================================
// @brief Destructor
//
//=============================================
CTurretNPC::~CTurretNPC( void )
{
}

//=============================================
// @brief Declares save-restore fields
//
//=============================================
void CTurretNPC::DeclareSaveFields( void )
{
	CBaseNPC::DeclareSaveFields();

	DeclareSaveField(DEFINE_DATA_FIELD(CTurretNPC, m_maxSpinTime, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CTurretNPC, m_spinState, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CTurretNPC, m_pEyeGlowSprite, EFIELD_ENTPOINTER));
	DeclareSaveField(DEFINE_DATA_FIELD(CTurretNPC, m_eyeBrightness, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CTurretNPC, m_deployHeight, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CTurretNPC, m_retractHeight, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CTurretNPC, m_minPitch, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CTurretNPC, m_baseTurnRate, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CTurretNPC, m_turnRate, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CTurretNPC, m_orientation, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CTurretNPC, m_isOn, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CTurretNPC, m_isBerserk, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CTurretNPC, m_shouldAutoStart, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CTurretNPC, m_lastSightTime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CTurretNPC, m_searchSpeed, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CTurretNPC, m_startYaw, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CTurretNPC, m_pingTime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CTurretNPC, m_spinUpTime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CTurretNPC, m_lastSightPosition, EFIELD_COORD));
	DeclareSaveField(DEFINE_DATA_FIELD(CTurretNPC, m_currentAngles, EFIELD_VECTOR));
	DeclareSaveField(DEFINE_DATA_FIELD(CTurretNPC, m_goalAngles, EFIELD_VECTOR));
	DeclareSaveField(DEFINE_DATA_FIELD(CTurretNPC, m_maxWaitTime, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CTurretNPC, m_damageTime, EFIELD_TIME));
}

//=============================================
// @brief Spawns the entity
//
//=============================================
bool CTurretNPC::Spawn( void )
{
	m_pState->movetype = MOVETYPE_FLY;
	m_pState->solid = SOLID_SLIDEBOX;
	m_pState->takedamage = TAKEDAMAGE_YES;

	m_pState->flags |= FL_NPC;
	SetUse(&CTurretNPC::TurretUse);

	if(HasSpawnFlag(FL_TURRET_AUTO_ACTIVATE) && !HasSpawnFlag(FL_TURRET_START_INACTIVE))
		m_shouldAutoStart = true;

	ResetSequenceInfo();
	SetBoneController(0, 0);
	SetBoneController(1, 0);

	m_fieldOfView = VIEW_FIELD_FULL;
	return true;
}

//=============================================
// @brief Performs precache functions
//
//=============================================
void CTurretNPC::Precache( void )
{
	for(Uint32 i = 0; i < NB_TURRET_SOUNDS; i++)
		gd_engfuncs.pfnPrecacheSound(TURRET_SOUNDS[i]);

	Util::PrecacheFixedNbSounds(TURRET_METAL_IMPACT_SOUND_PATTERN, TURRET_NB_METAL_IMPACT_SOUNDS);

	CAnimatingEntity::Precache();
}

//=============================================
// @brief Manages a keyvalue
//
//=============================================
bool CTurretNPC::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "maxsleep"))
	{
		m_maxWaitTime = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "orientation"))
	{
		Int32 orientation = SDL_atoi(kv.value);
		switch(orientation)
		{
		case 1:
			m_orientation = ORIENTATION_CEILING;
			break;
		default:
		case 0:
			m_orientation = ORIENTATION_FLOOR;
			break;
		}

		return true;
	}
	else if(!qstrcmp(kv.keyname, "searchspeed"))
	{
		m_searchSpeed = SDL_atoi(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "turnrate"))
	{
		m_baseTurnRate = SDL_atoi(kv.value);
		return true;
	}
	else
		return CBaseNPC::KeyValue(kv);
}

//=============================================
// @brief Makes the entity take on damage
//
//=============================================
bool CTurretNPC::TakeDamage( CBaseEntity* pInflictor, CBaseEntity* pAttacker, Float amount, Int32 damageFlags )
{
	if(m_pState->takedamage == TAKEDAMAGE_NO)
		return false;

	Float _amount = amount;
	if(!m_isOn)
		_amount = 0;

	m_pState->health -= _amount;
	if(m_pState->health <= 0)
	{
		Die();
		return false;
	}
	else if(!m_isBerserk)
	{
		if(m_pState->health <= 10)
		{
			if(m_isOn && Common::RandomLong(0, 1) == 1)
			{
				SetThink(&CTurretNPC::SearchThink);
				m_isBerserk = true;
			}
		}

		return true;
	}

	return false;
}

//=============================================
// @brief Called when the turret dies
//
//=============================================
void CTurretNPC::Die( void )
{
	m_pState->health = 0;
	m_pState->takedamage = TAKEDAMAGE_NO;
	m_damageTime = g_pGameVars->time;

	m_pState->flags &= ~FL_NPC;

	SetUse(nullptr);
	SetThink(&CTurretNPC::TurretDeathThink);
	m_pState->nextthink = g_pGameVars->time + 0.1;

	UseTargets(this, USE_ON, 0);
}

//=============================================
// @brief Handles damage calculation for a hitscan
//
//=============================================
void CTurretNPC::TraceAttack( CBaseEntity* pAttacker, Float damage, const Vector& direction, trace_t& tr, Int32 damageFlags )
{
	Float _damage = damage;

	if(tr.hitgroup == HITGROUP_HELMET)
	{
		if(m_damageTime != g_pGameVars->time || Common::RandomLong(0, 10) < 1)
		{
			Util::Ricochet(tr.endpos, tr.plane.normal, true);
			m_damageTime = g_pGameVars->time;
		}

		// Reduce damage taken to minimum
		_damage = 0.1;
	}

	// Spawn particle effect
	Util::CreateParticles(TURRET_METAL_IMPACT_PARTICLE_SCRIPT, tr.endpos, tr.plane.normal, PART_SCRIPT_CLUSTER);
	Util::PlayRandomAmbientSound(tr.endpos, TURRET_METAL_IMPACT_SOUND_PATTERN, TURRET_NB_METAL_IMPACT_SOUNDS);
	Util::CreateVBMDecal(tr.endpos, tr.plane.normal, TURRET_IMPACT_DECAL_NAME, m_pEdict, FL_DECAL_NORMAL_PERMISSIVE);

	// Don't take damage if not set to
	if(m_pState->takedamage == TAKEDAMAGE_NO)
		return;

	gMultiDamage.AddDamage(this, _damage, damageFlags);
}

//=============================================
// @brief Returns the classification
//
//=============================================
Int32 CTurretNPC::GetClassification( void ) const
{
	return (m_isOn || m_shouldAutoStart) ? CLASS_MACHINE : CLASS_NONE;
}

//=============================================
// @brief Calls use function
//
//=============================================
void CTurretNPC::TurretUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	bool prevState = m_isOn;
	switch(useMode)
	{
	case USE_ON:
		m_isOn = true;
		break;
	case USE_OFF:
		m_isOn = false;
		break;
	case USE_TOGGLE:
		{
			if(m_isOn)
				m_isOn = false;
			else
				m_isOn = true;
		}
		break;
	}

	if(prevState == m_isOn)
		return;

	if(!m_isOn)
	{
		m_enemy.reset();
		m_shouldAutoStart = false;

		SetThink(&CTurretNPC::RetireThink);
		m_pState->nextthink = g_pGameVars->time + 0.1;
	}
	else
	{
		if(HasSpawnFlag(FL_TURRET_AUTO_ACTIVATE))
			m_shouldAutoStart = true;

		SetThink(&CTurretNPC::DeployThink);
		m_pState->nextthink = g_pGameVars->time + 0.1;
	}
}

//=============================================
// @brief Called when active
//
//=============================================
void CTurretNPC::ActiveThink( void )
{
	m_pState->nextthink = g_pGameVars->time + 0.1;
	FrameAdvance();

	if(!m_isOn || !m_enemy)
	{
		m_enemy.reset();
		m_lastSightTime = g_pGameVars->time + m_maxWaitTime;
		SetThink(&CTurretNPC::SearchThink);
		return;
	}
	else
	{
		SetThink(&CTurretNPC::ActiveThink);
	}

	if(!m_enemy->IsAlive())
	{
		if(m_lastSightTime && m_lastSightTime <= g_pGameVars->time)
		{
			m_goalAngles.x = 0;
			m_enemy.reset();
			m_lastSightTime = g_pGameVars->time + m_maxWaitTime;
			SetThink(&CTurretNPC::SearchThink);
			return;
		}
		else if(!m_lastSightTime)
		{
			m_lastSightTime = g_pGameVars->time + 0.5;// TODO make const
		}
	}

	Vector shootOrigin = m_pState->origin + m_pState->view_offset;
	Vector enemyShootTarget = m_enemy->GetBodyTarget(shootOrigin);

	bool isEnemyVisible = Util::IsBoxVisible(this, m_enemy, enemyShootTarget, 0);
	Vector directionToEnemy = (enemyShootTarget - shootOrigin);
	Float distanceToEnemy = directionToEnemy.Length();
	directionToEnemy.Normalize();

	if(!isEnemyVisible || distanceToEnemy > TURRET_MAX_RANGE)
	{
		if(m_lastSightTime && m_lastSightTime <= g_pGameVars->time)
		{
			m_goalAngles.x = 0;
			m_enemy.reset();
			m_lastSightTime = g_pGameVars->time + m_maxWaitTime;
			SetThink(&CTurretNPC::SearchThink);
			return;
		}
		else if(!m_lastSightTime)
		{
			m_lastSightTime = g_pGameVars->time + 0.5;// TODO make const
		}

		// Make sure this is set to false
		isEnemyVisible = false;
	}
	else
	{
		// Set last position we sighted the enemy at
		m_lastSightPosition = enemyShootTarget;
	}

	Vector forward;
	Math::AngleVectors(m_currentAngles, &forward);
	forward.z *= -1;

	bool canAttack = (Math::DotProduct(forward, directionToEnemy) <= 0.866) ? false : true;
	if(m_spinState == SPIN_STATE_UP && (canAttack || m_isBerserk))
	{
		Vector attachmentPosition;
		GetAttachment(0, attachmentPosition);
		SetAnimation(TURRET_ANIM_FIRE);
		TurretShoot(attachmentPosition, forward);
	}
	else
	{
		SetAnimation(TURRET_ANIM_SPIN);
	}

	if(m_isBerserk)
	{
		if(Common::RandomLong(0, 9) == 0)
		{
			m_goalAngles.y = Common::RandomFloat(0, 360);
			m_goalAngles.x = Common::RandomFloat(0, 90);
			if(m_orientation == ORIENTATION_CEILING)
				m_goalAngles.x -= 90;

			m_pState->health -= 1;
			if(m_pState->health <= 0)
			{
				Die();
				return;
			}
		}
	}
	else if(isEnemyVisible)
	{
		Vector anglesToEnemy = Math::VectorToAngles(directionToEnemy);
		if(anglesToEnemy.y > 360.0f)
			anglesToEnemy.y -= 360.0f;

		if(anglesToEnemy.y < 0)
			anglesToEnemy.y += 360.0f;

		if(anglesToEnemy.x < -180.0f)
			anglesToEnemy.x += 360.0f;

		if(anglesToEnemy.x > 180.0f)
			anglesToEnemy.x -= 360.0f;

		if(m_orientation == ORIENTATION_FLOOR)
		{
			if(anglesToEnemy.x > 90)
				anglesToEnemy.x = 90;
			else if(anglesToEnemy.x < m_minPitch)
				anglesToEnemy.x = m_minPitch;
		}
		else
		{
			if(anglesToEnemy.x < -90)
				anglesToEnemy.x = -90;
			else if(anglesToEnemy.x < -m_minPitch)
				anglesToEnemy.x = -m_minPitch;
		}

		m_goalAngles.x = anglesToEnemy.x;
		m_goalAngles.y = anglesToEnemy.y;
	}

	SpinUpCall();
	MoveTurret();
}

//=============================================
// @brief Called when searching for enemies
//
//=============================================
void CTurretNPC::SearchThink( void )
{
	SetAnimation(TURRET_ANIM_SPIN);
	FrameAdvance();

	m_pState->nextthink = g_pGameVars->time + 0.1;

	if(m_spinUpTime == 0 && m_maxSpinTime)
		m_spinUpTime = g_pGameVars->time + m_maxSpinTime;

	Ping();

	if(m_enemy && !m_enemy->IsAlive())
		m_enemy.reset();

	if(!m_enemy)
	{
		m_lookDistance = TURRET_MAX_RANGE;
		Look();

		m_enemy = GetBestVisibleEnemy();
	}

	if(m_enemy)
	{
		m_lastSightTime = 0;
		m_spinUpTime = 0;
		SetThink(&CTurretNPC::ActiveThink);
	}
	else
	{
		if(m_lastSightTime <= g_pGameVars->time)
		{
			m_lastSightTime = 0;
			m_spinUpTime = 0;
			SetThink(&CTurretNPC::RetireThink);
		}
		else if(m_spinUpTime && m_spinUpTime <= g_pGameVars->time)
		{
			SpinDownCall();
		}

		// Look for new victims
		m_goalAngles.y = (m_goalAngles.y + 0.1 * m_turnRate);
		if(m_goalAngles.y >= 360.0f)
			m_goalAngles.y -= 360.0f;

		MoveTurret();
	}
}

//=============================================
// @brief Called when auto searching for enemies
//
//=============================================
void CTurretNPC::AutoSearchThink( void )
{
	FrameAdvance();
	m_pState->nextthink = g_pGameVars->time + 0.3;

	if(m_enemy && !m_enemy->IsAlive())
		m_enemy.reset();

	if(!m_enemy)
	{
		m_lookDistance = TURRET_MAX_RANGE;
		Look();

		m_enemy = GetBestVisibleEnemy();
	}

	if(m_enemy)
	{
		Util::EmitEntitySound(this, TURRET_SOUNDS[TURRET_SND_ALERT], SND_CHAN_BODY);
		SetThink(&CTurretNPC::DeployThink);
	}
}

//=============================================
// @brief Called when turret dies
//
//=============================================
void CTurretNPC::TurretDeathThink( void )
{
	FrameAdvance();
	m_pState->nextthink = g_pGameVars->time + 0.1;

	if(m_pState->deadstate != DEADSTATE_DEAD)
	{
		// Set as dead
		m_pState->deadstate = DEADSTATE_DEAD;

		CString soundToPlay;
		switch(Common::RandomLong(0, 2))
		{
		case 1:
			soundToPlay = TURRET_SOUNDS[TURRET_SND_DIE2];
			break;
		case 2:
			soundToPlay = TURRET_SOUNDS[TURRET_SND_DIE3];
			break;
		default:
		case 0:
			soundToPlay = TURRET_SOUNDS[TURRET_SND_DIE1];
			break;
		}

		Util::EmitEntitySound(this, soundToPlay.c_str(), SND_CHAN_BODY);

		if(m_orientation == ORIENTATION_FLOOR)
			m_goalAngles.x = -15;
		else
			m_goalAngles.x = -90;

		SetAnimation(TURRET_ANIM_DIE);
		EyeOn();
	}
	else
	{
		EyeOff();
	}

	// Spawn smoke if we died recently
	if(m_damageTime + Common::RandomFloat(0, 2) > g_pGameVars->time)
		Util::CreateParticles("engine_explosion_smoke.txt", m_pState->origin, ZERO_VECTOR, PART_SCRIPT_SYSTEM);

	if(m_damageTime + Common::RandomFloat(0, 5) > g_pGameVars->time)
	{
		Vector sparkOrigin = Vector(Common::RandomFloat(m_pState->absmin.x, m_pState->absmax.x),
			Common::RandomFloat(m_pState->absmin.y, m_pState->absmax.y),
			0);

		if(m_orientation == ORIENTATION_FLOOR)
			sparkOrigin.z = Common::RandomFloat(m_pState->origin.z, m_pState->absmax.z);
		else
			sparkOrigin.z = Common::RandomFloat(m_pState->absmin.z, m_pState->origin.z);

		Util::CreateSparks(sparkOrigin);
	}

	if(m_isSequenceFinished && !MoveTurret() && m_damageTime + 5 < g_pGameVars->time)
	{
		m_pState->framerate = 0;
		SetThink(nullptr);
	}
}

//=============================================
// @brief Called when spinning down
//
//=============================================
void CTurretNPC::SpinDownCall( void )
{
	m_spinState = SPIN_STATE_DOWN;
}

//=============================================
// @brief Called when spinning up
//
//=============================================
void CTurretNPC::SpinUpCall( void )
{
	m_spinState = SPIN_STATE_UP;
}

//=============================================
// @brief Called when deploying
//
//=============================================
void CTurretNPC::DeployThink( void )
{
	FrameAdvance();
	m_pState->nextthink = g_pGameVars->time + 0.1;

	if(m_pState->sequence != TURRET_ANIM_DEPLOY)
	{
		m_isOn = true;
		SetAnimation(TURRET_ANIM_DEPLOY);
		Util::EmitEntitySound(this, TURRET_SOUNDS[TURRET_SND_DEPLOY], SND_CHAN_BODY);
		UseTargets(this, USE_ON, 0);
	}

	if(m_isSequenceFinished)
	{
		m_pState->maxs.z = m_deployHeight;
		m_pState->mins.z = -m_deployHeight;
		gd_engfuncs.pfnSetMinsMaxs(m_pEdict, m_pState->mins, m_pState->maxs);

		m_currentAngles.x = 0;
		if(m_orientation == ORIENTATION_CEILING)
			m_currentAngles.y = Math::AngleMod(m_pState->angles.y + 180.0f);
		else
			m_currentAngles.y = Math::AngleMod(m_pState->angles.y);

		SetAnimation(TURRET_ANIM_SPIN);
		m_pState->framerate = 0;

		SetThink(&CTurretNPC::SearchThink);
	}

	m_lastSightTime = g_pGameVars->time + m_maxWaitTime;
}

//=============================================
// @brief Called when retiring
//
//=============================================
void CTurretNPC::RetireThink( void )
{
	m_goalAngles.x = 0;
	m_goalAngles.y = m_startYaw;

	FrameAdvance();
	m_pState->nextthink = g_pGameVars->time + 0.1;

	EyeOff();

	if(!MoveTurret())
	{
		if(m_spinState == SPIN_STATE_UP)
		{
			SpinDownCall();
		}
		else if(m_pState->sequence != TURRET_ANIM_RETIRE)
		{
			SetAnimation(TURRET_ANIM_RETIRE);
			Util::EmitEntitySound(this, TURRET_SOUNDS[TURRET_SND_DEPLOY], SND_CHAN_BODY);
			UseTargets(this, USE_OFF, 0);
		}
		else if(m_isSequenceFinished)
		{
			m_isOn = false;
			m_lastSightTime = 0;

			SetAnimation(TURRET_ANIM_NONE);
			m_pState->maxs.z = m_retractHeight;
			m_pState->mins.z = -m_retractHeight;
			gd_engfuncs.pfnSetMinsMaxs(m_pEdict, m_pState->mins, m_pState->maxs);

			if(m_shouldAutoStart)
				SetThink(&CTurretNPC::AutoSearchThink);
			else
				SetThink(nullptr);
		}
	}
	else
	{
		SetAnimation(TURRET_ANIM_SPIN);
	}
}

//=============================================
// @brief Called when initializing
//
//=============================================
void CTurretNPC::InitializeThink( void )
{
	m_isOn = false;
	m_isBerserk = false;
	m_spinState = SPIN_STATE_DOWN;

	SetBoneController(0, 0);
	SetBoneController(0, 0);

	if(m_baseTurnRate <= 0)
		m_baseTurnRate = TURRET_DEFAULT_TURN_RATE;

	if(m_maxWaitTime <= 0)
		m_maxWaitTime = TURRET_DEFAULT_MAX_WAIT_TIME;

	m_startYaw = m_pState->angles.y;

	if(m_orientation == ORIENTATION_CEILING)
	{
		m_pState->idealpitch = 180.0f;
		m_pState->view_offset.z = -m_pState->view_offset.z;
		m_pState->effects |= EF_INVLIGHT;
		m_pState->angles.x = 180;
		m_pState->angles.y = m_pState->angles.y + 180.0f;
		if(m_pState->angles.y > 360.0f)
			m_pState->angles.y = m_pState->angles.y - 360.0f;
	}

	m_goalAngles.x = 0;

	if(m_shouldAutoStart)
	{
		m_lastSightTime = g_pGameVars->time + m_maxWaitTime;
		SetThink(&CTurretNPC::AutoSearchThink);
		m_pState->nextthink = g_pGameVars->time + 0.1;
	}
}

//=============================================
// @brief Called to play ping sound
//
//=============================================
void CTurretNPC::Ping( void )
{
	if(!m_pingTime)
	{
		m_pingTime = g_pGameVars->time + 1;// TODO make cosnt
	}
	else if(m_pingTime <= g_pGameVars->time)
	{
		m_pingTime = g_pGameVars->time + 1;// TODO make cosnt
		Util::EmitEntitySound(this, TURRET_SOUNDS[TURRET_SND_PING], SND_CHAN_ITEM);
		EyeOn();
	}
	else if(m_eyeBrightness > 0)
	{
		EyeOff();
	}
}

//=============================================
// @brief Called to turn eye on
//
//=============================================
void CTurretNPC::EyeOn( void )
{
	if(!m_pEyeGlowSprite)
		return;

	m_eyeBrightness = 255;
	m_pEyeGlowSprite->SetRenderAmount(m_eyeBrightness);
}

//=============================================
// @brief Called to turn eye off
//
//=============================================
void CTurretNPC::EyeOff( void )
{
	if(!m_pEyeGlowSprite)
		return;

	if(m_eyeBrightness > 0)
	{
		m_eyeBrightness -= 30;// TODO make const
		if(m_eyeBrightness < 0)
			m_eyeBrightness = 0;

		m_pEyeGlowSprite->SetRenderAmount(m_eyeBrightness);
	}
}

//=============================================
// @brief Sets the turret animation
//
//=============================================
void CTurretNPC::SetAnimation( turret_animations_t animation )
{
	if(m_pState->sequence == animation)
		return;

	switch(animation)
	{
	case TURRET_ANIM_FIRE:
	case TURRET_ANIM_SPIN:
		{
			if(m_pState->sequence != TURRET_ANIM_SPIN 
				&& m_pState->sequence != TURRET_ANIM_FIRE)
				m_pState->frame = 0;
		}
		break;
	default:
		m_pState->frame = 0;
		break;
	}

	m_pState->sequence = animation;
	ResetSequenceInfo();

	switch(animation)
	{
	case TURRET_ANIM_RETIRE:
		{
			m_pState->frame = GetNumFrames();
			m_pState->framerate = -1.0f;
		}
		break;
	case TURRET_ANIM_DIE:
		{
			m_pState->framerate = 1.0f;
		}
		break;
	}
}

//=============================================
// @brief Called when turret moves
//
//=============================================
bool CTurretNPC::MoveTurret( void )
{
	// Whether we moved at all
	bool hasMoved = false;

	// Move on X axis
	if(m_currentAngles.x != m_goalAngles.x)
	{
		Float direction = (m_goalAngles.x > m_currentAngles.x) ? 1 : -1;
		m_currentAngles.x += 0.1 * m_turnRate * direction;

		if(direction == 1)
		{
			if(m_currentAngles.x > m_goalAngles.x)
				m_currentAngles.x = m_goalAngles.x;
		}
		else
		{
			if(m_currentAngles.x < m_goalAngles.x)
				m_currentAngles.x = m_goalAngles.x;
		}

		SetBoneController(1, (m_orientation == ORIENTATION_FLOOR) ? -m_currentAngles.x : m_currentAngles.x);
		hasMoved = true;
	}
	
	// Move on y axis
	if(m_currentAngles.y != m_goalAngles.y)
	{
		Float direction = (m_goalAngles.y > m_currentAngles.y) ? 1 : -1;
		Float distance = SDL_fabs(m_goalAngles.y - m_currentAngles.y);

		if(distance > 180.0f)
		{
			distance = 360 - distance;
			direction = -direction;
		}

		if(distance > 30)
		{
			if(m_turnRate < m_baseTurnRate*10)
				m_turnRate += m_baseTurnRate;
		}
		else if(m_turnRate > 45)
			m_turnRate -= m_baseTurnRate;
		else
			m_turnRate += m_baseTurnRate;

		m_currentAngles.y += 0.1*m_turnRate*direction;

		if(m_currentAngles.y < 0)
			m_currentAngles.y += 360.0f;
		else if(m_currentAngles.y >= 360.0f)
			m_currentAngles.y -= 360.0f;

		if(m_orientation == ORIENTATION_FLOOR)
			SetBoneController(0, m_currentAngles.y - m_pState->angles.y);
		else
			SetBoneController(0, m_pState->angles.y - 180 - m_currentAngles.y);

		hasMoved = true;
	}

	if(!hasMoved)
		m_turnRate = m_baseTurnRate;

	return hasMoved;
}