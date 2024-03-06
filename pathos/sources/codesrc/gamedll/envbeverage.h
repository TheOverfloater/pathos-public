/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef ENVBEVERAGE_H
#define ENVBEVERAGE_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CEnvBeverage : public CPointEntity
{
public:
	// Default number of cans in an env_beverage
	static const Uint32 DEFAULT_NB_BEVERAGES;
	
public:
	enum
	{
		CAN_TYPE1 = 0,
		CAN_TYPE2,
		CAN_TYPE3,
		CAN_TYPE4,
		CAN_TYPE5,
		CAN_TYPE6,
		CAN_RANDOM
	};

public:
	explicit CEnvBeverage( edict_t* pedict );
	virtual ~CEnvBeverage( void );

public:
	virtual bool Spawn( void ) override;
	virtual void Precache( void ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	virtual void ChildEntityRemoved( CBaseEntity* pEntity ) override;
};
#endif //ENVBEVERAGE_H