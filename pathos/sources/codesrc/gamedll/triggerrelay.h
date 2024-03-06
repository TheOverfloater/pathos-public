/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef TRIGGERRELAY_H
#define TRIGGERRELAY_H

#include "delayentity.h"

//=============================================
//
//=============================================
class CTriggerRelay : public CDelayEntity
{
public:
	enum
	{
		FL_REMOVE_ON_FIRE = (1<<0)
	};

public:
	explicit CTriggerRelay( edict_t* pedict );
	virtual ~CTriggerRelay( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	virtual Int32 GetEntityFlags( void ) override { return CDelayEntity::GetEntityFlags() & ~FL_ENTITY_TRANSITION; }

public:
	Int32 m_triggerMode;
};
#endif //TRIGGERRELAY_H