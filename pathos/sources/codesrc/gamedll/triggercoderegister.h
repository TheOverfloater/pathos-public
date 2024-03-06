/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef TRIGGERCODEREGISTER_H
#define TRIGGERCODEREGISTER_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CTriggerCodeRegister : public CPointEntity
{
public:
	enum
	{
		FL_AUTO_GEN_CODE = (1<<0)
	};
public:
	explicit CTriggerCodeRegister( edict_t* pedict );
	virtual ~CTriggerCodeRegister( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	
public:
	string_t m_codeId;
	string_t m_code;
};
#endif //TRIGGERCODEREGISTER_H