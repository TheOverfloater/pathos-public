/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef ENTITYMANAGER_H 
#define ENTITYMANAGER_H

#include "entitydata.h"
#include "constants.h"

class CCVar;

struct cache_model_t;
struct cl_entity_t;
struct decalgroupentry_t;

/*
====================
CEntityManager

====================
*/
class CEntityManager
{
public:
	CEntityManager( void );
	~CEntityManager( void );

public:
	// Initializes the class
	void Init( void );
	// Clears the class
	void Clear( void );
	// Performs shutdown functions
	void Shutdown( void );

	// Sets up data
	void Setup( void );
	// Updates frame related info
	void Frame( void );

public:
	// Decal external entities
	void DecalExternalEntities( const Vector& vpos, const Vector& vnorm, decalgroupentry_t *texptr, Int32 flags );

	// Parses entities from the entity data
	void ParseEntities( void );
	// Releases the entity data
	void FreeEntityData( void );

	// Returns the full entity data list
	const CArray<entitydata_t>& GetEntityList( void ) const;

private:
	// Loads entvars
	void LoadEntVars( void );

	// Spawns an env_decal entity
	void Entity_EnvDecal( const entitydata_t& entity );
	// Spawns an env_cable entity
	void Entity_EnvCable( const entitydata_t& entity );
	// Spawns an env_sprite entity
	void Entity_EnvSprite( const entitydata_t& entity, entindex_t& entindex );
	// Spawns an env_elight entity
	void Entity_EnvELight( const entitydata_t& entity, entindex_t& entindex );
	// Spawns an env_model entity
	void Entity_EnvModel( const entitydata_t& entity, entindex_t& entindex );
	// Spawns an info_light_origin entity
	void Entity_InfoLightOrigin( const entitydata_t& entity, entindex_t& entityindex );

	// Retreives an entity by it's targetname
	const entitydata_t* FindEntityByTargetName( const Char* pstrClassName, const Char* pstrTargetName );

private:
	// Entity list extracted from BSP.
	CArray<entitydata_t> m_bspEntitiesArray;

	// Total amount of renderable entities.
	CArray<cl_entity_t> m_entitiesArray;

	// CVar controlling the rendering of client entities
	CCVar *m_pCvarDrawClientEntities;

	// Prompt history hash list
	CHashList m_promptHashList;
	
	// Last entity identifier used
	Uint32 m_lastIdentifierUsed;
};

extern CEntityManager gEntityManager;
#endif
