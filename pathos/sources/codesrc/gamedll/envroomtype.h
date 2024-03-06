/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef ENVROOMTYPE_H
#define ENVROOMTYPE_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CEnvRoomType : public CPointEntity
{
public:
	explicit CEnvRoomType( edict_t* pedict );
	virtual ~CEnvRoomType( void );

public:
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
};
#endif //ENVROOMTYPE_H