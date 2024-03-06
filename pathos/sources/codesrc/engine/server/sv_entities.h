/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef SV_ENTITIES_H
#define SV_ENTITIES_H
extern bool SV_SetModel( edict_t* pedict, const Char* pstrFilepath, bool setbounds );
extern bool SV_SetModel( edict_t* pedict, const Char* pstrFilepath, bool setbounds );
extern void SV_SetMinsMaxs( edict_t* pedict, const Vector& mins, const Vector& maxs );
extern void SV_SetSize( edict_t* pedict, const Vector& size );
extern void SV_SetOrigin( edict_t* pedict, const Vector& origin );
extern void SV_AddToTouched( entindex_t hitent, trace_t& trace, const Vector& velocity );
extern Int32 SV_GetNumEdicts( void );
extern bool SV_InitPrivateData( edict_t* pedict, const Char* pstrClassname );
extern edict_t* SV_GetEdictByIndex( entindex_t entindex );
extern edict_t* SV_CreateEntity( const Char* pstrClassName );
extern void SV_RemoveEntity( edict_t* pentity );
extern bool SV_DropToFloor( edict_t* pentity );
extern bool SV_NPC_WalkMove( edict_t* pentity, Float yaw, Float dist, walkmove_t movemode );
#endif