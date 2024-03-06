/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef ENVLENSFLARE_H
#define ENVLENSFLARE_H

#include "pointentity.h"
#include "portal_shared.h"

//=============================================
//
//=============================================
class CEnvLensFlare : public CBaseEntity
{
public:
	enum
	{
		FL_START_ON = (1<<0)
	};

public:
	explicit CEnvLensFlare( edict_t* pedict );
	virtual ~CEnvLensFlare( void );

public:
	virtual bool Spawn( void ) override;
	virtual void Precache( void ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
};
#endif //ENVLENSFLARE_H