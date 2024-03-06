/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef AISETFOLLOWTARGET_H
#define AISETFOLLOWTARGET_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CAISetFollowTarget : public CPointEntity
{
public:
	explicit CAISetFollowTarget( edict_t* pedict );
	virtual ~CAISetFollowTarget( void );

public:
	virtual bool Spawn( void ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
};
#endif //AISETFOLLOWTARGET_H