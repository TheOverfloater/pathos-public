/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include <SDL.h>
#ifdef WIN32
#include <Windows.h>
#endif

#include "includes.h"
#include "compiler_types.h"

// Triangle allocation count
static const Uint32 TRIANGLE_ALLOCATION_COUNT = 256;
// Triangle vertex allocation count
static const Uint32 TRIVERTEX_ALLOCATION_COUNT = TRIANGLE_ALLOCATION_COUNT*3;
// Normal allocation count
static const Uint32 NORMAL_ALLOCATION_COUNT = 256;
// Weight info allocation count
static const Uint32 WEIGHT_ALLOCATION_COUNT = 256;

namespace smdl
{
	//===============================================
	// @brief Constructor for triangle_vertex_t
	//
	//===============================================
	triangle_vertex_t::triangle_vertex_t():
		vertexindex(NO_POSITION),
		normalindex(NO_POSITION), 
		flexindex(NO_POSITION),
		weightindex(NO_POSITION)
	{
		texcoords[0] = texcoords[1] = 0;
		int_texcoords[0] = int_texcoords[1] = 0;
	}

	//===============================================
	// @brief Constructor for vertex_t
	//
	//===============================================
	vertex_t::vertex_t():
		boneindex(NO_POSITION)
	{
	}

	//===============================================
	// @brief Constructor for normal_t
	//
	//===============================================
	normal_t::normal_t():
		skinref(NO_POSITION),
		boneindex(NO_POSITION)
	{
	}

	//===============================================
	// @brief Constructor for vertex_weightinfo_t
	//
	//===============================================
	vertex_weightinfo_t::vertex_weightinfo_t():
		numweights(0)
	{
		for(Uint32 i = 0; i < MAX_VBM_BONEWEIGHTS; i++)
		{
			boneindexes[i] = NO_POSITION;
			weights[i] = 0;
		}
	}

	//===============================================
	// @brief Constructor for bone_transforminfo_t
	//
	//===============================================
	bone_transforminfo_t::bone_transforminfo_t()
	{
		memset(matrix, 0, sizeof(matrix));
	}

	//===============================================
	// @brief Constructor for boneinfo_t
	//
	//===============================================
	boneinfo_t::boneinfo_t():
		index(NO_POSITION),
		parent_index(NO_POSITION),
		controller_index(NO_POSITION),
		flags(0),
		hitgroup(0),
		hitgroupset(false)
	{
	}

	//===============================================
	// @brief Constructor for hitbox_t
	//
	//===============================================
	hitbox_t::hitbox_t():
		boneindex(NO_POSITION),
		hitgroup(0)
	{
	}
	
	//===============================================
	// @brief Constructor for hitgroup_t
	//
	//==============================================
	hitgroup_t::hitgroup_t():
		hitgroup(0),
		script(false)
	{
	}

	//===============================================
	// @brief Constructor for bonecontroller_t
	//
	//===============================================
	bonecontroller_t::bonecontroller_t():
		boneindex(NO_POSITION),
		type(0),
		controllerindex(NO_POSITION),
		start_value(0),
		end_value(0)
	{
	}
	
	//===============================================
	// @brief Constructor for attachment_t
	//
	//===============================================
	attachment_t::attachment_t():
		attachment_index(NO_POSITION),
		bone_index(NO_POSITION)
	{
	}
	
	//===============================================
	// @brief Constructor for bone_node_t
	//
	//===============================================
	bone_node_t::bone_node_t():
		parentindex(NO_POSITION),
		ismirrored(false)
	{
	}
	
	//===============================================
	// @brief Constructor for bonematrix_t
	//
	//===============================================
	bonematrix_t::bonematrix_t()
	{
		memset(matrix, 0, sizeof(matrix));
	}
	
	//===============================================
	// @brief Constructor for anim_counts_t
	//
	//===============================================
	anim_counts_t::anim_counts_t()
	{
		for(Uint32 i = 0; i < 6; i++)
			counts[i] = 0;
	}

	//===============================================
	// @brief Constructor for anim_counts_t
	//
	//===============================================
	boneanim_t::boneanim_t()
	{
		for(Uint32 i = 0; i < 6; i++)
			pvalues[i] = nullptr;
	}

	//===============================================
	// @brief Constructor for animation_t
	//
	//===============================================
	animation_t::animation_t():
		startframe(0),
		endframe(0),
		flags(0)
	{}

	//===============================================
	// @brief Destructor for animation_t
	//
	//===============================================
	animation_t::~animation_t()
	{
		if(!pos_values.empty())
		{
			for(Uint32 i = 0; i < pos_values.size(); i++)
				delete pos_values[i];

			pos_values.clear();
		}

		if(!rot_values.empty())
		{
			for(Uint32 i = 0; i < rot_values.size(); i++)
				delete rot_values[i];

			rot_values.clear();
		}

		if(!animdata.empty())
		{
			for(Uint32 i = 0; i < animdata.size(); i++)
			{
				for(Uint32 j = 0; j < 6; j++)
					delete animdata[i].pvalues[j];
			}

			animdata.clear();
		}
	}

	//===============================================
	// @brief Constructor for sequence_t
	//
	//===============================================
	sequence_t::sequence_t():
		motiontype(0),
		flags(0),
		fps(0),
		numframes(0),
		activity(0),
		actweight(0),
		frameoffset(0),
		animindex(0),
		entrynode(0),
		exitnode(0),
		nodeflags(0),
		movementboneindex(0),
		scaleup(0),
		movementscale(0),
		zrotation(0),
		startframe(0),
		endframe(0)
	{
		for(Uint32 i = 0; i < 2; i++)
		{
			blendtype[i] = 0;
			blendstart[i] = 0;
			blendend[i] = 0;
		}
	}

	//===============================================
	// @brief Constructor for mesh_t
	//
	//===============================================
	mesh_t::mesh_t():
		numtrivertexes(0),
		skinref(NO_POSITION),
		numnorms(0)
	{
	}

	//===============================================
	// @brief Allocates space for three triangle_vertex_t elements
	// in the triangle array
	//
	//===============================================
	void mesh_t::addTriangleVertexes( triangle_vertex_t* ptriangles )
	{
		Uint32 allocsize = trivertexes.size();
		if((numtrivertexes + 3) >= trivertexes.size())
			trivertexes.resize(allocsize+TRIVERTEX_ALLOCATION_COUNT);

		for(Uint32 i = 0; i < 3; i++)
			trivertexes[numtrivertexes+i] = ptriangles[i];

		numtrivertexes += 3;
	}

	//===============================================
	// @brief Constructor for bone_t
	//
	//===============================================
	bone_t::bone_t():
		refcounter(0),
		globalindex(NO_POSITION)
	{
	}

	//===============================================
	// @brief Constructor for flexvertex_t
	//
	//===============================================
	flexvertex_t::flexvertex_t():
		vertexindex(NO_POSITION),
		animated(0),
		boneindex(NO_POSITION),
		smd_vertindex(NO_POSITION),
		smd_normindex(NO_POSITION),
		refcount(0)
	{
	}

	//===============================================
	// @brief Constructor for flexframe_t
	//
	//===============================================
	flexframe_t::flexframe_t():
		numvertexes(0)
	{
	}

	//===============================================
	// @brief Destructor for flexmodel_t
	//
	//===============================================
	flexmodel_t::~flexmodel_t()
	{
		if(!pflexes.empty())
		{
			for(Uint32 i = 0; i < pflexes.size(); i++)
				delete pflexes[i];

			pflexes.clear();
		}
	}

	//===============================================
	// @brief Constructor for lod_t
	//
	//===============================================
	lod_t::lod_t():
		lodtype(0),
		plodmodel(nullptr),
		distance(0),
		reverseTriangles(false),
		scale(0)
	{
	}

	//===============================================
	// @brief Destructor for lod_t
	//
	//===============================================
	lod_t::~lod_t()
	{
		if(plodmodel)
			delete plodmodel;
	}

	//===============================================
	// @brief Constructor for submodel_t
	//
	//===============================================
	submodel_t::submodel_t():
		numvertexes(0),
		numnormals(0),
		numweightinfos(0),
		pflexmodel(nullptr),
		scale(0),
		reverseTriangles(false)
	{}

	//===============================================
	// @brief Destructor for submodel_t
	//
	//===============================================
	submodel_t::~submodel_t()
	{
		if(!pmeshes.empty())
		{
			for(Uint32 i = 0; i < pmeshes.size(); i++)
				delete pmeshes[i];

			pmeshes.clear();
		}

		if(pflexmodel)
		{
			delete pflexmodel;
			pflexmodel = nullptr;
		}
	}

	//===============================================
	// @brief Get a mesh by the skinref value. Adds a
	// new mesh if not present.
	// @param skinref Skin reference index
	// @return Pointer to mesh with the skin reference value
	//===============================================
	mesh_t* submodel_t::getMesh( Int32 skinref )
	{
		for(Uint32 i = 0; i < pmeshes.size(); i++)
		{
			if(pmeshes[i]->skinref == skinref)
				return pmeshes[i];
		}

		Int32 insertIndex = pmeshes.size();
		pmeshes.resize(pmeshes.size()+1);
		pmeshes[insertIndex] = new mesh_t;
		pmeshes[insertIndex]->skinref = skinref;

		return pmeshes[insertIndex];
	}

	//===============================================
	// @brief Adds a new vertex to the array of vertexes,
	// or returns an index to an existing one that matches
	// this one.
	// @param vertex Vertex to add to the vertex array
	// @return Index of the vertex
	//===============================================
	Int32 submodel_t::addVertex( const vertex_t& vertex )
	{
		// Try and merge with an existing weight
		for(Uint32 i = 0; i < numvertexes; i++)
		{
			const vertex_t& curVertex = vertexes[i];
			if(curVertex.boneindex == vertex.boneindex && Math::VectorCompare(curVertex.position, vertex.position))
				return i;
		}

		// Resize if needed
		if(numvertexes >= vertexes.size())
			vertexes.resize(vertexes.size() + VERTEX_ALLOCATION_COUNT);

		Int32 vertexIndex = numvertexes;
		vertexes[vertexIndex] = vertex;
		numvertexes++;

		return vertexIndex;
	}

	//===============================================
	// @brief Adds a new normal to the array of normals,
	// or returns an index to an existing one that matches
	// this one.
	// @param normal Normal to add to the normal array
	// @param normalBlendTreshold Normal merging treshold
	// @return Index of the normal
	//===============================================
	Int32 submodel_t::addNormal( const normal_t& normal, Float normalBlendTreshold )
	{
		for(Uint32 i = 0; i < numnormals; i++)
		{
			const normal_t& curNormal = normals[i];

			// See if a close enough or matching normal exists
			if(normal.skinref == curNormal.skinref && normal.boneindex == curNormal.boneindex)
			{
				Float dp = Math::DotProduct(curNormal.normal, normal.normal);
				if(dp > normalBlendTreshold)
					return i;
			}
		}

		// Resize if needed
		if(numnormals >= normals.size())
			normals.resize(normals.size() + NORMAL_ALLOCATION_COUNT);

		Int32 normalIndex = numnormals;
		numnormals++;

		normals[normalIndex] = normal;
		return normalIndex;
	}

	//===============================================
	// @brief Adds a new weight info to the submodel's array,
	// or returns an index to an existing element that matches
	// this one.
	// @param pweights Pointer to array of weights
	// @param pboneindexes Pointer to array of bone indexes
	// @param numbones Number of bones
	// @param weightTreshold Weight merging/minimum value treshold
	// @return Index of the weight info
	//===============================================
	Int32 submodel_t::addWeightInfo( Float* pweights, Int32* pboneindexes, Int32 numbones, Float weightTreshold )
	{
		Int32 usedbones[MAX_VBM_BONEWEIGHTS] = { 0 };
		Float usedweights[MAX_VBM_BONEWEIGHTS] = { 0 };

		// Collect actually used bone weights
		Uint32 nbusedbones = 0;
		for(Uint32 i = 0; i < numbones; i++)
		{
			if(nbusedbones == MAX_VBM_BONEWEIGHTS)
				break;

			if(pweights[i] > weightTreshold)
			{
				usedweights[nbusedbones] = pweights[i];
				usedbones[nbusedbones] = pboneindexes[i];
				nbusedbones++;
			}
		}

		// Get combined weights
		Float fullweight = 0;
		for(Uint32 i = 0; i < nbusedbones; i++)
			fullweight += usedweights[i];

		// Re-normalize these values if needed
		if(fullweight != 1.0)
		{
			Float fraction = 1.0 / fullweight;
			for(Uint32 i = 0; i < nbusedbones; i++)
				usedweights[i] *= fraction;
		}

		// See if we already have this weight
		Uint32 i = 0;
		for(; i < numweightinfos; i++)
		{
			vertex_weightinfo_t& curWeight = weightinfos[i];
			if(curWeight.numweights != nbusedbones)
				continue;

			// Index both sides and match bones, as the mappings
			// are not guaranteed to be 1:1
			Uint32 j = 0;
			for(; j < curWeight.numweights; j++)
			{
				Uint32 k = 0;
				for(; k < nbusedbones; k++)
				{
					if(curWeight.boneindexes[j] == usedbones[k])
					{
						Float weight1 = curWeight.weights[j];
						Float weight2 = usedweights[k];
						Float diff = weight1 - weight2;
						if(SDL_fabs(diff) < weightTreshold)
							break;
					}
				}

				if(k == nbusedbones)
					break;
			}

			if(j == nbusedbones)
				return i;
		}

		if(numweightinfos >= weightinfos.size())
			weightinfos.resize(weightinfos.size() + WEIGHT_ALLOCATION_COUNT);

		Int32 weightIndex = numweightinfos;
		numweightinfos++;

		vertex_weightinfo_t& newWeight = weightinfos[weightIndex];
		newWeight.numweights = nbusedbones;

		for(Uint32 i = 0; i < nbusedbones; i++)
		{
			newWeight.boneindexes[i] = usedbones[i];
			newWeight.weights[i] = usedweights[i];
		}

		return weightIndex;
	}

	//===============================================
	// @brief Constructor for bodypart_t
	//
	//===============================================
	bodypart_t::bodypart_t():
		base(0)
	{
	}

	//===============================================
	// @brief Constructor for texture_t
	//
	//===============================================
	texture_t::texture_t():
		flags(0),
		width(0),
		height(0),
		filewidth(0),
		fileheight(0),
		ppalette(nullptr),
		ptexturedata(nullptr),
		parent(NO_POSITION),
		skinref(NO_POSITION)
	{
	}

	//===============================================
	// @brief Destructor for texture_t
	//
	//===============================================
	texture_t::~texture_t()
	{
		if(ppalette)
			delete[] ppalette;

		if(ptexturedata)
			delete[] ptexturedata;
	}

	//===============================================
	// @brief Constructor for rendermode_t
	//
	//===============================================
	rendermode_t::rendermode_t():
		texflags(0)
	{
	}

	//===============================================
	// @brief Constructor for rendermode_definition_t
	//
	//===============================================
	rendermode_definition_t::rendermode_definition_t():
		flag(0)
	{
	}

	//===============================================
	// @brief Constructor for rendermode_definition_t
	//
	//===============================================
	rendermode_definition_t::rendermode_definition_t( const Char* pstrName, Int32 flag ):
		rendermodename(pstrName),
		flag(flag)
	{
	}

	//===============================================
	// @brief Constructor for grouptexture_t
	//
	//===============================================
	grouptexture_t::grouptexture_t():
		skinref(NO_POSITION)
	{
	}

	//===============================================
	// @brief Constructor for submodelentry_t
	//
	//===============================================
	submodelentry_t::submodelentry_t():
		reverseTriangles(false),
		scale(0),
		movementScale(0)
	{
	}

	//===============================================
	// @brief Constructor for submodelentry_t
	//
	//===============================================
	hitgroup_bone_mapping_t::hitgroup_bone_mapping_t():
		hitgroupindex(NO_POSITION)
	{
	}
};

namespace vbm
{
	//===============================================
	// @brief Constructor for flexcontroller_vta_t
	//
	//===============================================
	flexcontroller_vta_t::flexcontroller_vta_t():
		flexindex(NO_POSITION)
	{
	}

	//===============================================
	// @brief Constructor for flexcontroller_t
	//
	//===============================================
	flexcontroller_t::flexcontroller_t():
		type(VBM_FLEX_SINE),
		minvalue(0),
		maxvalue(0)
	{
	}

	//===============================================
	// @brief Clears all structures used by the object
	//
	//===============================================
	void ref_frameinfo_t::clear( void )
	{
		if(!nodes.empty())
			nodes.clear();

		if(!bones.empty())
			bones.clear();

		if(!bonemap_inverse.empty())
			bonemap_inverse.clear();

		if(!bonetransforms.empty())
			bonetransforms.clear();
	}
};

