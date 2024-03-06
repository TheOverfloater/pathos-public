/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef TRIGGERAUTO_H
#define TRIGGERAUTO_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CTriggerAuto : public CDelayEntity
{
public:
	enum
	{
		FL_REMOVE_ON_FIRE = (1<<0)
	};
public:
	explicit CTriggerAuto( edict_t* pedict );
	virtual ~CTriggerAuto( void );

public:
	virtual bool Spawn( void ) override;
	virtual bool Restore( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void InitEntity( void ) override;
	virtual bool IsTriggerAutoEntity( void ) const override { return true; }

	virtual Int32 GetEntityFlags( void ) override { return CDelayEntity::GetEntityFlags() & ~FL_ENTITY_TRANSITION; }

public:
	string_t m_globalState;
	Int32 m_triggerMode;

	string_t m_p1GlobalState;
	string_t m_p1Target;

	string_t m_p2GlobalState;
	string_t m_p2Target;

	string_t m_p3GlobalState;
	string_t m_p3Target;
};
#endif //TRIGGERAUTO_H