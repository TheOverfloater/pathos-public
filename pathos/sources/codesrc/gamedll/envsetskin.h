/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef ENVSETSKIN_H
#define ENVSETSKIN_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CEnvSetSkin : public CPointEntity
{
public:
	explicit CEnvSetSkin( edict_t* pedict );
	virtual ~CEnvSetSkin( void );

public:
	virtual bool Spawn( void ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
};
#endif //ENVSETSKIN_H