/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/
// Code by valina354

#ifndef ENVCREDITS_H
#define ENVCREDITS_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CEnvCredits : public CPointEntity
{
public:
	// Default scroll speed
	static const Float DEFAULT_SCROLL_SPEED;

public:
	explicit CEnvCredits( edict_t* pedict );
	virtual ~CEnvCredits( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;

private:
	Float m_scrollSpeed;
};
#endif //ENVCREDITS_H