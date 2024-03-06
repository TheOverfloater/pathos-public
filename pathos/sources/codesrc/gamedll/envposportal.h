/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef ENVPOSPORTAL_H
#define ENVPOSPORTAL_H

#include "pointentity.h"
#include "portal_shared.h"

//=============================================
//
//=============================================
class CEnvPosPortal : public CPointEntity
{
public:
	explicit CEnvPosPortal( edict_t* pedict );
	virtual ~CEnvPosPortal( void );

public:
	virtual bool Spawn( void ) override;
	virtual void Precache( void ) override;
	virtual bool Restore( void ) override;
	virtual void FreeEntity( void ) override;
	virtual void InitEntity( void ) override;
	virtual void DeclareSaveFields( void ) override;
	virtual bool KeyValue( const keyvalue_t& kv ) override;

public:
	virtual bool IsEnvPosPortalEntity( void ) const override { return true; }
	virtual void AddPortalSurfaceEntity( const edict_t* pedict ) override;
	virtual bool CheckPortalBBox( const edict_t* pedict ) const override;

	virtual Uint32 GetNbPortalSurfaces( void ) const override;
	virtual const edict_t* GetPortalSurfaceByIndex( Uint32 index ) const override;

	virtual void SetPVSData( void ) override;
	virtual const byte* GetPVSData( void ) const override;

public:
	// Do not save-restore this
	byte* m_pPVSData;

	// Do not save-restore
	const edict_t* m_pPortalSurfaces[MAX_PORTAL_ENTITIES];
	Int32 m_numPortalSurfaces;

	// Sky texture name
	string_t m_skyTextureName;
};
#endif //ENVPOSPORTAL_H