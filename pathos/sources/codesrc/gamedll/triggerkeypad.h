/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef TRIGGERKEYPAD_H
#define TRIGGERKEYPAD_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CTriggerKeypad : public CPointEntity
{
public:
	enum
	{
		FL_STAY_TILL_NEXT = (1<<0)
	};

public:
	explicit CTriggerKeypad( edict_t* pedict );
	virtual ~CTriggerKeypad( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;

public:
	void FireTarget( CBaseEntity* pPlayer );

public:
	string_t m_keypadId;
};
#endif //TRIGGERKEYPAD_H