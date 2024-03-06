
/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef FUNCTRAIN_H
#define FUNCTRAIN_H

#include "plattrainentity.h"

//=============================================
//
//=============================================
class CFuncTrain : public CPlatTrainEntity
{
public:
	// Default train speed
	static const Float DEFAULT_SPEED;
	// Default train volume
	static const Float DEFAULT_VOLUME;
	// Train center correction offset
	static const Vector TRAIN_CENTER_OFFSET;

public:
	enum
	{
		FL_WAIT_RETRIGGER		= (1<<0),
		FL_NOT_SOLID			= (1<<3),
		FL_NO_PASS_TRIGGER		= (1<<4),
		FL_DONT_NUDGE_NPCS		= (1<<5),
		FL_SOUND_USE_ORIGIN		= (1<<7),
		FL_DONT_BLOCK_NODES		= (1<<8),
		FL_START_ON				= (1<<9),
		FL_USE_ORIGIN			= (1<<10),
		FL_ALWAYS_SET_AVELOCITY	= (1<<11)
	};
public:
	explicit CFuncTrain( edict_t* pedict );
	virtual ~CFuncTrain( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void InitEntity( void ) override;
	virtual void OnOverrideEntity( void ) override;
	virtual void Reroute( CBaseEntity* pTarget, Float speed ) override;
	virtual void MoveTrainToPathCorner( CBaseEntity* pPathCorner, CBaseEntity* pTargetingPathCorner ) override;
	virtual bool IsFuncTrainEntity( void ) override { return true; }

	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	virtual void CallBlocked( CBaseEntity* pBlocker ) override;

public:
	void EXPORTFN Wait( void );
	void EXPORTFN Next( void );

public:
	virtual bool TrainSetModel( void );
	virtual void SetSpawnProperties( void );
	virtual Vector GetDestinationVector( const Vector& destOrigin );

private:
	CBaseEntity*	m_pCurrentTarget;
	CBaseEntity*	m_pPreviousTarget;
	string_t		m_lastTargetName;
	string_t		m_currentPathCornerName;
	bool			m_isSoundPlaying;
	bool			m_skipSounds;
};
#endif //FUNCTRAIN_H