/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef AISETENEMY_H
#define AISETENEMY_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CAISetEnemy : public CPointEntity
{
public:
	explicit CAISetEnemy( edict_t* pedict );
	virtual ~CAISetEnemy( void );

public:
	virtual bool Spawn( void ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
};
#endif //AISETENEMY_H