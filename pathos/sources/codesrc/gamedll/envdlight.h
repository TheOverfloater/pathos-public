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
	struct customlightstyle_t
	{
		customlightstyle_t():
			interpolate(false),
			styleindex(0),
			framerate(0)
			{
			}

		CString pattern;
		bool interpolate;
		Uint32 styleindex;
		Float framerate;
	};

public:
	// start lightstyle index for dynlights
	static const Uint32 CUSTOM_LIGHTSTYLE_START_INDEX;
	// Default framerate value
	static const Float DEFAULT_LIGHTSTYLE_FRAMERATE;

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
	static void AddCustomLightStyle( const Char* pstrpattern, bool interpolate, Float framerate, Uint32& styleindex );
	static void SendLightStylesToClient( edict_t* pPlayer );
	static void ResetLightStyles( void );

private:
	string_t m_pattern;
	Uint32 m_style;
	bool m_interpolate;
	Float m_framerate;
	Uint32 m_oscillationh;
	Uint32 m_oscillationv;
	Vector m_baseOrigin;

private:
	// Array of custom lightstyles
	static CArray<customlightstyle_t> g_customLightStylesArray;
	// Next available lightstyle index
	static Uint32 g_nextLightStyleIndex;
};
#endif //ENVDLIGHT_H