/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef ENTITIES_H
#define ENTITIES_H

extern void CL_AddEntities( void );
extern void CL_GetClientEntityList( const struct entitydata_t*& pEntitiesPtr, Uint32& numEntities );
extern void CL_ParseEntityList( void );
extern void CL_FreeEntityData( void );
extern void CL_AdjustEntityTimers( struct entity_state_t* pstate, Double jointime );
extern void CL_VBMEvent( const struct mstudioevent_t* pvbmevent, struct cl_entity_t* pentity );

#endif //ENTITIES_H