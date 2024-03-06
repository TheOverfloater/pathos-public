/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef TRIGGERKILLPLAYER_H
#define TRIGGERKILLPLAYER_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CTriggerKillPlayer : public CPointEntity
{
public:
	explicit CTriggerKillPlayer( edict_t* pedict );
	virtual ~CTriggerKillPlayer( void );

public:
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
};
#endif //TRIGGERKILLPLAYER_H