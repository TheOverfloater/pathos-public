/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef TRIGGERNPCPULL_H
#define TRIGGERNPCPULL_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CTriggerNPCPull : public CBaseEntity
{
public:
	struct pullednpc_t
	{
		pullednpc_t():
			pentity(nullptr),
			waspulled(false)
			{}

		CBaseEntity* pentity;
		bool waspulled;
	};

public:
	explicit CTriggerNPCPull( edict_t* pedict );
	virtual ~CTriggerNPCPull( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	virtual void InitEntity( void ) override;
	virtual void RemovePulledNPC( CBaseEntity* pNPC ) override;

public:
	void EXPORTFN PullThink( void );
	void ClearNPCs( void );

private:
	bool GetPullVector( CBaseEntity* pEntity, Vector& outVector, Vector& outPullPosition );

protected:
	// Determines if the object is active
	bool m_isActive;

	// Pull bbox mins
	Vector m_pullBBoxMins;
	// Pull bbox maxs
	Vector m_pullBBoxMaxs;

	// Final pull position
	Vector m_finalPullPosition;

	// Time when we started pulling
	Double m_beginTime;
	// Time until we shut down
	Double m_shutdownTime;

	// Amount of time until full pull force is reached
	Float m_fullForceTime;
	// Time over wich pull force fades out
	Float m_forceFadeOutTime;

	// List of NPCs being pulled
	CLinkedList<pullednpc_t> m_pulledNPCsList;
};
#endif //TRIGGERNPCPULL_H