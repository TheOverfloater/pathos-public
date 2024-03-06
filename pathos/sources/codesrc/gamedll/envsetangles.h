/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef ENVSETANGLES_H
#define ENVSETANGLES_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CEnvSetAngles : public CPointEntity
{
public:
	explicit CEnvSetAngles( edict_t* pedict );
	virtual ~CEnvSetAngles( void );

public:
	virtual bool Spawn( void ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
};
#endif //ENVSETANGLES_H