/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef ENVMODEL_H
#define ENVMODEL_H

#include "animatingentity.h"

//=============================================
//
//=============================================
class CEnvModel : public CAnimatingEntity
{
public:
	enum
	{
		FL_CHANGE_SKIN		= (1<<0),
		FL_NO_LIGHTTRACES	= (1<<1),
		FL_NO_VISCHECKS		= (1<<2),
		FL_START_INVISIBLE	= (1<<3)
	};
public:
	explicit CEnvModel( edict_t* pedict );
	virtual ~CEnvModel( void );

public:
	virtual bool Spawn( void ) override;
	virtual void Precache( void ) override;
	virtual void InitEntity( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;

	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	virtual Int32 GetEntityFlags( void ) override { return CAnimatingEntity::GetEntityFlags() & ~FL_ENTITY_TRANSITION; }

protected:
	string_t m_sequence;
	string_t m_lightOrigin;
};

#endif //ENVMODEL_H
