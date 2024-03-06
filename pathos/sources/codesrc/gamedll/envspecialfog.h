/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef ENVSPECIALFOG_H
#define ENVSPECIALFOG_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CEnvSpecialFog : public CPointEntity
{
public:
	enum mode_t
	{
		MODE_ON = 0,
		MODE_OFF
	};

public:
	explicit CEnvSpecialFog( edict_t* pedict );
	virtual ~CEnvSpecialFog( void );

public:
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;

private:
	Int32 m_triggerMode;
};
#endif //ENVSPECIALFOG_H