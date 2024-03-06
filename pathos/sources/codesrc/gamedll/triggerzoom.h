/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef TRIGGERZOOM_H
#define TRIGGERZOOM_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CTriggerZoom : public CPointEntity
{
public:
	enum
	{
		FL_START_ON = (1<<0)
	};

	enum state_t
	{
		STATE_OFF = 0,
		STATE_INITIAL,
		STATE_BLENDING
	};

public:
	explicit CTriggerZoom( edict_t* pedict );
	virtual ~CTriggerZoom( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	virtual void SendInitMessage( const CBaseEntity* pPlayer ) override;

private:
	Int32 m_targetFOV;
	Int32 m_startFOV;
	Int32 m_state;

	Float m_duration;
	Double m_beginTime;

private:
	CBaseEntity* m_pPlayer;
};
#endif //TRIGGERZOOM_H