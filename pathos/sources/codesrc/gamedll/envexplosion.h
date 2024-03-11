/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef ENVEXPLOSION_H
#define ENVEXPLOSION_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CEnvExplosion : public CPointEntity
{
public:
	enum
	{
		FL_NO_DAMAGE	= (1<<0),
		FL_REPEATABLE	= (1<<1),
		FL_NO_FIREBALL	= (1<<2),
		FL_NO_SMOKE		= (1<<3),
		FL_NO_DECAL		= (1<<4),
		FL_NO_SPARKS	= (1<<5),
		FL_NO_SHAKE		= (1<<6),
		FL_NO_DYNLIGHT	= (1<<7)
	};
public:
	explicit CEnvExplosion( edict_t* pedict );
	virtual ~CEnvExplosion( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;

public:
	void SetMagnitude( Int32 magnitude );
	void SetDamageAmount( Float dmgamount );
	void SetDamageRadius( Float radius );
	void SetAttacker( CBaseEntity* pAttacker );
	void SetInflictor( CBaseEntity* pInflictor );

public:
	void EXPORTFN SmokeThink( void );

public:
	static void CreateEnvExplosion( const Vector& origin, const Vector& angles, Int32 magnitude, bool dodamage, CBaseEntity* pAttacker = nullptr, CBaseEntity* pInflictor = nullptr );
	static void CreateEnvExplosion( const Vector& origin, const Vector& angles, Float radius, Float dmgamount, bool dodamage, CBaseEntity* pAttacker = nullptr, CBaseEntity* pInflictor = nullptr );

private:
	Int32 m_magnitude;
	Float m_dmgRadius;
	Float m_dmgAmount;

	CEntityHandle m_attacker;
	CEntityHandle m_inflictor;
};
#endif //ENVEXPLOSION_H