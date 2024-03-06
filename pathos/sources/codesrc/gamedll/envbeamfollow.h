/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef ENVBEAMFOLLOW_H
#define ENVBEAMFOLLOW_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CEnvBeamFollow : public CPointEntity
{
public:
	explicit CEnvBeamFollow( edict_t* pedict );
	virtual ~CEnvBeamFollow( void );

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

private:
	// Sprite model name
	string_t m_spriteModelName;
	// Lifetime of beam
	Float m_life;
	// Beam width
	Float m_width;
	// Beam noise
	Float m_beamNoise;
	// Attachment index
	Int32 m_attachment;
};
#endif //ENVBEAMFOLLOW_H