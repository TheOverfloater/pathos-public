/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "cl_main.h"
#include "cl_efx.h"
#include "cl_utils.h"
#include "cl_snd.h"
#include "cl_tempentities.h"

#include "r_main.h"
#include "r_vbo.h"
#include "r_glsl.h"
#include "r_main.h"
#include "r_decals.h"
#include "r_vbm.h"
#include "r_sprites.h"
#include "r_cables.h"
#include "r_dlights.h"
#include "r_particles.h"
#include "r_cubemaps.h"
#include "r_bsp.h"
#include "r_postprocess.h"
#include "r_legacyparticles.h"
#include "r_water.h"
#include "r_beams.h"
#include "r_blackhole.h"
#include "r_lensflare.h"
#include "r_sky.h"
#include "r_lightstyles.h"
#include "r_tracers.h"

#include "ald.h"
#include "aldformat.h"
#include "efxapi.h"
#include "entity_extrainfo.h"
#include "flexmanager.h"
#include "system.h"
#include "cache_model.h"
#include "studio.h"
#include "vbmformat.h"
#include "enginestate.h"
#include "bsp_shared.h"

static cl_efxapi_t EFXAPI_INTERFACE_FUNCS =
{
	CL_AllocStaticSpriteEntity,		//pfnAllocStaticSpriteEntity
	CL_AllocTempSpriteEntity,		//pfnAllocTempSpriteEntity
	CL_CreateCableEntity,			//pfnCreateCableEntity
	CL_SetFogParameters,			//pfnSetFogParameters
	CL_SetSkyboxParameters,			//pfnSetSkyboxParameters
	CL_AllocEntityLight,			//pfnAllocEntityLight
	CL_AllocDynamicPointLight,		//pfnAllocDynamicPointLight
	CL_AllocDynamicSpotLight,		//pfnAllocDynamicSpotLight
	CL_SetDayStage,					//pfnSetDayStage
	CL_SetSpecialFog,				//pfnSetSpecialFog
	CL_FreeEntityData,				//pfnFreeEntityData
	CL_SetLightStyle,				//pfnSetLightStyle
	CL_AddCustomLightStyle,			//pfnAddCustomLightStyle
	CL_GetDecalList,				//pfnGetDecalList
	CL_DecalVBMEntity,				//pfnDecalVBMEntity
	CL_CreateGenericDecal,			//pfnCreateGenericDecal
	CL_PrecacheParticleScript,		//pfnPrecacheParticleScript
	CL_RemoveParticleSystem,		//pfnRemoveParticleSystem
	CL_SpawnParticleSystem,			//pfnSpawnParticleSystem
	CL_KillEntityParticleSystems,	//pfnKillEntityParticleSystems
	CL_SetMotionBlur,				//pfnSetMotionBlur
	CL_SetVignette,				    //pfnSetVignette
	CL_SetFilmGrain,				//pfnSetFilmGrain
	CL_SetBlackAndWhite,			//pfnSetBlackAndWhite
	CL_SetChromatic,				//pfnSetChromatic
	CL_SetScreenOverlay,			//pfnSetScreenOverlay
	CL_ClearScreenOverlay,			//pfnClearScreenOverlay
	CL_SetFade,						//pfnSetFade
	CL_SetGaussianBlur,				//pfnSetGaussianBlur
	CL_BreakModel,					//pfnBreakModel
	CL_Bubbles,						//pfnBubbles
	CL_BubbleTrail,					//pfnBubbleTrail
	CL_FunnelSprite,				//pfnFunnelSprite
	CL_SphereModel,					//pfnSphereModel
	CL_TempModel,					//pfnTempModel
	CL_TempSprite,					//pfnTempSprite
	CL_CreateTracer,				//pfnCreateTracer
	CL_ParticleExplosion1,			//pfnParticleExplosion1
	CL_ParticleExplosion2,			//pfnParticleExplosion2
	CL_BlobExplosion,				//pfnBlobExplosion
	CL_RocketExplosion,				//pfnRocketExplosion
	CL_ParticleEffect,				//pfnParticleEffect
	CL_LavaSplash,					//pfnLavaSplash
	CL_TeleportSplash,				//pfnTeleportSplash
	CL_RocketTrail,					//pfnRocketTrail
	CL_BeamLightning,				//pfnBeamLightning	
	CL_BeamCirclePoints,			//pfnBeamCirclePoints
	CL_BeamEntityPoint,				//pfnBeamEntityPoint
	CL_BeamEntities,				//pfnBeamEntities
	CL_BeamFollow,					//pfnBeamFollow
	CL_BeamPoints,					//pfnBeamPoints
	CL_BeamRing,					//pfnBeamRing
	CL_BeamVaporTrail,				//pfnBeamVaporTrail
	CL_KillEntityBeams,				//pfnKillEntityBeams
	CL_CreateBlackHole,				//pfnCreateBlackHole
	CL_KillBlackHole,				//pfnKillBlackHole
	CL_SetSunFlare,					//pfnSetSunFlare
	CL_AddSkyTextureSet,			//pfnAddSkyTextureSet
	CL_SetSkyTexture,				//pfnSetSkyTexture
};

//====================================
//
//====================================
cl_entity_t* CL_AllocStaticSpriteEntity( void )
{
	return gSpriteRenderer.AllocStaticSprite();
}

//====================================
//
//====================================
cl_entity_t* CL_AllocTempSpriteEntity( Int32 key, Float life )
{
	return gSpriteRenderer.AllocTempSprite(key, life);
}

//====================================
//
//====================================
mlight_t* CL_AllocEntityLight( Int32 key, Float life, Int32 attachment )
{
	Int32 _attachment = attachment;
	if(_attachment < 0 && _attachment != -1 || _attachment >= MAX_ATTACHMENTS)
	{
		Con_Printf("%s - Attachment index is invalid.\n", __FUNCTION__, _attachment);
		_attachment = 0;
	}

	// Try to find one with the same key
	if(key)
	{
		for(Uint32 i = 0; i < MAX_ENTITY_LIGHTS; i++)
		{
			entitylight_t& el = cls.entitylights[i];

			if(el.key == key)
			{
				// Clear
				el = entitylight_t();

				// Set basic values
				el.die = cls.cl_time + life;
				el.key = key;
				el.attachment = _attachment;
				el.mlight.entindex = key;

				return &el.mlight;
			}
		}
	}

	// Try to find a dead one
	for(Uint32 i = 0; i < MAX_ENTITY_LIGHTS; i++)
	{
		entitylight_t& el = cls.entitylights[i];

		if(el.die < cls.cl_time)
		{
			// Clear
			el = entitylight_t();

			// Set values
			el.die = cls.cl_time + life;
			el.key = key;
			el.attachment = _attachment;
			el.mlight.entindex = key;

			return &el.mlight;
		}
	}

	// Just return the one at index 0
	entitylight_t& el = cls.entitylights[0];

	if(el.die)
		el = entitylight_t();

	el.die = cls.cl_time + life;
	el.key = key;
	el.attachment = _attachment;
	el.mlight.entindex = key;

	return &el.mlight;
}

//====================================
//
//====================================
void CL_CreateCableEntity( const Vector& start, const Vector& end, Uint32 depth, Uint32 width, Uint32 numsegments )
{
	gCableRenderer.AddCable(start, end, depth, width, numsegments);
}

//====================================
//
//====================================
void CL_SetFogParameters( entindex_t entindex, const Vector& color, Float start, Float end, bool affectsky, Float blendtime )
{
	bool active = (end < 1 && start < 1) ? false : true;
	if(!active && rns.fog.settings.active && entindex != rns.fog.settings.entindex && entindex != NO_ENTITY_INDEX)
		return;

	if(rns.fog.settings.active && active && blendtime && rns.fog.settings.blend > 0)
	{
		//Perform blend
		memcpy(&rns.fog.blend1, &rns.fog.settings, sizeof(fog_settings_t));
		rns.fog.blend2.active = active;
		rns.fog.blend2.affectsky = affectsky;
		rns.fog.blend2.start = start;
		rns.fog.blend2.end = end;
		rns.fog.blend2.color = color;
		rns.fog.blend2.blend = blendtime;
		rns.fog.blend2.entindex = entindex;
		rns.fog.blendtime = cls.cl_time+blendtime;
	}
	else
	{
		//Just set.
		rns.fog.settings.active = active;
		rns.fog.settings.affectsky = affectsky;
		rns.fog.settings.start = start;
		rns.fog.settings.end = end;
		rns.fog.settings.color = color;
		rns.fog.settings.blend = blendtime;
		rns.fog.blendtime = 0;
	}

	if(!rns.fog.settings.active)
	{
		// So skybox works after disabling fog
		rns.fog.settings.affectsky = false;
	}

	// Always set
	rns.fog.settings.entindex = entindex;
}

//====================================
//
//====================================
void CL_SetSkyboxParameters( const Vector& worldorigin, Float skysize, Float fogend, Float fogstart, const Vector& fogcolor, Int32 skytexturesetindex, bool affectskybox, bool isactive )
{
	rns.sky.world_origin = worldorigin;
	rns.sky.skysize = skysize;
	rns.sky.fog.end = fogend;
	rns.sky.fog.start = fogstart;
	rns.sky.fog.color = fogcolor;
	rns.sky.fog.affectsky = affectskybox;
	rns.sky.skybox = isactive;

	// Set this
	gSkyRenderer.SetSkyBoxSkySet(skytexturesetindex);

	rns.sky.fog.active = (rns.sky.fog.end < 1 && rns.sky.fog.start < 1) ? false : true;
}

//====================================
//
//====================================
cl_dlight_t* CL_AllocDynamicPointLight( Int32 key, Int32 subkey, bool isstatic, bool noshadow, cl_entity_t *pentity )
{
	return gDynamicLights.AllocDynamicPointLight(key, subkey, isstatic, noshadow, pentity);
}

//====================================
//
//====================================
cl_dlight_t*CL_AllocDynamicSpotLight( Int32 key, Int32 subkey, bool isstatic, bool noshadow, cl_entity_t *pentity )
{
	return gDynamicLights.AllocDynamicSpotlight(key, subkey, isstatic, noshadow, pentity);
}

//====================================
//
//====================================
void CL_SetDayStage( daystage_t daystage )
{
	// Set value
	daystage_t prevdaystage = rns.daystage;
	rns.daystage = daystage;

	if (prevdaystage == rns.daystage)
		return;

	// Load in the ALD file
	if(!rns.isgameready)
		return;

	// Get worldmodel ptr
	brushmodel_t* pworldmodel = ens.pworld;
	// Data pointers for our load result
	byte* pdatapointers[NB_SURF_LIGHTMAP_LAYERS] = {nullptr};

	// If restoring to DAYSTAGE_NORMAL from any other stage, then we
	// need to load the restore file 
	daystage_t loadstage;
	if(daystage == DAYSTAGE_NORMAL && prevdaystage != DAYSTAGE_NORMAL)
		loadstage = DAYSTAGE_NORMAL_RESTORE;
	else
		loadstage = rns.daystage;

	if (!ALD_Load(loadstage, pdatapointers))
	{
		// Mark as having relevant data
		rns.hasdaystagedata = false;
		return;
	}

	// Set the new pointer
	for(Uint32 i = 0; i < NB_SURF_LIGHTMAP_LAYERS; i++)
	{
		// All data was successfully set, so release original data
		if (pworldmodel->plightdata[i])
			delete[] pworldmodel->plightdata[i];

		pworldmodel->plightdata[i] = reinterpret_cast<color24_t*>(pdatapointers[i]);
	}

	// Set new sampling data
	BSP_SetSamplingLightData(*ens.pworld);

	// Reset lighting on entities
	CL_ResetLighting();

	// Tell the renderer to reload certain things
	gBSPRenderer.InitLightmaps();

	// Load day stage cubemaps
	gCubemaps.InitGame();

	// Load day stage water scripts
	BSP_ReserveWaterLighting();

	gWaterShader.LoadScripts();
	gWaterShader.ReloadLightmapData();

	// Release the lightmap data
	BSP_ReleaseLightmapData(*ens.pworld);

	// Mark as having relevant data
	rns.hasdaystagedata = true;
}

//====================================
//
//====================================
void CL_SetSpecialFog( bool enablespecialfog )
{
	// Set the value
	rns.fog.specialfog = enablespecialfog;

	// Recompile VBO if required
	if(CL_IsGameActive() && rns.fog.prevspecialfog != rns.fog.specialfog)
	{
		gBSPRenderer.InitVBO();
		rns.fog.prevspecialfog = rns.fog.specialfog;
	}
}

//====================================
//
//====================================
void CL_FreeEntityData( entindex_t entindex, Int32 flags )
{
	if(!(flags & FREE_MSG_FL_KEEP_SOUNDS))
	{
		// Release any sounds tied to this entity
		gSoundEngine.FreeEntity(entindex);
	}

	// Release any beams
	gBeamRenderer.KillEntityBeams(entindex);

	if(!(flags & FREE_MSG_FL_KEEP_DLIGHTS))
	{
		// Release any dynlights
		gDynamicLights.ReleaseEntityDynamicLights(entindex);
	}

	if(!(flags & FREE_MSG_FL_KEEP_PARTICLES))
	{
		// Release any tied particles
		gParticleEngine.KillEntityParticleSystems(entindex);
	}

	// Release any dynlight data
	cl_entity_t* pentity = CL_GetEntityByIndex(entindex);
	if(!pentity)
		return;

	// This must be reset to avoid first-frame angle bug
 	pentity->curstate.msg_num = pentity->prevstate.msg_num = 0;
	pentity->curstate.msg_time = pentity->prevstate.msg_time = 0;

	// Release any decals
	gVBMRenderer.FreeEntityData(pentity);
}

//====================================
//
//====================================
void CL_AddCustomLightStyle( Int32 style, const Char* pstrpattern, bool interpolate, Float framerate )
{
	gLightStyles.AddCustomLightStyle(style, framerate, interpolate, pstrpattern);
}

//====================================
//
//====================================
void CL_SetLightStyle( Int32 style, const Char* pstrpattern, bool interpolate, Float framerate )
{
	gLightStyles.SetLightStyle(style, framerate, interpolate, pstrpattern);
}

//====================================
//
//====================================
CDecalList& CL_GetDecalList( void )
{
	return gDecals.GetDecalList();
}

//====================================
//
//====================================
void CL_DecalVBMEntity( const Vector& origin, const Vector& normal, decalgroupentry_t* pentry, cl_entity_t* pentity, Int32 flags )
{
	gVBMRenderer.CreateDecal(origin, normal, pentry, pentity, flags);
}

//====================================
//
//====================================
void CL_CreateGenericDecal( const Char *pstrname, const Vector& origin, const Vector& normal, Int32 flags, entindex_t entindex, Float life, Float fadetime, Float growthtime )
{
	gDecals.AddCached(nullptr, pstrname, origin, normal, flags, entindex, life, fadetime, growthtime);
}

//====================================
//
//====================================
void CL_PrecacheParticleScript( part_script_type_t type, const Char* pstrFilepath )
{
	gParticleEngine.PrecacheScript(type, pstrFilepath, false);
}

//====================================
//
//====================================
void CL_RemoveParticleSystem( Uint32 id, entindex_t entindex, bool keepcached )
{
	gParticleEngine.CacheRemoveSystem(id, entindex, keepcached);
}

//====================================
//
//====================================
void CL_KillEntityParticleSystems( Int32 entindex )
{
	gParticleEngine.KillEntityParticleSystems(entindex);
}

//====================================
//
//====================================
void CL_SpawnParticleSystem( const Vector& origin, const Vector& direction, part_script_type_t scripttype, const Char* pstrFilepath, Int32 id, entindex_t entindex, Int32 attachment, Int32 boneindex, Int32 attachflags )
{
	gParticleEngine.CacheCreateSystem(origin, direction, scripttype, pstrFilepath, id, entindex, attachment, boneindex, attachflags);
}

//====================================
//
//====================================
void CL_SetMotionBlur( bool active, Float blurfade, bool override )
{
	gPostProcess.SetMotionBlur(active, blurfade, override);
}

//====================================
//
//====================================
void CL_SetVignette( bool active, Float strength, Float radius )
{
	gPostProcess.SetVignette(active, strength, radius);
}

//====================================
// 
//====================================
void CL_SetFilmGrain( bool active, Float strength )
{
	gPostProcess.SetFilmGrain(active, strength);
}

//====================================
//
//====================================
void CL_SetBlackAndWhite( bool active, Float strength )
{
	gPostProcess.SetBlackAndWhite(active, strength);
}

//====================================
//
//====================================
void CL_SetChromatic( bool active, Float strength )
{
	gPostProcess.SetChromatic(active, strength);
}

//====================================
//
//====================================
void CL_SetScreenOverlay( Int32 layerindex, const Char* pstrtexturename, overlay_rendermode_t rendermode, const Vector& rendercolor, Float renderamt, overlay_effect_t effect, Float effectspeed, Float effectminalpha, Float fadetime )
{
	gPostProcess.SetOverlay(layerindex, pstrtexturename, rendermode, rendercolor, renderamt, effect, effectspeed, effectminalpha, fadetime);
}

//====================================
//
//====================================
void CL_ClearScreenOverlay( Int32 layerindex, Float fadetime )
{
	gPostProcess.ClearOverlay(layerindex, fadetime);
}

//====================================
//
//====================================
void CL_SetFade( Uint32 layerindex, Float duration, Float holdtime, Int32 flags, const color24_t& color, byte alpha, Float timeoffset )
{
	gPostProcess.SetFade(layerindex, duration, holdtime, flags, color, alpha, timeoffset);
}

//====================================
//
//====================================
void CL_SetGaussianBlur( bool active, Float alpha )
{
	gPostProcess.SetGaussianBlur(active, alpha);
}

//====================================
//
//====================================
void CL_BreakModel( const Vector& origin, const Vector& size, const Vector& velocity, Uint32 random, Float life, Uint32 num, Uint32 modelindex, Int32 sound, Float bouyancy, Float waterfriction, Int32 flags )
{
	gTempEntities.CreateBreakModel(origin, size, velocity, random, life, num, modelindex, sound, bouyancy, waterfriction, flags);
}

//====================================
//
//====================================
void CL_Bubbles( const Vector& mins, const Vector& maxs, Float height, Uint32 modelindex, Uint32 num, Float speed )
{
	gTempEntities.CreateBubbles(mins, maxs, height, modelindex, num, speed);
}

//====================================
//
//====================================
void CL_BubbleTrail( const Vector& start, const Vector& end, Float height, Uint32 modelindex, Uint32 num, Float speed )
{
	gTempEntities.CreateBubbleTrail(start, end, height, modelindex, num, speed);
}

//====================================
//
//====================================
void CL_FunnelSprite( const Vector& origin, const Vector& color, Float alpha, Uint32 modelindex, bool reverse )
{
	gTempEntities.CreateFunnel(origin, color, alpha, modelindex, reverse);
}

//====================================
//
//====================================
void CL_SphereModel( const Vector& origin, Float speed, Float life, Uint32 num, Uint32 modelindex, Float bouyancy, Float waterfriction, Int32 sound )
{
	gTempEntities.CreateSphereModel(origin, speed, life, num, modelindex, bouyancy, waterfriction, sound);
}

//====================================
//
//====================================
tempentity_t* CL_TempModel( const Vector& origin, const Vector& velocity, const Vector& angles, Float life, Uint32 modelindex, Int32 sound, Float bouyancy, Float waterfriction, Int32 flags )
{
	return gTempEntities.CreateTempModel(origin, velocity, angles, life, modelindex, sound, bouyancy, waterfriction, flags);
}

//====================================
//
//====================================
tempentity_t* CL_TempSprite( const Vector& origin, const Vector& velocity, Float scale, Uint32 modelindex, Int32 rendermode, Int32 renderfx, Float alpha, Float life, Int32 sound, Int32 flags )
{
	return gTempEntities.CreateTempSprite(origin, velocity, scale, modelindex, rendermode, renderfx, alpha, life, sound, flags);
}

//====================================
//
//====================================
void CL_ParticleExplosion1( const Vector& origin )
{
	gLegacyParticles.CreateParticleExplosion1(origin);
}

//====================================
//
//====================================
void CL_ParticleExplosion2( const Vector& origin, Uint32 colorstart, Uint32 colorlength )
{
	gLegacyParticles.CreateParticleExplosion2(origin, colorstart, colorlength);
}

//====================================
//
//====================================
void CL_BlobExplosion( const Vector& origin )
{
	gLegacyParticles.CreateBlobExplosion(origin);
}

//====================================
//
//====================================
void CL_RocketExplosion( const Vector& origin, Uint32 color )
{
	gLegacyParticles.CreateRocketExplosion(origin, color);
}

//====================================
//
//====================================
void CL_ParticleEffect( const Vector& origin, const Vector& velocity, Uint32 color, Uint32 count )
{
	gLegacyParticles.CreateParticleEffect(origin, velocity, color, count);
}

//====================================
//
//====================================
void CL_LavaSplash( const Vector& origin )
{
	gLegacyParticles.CreateLavaSplash(origin);
}

//====================================
//
//====================================
void CL_TeleportSplash( const Vector& origin )
{
	gLegacyParticles.CreateTeleportSplash(origin);
}

//====================================
//
//====================================
void CL_RocketTrail( const Vector& start, const Vector& end, Uint32 type )
{
	gLegacyParticles.CreateRocketTrail(start, end, type);
}

//====================================
//
//====================================
void CL_InitEffectsInterface( cl_efxapi_t &efxAPI )
{
	efxAPI = EFXAPI_INTERFACE_FUNCS;
}

//====================================
//
//====================================
beam_t* CL_BeamLightning( const Vector& src, const Vector& end, Int32 modelindex, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Int32 flags )
{
	return gBeamRenderer.BeamLightning(src, end, modelindex, life, width, amplitude, brightness, speed, noisespeed, flags);
}

//====================================
//
//====================================
beam_t* CL_BeamCirclePoints( beam_types_t type, const Vector& src, const Vector& end, Int32 modelindex, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Uint32 startframe, Float framerate, Float r, Float g, Float b, Int32 flags )
{
	return gBeamRenderer.BeamCirclePoints(type, src, end, modelindex, life, width, amplitude, brightness, speed, noisespeed, startframe, framerate, r, g, b, flags);
}

//====================================
//
//====================================
beam_t* CL_BeamEntityPoint( entindex_t startentity, Int32 attachment, const Vector& end, Int32 modelindex, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Uint32 startframe, Float framerate, Float r, Float g, Float b, Int32 flags )
{
	return gBeamRenderer.BeamEntityPoint(startentity, attachment, end, modelindex, life, width, amplitude, brightness, speed, noisespeed, startframe, framerate, r, g, b, flags);
}

//====================================
//
//====================================
beam_t* CL_BeamEntities( entindex_t startentity, entindex_t endentity, Int32 attachment1, Int32 attachment2, Int32 modelindex, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Uint32 startframe, Float framerate, Float r, Float g, Float b, Int32 flags )
{
	return gBeamRenderer.BeamEntities(startentity, endentity, attachment1, attachment2, modelindex, life, width, amplitude, brightness, speed, noisespeed, startframe, framerate, r, g, b, flags);
}

//====================================
//
//====================================
beam_t* CL_BeamFollow( entindex_t startentity, Int32 attachment, Int32 modelindex, Float life, Float width, Float brightness, Float r, Float g, Float b )
{
	return gBeamRenderer.BeamFollow(startentity, attachment, modelindex, life, width, brightness, r, g, b);
}

//====================================
//
//====================================
beam_t* CL_BeamPoints( const Vector& src, const Vector& end, Int32 modelindex, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Uint32 startframe, Float framerate, Float r, Float g, Float b, Int32 flags )
{
	return gBeamRenderer.BeamPoints(src, end, modelindex, life, width, amplitude, brightness, speed, noisespeed, startframe, framerate, r, g, b, flags);
}

//====================================
//
//====================================
beam_t* CL_BeamRing( entindex_t startentity, entindex_t endentity, Int32 attachment1, Int32 attachment2, Int32 modelindex, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Uint32 startframe, Float framerate, Float r, Float g, Float b, Int32 flags )
{
	return gBeamRenderer.BeamRing(startentity, endentity, attachment1, attachment2, modelindex, life, width, amplitude, brightness, speed, noisespeed, startframe, framerate, r, g, b, flags);
}

//====================================
//
//====================================
beam_t* CL_BeamVaporTrail( const Vector& src, const Vector& end, Int32 modelindex1, Int32 modelindex2, Float colorfadedelay, Float colorfadetime, Float life, Float width, Float brightness, Float r1, Float g1, Float b1, Float r2, Float g2, Float b2, Int32 flags )
{
	return gBeamRenderer.BeamVaporTrail(src, end, modelindex1, modelindex2, colorfadedelay, colorfadetime, life, width, brightness, r1, g1, b1, r2, g2, b2, flags);
}

//====================================
//
//====================================
void CL_KillEntityBeams( entindex_t entityindex )
{
	gBeamRenderer.KillEntityBeams(entityindex);
}

//====================================
//
//====================================
void CL_CreateBlackHole( Int32 key, const Vector& origin, Float life, Float scale, Float strength, Float rotation, Float growthtime, Float shrinktime )
{
	gBlackHoleRenderer.CreateBlackHole(key, origin, life, scale, strength, rotation, growthtime, shrinktime);
}

//====================================
//
//====================================
void CL_KillBlackHole( Int32 key )
{
	gBlackHoleRenderer.KillBlackHole(key);
}

//====================================
//
//====================================
void CL_SetSunFlare( entindex_t entindex, bool active, Float pitch, Float roll, Float scale, const Vector& color, bool portalSunFlare )
{
	gLensFlareRenderer.SetSunFlare(entindex, active, pitch, roll, scale, color, portalSunFlare);
}

//====================================
//
//====================================
void CL_AddSkyTextureSet( const Char* pstrSkyTextureName, Int32 skysetindex )
{
	gSkyRenderer.AddSkyTextureSet(pstrSkyTextureName, skysetindex);
}

//====================================
//
//====================================
void CL_SetSkyTexture( Int32 skysetindex )
{
	gSkyRenderer.SetSkyTexture(skysetindex);
}

//====================================
//
//====================================
tracer_t* CL_CreateTracer( const Vector& origin, const Vector& velocity, const Vector& color, Float alpha, Float width, Float length, Float life, tracer_type_t type )
{
	return gTracers.CreateTracer(origin, velocity, color, alpha, width, length, life, type);
}