/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef EDICTMANAGER_H
#define EDICTMANAGER_H

#include "edict.h"
#include "sv_shared.h"
#include "entitydata.h"

/*
=======================
CEdictManager

=======================
*/
class CEdictManager
{
public:
	CEdictManager( void );
	~CEdictManager( void );

public:
	// Allocates all edicts
	void AllocEdicts( void );
	// Clears all edicts
	void ClearEdicts( void );

	// Allocates an edict
	edict_t* AllocEdict( void );
	// Deallocates an edict
	void FreeEdict( edict_t* pedict );
	// Returns an edict for an index
	edict_t* GetEdict( Int32 index );

	// Returns the number of edicts
	Uint32 GetNbEdicts( void ) const { return m_numEdicts; }
	// Returns the mas possible nb of ectis
	Uint32 GetMaxEdicts( void ) const { return m_edictsArray.size(); }

public:
	// Parses entdata, allocates entities
	bool LoadEntities( const Char* pstrEntdata );
	// Creates an edict and initializes it's class data
	edict_t* CreateEntity( const Char* pstrClassname );
	// Creates a player edict and initializes it's class data
	edict_t* CreatePlayerEntity( Uint32 player_index );

private:
	// Allocates a new identifier
	Uint32 AllocIdentifier( void );
	// Clears an edict for use
	static void ClearEdict( edict_t* pedict );

public:
	// Array of all available edicts
	CArray<edict_t> m_edictsArray;
	// Number of edicts total used so far
	Uint32 m_numEdicts;
	// Last identifier assigned to an entity
	Uint32 m_lastIdentifier;
};
extern CEdictManager gEdicts;
#endif //EDICTMANAGER_H