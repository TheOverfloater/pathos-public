/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef TRIGGERREPEAT_H
#define TRIGGERREPEAT_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CTriggerRepeat : public CPointEntity
{
public:
	explicit CTriggerRepeat( edict_t* pedict );
	virtual ~CTriggerRepeat( void );

public:
	virtual bool Spawn( void ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	
public:
	void EXPORTFN RepeatThink( void );
};
#endif //TRIGGERREPEAT_H