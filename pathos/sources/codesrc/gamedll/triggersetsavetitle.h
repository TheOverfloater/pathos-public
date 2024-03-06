/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef TRIGGERSETSAVETITLE_H
#define TRIGGERSETSAVETITLE_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CTriggerSetSaveTitle : public CPointEntity
{
public:
	enum
	{
		FL_START_ON = (1<<0)
	};

public:
	explicit CTriggerSetSaveTitle( edict_t* pedict );
	virtual ~CTriggerSetSaveTitle( void );

public:
	virtual bool Spawn( void ) override;
	virtual void InitEntity( void ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
};
#endif //TRIGGERSETSAVETITLE_H