/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef GAMESTAMINAMODIFIER_H
#define GAMESTAMINAMODIFIER_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CGameStaminaModifier : public CPointEntity
{
public:
	explicit CGameStaminaModifier( edict_t* pedict );
	virtual ~CGameStaminaModifier( void );

public:
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;

private:
	Float m_sprintDrainMultiplier;
	Float m_normalMovemenetDrainFactor;
};

#endif //GAMESTAMINAMODIFIER_H
