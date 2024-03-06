/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef GRENADE_H
#define GRENADE_H

#include "animatingentity.h"

//=============================================
//
//=============================================
class CGrenade : public CAnimatingEntity
{
public:
	// Model file path
	static const Char MODEL_FILENAME[];

public:
	explicit CGrenade( edict_t* pedict );
	virtual ~CGrenade( void );

public:
	virtual bool Spawn( void ) override;
	virtual void Precache( void ) override;

public:
	void EXPORTFN BounceTouch( CBaseEntity* pOther );
	void EXPORTFN SlideTouch( CBaseEntity* pOther );
	void EXPORTFN ExplodeTouch( CBaseEntity* pOther );

	void EXPORTFN DangerSoundThink( void );
	void EXPORTFN TumbleThink( void );

public:
	void EXPORTFN Explode( void );
	void BounceSound( void );

public:
	void SetExplodeTime( Float explodeTime );
	void SetExplodeDelay( Float delay );
	void SetDamageAmount( Float dmgAmount );
	void SetDamageRadius( Float dmgRadius );
	void SetDamageTime( Float dmgtime );
	void SetAttacker( CBaseEntity* pAttacker );

public:
	static CGrenade* CreateTimed( CBaseEntity* pOwner, const Vector& origin, const Vector& velocity, Float time, Float radius, Float damage, bool contactDelayCountdown = false );
	static CGrenade* CreateContact( CBaseEntity* pOwner, const Vector& origin, const Vector& velocity, Float radius, Float damage );

private:
	Double m_nextDmgTime;
	Double m_explodeTime;
	Float m_explodeDelay;
	bool m_emittedNPCSound;

	Float m_damageAmount;
	Float m_damageRadius;
	CEntityHandle m_attacker;
};
#endif //GRENADE_H