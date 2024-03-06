/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef ENVLASER
#define ENVLASER

#include "beam.h"
#include "envsprite.h"

//=============================================
//
//=============================================
class CEnvLaser : public CBeam
{
public:
	explicit CEnvLaser( edict_t* pedict );
	virtual ~CEnvLaser( void );

public:
	// Spawns the entity
	virtual bool Spawn( void ) override;
	// Performs precache functions
	virtual void Precache( void ) override;
	// Calls for classes and their children
	virtual void DeclareSaveFields( void ) override;
	// Manages keyvalues
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	// Calls use function
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;

public:
	// Turns the laser on
	void TurnOn( void );
	// Turns the laser off
	void TurnOff( void );
	// Tells if the laser is on
	bool IsOn( void );

	// Fires the laser at a given point
	void FireAtPoint( trace_t& tr );

	// Called when striking
	void EXPORTFN StrikeThink( void );

public:
	// Sprite for laser
	CEntityHandle m_sprite;
	// Sprite model name
	string_t m_spriteModelName;
	// Firing position
	Vector m_firePosition;
	// Laser target entity name
	string_t m_laserTarget;
};
#endif //ENVBLUR