/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef FUNCONVEYOR_H
#define FUNCONVEYOR_H

#include "funcwall.h"

//=============================================
//
//=============================================
class CFuncConveyor : public CFuncWall
{
public:
	enum
	{
		FL_VISUAL_ONLY	= (1<<0),
		FL_NOT_SOLID	= (1<<1),
		FL_INVISIBLE	= (1<<2)
	};
public:
	explicit CFuncConveyor( edict_t* pedict );
	virtual ~CFuncConveyor( void );

public:
	virtual bool Spawn( void ) override;
	virtual void Precache( void ) override { }
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
};
#endif //FUNCONVEYOR_H