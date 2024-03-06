/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef TRIGGERFORCECLOSE_H
#define TRIGGERFORCECLOSE_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CTriggerForceClose : public CPointEntity
{
public:
	explicit CTriggerForceClose( edict_t* pedict );
	virtual ~CTriggerForceClose( void );

public:
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
};
#endif //TRIGGERFORCECLOSE_H
