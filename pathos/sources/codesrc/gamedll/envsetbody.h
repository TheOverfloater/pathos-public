/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef ENVSETBODY_H
#define ENVSETBODY_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CEnvSetBody : public CPointEntity
{
public:
	explicit CEnvSetBody( edict_t* pedict );
	virtual ~CEnvSetBody( void );

public:
	virtual bool Spawn( void ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
};
#endif //ENVSETBODY_H