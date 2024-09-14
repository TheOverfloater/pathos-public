/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef LIGHTENVIRONMENT_H
#define LIGHTENVIRONMENT_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CLightEnvironment : public CPointEntity
{
public:
	enum lightenv_mode_t
	{
		MODE_NORMAL = 0,
		MODE_NIGHT,
		MODE_DAYLIGHT_RETURN,
		MODE_DAYLIGHT_RETURN_AND_NIGHT
	};

public:
	explicit CLightEnvironment( edict_t* pedict );
	virtual ~CLightEnvironment( void );

public:
	virtual bool Spawn( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool IsLightEnvironment( void ) const override { return true; }
	virtual void SendInitMessage( const CBaseEntity* pPlayer ) override;
	virtual bool SetLightEnvValues( daystage_t daystage ) override;

public:
	static void CheckALDFile( void );

public:
	// Sunlight direction
	Vector m_sunlightDirection;
	// Sunlight color
	Vector m_sunlightColor;
	// Sunlight intensity
	Int32 m_sunlightIntensity;
	// Operational mode of light_env
	Int32 m_lightEnvMode;

private:
	// TRUE if we have an ALD file present for the map
	static bool g_isALDFilePresent;
};

#endif //LIGHTENVIRONMENT_H