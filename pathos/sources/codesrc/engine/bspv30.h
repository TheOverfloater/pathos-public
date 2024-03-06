/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef BSPV30_H
#define BSPV30_H

//
// BSP version
//
#define BSPV30_VERSION	30

//
// Load functions
//
brushmodel_t* BSPV30_Load( const byte* pfile, const dheader_t* pheader, const Char* pstrFilename );

//
// Functions for loading specific lumps
//
bool BSPV30_LoadVertexes( const byte* pfile, brushmodel_t& model, const lump_t& lump );
bool BSPV30_LoadEdges( const byte* pfile, brushmodel_t& model, const lump_t& lump );
bool BSPV30_LoadSurfedges( const byte* pfile, brushmodel_t& model, const lump_t& lump );
bool BSPV30_LoadTextures( const byte* pfile, brushmodel_t& model, const lump_t& lump );
bool BSPV30_LoadLighting( const byte* pfile, brushmodel_t& model, const lump_t& lump );
bool BSPV30_LoadPlanes( const byte* pfile, brushmodel_t& model, const lump_t& lump );
bool BSPV30_LoadTexinfo( const byte* pfile, brushmodel_t& model, const lump_t& lump );
bool BSPV30_LoadFaces( const byte* pfile, brushmodel_t& model, const lump_t& lump );
bool BSPV30_LoadMarksurfaces( const byte* pfile, brushmodel_t& model, const lump_t& lump );
bool BSPV30_LoadVisibility( const byte* pfile, brushmodel_t& model, const lump_t& lump );
bool BSPV30_LoadLeafs( const byte* pfile, brushmodel_t& model, const lump_t& lump );
bool BSPV30_LoadNodes( const byte* pfile, brushmodel_t& model, const lump_t& lump );
bool BSPV30_LoadClipnodes( const byte* pfile, brushmodel_t& model, const lump_t& lump );
bool BSPV30_LoadEntities( const byte* pfile, brushmodel_t& model, const lump_t& lump );
bool BSPV30_LoadSubmodels( const byte* pfile, brushmodel_t& model, const lump_t& lump );

//
// Functions for setting up other data
//
void BSPV30_SetNodeParent( mnode_t* pnode, mnode_t* pparent );
void BSPV30_MakeHullZero( brushmodel_t& model );
void BSPV30_SetupPAS( brushmodel_t& model );
bool BSPV30_CalcSurfaceExtents( msurface_t* psurf, brushmodel_t& model );

#endif //BSPV30_H