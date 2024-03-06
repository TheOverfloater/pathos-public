/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef ENVFUNNEL_H
#define ENVFUNNEL_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CEnvFunnel : public CPointEntity
{
public:
	// The default sprite model
	static const Char DEFAULT_SPRITE_NAME[];
public:
	enum
	{
		FL_REVERSE			= (1<<0),
		FL_REMOVE_ON_FIRE	= (1<<1)
	};
public:
	explicit CEnvFunnel( edict_t* pedict );
	virtual ~CEnvFunnel( void );

public:
	virtual bool Spawn( void ) override;
	virtual void Precache( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	
private:
	string_t m_spriteName;
	Int32 m_spriteModel;
};
#endif //ENVFUNNEL_H