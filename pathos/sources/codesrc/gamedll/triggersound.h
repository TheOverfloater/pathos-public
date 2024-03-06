/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef TRIGGERSOUND_H
#define TRIGGERSOUND_H

#include "triggerentity.h"

//=============================================
//
//=============================================
class CTriggerSound : public CTriggerEntity
{
public:
	explicit CTriggerSound( edict_t* pedict );
	virtual ~CTriggerSound( void );

public:
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallTouch( CBaseEntity* pOther ) override;

public:
	Int32 m_roomType;
	string_t m_masterEntity;
};

#endif //TRIGGERSOUND_H
