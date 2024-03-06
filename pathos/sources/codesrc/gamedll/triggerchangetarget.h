/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef TRIGGERCHANGETARGET_H
#define TRIGGERCHANGETARGET_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CTriggerChangeTarget : public CDelayEntity
{
public:
	explicit CTriggerChangeTarget( edict_t* pedict );
	virtual ~CTriggerChangeTarget( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	virtual Int32 GetEntityFlags( void ) override { return CDelayEntity::GetEntityFlags() & ~FL_ENTITY_TRANSITION; }
	virtual bool IsTriggerChangeTargetEntity( void ) const override { return true; }

public:
	string_t m_newTarget;
};
#endif //TRIGGERCHANGETARGET_H