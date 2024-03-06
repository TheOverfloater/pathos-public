/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef ENVBLUR
#define ENVBLUR

#include "pointentity.h"

//=============================================
//
//=============================================
class CEnvEarthQuake : public CPointEntity
{
public:
	explicit CEnvEarthQuake( edict_t* pedict );
	virtual ~CEnvEarthQuake( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;

public:
	EXPORTFN void QuakeThink( void );

protected:
	// Minimum delay
	Float m_minDelay;
	// Maximum delay
	Float m_maxDelay;

	// Minimum force
	Float m_minForce;
	// Maximum force
	Float m_maxForce;

	// Duration
	Float m_duration;
	// Fade out time
	Float m_fadeTime;

	// Shake end time
	Double m_shakeBeginTime;
};
#endif //ENVBLUR