/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef R_GLQUERIES_H
#define R_GLQUERIES_H

struct glowquery_t;
enum querytype_t;

extern void R_InitQueryObjects( void );
extern void R_ClearQueryObjects( void );
extern glowquery_t* R_AllocQueryObject( Int32 key, Uint32 numqueries, Uint32 renderpassidx, querytype_t type );
extern void R_ReleaseQueryObject( glowquery_t* pQuery );
extern void R_ReleaseRenderPassQueryObjects( Uint32 renderpassidx, querytype_t type );
extern Float R_CalcOcclusionFactor( const Vector& origin, Int32 key, Float width, Float scale, Float glowSpeed, Uint32 numGlowTraces, querytype_t queryType, struct glowstate_t& glowState, bool useQueries, bool traceAll, bool checkSky, bool checkPortal, Float (*viewMatrix)[4], void *pContext, void (*pfnPreDrawFnPtr)( void* pContext ), void (*pfnDrawFnPtr)( void* pContext, const Vector& origin ) );
#endif //R_GLQUERIES_H