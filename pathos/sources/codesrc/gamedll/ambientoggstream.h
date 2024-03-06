/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef AMBIENTOGGSTREAM_H
#define AMBIENTOGGSTREAM_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CAmbientOggStream : public CPointEntity
{
public:
	enum
	{
		FL_REMOVE_ON_TRIGGER	= (1<<0),
		FL_LOOP_MUSIC			= (1<<1)
	};

public:
	explicit CAmbientOggStream( edict_t* pedict );
	virtual ~CAmbientOggStream( void );

public:
	virtual bool Spawn( void ) override;
	virtual void Precache( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;

private:
	bool m_isActive;
	Float m_fadeInTime;
	Float m_fadeOutTime;
	Int32 m_channel;
};

#endif //AMBIENTOGGSTREAM_H