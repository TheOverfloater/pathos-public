/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef ENVGLOW_H
#define ENVGLOW_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CEnvGlow : public CBaseEntity
{
public:
	explicit CEnvGlow( edict_t* pedict );
	virtual ~CEnvGlow( void );

public:
	virtual bool Spawn( void ) override;
	virtual void Precache( void ) override;
	virtual Int32 GetEntityFlags( void ) override { return CBaseEntity::GetEntityFlags() & ~FL_ENTITY_TRANSITION; }	
};
#endif //ENVGLOW_H