/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef SV_MOVE_H
#define SV_MOVE_H

extern bool SV_NPC_MoveStep( edict_t* pedict, const Vector& move, bool relink, bool nonpcs );
extern bool SV_NPC_MoveTest( edict_t* pedict, const Vector& move, bool relink );
extern void SV_NPC_MoveToOrigin( edict_t* pedict, const Vector& goalPosition, Float moveyaw, Float dist, npc_movetype_t movetype );

#endif //SV_MOVE_H