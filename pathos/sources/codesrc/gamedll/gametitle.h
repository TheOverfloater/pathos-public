/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef GAMETITLE_H
#define GAMETITLE_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CGameTitle : public CPointEntity
{
public:
	// Game title message name
	static const Char TITLE_MESSAGE_NAME[];

public:
	explicit CGameTitle( edict_t* pedict );
	virtual ~CGameTitle( void );

public:
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
};
#endif //GAMETITLE_H