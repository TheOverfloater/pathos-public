/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef FUNCFRICTION_H
#define FUNCFRICTION_H

#include "baseentity.h"

//=============================================
//
//=============================================
class CFuncFriction : public CBaseEntity
{
public:
	explicit CFuncFriction( edict_t* pedict );
	virtual ~CFuncFriction( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallTouch( CBaseEntity* pOther ) override;
	virtual Int32 GetEntityFlags( void ) override { return CBaseEntity::GetEntityFlags() & ~FL_ENTITY_TRANSITION; }

private:
	Float m_frictionModifier;
};
#endif //FUNCFRICTION_H