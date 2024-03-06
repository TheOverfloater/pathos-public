/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef TRIGGERLOGIN_H
#define TRIGGERLOGIN_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CTriggerLogin : public CPointEntity
{
public:
	enum
	{
		FL_STAY_TILL_NEXT = (1<<0)
	};

public:
	explicit CTriggerLogin( edict_t* pedict );
	virtual ~CTriggerLogin( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	
public:
	void FireTarget( CBaseEntity* pPlayer );

private:
	string_t m_username;
	string_t m_password;
	string_t m_codeid;
};

#endif //TRIGGERLOGIN_H