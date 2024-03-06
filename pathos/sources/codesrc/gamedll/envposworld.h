/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef ENVPOSWORLD_H
#define ENVPOSWORLD_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CEnvPosWorld : public CPointEntity
{
public:
	enum
	{
		FL_START_ON			= (1<<0),
		FL_SEND_OFF_MESSAGE	= (1<<1)
	};

public:
	explicit CEnvPosWorld( edict_t* pedict );
	virtual ~CEnvPosWorld( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	virtual void SendInitMessage( const CBaseEntity* pPlayer ) override;
	virtual bool Restore( void ) override;

private:
	Float m_fogStartDist;
	Float m_fogEndDist;
	bool m_dontAffectSky;
	bool m_isActive;
	string_t m_skyTextureName;
	Int32 m_skyTextureSetIndex;
};

#endif //ENVPOSWORLD_H