/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef TRIGGERCHANGELEVEL_H
#define TRIGGERCHANGELEVEL_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CTriggerChangeLevel : public CBaseEntity
{
public:
	enum
	{
		FL_TRIGGER_ONLY = (1<<1)
	};

public:
	explicit CTriggerChangeLevel( edict_t* pedict );
	virtual ~CTriggerChangeLevel( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	virtual Int32 GetEntityFlags( void ) override { return CBaseEntity::GetEntityFlags() & ~FL_ENTITY_TRANSITION; }	

public:
	void BeginLevelChange( void );
	void EXPORTFN ChangeLevelTouch( CBaseEntity* pOther );

public:
	// Next level name
	string_t m_nextLevelName;
	// Landmark name
	string_t m_landmarkName;
	// Last time this entity was fired
	Double m_lastFiredtime;
};
#endif //TRIGGERCHANGELEVEL_H