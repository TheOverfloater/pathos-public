/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef ENVPARTICLESYSTEM_H
#define ENVPARTICLESYSTEM_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CEnvParticleSystem : public CPointEntity
{
public:
	enum
	{
		FL_START_ON			= (1<<0),
		FL_REMOVE_ON_FIRE	= (1<<1),
		FL_NOT_TOGGLED		= (1<<2)
	};

public:
	explicit CEnvParticleSystem( edict_t* pedict );
	virtual ~CEnvParticleSystem( void );

public:
	virtual bool Spawn( void ) override;
	virtual void Precache( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool Restore( void ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	virtual void SendInitMessage( const CBaseEntity* pPlayer ) override;

private:
	bool m_isActive;
	bool m_wasSent;
};

#endif //ENVPARTICLESYSTEM_H