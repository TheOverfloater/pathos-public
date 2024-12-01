/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef ENVSPARK_H
#define ENVSPARK_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CEnvSpark : public CPointEntity
{
public:
	// Default delay value
	static const Float DEFAULT_DELAY_TIME;

public:
	enum
	{
		FL_TOGGLE	= (1<<5),
		FL_START_ON	= (1<<6)
	};

public:
	explicit CEnvSpark( edict_t* pedict );
	virtual ~CEnvSpark( void );

public:
	virtual bool Spawn( void ) override;
	virtual void Precache( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	
public:
	void EXPORTFN SparkThink( void );

private:
	bool m_isActive;
	Float m_delay;
	CString m_soundName;
};
#endif //ENVSPARK_H