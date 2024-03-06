/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef EFXAPI_H
#define EFXAPI_H

struct cl_dlight_t;
struct cl_entity_t;
struct decalgroupentry_t;
struct tempentity_t;
struct mlight_t;
struct beam_t;

enum beam_types_t;

struct cl_efxapi_t
{
	// Miscellanous
	cl_entity_t*			(*pfnAllocStaticSpriteEntity)( void );
	cl_entity_t*			(*pfnAllocTempSpriteEntity)( Int32 key, Float life );

	void					(*pfnCreateCableEntity)( const Vector& start, const Vector& end, Uint32 depth, Uint32 width, Uint32 numsegments );
	void					(*pfnSetFogParameters)( entindex_t entindex, const Vector& color, Float start, Float end, bool affectsky, Float blendtime );
	void					(*pfnSetSkyboxParameters)( const Vector& worldorigin, Float skysize, Float fogend, Float fogstart, const Vector& fogcolor, Int32 skytexturesetindex, bool affectskybox, bool isactive );
	mlight_t*				(*pfnAllocEntityLight)( Int32 key, Float life, Int32 attachment );
	cl_dlight_t*			(*pfnAllocDynamicPointLight)( Int32 key, Int32 subkey, bool isstatic, bool noshadow, cl_entity_t *pentity );
	cl_dlight_t*			(*pfnAllocDynamicSpotLight)( Int32 key, Int32 subkey, bool isstatic, bool noshadow, cl_entity_t *pentity );
	void					(*pfnSetDayStage)( daystage_t daystage );
	void					(*pfnSetSpecialFog)( bool enablespecialfog );
	void					(*pfnFreeEntityData)( entindex_t entindex, Int32 flags );
	void					(*pfnSetLightStyle)( Int32 style, const Char* pstrpattern, bool interpolate, Float framerate );

	// Decals
	CDecalList&				(*pfnGetDecalList)( void );
	void					(*pfnDecalVBMEntity)( const Vector& origin, const Vector& normal, decalgroupentry_t* pentry, cl_entity_t* pentity, Int32 flags );
	void					(*pfnCreateGenericDecal)( const Char *pstrname, const Vector& origin, const Vector& normal, Int32 flags, entindex_t entindex, Float life, Float fadetime, Float growthtime );

	// Particles
	void					(*pfnPrecacheParticleScript)( part_script_type_t type, const Char* pstrFilepath );
	void					(*pfnRemoveParticleSystem)( Uint32 id, entindex_t entindex, bool keepcached );
	void					(*pfnSpawnParticleSystem)( const Vector& origin, const Vector& direction, part_script_type_t scripttype, const Char* pstrFilepath, Int32 id, entindex_t entindex, Int32 attachment, Int32 boneindex, Int32 attachflags );
	void					(*pfnKillEntityParticleSystems)( Int32 entindex );

	// Post process
	void					(*pfnSetMotionBlur)( bool active, Float blurfade, bool override );
	void					(*pfnSetFade)( Uint32 layerindex, Float duration, Float holdtime, Int32 flags, const color24_t& color, byte alpha, Float timeoffset );
	void					(*pfnSetGaussianBlur)( bool active, Float alpha );

	// Temporary entities
	void					(*pfnBreakModel)( const Vector& origin, const Vector& size, const Vector& velocity, Uint32 random, Float life, Uint32 num, Uint32 modelindex, Int32 sound, Float buoyancy, Float waterfriction, Int32 flags );
	void					(*pfnBubbles)( const Vector& mins, const Vector& maxs, Float height, Uint32 modelindex, Uint32 num, Float speed );
	void					(*pfnBubbleTrail)( const Vector& start, const Vector& end, Float height, Uint32 modelindex, Uint32 num, Float speed );
	void					(*pfnFunnelSprite)( const Vector& origin, const Vector& color, Float alpha, Uint32 modelindex, bool reverse );
	void					(*pfnSphereModel)( const Vector& origin, Float speed, Float life, Uint32 num, Uint32 modelindex, Float buoyancy, Float waterfriction, Int32 sound );
	tempentity_t*			(*pfnTempModel)( const Vector& origin, const Vector& velocity, const Vector& angles, Float life, Uint32 modelindex, Int32 sound, Float buoyancy, Float waterfriction, Int32 flags );
	tempentity_t*			(*pfnTempSprite)( const Vector& origin, const Vector& velocity, Float scale, Uint32 modelindex, Int32 rendermode, Int32 renderfx, Float alpha, Float life, Int32 sound, Int32 flags );

	// Legacy particle effects
	void					(*pfnParticleExplosion1)( const Vector& origin );
	void					(*pfnParticleExplosion2)( const Vector& origin, Uint32 colorstart, Uint32 colorlength );
	void					(*pfnBlobExplosion)( const Vector& origin );
	void					(*pfnRocketExplosion)( const Vector& origin, Uint32 color );
	void					(*pfnParticleEffect)( const Vector& origin, const Vector& velocity, Uint32 color, Uint32 count );
	void					(*pfnLavaSplash)( const Vector& origin );
	void					(*pfnTeleportSplash)( const Vector& origin );
	void					(*pfnRocketTrail)( const Vector& start, const Vector& end, Uint32 type );

	// Beam effects
	beam_t*					(*pfnBeamLightning)( const Vector& src, const Vector& end, Int32 modelindex, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Int32 flags );
	beam_t*					(*pfnBeamCirclePoints)( beam_types_t type, const Vector& src, const Vector& end, Int32 modelindex, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Uint32 startframe, Float framerate, Float r, Float g, Float b, Int32 flags );
	beam_t*					(*pfnBeamEntityPoint)( entindex_t startentity, Int32 attachment, const Vector& end, Int32 modelindex, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Uint32 startframe, Float framerate, Float r, Float g, Float b, Int32 flags );
	beam_t*					(*pfnBeamEntities)( entindex_t startentity, entindex_t endentity, Int32 attachment1, Int32 attachment2, Int32 modelindex, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Uint32 startframe, Float framerate, Float r, Float g, Float b, Int32 flags );
	beam_t*					(*pfnBeamFollow)( entindex_t startentity, Int32 attachment, Int32 modelindex, Float life, Float width, Float brightness, Float r, Float g, Float b );
	beam_t*					(*pfnBeamPoints)( const Vector& src, const Vector& end, Int32 modelindex, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Uint32 startframe, Float framerate, Float r, Float g, Float b, Int32 flags );
	beam_t*					(*pfnBeamRing)( entindex_t startentity, entindex_t endentity, Int32 attachment1, Int32 attachment2, Int32 modelindex, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Uint32 startframe, Float framerate, Float r, Float g, Float b, Int32 flags );
	beam_t*					(*pfnBeamVaporTrail)( const Vector& src, const Vector& end, Int32 modelindex1, Int32 modelindex2, Float colorfadedelay, Float colorfadetime, Float life, Float width, Float brightness, Float r1, Float g1, Float b1, Float r2, Float g2, Float b2, Int32 flags );
	void					(*pfnKillEntityBeams)( entindex_t entityindex );

	// Black holes
	void					(*pfnCreateBlackHole)( Int32 key, const Vector& origin, Float life, Float scale, Float strength, Float rotation, Float growthtime, Float shrinktime );
	void					(*pfnKillBlackHole)( Int32 key );

	// Sun flare
	void					(*pfnSetSunFlare)( entindex_t entindex, bool active, Float pitch, Float roll, Float scale, const Vector& color, bool portalSunFlare );

	// Custom sky texture management
	void					(*pfnAddSkyTextureSet)( const Char* pstrSkyTextureName, Int32 skysetindex );
	void					(*pfnSetSkyTexture)( Int32 skysetindex );
};
#endif //R_EFXAPI_H