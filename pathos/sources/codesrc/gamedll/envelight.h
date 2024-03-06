/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef ENVELIGHT_H
#define ENVELIGHT_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CEnvELight : public CPointEntity
{
public:
	enum
	{
		FL_START_ON = (1<<0)
	};
public:
	explicit CEnvELight( edict_t* pedict );
	virtual ~CEnvELight( void );

public:
	virtual bool Spawn( void ) override;
	virtual void Precache( void ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	virtual bool ShouldSetBoundsOnRestore( void ) override { return false; }
};

#endif //ENVELIGHT_H