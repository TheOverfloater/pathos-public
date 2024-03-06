/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef TRIGGERCOUNTER_H
#define TRIGGERCOUNTER_H

#include "delayentity.h"

//=============================================
//
//=============================================
class CTriggerCounter : public CDelayEntity
{
public:
	explicit CTriggerCounter( edict_t* pedict );
	virtual ~CTriggerCounter( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	virtual Int32 GetEntityFlags( void ) override { return CDelayEntity::GetEntityFlags() & ~FL_ENTITY_TRANSITION; }

public:
	Uint32 m_nbTriggersLeft;
};
#endif //TRIGGERCOUNTER_H