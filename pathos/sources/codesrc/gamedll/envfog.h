/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef ENVFOG_H
#define ENVFOG_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CEnvFog : public CPointEntity
{
public:
	enum
	{
		FL_START_ON			= (1<<0),
		FL_NO_OFF_MESSAGE	= (1<<1),
		FL_CLEAR_ON_FIRST	= (1<<2)
	};
public:
	explicit CEnvFog( edict_t* pedict );
	virtual ~CEnvFog( void );

public:
	virtual bool Spawn( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void DeclareSaveFields( void ) override;

	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	virtual void SendInitMessage( const CBaseEntity* pPlayer ) override;

public:
	static void ClearFogGlobals( void );
	static void UpdateFogGlobals( void );
	static bool FogCull( const edict_t& client, const edict_t& entity );
	static const Float GetFogEndDistance( void ) { return g_fogEndDist; }
	static void SetFogCullParams( Float endDistance, Float blendTime, bool affectSky );

protected:
	Float m_startDistance;
	Float m_endDistance;
	Float m_blendTime;
	
	bool m_isActive;
	bool m_dontAffectSky;

protected:
	// Current fog end distance
	static Uint32 g_fogEndDist;
	// Fog ideal distance to set
	static Uint32 g_fogIdealEndDist;
	// Time until fog blending is done
	static Double g_fogBlendTime;
	// TRUE if fog should affect skybox
	static bool g_fogAffectSky;
};

#endif //ENVMODEL_H
