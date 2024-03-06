/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef TRIGGERPUSH_H
#define TRIGGERPUSH_H

#include "triggerentity.h"

//=============================================
//
//=============================================
class CTriggerPush : public CTriggerEntity
{
public:
	// Default speed
	static const Float DEFAULT_SPEED;

public:
	enum
	{
		FL_PUSH_ONCE		= (1<<0),
		FL_START_OFF		= (1<<1),
		FL_FRICTION_MOD		= (1<<5)
	};

public:
	explicit CTriggerPush( edict_t* pedict );
	virtual ~CTriggerPush( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	virtual void CallTouch( CBaseEntity* pOther ) override;

private:
	bool m_isActive;
};
#endif //TRIGGERPUSH_H