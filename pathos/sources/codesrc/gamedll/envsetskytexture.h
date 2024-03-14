/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef ENVSETSKYTEXTURE_H
#define ENVSETSKYTEXTURE_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CEnvSetSkyTexture : public CPointEntity
{
public:
	enum
	{
		FL_START_ON			= (1<<0),
		FL_NO_OFF_MESSAGE	= (1<<1)
	};

public:
	explicit CEnvSetSkyTexture( edict_t* pedict );
	virtual ~CEnvSetSkyTexture( void );

public:
	virtual bool Spawn( void ) override;
	virtual bool Restore( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	virtual void SendInitMessage( const CBaseEntity* pPlayer ) override;

private:
	bool m_isActive;
};

#endif //ENVSETSKYTEXTURE_H