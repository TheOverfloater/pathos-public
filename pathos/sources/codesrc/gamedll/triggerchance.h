/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef TRIGGERCHANCE_H
#define TRIGGERCHANCE_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CTriggerChance : public CPointEntity
{
public:
	explicit CTriggerChance( edict_t* pedict );
	virtual ~CTriggerChance( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	
private:
	Int32 m_chancePercentage;
};
#endif //TRIGGERCHANCE_H