/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef ENVFADE_H
#define ENVFADE_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CEnvFade : public CPointEntity
{
public:
	enum
	{
		FL_FADE_IN				= (1<<0),
		FL_MODULATE				= (1<<1),
		FL_ONLY_ONE_CLIENT		= (1<<2),
		FL_PERMANENT			= (1<<3),
		FL_STAYOUT				= (1<<4),
		FL_ACTIVATE_ON_START	= (1<<5)
	};

public:
	explicit CEnvFade( edict_t* pedict );
	virtual ~CEnvFade( void );

public:
	virtual void DeclareSaveFields( void ) override;
	virtual bool Spawn( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	virtual void SendInitMessage( const CBaseEntity* pPlayer ) override;

public:
	Float m_duration;
	Float m_holdtime;
	Double m_useTime;
	Int32 m_layer;
};
#endif //ENVFADE_H