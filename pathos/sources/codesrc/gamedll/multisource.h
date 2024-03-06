/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef MULTISOURCE_H
#define MULTISOURCE_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CMultiSource : public CPointEntity
{
public:
	enum
	{
		FL_START_ON = (1<<0)
	};
public:
	explicit CMultiSource( edict_t* pedict );
	virtual ~CMultiSource( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	virtual Int32 GetEntityFlags( void ) override { return CPointEntity::GetEntityFlags() | FL_ENTITY_MASTER; }
	virtual bool IsTriggered( const CBaseEntity* pentity ) const override;

public:
	bool IsGlobalEnabled( void ) const;

private:
	string_t m_globalStateName;
	bool m_isTriggered;
};
#endif //MULTISOURCE_H