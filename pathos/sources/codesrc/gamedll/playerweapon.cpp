/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "playerweapon.h"
#include "player.h"
#include "constants.h"
#include "buttonbits.h"
#include "vbmutils.h"
#include "cl_entity.h"

// Array of weapon infos
weaponinfo_t CPlayerWeapon::WEAPON_INFO_LIST[MAX_WEAPONS];
// Array of ammo types
ammoinfo_t CPlayerWeapon::AMMO_INFO_LIST[MAX_AMMO_TYPES];

//=============================================
//
//=============================================
void Weapon_Precache( const Char* pstrClassname )
{
	edict_t* pedict = gd_engfuncs.pfnCreateEntity(pstrClassname);
	if(!pedict)
	{
		gd_engfuncs.pfnCon_Printf("Entity '%s' could not be precached.\n", pstrClassname);
		return;
	}

	CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
	if(pEntity)
	{
		if(!pEntity->IsWeapon())
		{
			gd_engfuncs.pfnCon_Printf("Entity '%s' is not a weapon.\n", pstrClassname);
			return;
		}

		// Precache resources
		pEntity->Spawn();

		// Register this player weapon
		CPlayerWeapon::RegisterWeapon(pEntity);
	}

	// Remove this entity
	Util::RemoveEntity(pedict);
}

//=============================================
// @brief
//
//=============================================
CPlayerWeapon::CPlayerWeapon( edict_t* pedict ):
	CAnimatingEntity(pedict),
	m_pPlayer(nullptr),
	m_pNext(nullptr),
	m_weaponId(WEAPON_NONE),
	m_dropWeapon(false),
	m_isDuplicate(false),
	m_hasDual(false),
	m_firstDraw(false),
	m_nextThinkTime(0),
	m_nextAttackTime(0),
	m_nextIdleTime(0),
	m_reloadTime(0),
	m_reloadDisabledTime(0),
	m_ammoType(0),
	m_clip(0),
	m_clientClip(0),
	m_rightClip(0),
	m_clientRightClip(0),
	m_leftClip(0),
	m_clientLeftClip(0),
	m_clientWeaponState(0),
	m_viewModelBody(0),
	m_viewModelSkin(0),
	m_isRetired(false),
	m_isForcedToRetire(false),
	m_isDeployed(false),
	m_inReload(false),
	m_inDual(false),
	m_makeImpactSound(false),
	m_defaultAmmo(0),
	m_recoilMultiplier(0),
	m_lastAttackButtonPressed(0)
{
}

//=============================================
// @brief
//
//=============================================
CPlayerWeapon::~CPlayerWeapon( void )
{
}

//=============================================
// @brief
//
//=============================================
void CPlayerWeapon::DeclareSaveFields( void )
{
	// Call base class to do it first
	CAnimatingEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerWeapon, m_pPlayer, EFIELD_ENTPOINTER));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerWeapon, m_pNext, EFIELD_ENTPOINTER));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerWeapon, m_weaponId, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerWeapon, m_dropWeapon, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerWeapon, m_hasDual, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerWeapon, m_firstDraw, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerWeapon, m_ammoType, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerWeapon, m_clip, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerWeapon, m_rightClip, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerWeapon, m_leftClip, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerWeapon, m_viewModelBody, EFIELD_INT64));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerWeapon, m_viewModelSkin, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerWeapon, m_isRetired, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerWeapon, m_isForcedToRetire, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerWeapon, m_isDeployed, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerWeapon, m_defaultAmmo, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerWeapon, m_recoilMultiplier, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CPlayerWeapon, m_inDual, EFIELD_BOOLEAN));
}

//=============================================
// @brief
//
//=============================================
bool CPlayerWeapon::Restore( void )
{
	if(!CAnimatingEntity::Restore())
		return false;

	m_inReload = false;
	m_nextAttackTime = 0;
	m_nextIdleTime = 0;
	m_nextThinkTime = 0;

	if(m_pPlayer && m_pPlayer->GetActiveWeapon() == this)
	{
		if(!m_pPlayer->ShouldHolster())
		{
			m_pPlayer->SetViewModel(gd_engfuncs.pfnGetString(m_pFields->modelname));
			m_pState->flags |= FL_INITIALIZE;
		}
	}
	else
	{
		// Fall to ground on restore
		StartFalling();
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CPlayerWeapon::InitEntity( void )
{
	// Force weapon to think
	PostThink();
}

//=============================================
// @brief
//
//=============================================
bool CPlayerWeapon::Spawn( void )
{
	m_pFields->modelname = gd_engfuncs.pfnAllocString(W_OBJECTS_MODEL_FILENAME);

	if(!CAnimatingEntity::Spawn())
		return false;

	// Apply glow
	Glow();
	// Set weapon-specific properties
	SetSpawnProperties();
	// Fall to place
	StartFalling();

	// Set this here, not in constructor
	m_firstDraw = true;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CPlayerWeapon::AddToPlayer( CPlayerEntity* pPlayer )
{
	m_pPlayer = pPlayer;
	m_pPlayer->SetWeaponBit(m_weaponId);

	if(!m_ammoType)
		m_ammoType = CPlayerWeapon::GetAmmoTypeIndex(GetAmmoTypeName());

	// Extract ammo from this weapon
	ExtractAmmo(this);

	// Reset this
	m_isDeployed = false;

	if(!HasSpawnFlag(FL_WEAPON_NO_NOTICE))
	{
		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.hudweaponpickup, nullptr, m_pPlayer->GetEdict());
			gd_engfuncs.pfnMsgWriteByte(m_weaponId);
		gd_engfuncs.pfnUserMessageEnd();
	}
}

//=============================================
// @brief
//
//=============================================
bool CPlayerWeapon::AddDuplicate( CPlayerWeapon* poriginal )
{
	if(m_defaultAmmo)
		return ExtractAmmo(poriginal);
	else
		return ExtractClipAmmo(poriginal);
}

//=============================================
// @brief
//
//=============================================
bool CPlayerWeapon::ExtractAmmo( CPlayerWeapon* pWeapon )
{
	bool result = true;

	const Char* pstrAmmoTypeName = GetAmmoTypeName();
	if(pstrAmmoTypeName)
	{
		result = pWeapon->AddAmmo(m_defaultAmmo, pstrAmmoTypeName, GetMaxClip(), GetMaxAmmo(), this);
		m_defaultAmmo = 0;
	}

	return result;
}

//=============================================
// @brief
//
//=============================================
bool CPlayerWeapon::ExtractClipAmmo( CPlayerWeapon* pWeapon )
{
	if(!pWeapon->m_pPlayer)
		return false;

	// Get ptr to weapon's player
	CPlayerEntity* pPlayer = pWeapon->m_pPlayer;

	// Determine amount
	Int32 amount = (m_clip == WEAPON_NO_CLIP) ? 0 : m_clip;
	Int32 result = pPlayer->GiveAmmo(amount, GetAmmoTypeName(), GetMaxAmmo(), true, pWeapon);

	return (result != NO_AMMO_INDEX) ? true : false;
}

//=============================================
// @brief
//
//=============================================
void CPlayerWeapon::SetWeaponAnimation( const Char* pstrsequence, Int64 body, Int32 skin, bool blend )
{
	if(!m_pPlayer)
		return;

	if(body != NO_BODY_VALUE)
		m_viewModelBody = body;

	if(skin != NO_SKIN_VALUE)
		m_viewModelSkin = skin;

	Int32 sequence = FindSequence(pstrsequence);
	if(sequence == NO_SEQUENCE_VALUE)
	{
		Util::EntityConPrintf(m_pEdict, "Couldn't find sequence '%s'.\n", pstrsequence);
		sequence = 0;
	}

	gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.viewmodel, nullptr, m_pPlayer->GetEdict());
	gd_engfuncs.pfnMsgWriteByte(VMODEL_SET_SEQUENCE);
	gd_engfuncs.pfnMsgWriteByte(sequence);
	gd_engfuncs.pfnMsgWriteInt64(m_viewModelBody);
	gd_engfuncs.pfnMsgWriteInt32(m_viewModelSkin);
	gd_engfuncs.pfnMsgWriteByte(blend);
	gd_engfuncs.pfnUserMessageEnd();
}

//=============================================
// @brief
//
//=============================================
Vector CPlayerWeapon::GetCone( void )
{
	return Weapon_GetConeSize(GetConeIndex(), m_pPlayer->GetLeanOffset(m_pPlayer->GetButtonBits()), m_pPlayer->GetVelocity(), m_pPlayer->GetPunchAngle());
}

//=============================================
// @brief
//
//=============================================
bool CPlayerWeapon::DefaultDeploy( const Char* pstrviewmodel, const Char* pstrsequence, Int64 body, Int32 skin )
{
	if(!CanDeploy())
		return false;

	if(!SetModel(pstrviewmodel))
		return false;

	// Set viewmodel on player
	m_pPlayer->SetViewModel(pstrviewmodel);

	// Set weapon animation
	SetWeaponAnimation(pstrsequence, body, skin);

	// Set time
	Float sequencetime = GetSequenceTime(pstrsequence);
	m_nextAttackTime = g_pGameVars->time + sequencetime;
	m_nextIdleTime = m_nextAttackTime;
	m_nextThinkTime = m_nextAttackTime;

	// Reset these
	m_isForcedToRetire = false;
	m_recoilMultiplier = 1.0;

	// Kill any particle systems on the view model
	if(m_pPlayer)
	{
		for(Int16 i = 0; i < MAX_ATTACHMENTS; i++)
		{
			gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.removeparticlesystem, nullptr, m_pPlayer->GetEdict());
			gd_engfuncs.pfnMsgWriteInt32((i+1));
			gd_engfuncs.pfnMsgWriteInt32(VIEWMODEL_ENTITY_INDEX);
			gd_engfuncs.pfnMsgWriteByte(FALSE);
			gd_engfuncs.pfnUserMessageEnd();
		}
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CPlayerWeapon::DefaultReload( Int32 clipsize, const Char* pstrsequence, Int64 body, Int32 skin, bool blendanimation )
{
	// Don't reload if clip is full
	if(m_clip == clipsize)
		return false;

	// Make sure we have any ammo
	Uint32 playerammo = m_pPlayer->GetAmmoCount(m_ammoType);
	Uint32 ammoneeded = clipsize - m_clip;
	if(ammoneeded > playerammo)
		ammoneeded = playerammo;

	if(!ammoneeded)
		return false;

	SetWeaponAnimation(pstrsequence, body, skin, blendanimation);

	// Set time
	Float sequencetime = GetSequenceTime(pstrsequence);
	m_nextAttackTime = g_pGameVars->time + sequencetime;
	m_nextIdleTime = m_nextAttackTime;
	m_reloadTime = m_nextAttackTime;
	m_inReload = true;

	// Reset recoil
	m_recoilMultiplier = 1.0;
	return true;
}

//=============================================
// @brief
//
//=============================================
void CPlayerWeapon::DestroyWeapon( void )
{
	if(m_pPlayer)
		m_pPlayer->RemovePlayerWeapon(this);

	FlagForRemoval();
}

//=============================================
// @brief
//
//=============================================
bool CPlayerWeapon::AddAmmo( Int32 count, const Char* pstrname, Int32 maxclip, Int32 maxcarry, CBaseEntity* pWeapon )
{
	Int32 ammoid;
	if(maxclip < 1)
	{
		m_clip = WEAPON_NO_CLIP;
		ammoid = m_pPlayer->GiveAmmo(count, pstrname, maxcarry, true, pWeapon);
	}
	else if(m_clip == 0)
	{
		Int32 clipgive = m_clip+count;
		if(clipgive > maxclip)
			clipgive = maxclip;

		clipgive -= m_clip;
		m_clip += clipgive;

		ammoid = m_pPlayer->GiveAmmo(count-clipgive, pstrname, maxcarry, true, pWeapon);

		// Re-deploy gun
		if(m_isDeployed)
			Deploy();
	}
	else
	{
		// Just give the ammo
		ammoid = m_pPlayer->GiveAmmo(count, pstrname, maxcarry, true, pWeapon);
	}

	// Set ammo type
	if(ammoid != NO_AMMO_INDEX)
		m_ammoType = ammoid;

	return (ammoid != NO_AMMO_INDEX) ? true : false;
}

//=============================================
// @brief
//
//=============================================
void CPlayerWeapon::LandTouch( CBaseEntity* pOther )
{
	// Do not let players pick us up until we touched the ground
	if(pOther->IsPlayer())
		return;
	
	// Play clatter sound
	Util::PlayWeaponClatterSound(m_pEdict);

	// Align to surface if needed
	if(m_pState->flags & FL_ONGROUND)
	{
		Util::AlignEntityToSurface(m_pEdict);
		EnablePlayerTouch();
	}

	// Link to world
	gd_engfuncs.pfnSetOrigin(m_pEdict, m_pState->origin);
}

//=============================================
// @brief
//
//=============================================
void CPlayerWeapon::DefaultTouch( CBaseEntity* pOther )
{
	if(!pOther->IsPlayer())
	{
		if(m_makeImpactSound)
		{
			Util::PlayWeaponClatterSound(m_pEdict);
			m_makeImpactSound = false;
		}

		Util::AlignEntityToSurface(m_pEdict);
		return;
	}

	if(!pOther->CanHaveWeapon(this))
	{
		// Handle if player can't have this item
		if(pOther->AddFullAmmoDual(this))
		{
			FlagForRemoval();

			if(HasSpawnFlag(FL_WEAPON_TRIGGER_ON_PICKUP_ONLY) 
				&& m_pFields->target != NO_STRING_VALUE)
			{
				UseTargets(pOther, USE_TOGGLE, 0);
				m_pFields->target = NO_STRING_VALUE;
			}
		}
		else
		{
			CPlayerWeapon* pWeapon = pOther->GetWeaponList();
			while(pWeapon)
			{
				if(!qstrcmp(pWeapon->GetClassName(), GetClassName()))
					break;

				pWeapon = pWeapon->GetNextWeapon();
			}

			if(pWeapon)
				pWeapon->AddAccessories(this);
		}

		return;
	}

	// Try attaching to player
	if(pOther->AddPlayerWeapon(this))
	{
		AttachToPlayer(pOther);

		if(HasSpawnFlag(FL_WEAPON_TRIGGER_ON_PICKUP_ONLY) 
			&& m_pFields->target != NO_STRING_VALUE)
		{
			UseTargets(pOther, USE_TOGGLE, 0);
			m_pFields->target = NO_STRING_VALUE;
		}
	}
	
	if(!HasSpawnFlag(FL_WEAPON_TRIGGER_ON_PICKUP_ONLY) 
		&& m_pFields->target != NO_STRING_VALUE)
	{
		UseTargets(pOther, USE_TOGGLE, 0);
		m_pFields->target = NO_STRING_VALUE;
	}
}

//=============================================
// @brief
//
//=============================================
void CPlayerWeapon::StartFalling( void )
{
	if(m_pPlayer)
		return;

	m_pState->movetype = MOVETYPE_TOSS;
	m_pState->forcehull = HULL_POINT;
	m_pState->solid = SOLID_TRIGGER;

	// If not dropped by someone, solidify immediately
	if(!m_dropWeapon)
	{
		EnablePlayerTouch();
		return;
	}

	// Reset size to be zero until we touch the ground
	gd_engfuncs.pfnSetMinsMaxs(m_pEdict, Vector(0, 0, 0), Vector(0, 0, 0));

	SetTouch(&CPlayerWeapon::LandTouch);
	m_dropWeapon = false;

	// Make sure this is reset
	if(m_pState->effects & EF_NODRAW)
	{
		m_pState->effects &= ~EF_NODRAW;
		m_pState->effects |= EF_UPDATEMODEL;
	}
}

//=============================================
// @brief
//
//=============================================
bool CPlayerWeapon::CanDeploy( void )
{	
	if(m_pPlayer->ShouldHolster())
		return false;
	else
		return true;
}

//=============================================
// @brief
//
//=============================================
void CPlayerWeapon::Retire( void )
{
	// Call weapon to holster
	Holster();

	m_isRetired = true;
	m_isForcedToRetire = true;
	m_inReload = false;
}

//=============================================
// @brief
//
//=============================================
void CPlayerWeapon::Holster( void )
{
	m_reloadTime = 0;
	m_inReload = false;
}

//=============================================
// @brief
//
//=============================================
void CPlayerWeapon::FinishReload( void )
{
	Uint32 playerammo = m_pPlayer->GetAmmoCount(m_ammoType);
	Uint32 ammoneeded = GetMaxClip() - m_clip;
	if(ammoneeded > playerammo)
		ammoneeded = playerammo;

	m_clip += ammoneeded;
	m_pPlayer->RemoveAmmo(GetAmmoIndex(), ammoneeded);
}

//=============================================
// @brief
//
//=============================================
void CPlayerWeapon::PostThink( void )
{
	// The player code should take care of this, but
	// let's be should anyway
	if(m_nextThinkTime > g_pGameVars->time)
		return;

	if(!m_pPlayer)
		return;

	// Don't think if switching
	if(m_pPlayer->GetNextWeapon())
		return;

	// If the weapon was flagged for drop
	if(m_dropWeapon)
	{
		// Clear viewmodel
		m_pPlayer->SetViewModel(nullptr);

		// Drop the gun
		DropWeapon();
	}
	else if(m_isRetired)
	{
		// Hide the weapon and mark as retired
		m_isRetired = false;
		m_pPlayer->SetViewModel(nullptr);
	}
	else if(!m_isForcedToRetire)
	{
		// Clear this
		if(m_firstDraw)
			m_firstDraw = false;

		// Clear this
		if(m_isDeployed)
			m_isDeployed = true;

		// Degrade recoil
		DegradeRecoil();

		// Reset these if possible
		if((m_lastAttackButtonPressed & IN_ATTACK2) && m_nextAttackTime <= g_pGameVars->time)
			m_lastAttackButtonPressed &= ~IN_ATTACK2;
		if((m_lastAttackButtonPressed & IN_ATTACK) && m_nextAttackTime <= g_pGameVars->time)
			m_lastAttackButtonPressed &= ~IN_ATTACK;
		if((m_lastAttackButtonPressed & IN_SPECIAL) && m_nextAttackTime <= g_pGameVars->time)
			m_lastAttackButtonPressed &= ~IN_SPECIAL;

		// Manage reloading
		if(m_reloadTime)
		{
			if(m_reloadTime <= g_pGameVars->time)
			{
				// Finish the reload
				FinishReload();

				m_inReload = false;
				m_reloadTime = 0;
			}
			else
			{
				// Don't allow anything else to happen while reloading
				return;
			}
		}

		// Check if delay can be removed on attacking
		Int32 playerbuttons = m_pPlayer->GetButtonBits();
		if(GetWeaponFlags() & FL_WEAPON_NO_FIRERATE_LIMIT && !(m_lastAttackButtonPressed & IN_ATTACK2) && CanResetAttackTime())
		{
			if(!(playerbuttons & IN_ATTACK) && m_nextAttackTime > g_pGameVars->time)
				m_nextAttackTime = 0;
		}

		if((playerbuttons & IN_SPECIAL) && m_nextAttackTime <= g_pGameVars->time)
		{
			WeaponSpecialFunction();

			// Clear this bit
			m_pPlayer->ClearButtonBits(IN_SPECIAL);
			m_lastAttackButtonPressed |= IN_SPECIAL;
		}
		else if((playerbuttons & IN_ATTACK2) && m_nextAttackTime <= g_pGameVars->time && !IgnoreSecondaryAttack())
		{
			SecondaryAttack();

			// Clear this bit
			m_pPlayer->ClearButtonBits(IN_ATTACK2);
			m_lastAttackButtonPressed |= IN_ATTACK2;
		}
		else if((playerbuttons & IN_ATTACK) && m_nextAttackTime <= g_pGameVars->time)
		{
			// Perform primary attack
			PrimaryAttack();
			m_lastAttackButtonPressed |= IN_ATTACK;
		}
		else if((playerbuttons & IN_RELOAD) && CanReload())
		{
			// Reload if not grabbed
			Reload();
		}
		else
		{
			Idle();
			return;
		}
	}
}

//=============================================
// @brief
//
//=============================================
void CPlayerWeapon::AttachToPlayer( CBaseEntity* pPlayer )
{
	m_pState->effects |= EF_NODRAW;
	m_pState->solid = SOLID_NOT;
	m_pState->movetype = MOVETYPE_FOLLOW;

	m_pState->owner = m_pPlayer->GetEntityIndex();
	m_pState->aiment = m_pPlayer->GetEntityIndex();

	// Reset this
	m_makeImpactSound = false;

	// Disable thinking
	SetThink(nullptr);
	m_pState->nextthink = 0;
}

//=============================================
// @brief
//
//=============================================
void CPlayerWeapon::Glow( void )
{
	m_pState->renderfx = RenderFx_GlowAura;
	m_pState->rendercolor = WEAPON_GLOW_COLOR;
}

//=============================================
// @brief
//
//=============================================
void CPlayerWeapon::DropWeapon( bool destroy )
{
	CPlayerEntity* pPlayer = m_pPlayer;
	m_pPlayer->RemovePlayerWeapon(this);
	m_pPlayer = nullptr;

	// Clear aiment and owner
	m_pState->aiment = NO_ENTITY_INDEX;
	m_pState->owner = NO_ENTITY_INDEX;

	// If marked for destruction, just call remove
	if(destroy)
	{
		Util::RemoveEntity(this);
		return;
	}

	// Call spawn again
	Spawn();

	Vector forward, right;
	const Vector& viewangles = pPlayer->GetViewAngles();
	Math::AngleVectors(viewangles, &forward, &right);

	// Set origin
	Vector origin = pPlayer->GetWeaponDropPosition();
	SetOrigin(origin);

	// apply velocity
	Vector velocity = pPlayer->GetVelocity() + forward * 180;
	SetVelocity(velocity);

	// Apply avelocity
	Vector avelocity = Vector(Common::RandomFloat(-25, 25),
		Common::RandomFloat(-25, 25),
		Common::RandomFloat(-25, 25));
	SetAngularVelocity(avelocity);

	if(GetWeaponFlags() & FL_WEAPON_EXHAUSTIBLE)
	{
		// Spawn n minus one number of extra objects
		Uint32 ammocount = pPlayer->GetAmmoCount(m_ammoType);
		pPlayer->RemoveAmmo(m_ammoType, ammocount);

		for(Uint32 i = 1; i < ammocount; i++)
		{
			Vector pieceOrigin = origin + forward*2*i + Common::RandomLong(-2, 2)*right*i;
			avelocity = Vector(Common::RandomFloat(-25, 25),
				Common::RandomFloat(-25, 25),
				Common::RandomFloat(-25, 25));

			// Create the entity
			CPlayerWeapon* pentity = reinterpret_cast<CPlayerWeapon*>(CBaseEntity::CreateEntity(gd_engfuncs.pfnGetString(m_pFields->classname), pieceOrigin, m_pState->angles, nullptr));
			if(pentity)
			{
				pentity->FlagDrop();
				pentity->SetVelocity(velocity);
				pentity->SetAngularVelocity(avelocity);

				// Call spawn function
				DispatchSpawn(pentity->m_pEdict);
			}
		}
	}
	else
	{
		if(m_hasDual)
		{
			Vector otherOrigin = origin + right*10;
			avelocity = Vector(Common::RandomFloat(-5, 5),
				Common::RandomFloat(-25, 25),
				Common::RandomFloat(-25, 25));

			// Create the entity
			CPlayerWeapon* pentity = reinterpret_cast<CPlayerWeapon*>(CBaseEntity::CreateEntity(gd_engfuncs.pfnGetString(m_pFields->classname), otherOrigin, m_pState->angles, nullptr));

			if(pentity)
			{
				pentity->FlagDrop();
				pentity->SetVelocity(velocity);
				pentity->SetAngularVelocity(avelocity);

				// Call spawn function
				DispatchSpawn(pentity->m_pEdict);
			}

			// Reset these
			m_isDuplicate = false;
			m_hasDual = false;

			// Reset clip and dual state
			if(m_inDual)
			{
				m_inDual = false;
				m_clip -= m_leftClip;
			}

			// Set these
			if(pentity)
			{
				pentity->m_defaultAmmo = m_leftClip;
				pentity->m_clip = 0;
			}
		}

		m_defaultAmmo = m_clip;
		m_clip = 0;

		m_leftClip = 0;
		m_rightClip = 0;
	}
}

//=============================================
// @brief
//
//=============================================
void CPlayerWeapon::EnablePlayerTouch( void )
{
	// Set collision box
	gd_engfuncs.pfnSetMinsMaxs(m_pEdict, ITEM_HULL_MIN, ITEM_HULL_MAX);

	// Change touch function to DefaultTouch
	SetTouch(&CPlayerWeapon::DefaultTouch);
}

//=============================================
// @brief
//
//=============================================
void CPlayerWeapon::CancelReload( void )
{
	m_inReload = false;
	m_nextThinkTime = 0;
	m_nextAttackTime = 0;
	m_nextIdleTime = 0;
}

//=============================================
// @brief
//
//=============================================
bool CPlayerWeapon::IsUsable( void )
{
	if(m_clip <= 0 && m_leftClip <= 0 && m_rightClip <= 0)
	{
		if(m_pPlayer->GetAmmoCount(m_ammoType) <= 0 && GetMaxAmmo() != WEAPON_NO_CLIP)
			return false;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CPlayerWeapon::UpdateClientData( CPlayerEntity* pPlayer )
{
	// TRUE if we need to update data
	bool updateData = false;

	if(m_pPlayer->ForceWeaponUpdate())
		updateData = true;

	if(m_pPlayer->GetActiveWeapon() == this 
		|| m_pPlayer->GetClientActiveWeapon() == this)
	{
		if(m_pPlayer->GetActiveWeapon() != m_pPlayer->GetClientActiveWeapon()
			|| GetConeIndex() != m_pPlayer->GetClientConeIndex())
			updateData = true;
	}

	if(m_clip != m_clientClip 
		|| m_pPlayer->GetFOV() != m_pPlayer->GetClientFOV()
		|| m_leftClip != m_clientLeftClip
		|| m_rightClip != m_clientRightClip)
		updateData = true;

	if(updateData)
	{
		Uint32 coneindex = GetConeIndex();
		gd_engfuncs.pfnUserMessageBegin(MSG_ONE, g_usermsgs.hudcurrentweapon, nullptr, m_pPlayer->GetEdict());
			gd_engfuncs.pfnMsgWriteByte((m_pPlayer->GetActiveWeapon() == this) ? 1 : 0);
			gd_engfuncs.pfnMsgWriteByte(m_weaponId);
			gd_engfuncs.pfnMsgWriteInt16(m_clip);
			gd_engfuncs.pfnMsgWriteByte(coneindex);
			gd_engfuncs.pfnMsgWriteByte(m_rightClip);
			gd_engfuncs.pfnMsgWriteByte(m_leftClip);
		gd_engfuncs.pfnUserMessageEnd();

		m_clientClip = m_clip;
		m_clientRightClip = m_rightClip;
		m_clientLeftClip = m_leftClip;

		if(m_pPlayer->GetActiveWeapon() == this)
			m_pPlayer->SetClientConeIndex(coneindex);
	}

	if(m_pNext)
		m_pNext->UpdateClientData(pPlayer);

	return updateData;
}

//=============================================
// @brief
//
//=============================================
Int32 CPlayerWeapon::GetClip( void ) const
{
	return m_clip;
}

//=============================================
// @brief
//
//=============================================
Int32 CPlayerWeapon::GetLeftClip( void ) const
{
	return m_leftClip;
}

//=============================================
// @brief
//
//=============================================
Int32 CPlayerWeapon::GetRightClip( void ) const
{
	return m_rightClip;
}

//=============================================
// @brief
//
//=============================================
void CPlayerWeapon::SetClip( Int32 clip )
{
	m_clip = clip;
}

//=============================================
// @brief
//
//=============================================
void CPlayerWeapon::SetRightClip( Int32 clip )
{
	m_rightClip = clip;
}

//=============================================
// @brief
//
//=============================================
void CPlayerWeapon::SetLeftClip( Int32 clip )
{
	m_leftClip = clip;
}

//=============================================
// @brief
//
//=============================================
Int32 CPlayerWeapon::GetId( void ) const
{
	return m_weaponId;
}

//=============================================
// @brief
//
//=============================================
Int32 CPlayerWeapon::GetHUDPosition( void )
{
	assert(m_weaponId >= 0 && m_weaponId < NUM_WEAPONS);
	return WEAPON_INFO_LIST[m_weaponId].position;
}

//=============================================
// @brief
//
//=============================================
const Char* CPlayerWeapon::GetAmmoTypeName( void )
{
	assert(m_weaponId >= 0 && m_weaponId < NUM_WEAPONS);
	return WEAPON_INFO_LIST[m_weaponId].ammo.c_str();
}

//=============================================
// @brief
//
//=============================================
const Char* CPlayerWeapon::GetWeaponName( void )
{
	assert(m_weaponId >= 0 && m_weaponId < NUM_WEAPONS);
	return WEAPON_INFO_LIST[m_weaponId].name.c_str();
}

//=============================================
// @brief
//
//=============================================
Int32 CPlayerWeapon::GetMaxClip( void )
{
	assert(m_weaponId >= 0 && m_weaponId < NUM_WEAPONS);
	return WEAPON_INFO_LIST[m_weaponId].maxclip;
}

//=============================================
// @brief
//
//=============================================
Int32 CPlayerWeapon::GetWeight( void )
{
	assert(m_weaponId >= 0 && m_weaponId < NUM_WEAPONS);
	return WEAPON_INFO_LIST[m_weaponId].weight;
}

//=============================================
// @brief
//
//=============================================
Int32 CPlayerWeapon::GetWeaponFlags( void )
{
	assert(m_weaponId >= 0 && m_weaponId < NUM_WEAPONS);
	return WEAPON_INFO_LIST[m_weaponId].flags;
}

//=============================================
// @brief
//
//=============================================
Uint32 CPlayerWeapon::GetConeIndex( void )
{
	assert(m_weaponId >= 0 && m_weaponId < NUM_WEAPONS);
	return WEAPON_INFO_LIST[m_weaponId].cone;
}

//=============================================
// @brief
//
//=============================================
Int32 CPlayerWeapon::GetHUDSlot( void )
{
	assert(m_weaponId >= 0 && m_weaponId < NUM_WEAPONS);
	return WEAPON_INFO_LIST[m_weaponId].slot;
}

//=============================================
// @brief
//
//=============================================
Int32 CPlayerWeapon::GetMaxAmmo( void )
{
	assert(m_weaponId >= 0 && m_weaponId < NUM_WEAPONS);
	return WEAPON_INFO_LIST[m_weaponId].maxammo; 
}

//=============================================
// @brief
//
//=============================================
void CPlayerWeapon::DegradeRecoil( void )
{
	if(m_recoilMultiplier == 1.0)
		return;

	m_recoilMultiplier = GetRecoilDegradeFactor() * g_pGameVars->frametime;
	if(m_recoilMultiplier < 1.0)
		m_recoilMultiplier = 1.0;
}

//=============================================
// @brief
//
//=============================================
void CPlayerWeapon::AddRecoil( Float recoil )
{
	m_recoilMultiplier += recoil;

	Float limit = GetRecoilLimit();
	if(m_recoilMultiplier > limit)
		m_recoilMultiplier = limit;
}

//=============================================
// @brief
//
//=============================================
Float CPlayerWeapon::GetAutoAimDegrees( void )
{
	assert(WEAPON_INFO_LIST[m_weaponId].autoaimdegrees >= 0 
		&& WEAPON_INFO_LIST[m_weaponId].autoaimdegrees < NB_AUTOAIM_DEGREES);

	return AUTOAIM_DEGREES_VALUES[WEAPON_INFO_LIST[m_weaponId].autoaimdegrees];
}

//=============================================
// @brief
//
//=============================================
void CPlayerWeapon::SetViewModelBodyGroup( Int32 group, Int32 value )
{
	if(!m_pState->modelindex || m_pFields->modelname == NO_STRING_VALUE)
		return;

	const cache_model_t* pmodel = gd_engfuncs.pfnGetModel(m_pState->modelindex);
	if(!pmodel)
	{
		gd_engfuncs.pfnCon_Printf("%s - Called on entity with no model.\n", __FUNCTION__);
		return;
	}

	VBM_SetBodyGroup(pmodel, group, value, m_viewModelBody);
}

//=============================================
// @brief
//
//=============================================
CPlayerWeapon* CPlayerWeapon::GetNextWeapon( void )
{
	return m_pNext;
}

//=============================================
// @brief
//
//=============================================
void CPlayerWeapon::SetEnableImpactSound( bool enable ) 
{ 
	m_makeImpactSound = enable; 
}

//=============================================
// @brief
//
//=============================================
void CPlayerWeapon::SetNextWeapon( CPlayerWeapon* pWeapon )
{
	m_pNext = pWeapon;
}

//=============================================
// @brief
//
//=============================================
bool CPlayerWeapon::CanReload( void )
{
	if(GetMaxClip() == WEAPON_NO_CLIP)
		return false;

	if(m_inReload)
		return false;

	if(m_reloadDisabledTime && m_reloadDisabledTime > g_pGameVars->time)
		return false;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CPlayerWeapon::RegisterWeapon( CBaseEntity* pWeapon )
{
	weaponinfo_t wpninfo;
	if(pWeapon->GetWeaponInfo(&wpninfo))
	{
		if(!wpninfo.ammo.empty())
		{
			// Register ammo type
			RegisterAmmoType(wpninfo.ammo.c_str());
		}
	}

	// Check if it's already registered
	if(WEAPON_INFO_LIST[wpninfo.id].id != WEAPON_NONE)
		return;

	// Get slot based on id
	weaponinfo_t& wpn = WEAPON_INFO_LIST[wpninfo.id];
	wpn = wpninfo;
}

//=============================================
// @brief
//
//=============================================
void CPlayerWeapon::RegisterAmmoType( const Char* pstrAmmoTypeName )
{
	Int32 emptyindex = NO_AMMO_INDEX;
	for(Uint32 i = 0; i < MAX_AMMO_TYPES; i++)
	{
		// Skip empty ones
		if(AMMO_INFO_LIST[i].name.empty())
		{
			if(emptyindex == NO_AMMO_INDEX)
				emptyindex = i;

			continue;
		}

		// Check if already in list
		if(!qstrcmp(AMMO_INFO_LIST[i].name, pstrAmmoTypeName))
			return;
	}

	// Make sure we didn't run out
	if(emptyindex == NO_AMMO_INDEX)
	{
		gd_engfuncs.pfnCon_Printf("Exceeded MAX_AMMO_TYPES.\n");
		return;
	}

	// Save it
	AMMO_INFO_LIST[emptyindex].name = pstrAmmoTypeName;
	AMMO_INFO_LIST[emptyindex].id = emptyindex;
}

//=============================================
// @brief
//
//=============================================
void CPlayerWeapon::ClearWeaponInfos( void )
{
	for(Uint32 i = 0; i < MAX_WEAPONS; i++)
		WEAPON_INFO_LIST[i] = weaponinfo_t();

	for(Uint32 i = 0; i < MAX_AMMO_TYPES; i++)
		AMMO_INFO_LIST[i] = ammoinfo_t();
}

//=============================================
// @brief
//
//=============================================
Int32 CPlayerWeapon::GetAmmoTypeIndex( const Char* pstrAmmoTypeName )
{
	for(Uint32 i = 0; i < MAX_AMMO_TYPES; i++)
	{
		if(AMMO_INFO_LIST[i].name.empty())
			continue;

		if(!qstrcmp(AMMO_INFO_LIST[i].name, pstrAmmoTypeName))
			return i;
	}

	return NO_AMMO_INDEX;
}

//=============================================
// @brief
//
//=============================================
weaponinfo_t& CPlayerWeapon::GetWeaponInfo( weaponid_t weaponid )
{
	assert(weaponid < NUM_WEAPONS);
	return WEAPON_INFO_LIST[weaponid];
}