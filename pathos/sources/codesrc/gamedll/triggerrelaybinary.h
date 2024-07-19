/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef TRIGGERRELAY_H
#define TRIGGERRELAY_H

#include "delayentity.h"

//=============================================
//
//=============================================
class CTriggerRelayBinary : public CDelayEntity
{
public:
	enum relay_state_t
	{
		RELAY_STATE_OFF = 0,
		RELAY_STATE_ON
	};

public:
	explicit CTriggerRelayBinary(edict_t* pedict);
	virtual ~CTriggerRelayBinary(void);

public:
	virtual bool Spawn(void) override;
	virtual void DeclareSaveFields(void) override;
	virtual bool KeyValue(const keyvalue_t& kv) override;
	virtual void CallUse(CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value) override;
	virtual Int32 GetEntityFlags(void) override { return CDelayEntity::GetEntityFlags() & ~FL_ENTITY_TRANSITION; }

public:
	Int32 m_triggerOnMode;
	Int32 m_triggerOffMode;
	Int32 m_relayState;
};
#endif //TRIGGERRELAY_H#pragma once
