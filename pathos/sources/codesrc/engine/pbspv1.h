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
extern bool PBSPV1_LoadVertexes( const byte* pfile, brushmodel_t& model, const dpbspv1lump_t& lump );
extern bool PBSPV1_LoadEdges( const byte* pfile, brushmodel_t& model, const dpbspv1lump_t& lump );
extern bool PBSPV1_LoadSurfedges( const byte* pfile, brushmodel_t& model, const dpbspv1lump_t& lump );
extern bool PBSPV1_LoadTextures( const byte* pfile, brushmodel_t& model, const dpbspv1lump_t& lump );
extern bool PBSPV1_LoadLighting( const byte* pfile, brushmodel_t& model, const dpbspv1lump_t& lump );
extern bool PBSPV1_LoadPlanes( const byte* pfile, brushmodel_t& model, const dpbspv1lump_t& lump );
extern bool PBSPV1_LoadTexinfo( const byte* pfile, brushmodel_t& model, const dpbspv1lump_t& lump );
extern bool PBSPV1_LoadFaces( const byte* pfile, brushmodel_t& model, const dpbspv1lump_t& lump );
extern bool PBSPV1_LoadMarksurfaces( const byte* pfile, brushmodel_t& model, const dpbspv1lump_t& lump );
extern bool PBSPV1_LoadVisibility( const byte* pfile, brushmodel_t& model, const dpbspv1lump_t& lump );
extern bool PBSPV1_LoadLeafs( const byte* pfile, brushmodel_t& model, const dpbspv1lump_t& lump );
extern bool PBSPV1_LoadNodes( const byte* pfile, brushmodel_t& model, const dpbspv1lump_t& lump );
extern bool PBSPV1_LoadClipnodes( const byte* pfile, brushmodel_t& model, const dpbspv1lump_t& lump );
extern bool PBSPV1_LoadEntities( const byte* pfile, brushmodel_t& model, const dpbspv1lump_t& lump );
extern bool PBSPV1_LoadSubmodels( const byte* pfile, brushmodel_t& model, const dpbspv1lump_t& lump );
#endif //PBSPV1_H