/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef TRIGGERVACUUM_H
#define TRIGGERVACUUM_H

#include "pointentity.h"

class CNPCTestHull;

//=============================================
//
//=============================================
class CTriggerVacuum : public CBaseEntity
{
public:
	explicit CTriggerVacuum( edict_t* pedict );
	virtual ~CTriggerVacuum( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	virtual void InitEntity( void ) override;

public:
	void EXPORTFN PullThink( void );
	Vector GetPullPosition( CBaseEntity* pPlayer, const Vector& playerPosition );

protected:
	// Determines if the object is active
	bool m_isActive;
	// The test hull NPC we use for movement testing
	CNPCTestHull* m_pTestHullNPC;

	// Time when we started pulling
	Double m_beginTime;
	// Time until we shut down
	Double m_shutdownTime;

	// Amount of time until full pull force is reached
	Float m_fullForceTime;
	// Time over wich pull force fades out
	Float m_forceFadeOutTime;
};
#endif //TRIGGERVACUUM_H