/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef TRIGGERPARALYZEPLAYER_H
#define TRIGGERPARALYZEPLAYER_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CTriggerParalyzePlayer : public CPointEntity
{
public:
	explicit CTriggerParalyzePlayer( edict_t* pedict );
	virtual ~CTriggerParalyzePlayer( void );

public:
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
};
#endif //TRIGGERPARALYZEPLAYER_H