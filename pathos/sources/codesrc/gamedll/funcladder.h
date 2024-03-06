/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef FUNCLADDER_H
#define FUNCLADDER_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CFuncLadder : public CBaseEntity
{
public:
	explicit CFuncLadder( edict_t* pedict );
	virtual ~CFuncLadder( void );

public:
	virtual bool Spawn( void ) override;
	virtual Int32 GetEntityFlags( void ) override { return CBaseEntity::GetEntityFlags() & ~FL_ENTITY_TRANSITION; }
};
#endif //FUNCLADDER_H