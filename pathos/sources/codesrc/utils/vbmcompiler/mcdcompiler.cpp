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
#include "studiocompiler.h"
#include "mcdcompiler.h"
#include "options.h"
#include "main.h"
#include "compiler_math.h"
#include "refframesmdparser.h"
#include "filefuncs.h"
#include "collisionsmdparser.h"

// Thanks to Jacco for the BVH article he made

// File buffer allocation size
const Uint32 CMCDCompiler::MCD_FILEBUFFER_ALLOC_SIZE = 1024*1024;

//===============================================
// @brief Constructor for CMCDCompiler class
//
// @param studioCompiler reference to studiomodel compiler object
//===============================================
CMCDCompiler::CMCDCompiler( CStudioModelCompiler& studioCompiler ):
	m_studioCompiler(studioCompiler),
	m_pSubModel(nullptr),
	m_pFileBuffer(nullptr),
	m_pMCDHeader(nullptr)
{
}

//===============================================
// @brief Destructor for CMCDCompiler class
//
//===============================================
CMCDCompiler::~CMCDCompiler( void )
{
	Clear();
}

//===============================================
// @brief Clears any data used by the class
//
//===============================================
void CMCDCompiler::Clear( void )
{
	if(!m_boneTransformInfoArray.empty())
		m_boneTransformInfoArray.clear();

	if(!m_pSubmodelsArray.empty())
	{
		for(Uint32 i = 0; i < m_pSubmodelsArray.size(); i++)
		{
			mcd::submodel_t* psubmodel = m_pSubmodelsArray[i];

			if(!psubmodel->pbvhnodes.empty())
			{
				for(Uint32 j = 0; j < psubmodel->pbvhnodes.size(); j++)
					delete psubmodel->pbvhnodes[j];

				psubmodel->pbvhnodes.clear();
			}
			
			delete psubmodel;
		}

		m_pSubmodelsArray.clear();
	}

	if(!m_pBodyPartsArray.empty())
	{
		for(Uint32 i = 0; i < m_pBodyPartsArray.size(); i++)
			delete m_pBodyPartsArray[i];

		m_pBodyPartsArray.clear();
	}

	if(!m_texturesArray.empty())
		m_texturesArray.clear();

	if(!m_bonesArray.empty())
		m_bonesArray.clear();

	m_pSubModel = nullptr;
	m_pMCDHeader = nullptr;

	if(m_pFileBuffer)
	{
		delete m_pFileBuffer;
		m_pFileBuffer = nullptr;
	}
}

//===============================================
// @brief Processes and writes the MCD file
//
//===============================================
bool CMCDCompiler::CreateMCDFile( void )
{
	// Build up the bodyparts array exactly like in studiocompiler
	Uint32 nbbodyparts = m_studioCompiler.GetNbBodyParts();
	m_pBodyPartsArray.resize(nbbodyparts);

	for(Uint32 i = 0; i < nbbodyparts; i++)
	{
		const smdl::bodypart_t* psrcbodypart = m_studioCompiler.GetBodyPart(i);
		mcd::bodypart_t* pdestbodypart = new mcd::bodypart_t();
		m_pBodyPartsArray[i] = pdestbodypart;

		pdestbodypart->base = psrcbodypart->base;
		pdestbodypart->name = psrcbodypart->name;

		// Reserve the amount we'll need
		pdestbodypart->psubmodels.resize(psrcbodypart->psubmodels.size());
		for(Uint32 j = 0; j < psrcbodypart->psubmodels.size(); j++)
		{
			smdl::submodel_t* psrcsubmodel = psrcbodypart->psubmodels[j];
			mcd::submodel_t* pdestsubmodel = new mcd::submodel_t();

			m_pSubmodelsArray.push_back(pdestsubmodel);
			pdestbodypart->psubmodels.push_back(pdestsubmodel);

			// If no collision mesh was supplied, then treat this as a null/empty collision mesh
			if(psrcsubmodel->collisionsmdname.empty())
				continue;

			pdestsubmodel->name = psrcsubmodel->collisionsmdname;
			pdestsubmodel->reversetriangles = !psrcsubmodel->reverseTriangles;

			// Parse the SMD in
			CCollisionSMDParser smdParser(m_studioCompiler, (*this), pdestsubmodel, pdestsubmodel->reversetriangles);
			if(!smdParser.ProcessFile(pdestsubmodel->name.c_str()))
				return false;
		}
	}

	// Set up bone transforms from the first frame of the first sequence
	Uint32 nbglobalbones = m_studioCompiler.GetNbBones();
	m_boneTransformInfoArray.resize(nbglobalbones);
	m_bonesArray.resize(nbglobalbones);

	const smdl::sequence_t* psequence = m_studioCompiler.GetSequence(0);
	const smdl::animation_t* panim = psequence->panims[0];
	for(Uint32 i = 0; i < panim->nodes.size(); i++)
	{
		smdl::bone_node_t& node = panim->nodes[i];
		if(panim->bonemap[i] == NO_POSITION)
			continue;

		Int32 boneindex = panim->bonemap[i];
		
		Int32 parentindex;
		if(node.parentindex != NO_POSITION)
			parentindex = panim->bonemap[node.parentindex];
		else
			parentindex = NO_POSITION;

		const Int32 firstFrameIndex = 0;
		const Vector& posvalue = (*panim->pos_values[firstFrameIndex])[i];
		const Vector& rotvalue = (*panim->rot_values[firstFrameIndex])[i];

		CompilerMath::SetupBoneTransform(boneindex, parentindex, posvalue, rotvalue, m_boneTransformInfoArray);

		// Save into bones array
		mcd::bone_t newBone;
		newBone.name = node.bonename;
		newBone.position = posvalue;
		newBone.rotation = rotvalue;
		newBone.parentindex = parentindex;

		m_bonesArray[boneindex] = newBone;
	}

	// Process each submodel separately
	for(Uint32 i = 0; i < m_pSubmodelsArray.size(); i++)
	{
		m_pSubModel = m_pSubmodelsArray[i];
		if(!m_pSubModel->nbtriangles)
			continue;

		// Initialize mins/maxs
		m_pSubModel->mins = NULL_MINS;
		m_pSubModel->maxs = NULL_MAXS;

		// Transform vertices used into their final positions
		for(Uint32 j = 0; j < m_pSubModel->vertexes.size(); j++)
		{
			mcd::vertex_t& vertex = m_pSubModel->vertexes[j];
			Int32 globalboneindex = m_pSubModel->bones[vertex.boneindex].globalindex;
			smdl::bone_transforminfo_t& boneTransform = m_boneTransformInfoArray[globalboneindex];

			Vector tmp = vertex.origin;
			Math::VectorTransform(tmp, boneTransform.matrix, vertex.origin);

			// Update mins/maxs
			for(Uint32 k = 0; k < 3; k++)
			{
				if(m_pSubModel->mins[k] > vertex.origin[k])
					m_pSubModel->mins[k] = vertex.origin[k];

				if(m_pSubModel->maxs[k] < vertex.origin[k])
					m_pSubModel->maxs[k] = vertex.origin[k];
			}
		}

		// Create collision data
		for(Uint32 j = 0; j < NB_MCD_COLLISION_TYPES; j++)
		{
			switch(j)
			{
			case MCD_COLLISION_BVH:
				{
					CreateSubmodelBVH();
					break;
				}
			case MCD_COLLISION_NULL:
			case MCD_COLLISION_TRIANGLES:
				{
					// Nothing to do here
					break;
				}
			default:
				{
					ErrorMsg("%s - Unhandled collison type '%d'.\n", __FUNCTION__, static_cast<Int32>(j));
					break;
				}
			}
		}
	}

	// Create output file
	m_pFileBuffer = new CBuffer(MCD_FILEBUFFER_ALLOC_SIZE);

	// Allocate and clear the data
	m_pMCDHeader = reinterpret_cast<mcdheader_t*>(m_pFileBuffer->getbufferdata());
	m_pFileBuffer->addpointer(reinterpret_cast<void**>(&m_pMCDHeader));
	m_pFileBuffer->append(nullptr, sizeof(mcdheader_t));

	// Set basic stuff
	m_pMCDHeader->id = MCD_FORMAT_HEADER;
	m_pMCDHeader->version = MCD_FORMAT_VERSION;

	Uint32 maxSize = sizeof(mcdheader_t::name);
	qstrcpy_s(m_pMCDHeader->name, g_options.outputname.c_str(), maxSize);

	// Save submodels
	Int32 submodeloffset = m_pFileBuffer->getsize();
	m_pFileBuffer->append(nullptr, m_pSubmodelsArray.size()*sizeof(mcdsubmodel_t));
	mcdsubmodel_t* psubmodels = reinterpret_cast<mcdsubmodel_t*>(reinterpret_cast<byte*>(m_pMCDHeader) + submodeloffset);
	m_pFileBuffer->addpointer(reinterpret_cast<void**>(&psubmodels));

	// Save bodyparts array
	m_pMCDHeader->bodypartoffset = m_pFileBuffer->getsize();
	m_pMCDHeader->numbodyparts = m_studioCompiler.GetNbBodyParts();
	m_pFileBuffer->append(nullptr, m_pMCDHeader->numbodyparts*sizeof(mcdbodypart_t));
	mcdbodypart_t* pbodyparts = reinterpret_cast<mcdbodypart_t*>(reinterpret_cast<byte*>(m_pMCDHeader) + m_pMCDHeader->bodypartoffset);
	m_pFileBuffer->addpointer(reinterpret_cast<void**>(&pbodyparts));

	// Set up the bodyparts
	for (Uint32 i = 0, j = 0; i < m_pMCDHeader->numbodyparts; i++)
	{
		const smdl::bodypart_t* psrcbodypart = m_studioCompiler.GetBodyPart(i);
		mcdbodypart_t* pdestbodypart = &pbodyparts[i];

		maxSize = sizeof(mcdbodypart_t::name);
		qstrcpy_s(pdestbodypart->name, psrcbodypart->name.c_str(), maxSize);

		pdestbodypart->base = psrcbodypart->base;
		pdestbodypart->numsubmodels = psrcbodypart->psubmodels.size();
		pdestbodypart->submodeloffset = reinterpret_cast<byte *>(&psubmodels[j]) - reinterpret_cast<byte*>(m_pMCDHeader);
		j += psrcbodypart->psubmodels.size();
	}

	// Set submodel data
	for(Uint32 i = 0; i < m_pSubmodelsArray.size(); i++)
	{
		m_pSubModel = m_pSubmodelsArray[i];
		if(!m_pSubModel->nbtriangles)
			continue;

		m_pSubModel = m_pSubmodelsArray[i];
		mcdsubmodel_t* pdestsubmodel = &psubmodels[i];
		m_pFileBuffer->addpointer(reinterpret_cast<void**>(&pdestsubmodel));

		Uint32 maxSize = sizeof(mcdsubmodel_t::name);
		qstrcpy_s(pdestsubmodel->name, m_pSubModel->name.c_str(), maxSize);
		pdestsubmodel->collisiontypesoffset = m_pFileBuffer->getsize();
		pdestsubmodel->numcollisiontypes = NB_MCD_COLLISION_TYPES;
		pdestsubmodel->mins = m_pSubModel->mins;
		pdestsubmodel->maxs = m_pSubModel->maxs;

		m_pFileBuffer->append(nullptr, pdestsubmodel->numcollisiontypes*sizeof(mcdcollisiontypemodel_t));

		// Create collision data
		for(Uint32 j = MCD_COLLISION_TRIANGLES; j < NB_MCD_COLLISION_TYPES; j++)
		{
			mcdcollisiontypemodel_t* pcollisiontypemodel = reinterpret_cast<mcdcollisiontypemodel_t*>(reinterpret_cast<byte*>(m_pMCDHeader) + pdestsubmodel->collisiontypesoffset) + j;
			pcollisiontypemodel->type = static_cast<mcdcollisiontype_t>(j);
			pcollisiontypemodel->dataoffset = m_pFileBuffer->getsize();

			m_pFileBuffer->addpointer(reinterpret_cast<void**>(&pcollisiontypemodel));

			switch(j)
			{
			case MCD_COLLISION_TRIANGLES:
				{
					// Append data for this type and get ptr
					m_pFileBuffer->append(nullptr, sizeof(mcdtrimeshtype_t));
					mcdtrimeshtype_t* ptrimesh = reinterpret_cast<mcdtrimeshtype_t*>(reinterpret_cast<byte*>(m_pMCDHeader) + pcollisiontypemodel->dataoffset);
					m_pFileBuffer->addpointer(reinterpret_cast<void**>(&ptrimesh));

					// Allocate triangles and fill data
					ptrimesh->triangleoffset = m_pFileBuffer->getsize();
					ptrimesh->numtriangles = m_pSubModel->triangles.size();
					m_pFileBuffer->append(nullptr, ptrimesh->numtriangles*sizeof(mcdtrimeshtriangle_t));
					mcdtrimeshtriangle_t* ptrimeshtriangles = reinterpret_cast<mcdtrimeshtriangle_t*>(reinterpret_cast<byte*>(m_pMCDHeader) + ptrimesh->triangleoffset);
					m_pFileBuffer->addpointer(reinterpret_cast<void**>(&ptrimeshtriangles));

					for(Uint32 k = 0; k < m_pSubModel->triangles.size(); k++)
					{
						// Calculate plane of triangle
						Vector a1, a2, sn;
						Vector dotvec;

						const Vector& v1 = m_pSubModel->vertexes[m_pSubModel->triangles[k].vertexes[0]].origin;
						const Vector& v2 = m_pSubModel->vertexes[m_pSubModel->triangles[k].vertexes[1]].origin;
						const Vector& v3 = m_pSubModel->vertexes[m_pSubModel->triangles[k].vertexes[2]].origin;

						Math::VectorSubtract(v2, v1, a1);
						Math::VectorSubtract(v3, v1, a2);
						Math::CrossProduct(a1, a2, sn);
						sn.Normalize();

						ptrimeshtriangles[k].normal = sn;
						ptrimeshtriangles[k].distance = Math::DotProduct(v2, sn);
						ptrimeshtriangles[k].skinref = m_pSubModel->triangles[k].skinref;

						Uint32 l = 0;
						for(; l < 3; l++)
						{
							if(sn[l] < 0)
								ptrimeshtriangles[k].signbits |= (1 << l);

							if(SDL_fabs(sn[l]) == 1.0)
							{
								ptrimeshtriangles[k].planetype = l;
								break;
							}
						}

						if(l == 3)
						{
							if(sn[0] >= sn[1] && sn[0] >= sn[2])
								ptrimeshtriangles[k].planetype = PLANE_AX;
							else if(sn[1] >= sn[0] && sn[1] >= sn[2])
								ptrimeshtriangles[k].planetype = PLANE_AY;
							else
								ptrimeshtriangles[k].planetype = PLANE_AZ;
						}

						for(Uint32 l = 0; l < 3; l++)
							ptrimeshtriangles[k].trivertexes[l] = m_pSubModel->triangles[k].vertexes[l];
					}

					// Allocate vertexes and fill data
					ptrimesh->vertexoffset = m_pFileBuffer->getsize();
					ptrimesh->numvertexes = m_pSubModel->vertexes.size();
					m_pFileBuffer->append(nullptr, ptrimesh->numvertexes*sizeof(mcdvertex_t));
					mcdvertex_t* ptrimeshvertexes = reinterpret_cast<mcdvertex_t*>(reinterpret_cast<byte*>(m_pMCDHeader) + ptrimesh->vertexoffset);
					m_pFileBuffer->addpointer(reinterpret_cast<void**>(&ptrimeshvertexes));

					for(Uint32 k = 0; k < m_pSubModel->vertexes.size(); k++)
					{
						ptrimeshvertexes[k].boneindex = m_pSubModel->vertexes[k].boneindex;
						ptrimeshvertexes[k].origin = m_pSubModel->vertexes[k].origin;
					}

					m_pFileBuffer->removepointer(reinterpret_cast<void**>(&ptrimesh));
					m_pFileBuffer->removepointer(reinterpret_cast<void**>(&ptrimeshtriangles));
					m_pFileBuffer->removepointer(reinterpret_cast<void**>(&ptrimeshvertexes));
				}
				break;
			case MCD_COLLISION_BVH:
				{
					// Append data for this type and get ptr
					m_pFileBuffer->append(nullptr, sizeof(mcdbvhtype_t));
					mcdbvhtype_t* pvbhdata = reinterpret_cast<mcdbvhtype_t*>(reinterpret_cast<byte*>(m_pMCDHeader) + pcollisiontypemodel->dataoffset);			
					m_pFileBuffer->addpointer(reinterpret_cast<void**>(&pvbhdata));

					// Allocate BVH nodes
					pvbhdata->bvhnodeoffset = m_pFileBuffer->getsize();
					pvbhdata->numbvhnodes = m_pSubModel->pbvhnodes.size();
					m_pFileBuffer->append(nullptr, sizeof(mcdbvhnode_t)*pvbhdata->numbvhnodes);

					mcdbvhnode_t* pbvhnodes = reinterpret_cast<mcdbvhnode_t*>(reinterpret_cast<byte*>(m_pMCDHeader) + pvbhdata->bvhnodeoffset);			
					m_pFileBuffer->addpointer(reinterpret_cast<void**>(&pbvhnodes));

					// Fill node data
					for(Uint32 k = 0; k < m_pSubModel->pbvhnodes.size(); k++)
					{
						mcd::bvhnode_t* psrcbvhnode = m_pSubModel->pbvhnodes[k];
						mcdbvhnode_t* pdestbvhnode = &pbvhnodes[k];
						m_pFileBuffer->addpointer(reinterpret_cast<void**>(&pdestbvhnode));

						pdestbvhnode->index = psrcbvhnode->index;
						pdestbvhnode->isleaf = psrcbvhnode->isleaf;
						pdestbvhnode->mins = psrcbvhnode->mins;
						pdestbvhnode->maxs = psrcbvhnode->maxs;

						for(Uint32 l = 0; l < 2; l++)
							pdestbvhnode->children[l] = psrcbvhnode->childindexes[l];

						// Write triangles if leaf node
						if(!psrcbvhnode->triindexesarray.empty())
						{
							pdestbvhnode->triindexoffset = m_pFileBuffer->getsize();
							pdestbvhnode->numtriangles = psrcbvhnode->triindexesarray.size();
							m_pFileBuffer->append(nullptr, sizeof(Int32)*pdestbvhnode->numtriangles);
							Int32* ptriindexes = reinterpret_cast<Int32*>(reinterpret_cast<byte*>(m_pMCDHeader) + pdestbvhnode->triindexoffset);		

							for(Uint32 l = 0; l < psrcbvhnode->triindexesarray.size(); l++)
								ptriindexes[l] = psrcbvhnode->triindexesarray[l];
						}

						// Clean up
						m_pFileBuffer->removepointer(reinterpret_cast<void**>(&pdestbvhnode));
					}

					// Clean up
					m_pFileBuffer->removepointer(reinterpret_cast<void**>(&pbvhnodes));
					m_pFileBuffer->removepointer(reinterpret_cast<void**>(&pvbhdata));
				}
				break;
			default:
				ErrorMsg("%s - Unhandled collison type '%d'.\n", __FUNCTION__, static_cast<Int32>(j));
				break;
			}

			// Clean up
			m_pFileBuffer->removepointer(reinterpret_cast<void**>(&pcollisiontypemodel));
		}

		// Clean up
		m_pFileBuffer->removepointer(reinterpret_cast<void**>(&pdestsubmodel));
	}

	// Set texture data
	m_pMCDHeader->textureoffset = m_pFileBuffer->getsize();
	m_pMCDHeader->numtextures = m_texturesArray.size();
	m_pFileBuffer->append(nullptr, sizeof(mcdtexture_t)*m_pMCDHeader->numtextures);

	mcdtexture_t* ptextures = reinterpret_cast<mcdtexture_t*>(reinterpret_cast<byte*>(m_pMCDHeader) + m_pMCDHeader->textureoffset);
	for(Uint32 i = 0; i < m_pMCDHeader->numtextures; i++)
	{
		Uint32 maxSize = sizeof(mcdtexture_t::name);
		qstrcpy_s(ptextures[i].name, m_texturesArray[i].c_str(), maxSize);
	}

	// Save bone data
	m_pMCDHeader->boneoffset = m_pFileBuffer->getsize();
	m_pMCDHeader->numbones = m_bonesArray.size();
	m_pFileBuffer->append(nullptr, sizeof(mcdbone_t)*m_pMCDHeader->numbones);

	mcdbone_t* pbones = reinterpret_cast<mcdbone_t*>(reinterpret_cast<byte*>(m_pMCDHeader) + m_pMCDHeader->boneoffset);
	for(Uint32 i = 0; i < m_pMCDHeader->numbones; i++)
	{
		mcd::bone_t& srcbone = m_bonesArray[i];
		mcdbone_t* pdestbone = &pbones[i];

		Uint32 maxSize = sizeof(mcdbone_t::name);
		qstrcpy_s(pdestbone->name, srcbone.name.c_str(), maxSize);
		pdestbone->parentindex = srcbone.parentindex;
		pdestbone->position = srcbone.position;
		pdestbone->rotation = srcbone.rotation;
	}

	// Set final size
	m_pMCDHeader->size = m_pFileBuffer->getsize();

	// Write the output
	bool result = WriteFile();

	// Clean up
	m_pFileBuffer->removepointer(reinterpret_cast<void**>(&m_pMCDHeader));
	m_pFileBuffer->removepointer(reinterpret_cast<void**>(&psubmodels));
	m_pFileBuffer->removepointer(reinterpret_cast<void**>(&pbodyparts));

	// Clear everything out
	Clear();

	return result;
}

//===============================================
// @brief Create bounding volume hierarchy
//
//===============================================
void CMCDCompiler::CreateSubmodelBVH( void )
{
	// Allocate bvh root node and assign all triangles to it
	mcd::bvhnode_t* pRootNode = new mcd::bvhnode_t();
	pRootNode->triindexesarray.resize(m_pSubModel->triangles.size());

	// Calculate centroids
	for(Uint32 i = 0; i < m_pSubModel->triangles.size(); i++)
	{
		mcd::triangle_t& triangle = m_pSubModel->triangles[i];
		Vector& v1 = m_pSubModel->vertexes[triangle.vertexes[0]].origin;
		Vector& v2 = m_pSubModel->vertexes[triangle.vertexes[1]].origin;
		Vector& v3 = m_pSubModel->vertexes[triangle.vertexes[2]].origin;

		triangle.centroid = (v1 + v2 + v3) * 0.3333f;
		pRootNode->triindexesarray[i] = i;
	}

	pRootNode->index = m_pSubModel->pbvhnodes.size();
	m_pSubModel->pbvhnodes.push_back(pRootNode);

	// Update bounds
	UpdateBVHNodeBounds(pRootNode);

	// Subdivide this node
	SubdivideBVHNode(pRootNode);
}

//===============================================
// @brief Updates bounds of a BVH node
//
//===============================================
void CMCDCompiler::UpdateBVHNodeBounds( mcd::bvhnode_t* pnode )
{
	// Reset to null
	pnode->mins = NULL_MINS;
	pnode->maxs = NULL_MAXS;

	// Get all tris to calculate bounding box of this node
	for(Uint32 i = 0; i < pnode->triindexesarray.size(); i++)
	{
		Int32 triangleindex = pnode->triindexesarray[i];
		mcd::triangle_t& triangle = m_pSubModel->triangles[triangleindex];

		for(Uint32 j = 0; j < 3; j++)
		{
			Vector& vertex = m_pSubModel->vertexes[triangle.vertexes[j]].origin;

			for(Uint32 k = 0; k < 3; k++)
			{
				if(pnode->mins[k] > vertex[k])
					pnode->mins[k] = vertex[k];

				if(pnode->maxs[k] < vertex[k])
					pnode->maxs[k] = vertex[k];
			}
		}
	}
}

//===============================================
// @brief Subdivides a BVH node
//
//===============================================
void CMCDCompiler::SubdivideBVHNode( mcd::bvhnode_t* pnode )
{
	// Find our longest axis
	Vector extents = (pnode->maxs - pnode->mins);
	
	Int32 j = 0;
	for(Uint32 i = 1; i < 3; i++)
	{
		if(extents[i] > extents[j])
			j = i;
	}

	// Assign triangles from parent to children based on split position
	Float splitPosition = pnode->mins[j] + extents[j] * 0.5;
	
	CArray<Int32> leftNodeTriangles;
	CArray<Int32> rightNodeTriangles;
	for(Uint32 i = 0; i < pnode->triindexesarray.size(); i++)
	{
		Int32 triangleIndex = pnode->triindexesarray[i];
		mcd::triangle_t& triangle = m_pSubModel->triangles[i];
		if(triangle.centroid[j] < splitPosition)
			leftNodeTriangles.push_back(triangleIndex);
		else
			rightNodeTriangles.push_back(triangleIndex);
	}

	if(leftNodeTriangles.empty() || rightNodeTriangles.empty())
	{
		// If left or right node is empty, we're in a leaf node
		pnode->isleaf = true;
		return;
	}

	// Create left child node
	Int32 leftChildIndex = m_pSubModel->pbvhnodes.size();
	pnode->childindexes[0] = leftChildIndex;

	mcd::bvhnode_t* pLeftChild = new mcd::bvhnode_t();
	pLeftChild->index = leftChildIndex;
	pLeftChild->triindexesarray = leftNodeTriangles;
	m_pSubModel->pbvhnodes.push_back(pLeftChild);

	UpdateBVHNodeBounds(pLeftChild);

	// Create right child node
	Int32 rightChildIndex = m_pSubModel->pbvhnodes.size();
	pnode->childindexes[1] = rightChildIndex;

	mcd::bvhnode_t* pRightChild = new mcd::bvhnode_t();
	pRightChild->index = rightChildIndex;
	pRightChild->triindexesarray = rightNodeTriangles;
	m_pSubModel->pbvhnodes.push_back(pRightChild);

	UpdateBVHNodeBounds(pRightChild);

	// Clear this node, as it's not a leaf node
	pnode->triindexesarray.clear();
	pnode->isleaf = false;

	// Recurse further down the children
	SubdivideBVHNode(pLeftChild);
	SubdivideBVHNode(pRightChild);
}

//===============================================
// @brief Returns a texture skinref for a texture name
//
//===============================================
Int32 CMCDCompiler::GetTextureIndex( const Char* pstrTextureName )
{
	for(Uint32 i = 0; i < m_texturesArray.size(); i++)
	{
		if(!qstrcicmp(m_texturesArray[i], pstrTextureName))
			return i;
	}

	Int32 returnIndex = m_texturesArray.size();
	m_texturesArray.push_back(pstrTextureName);
	return returnIndex;
}

//===============================================
// @brief Writes the final output
//
// @return TRUE on success, FAIL if an error occurred
//===============================================
bool CMCDCompiler::WriteFile( void )
{
	CString outputPath;
	if(g_options.outputname.find(0, ":") != CString::CSTRING_NO_POSITION)
		outputPath << g_options.outputname << ".mcd";
	else
		outputPath << g_options.basedirectory << PATH_SLASH_CHAR << g_options.outputname << ".mcd";

	outputPath = Common::CleanupPath(outputPath.c_str());

	const byte* pdata = reinterpret_cast<const byte*>(m_pFileBuffer->getbufferdata());
	if(!g_fileInterface.pfnWriteFile(pdata, m_pMCDHeader->size, outputPath.c_str(), false))
	{
		ErrorMsg("Failed to open %s for writing: %s.\n", outputPath.c_str());
		return false;
	}

	Msg("Wrote MCD file '%s', %.2f mbytes.\n", outputPath.c_str(), Common::BytesToMegaBytes(m_pMCDHeader->size));
	return true;
}