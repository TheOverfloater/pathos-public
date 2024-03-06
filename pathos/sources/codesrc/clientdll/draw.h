/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef DRAW_H
#define DRAW_H

struct ref_params_t;
struct cl_dlight_t;

extern bool CL_DrawHUD( bool hudOnly );
extern void CL_SetupView( const ref_params_t& params );
extern bool CL_DrawNormal( void );
extern bool CL_DrawTransparent( void );
extern bool CL_DrawViewObjects( void );
extern bool CL_DrawViewObjectsForVSM( cl_dlight_t* dl );
extern bool CL_DrawLadders( void );
extern bool CL_DrawLaddersForVSM( cl_dlight_t* dl );
#endif