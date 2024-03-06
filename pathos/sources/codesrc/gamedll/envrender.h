/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef ENVRENDER_H
#define ENVRENDER_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CEnvRender : public CPointEntity
{
public:
	enum
	{
		FL_DONT_SET_RENDERFX	= (1<<0),
		FL_DONT_SET_RENDERAMT	= (1<<1),
		FL_DONT_SET_RENDERMODE	= (1<<2),
		FL_DONT_SET_RENDERCOLOR	= (1<<3)
	};

public:
	explicit CEnvRender( edict_t* pedict );
	virtual ~CEnvRender( void );

public:
	virtual bool Spawn( void ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
};
#endif //ENVRENDER_H