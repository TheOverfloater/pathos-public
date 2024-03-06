/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "enginestate.h"

#include "cl_entity.h"
#include "cl_main.h"
#include "cl_msg.h"

#include "sv_main.h"
#include "networking.h"
#include "system.h"
#include "usercmd.h"
#include "console.h"
#include "input.h"
#include "commands.h"
#include "texturemanager.h"
#include "cl_snd.h"
#include "r_main.h"
#include "modelcache.h"
#include "r_sprites.h"
#include "r_decals.h"
#include "flexmanager.h"
#include "r_vbm.h"
#include "uimanager.h"
#include "r_menu.h"

//=============================================
//
//=============================================
const en_material_t* Engine_GetMaterialScript( const Char* pstrTextureName )
{
	return CTextureManager::GetInstance()->FindMaterialScript(pstrTextureName, RS_GAME_LEVEL);
}

//=============================================
//
//=============================================
const mleaf_t* Engine_PointInLeaf( const Vector& position )
{
	return Mod_PointInLeaf(position, *ens.pworld);
}

//=============================================
//
//=============================================
const cache_model_t* Engine_LoadModel( const Char* pstrModelPath )
{
	return gModelCache.LoadModel(pstrModelPath);
}

//=============================================
//
//=============================================
const Char* Engine_GetLevelName( void )
{
	cache_model_t* pworld = gModelCache.GetModelByIndex(WORLD_MODEL_INDEX);
	if(!pworld)
		return nullptr;

	return pworld->name.c_str();
}

//=============================================
//
//=============================================
Double Engine_GetEngineTime( void )
{
	return ens.time;
}

//=============================================
//
//=============================================
CCVar* Engine_CreateCVar( cvar_type_t type, Int32 flags, const Char* pstrName, const Char* pstrValue, const Char* pstrDescription )
{
	return gConsole.CreateCVar(type, flags, pstrName, pstrValue, pstrDescription);
}

//=============================================
//
//=============================================
CCVar* Engine_CreateCVarCallback( cvar_type_t type, Int32 flags, const Char* pstrName, const Char* pstrValue, const Char* pstrDescription, pfnCVarCallback_t pfnCallback )
{
	return gConsole.CreateCVar(type, flags, pstrName, pstrValue, pstrDescription, pfnCallback);
}

//=============================================
//
//=============================================
CCVar* Engine_GetCVarPointer( const Char* pstrName )
{
	return gConsole.GetCVar(pstrName);
}

//=============================================
//
//=============================================
void Engine_SetCVarFloat( const Char* pstrName, Float value )
{
	gConsole.CVarSetFloatValue(pstrName, value);
}

//=============================================
//
//=============================================
void Engine_SetCVarString( const Char* pstrName, const Char* pstrValue )
{
	gConsole.CVarSetStringValue(pstrName, pstrValue);
}

//=============================================
//
//=============================================
void Engine_CreateCommand( const Char* name, cmdfunc_t pfn, const Char* description )
{
	gCommands.CreateCommand(name, pfn, description);
}

//=============================================
//
//=============================================
Uint32 Engine_Cmd_Argc( void )
{
	return gCommands.Cmd_Argc();
}

//=============================================
//
//=============================================
const Char* Engine_Cmd_Argv( Uint32 index )
{
	return gCommands.Cmd_Argv(index);
}

//=============================================
//
//=============================================
Float Engine_GetCvarFloatValue( const Char* pstrCvarName )
{
	CCVar* pCvar = gConsole.GetCVar(pstrCvarName);
	if(!pCvar)
	{
		Con_Printf("%s - No such cvar '%s'.\n", __FUNCTION__, pstrCvarName);
		return 0;
	}

	if(pCvar->GetType() != CVAR_FLOAT)
	{
		Con_Printf("%s - '%s' is not a float type cvar.\n", __FUNCTION__, pstrCvarName);
		return 0;
	}

	return pCvar->GetValue();
}

//=============================================
//
//=============================================
const Char* Engine_GetCvarStringValue( const Char* pstrCvarName )
{
	CCVar* pCvar = gConsole.GetCVar(pstrCvarName);
	if(!pCvar)
	{
		Con_Printf("%s - No such cvar '%s'.\n", __FUNCTION__, pstrCvarName);
		return "";
	}

	if(pCvar->GetType() != CVAR_STRING)
	{
		Con_Printf("%s - '%s' is not a string type cvar.\n", __FUNCTION__, pstrCvarName);
		return "";
	}

	return pCvar->GetStrValue();
}

//=============================================
//
//=============================================
Uint32 Engine_GetVISBufferSize( void )
{
	return ens.visbuffersize;
}