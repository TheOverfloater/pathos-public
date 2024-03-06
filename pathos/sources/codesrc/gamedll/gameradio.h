/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef GAMERADIO_H
#define GAMERADIO_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CGameRadio : public CPointEntity
{
public:
	enum
	{
		FL_REMOVE_ON_FIRE	= (1<<0),
		FL_NO_MUTE			= (1<<1)
	};

public:
	explicit CGameRadio( edict_t* pedict );
	virtual ~CGameRadio( void );

public:
	virtual bool Spawn( void ) override;
	virtual void Precache( void ) override;
	virtual bool Restore( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	
public:
	Double m_beginTime;
	CBaseEntity* m_pPlayer;
};
#endif //GAMERADIO_H