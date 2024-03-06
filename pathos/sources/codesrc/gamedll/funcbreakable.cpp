/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "funcbreakable.h"
#include "envexplosion.h"
#include "playerweapon.h"

// Object types spawned by breakable
const Char* CFuncBreakable::SPAWN_OBJECT_TYPES[SPAWN_OBJECT_TYPES_COUNT] = 
{
	nullptr,				// 0
	"item_kevlar",			// 1
	"item_healthkit",		// 2
	"weapon_glock",			// 3
	"ammo_glock_clip",		// 4
	"weapon_handgrenade",	// 5
};

// Wood gibs model file
const Char CFuncBreakable::GIB_MODEL_WOOD_FILENAME[] = "models/woodgibs.mdl";
// Flesh gibs model file
const Char CFuncBreakable::GIB_MODEL_FLESH_FILENAME[] = "models/fleshgibs.mdl";
// Computer gibs model file
const Char CFuncBreakable::GIB_MODEL_COMPUTER_FILENAME[] = "models/computergibs.mdl";
// Glass gibs model file
const Char CFuncBreakable::GIB_MODEL_GLASS_FILENAME[] = "models/glassgibs.mdl";
// Metal gibs model file
const Char CFuncBreakable::GIB_MODEL_METAL_FILENAME[] = "models/metalplategibs.mdl";
// Cinder block gibs model file
const Char CFuncBreakable::GIB_MODEL_CINDERBLOCKS_FILENAME[] = "models/cindergibs.mdl";
// Rock gibs model file
const Char CFuncBreakable::GIB_MODEL_ROCK_FILENAME[] = "models/rockgibs.mdl";
// Ceiling gibs model file
const Char CFuncBreakable::GIB_MODEL_CEILING_FILENAME[] = "models/ceilinggibs.mdl";

// Wood damage sounds pattern
const Char CFuncBreakable::WOOD_DMG_SOUNDS_PATTERN[] = "debris/wood_dmg%d.wav";
// Wood break sounds pattern
const Char CFuncBreakable::WOOD_BREAK_SOUNDS_PATTERN[] = "debris/wood_break%d.wav";
// Flesh sounds pattern
const Char CFuncBreakable::FLESH_DMG_SOUNDS_PATTERN[] = "debris/flesh_splatter%d.wav";
// Glass damage sounds pattern
const Char CFuncBreakable::GLASS_DMG_SOUNDS_PATTERN[] = "debris/glass_dmg%d.wav";
// Glass break sounds pattern
const Char CFuncBreakable::GLASS_BREAK_SOUNDS_PATTERN[] = "debris/glass_break%d.wav";;
// Metal damage sounds pattern
const Char CFuncBreakable::METAL_DMG_SOUNDS_PATTERN[] = "debris/metal_dmg%d.wav";
// Metal break sounds pattern
const Char CFuncBreakable::METAL_BREAK_SOUNDS_PATTERN[] = "debris/metal_break%d.wav";
// Concrete damage sounds pattern
const Char CFuncBreakable::CONCRETE_DMG_SOUNDS_PATTERN[] = "debris/concrete_dmg%d.wav";
// Concrete break sounds pattern
const Char CFuncBreakable::CONCRETE_BREAK_SOUNDS_PATTERN[] = "debris/concrete_break%d.wav";
// Computer break sounds pattern
const Char CFuncBreakable::COMPUTER_DMG_SOUNDS_PATTERN[] = "misc/spark%d.wav";

// Gib model lifetime
const Float CFuncBreakable::GIB_MODEL_LIFETIME = 30;

// Weapon->ammo associations
const CFuncBreakable::ammo_assocation_t CFuncBreakable::WPN_AMMO_ASSOCIATIONS[NUM_WEAPONS] = {
	{ WEAPON_NONE, "" },
	{ WEAPON_GLOCK, "ammo_glock_clip" },
	{ WEAPON_HANDGRENADE, "" },
	{ WEAPON_KNIFE, "" },
};

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(func_breakable, CFuncBreakable);

//=============================================
// @brief
//
//=============================================
CFuncBreakable::CFuncBreakable( edict_t* pedict ):
	CDelayEntity(pedict),
	m_material(MAT_GLASS),
	m_breakDirection(BREAKDIR_RANDOM),
	m_tempentSoundFlag(TE_BOUNCE_NONE),
	m_breakModelIndex(NO_PRECACHE),
	m_angle(0),
	m_explodeMagnitude(0),
	m_gibModelName(NO_STRING_VALUE),
	m_spawnEntityName(NO_STRING_VALUE),
	m_dmgSoundsPattern(NO_STRING_VALUE),
	m_nbDmgSounds(0),
	m_breakSoundsPattern(NO_STRING_VALUE),
	m_nbBreakSounds(0),
	m_spawnChance(1)
{
}

//=============================================
// @brief
//
//=============================================
CFuncBreakable::~CFuncBreakable( void )
{
}

//=============================================
// @brief
//
//=============================================
void CFuncBreakable::DeclareSaveFields( void )
{
	// Call base class to do it first
	CDelayEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncBreakable, m_material, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncBreakable, m_breakDirection, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncBreakable, m_tempentSoundFlag, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncBreakable, m_angle, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncBreakable, m_explodeMagnitude, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncBreakable, m_gibModelName, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncBreakable, m_spawnEntityName, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncBreakable, m_dmgSoundsPattern, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncBreakable, m_nbDmgSounds, EFIELD_UINT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncBreakable, m_breakSoundsPattern, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncBreakable, m_nbBreakSounds, EFIELD_UINT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncBreakable, m_spawnChance, EFIELD_UINT32));
}

//=============================================
// @brief
//
//=============================================
bool CFuncBreakable::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "explosion"))
	{
		if(!qstrcmp(kv.value, "directed"))
			m_breakDirection = BREAKDIR_ATTACKDIR;
		else
			m_breakDirection = BREAKDIR_RANDOM;
		return true;
	}
	else if(!qstrcmp(kv.keyname, "material"))
	{
		m_material = SDL_atoi(kv.value);
		if(m_material < 0 || m_material >= NB_BREAK_MATERIALS)
			m_material = MAT_WOOD;
		return true;
	}
	else if(!qstrcmp(kv.keyname, "gibmodel"))
	{
		m_gibModelName = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "spawnobject"))
	{
		Int32 spawnObjectIndex = SDL_atoi(kv.value);
		if(spawnObjectIndex > 0 && spawnObjectIndex < SPAWN_OBJECT_TYPES_COUNT)
			m_spawnEntityName = gd_engfuncs.pfnAllocString(SPAWN_OBJECT_TYPES[spawnObjectIndex]);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "explodemagnitude"))
	{
		m_explodeMagnitude = SDL_atoi(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "spawnchance"))
	{
		m_spawnChance = SDL_atoi(kv.value);
		return true;
	}
	else
		return CDelayEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
void CFuncBreakable::SetPrecacheObjects( void )
{
	CString gibmodelname;
	CString dmgsoundpattern;
	CString breaksoundpattern;

	switch(m_material)
	{
	case MAT_FLESH:
		{
			gibmodelname = GIB_MODEL_FLESH_FILENAME;
			dmgsoundpattern = FLESH_DMG_SOUNDS_PATTERN;
			breaksoundpattern = FLESH_DMG_SOUNDS_PATTERN;
			m_tempentSoundFlag = TE_BOUNCE_FLESH;
		}
		break;
	case MAT_COMPUTER:
		{
			gibmodelname = GIB_MODEL_COMPUTER_FILENAME;
			dmgsoundpattern = COMPUTER_DMG_SOUNDS_PATTERN;
			breaksoundpattern = METAL_BREAK_SOUNDS_PATTERN;
			m_tempentSoundFlag = TE_BOUNCE_METAL;
		}
		break;
	case MAT_GLASS:
	case MAT_UNBREAKABLE_GLASS:
		{
			gibmodelname = GIB_MODEL_GLASS_FILENAME;
			dmgsoundpattern = GLASS_DMG_SOUNDS_PATTERN;
			breaksoundpattern = GLASS_BREAK_SOUNDS_PATTERN;
			m_tempentSoundFlag = TE_BOUNCE_GLASS;
		}
		break;
	case MAT_METAL:
		{
			gibmodelname = GIB_MODEL_METAL_FILENAME;
			dmgsoundpattern = METAL_DMG_SOUNDS_PATTERN;
			breaksoundpattern = METAL_BREAK_SOUNDS_PATTERN;
			m_tempentSoundFlag = TE_BOUNCE_METAL;
		}
		break;
	case MAT_CINDERBLOCK:
		{
			gibmodelname = GIB_MODEL_CINDERBLOCKS_FILENAME;
			dmgsoundpattern = CONCRETE_DMG_SOUNDS_PATTERN;
			breaksoundpattern = CONCRETE_BREAK_SOUNDS_PATTERN;
			m_tempentSoundFlag = TE_BOUNCE_CONCRETE;
		}
		break;
	case MAT_ROCKS:
		{
			gibmodelname = GIB_MODEL_ROCK_FILENAME;
			dmgsoundpattern = CONCRETE_DMG_SOUNDS_PATTERN;
			breaksoundpattern = CONCRETE_BREAK_SOUNDS_PATTERN;
			m_tempentSoundFlag = TE_BOUNCE_CONCRETE;
		}
		break;
	case MAT_CEILINGTILE:
		{
			gibmodelname = GIB_MODEL_CEILING_FILENAME;
			dmgsoundpattern = CONCRETE_DMG_SOUNDS_PATTERN;
			breaksoundpattern = CONCRETE_BREAK_SOUNDS_PATTERN;
			m_tempentSoundFlag = TE_BOUNCE_CONCRETE;
		}
		break;
	case MAT_WOOD:
	default:
		{
			gibmodelname = GIB_MODEL_WOOD_FILENAME;
			dmgsoundpattern = WOOD_DMG_SOUNDS_PATTERN;
			breaksoundpattern = WOOD_BREAK_SOUNDS_PATTERN;
			m_tempentSoundFlag = TE_BOUNCE_WOOD;
		}
		break;
	}

	m_gibModelName = gd_engfuncs.pfnAllocString(gibmodelname.c_str());
	m_dmgSoundsPattern = gd_engfuncs.pfnAllocString(dmgsoundpattern.c_str());
	m_breakSoundsPattern = gd_engfuncs.pfnAllocString(breaksoundpattern.c_str());
}

//=============================================
// @brief
//
//=============================================
void CFuncBreakable::Precache( void )
{
	const Char* pstrDmgSound = gd_engfuncs.pfnGetString(m_dmgSoundsPattern);
	if(pstrDmgSound && qstrlen(pstrDmgSound))
	{
		Util::PrecacheVariableNbSounds(pstrDmgSound, m_nbDmgSounds);
		if(!m_nbDmgSounds)
			Util::EntityConPrintf(m_pEdict, "Couldn't precache damage sounds.\n");
	}

	const Char* pstrBreakSound = gd_engfuncs.pfnGetString(m_breakSoundsPattern);
	if(pstrBreakSound && qstrlen(pstrBreakSound))
	{
		Util::PrecacheVariableNbSounds(pstrBreakSound, m_nbBreakSounds);
		if(!m_nbBreakSounds)
			Util::EntityConPrintf(m_pEdict, "Couldn't precache break sounds.\n");
	}

	if(m_gibModelName != NO_STRING_VALUE)
	{
		const Char* pstrGibFilePath = gd_engfuncs.pfnGetString(m_gibModelName);
		m_breakModelIndex = gd_engfuncs.pfnPrecacheModel(pstrGibFilePath);
	}

	if(m_spawnEntityName != NO_STRING_VALUE)
	{
		const Char* pstrSpawnObject = gd_engfuncs.pfnGetString(m_spawnEntityName);
		Util::PrecacheEntity(pstrSpawnObject);
	}
}

//=============================================
// @brief
//
//=============================================
bool CFuncBreakable::Spawn( void )
{
	// Set precache objects
	SetPrecacheObjects();

	if(!CDelayEntity::Spawn())
		return false;

	if(HasSpawnFlag(FL_BREAK_ON_TRIGGER_ONLY))
		m_pState->takedamage = TAKEDAMAGE_NO;
	else
		m_pState->takedamage = TAKEDAMAGE_YES;

	m_angle = m_pState->angles.y;

	SetSpawnProperties();

	if(!SetModel(m_pFields->modelname))
		return false;

	if(!HasSpawnFlag(FL_BREAK_ON_TRIGGER_ONLY))
		SetTouch(&CFuncBreakable::BreakTouch);
	
	if(!IsFuncBreakableEntity() && m_pState->rendermode != RENDER_NORMAL)
	{
		const cache_model_t* pmodel = gd_engfuncs.pfnGetModel(m_pEdict->state.modelindex);

		if(pmodel && pmodel->type == MOD_BRUSH)
			m_pState->flags |= FL_WORLDBRUSH;
	}

	if(m_spawnChance < 1)
		m_spawnChance = 1;

	// Used by env_model_breakable
	SetBoundingBox();

	return true;
}

//=============================================
// @brief
//
//=============================================
void CFuncBreakable::SetSpawnProperties( void )
{
	m_pState->solid = SOLID_BSP;
	m_pState->movetype = MOVETYPE_PUSH;
	m_pState->angles.y = 0;
}

//=============================================
// @brief
//
//=============================================
void CFuncBreakable::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	// Don't break if not breakable
	if(!IsFuncBreakableEntity())
		return;

	Vector realAngles = m_pState->angles;
	realAngles.y = m_angle;

	Vector forward;
	Math::AngleVectors(realAngles, &forward);
	gMultiDamage.SetAttackDirection(forward);

	DieThink();
}

//=============================================
// @brief
//
//=============================================
bool CFuncBreakable::IsFuncBreakableEntity( void ) const
{
	return (m_material != MAT_UNBREAKABLE_GLASS) ? true : false;
}

//=============================================
// @brief
//
//=============================================
bool CFuncBreakable::TakeDamage( CBaseEntity* pInflictor, CBaseEntity* pAttacker, Float amount, Int32 damageFlags )
{
	// Don't take damage if not breakable
	if(!IsFuncBreakableEntity())
		return false;

	// Club weapons do twice the damage
	Float damagedealt;
	if(damageFlags & (DMG_MELEE|DMG_AXE))
		damagedealt = amount *= 2;
	else 
		damagedealt = amount;

	if(damagedealt <= 0)
		return true;

	// Determine attack vector
	Vector attackVector;
	if(pInflictor)
		attackVector = pInflictor->GetOrigin() - (m_pState->absmin + (m_pState->size*0.5));

	gMultiDamage.SetAttackDirection(attackVector);

	// Deal the damage
	m_pState->health -= damagedealt;
	if(m_pState->health <= 0)
	{
		m_attacker = pAttacker;
		Killed(pAttacker, GIB_NORMAL);
		DieThink();
		return false;
	}

	// Play damage sound
	PlayDamageSound();
	return true;
}

//=============================================
// @brief
//
//=============================================
void CFuncBreakable::TraceAttack( CBaseEntity* pAttacker, Float damage, const Vector& direction, trace_t& tr, Int32 damageFlags )
{
	// Add special effects
	switch(m_material)
	{
	case MAT_COMPUTER:
		{
			// Create spark effect at random
			if(Common::RandomLong(0, 1))
				Util::CreateSparks(tr.endpos);
		}
		break;
	case MAT_UNBREAKABLE_GLASS:
		{
			// Create ricochet effect
			Util::Ricochet(tr.endpos, tr.plane.normal, false);
		}
		break;
	}
	
	// Check if we allow bullet penetration to damage us
	if(HasSpawnFlag(FL_NO_PENETRATION_DAMAGE) && (damageFlags & DMG_PENETRATION))
		damage = 0;

	// Everything else is managed by the base entity class
	CDelayEntity::TraceAttack(pAttacker, damage, direction, tr, damageFlags);
}

//=============================================
// @brief
//
//=============================================
void CFuncBreakable::BreakTouch( CBaseEntity* pOther )
{
	// Only player touch can break this
	if(!pOther->IsPlayer() || !IsFuncBreakableEntity())
		return;

	// Break if touched by player if set
	if(HasSpawnFlag(FL_BREAK_ON_TOUCH))
	{
		// Calculate damage dealt
		Float damage = pOther->GetVelocity().Length() * 0.01;
		if(damage >= m_pState->health)
		{
			SetTouch(nullptr);
			TakeDamage(pOther, pOther, damage, DMG_CRUSH);

			// Apply slash damage to player if computer or glass
			if(m_material == MAT_GLASS || m_material == MAT_COMPUTER)
				pOther->TakeDamage(this, this, damage/4.0f, DMG_SLASH);
		}
	}

	// Break if stood upon and set to do so
	if(HasSpawnFlag(FL_BREAK_ON_PRESSURE) && pOther->GetGroundEntity() == this)
	{
		// Play damage sound
		PlayDamageSound();

		// Disable touch
		SetTouch(nullptr);

		// Set to break with delay
		SetThink(&CFuncBreakable::DieThink);
		Float delay = m_delay;
		if(!delay)
			delay = 0.1;

		m_pState->nextthink = m_pState->ltime + delay;
	}
}

//=============================================
// @brief
//
//=============================================
void CFuncBreakable::DieThink( void )
{
	// Calculate pitch
	Int32 pitch = PITCH_NORM + Common::RandomLong(-5, 25);

	// Calculate volume
	Float volume = Common::RandomFloat(0.85, VOL_NORM);
	volume += SDL_fabs(m_pState->health)/100.0f;
	volume = clamp(volume, 0.0, VOL_NORM);

	// Get center of bmodel
	Vector center = GetCenter();

	// Play material breaking sound
	if(m_nbBreakSounds > 0)
	{
		const Char* pstrBreakSound = gd_engfuncs.pfnGetString(m_breakSoundsPattern);
		if(pstrBreakSound && qstrlen(pstrBreakSound))
			Util::PlayRandomAmbientSound(center, pstrBreakSound, m_nbBreakSounds, volume, ATTN_NORM, pitch);
	}

	// Calculate gib velocity
	Vector velocity;
	switch(m_breakDirection)
	{
	case BREAKDIR_ATTACKDIR:
		{
			Vector attackDir = gMultiDamage.GetAttackDirection();
			velocity = attackDir * 200;
		}
		break;
	case BREAKDIR_RANDOM:
	default:
		break;
	}

	Float bouyancy = 0;
	Float waterfriction = 0;
	switch(m_material)
	{
	case MAT_FLESH: 
		bouyancy = 500; 
		waterfriction = 1.0;
		break;
	case MAT_GLASS:
	case MAT_UNBREAKABLE_GLASS:
	case MAT_COMPUTER: 
		bouyancy = 400; 
		waterfriction = 0.5;
		break;
	case MAT_METAL:
		bouyancy = 250;
		waterfriction = 0.8;
		break;
	case MAT_CINDERBLOCK:
	case MAT_ROCKS:
		bouyancy = 200;
		waterfriction = 0.3;
		break;
	case MAT_CEILINGTILE:
		bouyancy = 600;
		waterfriction = 0.7;
		break;
	case MAT_WOOD:
		bouyancy = 700;
		waterfriction = 1.3;
		break;
	default:
		bouyancy = 0;
		waterfriction = 0;
		break;
	}

	// Spawn the gib models
	if(!HasSpawnFlag(FL_NO_GIBS))
	{
		Util::CreateBreakModel(center, 
			m_pState->size, 
			velocity, 
			10, 
			GIB_MODEL_LIFETIME, 
			0, 
			m_breakModelIndex,
			m_tempentSoundFlag,
			bouyancy,
			waterfriction,
			0);
	}

	// Unlink any groundents
	for(Int32 i = 0; i < g_pGameVars->numentities; i++)
	{
		edict_t* pedict = gd_engfuncs.pfnGetEdictByIndex(i);
		if(!pedict || Util::IsNullEntity(pedict))
			continue;

		if(pedict->state.groundent == m_pEdict->entindex)
		{
			pedict->state.flags &= ~FL_ONGROUND;
			pedict->state.groundent = NO_ENTITY_INDEX;
		}
	}

	// Remove targetname and solidity
	m_pFields->targetname = NO_STRING_VALUE;
	m_pState->solid = SOLID_NOT;

	// Fire targets
	UseTargets(nullptr, USE_TOGGLE, 0);

	Vector spawnAngle = m_pState->angles;
	spawnAngle.y = m_angle;

	if(m_spawnChance <= 1 || Common::RandomLong(1, m_spawnChance) == 1)
	{
		// Spawn drop object if any
		if(!HasSpawnFlag(FL_SMART_AMMO_SPAWN))
		{
			if(m_spawnEntityName != NO_STRING_VALUE)
			{
				const Char* pstrSpawnObject = gd_engfuncs.pfnGetString(m_spawnEntityName);
				CBaseEntity* pEntity = CBaseEntity::CreateEntity(pstrSpawnObject, center, spawnAngle, this);
				if(pEntity)
				{
					if(!pEntity->Spawn())
						Util::RemoveEntity(pEntity);
				}
			}
		}
		else if(m_attacker && m_attacker->IsPlayer())
		{
			// Try to find the weapon with the lowest ammo
			CBaseEntity* pAttacker = m_attacker;
			
			// Weapon with lowest ammo
			CPlayerWeapon* pLowestAmmoWeapon = nullptr;
			Uint32 lowestAmmoPercentage = 0;

			CPlayerWeapon* pWeapon = pAttacker->GetWeaponList();
			while(pWeapon)
			{
				if(pWeapon->GetMaxAmmo() != -1)
				{
					Int32 ammoType = pWeapon->GetAmmoIndex();
					Uint32 ammoCount = m_attacker->GetAmmoCount(ammoType);

					Int32 weaponId = pWeapon->GetId();
					assert(weaponId >= 0 && weaponId < NUM_WEAPONS);
					Uint32 maxAmmo = MAX_AMMO_COUNTS[weaponId];
					assert(maxAmmo > 0);

					Uint32 ammoPercentage = ((Float)ammoCount/(Float)maxAmmo) * 100;
					if(!pLowestAmmoWeapon || ammoPercentage < lowestAmmoPercentage)
					{
						pLowestAmmoWeapon = pWeapon;
						lowestAmmoPercentage = ammoPercentage;
					}
				}

				pWeapon = pWeapon->GetNextWeapon();
			}

			if(pLowestAmmoWeapon)
			{
				Int32 weaponId = pLowestAmmoWeapon->GetId();
				assert(weaponId >= 0 && weaponId < NUM_WEAPONS);

				if(weaponId != -1)
				{
					const ammo_assocation_t& association = WPN_AMMO_ASSOCIATIONS[weaponId];
					if(association.pstrAmmoName[0] != '\0')
					{
						CBaseEntity* pEntity = CBaseEntity::CreateEntity(association.pstrAmmoName, center, spawnAngle, this);
						if(pEntity)
						{
							if(!pEntity->Spawn())
								Util::RemoveEntity(pEntity);
						}
					}
				}
			}
		}
	}

	// Add explosion if set
	if(m_explodeMagnitude > 0)
		CEnvExplosion::CreateEnvExplosion(center, spawnAngle, m_explodeMagnitude, true, nullptr);

	// Remove this entity
	Util::RemoveEntity(this);
}

//=============================================
// @brief
//
//=============================================
void CFuncBreakable::PlayDamageSound( void )
{
	if(m_dmgSoundsPattern == NO_STRING_VALUE)
		return;

	// Set pitch
	Int32 pitch;
	if(Common::RandomLong(0, 2))
		pitch = PITCH_NORM;
	else
		pitch = PITCH_NORM + Common::RandomLong(-5, 30);

	// Set volume
	Float volume = Common::RandomFloat(0.75, VOL_NORM);

	Util::PlayRandomEntitySound(this, gd_engfuncs.pfnGetString(m_dmgSoundsPattern), m_nbDmgSounds, SND_CHAN_VOICE, volume, ATTN_NORM, pitch);
}

//=============================================
// @brief
//
//=============================================
Int32 CFuncBreakable::GetExplosionMagnitude( void ) const
{
	return m_explodeMagnitude;
}

//=============================================
// @brief
//
//=============================================
void CFuncBreakable::SetExplosionMagnitude( Int32 magnitude )
{
	m_explodeMagnitude = magnitude;
}
