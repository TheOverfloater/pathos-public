/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef ENVSYNCANIMATION_H
#define ENVSYNCANIMATION_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CEnvSyncAnimation : public CPointEntity
{
public:
	explicit CEnvSyncAnimation( edict_t* pedict );
	virtual ~CEnvSyncAnimation( void );

public:
	virtual bool Spawn( void ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	virtual void DeclareSaveFields( void ) override;

private:
	// True of sync is set
	bool m_isSyncEnabled;
};
#endif //ENVSYNCANIMATION_H