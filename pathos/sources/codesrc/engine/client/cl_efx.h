/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef CL_EFX_H
#define CL_EFX_H

struct cl_efxapi_t;
struct cl_dlight_t;
struct cl_entity_t;
struct tempentity_t;
struct beam_t;

enum beam_types_t;

extern void CL_SetFade( Uint32 layerindex, Float duration, Float holdtime, Int32 flags, const color24_t& color, byte alpha, Float timeoffset );
extern void CL_SetMotionBlur( bool active, Float blurfade, bool override );
extern void CL_SpawnParticleSystem( const Vector& origin, const Vector& direction, part_script_type_t scripttype, const Char* pstrFilepath, Int32 id, entindex_t entindex, Int32 attachment, Int32 boneindex, Int32 attachflags );
extern void CL_RemoveParticleSystem( Uint32 id, entindex_t entindex, bool keepcached );
extern void CL_KillEntityParticleSystems( Int32 entindex );
extern void CL_PrecacheParticleScript( part_script_type_t type, const Char* pstrFilepath );
extern void CL_CreateGenericDecal( const Char *pstrname, const Vector& origin, const Vector& normal, Int32 flags, entindex_t entindex, Float life, Float fadetime, Float growthtime );
extern void CL_DecalVBMEntity( const Vector& origin, const Vector& normal, decalgroupentry_t* pentry, cl_entity_t* pentity, Int32 flags );
extern CDecalList& CL_GetDecalList( void );
extern void CL_SetLightStyle( Int32 style, const Char* pstrpattern, bool interpolate, Float framerate );
extern void CL_FreeEntityData( entindex_t entindex, Int32 flags );
extern void CL_SetDayStage( daystage_t daystage );
extern void CL_SetSpecialFog( bool enablespecialfog );
extern cl_dlight_t* CL_AllocDynamicSpotLight( Int32 key, Int32 subkey, bool isstatic, bool noshadow, cl_entity_t *pentity );
extern cl_dlight_t* CL_AllocDynamicPointLight( Int32 key, Int32 subkey, bool isstatic, bool noshadow, cl_entity_t *pentity );
extern void CL_SetSkyboxParameters( const Vector& worldorigin, Float skysize, Float fogend, Float fogstart, const Vector& fogcolor, Int32 skytexturesetindex, bool affectskybox, bool isactive );
extern void CL_SetFogParameters( entindex_t entindex, const Vector& color, Float start, Float end, bool affectsky, Float blendtime );
extern void CL_CreateCableEntity( const Vector& start, const Vector& end, Uint32 depth, Uint32 width, Uint32 numsegments );
extern cl_entity_t* CL_AllocStaticSpriteEntity( void );
extern void CL_BreakModel( const Vector& origin, const Vector& size, const Vector& velocity, Uint32 random, Float life, Uint32 num, Uint32 modelindex, Int32 sound, Float buoyancy, Float waterfriction, Int32 flags );
extern void CL_Bubbles( const Vector& mins, const Vector& maxs, Float height, Uint32 modelindex, Uint32 num, Float speed );
extern void CL_BubbleTrail( const Vector& start, const Vector& end, Float height, Uint32 modelindex, Uint32 num, Float speed );
extern void CL_FunnelSprite( const Vector& origin, const Vector& color, Float alpha, Uint32 modelindex, bool reverse );
extern void CL_SphereModel( const Vector& origin, Float speed, Float life, Uint32 num, Uint32 modelindex, Float buoyancy, Float waterfriction, Int32 sound );
extern tempentity_t* CL_TempModel( const Vector& origin, const Vector& velocity, const Vector& angles, Float life, Uint32 modelindex, Int32 sound, Float buoyancy, Float waterfriction, Int32 flags );
extern tempentity_t* CL_TempSprite( const Vector& origin, const Vector& velocity, Float scale, Uint32 modelindex, Int32 rendermode, Int32 renderfx, Float alpha, Float life, Int32 sound, Int32 flags );
extern void CL_ParticleExplosion1( const Vector& origin );
extern void CL_ParticleExplosion2( const Vector& origin, Uint32 colorstart, Uint32 colorlength );
extern void CL_BlobExplosion( const Vector& origin );
extern void CL_RocketExplosion( const Vector& origin, Uint32 color );
extern void CL_ParticleEffect( const Vector& origin, const Vector& velocity, Uint32 color, Uint32 count );
extern void CL_LavaSplash( const Vector& origin );
extern void CL_TeleportSplash( const Vector& origin );
extern void CL_RocketTrail( const Vector& start, const Vector& end, Uint32 type );
extern void CL_InitEffectsInterface( cl_efxapi_t &efxAPI );
extern void CL_SetGaussianBlur( bool active, Float alpha );
extern cl_entity_t* CL_AllocTempSpriteEntity( Int32 key, Float life );
extern mlight_t* CL_AllocEntityLight( Int32 key, Float life, Int32 attachment );
extern beam_t* CL_BeamLightning( const Vector& src, const Vector& end, Int32 modelindex, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Int32 flags );
extern beam_t* CL_BeamCirclePoints( beam_types_t type, const Vector& src, const Vector& end, Int32 modelindex, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Uint32 startframe, Float framerate, Float r, Float g, Float b, Int32 flags );
extern beam_t* CL_BeamEntityPoint( entindex_t startentity, Int32 attachment, const Vector& end, Int32 modelindex, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Uint32 startframe, Float framerate, Float r, Float g, Float b, Int32 flags );
extern beam_t* CL_BeamEntities( entindex_t startentity, entindex_t endentity, Int32 attachment1, Int32 attachment2, Int32 modelindex, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Uint32 startframe, Float framerate, Float r, Float g, Float b, Int32 flags );
extern beam_t* CL_BeamFollow( entindex_t startentity, Int32 attachment, Int32 modelindex, Float life, Float width, Float brightness, Float r, Float g, Float b );
extern beam_t* CL_BeamPoints( const Vector& src, const Vector& end, Int32 modelindex, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Uint32 startframe, Float framerate, Float r, Float g, Float b, Int32 flags );
extern beam_t* CL_BeamRing( entindex_t startentity, entindex_t endentity, Int32 attachment1, Int32 attachment2, Int32 modelindex, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Uint32 startframe, Float framerate, Float r, Float g, Float b, Int32 flags );
extern beam_t* CL_BeamVaporTrail( const Vector& src, const Vector& end, Int32 modelindex1, Int32 modelindex2, Float colorfadedelay, Float colorfadetime, Float life, Float width, Float brightness, Float r1, Float g1, Float b1, Float r2, Float g2, Float b2, Int32 flags );
extern void CL_KillEntityBeams( entindex_t entityindex );
extern void CL_CreateBlackHole( Int32 key, const Vector& origin, Float life, Float scale, Float strength, Float rotation, Float growthtime, Float shrinktime );
extern void CL_KillBlackHole( Int32 key );
extern void CL_SetSunFlare( entindex_t entindex, bool active, Float pitch, Float roll, Float scale, const Vector& color, bool portalSunFlare );
extern void CL_AddSkyTextureSet( const Char* pstrSkyTextureName, Int32 skysetindex );
extern void CL_SetSkyTexture( Int32 skysetindex );
#endif //CL_EFX_H