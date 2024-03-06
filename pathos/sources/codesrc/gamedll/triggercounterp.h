/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef TRIGGERCOUNTERP_H
#define TRIGGERCOUNTERP_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CTriggerCounterP : public CPointEntity
{
public:
	explicit CTriggerCounterP( edict_t* pedict );
	virtual ~CTriggerCounterP( void );

public:
	virtual bool Spawn( void ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
};
#endif //TRIGGERCOUNTERP_H