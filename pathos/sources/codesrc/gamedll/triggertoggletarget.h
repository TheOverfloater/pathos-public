/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef TRIGGERTOGGLETARGET_H
#define TRIGGERTOGGLETARGET_H

#include "pointentity.h"

// Max entities remembered by trigger_toggletarget
static const Uint32 MAX_TOGGLETARGET_ENTITIES = 64;

//=============================================
//
//=============================================
class CTriggerToggleTarget : public CPointEntity
{
public:
	enum mode_t
	{
		TRIGGERTOGGLE_DISABLE = 0,
		TRIGGERTOGGLE_ENABLE
	};
public:
	explicit CTriggerToggleTarget( edict_t* pedict );
	virtual ~CTriggerToggleTarget( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	
private:
	Int32 m_triggerMode;
};

#endif //TRIGGERTOGGLETARGET_H