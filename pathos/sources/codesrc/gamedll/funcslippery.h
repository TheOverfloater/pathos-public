/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef FUNCSLIPPERY_H
#define FUNCSLIPPERY_H

#include "baseentity.h"

//=============================================
//
//=============================================
class CFuncSlippery : public CBaseEntity
{
public:
	explicit CFuncSlippery( edict_t* pedict );
	virtual ~CFuncSlippery( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallTouch( CBaseEntity* pOther ) override;
	virtual Int32 GetEntityFlags( void ) override { return CBaseEntity::GetEntityFlags() & ~FL_ENTITY_TRANSITION; }

private:
	Float m_planeZCap;
};
#endif //FUNCSLIPPERY_H