/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "funcdoor.h"
#include "funcbutton.h"

// Default speed for doors
const Float CFuncDoor::DEFAULT_SPEED = 100;

// Number of legacy door sounds
const Uint32 CFuncDoor::NUM_LEGACY_MOVE_SOUNDS = 12;
// Legacy door sounds
const Char* CFuncDoor::LEGACY_MOVE_SOUNDS[NUM_LEGACY_MOVE_SOUNDS] = 
{
	"common/null.wav",
	"doors/doormove1.wav",
	"doors/doormove2.wav",
	"doors/doormove3.wav",
	"doors/doormove4.wav",
	"doors/doormove5.wav",
	"doors/doormove6.wav",
	"doors/doormove7.wav",
	"doors/doormove8.wav",
	"doors/doormove9.wav",
	"doors/doormove10.wav",
	"doors/slidingopen1.wav"
};

// Number of legacy door sounds
const Uint32 CFuncDoor::NUM_LEGACY_STOP_SOUNDS = 10;
// Legacy move sounds
const Char* CFuncDoor::LEGACY_STOP_SOUNDS[NUM_LEGACY_STOP_SOUNDS] = 
{
	"common/null.wav",
	"doors/doorstop1.wav",
	"doors/doorstop2.wav",
	"doors/doorstop3.wav",
	"doors/doorstop4.wav",
	"doors/doorstop5.wav",
	"doors/doorstop6.wav",
	"doors/doorstop7.wav",
	"doors/doorstop8.wav",
	"doors/slidingclose1.wav",
};

// Wait time between locked sounds
const Float CFuncDoor::LOCKED_SOUND_DELAY = 4;

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(func_door, CFuncDoor);

//=============================================
// @brief
//
//=============================================
CFuncDoor::CFuncDoor( edict_t* pedict ):
	CToggleEntity(pedict),
	m_legacyMoveSound(0),
	m_legacyStopSound(0),
	m_legacyLockedSound(0),
	m_legacyUnlockedSound(0),
	m_forcedToClose(false),
	m_isBlocked(false),
	m_isSilent(false),
	m_nextLockedSoundTime(0),
	m_numSlaveDoors(0),
	m_numRelatedDoors(0)
{
	for(Uint32 i = 0; i < MAX_SLAVE_DOORS; i++)
		m_pSlaveDoors[i] = nullptr;
	
	for(Uint32 i = 0; i < MAX_RELATED_DOORS; i++)
		m_pRelatedDoors[i] = nullptr;
}

//=============================================
// @brief
//
//=============================================
CFuncDoor::~CFuncDoor( void )
{
}

//=============================================
// @brief
//
//=============================================
void CFuncDoor::DeclareSaveFields( void )
{
	// Call base class to do it first
	CToggleEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncDoor, m_forcedToClose, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncDoor, m_isBlocked, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncDoor, m_isSilent, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncDoor, m_activatorOrigin, EFIELD_COORD));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncDoor, m_nextLockedSoundTime, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD_ARRAY(CFuncDoor, m_pSlaveDoors, EFIELD_ENTPOINTER, MAX_SLAVE_DOORS));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncDoor, m_numSlaveDoors, EFIELD_UINT32));
	DeclareSaveField(DEFINE_DATA_FIELD_ARRAY(CFuncDoor, m_pRelatedDoors, EFIELD_ENTPOINTER, MAX_SLAVE_DOORS));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncDoor, m_numRelatedDoors, EFIELD_UINT32));
}

//=============================================
// @brief
//
//=============================================
bool CFuncDoor::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "movesnd"))
	{
		m_legacyMoveSound = SDL_atoi(kv.value);
		if(m_legacyMoveSound < 0 || m_legacyMoveSound >= (Int32)NUM_LEGACY_MOVE_SOUNDS)
		{
			Util::EntityConPrintf(m_pEdict, "Keyvalue '%s' for 'movesnd' is not a valid value.\n", kv.value);
			m_legacyMoveSound = 0;
		}
		return true;
	}
	else if(!qstrcmp(kv.keyname, "stopsnd"))
	{
		m_legacyStopSound = SDL_atoi(kv.value);
		if(m_legacyStopSound < 0 || m_legacyStopSound >= (Int32)NUM_LEGACY_STOP_SOUNDS)
		{
			Util::EntityConPrintf(m_pEdict, "Keyvalue '%s' for 'stopsnd' is not a valid value.\n", kv.value);
			m_legacyStopSound = 0;
		}
		return true;
	}
	else if(!qstrcmp(kv.keyname, "locked_sound"))
	{
		m_legacyLockedSound = SDL_atoi(kv.value);
		if(m_legacyLockedSound < 0 || m_legacyLockedSound >= (Int32)CFuncButton::NUM_LEGACY_BUTTON_SOUNDS)
		{
			Util::EntityConPrintf(m_pEdict, "Keyvalue '%s' for 'locked_sound' is not a valid value.\n", kv.value);
			m_legacyLockedSound = 0;
		}
		return true;
	}
	else if(!qstrcmp(kv.keyname, "locked_sound_custom"))
	{
		m_lockedSoundFile = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "unlocked_sound"))
	{
		m_legacyUnlockedSound = SDL_atoi(kv.value);
		if(m_legacyUnlockedSound < 0 || m_legacyUnlockedSound >= (Int32)CFuncButton::NUM_LEGACY_BUTTON_SOUNDS)
		{
			Util::EntityConPrintf(m_pEdict, "Keyvalue '%s' for 'unlocked_sound' is not a valid value.\n", kv.value);
			m_legacyUnlockedSound = 0;
		}
		return true;
	}
	else if(!qstrcmp(kv.keyname, "unlocked_sound_custom"))
	{
		m_unlockedSoundFile = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else
		return CToggleEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
bool CFuncDoor::Spawn( void )
{
	// Set movement sound if not set already to custom
	if(m_moveSoundFile == NO_STRING_VALUE)
	{
		assert(m_legacyMoveSound >= 0 && m_legacyMoveSound < (Int32)NUM_LEGACY_MOVE_SOUNDS);
		m_moveSoundFile = gd_engfuncs.pfnAllocString(LEGACY_MOVE_SOUNDS[m_legacyMoveSound]);
	}

	// Set stop sound if not set already to custom
	if(m_stopSoundFile == NO_STRING_VALUE)
	{
		assert(m_legacyStopSound >= 0 && m_legacyStopSound < (Int32)NUM_LEGACY_STOP_SOUNDS);
		m_stopSoundFile = gd_engfuncs.pfnAllocString(LEGACY_STOP_SOUNDS[m_legacyStopSound]);
	}

	// Set locked sound if not set already to custom
	if(m_lockedSoundFile == NO_STRING_VALUE)
	{
		assert(m_legacyLockedSound >= 0 && m_legacyLockedSound < (Int32)CFuncButton::NUM_LEGACY_BUTTON_SOUNDS);
		m_lockedSoundFile = gd_engfuncs.pfnAllocString(CFuncButton::LEGACY_BUTTON_SOUNDS[m_legacyLockedSound]);
	}

	// Set unlocked sound if not set already to custom
	if(m_unlockedSoundFile == NO_STRING_VALUE)
	{
		assert(m_legacyUnlockedSound >= 0 && m_legacyUnlockedSound < (Int32)CFuncButton::NUM_LEGACY_BUTTON_SOUNDS);
		m_unlockedSoundFile = gd_engfuncs.pfnAllocString(CFuncButton::LEGACY_BUTTON_SOUNDS[m_legacyUnlockedSound]);
	}

	// This function will eventually precache the sounds
	if(!CToggleEntity::Spawn())
		return false;

	// Set solidity/skin using shared function
	SetSpawnProperties();
	
	// Set movetype
	m_pState->movetype = MOVETYPE_PUSH;

	if(!SetModel(m_pFields->modelname))
		return false;

	if(m_pState->speed <= 0)
		m_pState->speed = DEFAULT_SPEED;

	SetMovementVectors();

	// Set toggle state
	m_toggleState = TS_AT_BOTTOM;
	m_isBlocked = false;

	// Set as nodraw if set
	if(HasSpawnFlag(FL_NODRAW))
		m_pState->effects |= EF_NODRAW;

	// Set touch function if not use only
	if(!HasSpawnFlag(FL_USE_ONLY))
		SetTouch(&CFuncDoor::DoorTouch);

	// Check for slave doors to be initialized on
	// non-zero origin door entities
	if(m_pFields->targetname != NO_STRING_VALUE)
		m_pState->flags |= FL_INITIALIZE;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CFuncDoor::SetMovementVectors( void )
{
	// Set the two positions
	m_position1 = m_pState->origin;

	// -2 is because the mins/maxs is padded by 1 on both sides
	static const Float correction = 0;
	Vector offset;
	offset = (m_pState->movedir 
		* (fabs( m_pState->movedir.x * (m_pState->size.x-correction) ) 
		+ fabs( m_pState->movedir.y * (m_pState->size.y-correction) ) 
		+ fabs( m_pState->movedir.z * (m_pState->size.z-correction) ) - m_lip));
	m_position2 = m_position1 + offset;

	// Swap origins if starting open
	if(HasSpawnFlag(FL_START_OPEN))
	{
		gd_engfuncs.pfnSetOrigin(m_pEdict, m_position2);
		m_position2 = m_position1;
		m_position1 = m_pState->origin;
	}
}

//=============================================
// @brief
//
//=============================================
void CFuncDoor::SetSpawnProperties( void )
{
	// Set move direction
	Util::SetMoveDirection(*m_pState);

	if(HasSpawnFlag(FL_NOT_SOLID))
		m_pState->solid = SOLID_NOT;
	else
		m_pState->solid = SOLID_BSP;
}

//=============================================
// @brief
//
//=============================================
void CFuncDoor::InitEntity( void )
{
	const Char* pstrClassName = gd_engfuncs.pfnGetString(m_pFields->classname);
	const Char* pstrTargetName = gd_engfuncs.pfnGetString(m_pFields->targetname);
	if(!pstrTargetName || !qstrlen(pstrTargetName))
		return;

	// Find slave doors with same position and name if set to touch opens
	if(HasSpawnFlag(FL_TOUCH_OPENS) && !m_pState->origin.IsZero())
	{
		edict_t* pedict = nullptr;
		while(true)
		{
			pedict = Util::FindEntityByTargetName(pedict, pstrTargetName);
			if(!pedict)
				break;

			if(Util::IsNullEntity(pedict) || pedict == m_pEdict)
				continue;

			CBaseEntity* pOther = CBaseEntity::GetClass(pedict);
			if(!pOther)
				continue;

			// Check for exact classname
			if(qstrcmp(pstrClassName, pOther->GetClassName()))
				continue;

			// Set this as the parent door
			if(pOther->IsFuncDoorEntity() && Math::VectorCompare(pOther->GetOrigin(), m_pState->origin))
			{
				if(m_numSlaveDoors >= MAX_SLAVE_DOORS)
					break;

				m_pSlaveDoors[m_numSlaveDoors] = reinterpret_cast<CFuncDoor*>(pOther);
				m_numSlaveDoors++;

				pOther->SetParentDoor(this);
			}
		}
	}

	// Find related doors that might not share the same origin
	edict_t* pedict = nullptr;
	while(true)
	{
		pedict = Util::FindEntityByTargetName(pedict, pstrTargetName);
		if(!pedict)
			break;

		if(Util::IsNullEntity(pedict) || pedict == m_pEdict)
			continue;

		CBaseEntity* pOther = CBaseEntity::GetClass(pedict);
		if(!pOther || !pOther->IsFuncDoorEntity())
			continue;

		// Check for exact classname
		if(qstrcmp(pstrClassName, pOther->GetClassName()))
			continue;

		// Check that it's not a slave door
		Uint32 j = 0;
		for(; j < m_numSlaveDoors; j++)
		{
			if(m_pSlaveDoors[j] == pOther)
				break;
		}

		if(j != m_numSlaveDoors)
			continue;

		if(m_numRelatedDoors == MAX_RELATED_DOORS)
		{
			Util::EntityConPrintf(m_pEdict, "Exceeded MAX_RELATED_DOORS.\n");
			break;
		}

		// Add as related door
		m_pRelatedDoors[m_numRelatedDoors] = reinterpret_cast<CFuncDoor*>(pOther);
		m_numRelatedDoors++;
	}
}

//=============================================
// @brief
//
//=============================================
void CFuncDoor::SetParentDoor( CFuncDoor* pParent )
{
	if(!pParent)
		return;

	// Use existing parenting for this
	SetParent(pParent);

	// Set to track angles of parent
	m_pState->effects |= EF_TRACKANGLES;

	// Disable block and touch functions
	SetTouch(nullptr);
	SetBlocked(nullptr);
}

//=============================================
// @brief
//
//=============================================
void CFuncDoor::RealignRelatedDoor( CFuncDoor* pDoor )
{
	pDoor->SetOrigin(m_pState->origin);
	pDoor->SetVelocity(ZERO_VECTOR);
}

//=============================================
// @brief
//
//=============================================
void CFuncDoor::CallBlocked( CBaseEntity* pOther )
{
	// If parented, don't bother
	if(m_pState->parent != NO_ENTITY_INDEX)
		return;

	// Set this so we don't try to check for npcs when going back
	m_isBlocked = true;

	// Stop sound if not silent
	if(!m_isSilent)
		Util::EmitEntitySound(this, m_moveSoundFile, SND_CHAN_BODY, 0, 0, 0, SND_FL_STOP);

	// Deal damage
	if(m_damageDealt && !m_forcedToClose)
		pOther->TakeDamage(this, this, m_damageDealt, DMG_CRUSH);
	else if(m_forcedToClose)
		pOther->TakeDamage(this, this, 100, DMG_CRUSH);
		
	// Don't return of wait is null, or forced to close
	if(m_waitTime != -1 && !m_forcedToClose)
	{
		if(m_toggleState == TS_GOING_DOWN)
			GoUp();
		else
			GoDown();
	}

	// Reset related pieces
	for(Uint32 i = 0; i < m_numRelatedDoors; i++)
	{
		CFuncDoor* pRelatedDoor = m_pRelatedDoors[i];
		if(!pRelatedDoor)
			continue;

		// Realign related door
		if(pRelatedDoor->GetDelay() >= 0)
		{
			// Realign the door if velocities match
			if(Math::VectorCompare(pOther->GetVelocity(), m_pState->velocity) && Math::VectorCompare(pOther->GetAngularVelocity(), m_pState->avelocity))
				RealignRelatedDoor(pRelatedDoor);

			if(pRelatedDoor->GetToggleState() == TS_GOING_DOWN)
				pRelatedDoor->GoUp();
			else
				pRelatedDoor->GoDown();
		}
	}
}

//=============================================
// @brief
//
//=============================================
Int32 CFuncDoor::GetEntityFlags( void )
{
	Int32 flags = (CToggleEntity::GetFlags() & ~FL_ENTITY_TRANSITION);
	if(HasSpawnFlag(FL_USE_ONLY))
		flags |= FL_ENTITY_PLAYER_USABLE;

	return flags;
}

//=============================================
// @brief
//
//=============================================
void CFuncDoor::SetToggleState( togglestate_t state, bool reverse )
{
	Vector setPosition;
	if(state == TS_AT_TOP)
		setPosition = m_position2;
	else
		setPosition = m_position1;

	gd_engfuncs.pfnSetOrigin(m_pEdict, setPosition);
	m_toggleState = state;
}

//=============================================
// @brief
//
//=============================================
void CFuncDoor::SetForcedClose( void )
{
	// Don't bother with closed or closing doors
	if(m_toggleState == TS_AT_BOTTOM || m_toggleState == TS_GOING_DOWN)
		return;

	// Remember so we don't allow overrides while closing
	m_forcedToClose = true;

	// Force it to close
	GoDown();
}

//=============================================
// @brief
//
//=============================================
void CFuncDoor::DoorBeginMoveUp( void )
{
	// Simple linear movement
	LinearMove(m_position2, m_pState->speed);
}

//=============================================
// @brief
//
//=============================================
void CFuncDoor::DoorBeginMoveDown( void )
{
	// Simple linear movement
	LinearMove(m_position1, m_pState->speed);
}

//=============================================
// @brief
//
//=============================================
bool CFuncDoor::DoorActivate( void )
{
	// Only check master when not forced to close
	if(!m_forcedToClose)
	{
		if(IsLockedByMaster())
			return false;
	}

	// Don't return if we're moving
	if(!HasSpawnFlag(FL_NO_AUTO_RETURN) && m_toggleState != TS_AT_BOTTOM 
		|| HasSpawnFlag(FL_NO_AUTO_RETURN) && m_toggleState != TS_AT_TOP 
		&& m_toggleState != TS_AT_BOTTOM)
		return false;

	// Remember activator's origin for correction
	if(m_activator && (m_activator->IsPlayer() || m_activator->IsNPC()))
		m_activatorOrigin = m_activator->GetOrigin();

	// Reset blocked state
	m_isBlocked = false;

	if(HasSpawnFlag(FL_NO_AUTO_RETURN) && m_toggleState == TS_AT_TOP)
	{
		// Close the door
		GoDown();
	}
	else
	{
		// Play unlock sound, then reset it
		if(m_unlockedSoundFile != NO_STRING_VALUE)
		{
			PlayLockSounds(false, false, LOCKED_SOUND_DELAY, m_nextLockedSoundTime);
			m_unlockedSoundFile = NO_STRING_VALUE;
		}

		GoUp();
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CFuncDoor::ShouldAutoCloseDoor( void )
{
	if(m_pFields->targetname != NO_STRING_VALUE && !HasSpawnFlag(FL_TOUCH_OPENS))
		return true;

	if(m_isBlocked || m_forcedToClose || HasSpawnFlag(FL_NO_PROXIMITY_CHECKS))
		return true;

	Float size = m_pState->size.x > m_pState->size.y ? m_pState->size.x : m_pState->size.y;
	Vector center = (m_pState->absmin + m_pState->absmax)*0.5;

	Vector mins, maxs;
	for(Uint32 i = 0; i < 3; i++)
	{
		mins[i] = center[i] - size;
		maxs[i] = center[i] + size;
	}

	edict_t* pEdict = nullptr;
	while(true)
	{
		pEdict = Util::FindEntityInBBox(pEdict, mins, maxs);
		if(!pEdict)
			break;

		if(!Util::IsNullEntity(pEdict))
		{
			CBaseEntity* pEntity = CBaseEntity::GetClass(pEdict);
			if(!pEntity)
				continue;

			if(pEntity->IsAlive() && pEntity->IsNPC() || pEntity->IsPlayer())
				return false;
		}
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CFuncDoor::GoUp( void )
{
	// Set toggle-state
	m_toggleState = TS_GOING_UP;

	// Play sound if not silent
	if(!m_isSilent)
		Util::EmitEntitySound(this, m_moveSoundFile, SND_CHAN_BODY, VOL_NORM, ATTN_NORM, PITCH_NORM, SND_FL_OCCLUSIONLESS);

	// Set move done function
	SetMoveDone(&CFuncDoor::HitTop);

	// Begin movement
	DoorBeginMoveUp();
}

//=============================================
// @brief
//
//=============================================
void CFuncDoor::GoDown( void )
{
	// Don't close yet if we have a npc or the player enarby
	if(!ShouldAutoCloseDoor())
	{
		SetThink(&CFuncDoor::GoDown);
		m_pState->nextthink = m_pState->ltime + m_waitTime;
		return;
	}

	// Play sound if not silent
	if(!m_isSilent)
		Util::EmitEntitySound(this, m_moveSoundFile, SND_CHAN_BODY, VOL_NORM, ATTN_NORM, PITCH_NORM, SND_FL_OCCLUSIONLESS);

	// Begin moving down
	m_toggleState = TS_GOING_DOWN;

	// Go and hit rock bottom
	SetMoveDone(&CFuncDoor::HitBottom);
	DoorBeginMoveDown();
}

//=============================================
// @brief
//
//=============================================
void CFuncDoor::HitTop( void )
{
	// Reset these
	m_forcedToClose = false;
	m_isBlocked = false;

	// Set toggle state
	m_toggleState = TS_AT_TOP;

	if(!m_isSilent)
	{
		// Play sound if not silent
		Util::EmitEntitySound(this, m_moveSoundFile, SND_CHAN_BODY, 0, 0, 0, SND_FL_STOP);
		Util::EmitEntitySound(this, m_stopSoundFile, SND_CHAN_BODY, VOL_NORM, ATTN_NORM, PITCH_NORM, SND_FL_OCCLUSIONLESS);
	}

	if(HasSpawnFlag(FL_NO_AUTO_RETURN) && !HasSpawnFlag(FL_USE_ONLY))
	{
		// Re-set touch function
		SetTouch(&CFuncDoor::DoorTouch);
	}
	else if(!HasSpawnFlag(FL_NO_AUTO_RETURN))
	{
		if(m_waitTime == -1)
		{
			// Don't move again
			SetThink(nullptr);
			m_pState->nextthink = 0;
		}
		else
		{
			// Go down after a delay
			SetThink(&CFuncDoor::GoDown);
			m_pState->nextthink = m_pState->ltime + m_waitTime;
		}
	}

	// Trigger any targets
	UseTargets(m_activator, USE_TOGGLE, 0);

	// Fire the close target if we're set to "starts open"
	if(m_pFields->netname != NO_STRING_VALUE && HasSpawnFlag(FL_START_OPEN))
		Util::FireTargets(gd_engfuncs.pfnGetString(m_pFields->netname), m_activator, this, USE_TOGGLE, 0);

	// Fix ground entities
	Util::FixGroundEntities(this);
}

//=============================================
// @brief
//
//=============================================
void CFuncDoor::HitBottom( void )
{
	// Reset these
	m_forcedToClose = false;
	m_isBlocked = false;

	// Set toggle state
	m_toggleState = TS_AT_BOTTOM;

	if(!m_isSilent)
	{
		// Play sound if not silent
		Util::EmitEntitySound(this, m_moveSoundFile, SND_CHAN_BODY, 0, 0, 0, SND_FL_STOP);
		Util::EmitEntitySound(this, m_stopSoundFile, SND_CHAN_BODY, VOL_NORM, ATTN_NORM, PITCH_NORM, SND_FL_OCCLUSIONLESS);
	}

	// Re-instate touch function
	if(!HasSpawnFlag(FL_USE_ONLY))
		SetTouch(&CFuncDoor::DoorTouch);

	// Trigger any targets
	UseTargets(m_activator, USE_TOGGLE, 0);

	// Fire the close target
	if(m_pFields->netname != NO_STRING_VALUE && !HasSpawnFlag(FL_START_OPEN))
		Util::FireTargets(gd_engfuncs.pfnGetString(m_pFields->netname), m_activator, this, USE_TOGGLE, 0);

	// Fix ground entities
	Util::FixGroundEntities(this);
}

//=============================================
// @brief
//
//=============================================
void CFuncDoor::CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	// If we're parented, don't react to use function
	if(m_pState->parent != NO_ENTITY_INDEX)
		return;

	// Remember activator
	m_activator = pActivator;

	// If not ready to be used, ignore
	if(m_toggleState == TS_AT_BOTTOM || HasSpawnFlag(FL_NO_AUTO_RETURN) && m_toggleState == TS_AT_TOP)
		DoorActivate();
}

//=============================================
// @brief
//
//=============================================
void CFuncDoor::DoorTouch( CBaseEntity* pOther )
{
	// Ignore anything but player
	if(!pOther->IsPlayer())
		return;

	// Don't open is locked by master
	if(IsLockedByMaster() || m_pFields->targetname != NO_STRING_VALUE && !HasSpawnFlag(FL_TOUCH_OPENS))
	{
		if(m_toggleState == TS_AT_BOTTOM)
			PlayLockSounds(true, false, LOCKED_SOUND_DELAY, m_nextLockedSoundTime);

		// Don't do anything
		return;
	}

	// Set activator
	m_activator = pOther;

	// Activate the door
	if(DoorActivate())
	{
		// Trigger slave doors
		for(Uint32 i = 0; i < m_numSlaveDoors; i++)
			m_pSlaveDoors[i]->CallUse(m_activator, m_activator, USE_TOGGLE, 0);

		// Disable touch function util done
		SetTouch(nullptr);
	}
}

//=============================================
// @brief
//
//=============================================
void CFuncDoor::SendInitMessage( const CBaseEntity* pPlayer )
{
	if(m_toggleState == TS_GOING_UP || m_toggleState == TS_GOING_DOWN)
		Util::EmitEntitySound(this, m_moveSoundFile, SND_CHAN_BODY, VOL_NORM, ATTN_NORM, PITCH_NORM, SND_FL_OCCLUSIONLESS);
}

//=============================================
// @brief
//
//=============================================
void CFuncDoor::ChildEntityRemoved( CBaseEntity* pEntity )
{
	for(Uint32 i = 0; i < m_numSlaveDoors; i++)
	{
		if(m_pSlaveDoors[i] == pEntity)
		{
			for(Uint32 j = i; j < (m_numSlaveDoors-1); j++)
				m_pSlaveDoors[j] = m_pSlaveDoors[j+1];

			m_numSlaveDoors--;
			i--;
		}
	}

	for(Uint32 i = 0; i < m_numRelatedDoors; i++)
	{
		if(m_pRelatedDoors[i] == pEntity)
		{
			for(Uint32 j = i; j < (m_numRelatedDoors-1); j++)
				m_pRelatedDoors[j] = m_pRelatedDoors[j+1];

			m_numRelatedDoors--;
			i--;
		}
	}
}

//=============================================
// @brief
//
//=============================================
usableobject_type_t CFuncDoor::GetUsableObjectType( void )
{ 
	if(IsLockedByMaster())
		return USABLE_OBJECT_LOCKED;
	else
		return USABLE_OBJECT_DEFAULT;
}