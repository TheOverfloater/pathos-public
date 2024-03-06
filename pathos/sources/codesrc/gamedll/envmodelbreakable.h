/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef ENVMODELBREAKABLE_H
#define ENVMODELBREAKABLE_H

#include "funcbreakable.h"

//=============================================
//
//=============================================
class CEnvModelBreakable : public CFuncBreakable
{
public:
	// Material names for material types
	static const Char* MAT_TYPE_MAT_NAMES[];

public:
	explicit CEnvModelBreakable( edict_t* pedict );
	virtual ~CEnvModelBreakable( void );

public:
	virtual void Precache( void ) override;
	virtual void SetSpawnProperties( void ) override;
	virtual void SetBoundingBox( void ) override;
	virtual void TraceAttack( CBaseEntity* pAttacker, Float damage, const Vector& direction, trace_t& tr, Int32 damageFlags ) override;
};
#endif //ENVMODELBREAKABLE_H