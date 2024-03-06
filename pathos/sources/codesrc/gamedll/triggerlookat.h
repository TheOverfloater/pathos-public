/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef TRIGGERLOOKAT_H
#define TRIGGERLOOKAT_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CTriggerLookAt : public CPointEntity
{
public:
	enum
	{
		FL_START_OFF	= (1<<0),
		FL_IGNORE_GLASS	= (1<<1)
	};

public:
	explicit CTriggerLookAt( edict_t* pedict );
	virtual ~CTriggerLookAt( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	
public:
	void EXPORTFN LookAtThink( void );

private:
	Float m_radius;
	Double m_lookTime;
	Double m_accumulatedTime;

	bool m_isActive;
};
#endif //TRIGGERLOOKAT_H