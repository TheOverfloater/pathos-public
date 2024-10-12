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

void BSP_SetNodeParent( mnode_t* pnode, mnode_t* pparent );
void BSP_MakeHullZero( brushmodel_t& model );
void BSP_SetupPAS( brushmodel_t& model );
bool BSP_CalcSurfaceExtents( msurface_t* psurf, brushmodel_t& model );
#endif