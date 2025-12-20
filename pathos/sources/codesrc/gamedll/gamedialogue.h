/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef GAMEDIALOGUE_H
#define GAMEDIALOGUE_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CGameDialogue : public CPointEntity
{
public:
	enum
	{
		FL_LOOK_AT		= (1<<9),
		FL_START_OFF	= (1<<10),
		FL_REPEATABLE	= (1<<11)
	};

public:
	explicit CGameDialogue( edict_t* pedict );
	virtual ~CGameDialogue( void );

public:
	virtual bool Spawn( void ) override;
	virtual void Precache( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	virtual void SendInitMessage( const CBaseEntity* pPlayer ) override;

public:
	void EXPORTFN DialogueThink( void );

public:
	static CGameDialogue* CreateDialogue( const Char* pstrPath, const Vector& origin, Float radius, bool visibleOnly );

private:
	Float m_radius;
	bool m_isActive;
	Double m_beginTime;
	Float m_duration;
};

#endif //GAMEDIALOGUE_H