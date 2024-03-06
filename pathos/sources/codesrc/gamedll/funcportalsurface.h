/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef FUNCPORTALSURFACE_H
#define FUNCPORTALSURFACE_H

#include "pointentity.h"

//=============================================
//
//=============================================
class CFuncPortalSurface : public CBaseEntity
{
public:
	enum
	{
		FL_START_OFF = (1<<0),
		FL_NOT_SOLID = (1<<1)
	};
public:
	explicit CFuncPortalSurface( edict_t* pedict );
	virtual ~CFuncPortalSurface( void );

public:
	virtual bool Spawn( void ) override;
	virtual bool Restore( void ) override;
	virtual void CallUse( CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value ) override;

	virtual Int32 GetEntityFlags( void ) override { return CBaseEntity::GetEntityFlags() & ~FL_ENTITY_TRANSITION; }
	virtual bool IsFuncPortalSurfaceEntity( void ) const override { return true; }
	virtual const CBaseEntity* GetEnvPosPortalEntity( void ) const override;
	virtual void InitEntity( void ) override;
	
public:
	static void ClearPortalSurfaceList( void );
	static void AddPortalSurfaceEntity( CBaseEntity* pMonitor );

	static Uint32 GetNbPortalSurfaces( void );
	static const CBaseEntity* GetPortalSurfaceByIndex( Uint32 index );
	static bool CheckVISForEntity( const edict_t* pclient, const edict_t* pedict, const byte* pset );

private:
	static CArray<CBaseEntity*> m_pPortalSurfacesArray;
};
#endif //FUNCPORTALSURFACE_H