/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef TRIGGERGLOBALDELAYED_H
#define TRIGGERGLOBALDELAYED_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CTriggerGlobalDelayed : public CPointEntity
{
public:
	enum mode_t
	{
		MODE_SET = 0,
		MODE_CLEAR
	};

public:
	explicit CTriggerGlobalDelayed( edict_t* pedict );
	virtual ~CTriggerGlobalDelayed( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;

private:
	Int32 m_mode;
	Float m_duration;
};

#endif //GAMESETTAUNTBITS_H