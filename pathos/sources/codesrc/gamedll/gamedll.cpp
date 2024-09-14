/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifdef USE_VLD
#include <vld.h>
#endif

#include "includes.h"

#include "vector.h"
#include "edict.h"
#include "gamedll.h"
#include "gdll_interface.h"
#include "entities.h"
#include "playermove.h"
#include "gamevars.h"
#include "player.h"
#include "constants.h"
#include "file_interface.h"
#include "msgreader.h"
#include "usermsgs.h"
#include "envfog.h"
#include "globalstate.h"
#include "envdlight.h"
#include "util.h"
#include "game.h"
#include "funcmonitor.h"
#include "cache_model.h"
#include "com_math.h"
#include "save_shared.h"
#include "envpossky.h"
#include "playerloadsaved.h"
#include "skilldata.h"
#include "ai_sounds.h"
#include "ai_nodegraph.h"
#include "funcportalsurface.h"
#include "triggercameramodel.h"
#include "skytexturesets.h"

// Declaration of gamedll enginefuncs struct
gdll_engfuncs_t gd_engfuncs;
// Declaration of traceline interface
trace_interface_t gd_tracefuncs;
// File functions
file_interface_t gd_filefuncs;

// Declaration of gamevars pointer
gamevars_t* g_pGameVars = nullptr;
// Class object declaration
CPlayerMovement gMovement;

// VIS buffer for server
byte* g_pVISBuffer = nullptr;
// VIS buffer size
Uint32 g_visBufferSize = 0;

// Node graph generation time
static Double g_nodeGraphGenerationTime = 0;

//
// Game DLL functions
//
static gdll_funcs_t GAMEDLL_FUNCTIONS =
{
	GameDLLInit,				//pfnGameDLLInit
	GameDLLShutdown,			//pfnGameDLLShutdown
	GameInit,					//pfnGameInit
	GameShutdown,				//pfnGameShutdown
	ServerFrame,				//pfnServerFrame
	DispatchSpawn,				//pfnDispatchSpawn
	DispatchRestore,			//pfnDispatchRestore
	DispatchThink,				//pfnDispatchThink
	DispatchTouch,				//pfnDispatchTouch
	DispatchBlocked,			//pfnDispatchBlocked
	DispatchCrossedWater,		//pfnDispatchCrossedWater
	DispatchDeclareSaveFields,	//pfnDispatchDeclareSaveFields
	DispatchReleaseSaveFields,	//pfnDispatchReleaseSaveFields
	ShouldCollide,				//pfnShouldCollide
	SetAbsBox,					//pfnSetAbsBox
	RunEntityPhysics,			//pfnRunEntityPhysics
	PrecacheResources,			//pfnPrecacheResources
	PostSpawnGame,				//pfnPostSpawnGame
	KeyValue,					//pfnKeyValue
	FreeEntity,					//pfnFreeEntity
	OnAimentFreed,				//pfnOnAimentFreed
	GetHullSizes,				//pfnGetHullSizes
	SetupVisiblity,				//pfnSetupVisibility
	AddPacketEntity,			//pfnAddPacketEntity
	CmdStart,					//pfnCmdStart
	CmdEnd,						//pfnCmdEnd
	RunPlayerMovement,			//pfnRunPlayerMovement
	Util::FindEntityByString,	//pfnFindEntityByString
	ClientCommand,				//pfnClientCommand
	ClientPreThink,				//pfnClientPreThink
	ClientPostThink,			//pfnClientPostThink
	ClientConnect,				//pfnClientConnect
	ClientDisconnected,			//pfnClientDisconnected
	InitializeClientData,		//pfnInitializeClientData
	SaveEntityStateData,		//pfnSaveEntityStateData
	SaveEntityFieldsData,		//pfnSaveEntityFieldsData
	SaveEntityClassData,		//pfnSaveEntityClassData
	ShouldTransitionEntity,		//pfnShouldTransitionEntity
	IsGlobalTransitioningEntity,//pfnIsGlobalTransitioningEntity
	ShouldSaveEntity,			//pfnShouldSaveEntity
	GetSaveGameTitle,			//pfnGetSaveGameTitle
	BeginLoadSave,				//pfnBeginLoadSave
	ReadEntityStateData,		//pfnReadEntityStateData
	ReadEntityFieldData,		//pfnReadEntityFieldData
	PrepareEntityClassData,		//pfnPrepareEntityClassData
	ReadEntityClassData,		//pfnReadEntityClassData
	FindGlobalEntity,			//pfnFindGlobalEntity
	AdjustEntityPositions,		//pfnAdjustEntityPositions
	InconsistentFile,			//pfnInconsistentFile
	GetNbGlobalStates,			//pfnGetNbGlobalStates
	SaveGlobalStates,			//pfnReagGlobalStateData
	ReadGlobalStateData,		//pfnReadGlobalStateData
	AreCheatsEnabled,			//pfnAreCheatsEnabled
	GetTransitioningEntities,	//pfnGetTransitionList
	AdjustLandmarkPVSData,		//pfnAdjustLandmarkPVSData
	CanSaveGame,				//pfnCanSaveGame
	CanLoadGame,				//pfnCanLoadGame
	ShowSaveGameMessage,		//pfnShowSaveGameMessage
	ShowAutoSaveGameMessage,	//pfnShowAutoSaveGameMessage
	ShowSaveGameBlockedMessage,	//pfnShowSaveGameBlockedMessage
	RestoreDecal,				//pfnRestoreDecal
	FormatKeyValue,				//pfnFormatKeyValue
};

//=============================================
// @brief
//
//=============================================
bool GameDLLInit( void )
{
#ifdef _DEBUG
	gd_engfuncs.pfnCon_Printf("Game DLL loaded - [color r255]DEBUG[/color] build.\n");
#else
	gd_engfuncs.pfnCon_Printf("Game DLL loaded.\n");
#endif
	gd_engfuncs.pfnCon_Printf("Game DLL build date: %s.\n", __DATE__);

	// Register user messages
	InitClientUserMessages();

	// Init game objects
	if(!InitGameObjects())
		return false;

	// Set skill cvars
	if(!gSkillData.InitGame())
		return false;

	// Allocate the VIS buffer
	if(!g_pVISBuffer)
	{
		g_visBufferSize = gd_engfuncs.pfnGetVISBufferSize();
		g_pVISBuffer = new byte[g_visBufferSize];
		memset(g_pVISBuffer, 0, sizeof(byte)*g_visBufferSize);
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void GameDLLShutdown( void )
{
	ClearGameObjects();

	if(g_pVISBuffer)
	{
		delete[] g_pVISBuffer;
		g_pVISBuffer = nullptr;
	}
}

//=============================================
// @brief
//
//=============================================
void PrecacheResources( void )
{
	// Register and precache player items
	PrecachePlayerItems();
	// Precache generic game resources
	PrecacheGenericResources();
}

//=============================================
// @brief
//
//=============================================
bool GameInit( void )
{
	// Call to initialize entities
	InitializeEntities();

	// Reset this
	CTriggerCameraModel::SetBlockingSaving(false);

	// Init game objects
	if(!InitGame())
		return false;

	if(!gNodeGraph.LoadNodeGraph() && gNodeGraph.GetNumTempNodes() > 0)
	{
		g_nodeGraphGenerationTime = g_pGameVars->time + NODE_GRAPH_GENERATION_DELAY;
		Util::PrintScreenText(-1, -1, NODE_GRAPH_GENERATION_DELAY + 1, "No valid node graph, building new one.");
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void GameShutdown( void )
{
	// Note: Entities are cleared at this point,
	// so don't try to access any entities

	// Clear fog states
	CEnvFog::ClearFogGlobals();
	// Clear lightstyles
	CEnvDLight::ResetLightStyles();
	// Clear monitors
	CFuncMonitor::ClearMonitorList();
	// Clear portals
	CFuncPortalSurface::ClearPortalSurfaceList();
	// Clear blocking state
	CPlayerLoadSaved::ClearSaveGameBlock();
	// Clear skybox entity
	CEnvPosSky::ClearSkyboxEntity();

	// Clear sky texture sets
	gSkyTextureSets.Reset();

	// Clear game objects
	ClearGame();

	// Reset pointers
	gMovement.Reset();
}

//=============================================
// @brief
//
//=============================================
void PostSpawnGame( void )
{
}

//=============================================
// @brief
//
//=============================================
void InitializeClientData( edict_t* pclient )
{
	// !!! - Needs to be sent before SendEntityInitMessages
	gSkyTextureSets.RegisterSets(pclient);

	// Call entities to send init messages
	SendEntityInitMessages(pclient);

	// Call dynlights to send lightstyles
	CEnvDLight::SendLightStylesToClient(pclient);
}

//=============================================
// @brief
//
//=============================================
void GetHullSizes( Int32 hullIndex, Vector& pmins, Vector& pmaxs )
{
	switch(hullIndex)
	{
	case 0:
		// Point sized hull
		pmins = Vector(0, 0, 0);
		pmaxs = Vector(0, 0, 0);
		break;
	case 1:
		// Normal player
		pmins = VEC_HULL_MIN;
		pmaxs = VEC_HULL_MAX;
		break;
	case 2:
		// Large monster hull - not used
		pmins = Vector(0, 0, 0);
		pmaxs = Vector(0, 0, 0);
		break;
	case 3:
		// Ducked player
		pmins = VEC_DUCK_HULL_MIN;
		pmaxs = VEC_DUCK_HULL_MAX;
		break;
	}
}

//=============================================
// @brief
//
//=============================================
void ServerFrame( void )
{
	// Generate node graph if possible
	if(g_nodeGraphGenerationTime && g_nodeGraphGenerationTime <= g_pGameVars->time)
	{
		gNodeGraph.InitNodeGraph();
		g_nodeGraphGenerationTime = 0;
	}

	// Update fog globals
	CEnvFog::UpdateFogGlobals();

	// Manage NPC sound updates
	gAISounds.Think();

	// Reset these
	g_nbNPCPenetrations = 0;
}

//=============================================
// @brief
//
//=============================================
Double GetTime( void )
{
	return g_pGameVars->time;
}

//=============================================
// @brief
//
//=============================================
void RunPlayerMovement( const usercmd_t& cmd, pm_info_t* pminfo )
{
	bool isMultiplayer = (g_pGameVars->maxclients > 1) ? true : false;

	// Run movement logic for the client
	gMovement.RunMovement(cmd, pminfo, true, isMultiplayer);
}

//=============================================
// @brief
//
//=============================================
Int32 GetNbEntities( void )
{
	return gd_engfuncs.pfnGetNbEdicts();
}

//=============================================
// @brief
//
//=============================================
const entity_state_t* GetEntityState( entindex_t entindex )
{
	edict_t* pedict = gd_engfuncs.pfnGetEdictByIndex(entindex);
	if(!pedict || pedict->free)
		return nullptr;

	return &pedict->state;
}

//=============================================
// @brief
//
//=============================================
void SaveEntityStateData( edict_t* pedict, bool istransitionsave )
{
	SaveEntityState(pedict->state, istransitionsave);
}

//=============================================
// @brief
//
//=============================================
void SaveEntityFieldsData( edict_t* pedict, bool istransitionsave )
{
	SaveEntityFields(pedict->fields, istransitionsave);
}

//=============================================
// @brief
//
//=============================================
void SaveEntityClassData( edict_t* pedict, bool istransitionsave )
{
	CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
	pEntity->SaveEntityClassData(istransitionsave);
}

//=============================================
// @brief
//
//=============================================
bool IsGlobalTransitioningEntity( edict_t* pedict )
{
	// Do not transfer invalid entities
	if(Util::IsNullEntity(pedict))
		return false;

	CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
	if(!pEntity)
		return false;

	if(!pEntity->HasGlobalName())
		return false;

	// List of entites that can potentially transition
	const Int32* pTransitionEntitiesList = nullptr;
	Uint32 numTransitionEntities = 0;

	gd_engfuncs.pfnGetTransitionList(&pTransitionEntitiesList, numTransitionEntities);

	// Make sure it's in the list
	for(Uint32 i = 0; i < numTransitionEntities; i++)
	{
		if(pTransitionEntitiesList[i] == pedict->entindex)
			return true;
	}

	return false;
}

//=============================================
// @brief
//
//=============================================
bool ShouldTransitionEntity( edict_t* pedict )
{
	// Do not transfer invalid entities
	if(Util::IsNullEntity(pedict))
		return false;

	CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
	if(!pEntity)
		return false;

	if(!(pEntity->GetEntityFlags() & FL_ENTITY_TRANSITION) && pEntity->HasGlobalName())
		return false;

	// List of entites that can potentially transition
	const Int32* pTransitionEntitiesList = nullptr;
	Uint32 numTransitionEntities = 0;

	gd_engfuncs.pfnGetTransitionList(&pTransitionEntitiesList, numTransitionEntities);

	// Make sure it's in the list
	for(Uint32 i = 0; i < numTransitionEntities; i++)
	{
		if(pTransitionEntitiesList[i] == pedict->entindex)
			return true;
	}

	return false;
}

//=============================================
// @brief
//
//=============================================
bool ShouldSaveEntity( edict_t* pedict )
{
	// Do not transfer invalid entities
	if(Util::IsNullEntity(pedict))
		return false;

	CBaseEntity* pEntity = CBaseEntity::GetClass(pedict);
	if(!pEntity)
		return false;

	if(pEntity->GetEntityFlags() & FL_ENTITY_DONT_SAVE)
		return false;

	return true;
}

//=============================================
// @brief
//
//=============================================
void GetSaveGameTitle( Char* pstrBuffer, Int32 maxlength )
{
	const Char* pstrchaptertitle = nullptr;

	CBaseEntity* pEntity = Util::GetHostPlayer();
	if(pEntity)
		pstrchaptertitle = pEntity->GetSaveGameTitle();

	if(!pstrchaptertitle || !qstrlen(pstrchaptertitle))
	{
		// Just use map name
		qstrncpy(pstrBuffer, g_pGameVars->levelname.c_str(), maxlength);
	}
	else
	{
#ifdef _DEBUG
		CString savetitle;
		savetitle << pstrchaptertitle << "(" << g_pGameVars->levelname.c_str() << ")";
		qstrncpy(pstrBuffer, savetitle.c_str(), maxlength);
#else
		qstrncpy(pstrBuffer, pstrchaptertitle, qstrlen(pstrchaptertitle));
#endif
	}

}

//=============================================
// @brief
//
//=============================================
bool InconsistentFile( const Char* pstrFilename )
{
	return true;
}

//=============================================
// @brief
//
//=============================================
bool AreCheatsEnabled( void )
{
	return (g_pCvarCheats->GetValue() > 0) ? true : false;
}

//=============================================
// @brief
//
//=============================================
bool CanSaveGame( savefile_type_t type )
{
	switch(type)
	{
	case SAVE_REGULAR:
	case SAVE_QUICK:
	case SAVE_AUTO:
	{
		if(CPlayerLoadSaved::IsBlockingSaving())
			return false;

		if(CTriggerCameraModel::IsBlockingSaving())
			return false;
	}
	break;
	case SAVE_TRANSITION:
	case SAVE_MAPSAVE:
	default:
		break;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void FormatKeyValue( const Char* pstrKeyValueName, Char* pstrValue, Uint32 maxLength )
{
	// Check for old "monster_" prefixes and change them
	if(!qstrcmp(pstrKeyValueName, "classname"))
	{
		// Support for legacy mod levels
		if(!qstrncmp(pstrValue, "monster_", 8))
		{
			CString tmp(pstrValue);
			tmp.erase(0, 8);
			tmp.insert(0, "npc_");
			qstrcpy(pstrValue, tmp.c_str());
		}
		else if(!qstrcmp(pstrValue, "monstermaker"))
		{
			// Rename "monstermaker" to "npcmaker"
			qstrcpy(pstrValue, "npcmaker");
		}
		else if(!qstrcmp(pstrValue, "func_monsterclip"))
		{
			// Rename "func_monsterclip" to "func_npcclip"
			qstrcpy(pstrValue, "func_npcclip");
		}
	}
}

//=============================================
// @brief
//
//=============================================
bool CanLoadGame( void )
{
	return true;
}

//=============================================
// @brief
//
//=============================================
void ShowSaveGameMessage( void )
{
	Util::ShowMessageAllPlayers("GAMESAVED");
}

//=============================================
// @brief
//
//=============================================
void ShowAutoSaveGameMessage( void )
{
	Util::ShowMessageAllPlayers("AUTOSAVED");
}

//=============================================
// @brief
//
//=============================================
void ShowSaveGameBlockedMessage( void )
{
	Util::ShowMessageAllPlayers("SAVEBLOCKED");
}

//=============================================
// @brief
//
//=============================================
void RestoreDecal( const Vector& origin, const Vector& normal, edict_t* pedict, const Char* pstrDecalTexture, Int32 decalflags )
{
	entindex_t entityindex = pedict ? pedict->entindex : NO_ENTITY_INDEX;

	// Create the decal on the client
	Util::CreateGenericDecal(origin, &normal, pstrDecalTexture, decalflags, entityindex);
}

//=============================================
// @brief
//
//=============================================
void AdjustLandmarkPVSData( edict_t* pLandmarkEdict, byte* pPVS, Uint32 pvsBufferSize )
{
	if (!pLandmarkEdict)
		return;

	if (!pPVS)
		return;

	const Char* pstrLandmarkName = gd_engfuncs.pfnGetString(pLandmarkEdict->fields.targetname);
	if (!pstrLandmarkName || !qstrlen(pstrLandmarkName))
		return;

	const cache_model_t* pworld = gd_engfuncs.pfnGetModel(1);
	if (!pworld)
		return;

	const brushmodel_t* pworldmodel = pworld->getBrushmodel();
	if (!pworldmodel)
		return;

	byte* ppvsdata = nullptr;

	edict_t* pedict = nullptr;
	while (true)
	{
		pedict = Util::FindEntityByTargetName(pedict, pstrLandmarkName);
		if (!pedict)
			break;

		const Char* pstrEntityClassname = gd_engfuncs.pfnGetString(pedict->fields.classname);
		if (!pstrEntityClassname)
			continue;

		if (!qstrcmp(pstrEntityClassname, "info_vismark"))
		{
			const mleaf_t* pleaf = nullptr;
			for (Uint32 i = 0; i < 4; i++)
			{
				Vector testPosition = pedict->state.origin + Vector(0, 0, static_cast<Float>(i));
				pleaf = gd_engfuncs.pfnPointInLeaf(testPosition, (*pworldmodel));
				if (!pleaf || pleaf->contents != CONTENTS_SOLID)
					break;
			}

			// Check if leaf was valid in the end
			if (!pleaf || pleaf->contents == CONTENTS_SOLID)
				continue;

			if (!ppvsdata)
			{
				ppvsdata = new byte[pvsBufferSize];
				memset(ppvsdata, 0, sizeof(byte) * pvsBufferSize);
			}

			// Get PVS for this vismark entity
			gd_engfuncs.pfnLeafPVS(ppvsdata, pvsBufferSize, (*pleaf), (*pworldmodel));

			// Add the contents to the server PVS
			Uint32 bytecount = (pworldmodel->numleafs + 7) >> 3;
			for (Uint32 i = 0; i < bytecount; i++)
				pPVS[i] |= ppvsdata[i];
		}
	}

	if (ppvsdata)
		delete[] ppvsdata;
}

//=============================================
// @brief
//
//=============================================
extern "C" bool DLLEXPORT GameDLL_Init( Uint32 version, gdll_funcs_t& dllFuncs, const gdll_engfuncs_t& engFuncs, const trace_interface_t& traceFuncs, const file_interface_t& fileFuncs, gamevars_t& gamevars )
{
	if(version != GDLL_INTERFACE_VERSION)
		return false;

	// Set our engine funcs
	gd_engfuncs = engFuncs;
	// Set our traceline funcs
	gd_tracefuncs = traceFuncs;
	// Set file funcs
	gd_filefuncs = fileFuncs;
	// Set our gamevars pointer
	g_pGameVars = &gamevars;

	// Set the interface for the engine
	dllFuncs = GAMEDLL_FUNCTIONS;

	// Set playermove here, because we reference engine functions
	pm_interface_t pmInterface = 
	{
		engFuncs.pfnCon_Printf,						//pfnCon_Printf
		engFuncs.pfnCon_DPrintf,					//pfnCon_DPrintf
		engFuncs.pfnCon_VPrintf,					//pfnCon_VPrintf
		engFuncs.pfnCon_EPrintf,					//pfnCon_EPrintf
		GetTime,									//pfnGetTime
		engFuncs.pfnGetModelType,					//pfnGetModelType
		engFuncs.pfnGetModelBounds,					//pfnGetModelBounds
		engFuncs.pfnGetModel,						//pfnGetModel
		GetNbEntities,								//pfnGetNumEntities
		GetEntityState,								//pfnGetEntityState
		engFuncs.pfnGetMapTextureMaterialScript,	//pfnGetMapTextureMaterialScript
		PM_PlaySound,								//pfnPlaySound
		PM_PlayStepSound,							//pfnPlayStepSound
		engFuncs.pfnAddToTouched,					//pfnAddToTouched
	};

	// Initialize playermove
	gMovement.Init(traceFuncs, pmInterface, true);

	return true;
}