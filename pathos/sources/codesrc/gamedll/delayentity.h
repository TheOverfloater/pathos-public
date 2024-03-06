/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef DELAYENTITY_H
#define DELAYENTITY_H

#include "baseentity.h"

//=============================================
//
//=============================================
class CDelayEntity : public CBaseEntity
{
public:
	explicit CDelayEntity( edict_t* pedict );
	virtual ~CDelayEntity( void );

public:
	// Declares save/restore fields
	virtual void DeclareSaveFields( void ) override;
	// Manages keyvalues
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	// Triggers targets
	virtual void UseTargets( CBaseEntity* pActivator, usemode_t useMode, Float value, string_t target = NO_STRING_VALUE ) override;

public:
	// Performs delayed triggering
	void EXPORTFN DelayThink( void );

protected:
	// Delay before trigger
	Float m_delay;
	// Entities to kill on trigger
	string_t m_killTarget;
	// Use mode to trigger with
	usemode_t m_useMode;
};
#endif //DELAYENTITY_H