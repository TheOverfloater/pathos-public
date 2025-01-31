/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef ENVHUDPICKUPMSG_H
#define ENVHUDPICKUPMSG_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CEnvHudPickupMsg : public CPointEntity
{
public:
	enum
	{
		FL_ONLY_ONCE	= (1<<0)
	};

public:
	explicit CEnvHudPickupMsg( edict_t* pedict );
	virtual ~CEnvHudPickupMsg( void );

public:
	virtual bool Spawn( void ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
};
#endif //ENVMESSAGE_H