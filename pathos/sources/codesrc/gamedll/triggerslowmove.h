/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef TRIGGERSLOWMOVE_H
#define TRIGGERSLOWMOVE_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CTriggerSlowMove : public CPointEntity
{
public:
	enum mode_t
	{
		MODE_OFF = 0,
		MODE_ON,
		MODE_TOGGLE
	};

public:
	explicit CTriggerSlowMove( edict_t* pedict );
	virtual ~CTriggerSlowMove( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	
public:
	Int32 m_mode;
	bool m_isActive;
};
#endif //TRIGGERSLOWMOVE_H