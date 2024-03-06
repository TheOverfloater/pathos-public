/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef CL_RESOURCE_H
#define CL_RESOURCE_H

extern bool CL_BeginFilesDownload( void );
extern bool CL_ReadResourceMessage( void );
extern bool CL_LoadResources( void );
extern void CL_ClearResources( void );
extern bool CL_CancelDownload( void );
#endif //CL_RESOURCE_H