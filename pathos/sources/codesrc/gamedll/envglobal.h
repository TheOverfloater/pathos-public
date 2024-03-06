/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef ENVGLOBAL_H
#define ENVGLOBAL_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CEnvGobal : public CPointEntity
{
public:
	enum
	{
		FL_SET_INITIAL_STATE = (1<<0)
	};

public:
	enum
	{
		STATE_OFF = 0,
		STATE_ON,
		STATE_DEAD,
		STATE_TOGGLE,
		STATE_DELETED = 4
	};

public:
	explicit CEnvGobal( edict_t* pedict );
	virtual ~CEnvGobal( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	
public:
	void SetState( Int32 newstate );

private:
	string_t m_globalStateName;
	Int32 m_triggerMode;
	Int32 m_initialState;
};
#endif //ENVGLOBAL_H