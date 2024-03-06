/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef TRIGGERSPEEDUP_H
#define TRIGGERSPEEDUP_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CTriggerSpeedup : public CPointEntity
{
public:
	explicit CTriggerSpeedup( edict_t* pedict );
	virtual ~CTriggerSpeedup( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	
public:
	void EXPORTFN SpeedupThink( void );

private:
	Float m_targetSpeed;
	Float m_startSpeed;
	Float m_duration;
	Double m_beginTime;
};

#endif //TRIGGERSPEEDUP_H