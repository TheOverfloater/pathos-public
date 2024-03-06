/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef ENTITYNAME_H
#define ENTITYNAME_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CTriggerMoveTrain : public CPointEntity
{
public:
	enum
	{
		FL_REROUTE = (1<<0)
	};

public:
	explicit CTriggerMoveTrain( edict_t* pedict );
	virtual ~CTriggerMoveTrain( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	
private:
	string_t m_targetPathName;
};
#endif //ENTITYNAME_H