/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "funcbutton.h"
#include "funcdoor.h"

// Locked sound wait time
const Float CFuncButton::LOCKED_SOUND_WAIT_TIME = 0.5;
// Default button speed
const Float CFuncButton::DEFAULT_SPEED = 40;
// Default button wait time
const Float CFuncButton::DEFAULT_WAIT_TIME = 1.0;
// Default button lip value
const Float CFuncButton::DEFAULT_LIP_VALUE = 4.0;
// Legacy button sound count
const Uint32 CFuncButton::NUM_LEGACY_BUTTON_SOUNDS = 27;
// Legacy button sounds
const Char* CFuncButton::LEGACY_BUTTON_SOUNDS[NUM_LEGACY_BUTTON_SOUNDS] = 
{
	"common/null.wav",
	"buttons/button1.wav",
	"buttons/button2.wav", 
	"buttons/button3.wav", 
	"buttons/button4.wav", 
	"buttons/button5.wav", 
	"buttons/button6.wav", 
	"buttons/button7.wav", 
	"buttons/button8.wav", 
	"buttons/button9.wav", 
	"buttons/button10.wav", 
	"buttons/button11.wav", 
	"buttons/latchlocked1.wav",
	"buttons/latchunlocked1.wav",
	"buttons/lightswitch2.wav",
	"common/null.wav",
	"common/null.wav",
	"common/null.wav",
	"common/null.wav",
	"common/null.wav",
	"common/null.wav",
	"buttons/lever1.wav",
	"buttons/lever2.wav",
	"buttons/lever3.wav",
	"buttons/lever4.wav",
	"buttons/lever5.wav",
	"buttons/button9.wav"
};

// Link the entity to it's class
LINK_ENTITY_TO_CLASS(func_button, CFuncButton);

//=============================================
// @brief
//
//=============================================
CFuncButton::CFuncButton( edict_t* pedict ):
	CToggleEntity(pedict),
	m_stayPushed(false),
	m_changeTargetName(NO_STRING_VALUE),
	m_lockedTriggerTarget(NO_STRING_VALUE),
	m_pairButtonName(NO_STRING_VALUE),
	m_legacyUseSound(0),
	m_legacyLockedSound(0),
	m_legacyUnlockedSound(0),
	m_useSoundFile(NO_STRING_VALUE),
	m_nextLockSoundTime(0),
	m_nextUsableTime(0)
{
}

//=============================================
// @brief
//
//=============================================
CFuncButton::~CFuncButton( void )
{
}

//=============================================
// @brief
//
//=============================================
void CFuncButton::DeclareSaveFields( void )
{
	// Call base class to do it first
	CToggleEntity::DeclareSaveFields();
	
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncButton, m_stayPushed, EFIELD_BOOLEAN));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncButton, m_changeTargetName, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncButton, m_lockedTriggerTarget, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncButton, m_pairButtonName, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncButton, m_useSoundFile, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CFuncButton, m_nextUsableTime, EFIELD_TIME));
}

//=============================================
// @brief
//
//=============================================
bool CFuncButton::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "changetarget"))
	{
		m_changeTargetName = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "lockedtarget"))
	{
		m_lockedTriggerTarget = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "pairbutton"))
	{
		m_pairButtonName = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "locked_sound"))
	{
		m_legacyLockedSound = SDL_atoi(kv.value);
		if(m_legacyUnlockedSound < 0 || m_legacyUnlockedSound >= NUM_LEGACY_BUTTON_SOUNDS)
		{
			Util::EntityConPrintf(m_pEdict, "Keyvalue for 'locked_sound' is not a valid value.\n");
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
		if(m_legacyUnlockedSound < 0 || m_legacyUnlockedSound >= NUM_LEGACY_BUTTON_SOUNDS)
		{
			Util::EntityConPrintf(m_pEdict, "Keyvalue for 'unlocked_sound' is not a valid value.\n");
			m_legacyUnlockedSound = 0;
		}
		return true;
	}
	else if(!qstrcmp(kv.keyname, "unlocked_sound_custom"))
	{
		m_unlockedSoundFile = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "sounds"))
	{
		m_legacyUseSound = SDL_atoi(kv.value);
		if(m_legacyUseSound < 0 || m_legacyUseSound >= NUM_LEGACY_BUTTON_SOUNDS)
		{
			Util::EntityConPrintf(m_pEdict, "Keyvalue for 'sounds' is not a valid value.\n");
			m_legacyUseSound = 0;
		}
		return true;
	}
	else if(!qstrcmp(kv.keyname, "sound_custom"))
	{
		m_useSoundFile = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else
		return CToggleEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
void CFuncButton::Precache( void )
{
	if(HasSpawnFlag(FL_SPARK_WHEN_OFF))
		Util::PrecacheFixedNbSounds("misc/spark%d.wav", 6);

	if(m_useSoundFile != NO_STRING_VALUE)
		gd_engfuncs.pfnPrecacheSound(gd_engfuncs.pfnGetString(m_useSoundFile));
}

//=============================================
// @brief
//
//=============================================
bool CFuncButton::Spawn( void )
{
	// Set stop sound if not set already to custom
	if(m_useSoundFile == NO_STRING_VALUE)
	{
		assert(m_legacyUseSound >= 0 && m_legacyUseSound < NUM_LEGACY_BUTTON_SOUNDS);
		m_useSoundFile = gd_engfuncs.pfnAllocString(LEGACY_BUTTON_SOUNDS[m_legacyUseSound]);
	}

	// Set locked sound if not set already to custom
	if(m_lockedSoundFile == NO_STRING_VALUE)
	{
		assert(m_legacyLockedSound >= 0 && m_legacyLockedSound < NUM_LEGACY_BUTTON_SOUNDS);
		m_lockedSoundFile = gd_engfuncs.pfnAllocString(LEGACY_BUTTON_SOUNDS[m_legacyLockedSound]);
	}

	// Set unlocked sound if not set already to custom
	if(m_unlockedSoundFile == NO_STRING_VALUE)
	{
		assert(m_legacyUnlockedSound >= 0 && m_legacyUnlockedSound < NUM_LEGACY_BUTTON_SOUNDS);
		m_unlockedSoundFile = gd_engfuncs.pfnAllocString(LEGACY_BUTTON_SOUNDS[m_legacyUnlockedSound]);
	}

	if(!CToggleEntity::Spawn())
		return false;

	if(!SetModel(m_pFields->modelname))
		return false;

	m_pState->movetype = MOVETYPE_PUSH;

	if(!m_pState->speed)
		m_pState->speed = DEFAULT_SPEED;

	if(!m_waitTime)
		m_waitTime = DEFAULT_WAIT_TIME;

	if(!m_lip)
		m_lip = DEFAULT_LIP_VALUE;

	m_toggleState = TS_AT_BOTTOM;

	// Set movement vectors
	SetSpawnProperties();

	// Check if the button is to stay pushed
	m_stayPushed = (m_waitTime == -1) ? true : false;

	if(HasSpawnFlag(FL_TOUCH_ONLY))
	{
		// Only touch can activate this button
		SetTouch(&CFuncButton::ButtonTouch);
	}
	else
	{
		// Activted by impulse use
		SetUse(&CFuncButton::ButtonUse);
	}

	// Hide button if set to do so
	if(HasSpawnFlag(FL_NODRAW))
		m_pState->effects |= EF_NODRAW;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CFuncButton::ButtonUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value )
{
	// Don't toggle if moving
	if(m_toggleState == TS_GOING_UP || m_toggleState == TS_GOING_DOWN)
		return;

	// Check if we're a paired button that got disabled
	if(m_nextUsableTime > g_pGameVars->time)
		return;

	m_activator = pActivator;

	if(m_toggleState == TS_AT_TOP)
	{
		if(!m_stayPushed && HasSpawnFlag(FL_TOGGLE))
		{
			Util::EmitEntitySound(this, m_useSoundFile, SND_CHAN_BODY);
			ReturnThink();
			SetDelayOnPairs();
		}
	}
	else
	{
		// Activate the button
		ButtonActivate();
	}
}

//=============================================
// @brief
//
//=============================================
void CFuncButton::SetDelayOnPairs( void )
{
	if(m_pairButtonName == NO_STRING_VALUE)
		return;

	const char* pstrPairName = gd_engfuncs.pfnGetString(m_pairButtonName);
	if(!pstrPairName || !qstrlen(pstrPairName))
		return;

	Float waitTime = m_waitTime;
	if(HasSpawnFlag(FL_MOVE) && !m_stayPushed)
	{
		Float travelTime = (m_position1 - m_position2).Length()/m_pState->speed;
		waitTime += travelTime;
	}

	edict_t* pedict = nullptr;
	while(true)
	{
		pedict = Util::FindEntityByTargetName(pedict, pstrPairName);
		if(!pedict)
			break;

		if(pedict == m_pEdict)
			continue;

		CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
		if(!pEntity || !pEntity->IsFuncButtonEntity())
			continue;

		pEntity->SetPairedButtonDelay(waitTime);
	}
}

//=============================================
// @brief
//
//=============================================
void CFuncButton::SetPairedButtonDelay( Float delayTime )
{
	if(m_stayPushed)
		m_toggleState = TS_AT_TOP;
	else
		m_nextUsableTime = g_pGameVars->time + delayTime;
}

//=============================================
// @brief
//
//=============================================
void CFuncButton::ButtonTouch( CBaseEntity* pOther )
{
	// Only players
	if(!pOther->IsPlayer())
		return;

	// Check if we're a paired button that got disabled
	if(m_nextUsableTime > g_pGameVars->time)
		return;

	m_activator = pOther;

	// Get response code
	buttoncode_t code = GetResponseToTouch();
	if(code == BUTTON_CODE_NONE)
		return;

	// Check for master
	if(IsLockedByMaster())
	{
		PlayLockSounds(true, true, LOCKED_SOUND_WAIT_TIME, m_nextLockSoundTime);
		return;
	}

	// Disable touching for a while
	SetTouch(nullptr);

	if(code == BUTTON_CODE_RETURN)
	{
		Util::EmitEntitySound(this, m_useSoundFile, SND_CHAN_BODY);
		UseTargets(m_activator, USE_TOGGLE, 0);
		
		// Go back
		ReturnThink();
	}
	else
	{
		// Activate the button
		ButtonActivate();
	}
}

//=============================================
// @brief
//
//=============================================
void CFuncButton::SetSpawnProperties( void )
{
	// Set movement direction
	Util::SetMoveDirection(*m_pState);

	m_pState->solid = SOLID_BSP;

	// Set base position
	m_position1 = m_pState->origin;

	// -2 is because the mins/maxs is padded by 1 on both sides
	static const Float correction = 0;
	m_position2 = m_position1 + (
		m_pState->movedir * (fabs( m_pState->movedir.x * (m_pState->size.x-correction) ) 
		+ fabs( m_pState->movedir.y * (m_pState->size.y-correction) ) 
		+ fabs( m_pState->movedir.z * (m_pState->size.z-correction) ) - m_lip));

	// Check if this button is moving at all
	if((m_position2-m_position1).Length() < 1.0f || !HasSpawnFlag(FL_MOVE))
		m_position2 = m_position1;

	// Spark if set to do so
	if(HasSpawnFlag(FL_SPARK_WHEN_OFF))
	{
		SetThink(&CFuncButton::SparkThink);
		m_pState->nextthink = g_pGameVars->time + 0.5;
	}
}

//=============================================
// @brief
//
//=============================================
void CFuncButton::SparkThink( void )
{
	SetThink(&CFuncButton::SparkThink);
	m_pState->nextthink = g_pGameVars->time + (0.1+Common::RandomFloat(0, 1.5));

	CString soundfile;
	soundfile << "misc/spark" << (Int32)Common::RandomLong(1, 6) << ".wav";

	Float volume = Common::RandomFloat(0.1, 0.6);
	Util::EmitAmbientSound(m_pState->origin, soundfile.c_str(), volume);

	Vector position = (m_pState->absmin+m_pState->absmax)*0.5;
	Util::CreateSparks(position);
}

//=============================================
// @brief
//
//=============================================
void CFuncButton::TriggerAndWait( void )
{
	// Complain about state if not valid
	if(m_toggleState != TS_GOING_UP)
	{
		Util::EntityConPrintf(m_pEdict, "Expected to be going up, but state is %d.\n", m_toggleState);
		return;
	}

	// Check for master lock
	if(IsLockedByMaster())
		return;

	m_toggleState = TS_AT_TOP;

	// If button is to stay pushed, then reset accordingly
	if(m_stayPushed || HasSpawnFlag(FL_TOGGLE))
	{
		if(!HasSpawnFlag(FL_TOUCH_ONLY))
			SetTouch(nullptr);
		else
			SetTouch(&CFuncButton::ButtonTouch);
	}
	else
	{
		SetThink(&CFuncButton::ReturnThink);
		m_pState->nextthink = m_pState->ltime + m_waitTime;
	}

	// Set frame
	m_pState->frame = 1;

	// Trigger targets
	UseTargets(m_activator, USE_TOGGLE, 0);
}

//=============================================
// @brief
//
//=============================================
void CFuncButton::ReturnThink( void )
{
	// Complain about state if not valid
 	if(m_toggleState != TS_AT_TOP)
	{
		Util::EntityConPrintf(m_pEdict, "Expected to be at top, but state is %d.\n", m_toggleState);
		return;
	}

	m_toggleState = TS_GOING_DOWN;

	SetMoveDone(&CFuncButton::ReturnBack);
	BeginReturnMovement();

	// Reset frame
	m_pState->frame = 0;
}

//=============================================
// @brief
//
//=============================================
void CFuncButton::ReturnBack( void )
{
	// Complain about state if not valid
 	if(m_toggleState != TS_GOING_DOWN)
	{
		Util::EntityConPrintf(m_pEdict, "Expected to be going up, but state is %d.\n", m_toggleState);
		return;
	}

	// Trigger target if toggle
	if(HasSpawnFlag(FL_TOGGLE))
		UseTargets(m_activator, USE_TOGGLE, 0);
	
	// Re-trigger multisources(why does HL need this?)
	if(m_pFields->target != NO_STRING_VALUE)
	{
		const Char* pstrTarget = gd_engfuncs.pfnGetString(m_pFields->target);
		if(pstrTarget && qstrlen(pstrTarget) > 0)
		{
			edict_t* pedict = nullptr;
			while(true)
			{
				pedict = Util::FindEntityByTargetName(pedict, pstrTarget);
				if(!pedict)
					break;

				if(Util::IsNullEntity(pedict) || qstrcmp("multisource", gd_engfuncs.pfnGetString(pedict->fields.targetname)))
					continue;

				CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
				if(pEntity)
					pEntity->CallUse(m_activator, this, USE_TOGGLE, 0);
			}
		}
	}

	// Re-instate touch method
	if(!HasSpawnFlag(FL_TOUCH_ONLY))
		SetTouch(nullptr);
	else
		SetTouch(&CFuncButton::ButtonTouch);

	// Re-instate sparks if set
	SetReturnBackSparking();

	// Set toggle state
	m_toggleState = TS_AT_BOTTOM;
}

//=============================================
// @brief
//
//=============================================
void CFuncButton::SetReturnBackSparking( void )
{
	if(HasSpawnFlag(FL_SPARK_WHEN_OFF))
	{
		SetThink(&CFuncButton::SparkThink);
		m_pState->nextthink = g_pGameVars->time + 0.5;
	}
	else
	{
		SetThink(nullptr);
		m_pState->nextthink = 0;
	}
}

//=============================================
// @brief
//
//=============================================
void CFuncButton::ButtonActivate( void )
{
	// Check if button is locked
	if(IsLockedByMaster())
	{
		// Play locked sound
		PlayLockSounds(true, true, LOCKED_SOUND_WAIT_TIME, m_nextLockSoundTime);
		
		// Trigger locked target if set
		if(m_lockedTriggerTarget != NO_STRING_VALUE)
			Util::FireTargets(gd_engfuncs.pfnGetString(m_lockedTriggerTarget), m_activator, this, USE_TOGGLE, 0);

		return;
	}
	else if(m_unlockedSoundFile != NO_STRING_VALUE)
	{
		// Play unlocked sound
		PlayLockSounds(false, true, LOCKED_SOUND_WAIT_TIME, m_nextLockSoundTime);
		m_unlockedSoundFile = NO_STRING_VALUE;
	}

	// Play the sound
	Util::EmitEntitySound(this, m_useSoundFile, SND_CHAN_BODY);

	if(m_toggleState != TS_AT_BOTTOM)
	{
		Util::EntityConPrintf(m_pEdict, "Expected to be at bottom, but state is %d.\n", m_toggleState);
		return;
	}

	m_toggleState = TS_GOING_UP;

	// Set move done function
	SetMoveDone(&CFuncButton::TriggerAndWait);

	// Begin moving out
	BeginPressedMovement();

	// Disable any paired func_buttons
	SetDelayOnPairs();
}

//=============================================
// @brief
//
//=============================================
void CFuncButton::BeginPressedMovement( void )
{
	LinearMove(m_position2, m_pState->speed);
}

//=============================================
// @brief
//
//=============================================
void CFuncButton::BeginReturnMovement( void )
{
	LinearMove(m_position1, m_pState->speed);
}

//=============================================
// @brief
//
//=============================================
CFuncButton::buttoncode_t CFuncButton::GetResponseToTouch( void )
{
	if(m_toggleState == TS_GOING_UP || m_toggleState == TS_GOING_DOWN
		|| (m_toggleState == TS_AT_TOP && !m_stayPushed && !HasSpawnFlag(FL_TOGGLE)))
		return BUTTON_CODE_NONE;

	if(m_toggleState == TS_AT_TOP)
	{
		if(HasSpawnFlag(FL_TOGGLE) && !m_stayPushed)
			return BUTTON_CODE_RETURN;
	}
	else
	{
		// Activate otherwise
		return BUTTON_CODE_ACTIVATE;
	}

	return BUTTON_CODE_NONE;
}

//=============================================
// @brief
//
//=============================================
usableobject_type_t CFuncButton::GetUsableObjectType( void )
{
	bool hasValidTarget = false;
	if(m_pFields->target != NO_STRING_VALUE)
	{
		const Char* pstrTarget = gd_engfuncs.pfnGetString(m_pFields->target);
		if(qstrcicmp(pstrTarget, "null"))
		{
			edict_t* pedict = Util::FindEntityByTargetName(nullptr, pstrTarget);
			if(pedict)
				hasValidTarget = true;
		}
	}

	bool isTargetedByChangeTarget = false;
	if(!hasValidTarget)
	{
		const Char* pstrTargetName = GetTargetName();
		edict_t* pedict = nullptr;
		while(true)
		{
			pedict = Util::FindEntityByTarget(pedict, pstrTargetName);
			if(!pedict)
				break;

			CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
			if(pEntity->IsTriggerChangeTargetEntity())
			{
				isTargetedByChangeTarget = true;
				break;
			}
		}
	}

	usableobject_type_t type = USABLE_OBJECT_NONE;
	if(!hasValidTarget && !isTargetedByChangeTarget)
	{
		// Not targeting anything, or is targeted by anyone
		type = USABLE_OBJECT_UNUSABLE;
	}
	else if(IsLockedByMaster() || isTargetedByChangeTarget
		|| m_nextUsableTime && m_nextUsableTime > g_pGameVars->time)
	{
		// Locked by master, or has no target, but targeted by changetarget
		if(!HasSpawnFlag(FL_DISABLED_NO_RETICLE))
			type = USABLE_OBJECT_LOCKED;
	}
	else if(HasSpawnFlag(FL_TOGGLE) && !m_stayPushed)
	{
		if(m_toggleState == TS_GOING_UP
			|| m_toggleState == TS_GOING_DOWN)
		{
			if(!HasSpawnFlag(FL_DISABLED_NO_RETICLE))
				type = USABLE_OBJECT_LOCKED;
		}
		else
			type = USABLE_OBJECT_DEFAULT;
	}	
	else if(m_toggleState != TS_AT_BOTTOM)
	{
		if(m_nextUsableTime > g_pGameVars->time 
			|| m_toggleState != TS_AT_BOTTOM)
		{
			if(!HasSpawnFlag(FL_DISABLED_NO_RETICLE))
			{
				if(!m_stayPushed)
					type = USABLE_OBJECT_LOCKED;
				else
					type = USABLE_OBJECT_UNUSABLE;
			}
		}
	}
	else
	{
		// Usable
		type = USABLE_OBJECT_DEFAULT;
	}

	return type;
}