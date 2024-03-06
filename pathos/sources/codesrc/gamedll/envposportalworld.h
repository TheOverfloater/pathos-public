/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef ENVPOSPORTALWORLD_H
#define ENVPOSPORTALWORLD_H

#include "pointentity.h"
#include "portal_shared.h"

//=============================================
//
//=============================================
class CEnvPosPortalWorld : public CPointEntity
{
public:
	explicit CEnvPosPortalWorld( edict_t* pedict );
	virtual ~CEnvPosPortalWorld( void );

public:
	virtual bool Spawn( void ) override;
	virtual void Precache( void ) override;
	virtual bool IsEnvPosPortalWorldEntity( void ) const override { return true; }
};
#endif //ENVPOSPORTALWORLD_H