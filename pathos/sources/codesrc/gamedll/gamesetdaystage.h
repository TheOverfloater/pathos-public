/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef GAMESETDAYSTAGE_H
#define GAMESETDAYSTAGE_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CGameSetDayStage : public CPointEntity
{
public:
	explicit CGameSetDayStage( edict_t* pedict );
	virtual ~CGameSetDayStage( void );

public:
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;

private:
	// Day stage
	Int32 m_dayStage;
};
#endif //GAMESETDAYSTAGE_H