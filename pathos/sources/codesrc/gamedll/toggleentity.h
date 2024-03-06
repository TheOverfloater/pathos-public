/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef TOGGLEENTITY_H
#define TOGGLEENTITY_H

#include "delayentity.h"
#include "ehandle.h"

class CToggleEntity;

#ifdef _DEBUG
typedef void (CToggleEntity::*MOVEDONEPTR)( void );

#define SetMoveDone( function ) _SetMoveDone(static_cast <void (CToggleEntity::*)( void )>(function), #function)
#else
#define SetMoveDone( function ) m_pfnMoveDoneFunction = static_cast <void (CToggleEntity::*)( void )>(function)
#endif

//=============================================
//
//=============================================
class CToggleEntity : public CDelayEntity
{
public:
	explicit CToggleEntity( edict_t* pedict );
	virtual ~CToggleEntity( void );

public:
	virtual void Precache( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual togglestate_t GetToggleState( void ) override;
	virtual bool IsLockedByMaster( void ) override;
	virtual Float GetWaitTime( void ) override { return m_waitTime; }

public:
	virtual Float GetDelay( void ) override;

	void PlayLockSounds( bool locked, bool button, Float waittime, Double& nextSoundTime );

	void LinearMove( const Vector& destPosition, Float speed );
	void AngularMove( const Vector& destAngle, Float speed );

	void EXPORTFN LinearMoveDone( void );
	void EXPORTFN AngularMoveDone( void );

	const Vector& GetPosition1( void ) const;
	const Vector& GetPosition2( void ) const;

public:
#ifdef _DEBUG
public:
	// Sets the move done function in Debug mode
	MOVEDONEPTR _SetMoveDone( MOVEDONEPTR pfnptr, const Char* pstrFunctionName );
#endif

protected:
	Int32 m_toggleState;
	Double m_activateFinished;

	Float m_moveDistance;
	Float m_waitTime;
	Float m_lip;
	Float m_tWidth;
	Float m_tLength;

	Vector m_position1;
	Vector m_position2;

	Vector m_angle1;
	Vector m_angle2;

	Vector m_finalDest;
	Vector m_finalAngle;

	Float m_height;

	CEntityHandle m_activator;

	Float m_damageDealt;
	Int32 m_damageBits;

	string_t m_masterEntityName;
	string_t m_moveSoundFile;
	string_t m_stopSoundFile;
	string_t m_lockedSoundFile;
	string_t m_unlockedSoundFile;

protected:
	// Move done function pointer
	void (CToggleEntity::*m_pfnMoveDoneFunction)( void );
};
#endif //TOGGLEENTITY_H
