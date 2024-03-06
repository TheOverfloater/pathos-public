/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef ENVBLUR
#define ENVBLUR

#include "pointentity.h"

//=============================================
//
//=============================================
class CEnvBlur : public CPointEntity
{
public:
	enum
	{
		FL_START_ON = (1<<1)
	};
public:
	explicit CEnvBlur( edict_t* pedict );
	virtual ~CEnvBlur( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	virtual void SendInitMessage( const CBaseEntity* pPlayer ) override;

private:
	bool m_isActive;
	Float m_blurFade;
};
#endif //ENVBLUR