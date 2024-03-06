/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef NULLENTITY_H
#define NULLENTITY_H

#include "baseentity.h"

//=============================================
//
//=============================================
class CNullEntity : public CBaseEntity
{
public:
	explicit CNullEntity( edict_t* pedict );
	virtual ~CNullEntity( void );

public:
	// Spawns the entity
	virtual bool Spawn( void ) override;
	// Manages keyvalues
	virtual bool KeyValue( const keyvalue_t& kv ) override { return true; }; // don't care about unhandled keyvalues
	// Returns entity's flags
	virtual Int32 GetEntityFlags( void ) override { return CBaseEntity::GetEntityFlags() & ~FL_ENTITY_TRANSITION; }
};
#endif //NULLENTITY_H