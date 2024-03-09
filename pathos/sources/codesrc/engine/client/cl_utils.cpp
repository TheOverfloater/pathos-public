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
#include "textschemas.h"

//=============================================
//
//=============================================
cl_entity_t* CL_GetEntityByIndex( Int32 index )
{
	if(index == VIEWMODEL_ENTITY_INDEX)
		return cls.dllfuncs.pfnGetViewModel();
	
	if(index < 0 || index >= cls.numentities)
		return nullptr;

	if(index > (Int32)cls.entities.size())
		return nullptr;

	return &cls.entities[index];
}

//=============================================
//
//=============================================
cl_entity_t* CL_GetLocalPlayer( void )
{
	if(cls.cl_state != CLIENT_ACTIVE)
		return nullptr;

	if(!cls.clinfo.entindex)
		return nullptr;

	return CL_GetEntityByIndex(cls.clinfo.entindex);
}

//=============================================
//
//=============================================
Double CL_GetClientTime( void )
{
	return cls.cl_time;
}

//=============================================
//
//=============================================
const Vector& CL_GetViewAngles( void )
{
	return cls.clinfo.viewangles;
}

//=============================================
//
//=============================================
void CL_SetViewAngles( const Vector& angles )
{
	cls.clinfo.viewangles = angles;
}

//=============================================
//
//=============================================
void CL_GetMouseDelta( Int32& deltaX, Int32& deltaY )
{
	gInput.GetMouseDelta(deltaX, deltaY);
}

//=============================================
//
//=============================================
const movevars_t* CL_GetMoveVars( void )
{
	return &cls.pminfo.movevars;
}

//=============================================
//
//=============================================
Int32 CL_GetNumEntities( void )
{
	return (Int32)cls.numentities;
}

//=============================================
//
//=============================================
void CL_PlayEntitySound( entindex_t entindex, Int32 channel, const CString& sample, Float volume, Float attenuation, Int32 pitch, Int32 flags, Float timeoffset )
{
	cl_entity_t* pentity = CL_GetEntityByIndex(entindex);
	if(!pentity || !pentity->pmodel)
		return;

	gSoundEngine.PlaySound(sample.c_str(), nullptr, flags, channel, volume, pitch, attenuation, pentity, entindex);
}

//=============================================
//
//=============================================
void CL_PlayMusic( const CString& sample, Int32 channel, Float timeOffset, Float fadeInTime, Int32 flags )
{
	gSoundEngine.PlayOgg(sample.c_str(), channel, timeOffset, flags, fadeInTime);
}

//=============================================
//
//=============================================
void CL_StopMusic( Int32 channel )
{
	gSoundEngine.PlayOgg(nullptr, channel, 0, OGG_FL_STOP, 0);
}

//=============================================
//
//=============================================
void CL_PlayAmbientSound( entindex_t entindex, const Vector& vecOrigin, Int32 channel, const CString& sample, Float volume, Float attenuation, Int32 pitch, Int32 flags, Float timeoffset )
{
	gSoundEngine.PlaySound(sample.c_str(), &vecOrigin, flags, SND_CHAN_AUTO, volume, pitch, attenuation);
}

//=============================================
//
//=============================================
void CL_ApplySoundEffect( entindex_t entindex, const Char *sample,  Int32 channel, snd_effects_t effect, Float duration, Float targetvalue )
{
	gSoundEngine.ApplySoundEffect(entindex, sample, channel, effect, duration, targetvalue);
}

//=============================================
//
//=============================================
const Char*	CL_GetSoundFileForServerIndex( Int32 serverindex )
{
	return gSoundEngine.GetSoundFileForServerIndex(serverindex);
}

//=============================================
//
//=============================================
void CL_PrecacheDecal( const Char* pstrDecalName )
{
	for(Uint32 i = 0; i < cls.netinfo.decalcache.size(); i++)
	{
		const decalcache_t& cache = cls.netinfo.decalcache[i];
		if(!qstrcmp(cache.name, pstrDecalName) && cache.type == DECAL_CACHE_SINGLE)
			return;
	}

	decalcache_t newcache;
	newcache.name = pstrDecalName;
	newcache.type = DECAL_CACHE_SINGLE;

	cls.netinfo.decalcache.push_back(newcache);
}

//=============================================
//
//=============================================
void CL_PrecacheDecalGroup( const Char* pstrDecalName )
{
	for(Uint32 i = 0; i < cls.netinfo.decalcache.size(); i++)
	{
		const decalcache_t& cache = cls.netinfo.decalcache[i];
		if(!qstrcmp(cache.name, pstrDecalName) && cache.type == DECAL_CACHE_GROUP)
			return;
	}

	decalcache_t newcache;
	newcache.name = pstrDecalName;
	newcache.type = DECAL_CACHE_GROUP;

	cls.netinfo.decalcache.push_back(newcache);
}

//=============================================
//
//=============================================
Int32 CL_PrecacheSound( const Char* pstrSample, rs_level_t level )
{
	CSoundEngine::snd_cache_t* pcache = gSoundEngine.PrecacheSound(pstrSample, NO_SERVER_PRECACHE, level, false);
	if(!pcache)
	{
		Con_Printf("%s - Could not precache '%s'.\n", __FUNCTION__, pstrSample);
		return NO_PRECACHE;
	}

	return pcache->index;
}

//=============================================
//
//=============================================
Vector CL_GetAttachment( entindex_t entindex, Uint32 attachment )
{
	cl_entity_t* pentity = CL_GetEntityByIndex(entindex);
	if(!pentity || !pentity->pmodel)
		return ZERO_VECTOR;

	if(attachment >= MAX_ATTACHMENTS)
	{
		Con_Printf("%s - Bogus attachment index %d.\n", __FUNCTION__, attachment);
		return ZERO_VECTOR;
	}

	gVBMRenderer.UpdateAttachments(pentity);
	return pentity->getAttachment(attachment);
}

//=============================================
//
//=============================================
Vector CL_GetBonePosition( entindex_t entindex, const Char* pstrbonename )
{
	cl_entity_t* pentity = CL_GetEntityByIndex(entindex);
	if(!pentity || !pentity->pmodel)
		return ZERO_VECTOR;

	Vector boneposition;
	if(!gVBMRenderer.GetBonePosition(pentity, pstrbonename, boneposition))
	{
		Con_Printf("%s - Failed to get bone '%s' for entity %d.\n", __FUNCTION__, pstrbonename, (Int32)entindex);
		return ZERO_VECTOR;
	}

	return boneposition;
}

//=============================================
//
//=============================================
ui_schemeinfo_t* CL_UILoadSchemaFile( const Char* pstrFilename )
{
	return gUIManager.LoadSchemaFile(pstrFilename);
}

//=============================================
//
//=============================================
const en_material_t* CL_GetMapTextureMaterial( const Char* pstrtexturename )
{
	for(Uint32 i = 0; i < cls.mapmaterialfiles.size(); i++)
	{
		if(!qstrcmp(cls.mapmaterialfiles[i].maptexturename, pstrtexturename))
		{
			return CTextureManager::GetInstance()->FindMaterialScript(cls.mapmaterialfiles[i].materialfilepath.c_str(), RS_GAME_LEVEL);
		}
	}

	return nullptr;
}

//=============================================
//
//=============================================
Double CL_GetFrameTime( void )
{
	return cls.frametime;
}

//=============================================
//
//=============================================
void CL_GetMousePosition( Int32& x, Int32& y )
{
	gInput.GetMousePosition(x, y);
}

//=============================================
//
//=============================================
void CL_ShowMouse( void )
{
	gInput.ShowMouse();
}

//=============================================
//
//=============================================
void CL_HideMouse( void )
{
	gInput.HideMouse();
}

//=============================================
//
//=============================================
void CL_UpdateMousePositions( void )
{
	gInput.UpdateMousePositions();
}

//=============================================
//
//=============================================
void CL_ResetMouse( void )
{
	gInput.ResetMouse();
}

//=============================================
//
//=============================================
void CL_SetShouldHideMouse( bool shouldhide )
{
	gMenu.SetShouldHideMouse(shouldhide);
}

//=============================================
//
//=============================================
const byte* CL_LeafPVS( const mleaf_t& leaf )
{
	return Mod_LeafPVS(rns.psecondaryvisbuffer, ens.visbuffersize, leaf, *ens.pworld);
}

//=============================================
//
//=============================================
const font_set_t* CL_GetSchemaFontSet( const Char* schemaFileName )
{
	return gTextSchemas.GetSchemaFontSet(schemaFileName);
}

//=============================================
//
//=============================================
const font_set_t* CL_GetResolutionSchemaFontSet( const Char* schemaFileName, Uint32 resolution )
{
	return gTextSchemas.GetResolutionSchemaFontSet(schemaFileName, resolution);
}
