/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "weaponhandgrenade.h"
#include "player.h"
#include "buttonbits.h"
#include "game.h"
#include "grenade.h"

// Sequence names for weapon
const Char* CWeaponHandGrenade::m_sequenceNames[CWeaponHandGrenade::NUM_WEAPON_ANIMATIONS] = 
{
	"idle1",
	"idle2",
	"pinpull",
	"pinpull_toss",
	"throw1",
	"throw2",
	"throw3",
	"toss",
	"deploy",
	"holster"
};

// Weapon view model
const Char CWeaponHandGrenade::WEAPON_VIEWMODEL[] = "models/v_grenade.mdl";
// Weapon weight
const Int32 CWeaponHandGrenade::WEAPON_WEIGHT = 80;
// Weapon slot
const Int32 CWeaponHandGrenade::WEAPON_SLOT = 4;
// Weapon slot position
const Int32 CWeaponHandGrenade::WEAPON_SLOT_POSITION = 0;
// Default ammo for weapon
const Uint32 CWeaponHandGrenade::WEAPON_DEFAULT_GIVE = 1;
// Grenade explosion delay
const Float CWeaponHandGrenade::GRENADE_EXPLODE_DELAY = 3.0;
// Grenade cooking cut-off time
const Float CWeaponHandGrenade::GRENADE_CUTOFF_TIME = 2.5;

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(weapon_handgrenade, CWeaponHandGrenade);

//=============================================
// @brief
//
//=============================================
CWeaponHandGrenade::CWeaponHandGrenade( edict_t* pedict ):
	CPlayerWeapon(pedict),
	m_throwStartTime(0),
	m_throwReleaseTime(0),
	m_isTossing(false)
{
}

//=============================================
// @brief
//
//=============================================
CWeaponHandGrenade::~CWeaponHandGrenade( void )
{
}

//=============================================
// @brief
//
//=============================================
void CWeaponHandGrenade::SetSpawnProperties( void )
{
	SetBodyGroup(WMODEL_BODY_BASE, WMODEL_GRENADE);

	m_defaultAmmo = WEAPON_DEFAULT_GIVE;
	m_weaponId = WEAPON_HANDGRENADE;
}

//=============================================
// @brief
//
//=============================================
void CWeaponHandGrenade::Precache( void )
{
	CPlayerWeapon::Precache();

	gd_engfuncs.pfnPrecacheModel(WEAPON_VIEWMODEL);
	gd_engfuncs.pfnPrecacheModel(BULLET_CASINGS_MODEL);

	gd_engfuncs.pfnPrecacheSound("weapons/gren_pinpull.wav");
	gd_engfuncs.pfnPrecacheSound("weapons/gren_throw.wav");
}

//=============================================
// @brief
//
//=============================================
bool CWeaponHandGrenade::GetWeaponInfo( weaponinfo_t* pWeapon )
{
	pWeapon->name = gd_engfuncs.pfnGetString(m_pFields->classname);
	pWeapon->ammo = AMMOTYPE_MK2GRENADE_NAME;
	pWeapon->maxammo = MAX_MK2_GRENADES;
	pWeapon->maxclip = WEAPON_NO_CLIP;
	pWeapon->slot = WEAPON_SLOT;
	pWeapon->position = WEAPON_SLOT_POSITION;
	pWeapon->id = WEAPON_HANDGRENADE;
	pWeapon->weight = WEAPON_WEIGHT;
	pWeapon->flags = (FL_WEAPON_LIMIT_IN_WORLD|FL_WEAPON_EXHAUSTIBLE|FL_WEAPON_NO_AUTO_AIM);
	return true;
}

//=============================================
// @brief
//
//=============================================
bool CWeaponHandGrenade::Deploy( void )
{
	// Reset these
	m_throwStartTime = 0;
	m_throwReleaseTime = 0;

	// Call to play animation
	return DefaultDeploy(WEAPON_VIEWMODEL, m_sequenceNames[HANDGRENADE_DRAW]);
}

//=============================================
// @brief
//
//=============================================
bool CWeaponHandGrenade::CanHolster( void )
{
	return (!m_throwStartTime && !m_throwReleaseTime) ? true : false;
}

//=============================================
// @brief
//
//=============================================
void CWeaponHandGrenade::Holster( void )
{
	Int32 ammocount = m_pPlayer->GetAmmoCount(m_ammoType);
	if(ammocount > 0)
	{
		const Char* pstrSequenceName = m_sequenceNames[HANDGRENADE_HOLSTER];
		SetWeaponAnimation(pstrSequenceName);
		m_nextThinkTime = g_pGameVars->time + GetSequenceTime(pstrSequenceName);
		m_nextAttackTime = m_nextThinkTime;
	}
	else
	{
		m_pPlayer->RemoveWeaponBit((1<<WEAPON_HANDGRENADE));
		SetThink(&CPlayerWeapon::DestroyWeapon);
		m_pState->nextthink = g_pGameVars->time + 0.1;
	}

	m_throwStartTime = 0;
	m_throwReleaseTime = 0;

	CPlayerWeapon::Holster();
}

//=============================================
// @brief
//
//=============================================
void CWeaponHandGrenade::PrimaryAttack( void )
{
	if(m_pPlayer->GetAmmoCount(m_ammoType) <= 0)
		return;

	// Regular throw
	Begin(false);
}

//=============================================
// @brief
//
//=============================================
void CWeaponHandGrenade::SecondaryAttack( void )
{
	if(m_pPlayer->GetAmmoCount(m_ammoType) <= 0)
		return;

	// Toss
	Begin(true);
}

//=============================================
// @brief
//
//=============================================
void CWeaponHandGrenade::Begin( bool istossing )
{
	Int32 animationIndex = istossing ? HANDGRENADE_PINPULL_TOSS : HANDGRENADE_PINPULL;
	const Char* pstrSequenceName = m_sequenceNames[animationIndex];

	m_throwStartTime = g_pGameVars->time + GetSequenceTime(pstrSequenceName);
	m_nextThinkTime = m_nextAttackTime = m_nextIdleTime = m_throwStartTime;
	m_throwReleaseTime = 0;
	
	// Set animation
	SetWeaponAnimation(pstrSequenceName);

	// Not tossing it
	m_isTossing = istossing;
}

//=============================================
// @brief
//
//=============================================
void CWeaponHandGrenade::PostThink( void )
{
	if(m_throwStartTime)
	{
		if(m_throwStartTime <= g_pGameVars->time)
		{
			// Get time spent cooking the grenade
			Float timeafter = m_throwStartTime - g_pGameVars->time + GRENADE_EXPLODE_DELAY;
			if(timeafter < 0)
				timeafter = 0;
			
			// Do not allow more than 0.5 seconds left
			if(m_pPlayer->GetButtonBits() & (IN_ATTACK|IN_ATTACK2) && timeafter > (GRENADE_EXPLODE_DELAY-GRENADE_CUTOFF_TIME))
				return;

			// Get player's view angles
			Vector throwangles = m_pPlayer->GetViewAngles();

			Vector forward, right, up;
			Math::AngleVectors(throwangles, &forward, &right, &up);

			// Place where we want to toss it
			Vector gunpos = m_pPlayer->GetGunPosition();
			Vector endpos = gunpos + forward * 16384;
			
			trace_t tr;
			Util::TraceLine(gunpos, endpos, false, false, m_pPlayer->GetEdict(), tr);
			
			// Update with one from traceline
			if(!tr.startSolid() && !tr.allSolid())
				endpos = tr.endpos;

			// Calculate speed and strength
			Float throwstrength = 4 + (g_pGameVars->time - m_throwStartTime)/GRENADE_EXPLODE_DELAY * 3;
			Float throwspeed = (90 - throwangles.x)*throwstrength;
			if(throwspeed > 1200)
				throwspeed = 1200;

			Vector origin;
			Vector velocity;
			Int32 animationIndex = 0;

			if(!m_isTossing)
			{
				// Apply axis punch
				m_pPlayer->ApplyAxisPunch( 0, Common::RandomFloat( 50, 70 ) );
				m_pPlayer->ApplyAxisPunch( 1, Common::RandomFloat( 10, 30 ) );

				if(throwangles.x < 0)
					throwangles.x = -10 + throwangles.x * ((90-10)/90);
				else
					throwangles.x = -10 + throwangles.x * ((90+10)/90);

				// Calculate toss origin and direction
				origin = gunpos + forward*4 + right*6 + up*8;
				Vector tossdir = (endpos-origin).Normalize();
				tossdir = (tossdir+up*0.3).Normalize();
				velocity = tossdir * throwspeed + m_pPlayer->GetVelocity();

				if(throwspeed < 500)
					animationIndex = HANDGRENADE_THROW1;
				else if(throwspeed < 1000)
					animationIndex = HANDGRENADE_THROW2;
				else
					animationIndex = HANDGRENADE_THROW3;

				Util::EmitEntitySound(m_pPlayer, "weapons/gren_throw.wav", SND_CHAN_WEAPON, VOL_NORM, ATTN_NORM, PITCH_NORM + Common::RandomLong(-5, +10));
			}
			else
			{
				// Tossing sends it on a more straight path
				origin = gunpos + forward*4 + right*4 - up*8;
				Vector tossdir = (endpos-origin).Normalize();
				tossdir = (tossdir+up*0.3).Normalize();
				velocity = tossdir * throwspeed + m_pPlayer->GetVelocity();

				animationIndex = HANDGRENADE_TOSS;
			}

			// Set animation
			SetWeaponAnimation(m_sequenceNames[animationIndex]);

			// Remove from ammo
			m_pPlayer->RemoveAmmo(m_ammoType, 1);

			CGrenade::CreateTimed(m_pPlayer,
				origin,
				velocity,
				timeafter,
				gSkillData.GetSkillCVarSetting(g_skillcvars.skillPlayerHandGrenadeRadius),
				gSkillData.GetSkillCVarSetting(g_skillcvars.skillPlayerDmgHandGrenade));

			// Spawn the pin
			SpawnPin(origin, m_pPlayer->GetViewAngles(), velocity);

			// If we have more, set the recovery time
			Double nextTime = g_pGameVars->time + GetSequenceTime(m_sequenceNames[animationIndex]);
			// IMPORTANT - Set next think time
			m_nextIdleTime = m_nextThinkTime = nextTime;

			if(m_pPlayer->GetAmmoCount(m_ammoType) > 0)
				m_throwReleaseTime = nextTime;

			// Reset this
			m_throwStartTime = 0;
		}

		return;
	}

	if(m_throwReleaseTime)
	{
		if(m_throwReleaseTime <= g_pGameVars->time)
		{
			SetWeaponAnimation(m_sequenceNames[HANDGRENADE_DRAW]);
			m_nextAttackTime = m_nextIdleTime = m_nextThinkTime = g_pGameVars->time + GetSequenceTime(m_sequenceNames[HANDGRENADE_DRAW]);
			m_throwReleaseTime = 0;
		}

		return;
	}

	CPlayerWeapon::PostThink();
}

//=============================================
// @brief
//
//=============================================
void CWeaponHandGrenade::Idle( void )
{
	if(m_nextIdleTime > g_pGameVars->time)
		return;

	if(!m_pPlayer->GetAmmoCount(m_ammoType))
		return;

	Uint32 sequenceIndex = 0;
	Float flRand = Common::RandomFloat(0.0, 1.0);

	if(flRand <= 0.75)
		sequenceIndex = HANDGRENADE_IDLE;
	else
		sequenceIndex = HANDGRENADE_FIDGET;

	Float sequenceTime = GetSequenceTime(m_sequenceNames[sequenceIndex]);
	m_nextIdleTime = g_pGameVars->time + sequenceTime*Common::RandomLong(1, 3);

	SetWeaponAnimation( m_sequenceNames[sequenceIndex] );
}

//=============================================
// @brief
//
//=============================================
void CWeaponHandGrenade::SpawnPin( const Vector& origin, const Vector& angles, const Vector& velocity )
{
	Int64 bodyValue = Util::GetBodyValueForSubmodel(SHELLCASINGS_MODEL_FILENAME, "spoon_reference");
	if(bodyValue == NO_POSITION)
		return;

	Util::CreateTempModel(origin, angles, velocity, 5, 1, BULLET_CASINGS_MODEL, TE_BOUNCE_NONE, 300, 0.4, TE_FL_NONE, bodyValue);
}