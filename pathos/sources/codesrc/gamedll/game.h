/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef GAME_H
#define GAME_H

extern CCVar* g_pCvarCheats;
extern CCVar* g_pCvarTriggerDebug;
extern CCVar* g_pCvarWeaponHolster;
extern CCVar* g_pCvarHoldToZoom;
extern CCVar* g_pCvarGravity;
extern CCVar* g_pCvarAutoAim;
extern CCVar* g_pCvarHoldToWalk;
extern CCVar* g_pCvarOldSchoolBlood;
extern CCVar* g_pCvarOldSchoolExplosions;

enum bullet_types_t;

class CFlexManager;
class CSentencesFile;

// Decal list object
extern class CDecalList gDecalList;
// Flex manager object
extern CFlexManager* g_pFlexManager;
// Sentences file object
extern CSentencesFile* g_pSentencesFile;

// Number of penetrations done by NPCs
extern Uint32 g_nbNPCPenetrations;

// Smoke sprite precache index
extern Int32 g_smokeSpriteIndex;

// Old-school explosion sprite
extern const Char OLDSCHOOL_EXPLOSION_SPRITE_PATH[];
// Old-school explosion sound
extern const Char OLDSCHOOL_EXPLOSION_SOUND_PATH[];

extern bool InitGameObjects( void );
extern void ClearGameObjects( void );
extern bool InitGame( void );
extern void ClearGame( void );
extern bool InitDecalList( void );
extern bool InitMaterialDefinitions( void );
extern void PrecachePlayerItems( void );
extern void DumpGlobals( void );
extern void ToggleBikeBlockers( bool enable );
extern void PrecacheGenericResources( void );
extern bool InitSentences( void );

extern void FireBullets( Uint32 nbshots, const Vector& gunPosition, const Vector& aimForward, const Vector& aimRight, const Vector& aimUp, const Vector& spread, Float distance, bullet_types_t bulletType, Int32 tracerFrequency, Float damage, CBaseEntity* pAttacker, CBaseEntity* pWeapon = nullptr, bool mirrorTracer = false );
extern void RadiusDamage( const Vector& vecPosition, CBaseEntity* pInflictor, CBaseEntity* pAttacker, Float damageDealt, Float damageRadius, Int32 classToIgnore, Int32 damageFlags, CBaseEntity* pGibEntity = nullptr, CPlayerWeapon* pWeapon = nullptr );
#endif //GAME_H