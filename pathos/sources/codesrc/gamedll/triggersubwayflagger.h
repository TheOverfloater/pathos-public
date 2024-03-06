/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef TRIGGERSUBWAYFLAGGER_H
#define TRIGGERSUBWAYFLAGGER_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CTriggerSubwayFlagger : public CPointEntity
{
public:
	enum
	{
		SW_FLAG_BERGEN_ST = 0,
		SW_FLAG_IBMANN_ST,
		SW_FLAG_ECKHART_ST,
		SW_FLAG_MARSHALL_ST,
		SW_FLAG_HACKED
	};
public:
	explicit CTriggerSubwayFlagger( edict_t* pedict );
	virtual ~CTriggerSubwayFlagger( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;

public:
	Int32	m_flag;
};
#endif //TRIGGERSUBWAYFLAGGER_H