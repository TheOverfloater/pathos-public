/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef ALD_H
#define ALD_H
enum daystage_t;
enum aldcompression_t;
struct cache_model_t;

extern bool ALD_Load( daystage_t stage, byte** pdestarrays );
extern void ALD_ExportLightmaps( aldcompression_t compressionType, daystage_t daystage );
extern void ALD_CopyAndExportLightmaps( const Char* psrcaldfilename, daystage_t srcstage, daystage_t dststage );
extern bool ALD_HasStageData( daystage_t stage );
extern void ALD_ClearGame( void );
extern void ALD_DeleteRestoreFile( void );
extern void ALD_InitGame( void );
extern bool ALD_FileExists( void );
extern CString ALD_GetFilePath( daystage_t daystage, cache_model_t* pworldcache );
#endif //ALD_H