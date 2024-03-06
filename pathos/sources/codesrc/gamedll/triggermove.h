/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef TRIGGERMOVE_H
#define TRIGGERMOVE_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CTriggerMove : public CPointEntity
{
public:
	enum
	{
		FL_COPY_ANGLES = (1<<0)
	};
public:
	explicit CTriggerMove( edict_t* pedict );
	virtual ~CTriggerMove( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	
public:
	string_t m_landmarkEntity;
	string_t m_groundEntity;
};
#endif //TRIGGERMOVE_H