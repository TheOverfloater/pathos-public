/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef ENVBLOOD_H
#define ENVBLOOD_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CEnvBlood : public CPointEntity
{
public:
	enum
	{
		FL_RANDOM	= (1<<0),
		FL_STREAM	= (1<<1),
		FL_PLAYER	= (1<<2),
		FL_DECAL	= (1<<3)
	};
	enum color_t
	{
		COLOR_RED = 0,
		COLOR_YELLOW,
	};
public:
	explicit CEnvBlood( edict_t* pedict );
	virtual ~CEnvBlood( void );

public:
	virtual bool Spawn( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bloodcolor_t GetBloodColor( void ) override;

public:
	Float GetBloodAmount( void ) const;

	void SetBloodColor( bloodcolor_t color );
	void SetBloodAmount( Float amount );

	Vector GetBloodDirection( void ) const;
	Vector GetBloodPosition( CBaseEntity* pActivator ) const;

private:
	Int32 m_bloodColor;
	Float m_bloodAmount;
};
#endif //ENVBLOOD_H