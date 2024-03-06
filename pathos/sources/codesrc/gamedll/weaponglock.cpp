/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "weaponglock.h"
#include "player.h"
#include "buttonbits.h"
#include "game.h"

// Sequence names for weapon
const Char* CWeaponGlock::m_sequenceNames[CWeaponGlock::NUM_WEAPON_ANIMATIONS] = 
{
	"idle1",
	"idle1_empty",
	"idle2",
	"idle2_empty",
	"shoot",
	"shoot_empty",
	"shoot_sil",
	"shoot_empty_sil",
	"reload_empty",
	"reload",
	"deploy",
	"deploy_empty",
	"deploy_first",
	"holster",
	"holster_empty",
	"flash_toggle",
	"flash_toggle_empty",
	"add_flashlight",
	"add_flashlight_empty",
	"add_silencer",
	"add_silencer_empty",
	"remove_silencer",
	"remove_silencer_empty"
};

// Weapon view model
const Char CWeaponGlock::WEAPON_VIEWMODEL[] = "models/v_glock.mdl";
// Weapon weight
const Int32 CWeaponGlock::WEAPON_WEIGHT = 20;
// Weapon slot
const Int32 CWeaponGlock::WEAPON_SLOT = 1;
// Weapon slot position
const Int32 CWeaponGlock::WEAPON_SLOT_POSITION = 0;
// Default ammo for weapon
const Uint32 CWeaponGlock::WEAPON_DEFAULT_GIVE = 12;
// Max clip capacity for weapon
const Uint32 CWeaponGlock::WEAPON_MAX_CLIP = 17;
// Weapon cone id
const Uint32 CWeaponGlock::WEAPON_CONE_ID = 1;

// w_ model groups
const Uint32 CWeaponGlock::WGLOCK_SL_GROUP_INDEX = 1;
const Uint32 CWeaponGlock::WGLOCK_SL_OFF = 0;
const Uint32 CWeaponGlock::WGLOCK_SL_ON = 1;

const Uint32 CWeaponGlock::WGLOCK_FL_GROUP_INDEX = 2;
const Uint32 CWeaponGlock::WGLOCK_FL_OFF = 0;
const Uint32 CWeaponGlock::WGLOCK_FL_ON = 1;

// v_ model groups
const Uint32 CWeaponGlock::VGLOCK_SL_GROUP_INDEX = 2;
const Uint32 CWeaponGlock::VGLOCK_SL_OFF = 0;
const Uint32 CWeaponGlock::VGLOCK_SL_ON = 1;

const Uint32 CWeaponGlock::VGLOCK_FL_GROUP_INDEX = 1;
const Uint32 CWeaponGlock::VGLOCK_FL_OFF = 0;
const Uint32 CWeaponGlock::VGLOCK_FL_ON = 1;

// Recoil degrade speed
const Float CWeaponGlock::WEAPON_RECOIL_DEGRADE = 2;

// Silencer item name
const Char CWeaponGlock::SILENCER_ENTITY_NAME[] = "item_glock_silencer";
// Flashlight item name
const Char CWeaponGlock::FLASHLIGHT_ENTITY_NAME[] = "item_glock_flashlight";

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(weapon_glock, CWeaponGlock);

//=============================================
// @brief
//
//=============================================
CWeaponGlock::CWeaponGlock( edict_t* pedict ):
	CPlayerWeapon(pedict),
	m_hasSilencer(false),
	m_isSilenced(false),
	m_hasFlashlight(false),
	m_isFlashlightEquipped(false),
	m_isFlashlightActive(false),
	m_isFlashlightRequested(false),
	m_toggleTime(0),
	m_silencerTime(0),
	m_fireRateDelayTime(0)
{
}

//=============================================
// @brief
//
//=============================================
CWeaponGlock::~CWeaponGlock( void )
{
}

//=============================================
// @brief
//
//=============================================
void CWeaponGlock::SetSpawnProperties( void )
{
	SetBodyGroup(WMODEL_BODY_BASE, WMODEL_GLOCK);

	// Set silencer submodel
	SetBodyGroup(WGLOCK_SL_GROUP_INDEX, m_isSilenced ? WGLOCK_SL_ON : WGLOCK_SL_OFF);

	// Set flashlight submodel
	SetBodyGroup(WGLOCK_FL_GROUP_INDEX, m_isFlashlightEquipped ? WGLOCK_FL_ON : WGLOCK_FL_OFF);

	m_defaultAmmo = WEAPON_DEFAULT_GIVE;
	m_weaponId = WEAPON_GLOCK;
}

//=============================================
// @brief
//
//=============================================
bool CWeaponGlock::KeyValue( const keyvalue_t& kv )
{
	if (!qstrcmp(kv.keyname, "silenced"))
	{
		m_isSilenced = (SDL_atoi(kv.value) == 1) ? true : false;
		m_hasSilencer = m_isSilenced;
		return true;
	}
	else if (!qstrcmp(kv.keyname, "flashlight"))
	{
		m_hasFlashlight = (SDL_atoi(kv.value) == 1) ? true : false;
		m_isFlashlightEquipped = m_hasFlashlight;
		return true;
	}
	else
		return CPlayerWeapon::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
void CWeaponGlock::Precache( void )
{
	CPlayerWeapon::Precache();

	Int32 modelIndex = gd_engfuncs.pfnPrecacheModel(WEAPON_VIEWMODEL);
	if(modelIndex != NO_PRECACHE)
		PrecacheModelSounds(modelIndex);

	gd_engfuncs.pfnPrecacheSound("weapons/glock_fire1.wav");
	gd_engfuncs.pfnPrecacheSound("weapons/glock_fire2.wav");
	gd_engfuncs.pfnPrecacheSound("weapons/glock_fire3.wav");
	gd_engfuncs.pfnPrecacheSound("weapons/glock_fire_silenced1.wav");
	gd_engfuncs.pfnPrecacheSound("weapons/glock_fire_silenced2.wav");
}

//=============================================
// @brief
//
//=============================================
void CWeaponGlock::DeclareSaveFields( void )
{
	CPlayerWeapon::DeclareSaveFields();

	DeclareSaveField(DEFINE_DATA_FIELD(CWeaponGlock, m_hasSilencer, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CWeaponGlock, m_isSilenced, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CWeaponGlock, m_hasFlashlight, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CWeaponGlock, m_isFlashlightEquipped, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CWeaponGlock, m_isFlashlightActive, EFIELD_BOOLEAN));
}

//=============================================
// @brief
//
//=============================================
bool CWeaponGlock::GetWeaponInfo( weaponinfo_t* pWeapon )
{
	pWeapon->name = gd_engfuncs.pfnGetString(m_pFields->classname);
	pWeapon->ammo = AMMOTYPE_9MM_NAME;
	pWeapon->maxammo = MAX_9MM_AMMO;
	pWeapon->maxclip = WEAPON_MAX_CLIP;
	pWeapon->slot = WEAPON_SLOT;
	pWeapon->position = WEAPON_SLOT_POSITION;
	pWeapon->id = WEAPON_GLOCK;
	pWeapon->weight = WEAPON_WEIGHT;
	pWeapon->flags = FL_WEAPON_NO_FIRERATE_LIMIT;
	pWeapon->cone = WEAPON_CONE_ID;
	pWeapon->autoaimdegrees = AUTOAIM_10DEGREES;
	return true;
}

//=============================================
// @brief
//
//=============================================
bool CWeaponGlock::Deploy( void )
{
	if(m_isFlashlightActive)
		m_pPlayer->TurnFlashlightOn(false);

	Uint32 animationIndex;
	if(!m_firstDraw || !m_clip)
	{
		if(!m_clip)
			animationIndex = GLOCK_DEPLOY_EMPTY;
		else
			animationIndex = GLOCK_DEPLOY;
	}
	else
		animationIndex = GLOCK_DEPLOY_FIRST;

	// Set the model before calling DefaultDeploy so SetViewModelBodyGroup works
	if(!SetModel(WEAPON_VIEWMODEL, false))
		return false;

	SetViewModelBodyGroup(VGLOCK_FL_GROUP_INDEX, m_isFlashlightEquipped ? VGLOCK_FL_ON : VGLOCK_FL_OFF);
	SetViewModelBodyGroup(VGLOCK_SL_GROUP_INDEX, m_isSilenced ? VGLOCK_SL_ON : VGLOCK_SL_OFF);

	// Call to play animation
	return DefaultDeploy(WEAPON_VIEWMODEL, m_sequenceNames[animationIndex]);
}

//=============================================
// @brief
//
//=============================================
void CWeaponGlock::Holster( void )
{
	Uint32 animationIndex = (!m_clip) ? GLOCK_HOLSTER_EMPTY : GLOCK_HOLSTER;

	const Char* pstrSequenceName = m_sequenceNames[animationIndex];
	m_nextThinkTime = g_pGameVars->time + GetSequenceTime(pstrSequenceName);
	SetWeaponAnimation(pstrSequenceName);

	if(m_pPlayer->IsFlashlightOn(true))
		m_pPlayer->TurnFlashlightOff(true);

	m_silencerTime = 0;
	CPlayerWeapon::Holster();
}

//=============================================
// @brief
//
//=============================================
void CWeaponGlock::PrimaryAttack( void )
{
	if(m_clip <= 0)
		return;

	if(m_pPlayer->GetWaterLevel() >= WATERLEVEL_FULL)
		return;

	// Subtract from clip
	m_clip--;

	// Set effects if not silenced
	if(!m_isSilenced)
	{
		m_pPlayer->SetEffectFlag(EF_MUZZLEFLASH);
		m_pPlayer->SetWeaponFlashBrightness(GUN_FLASH_NORMAL);
		m_pPlayer->SetWeaponVolume(GUN_VOLUME_NORMAL);
	}

	// Get gun position and angles
	Vector gunPosition = m_pPlayer->GetGunPosition();
	Vector aimAngles = m_pPlayer->GetGunAngles();

	// Perform the shot
	Vector forward, right, up;
	Math::AngleVectors(aimAngles, &forward, &right, &up);

	FireBullets(1, gunPosition, forward, right, up, GetCone(), BULLET_MAX_DISTANCE, BULLET_PLAYER_GLOCK, 0, 0, m_pPlayer);

	// Apply view punch
	m_pPlayer->ApplyAxisPunch( 0, -Common::RandomFloat( 6, 12 ) );
	m_pPlayer->ApplyAxisPunch( 1, -Common::RandomFloat( 2, 4 ) );

	m_pPlayer->ApplyDirectAxisPunch( 0, -Common::RandomFloat( 0.4, 1.4 ) );
	m_pPlayer->ApplyDirectAxisPunch( 1, -Common::RandomFloat( 0.2, 0.8 ) );
	m_pPlayer->ApplyDirectAxisPunch( 2, Common::RandomFloat( -1.25, 1.25 ) );

	// Set animation
	Uint32 animationIndex;
	if(m_isSilenced)
		animationIndex = (m_clip == 0) ? GLOCK_SHOOT_EMPTY_SILENCED : GLOCK_SHOOT_SILENCED;
	else
		animationIndex = (m_clip == 0) ? GLOCK_SHOOT_EMPTY : GLOCK_SHOOT;

	SetWeaponAnimation(m_sequenceNames[animationIndex], NO_BODY_VALUE, NO_SKIN_VALUE, false);

	// Play sound
	CString soundfile;
	if(m_isSilenced)
	{
		switch(Common::RandomLong(0, 1))
		{
			case 0: soundfile = "weapons/glock_fire_silenced1.wav"; break;
			case 1: soundfile = "weapons/glock_fire_silenced2.wav"; break;
		}
	}
	else
	{
		// Not silenced
		switch(Common::RandomLong(0, 2))
		{
			case 0: soundfile = "weapons/glock_fire1.wav"; break;
			case 1: soundfile = "weapons/glock_fire2.wav"; break;
			case 2: soundfile = "weapons/glock_fire3.wav"; break;
		}
	}

	Util::EmitEntitySound(m_pPlayer, soundfile.c_str(), SND_CHAN_WEAPON, VOL_NORM, ATTN_NORM, PITCH_NORM + Common::RandomLong(-5, +10));

	// Add extra recoil if rapidly firing
	if(m_nextAttackTime == -1)
		AddRecoil(1.2);

	m_nextAttackTime = g_pGameVars->time + 0.3;
	m_nextIdleTime = g_pGameVars->time + Common::RandomFloat(5, 10);
}

//=============================================
// @brief
//
//=============================================
void CWeaponGlock::SecondaryAttack( void )
{
	if(!m_hasFlashlight)
		return;

	Int32 animationIndex;
	if(m_clip == 0)
		animationIndex = GLOCK_FLASHLIGHT_TOGGLE_EMPTY;
	else
		animationIndex = GLOCK_FLASHLIGHT_TOGGLE;

	m_nextAttackTime = g_pGameVars->time +  GetSequenceTime(m_sequenceNames[animationIndex]);
	m_reloadDisabledTime = m_nextIdleTime = m_nextAttackTime;
	m_toggleTime = g_pGameVars->time + 0.6;

	SetWeaponAnimation(m_sequenceNames[animationIndex]);
}

//=============================================
// @brief
//
//=============================================
void CWeaponGlock::WeaponSpecialFunction( void )
{
	if(!m_hasSilencer)
		return;

	// Set animation
	Uint32 animationIndex;
	Float silencerTime;

	if(m_isSilenced)
	{
		animationIndex = (m_clip == 0) ? GLOCK_REMOVE_SILENCER_EMPTY : GLOCK_REMOVE_SILENCER;
		silencerTime = 3.667;
		m_isSilenced = false;
	}
	else
	{
		animationIndex = (m_clip == 0) ? GLOCK_ADD_SILENCER_EMPTY : GLOCK_ADD_SILENCER;
		silencerTime = 0.26;
		m_isSilenced = true;
	}

	SetWeaponAnimation(m_sequenceNames[animationIndex]);

	m_nextAttackTime = g_pGameVars->time + GetSequenceTime(m_sequenceNames[animationIndex]);
	m_reloadDisabledTime = m_nextIdleTime = m_nextAttackTime;
	m_fireRateDelayTime = m_nextAttackTime;

	m_silencerTime = g_pGameVars->time + silencerTime;
}

//=============================================
// @brief
//
//=============================================
void CWeaponGlock::Reload( void )
{
	if(m_clip == WEAPON_MAX_CLIP || !m_pPlayer->GetAmmoCount(m_ammoType))
		return;

	Uint32 animationIndex = (m_clip == 0) ? GLOCK_RELOAD : GLOCK_RELOAD_NOT_EMPTY;
	DefaultReload(WEAPON_MAX_CLIP, m_sequenceNames[animationIndex]);
}

//=============================================
// @brief
//
//=============================================
void CWeaponGlock::FlashlightToggle( bool drain )
{
	if(drain)
		m_isFlashlightActive = false;
	else
		SecondaryAttack();
}

//=============================================
// @brief
//
//=============================================
void CWeaponGlock::SetFlashlightRequest( bool requested )
{
	m_isFlashlightRequested = requested;
}

//=============================================
// @brief
//
//=============================================
void CWeaponGlock::Idle( void )
{
	if (m_nextIdleTime > g_pGameVars->time)
		return;

	Uint32 sequenceIndex = 0;
	if(Common::RandomFloat(0.0, 1.0) <= 0.4)
		sequenceIndex = (m_clip == 0) ? GLOCK_IDLE1_EMPTY : GLOCK_IDLE1;
	else
		sequenceIndex = (m_clip == 0) ? GLOCK_IDLE2_EMPTY : GLOCK_IDLE2;

	Float sequenceTime = GetSequenceTime(m_sequenceNames[sequenceIndex]);
	m_nextIdleTime = g_pGameVars->time + sequenceTime*Common::RandomLong(1, 3);

	SetWeaponAnimation( m_sequenceNames[sequenceIndex] );
}

//=============================================
// @brief
//
//=============================================
Float CWeaponGlock::GetRecoilDegradeFactor( void )
{
	return WEAPON_RECOIL_DEGRADE;
}

//=============================================
// @brief
//
//=============================================
void CWeaponGlock::PostThink( void )
{
	if(m_silencerTime && m_silencerTime <= g_pGameVars->time)
	{
		SetViewModelBodyGroup(VGLOCK_SL_GROUP_INDEX, m_isSilenced ? VGLOCK_SL_ON : VGLOCK_SL_OFF);
		m_silencerTime = 0;

		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.viewmodel, nullptr, m_pPlayer->GetEdict());
			gd_engfuncs.pfnMsgWriteByte(VMODEL_SET_BODY);
			gd_engfuncs.pfnMsgWriteInt64(m_viewModelBody);
		gd_engfuncs.pfnUserMessageEnd();
	}

	if(m_hasFlashlight && !m_isFlashlightEquipped)
	{
		Uint32 animationIndex = (m_clip == 0) ? GLOCK_ADD_FLASHLIGHT_EMPTY : GLOCK_ADD_FLASHLIGHT;
		m_isFlashlightEquipped = true;

		SetViewModelBodyGroup(VGLOCK_FL_GROUP_INDEX, VGLOCK_FL_ON);
		SetWeaponAnimation(m_sequenceNames[animationIndex], NO_BODY_VALUE, NO_SKIN_VALUE, false);

		m_reloadDisabledTime = m_nextAttackTime = g_pGameVars->time + GetSequenceTime(m_sequenceNames[animationIndex]);
		m_fireRateDelayTime = m_nextIdleTime = m_nextAttackTime;
	}

	if(m_isFlashlightRequested)
	{
		if(!m_isFlashlightActive)
			SecondaryAttack();

		m_isFlashlightRequested = false;
	}

	if(m_toggleTime)
	{
		if(m_pPlayer->IsForceHolsterSet())
		{
			m_toggleTime = 0;
			return;
		}

		if(m_toggleTime <= g_pGameVars->time)
		{
			Util::EmitEntitySound(m_pPlayer, CPlayerEntity::FLASHLIGHT_TOGGLE_SOUND, SND_CHAN_WEAPON);
			m_toggleTime = 0;

			if( m_isFlashlightActive )
			{
				m_pPlayer->TurnFlashlightOff(true);
				m_isFlashlightActive = false;
			}
			else 
			{
				m_pPlayer->TurnFlashlightOn(false);
				m_isFlashlightActive = true;
			}
		}
		return;
	}

	// Let the rest run it's course
	CPlayerWeapon::PostThink();
}

//=============================================
// @brief
//
//=============================================
bool CWeaponGlock::CanResetAttackTime( void ) 
{
	return (m_fireRateDelayTime < g_pGameVars->time) ? true : false;
}

//=============================================
// @brief
//
//=============================================
bool CWeaponGlock::AddDuplicate( CPlayerWeapon* poriginal )
{
	if(m_hasSilencer && !poriginal->HasSilencer())
	{
		CBaseEntity* pOriginalPlayer = poriginal->GetPlayer();
		if(pOriginalPlayer)
		{
			gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.huditempickup, nullptr, pOriginalPlayer->GetEdict());
				gd_engfuncs.pfnMsgWriteString(SILENCER_ENTITY_NAME);
			gd_engfuncs.pfnUserMessageEnd();

			Util::EmitEntitySound(pOriginalPlayer, "items/pickup_weapon.wav", SND_CHAN_VOICE);
		}

		poriginal->SetHasSilencer(true);
	}

	if(m_hasFlashlight && !poriginal->HasFlashlight())
	{
		CBaseEntity* pOriginalPlayer = poriginal->GetPlayer();
		if(pOriginalPlayer)
		{
			gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.huditempickup, nullptr, pOriginalPlayer->GetEdict());
				gd_engfuncs.pfnMsgWriteString(FLASHLIGHT_ENTITY_NAME);
			gd_engfuncs.pfnUserMessageEnd();

			Util::EmitEntitySound(pOriginalPlayer, "items/pickup_weapon.wav", SND_CHAN_VOICE);
		}

		poriginal->SetHasFlashlight(true);
	}

	return CPlayerWeapon::AddDuplicate(poriginal);
}

//=============================================
// @brief
//
//=============================================
bool CWeaponGlock::HasSilencer( void )
{
	return m_hasSilencer;
}

//=============================================
// @brief
//
//=============================================
void CWeaponGlock::SetHasSilencer( bool hasSilencer )
{
	m_hasSilencer = hasSilencer;
}

//=============================================
// @brief
//
//=============================================
bool CWeaponGlock::HasFlashlight( void )
{
	return m_hasFlashlight;
}

//=============================================
// @brief
//
//=============================================
void CWeaponGlock::SetHasFlashlight( bool hasFlashlight )
{
	m_hasFlashlight = hasFlashlight;
}

//=============================================
// @brief
//
//=============================================
void CWeaponGlock::AddAccessories( CPlayerWeapon* pWeapon )
{
	if(pWeapon->HasSilencer() && !HasSilencer())
	{
		pWeapon->SetHasSilencer(false);
		pWeapon->SetBodyGroup(WGLOCK_SL_GROUP_INDEX, m_isSilenced ? WGLOCK_SL_ON : WGLOCK_SL_OFF);
		m_hasSilencer = true;

		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.huditempickup, nullptr, m_pPlayer->GetEdict());
			gd_engfuncs.pfnMsgWriteString(SILENCER_ENTITY_NAME);
		gd_engfuncs.pfnUserMessageEnd();

		Util::EmitEntitySound(m_pPlayer, "items/pickup_weapon.wav", SND_CHAN_VOICE);
	}

	if(pWeapon->HasFlashlight() && !HasFlashlight())
	{
		pWeapon->SetHasFlashlight(false);
		pWeapon->SetBodyGroup(WGLOCK_FL_GROUP_INDEX, m_isSilenced ? WGLOCK_FL_ON : WGLOCK_FL_OFF);
		m_hasFlashlight = true;

		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.huditempickup, nullptr, m_pPlayer->GetEdict());
			gd_engfuncs.pfnMsgWriteString(FLASHLIGHT_ENTITY_NAME);
		gd_engfuncs.pfnUserMessageEnd();

		Util::EmitEntitySound(m_pPlayer, "items/pickup_weapon.wav", SND_CHAN_VOICE);
	}
}
