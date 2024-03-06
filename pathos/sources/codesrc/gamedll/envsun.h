/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef ENVSUN_H
#define ENVSUN_H

#include "pointentity.h"
#include "portal_shared.h"

//=============================================
//
//=============================================
class CEnvSun : public CPointEntity
{
public:
	enum
	{
		FL_START_ON = (1<<0),
		FL_PORTAL_SUN = (1<<1)
	};

public:
	explicit CEnvSun( edict_t* pedict );
	virtual ~CEnvSun( void );

public:
	virtual bool Spawn( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	virtual void SendInitMessage( const CBaseEntity* pPlayer ) override;

private:
	bool m_isActive;
	Float m_pitch;
	Float m_roll;
};
#endif //ENVSUN_H