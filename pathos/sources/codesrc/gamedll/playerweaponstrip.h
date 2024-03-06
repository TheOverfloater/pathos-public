/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef PLAYERWEAPONSTRIP_H
#define PLAYERWEAPONSTRIP_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CPlayerWeaponStrip : public CPointEntity
{
public:
	explicit CPlayerWeaponStrip( edict_t* pedict );
	virtual ~CPlayerWeaponStrip( void );

public:
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
};
#endif //PLAYERWEAPONSTRIP_H