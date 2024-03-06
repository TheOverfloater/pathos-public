/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef PLAYERDUALWEAPON_H
#define PLAYERDUALWEAPON_H

#include "playerweapon.h"

//=============================================
//
//=============================================
class CPlayerDualWeapon : public CPlayerWeapon
{
public:
	explicit CPlayerDualWeapon( edict_t* pedict );
	virtual ~CPlayerDualWeapon( void );

public:
	virtual bool AddDuplicate( CPlayerWeapon* pOriginal ) override;
	virtual bool AddFullAmmoDual( CPlayerWeapon* pcheckweapon ) override;
	virtual bool AddAmmo( Int32 count, const Char* pstrname, Int32 maxclip, Int32 maxcarry, CBaseEntity* pWeapon ) override;

	virtual void FinishReload( void ) override;

public:
	virtual Uint32 GetMaxClipSingle( void ) = 0;
};
#endif //PLAYERDUALWEAPON_H