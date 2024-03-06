/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef NPCTESTHULL_H
#define NPCTESTHULL_H

#include "statestack.h"
#include "nodeignorelist.h"
#include "ai_basenpc.h"

class CBaseNPC;
class CBaseEntity;
class CEntityHandle;
class CNodeIgnoreList;

enum node_hull_types_t;

/*
=======================
CNPCTestHull

=======================
*/
class CNPCTestHull : public CBaseNPC
{
public:
	explicit CNPCTestHull( edict_t* pedict );
	~CNPCTestHull( void );

public:
	// Manages spawning
	virtual bool Spawn( void ) override;
	// Returns entity's flags
	virtual Int32 GetEntityFlags( void ) override { return CBaseEntity::GetEntityFlags() & ~FL_ENTITY_TRANSITION; }

public:
	// Sets the yaw speed
	virtual void SetYawSpeed( void ) override;
	// Returns the sound mask
	virtual Uint64 GetSoundMask( void ) override;
	// Returns the gun position
	virtual Vector GetGunPosition( stance_t stance = STANCE_ACTUAL ) override { return m_pState->origin; }

public:
	// Makes the entity non-solid
	void MakeInert( void );
	// Makes the entity solid
	void MakeSolid( void );
};
#endif //NPCTESTHULL_H