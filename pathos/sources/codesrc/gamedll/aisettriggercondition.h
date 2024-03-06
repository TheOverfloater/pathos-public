/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef AISETTRIGGERCONDITION_H
#define AISETTRIGGERCONDITION_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CAISetTriggerCondition : public CPointEntity
{
public:
	// Number of AI condition triggers
	static const Uint32 NUM_CONDITION_TRIGGERS;

public:
	explicit CAISetTriggerCondition( edict_t* pedict );
	virtual ~CAISetTriggerCondition( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	
public:
	Int32 m_triggerCondition;
	string_t m_triggerTarget;
	Int32 m_conditionIndex;
};
#endif //AISETTRIGGERCONDITION_H