/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef COMPILER_TYPES_H
#define COMPILER_TYPES_H

#include <map>

#include "studio.h"
#include "constants.h"
#include "vbmformat.h"
#include "com_math.h"

#pragma warning( disable : 4244 )
#pragma warning( disable : 4237 )
#pragma warning( disable : 4305 )

// SMD version
static const Int32 SMD_VERSION = 1;
// VTA version
static const Int32 VTA_VERSION = 1;

// Vertex merge cutoff
static const Float VERTEX_ROUNDING_VALUE = 1000.0;
// Vertex allocation count
static const Uint32 VERTEX_ALLOCATION_COUNT = 256;

// Max possible bone count due to the use of byte for bone indexes
static const Uint32 MAX_TOTAL_BONES = 255;

typedef std::list<CString> CStringList_t;
typedef std::map<CString, Int32> CStringInt32Map_t;
typedef std::map<CString, CString> CStringMap_t;

// Types used by studiomdl
namespace smdl
{
	struct triangle_vertex_t
	{
		triangle_vertex_t();

		Int32 vertexindex;
		Int32 normalindex;
		Int32 flexindex;
		Int32 weightindex;
		Float texcoords[2];
		Float int_texcoords[2];
		Vector normal;
	};

	struct vertex_t
	{
		vertex_t();

		Int32 boneindex;
		Vector position;
		Vector pos_original;
	};

	struct normal_t
	{
		normal_t();

		Int32 skinref;
		Int32 boneindex;
		Vector normal;
		Vector normal_original;
	};

	struct vertex_weightinfo_t
	{
		vertex_weightinfo_t();

		Int32 boneindexes[MAX_VBM_BONEWEIGHTS];
		Float weights[MAX_VBM_BONEWEIGHTS];
		Uint32 numweights;
	};

	struct bone_transforminfo_t
	{
		bone_transforminfo_t();

		Float matrix[3][4];
	};

	struct boneinfo_t
	{
		boneinfo_t();

		CString name;
		Int32 index;
		Int32 parent_index;
		Int32 controller_index;
		Int32 flags;
		Vector position;
		Vector position_scale;
		Vector rotation;
		Vector rotation_scale;
		Int32 hitgroup;
		bool hitgroupset;
		Vector mins;
		Vector maxs;
	};

	struct hitbox_t
	{
		hitbox_t();

		CString bonename;
		Int32 boneindex;
		Int32 hitgroup;
		Vector mins;
		Vector maxs;
	};

	struct hitgroup_t
	{
		hitgroup_t();

		Int32 hitgroup;
		CString name;
		bool script;
	};

	struct bonecontroller_t
	{
		bonecontroller_t();

		CString bonename;
		Int32 boneindex;
		Int32 type;
		Int32 controllerindex;
		Float start_value;
		Float end_value;
	};

	struct attachment_t
	{
		attachment_t();

		CString bonename;
		Int32 attachment_index;
		Int32 bone_index;
		Vector origin;
	};

	struct bone_node_t
	{
		bone_node_t();

		CString bonename;
		Int32 parentindex;
		bool ismirrored;
	};

	struct bonematrix_t
	{
		bonematrix_t();

		Float matrix[3][4];
	};

	struct anim_counts_t
	{
		anim_counts_t();

		Uint32 counts[6];
	};

	struct boneanim_t
	{
		boneanim_t();

		mstudioanimvalue_t* pvalues[6];
	};

	struct animation_t
	{
		animation_t();
		~animation_t();

		CString name;
		Int32 startframe;
		Int32 endframe;
		Int32 flags;

		CArray<bone_node_t> nodes;
		CArray<Int32> bonemap;
		CArray<Int32> bonemap_inverse;

		CArray<CArray<Vector>*> pos_values;
		CArray<CArray<Vector>*> rot_values;

		CArray<anim_counts_t> numanims;
		CArray<boneanim_t> animdata;
	};

	struct animevent_t
	{
		animevent_t():
			eventid(0),
			frame(0)
		{}

		Int32 eventid;
		Int32 frame;
		CString params;
	};

	struct pivot_t
	{
		pivot_t():
			index(NO_POSITION),
			start(0),
			end(0)
		{}

		Int32 index;
		Vector origin;
		Int32 start;
		Int32 end;
	};

	struct sequence_t
	{
		sequence_t();

		Int32 motiontype;
		Vector linearmovement;

		CString name;
		Int32 flags;
		Float fps;
		Int32 numframes;
		Int32 activity;
		Int32 actweight;

		CArray<CString> smdfilenames;

		Int32 frameoffset;

		CArray<animevent_t> events;
		CArray<pivot_t> pivots;

		CArray<animation_t*> panims;

		Float blendtype[2];
		Float blendstart[2];
		Float blendend[2];

		CArray<Vector> automovepositions;
		CArray<Vector> automoveangles;

		Int32 animindex;

		Vector bboxmins;
		Vector bboxmaxs;

		Int32 entrynode;
		Int32 exitnode;
		Int32 nodeflags;
		Int32 movementboneindex;

		Float scaleup;
		Float movementscale;
		Vector adjust;
		Float zrotation;

		Int32 startframe;
		Int32 endframe;
	};

	struct mesh_t
	{
		mesh_t();
		void addTriangleVertexes( triangle_vertex_t* ptriangles );

		CArray<triangle_vertex_t> trivertexes;
		Uint32 numtrivertexes;

		Int32 skinref;
		Int32 numnorms;
	};

	struct bone_t
	{
		bone_t();

		Vector position;
		Vector rotation;

		Int32 refcounter;
		Int32 globalindex;
	};

	struct flexvertex_t
	{
		flexvertex_t();

		Int32 vertexindex;
		bool animated;
		Int32 boneindex;
		Int32 smd_vertindex;
		Int32 smd_normindex;
		Vector origin;
		Vector normal;
		Uint32 refcount;
	};

	struct flexframe_t
	{
		flexframe_t();

		CArray<flexvertex_t> vertexes;
		Uint32 numvertexes;
	};

	struct flexmodel_t
	{
		~flexmodel_t();

		CString name;
		CArray<flexframe_t*> pflexes;
	};

	struct submodel_t
	{
		submodel_t();
		~submodel_t();
		mesh_t* getMesh( Int32 skinref );
		Int32 addVertex( const vertex_t& vertex );
		Int32 addNormal( const normal_t& normal, Float normalBlendTreshold );
		Int32 addWeightInfo( Float* pweights, Int32* pboneindexes, Int32 numbones, Float weightTreshold );

		CString name;

		CArray<bone_node_t> nodes;
		CArray<bone_t> bones;
		CArray<Int32> boneimap;

		CArray<vertex_t> vertexes;
		Uint32 numvertexes;

		CArray<normal_t> normals;
		Uint32 numnormals;

		CArray<vertex_weightinfo_t> weightinfos;
		Uint32 numweightinfos;

		CArray<mesh_t*> pmeshes;
		CArray<struct lod_t*> plods;

		CString vtaname;
		flexmodel_t *pflexmodel;

		Float scale;
		bool reverseTriangles;
	};

	struct lod_t
	{
		lod_t();
		~lod_t();

		CString submodelname;
		CString lodfilename;

		Int32 lodtype;
		submodel_t* plodmodel;
		Float distance;

		bool reverseTriangles;
		Float scale;
	};

	struct bodypart_t
	{
		bodypart_t();

		CString name;
		Int32 base;

		// Pointer to submodel in the array of submodels
		// DO NOT free from this array
		CArray<submodel_t*> psubmodels;
	};

	struct texture_t
	{
		texture_t();
		~texture_t();

		CString name;
		Int32 flags;
		
		Uint32 width;
		Uint32 height;

		Uint32 filewidth;
		Uint32 fileheight;

		byte* ppalette;
		byte* ptexturedata;

		Int32 parent;
		Int32 skinref;
	};

	struct rendermode_t
	{
		rendermode_t();

		Int32 texflags;
		CString texturename;
	};

	struct rendermode_definition_t
	{
		rendermode_definition_t();
		rendermode_definition_t(const Char* pstrName, Int32 flag);

		CString rendermodename;
		Int32 flag;
	};

	struct grouptexture_t
	{
		grouptexture_t();

		CString name;
		Int32 skinref;
	};

	struct texturegroup_t
	{
		CString groupname;
		CArray<grouptexture_t> originals;
		CArray<CArray<grouptexture_t>> replacements;
	};

	struct submodelentry_t
	{
		submodelentry_t();

		CString submodelname;
		CString flexname;

		bool reverseTriangles;
		Float scale;
		Float movementScale;
	};

	struct hitgroup_bone_mapping_t
	{
		hitgroup_bone_mapping_t();

		CString partialname;
		Int32 hitgroupindex;
	};
};

// Types specific to vbm compiler
namespace vbm
{
	struct ref_frameinfo_t
	{
		void clear( void );

		CArray<smdl::bone_node_t> nodes;
		CArray<smdl::bone_t> bones;
		CArray<smdl::bone_transforminfo_t> bonetransforms;
		CArray<Int32> bonemap_inverse;
	};

	struct flexcontroller_vta_t
	{
		flexcontroller_vta_t();

		CString name;
		Int32 flexindex;
	};

	struct flexcontroller_t
	{
		flexcontroller_t();

		CString name;

		CArray<flexcontroller_vta_t> vtas;

		vbmflexinterp_t type;

		Float minvalue;
		Float maxvalue;
	};
};

#endif //COMPILER_TYPES_H