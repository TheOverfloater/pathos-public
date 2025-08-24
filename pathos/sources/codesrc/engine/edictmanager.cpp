/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "com_math.h"
#include "file.h"

#include "edict.h"
#include "edictmanager.h"
#include "enginestate.h"
#include "sv_main.h"
#include "system.h"
#include "sv_world.h"
#include "sv_entities.h"
#include "vbmtrace.h"

//
// Some of the code here was written while referencing Quake 1 and ReHLDS. I want to thank Id Software
// and dreamstalker for their invaluable work.

// Object declaration
CEdictManager gEdicts;

//=============================================
// @brief Default constructor
//
//=============================================
CEdictManager::CEdictManager( void ):
	m_numEdicts(0),
	m_lastIdentifier(1)
{
}

//=============================================
// @brief Destructor
//
//=============================================
CEdictManager::~CEdictManager( void )
{
	ClearEdicts();
}

//=============================================
// @brief
//
//=============================================
void CEdictManager::AllocEdicts( void )
{
	// Clear existing array
	ClearEdicts();

	// Determine the max size
	Uint32 maxEdicts;
	if(ens.arg_max_edicts != 0)
	{
		if(ens.arg_max_edicts > MAX_SERVER_ENTITIES)
			maxEdicts = MAX_SERVER_ENTITIES;
		else
			maxEdicts = ens.arg_max_edicts;
	}
	else
		maxEdicts = DEFAULT_MAX_EDICTS;

	// Allocate edicts
	m_edictsArray.resize(maxEdicts);
	m_numEdicts = svs.maxclients+1;

	for(Uint32 i = 0; i < maxEdicts; i++)
	{
		m_edictsArray[i].entindex = i;
		m_edictsArray[i].state.entindex = i;
	}
}

//=============================================
// @brief
//
//=============================================
void CEdictManager::ClearEdicts( void )
{
	// Clear existing array
	if(m_edictsArray.empty())
		return;

	for(Uint32 i = 0; i < m_edictsArray.size(); i++)
	{
		edict_t* pedict = &m_edictsArray[i];
		if(pedict->free)
			continue;

		FreeEdict(pedict, EDICT_REMOVED_GAMECLEAR);
	}

	m_numEdicts = svs.maxclients+1;
	m_lastIdentifier = 1;
}

//=============================================
// @brief
//
//=============================================
bool CEdictManager::LoadEntities( const Char* pstrEntdata )
{
	if(!pstrEntdata)
		return false;

	Char token[MAX_PARSE_LENGTH];
	Char value[MAX_PARSE_LENGTH];

	Int32 eindex = 0;
	// Parse the entdata strings
	const Char* pstr = pstrEntdata;
	while(pstr)
	{
		// Parse the opening brace
		pstr = Common::Parse(pstr, token);
		if(qstrcmp(token, "{"))
		{
			Con_EPrintf("Map entity data is invalid.\n");
			return false;
		}

		// Holds entity key-value pairs
		entitydata_t entity;

		// Parse the keyvalues
		while(true)
		{
			// Parse the next token
			pstr = Common::Parse(pstr, token);
			if(!qstrlen(token))
			{
				Con_EPrintf("Map entity data is invalid.\n");
				return false;
			}
			
			// Break if we reached the end
			if(!qstrcmp(token, "}"))
				break;

			// Read the value
			pstr = Common::Parse(pstr, value);
			if(!pstr || !qstrlen(value))
			{
				Con_EPrintf("Map entity data is invalid.\n");
				return false;
			}

			// Do not allow mappers to specify "rendertype"
			if(!qstrcmp(token, "rendertype"))
			{
				Con_Printf("Entity property 'rendertype' not allowed as keyvalue.\n");
				continue;
			}

			// Allow game dll to format keyvalues
			svs.dllfuncs.pfnFormatKeyValue(token, value, MAX_PARSE_LENGTH);

			// Handle classname specially
			if(!qstrcmp(token, "classname"))
				entity.classname = value;

			// Handle modelname specially
			if(!qstrcmp(token, "model"))
				qstrcpy(token, "modelname");

			// Manage "angle" keyvalue
			if(!qstrcmp(token, "angle"))
			{
				Float flvalue = SDL_atof(value);
				if(flvalue >= 0.0)
					sprintf(value, "0 %f 0", flvalue);
				else if(static_cast<Int32>(flvalue) == -1)
					qstrcpy(value, "-90 0 0");
				else
					qstrcpy(value, "90 0 0");

				// Rename token to "angles"
				qstrcpy(token, "angles");
			}

			// Add the keyvalue
			keyvalue_t* pkv = new keyvalue_t;
			qstrcpy(pkv->keyname, token);
			qstrcpy(pkv->value, value);

			entity.values.push_back(pkv);
		}
		eindex++;

		// Initialize the entity
		edict_t* pedict = nullptr;
		if(!qstrcmp(entity.classname, "worldspawn"))// Worldspawn is handled specially
			pedict = GetEdict(WORLDSPAWN_ENTITY_INDEX);
		else
			pedict = AllocEdict();
		
		// Make sure we have enough edicts
		if(!pedict)
			return false;

		// Set this to false;
		pedict->free = false;

		// Initialize private data
		if(!SV_InitPrivateData(pedict, entity.classname.c_str()))
		{
			Con_Printf("[flags=onlyonce_game]Failed to allocate private data for entity '%s'.\n", entity.classname.c_str());
			FreeEdict(pedict, EDICT_REMOVED_AT_INIT);
			continue;
		}

		// Initialize keyvalues
		for(Uint32 i = 0; i < entity.values.size(); i++)
		{
			if(!svs.dllfuncs.pfnKeyValue(pedict, *entity.values[i]))
				Con_DPrintf("[flags=onlyonce_game]Entity %s - Unhandled keyvalue '%s'.\n", entity.classname.c_str(), entity.values[i]->keyname);
		}

		// Dispatch spawn function
		if(!svs.dllfuncs.pfnDispatchSpawn(pedict))
		{
			CString msg;
			msg << __FUNCTION__ << " - Entity " << entity.classname;
			if(pedict->fields.targetname != NO_STRING_VALUE)
				msg << "(" << SV_GetString(pedict->fields.targetname) << ")";
			msg << " returned error on spawn.\n";
			Con_DPrintf(msg.c_str());

			// Release the edict
			FreeEdict(pedict, EDICT_REMOVED_AT_SPAWN);
			continue;
		}
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
edict_t* CEdictManager::CreateEntity( const Char* pstrClassname )
{
	// Initialize the entity
	edict_t* pedict = AllocEdict();
	pedict->free = false;
		
	// Initialize private data
	if(!SV_InitPrivateData(pedict, pstrClassname))
	{
		Con_Printf("Failed to initialize entity '%s'.\n", pstrClassname);
		FreeEdict(pedict, EDICT_REMOVED_AT_INIT);
		return nullptr;
	}

	return pedict;
}

//=============================================
// @brief
//
//=============================================
edict_t* CEdictManager::CreatePlayerEntity( Uint32 player_index )
{
	if(player_index >= svs.maxclients)
	{
		Con_Printf("Bogus player index %d specified for CreatePlayerEntity.\n", player_index);
		return nullptr;
	}

	// Initialize the entity
	edict_t* pedict = GetEdict(player_index+1);
	pedict->free = false;
	
	// Set classname for player
	pedict->fields.classname = SV_AllocString("player");

	// Initialize private data
	if(!SV_InitPrivateData(pedict, "player"))
	{
		Con_Printf("Failed to initialize player entity.\n");
		FreeEdict(pedict, EDICT_REMOVED_AT_INIT);
		return nullptr;
	}

	return pedict;
}

//=============================================
// @brief
//
//=============================================
Uint32 CEdictManager::AllocIdentifier( void )
{
	// Jump over the reserved space
	if(m_lastIdentifier == (ENTITY_IDENTIFIER_RESERVED_MIN - 1))
		m_lastIdentifier = ENTITY_IDENTIFIER_RESERVED_MAX + 1;

	// Set new identifier
	Uint32 identifier = m_lastIdentifier;
	m_lastIdentifier++;

	return identifier;
}

//=============================================
// @brief
//
//=============================================
edict_t* CEdictManager::AllocEdict( void )
{
	// skip clients and world
	Uint32 i = svs.maxclients + 1; 
	for(; i < m_numEdicts; i++)
	{
		edict_t* pedict = GetEdict(i);

		// Avoid taking edicts during level start or too early
		if(pedict->free && (pedict->freetime <= 2 
			|| svs.time - pedict->freetime >= 0.5))
		{
			// Clear this edict
			ClearEdict(pedict);

			// Set new identifier
			pedict->identifier = AllocIdentifier();
			return pedict;
		}
	}

	if(i == m_edictsArray.size())
	{
		Con_EPrintf("[flags=onlyonce_game]%s - No free edicts left.\n", __FUNCTION__);
		return nullptr;
	}

	m_numEdicts++;
	edict_t* pedict = GetEdict(i);

	// Clear this edict
	ClearEdict(pedict);

	// Set edict identifier
	pedict->identifier = AllocIdentifier();

	return pedict;
}

//=============================================
// @brief
//
//=============================================
void CEdictManager::FreeEdict( edict_t* pedict, edict_removed_t freeMode )
{
	for(Uint32 i = 0; i < m_numEdicts; i++)
	{
		edict_t* pother = GetEdict(i);
		if(pother->pprivatedata && !pother->free 
			&& pother->state.aiment == pedict->entindex)
		{
			svs.dllfuncs.prnOnAimentFreed(pother);
			pother->state.aiment = NO_ENTITY_INDEX;
		}
	}

	// clear private data with game dll
	if(pedict->pprivatedata)
	{
		svs.dllfuncs.pfnFreeEntity(pedict, freeMode);
		pedict->pprivatedata = nullptr;
	}

	pedict->free = true;
	pedict->freetime = svs.time;

	// Clear entity state struct
	pedict->state = entity_state_t();
	pedict->fields = edict_fields_t();

	// Make sure this is ALWAYS set
	pedict->state.entindex = pedict->entindex;

	SV_UnlinkEdict(pedict);

	// Clear vbm hull data if any
	if(pedict->pvbmhulldata)
	{
		delete pedict->pvbmhulldata;
		pedict->pvbmhulldata = nullptr;
	}
}

//=============================================
// @brief
//
//=============================================
void CEdictManager::ClearEdict( edict_t* pedict )
{
	assert(pedict->free);
	assert(!pedict->pprivatedata);
	assert(!pedict->pvbmhulldata);

	// Clear entvars
	pedict->state = entity_state_t();
	pedict->fields = edict_fields_t();
	pedict->free = false;

	// Make sure this is ALWAYS set
	pedict->state.entindex = pedict->entindex;
}

//=============================================
// @brief
//
//=============================================
edict_t* CEdictManager::GetEdict( Int32 index )
{
	if(index == NO_ENTITY_INDEX)
		return nullptr;

	if(index >= 0 && index >= static_cast<Int32>(m_edictsArray.size()))
	{
		Con_EPrintf("Invalid edict index '%d'.\n", index);
		return nullptr;
	}

	return &m_edictsArray[index];
}
