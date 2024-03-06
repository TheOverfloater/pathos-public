/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef ENVSHAKE_H
#define ENVSHAKE_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CEnvShake : public CPointEntity
{
public:
	enum
	{
		FL_SHAKE_EVERYWHERE	= (1<<0),
		FL_DISRUPT_CONTROLS	= (1<<1),
		FL_SHAKE_INAIR		= (1<<2)
	};

public:
	explicit CEnvShake( edict_t* pedict );
	virtual ~CEnvShake( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	
public:
	Float GetAmplitude( void );
	Float GetFrequency( void );
	Float GetDuration( void );
	Float GetRadius( void );

	void SetAmplitude( Float amplitude );
	void SetFrequency( Float frequency );
	void SetDuration( Float duration );
	void SetRadius( Float radius );

private:
	Float m_amplitude;
	Float m_frequency;
	Float m_duration;
	Float m_radius;
};
#endif //ENVSHAKE_H