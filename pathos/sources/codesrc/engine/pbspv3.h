/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef PBSPV3_H
#define PBSPV3_H

//
// Load functions
//
brushmodel_t* PBSPV3_Load( const byte* pfile, const dpbspv3header_t* pheader, const Char* pstrFilename );

//
// Functions for loading specific lumps
//
extern bool PBSPV3_LoadVertexes( const byte* pfile, brushmodel_t& model, const dpbspv3lump_t& lump );
extern bool PBSPV3_LoadEdges( const byte* pfile, brushmodel_t& model, const dpbspv3lump_t& lump );
extern bool PBSPV3_LoadSurfedges( const byte* pfile, brushmodel_t& model, const dpbspv3lump_t& lump );
extern bool PBSPV3_LoadTextures( const byte* pfile, brushmodel_t& model, const dpbspv3lump_t& lump );
extern bool PBSPV3_LoadDefaultLighting( const byte* pfile, brushmodel_t& model, const dpbspv3lump_t& lump );
extern bool PBSPV3_LoadLightingDataLayer( const byte* pfile, brushmodel_t& model, const dpbspv3lump_t& lump, surf_lmap_layers_t layer );
extern bool PBSPV3_LoadPlanes( const byte* pfile, brushmodel_t& model, const dpbspv3lump_t& lump );
extern bool PBSPV3_LoadTexinfo( const byte* pfile, brushmodel_t& model, const dpbspv3lump_t& lump );
extern bool PBSPV3_LoadFaces( const byte* pfile, brushmodel_t& model, const dpbspv3lump_t& lump );
extern bool PBSPV3_LoadMarksurfaces( const byte* pfile, brushmodel_t& model, const dpbspv3lump_t& lump );
extern bool PBSPV3_LoadVisibility( const byte* pfile, brushmodel_t& model, const dpbspv3lump_t& lump );
extern bool PBSPV3_LoadLeafs( const byte* pfile, brushmodel_t& model, const dpbspv3lump_t& lump );
extern bool PBSPV3_LoadNodes( const byte* pfile, brushmodel_t& model, const dpbspv3lump_t& lump );
extern bool PBSPV3_LoadClipnodes( const byte* pfile, brushmodel_t& model, const dpbspv3lump_t& lump );
extern bool PBSPV3_LoadEntities( const byte* pfile, brushmodel_t& model, const dpbspv3lump_t& lump );
extern bool PBSPV3_LoadSubmodels( const byte* pfile, brushmodel_t& model, const dpbspv3lump_t& lump );
extern bool PBSPV3_LoadVertexLighting(const byte* pfile, brushmodel_t& model, const dpbspv3lump_t& lump, surf_lmap_layers_t layer);
extern bool PBSPV3_DecompressLightingData( const byte* pfile, brushmodel_t& model, const dpbspv3lump_t& lump, color24_t*& pdestptr, Uint32& destsize, byte*& poriginaldataptr, Uint32& originalsize, Int32& compression, Int32 compressionlevel );
#endif //PBSPV3_H