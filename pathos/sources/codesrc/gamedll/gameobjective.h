/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef GAMEOBJECTIVE_H
#define GAMEOBJECTIVE_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CGameObjective : public CPointEntity
{
public:
	enum
	{
		FL_NO_NOTIFICATIONS = (1<<0)
	};

	enum mode_t
	{
		OBJ_MODE_ADD = 0,
		OBJ_MODE_REMOVE
	};

public:
	explicit CGameObjective( edict_t* pedict );
	virtual ~CGameObjective( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;

private:
	Int32 m_mode;
};

#endif //GAMESETTAUNTBITS_H