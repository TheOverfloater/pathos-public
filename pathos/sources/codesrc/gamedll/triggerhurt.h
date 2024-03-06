/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef TRIGGERONCE_H
#define TRIGGERONCE_H

#include "triggerentity.h"

//=============================================
//
//=============================================
class CTriggerHurt : public CTriggerEntity
{
public:
	// Damage delay time
	static const Float DEFAULT_DMG_DELAY;

public:
	enum
	{
		FL_TARGET_ONCE		= (1<<0),
		FL_START_OFF		= (1<<1),
		FL_NO_CLIENTS		= (1<<2),
		FL_FIRE_ONLY_CLIENT	= (1<<3),
		FL_ONLY_CLIENTS		= (1<<4),
		FL_HURT_ONLY_ONCE	= (1<<5)
	};
public:
	explicit CTriggerHurt( edict_t* pedict );
	virtual ~CTriggerHurt( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;

public:
	void EXPORTFN HurtTouch( CBaseEntity* pOther );

private:
	bool m_isActive;
	Double m_nextDamageTime;
	Double m_damageTime;
	Float m_dmgAmount;
	Float m_dmgDelay;
	Int32 m_bitsDamageInflict;
	Int32 m_playerDamageBits;
};
#endif //TRIGGERONCE_H