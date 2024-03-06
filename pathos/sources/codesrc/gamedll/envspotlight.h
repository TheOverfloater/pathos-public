/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef ENVSPOTLIGHT_H
#define ENVSPOTLIGHT_H

#include "envdlight.h"

//=============================================
//
//=============================================
class CEnvSpotlight : public CEnvDLight
{
public:
	explicit CEnvSpotlight( edict_t* pedict );
	virtual ~CEnvSpotlight( void );

public:
	virtual void SetMinsMaxs( void ) override;
	virtual void SetLightRenderFx( void ) override;

private:
	// Only CEnvSpotlight can call this, not derived/parent classes
	void SetSpotlightValues( const Vector& origin, const Vector& angles, const Vector& color, Uint32 radius, Uint32 conesize );

public:
	static CEnvSpotlight* SpawnSpotlight( const Vector& origin, const Vector& angles, const Vector& color, Uint32 radius, Uint32 conesize );
};
#endif //ENVSPOTLIGHT_H