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
class CTriggerOnce : public CTriggerEntity
{
public:
	enum
	{
		FL_ALLOW_NPCS	= (1<<0),
		FL_NO_CLIENTS	= (1<<1),
		FL_PUSHABLES	= (1<<2)
	};

public:
	explicit CTriggerOnce( edict_t* pedict );
	virtual ~CTriggerOnce( void );

public:
	virtual bool Spawn( void ) override;

public:
	void EXPORTFN TriggerTouch( CBaseEntity* pOther );

public:
	virtual void PostTrigger( void );
};
#endif //TRIGGERONCE_H