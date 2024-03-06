/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef POINTENTITY_H
#define POINTENTITY_H

#include "delayentity.h"

//=============================================
//
//=============================================
class CPointEntity : public CBaseEntity
{
public:
	explicit CPointEntity( edict_t* pedict );
	virtual ~CPointEntity( void );

public:
	// Manages spawning
	virtual bool Spawn( void ) override;
	// Returns entity's flags
	virtual Int32 GetEntityFlags( void ) override { return CBaseEntity::GetEntityFlags() & ~FL_ENTITY_TRANSITION; }
};
#endif //POINTENTITY_H