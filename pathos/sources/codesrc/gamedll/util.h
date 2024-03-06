/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef UTIL_H
#define UTIL_H

#include "snd_shared.h"
#include "beam_shared.h"
#include "trace.h"

enum part_script_type_t;
enum flextypes_t;
enum usemode_t;
enum beam_types_t;

namespace Util
{
	extern void RemoveEntity( CBaseEntity* pEntity );
	extern void RemoveEntity( edict_t* pEdict );
	extern edict_t* FindEntityByString( edict_t* pStartEntity, const Char* pstrFieldName, const Char* pstrValue );
	extern edict_t* FindEntityByClassname( edict_t* pStartEntity, const Char* pstrValue );
	extern edict_t* FindEntityByTargetName( edict_t* pStartEntity, const Char* pstrValue );
	extern edict_t* FindEntityByTarget( edict_t* pStartEntity, const Char* pstrValue );
	extern edict_t* FindEntityInBBox( edict_t* pStartEntity, const Vector& mins, const Vector& maxs );
	extern void SetMoveDirection( entity_state_t& state );
	extern void SetAxisDirection( entity_state_t& state, Int32 flags, Int32 flagz, Int32 flagx );
	extern Float GetAxisValue( Int32 flags, const Vector& angles, Int32 flagz, Int32 flagx );
	extern Float GetAxisDelta( Int32 flags, const Vector& angle1, const Vector& angle2, Int32 flagz, Int32 flagx );
	extern void TraceLine( const Vector& start, const Vector& end, bool ignorenpcs, bool usehitboxes, bool ignoreglass, bool hitcorpses, const edict_t* pignoreent, trace_t& tr );
	extern void TraceLine( const Vector& start, const Vector& end, bool ignorenpcs, bool usehitboxes, bool ignoreglass, const edict_t* pignoreent, trace_t& tr );
	extern void TraceLine( const Vector& start, const Vector& end, bool ignorenpcs, bool usehitboxes, const edict_t* pignoreent, trace_t& tr );
	extern void TraceHull( const Vector& start, const Vector& end, bool ignorenpcs, bool usehitboxes, bool ignoreglass, hull_types_t hulltype, const edict_t* pignoreent, trace_t& tr );
	extern void TraceHull( const Vector& start, const Vector& end, bool ignorenpcs, bool usehitboxes, hull_types_t hulltype, const edict_t* pignoreent, trace_t& tr );
	extern void TraceModel( const CBaseEntity* pEntity, const Vector& start, const Vector& end, bool usehitboxes, hull_types_t hulltype, trace_t& tr );
	extern const Char* TraceTexture( entindex_t hitentity, const Vector& position, const Vector& planeNormal );
	extern void FireTargets( const Char* pstrtargetname, CBaseEntity* pActivator, CBaseEntity* pCaller, usemode_t useMode, Float value );
	extern bool IsMasterTriggered( const Char* pstrtargetname, const CBaseEntity* pentity, const CBaseEntity* pslave );
	extern bool IsNullEntity( const edict_t* pentity );
	extern bool IsNullEntity( const entindex_t entindex );
	extern bool IsVBMEntity( const edict_t* pentity );
	extern bool IsVBMEntity( const entindex_t entindex );
	extern class CBaseEntity* GetHostPlayer( void );
	extern class CBaseEntity* GetPlayerByIndex( Uint32 index );
	extern void WarnEmptyEntity( const edict_t* pentity );
	extern void EntityConPrintf( const edict_t* pentity, const Char *fmt, ... );
	extern void EntityConDPrintf( const edict_t* pentity, const Char *fmt, ... );
	extern void CreateParticles( const Char* pstrscriptname, const Vector& origin, const Vector& direction, part_script_type_t type );
	extern void CreateParticles( const Char* pstrscriptname, const Vector& origin, const Vector& direction, part_script_type_t type, const edict_t* pentity, Uint32 attachment, Int32 id, Int32 boneindex = NO_POSITION, Int32 attachflags = PARTICLE_ATTACH_NONE );
	extern void PrecacheEntity( const Char* pstrClassname );
	extern void ShowMessage( const Char* pmsgname, CBaseEntity* pPlayer );
	extern void ShowMessageAllPlayers( const Char* pmsgname );
	extern void EmitEntitySound( const CBaseEntity* pEntity, const Char* pstrfilename, snd_channels_t channel, Float volume = VOL_NORM, Float attenuation = ATTN_NORM, Int32 pitch = PITCH_NORM, Int32 flags = SND_FL_NONE, const CBaseEntity* pPlayer = nullptr, Float timeoffset = 0 );
	extern void EmitEntitySound( const CBaseEntity* pEntity, string_t filename, snd_channels_t channel, Float volume = VOL_NORM, Float attenuation = ATTN_NORM, Int32 pitch = PITCH_NORM, Int32 flags = SND_FL_NONE, const CBaseEntity* pPlayer = nullptr, Float timeoffset = 0 );
	extern void EmitAmbientSound( const Vector& origin, const Char* pstrfilename, Float volume = VOL_NORM, Float attenuation = ATTN_NORM, Int32 pitch = PITCH_NORM, Int32 flags = SND_FL_NONE, const CBaseEntity* pEntity = nullptr, const CBaseEntity* pPlayer = nullptr, Float timeoffset = 0 );
	extern void EmitAmbientSound( const Vector& origin, string_t filename, Float volume = VOL_NORM, Float attenuation = ATTN_NORM, Int32 pitch = PITCH_NORM, Int32 flags = SND_FL_NONE, const CBaseEntity* pEntity = nullptr, const CBaseEntity* pPlayer = nullptr, Float timeoffset = 0 );
	extern void StopEntitySounds( CBaseEntity* pEntity, snd_channels_t channel, const CBaseEntity* pPlayer = nullptr );
	extern void SpawnBloodParticles( const trace_t& tr, bloodcolor_t color, bool isplayer );
	extern void SpawnBloodParticles( const Vector& origin, const Vector& direction, bloodcolor_t color, bool isplayer );
	extern void SpawnBloodDecal( const trace_t& tr, bloodcolor_t color, bool decalvbm );
	extern void CreateGenericDecal( const Vector& origin, const Vector* pnormal, const Char* pstrname, Int32 decalflags, entindex_t entindex = NO_ENTITY_INDEX, Float life = 0, Float fadetime = 0, Float growthtime = 0, const edict_t* pplayer = nullptr );
	extern void CreateVBMDecal( const Vector& origin, const Vector& normal, const Char* pstrname, const edict_t* pentity, Int32 decalflags );
	extern Float GetDamageForce( const edict_t& entity, Float damage );
	extern void ScreenFadePlayer( const edict_t* pplayer, const Vector& color, Float fadetime, Float fadeholdtime, Int32 alpha, Int32 flags, Int32 layer = 0, Float timeoffset = 0 );
	extern void ScreenFadePlayer( const edict_t* pplayer, const color24_t& color, Float fadetime, Float fadeholdtime, Int32 alpha, Int32 flags, Int32 layer = 0, Float timeoffset = 0 );
	extern void ScreenFadeAllPlayers( const Vector& color, Float fadetime, Float fadeholdtime, Int32 alpha, Int32 flags, Int32 layer = 0, Float timeoffset = 0 );
	extern void ScreenFadeAllPlayers( const color24_t& color, Float fadetime, Float fadeholdtime, Int32 alpha, Int32 flags, Int32 layer = 0, Float timeoffset = 0 );
	extern void Ricochet( const Vector &position, const Vector &direction, bool metalsound = false );
	extern Vector ClampVectorToBox( const Vector& vectoadj, const Vector& boxsize );
	extern CBaseEntity* FindEntityAtDirection( const Vector& origin, const Vector& angles, const edict_t* pedict );
	extern Int32 CalculateLightIllumination( const Vector& origin, const Vector& lightorigin, const Vector& lightcolor, Float lightradius );
	extern Int32 GetIllumination( const Vector& position );
	extern void CreateImpactEffects( const trace_t& tr, const Vector& traceBegin, bool createDecal, bool vbmDecal = false, bool playSounds = true, const Char* pstrDecalGroupName = nullptr );
	extern void MakeEntityDormant( edict_t* pedict );
	extern void CreateTempModel( const Vector& origin, const Vector& angles, const Vector& velocity, Float life, Uint32 num, const Char* pstrModelname, Int32 sound, Float bouyancy, Float waterfriction, Int32 flags, Int32 body );
	extern void CreateTempModel( const Vector& origin, const Vector& angles, const Vector& velocity, Float life, Uint32 num, Int32 modelindex, Int32 sound, Float bouyancy, Float waterfriction, Int32 flags, Int32 body );
	extern void CreateBreakModel( const Vector& origin, const Vector& size, const Vector& velocity, Uint32 randomvel, Float life, Uint32 num, const Char* pstrModelname, Int32 sound, Float bouyancy, Float waterfriction, Int32 flags );
	extern void CreateBreakModel( const Vector& origin, const Vector& size, const Vector& velocity, Uint32 randomvel, Float life, Uint32 num, Int32 modelindex, Int32 sound, Float bouyancy, Float waterfriction, Int32 flags );
	extern void CreateBubbles( const Vector& mins, const Vector& maxs, const Float height, const Char* pstrSpritename, Uint32 num, Float speed );
	extern void CreateBubbles( const Vector& mins, const Vector& maxs, const Float height, Int32 modelindex, Uint32 num, Float speed );
	extern void CreateBubbleTrail( const Vector& start, const Vector& end, const Float height, const Char* pstrSpritename, Uint32 num, Float speed );
	extern void CreateBubbleTrail( const Vector& start, const Vector& end, const Float height, Int32 modelindex, Uint32 num, Float speed );
	extern void CreateFunnelSprite( const Vector& origin, const Vector& color, Float alpha, const Char* pstrSpritename, bool reverse );
	extern void CreateFunnelSprite( const Vector& origin, const Vector& color, Float alpha, Int32 modelindex, bool reverse );
	extern void CreateSphereModel( const Vector& origin, Float speed, Float life, Uint32 num, const Char* pstrModelname, Int32 sound, Float bouyancy, Float waterfriction );
	extern void CreateSphereModel( const Vector& origin, Float speed, Float life, Uint32 num, Int32 modelindex, Int32 sound, Float bouyancy, Float waterfriction );
	extern void CreateTempSprite( const Vector& origin, const Vector& velocity, Float life, Float scale, const Char* pstrSpritename, rendermode_t rendermode, Int32 renderfx, Float alpha, Int32 sound, Int32 flags );
	extern void CreateTempSprite( const Vector& origin, const Vector& velocity, Float life, Float scale, Int32 modelindex, rendermode_t rendermode, Int32 renderfx, Float alpha, Int32 sound, Int32 flags );
	extern void CreateParticleExplosion1( const Vector& origin );
	extern void CreateParticleExplosion2( const Vector& origin, Int32 colorstart, Int32 colorlength );
	extern void CreateBlobExplosion( const Vector& origin );
	extern void CreateRocketExplosion( const Vector& origin, Int32 color );
	extern void CreateParticleEffect( const Vector& origin, const Vector& velocity, Int32 color, Uint32 count );
	extern void CreateLavaSplash( const Vector& origin );
	extern void CreateTeleportSplash( const Vector& origin );
	extern void CreateRocketTrail( const Vector& start, const Vector& end, Uint32 type );
	extern void PrecacheFixedNbSounds( const Char* pstrPattern, Uint32 count );
	extern void PrecacheVariableNbSounds( const Char* pstrPattern, Uint32& outcount );
	extern CString PlayRandomEntitySound( CBaseEntity* pEntity, const Char* pstrPattern, Int32 count, snd_channels_t channel = SND_CHAN_AUTO, Float volume = VOL_NORM, Float attenuation = ATTN_NORM, Int32 pitch = PITCH_NORM, Int32 flags = SND_FL_NONE );
	extern CString PlayRandomAmbientSound( const Vector& origin, const Char* pstrPattern, Int32 count, Float volume = VOL_NORM, Float attenuation = ATTN_NORM, Int32 pitch = PITCH_NORM, Int32 flags = SND_FL_NONE );
	extern void PlayWeaponClatterSound( const edict_t* pedict );
	extern void AlignEntityToSurface( edict_t* pedict );
	extern Vector GetRandomBloodVector( void );
	extern void ScreenShake( const Vector& origin, Float amplitude, Float frequency, Float duration, Float radius, bool inair = false, bool disruptcontrols = false );
	extern void ScreenShakeAll( const Vector& origin, Float amplitude, Float frequency, Float duration );
	extern void CreateSparks( const Vector& origin );
	extern void CreateDynamicLight( const Vector& origin, Float radius, Int32 r, Int32 g, Int32 b, Float life, Int32 decay, Float decaydelay, byte flags, Int32 entindex = NO_ENTITY_INDEX, Int32 attachment = NO_POSITION, Int32 lightstyle = LS_NORMAL );
	extern void CreateDynamicLightWithSubkey( const Vector& origin, Float radius, Int32 r, Int32 g, Int32 b, Float life, Int32 decay, Float decaydelay, byte flags, Int32 entindex, Int32 subkey, Int32 attachment = NO_POSITION, Int32 lightstyle = LS_NORMAL );
	extern void ExplosionSound( const Vector& origin );
	extern void ReadColor24FromString( const Char* pstrString, color24_t& outcolor );
	extern void ReadColor32FromString( const Char* pstrString, color32_t& outcolor );
	extern Float Approach( Float target, Float value, Float speed );
	extern Float ApproachAngle( Float targetvalue, Float curvalue, Float speed );
	extern Float AngleDistance( Float next, Float cur );
	extern void FixGroundEntities( const CBaseEntity* pGroundEntity, bool noDropToFloor = false );
	extern const Char* GetDebrisSound( breakmaterials_t material );
	extern void PrecacheDebrisSounds( breakmaterials_t material );
	extern bool IsDataEmpty( const byte* pdata, Uint16 size );
	extern void CreateRicochetEffect( const Vector& position, const Vector& normal );
	extern void PrecacheFlexScript( flextypes_t npctype, const Char* pstrscriptname );
	extern bool IsInViewCone( const Vector& origin, const Vector& angles, Float fieldOfView, const Vector& position );
	extern CBaseEntity* GetEntityFromTrace( const trace_t& tr );
	extern bool CheckTraceLine( const Vector& startPosition, const Vector& endPosition, bool ignoreNPCs = true, bool ignoreGlass = false, edict_t* pIgnoreEntity = nullptr );
	extern Float VectorToYaw( const Vector& inVector );
	extern Float VectorToPitch( const Vector& inVector );
	extern node_hull_types_t GetNodeHullForNPC( const CBaseEntity* pEntity );
	extern Uint64 GetNodeTypeForNPC( const CBaseEntity* pEntity );
	extern CBaseEntity* GetEntityByIndex( Int32 index );
	extern void FindLinkEntities( CBaseEntity* pLinkEntity, CArray<CBaseEntity*>& entitesArray, CBaseEntity* pNPC );
	extern void PrintScreenText( Int32 xcoord, Int32 ycoord, Float lifetime, const Char *fmt, ... );
	extern Float DotPoints( const Vector& src, const Vector& check, const Vector& direction );
	extern bool CheckToss( CBaseEntity* pEntity, const Vector& spot1, const Vector& spot2, Float gravityAdjust, Vector& outVelocity );
	extern bool CheckThrow( CBaseEntity* pEntity, const Vector& spot1, const Vector& spot2, Float speed, Float gravityAdjust, Vector& outVelocity );
	extern bool IsBoxVisible( CBaseEntity* pLooker, CBaseEntity* pTarget, Vector& targetOrigin, Float size );
	extern void CreateBeamPoints( const Vector& startpos, const Vector& endpos, const Char* pstrSpriteName, Uint32 startframe, Float framerate, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Float r, Float g, Float b, Int32 flags = FL_BEAM_NONE );
	extern void CreateBeamPoints( const Vector& startpos, const Vector& endpos, Int32 modelindex, Uint32 startframe, Float framerate, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Float r, Float g, Float b, Int32 flags = FL_BEAM_NONE );
	extern void CreateBeamEntities( const CBaseEntity* pstartentity, const CBaseEntity* pendentity, Int32 attachment1, Int32 attachment2, const Char* pstrSpriteName, Uint32 startframe, Float framerate, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Float r, Float g, Float b, Int32 flags = FL_BEAM_NONE );
	extern void CreateBeamEntities( const CBaseEntity* pstartentity, const CBaseEntity* pendentity, Int32 attachment1, Int32 attachment2, Int32 modelindex, Uint32 startframe, Float framerate, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Float r, Float g, Float b, Int32 flags = FL_BEAM_NONE );
	extern void CreateBeamEntityPoint( const CBaseEntity* pentity, const Vector& endpos, Int32 attachment, const Char* pstrSpriteName, Uint32 startframe, Float framerate, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Float r, Float g, Float b, Int32 flags = FL_BEAM_NONE );
	extern void CreateBeamEntityPoint( const CBaseEntity* pentity, const Vector& endpos, Int32 attachment, Int32 modelindex, Uint32 startframe, Float framerate, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Float r, Float g, Float b, Int32 flags = FL_BEAM_NONE );
	extern void CreateBeamSprite( const Vector& startpos, const Vector& endpos, const Char* pstrBeamSpriteName, const Char* pstrSpriteName, Uint32 startframe, Float framerate, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Float r, Float g, Float b, Int32 flags, Float sprscale, rendermode_t sprrendermode, Float spralpha );
	extern void CreateBeamSprite( const Vector& startpos, const Vector& endpos, Int32 modelindex, Int32 sprmodelindex, Uint32 startframe, Float framerate, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Float r, Float g, Float b, Int32 flags, Float sprscale, rendermode_t sprrendermode, Float spralpha );
	extern void CreateBeamTorus( const Vector& startpos, const Vector& endpos, const Char* pstrSpriteName, Uint32 startframe, Float framerate, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Float r, Float g, Float b, Int32 flags = FL_BEAM_NONE );
	extern void CreateBeamTorus( const Vector& startpos, const Vector& endpos, Int32 modelindex, Uint32 startframe, Float framerate, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Float r, Float g, Float b, Int32 flags = FL_BEAM_NONE );
	extern void CreateBeamDisk( const Vector& startpos, const Vector& endpos, const Char* pstrSpriteName, Uint32 startframe, Float framerate, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Float r, Float g, Float b, Int32 flags = FL_BEAM_NONE );
	extern void CreateBeamDisk( const Vector& startpos, const Vector& endpos, Int32 modelindex, Uint32 startframe, Float framerate, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Float r, Float g, Float b, Int32 flags = FL_BEAM_NONE );
	extern void CreateBeamCylinder( const Vector& startpos, const Vector& endpos, const Char* pstrSpriteName, Uint32 startframe, Float framerate, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Float r, Float g, Float b, Int32 flags = FL_BEAM_NONE );
	extern void CreateBeamCylinder( const Vector& startpos, const Vector& endpos, Int32 modelindex, Uint32 startframe, Float framerate, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Float r, Float g, Float b, Int32 flags = FL_BEAM_NONE );
	extern void CreateBeamOfType( beam_types_t type, const Vector& startpos, const Vector& endpos, Int32 modelindex, Uint32 startframe, Float framerate, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Float r, Float g, Float b, Int32 flags = FL_BEAM_NONE );
	extern void CreateBeamFollow( const CBaseEntity* pentity, Int32 attachment, const Char* pstrSpriteName, Float life, Float width, Float amplitude, Float brightness, Float r, Float g, Float b );
	extern void CreateBeamFollow( const CBaseEntity* pentity, Int32 attachment, Int32 modelindex, Float life, Float width, Float amplitude, Float brightness, Float r, Float g, Float b );
	extern void CreateBeamRing( const CBaseEntity* pstartentity, const CBaseEntity* pendentity, Int32 attachment1, Int32 attachment2, const Char* pstrSpriteName, Uint32 startframe, Float framerate, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Float r, Float g, Float b, Int32 flags = FL_BEAM_NONE );
	extern void CreateBeamRing( const CBaseEntity* pstartentity, const CBaseEntity* pendentity, Int32 attachment1, Int32 attachment2, Int32 modelindex, Uint32 startframe, Float framerate, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Float r, Float g, Float b, Int32 flags = FL_BEAM_NONE );
	extern void KillEntityBeams( const CBaseEntity* pentity );
	extern Int32 GetBoneIndexFromTrace( const trace_t& tr );
	extern Int64 GetBodyValueForSubmodel( const Char* pstrModelName, const Char* pstrSubmodelName );
};
#endif //UTIL_H