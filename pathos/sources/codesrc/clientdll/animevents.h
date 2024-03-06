/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef ANIMEVENTS_H
#define ANIMEVENTS_H

extern void EV_MuzzleFlashLight( const Vector& position, Int32 key );
extern void EV_SimpleMuzzleFlash( const Vector& position, const Vector& angles, entindex_t entindex, Int32 attachment );
extern void EV_AttachmentDirectionMuzzleFlash( const cl_entity_t *pentity, const mstudioevent_t *pevent, Int32 attachment1, Int32 attachment2 );
extern void EV_AngleForwardMuzzleFlash( Int32 attachment, const Vector origin, const cl_entity_t *pentity, const mstudioevent_t *pevent );
extern void EV_EjectBullet( const Vector& attachment, const mstudioevent_t *pevent, const cl_entity_t *pentity );
extern void EV_NPC_StepSound( cl_entity_t *pentity, const Char* pstrsteptype );
extern void EV_NPC_StepSound_Legacy( cl_entity_t *pentity, const mstudioevent_t *pevent );
extern void EV_ViewModelMuzzleflash( cl_entity_t* pentity, const mstudioevent_t *pevent, const Vector& position, Int32 attachment );
extern void EV_WeaponVaporTrail_Smoke( const Vector& attachment, const mstudioevent_t *pevent, const cl_entity_t *pentity );
extern void EV_WeaponVaporTrail_Glow( const Vector& attachment, const mstudioevent_t *pevent, const cl_entity_t *pentity );
#endif //ANIMEVENTS_H