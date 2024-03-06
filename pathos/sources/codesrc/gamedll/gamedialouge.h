/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef GAMEDIALOUGE_H
#define GAMEDIALOUGE_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CGameDialouge : public CPointEntity
{
public:
	enum
	{
		FL_LOOK_AT		= (1<<9),
		FL_START_OFF	= (1<<10),
		FL_REPEATABLE	= (1<<11)
	};

public:
	explicit CGameDialouge( edict_t* pedict );
	virtual ~CGameDialouge( void );

public:
	virtual bool Spawn( void ) override;
	virtual void Precache( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	virtual void SendInitMessage( const CBaseEntity* pPlayer ) override;

public:
	void EXPORTFN DialougeThink( void );

public:
	static CGameDialouge* CreateDialouge( const Char* pstrPath, const Vector& origin, Float radius, bool visibleOnly );

private:
	Float m_radius;
	bool m_isActive;
	Double m_beginTime;
	Float m_duration;
};

#endif //GAMEDIALOUGE_H