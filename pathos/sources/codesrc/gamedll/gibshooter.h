/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef GIBSHOOTER_H
#define GIBSHOOTER_H

#include "pointentity.h"
#include "gib.h"

//=============================================
//
//=============================================
class CGibShooter : public CPointEntity
{
public:
	enum
	{
		FL_REPEATABLE = (1<<0)
	};

public:
	explicit CGibShooter( edict_t* pedict );
	virtual ~CGibShooter( void );

public:
	virtual bool Spawn( void ) override;
	virtual void Precache( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;

public:
	void EXPORTFN ShootThink( void );

public:
	virtual CGib* CreateGib( void );

protected:
	Uint32 m_numGibs;
	Uint32 m_gibCapacity;
	Int32 m_gibMaterial;
	Int32 m_gibModelIndex;
	Int32 m_nbBodyVariations;

	Float m_gibVelocity;
	Float m_variance;
	Float m_gibLifetime;
	Float m_delay;
};
#endif //GIBSHOOTER_H