/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/

//
// studiomdl.c: generates a studio .mdl file from a .qc script
// models/<scriptname>.mdl.
//

#ifndef VBMCONVERT_H
#define VBMCONVERT_H

/************************
	Conversion helper structures

************************/
struct studiovert_t
{
	Int32 vertindex;
	Int32 normindex;
	Int32 texcoord[2];
	byte boneindex;
};

struct studiotri_t
{
	bool processed;
	studiovert_t verts[3];
};

struct mesh_group_t
{
	studiotri_t *triangles;
	Int32 numtris;

	byte bones[MAX_SHADER_BONES];
	Int32 numbones;

	const mstudiotexture_t *ptex;
	Int32 skinref;
	Int32 bonegrp;
};

extern void VBM_ConvertModel( const Char* pstrModelFilePath, const Char* pstrTextureModelFilePath, const Char* pstrOutputPath );
extern void VBM_TryMergeGroups( void );
extern void VBM_ProcessConvGroups( vbmsubmodel_t* pvbosubmodel );
extern void VBM_ProcessVertex( studiovert_t *pvert, mesh_group_t* pgroup );
extern void VBM_SplitMesh( mstudiomesh_t *pmesh, mstudiomodel_t *psub, const mstudiotexture_t *ptex );
extern void VBM_RecurseBone( Int32 index, mstudiobone_t *pbone );
extern void VBM_CalculateTangents( void );
#endif