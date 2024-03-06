/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef NPCGENERIC_H
#define NPCGENERIC_H

#include "ai_basenpc.h"

//=============================================
//
//=============================================
class CNPCGeneric : public CBaseNPC
{
public:
	// Special renderfx value for mirror only(for legacy support)
	static const Int32 NPC_GENERIC_MIRROR_ONLY_FX_VALUE;
	// Yaw speed for npc
	static const Uint32 NPC_YAW_SPEED;
	// Default health for NPC
	static const Float NPC_DEFAULT_HEALTH;

public:
	enum
	{
		FL_GENERICNPC_NOT_SOLID = (1<<2)
	};

	enum hulltype_t
	{
		NPC_GENERIC_HULL_HUMAN = 0,
		NPC_GENERIC_HULL_PLAYER
	};

public:
	explicit CNPCGeneric( edict_t* pedict );
	virtual ~CNPCGeneric( void );

public:
	// Spawns the entity
	virtual bool Spawn( void ) override;
	// Manages a keyvalue
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	// Returns the classification
	virtual Int32 GetClassification( void ) const override;

public:
	// Sets the ideal yaw speed
	virtual void SetYawSpeed( void ) override;
	// Returns the sound mask for the NPC
	virtual Uint64 GetSoundMask( void ) override;
	// Returns the gun position
	virtual Vector GetGunPosition( stance_t stance = STANCE_ACTUAL ) override { return m_pState->origin; }

private:
	// Hull type
	Int32 m_hullType;
};
#endif //NPCGENERIC_H