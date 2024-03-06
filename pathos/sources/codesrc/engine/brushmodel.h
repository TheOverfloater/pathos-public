/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef BRUSHMODEL_H
#define BRUSHMODEL_H

#include "brushmodel_shared.h"

//
// Functions dealing with bsp data
//
extern const mleaf_t* Mod_PointInLeaf( const Vector& position, const brushmodel_t& model );
extern byte* Mod_DecompressVIS( byte* pbuffer, byte* pin, const brushmodel_t& model, Uint32 bytecount );
extern const byte* Mod_LeafPVS( byte* pbuffer, Uint32 bufsize, const mleaf_t& leaf, const brushmodel_t& model );
extern const byte* Mod_LeafPAS( byte* pbuffer, Uint32 bufsize, const mleaf_t& leaf, const brushmodel_t& model );
extern byte* Mod_CompressVIS( const byte* pin, Uint32* psize, const brushmodel_t& model, Int32 visbuffersize );
extern const msurface_t* Mod_SurfaceAtPoint( const brushmodel_t* pmodel, const mnode_t* pnode, const Vector& start, const Vector& end );
extern void Mod_FindTouchedLeafs( const brushmodel_t* pworld, CArray<Uint32>& leafnumsarray, const Vector& mins, const Vector& maxs, mnode_t* pnode );
extern bool Mod_RecursiveLightPoint( const brushmodel_t* pworld, mnode_t *pnode, const Vector &start, const Vector &end, Vector &color );
extern bool Mod_RecursiveLightPoint_BumpData( const brushmodel_t* pworld, mnode_t *pnode, const Vector &start, const Vector &end, Vector &ambientcolor, Vector& diffusecolor, Vector& lightdir );
#endif //BMODEL_H