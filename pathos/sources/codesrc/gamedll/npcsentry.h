/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef NPCSENTRY_H
#define NPCSENTRY_H

#include "ai_turretnpc.h"

//=============================================
//
//=============================================
class CNPCSentry : public CTurretNPC
{
public:
	// Model used by this NPC
	static const Char NPC_MODEL_NAME[];
	// Retracted height
	static const Float NPC_RETRACTED_HEIGHT;
	// Deployed height
	static const Float NPC_DEPLOYED_HEIGHT;
	// Minimum pitch value
	static const Float NPC_MIN_PITCH_VALUE;
	// View offset for npc
	static const Vector NPC_VIEW_OFFSET;
	// X size of NPC
	static const Float NPC_X_SIZE;
	// Y size of NPC
	static const Float NPC_Y_SIZE;

public:
	explicit CNPCSentry( edict_t* pedict );
	virtual ~CNPCSentry( void );

public:
	// Spawns the entity
	virtual bool Spawn( void ) override;
	// Makes the entity take on damage
	virtual bool TakeDamage( CBaseEntity* pInflictor, CBaseEntity* pAttacker, Float amount, Int32 damageFlags ) override;
	// Returns the classification
	virtual Int32 GetClassification( void ) const override;

	// Called when firing
	virtual void TurretShoot( const Vector& shootOrigin, const Vector& enemyDirection ) override;
	// Returns the gun position
	virtual Vector GetGunPosition( stance_t stance = STANCE_ACTUAL ) override { return m_pState->origin + NPC_VIEW_OFFSET; }
	// Return bullet type used by NPC
	virtual bullet_types_t GetBulletType( void ) override;

	// Called on initialization
	virtual void InitializeTurret( void ) override;

public:
	// Called when touching the sentry
	void EXPORTFN SentryTouch( CBaseEntity* pOther );
	// Called when sentry dies
	void EXPORTFN SentryDeath( void );
};
#endif