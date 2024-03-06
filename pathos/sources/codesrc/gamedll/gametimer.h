/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef GAMETIMER_H
#define GAMETIMER_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CGameTimer : public CPointEntity
{
public:
	enum mode_t
	{
		TIMER_MODE_SET = 0,
		TIMER_MODE_CLEAR
	};

public:
	explicit CGameTimer( edict_t* pedict );
	virtual ~CGameTimer( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;

private:
	Int32 m_mode;
	Float m_duration;
};

#endif //GAMESETTAUNTBITS_H