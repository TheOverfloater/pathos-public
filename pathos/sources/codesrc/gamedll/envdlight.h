/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef ENVDLIGHT_H
#define ENVDLIGHT_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CEnvDLight : public CPointEntity
{
public:
	enum
	{
		FL_START_ON	= (1<<0),
		FL_NO_PVS	= (1<<1)
	};

public:
	explicit CEnvDLight( edict_t* pedict );
	virtual ~CEnvDLight( void );

public:
	virtual bool Spawn( void ) override;
	virtual void Precache( void ) override;
	virtual bool Restore( void ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;

	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual bool ShouldSetBoundsOnRestore( void ) override { return false; }
	virtual bool ShouldOverrideKeyValue( const Char* pstrKeyValue ) override;

public:
	void ManageLightStyle( void );
	void EXPORTFN OscillateThink( void );
	virtual void SetMinsMaxs( void );
	virtual void SetLightRenderFx( void );

private:
	// Only CEnvSpotlight can call this, not derived/parent classes
	void SetValues( const Vector& origin, const Vector& color, Uint32 radius );

public:
	static CEnvDLight* SpawnLight( const Vector& origin, const Vector& color, Uint32 radius );

private:
	string_t m_pattern;
	Uint32 m_style;
	bool m_interpolate;
	Float m_framerate;
	Uint32 m_oscillationh;
	Uint32 m_oscillationv;
	Vector m_baseOrigin;
};
#endif //ENVDLIGHT_H