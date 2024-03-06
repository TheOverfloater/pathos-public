/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef ENVMESSAGE_H
#define ENVMESSAGE_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CEnvMessage : public CPointEntity
{
public:
	enum
	{
		FL_ONLY_ONCE	= (1<<0),
		FL_ALL_PLAYERS	= (1<<1)
	};

public:
	enum
	{
		ATTEN_SMALL = 0,
		ATTEN_MEDIUM,
		ATTEN_LARGE,
		ATTEN_GLOBAL
	};
public:
	explicit CEnvMessage( edict_t* pedict );
	virtual ~CEnvMessage( void );

public:
	virtual bool Spawn( void ) override;
	virtual void Precache( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	
private:
	string_t m_msgSound;
	Float m_msgVolume;
	Float m_msgAttenuation;
};
#endif //ENVMESSAGE_H