/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef PBSPV2_H
#define PBSPV2_H

//
// Load functions
//
brushmodel_t* PBSPV2_Load( const byte* pfile, const dpbspv2header_t* pheader, const Char* pstrFilename );

//
// Functions for loading specific lumps
//
extern bool PBSPV2_LoadVertexes( const byte* pfile, brushmodel_t& model, const dpbspv2lump_t& lump );
extern bool PBSPV2_LoadEdges( const byte* pfile, brushmodel_t& model, const dpbspv2lump_t& lump );
extern bool PBSPV2_LoadSurfedges( const byte* pfile, brushmodel_t& model, const dpbspv2lump_t& lump );
extern bool PBSPV2_LoadTextures( const byte* pfile, brushmodel_t& model, const dpbspv2lump_t& lump );
extern bool PBSPV2_LoadDefaultLighting( const byte* pfile, brushmodel_t& model, const dpbspv2lump_t& lump );
extern bool PBSPV2_LoadLightingDataLayer( const byte* pfile, brushmodel_t& model, const dpbspv2lump_t& lump, surf_lmap_layers_t layer );
extern bool PBSPV2_LoadPlanes( const byte* pfile, brushmodel_t& model, const dpbspv2lump_t& lump );
extern bool PBSPV2_LoadTexinfo( const byte* pfile, brushmodel_t& model, const dpbspv2lump_t& lump );
extern bool PBSPV2_LoadFaces( const byte* pfile, brushmodel_t& model, const dpbspv2lump_t& lump );
extern bool PBSPV2_LoadMarksurfaces( const byte* pfile, brushmodel_t& model, const dpbspv2lump_t& lump );
extern bool PBSPV2_LoadVisibility( const byte* pfile, brushmodel_t& model, const dpbspv2lump_t& lump );
extern bool PBSPV2_LoadLeafs( const byte* pfile, brushmodel_t& model, const dpbspv2lump_t& lump );
extern bool PBSPV2_LoadNodes( const byte* pfile, brushmodel_t& model, const dpbspv2lump_t& lump );
extern bool PBSPV2_LoadClipnodes( const byte* pfile, brushmodel_t& model, const dpbspv2lump_t& lump );
extern bool PBSPV2_LoadEntities( const byte* pfile, brushmodel_t& model, const dpbspv2lump_t& lump );
extern bool PBSPV2_LoadSubmodels( const byte* pfile, brushmodel_t& model, const dpbspv2lump_t& lump );
extern bool PBSPV2_DecompressLightingData( const byte* pfile, brushmodel_t& model, const dpbspv2lump_t& lump, color24_t*& pdestptr, Uint32& destsize, byte*& poriginaldataptr, Uint32& originalsize, Int32& compression, Int32 compressionlevel );
#endif //PBSPV2_H