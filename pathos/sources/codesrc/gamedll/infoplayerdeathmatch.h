/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef INFOPLAYEREDEATCHMATCH_H
#define INFOPLAYEREDEATCHMATCH_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CInfoPlayerDeathMatch : public CPointEntity
{
public:
	explicit CInfoPlayerDeathMatch( edict_t* pedict );
	virtual ~CInfoPlayerDeathMatch( void );

public:
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual bool IsTriggered( const CBaseEntity* pentity ) const override;
};

#endif //INFOPLAYEREDEATCHMATCH_H