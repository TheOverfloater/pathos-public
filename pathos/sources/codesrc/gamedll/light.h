#pragma once
/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef LIGHT_H
#define LIGHT_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CLight : public CPointEntity
{
public:
	// Starting index for switchable lightstyles
	static const Uint32 SWITCHABLE_LIGHT_FIRST_STYLEINDEX;

public:
	enum
	{
		FL_LIGHT_START_OFF	= (1<<0)
	};

public:
	explicit CLight( edict_t* pedict );
	virtual ~CLight( void );

public:
	virtual bool Spawn( void ) override;
	virtual bool Restore( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	virtual bool ShouldOverrideKeyValue( const Char* pstrKeyValue ) override;

private:
	void SetCurrentStyle( void );

private:
	Int32		m_styleIndex;
	string_t	m_stylePattern;
	bool		m_interpolatePattern;
	Float		m_patternFramerate;
	bool		m_isActive;
};
#endif //LIGHT_H