/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef VBMUTILS_H
#define VBMUTILS_H

extern Int32 VBM_FindSequenceByActivity( const cache_model_t* pmodel, Int32 activity );
extern Int32 VBM_FindSequenceWithHeaviestActivity( const cache_model_t* pmodel, Int32 activity );
extern void VBM_GetSequenceInfo( const cache_model_t* pmodel, Int32 sequence, Float* pframerate, Float* pgroundspeed );
extern Int32 VBM_GetSequenceFlags( const cache_model_t* pmodel, Int32 sequence );
extern Uint32 VBM_GetSequenceNumber( const cache_model_t* pmodel );
extern Float VBM_SetBlending( const cache_model_t* pmodel, Uint32 blendingindex, Int32 sequence, Float value, Float* pblending );
extern Uint32 VBM_GetNumSequenceBlenders( const cache_model_t* pmodel, Int32 sequence );
extern void VBM_GetEyePosition( const cache_model_t* pmodel, Vector& eyeposition );
extern void VBM_PrecacheSequenceResources( const cache_model_t* pmodel, const Char* pstrsequencename );
extern Int32 VBM_FindTransition( const cache_model_t* pmodel, Int32 endanim, Int32 startanim, Int32* pdirection );
extern void VBM_SetBodyGroup( const cache_model_t* pmodel, Int32 groupindex, Int32 value, Int64& body );
extern Int32 VBM_GetBodyGroupIndexByName( const cache_model_t* pmodel, const Char* pstrBodyGroupName );
extern Int32 VBM_GetSubmodelIndexByName( const cache_model_t* pmodel, Int32 bodyGroupIndex, const Char* pstrSubmodelName );
extern Int32 VBM_GetBodyGroupValue( const cache_model_t* pmodel, Int32 groupindex, Int64 body );
extern Uint32 VBM_GetNumSequenceFrames( const cache_model_t* pmodel, Int32 sequence );
extern bool VBM_GetSequenceBoundingBox( const cache_model_t* pmodel, Int32 sequence, Vector& mins, Vector& maxs );
#endif //VBMUTILS_H