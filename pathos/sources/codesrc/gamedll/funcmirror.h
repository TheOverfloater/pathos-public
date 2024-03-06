/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef FUNCWALLTOGGLE_H
#define FUNCWALLTOGGLE_H

#include "funcwall.h"

//=============================================
//
//=============================================
class CFuncMirror : public CFuncWall
{
public:
	enum
	{
		FL_START_OFF = (1<<0)
	};

public:
	explicit CFuncMirror( edict_t* pedict );
	virtual ~CFuncMirror( void );

public:
	virtual bool Spawn( void ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
};
#endif //FUNCWALLTOGGLE_H