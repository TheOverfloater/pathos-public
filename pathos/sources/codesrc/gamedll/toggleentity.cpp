/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "toggleentity.h"

//=============================================
// @brief
//
//=============================================
CToggleEntity::CToggleEntity( edict_t* pedict ):
	CDelayEntity(pedict),
	m_toggleState(TS_NONE),
	m_activateFinished(0),
	m_moveDistance(0),
	m_waitTime(0),
	m_lip(0),
	m_tWidth(0),
	m_tLength(0),
	m_height(0),
	m_damageDealt(0),
	m_damageBits(0),
	m_masterEntityName(NO_STRING_VALUE),
	m_moveSoundFile(NO_STRING_VALUE),
	m_stopSoundFile(NO_STRING_VALUE),
	m_lockedSoundFile(NO_STRING_VALUE),
	m_unlockedSoundFile(NO_STRING_VALUE),
	m_pfnMoveDoneFunction(nullptr)
{
}

//=============================================
// @brief
//
//=============================================
CToggleEntity::~CToggleEntity( void )
{
}

//=============================================
// @brief
//
//=============================================
void CToggleEntity::DeclareSaveFields( void )
{
	// Call base class to do it first
	CDelayEntity::DeclareSaveFields();

	DeclareSaveField(DEFINE_DATA_FIELD(CToggleEntity, m_toggleState, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CToggleEntity, m_activateFinished, EFIELD_TIME));
	DeclareSaveField(DEFINE_DATA_FIELD(CToggleEntity, m_moveDistance, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CToggleEntity, m_waitTime, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CToggleEntity, m_lip, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CToggleEntity, m_tWidth, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CToggleEntity, m_tLength, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CToggleEntity, m_position1, EFIELD_COORD));
	DeclareSaveField(DEFINE_DATA_FIELD(CToggleEntity, m_position2, EFIELD_COORD));
	DeclareSaveField(DEFINE_DATA_FIELD(CToggleEntity, m_angle1, EFIELD_VECTOR));
	DeclareSaveField(DEFINE_DATA_FIELD(CToggleEntity, m_angle2, EFIELD_VECTOR));
	DeclareSaveField(DEFINE_DATA_FIELD(CToggleEntity, m_finalDest, EFIELD_VECTOR));
	DeclareSaveField(DEFINE_DATA_FIELD(CToggleEntity, m_finalAngle, EFIELD_VECTOR));
	DeclareSaveField(DEFINE_DATA_FIELD(CToggleEntity, m_height, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CToggleEntity, m_activator, EFIELD_EHANDLE));
	DeclareSaveField(DEFINE_DATA_FIELD(CToggleEntity, m_damageDealt, EFIELD_FLOAT));
	DeclareSaveField(DEFINE_DATA_FIELD(CToggleEntity, m_damageBits, EFIELD_INT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CToggleEntity, m_masterEntityName, EFIELD_STRING));
	DeclareSaveField(DEFINE_DATA_FIELD(CToggleEntity, m_moveSoundFile, EFIELD_SOUNDNAME));
	DeclareSaveField(DEFINE_DATA_FIELD(CToggleEntity, m_stopSoundFile, EFIELD_SOUNDNAME));
	DeclareSaveField(DEFINE_DATA_FIELD(CToggleEntity, m_lockedSoundFile, EFIELD_SOUNDNAME));
	DeclareSaveField(DEFINE_DATA_FIELD(CToggleEntity, m_unlockedSoundFile, EFIELD_SOUNDNAME));
	DeclareSaveField(DEFINE_DATA_FIELD(CToggleEntity, m_pfnMoveDoneFunction, EFIELD_FUNCPTR));
}

//=============================================
// @brief
//
//=============================================
void CToggleEntity::Precache( void )
{
	CDelayEntity::Precache();

	if(m_moveSoundFile != NO_STRING_VALUE)
		gd_engfuncs.pfnPrecacheSound(gd_engfuncs.pfnGetString(m_moveSoundFile));

	if(m_stopSoundFile != NO_STRING_VALUE)
		gd_engfuncs.pfnPrecacheSound(gd_engfuncs.pfnGetString(m_stopSoundFile));

	if(m_lockedSoundFile != NO_STRING_VALUE)
		gd_engfuncs.pfnPrecacheSound(gd_engfuncs.pfnGetString(m_lockedSoundFile));

	if(m_unlockedSoundFile != NO_STRING_VALUE)
		gd_engfuncs.pfnPrecacheSound(gd_engfuncs.pfnGetString(m_unlockedSoundFile));
}

//=============================================
// @brief
//
//=============================================
bool CToggleEntity::KeyValue( const keyvalue_t& kv )
{
	if(!qstrcmp(kv.keyname, "lip"))
	{
		m_lip = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "wait"))
	{
		m_waitTime = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "dmg"))
	{
		m_damageDealt = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "distance"))
	{
		m_moveDistance = SDL_atof(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "master"))
	{
		m_masterEntityName = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "custom_movesnd"))
	{
		m_moveSoundFile = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else if(!qstrcmp(kv.keyname, "custom_stopsnd"))
	{
		m_stopSoundFile = gd_engfuncs.pfnAllocString(kv.value);
		return true;
	}
	else
		return CDelayEntity::KeyValue(kv);
}

//=============================================
// @brief
//
//=============================================
togglestate_t CToggleEntity::GetToggleState( void )
{
	return (togglestate_t)m_toggleState;
}

//=============================================
// @brief
//
//=============================================
Float CToggleEntity::GetDelay( void )
{
	return m_waitTime;
}

//=============================================
// @brief
//
//=============================================
void CToggleEntity::PlayLockSounds( bool locked, bool button, Float waittime, Double& nextSoundTime )
{
	if(nextSoundTime > g_pGameVars->time)
		return;

	const Char* pstrSound = nullptr;
	if(locked && m_lockedSoundFile != NO_STRING_VALUE)
		pstrSound = gd_engfuncs.pfnGetString(m_lockedSoundFile);
	else if(!locked && m_unlockedSoundFile  != NO_STRING_VALUE)
		pstrSound = gd_engfuncs.pfnGetString(m_unlockedSoundFile);

	if(!pstrSound || !qstrlen(pstrSound))
		return;

	Util::EmitEntitySound(this, pstrSound, SND_CHAN_ITEM);
	nextSoundTime = g_pGameVars->time + waittime;
}

//=============================================
// @brief
//
//=============================================
void CToggleEntity::LinearMove( const Vector& destPosition, Float speed )
{
	// Set destination position
	m_finalDest = destPosition;

	// Dont' do anything if we're already there
	if(m_finalDest == m_pState->origin)
	{
		LinearMoveDone();
		return;
	}

	// Calculate travel time
	Vector destDelta = destPosition - m_pState->origin;
	Double travelTime = destDelta.Length()/speed;

	// Set next think time
	SetThink(&CToggleEntity::LinearMoveDone);
	m_pState->nextthink = m_pState->ltime + travelTime;

	// Set velocity
	Math::VectorScale(destDelta, 1.0f/travelTime, m_pState->velocity);
}

//=============================================
// @brief
//
//=============================================
void CToggleEntity::AngularMove( const Vector& destAngle, Float speed )
{
	// Set dest angle
	m_finalAngle = destAngle;

	// Don't do anything if we're already there
	if(m_finalAngle == m_pState->angles)
	{
		AngularMoveDone();
		return;
	}

	// Calculate duration and vector
	Vector destDelta = destAngle - m_pState->angles;
	Double travelTime = destDelta.Length()/speed;

	// Set next think time
	SetThink(&CToggleEntity::AngularMoveDone);
	m_pState->nextthink = m_pState->ltime + travelTime;

	// Set velocity
	Math::VectorScale(destDelta, 1.0f/travelTime, m_pState->avelocity);
}

//=============================================
// @brief
//
//=============================================
bool CToggleEntity::IsLockedByMaster( void )
{
	if(m_masterEntityName != NO_STRING_VALUE)
	{
		const Char* pstrMasterName = gd_engfuncs.pfnGetString(m_masterEntityName);
		if(!Util::IsMasterTriggered(pstrMasterName, m_activator, this))
			return true;
	}

	return false;
}

//=============================================
// @brief
//
//=============================================
void CToggleEntity::LinearMoveDone( void )
{
	// Snap entity to final position
	gd_engfuncs.pfnSetOrigin(m_pEdict, m_finalDest);
	
	// Clear velocity and thinking
	m_pState->velocity.Clear();
	m_pState->nextthink = 0;

	// Call move done function
	if(m_pfnMoveDoneFunction)
		(this->*m_pfnMoveDoneFunction)();
}

//=============================================
// @brief
//
//=============================================
void CToggleEntity::AngularMoveDone( void )
{
	// Set final angles
	m_pState->angles = m_finalAngle;

	// Clear velocity and thinking
	m_pState->avelocity.Clear();
	m_pState->nextthink = 0;

	// Call move done function
	if(m_pfnMoveDoneFunction)
		(this->*m_pfnMoveDoneFunction)();
}

//=============================================
// @brief
//
//=============================================
const Vector& CToggleEntity::GetPosition1( void ) const
{
	return m_position1;
}

//=============================================
// @brief
//
//=============================================
const Vector& CToggleEntity::GetPosition2( void ) const
{
	return m_position2;
}

#ifdef _DEBUG
//=============================================
// @brief
//
//=============================================
MOVEDONEPTR CToggleEntity::_SetMoveDone( MOVEDONEPTR pfnptr, const Char* pstrFunctionName )
{
	m_pfnMoveDoneFunction = pfnptr;
	CheckFunction(reinterpret_cast<void*>(*(reinterpret_cast<Int64*>(reinterpret_cast<byte*>(this) + offsetof(CToggleEntity, m_pfnMoveDoneFunction)))), pstrFunctionName);
	return m_pfnMoveDoneFunction;
}
#endif //_DEBUG