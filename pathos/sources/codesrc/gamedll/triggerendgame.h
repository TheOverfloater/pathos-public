/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef TRIGGERENDGAME_H
#define TRIGGERENDGAME_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CTriggerEndGame : public CPointEntity
{
public:
	explicit CTriggerEndGame( edict_t* pedict );
	virtual ~CTriggerEndGame( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	
private:
	string_t m_endGameCode;
};
#endif //TRIGGERENDGAME_H