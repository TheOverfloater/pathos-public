/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef ENVLADDER_H
#define ENVLADDER_H

#include "animatingentity.h"
#include "ladder_shared.h"

class CPlayerEntity;

//=============================================
//
//=============================================
class CEnvLadder : public CAnimatingEntity
{
public:
	enum
	{
		FL_TOP_ACCESS	= (1<<0),
		FL_START_OFF	= (1<<1)
	};

public:
	explicit CEnvLadder( edict_t* pedict );
	virtual ~CEnvLadder( void );

public:
	virtual bool Spawn( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;
	virtual Int32 GetEntityFlags( void ) override { return (CAnimatingEntity::GetEntityFlags() | FL_ENTITY_PLAYER_USABLE | FL_ENTITY_TRANSITION); }
	virtual void InitEntity( void ) override;
	virtual bool ShouldSetBoundsOnRestore( void ) override { return false; }
	virtual usableobject_type_t GetUsableObjectType( void ) override;
	virtual void GetUseReticleMinsMaxs( Vector& outMins, Vector& outMaxs, CBaseEntity* pPlayer ) override;
	virtual void TraceAttack( CBaseEntity* pAttacker, Float damage, const Vector& direction, trace_t& tr, Int32 damageFlags ) override;

public:
	void EnterLadder( CBaseEntity* pPlayer );
	void ExitLadder( void );
	bool IsTopAccessible( CBaseEntity* pPlayer );

	void RepositionPlayer( CBaseEntity* pPlayer, Vector* porigin = nullptr, Vector* pangles = nullptr );
	bool GetExitVectors( ladder_exitpoints_t exit, Vector* porigin = nullptr, Vector* pangles = nullptr,  Float* pfldiff = nullptr );
	ladder_exitpoints_t CanUseExit( Vector* pangles, Vector* porigin, Float* pfltime, Float* pfldiff );

	ladder_verify_codes_t VerifyMove( const Vector& origin, CBaseEntity* pPlayer, Int32 direction );
	ladder_entrypoints_t GetEntryAnimation( void );

private:
	// Tells if ladder is active
	bool m_isActive;
	// Number of segments
	Int32 m_numSegments;
	// Pointer to player entity
	CBaseEntity* m_pPlayer;
};

#endif //ENVLADDER_H