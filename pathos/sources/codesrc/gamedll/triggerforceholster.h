/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef TRIGGERFORCEHOLSTER_H
#define TRIGGERFORCEHOLSTER_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CTriggerForceHolster : public CPointEntity
{
public:
	enum mode_t
	{
		MODE_OFF = 0,
		MODE_ON,
		MODE_TOGGLE
	};

	enum
	{
		FL_IS_IN_DREAM = (1<<0),
		FL_NO_SLOW_MOVE = (1<<1)
	};

public:
	explicit CTriggerForceHolster( edict_t* pedict );
	virtual ~CTriggerForceHolster( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	
public:
	Int32 m_mode;
};
#endif //TRIGGERFORCEHOLSTER_H