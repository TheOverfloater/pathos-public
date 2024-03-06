/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef AMMOGLOCKCLIP_H
#define AMMOGLOCKCLIP_H

#include "playerammo.h"

//=============================================
//
//=============================================
class CAmmoGlockClip : public CPlayerAmmo
{
public:
	explicit CAmmoGlockClip( edict_t* pedict );
	virtual ~CAmmoGlockClip( void );

public:
	virtual void SetSpawnProperties( void ) override;
	virtual Int32 GetAmmoAmount( void ) override;
	virtual Int32 GetMaxAmmo( void ) override;
	virtual const Char* GetAmmoTypeName( void ) override;
};
#endif //AMMOGLOCKCLIP_H