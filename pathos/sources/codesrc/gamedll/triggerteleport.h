/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef TRIGGERTELEPORT_H
#define TRIGGERTELEPORT_H

#include "triggerentity.h"

//=============================================
//
//=============================================
class CTriggerTeleport : public CTriggerEntity
{
public:
	enum
	{
		FL_ALLOW_NPCS		= (1<<0),
		FL_NO_CLIENTS		= (1<<1),
		FL_KEEP_ANGLES		= (1<<3),
		FL_RELATIVE			= (1<<4)
	};
public:
	explicit CTriggerTeleport( edict_t* pedict );
	virtual ~CTriggerTeleport( void );

public:
	virtual bool Spawn( void ) override;
	virtual void CallTouch( CBaseEntity* pOther ) override;
};
#endif //TRIGGERONCE_H