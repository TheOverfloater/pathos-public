/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef PBSPV1_H
#define PBSPV1_H

//
// Load functions
//
brushmodel_t* PBSPV1_Load( const byte* pfile, const dpbspv1header_t* pheader, const Char* pstrFilename );

//
// Functions for loading specific lumps
//
bool PBSPV1_LoadVertexes( const byte* pfile, brushmodel_t& model, const lump_t& lump );
bool PBSPV1_LoadEdges( const byte* pfile, brushmodel_t& model, const lump_t& lump );
bool PBSPV1_LoadSurfedges( const byte* pfile, brushmodel_t& model, const lump_t& lump );
bool PBSPV1_LoadTextures( const byte* pfile, brushmodel_t& model, const lump_t& lump );
bool PBSPV1_LoadLighting( const byte* pfile, brushmodel_t& model, const lump_t& lump );
bool PBSPV1_LoadPlanes( const byte* pfile, brushmodel_t& model, const lump_t& lump );
bool PBSPV1_LoadTexinfo( const byte* pfile, brushmodel_t& model, const lump_t& lump );
bool PBSPV1_LoadFaces( const byte* pfile, brushmodel_t& model, const lump_t& lump );
bool PBSPV1_LoadMarksurfaces( const byte* pfile, brushmodel_t& model, const lump_t& lump );
bool PBSPV1_LoadVisibility( const byte* pfile, brushmodel_t& model, const lump_t& lump );
bool PBSPV1_LoadLeafs( const byte* pfile, brushmodel_t& model, const lump_t& lump );
bool PBSPV1_LoadNodes( const byte* pfile, brushmodel_t& model, const lump_t& lump );
bool PBSPV1_LoadClipnodes( const byte* pfile, brushmodel_t& model, const lump_t& lump );
bool PBSPV1_LoadEntities( const byte* pfile, brushmodel_t& model, const lump_t& lump );
bool PBSPV1_LoadSubmodels( const byte* pfile, brushmodel_t& model, const lump_t& lump );

//
// Functions for setting up other data
//
void PBSPV1_SetNodeParent( mnode_t* pnode, mnode_t* pparent );
void PBSPV1_MakeHullZero( brushmodel_t& model );
void PBSPV1_SetupPAS( brushmodel_t& model );
bool PBSPV1_CalcSurfaceExtents( msurface_t* psurf, brushmodel_t& model );

#endif //PBSPV1_H