/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef BSP_SHARED_H
#define BSP_SHARED_H

//
// Common functions for setting up other data for BSP files
//

extern void BSP_SetNodeParent( mnode_t* pnode, mnode_t* pparent );
extern void BSP_MakeHullZero( brushmodel_t& model );
extern void BSP_SetupPAS( brushmodel_t& model );
extern bool BSP_CalcSurfaceExtents( msurface_t* psurf, brushmodel_t& model, Uint32 maxextents );
extern void BSP_SetSamplingLightData( brushmodel_t& model );
extern void BSP_ReleaseLightmapData( brushmodel_t& model );
extern void BSP_ReserveWaterLighting( void );
extern void BSP_Model_ReserveWaterLighting( brushmodel_t& model, color24_t* psrclightdataptrs[] );
#endif