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

#include <unordered_map>

#include "includes.h"
#include "studiocompiler.h"
#include "options.h"
#include "main.h"
#include "geometrysmdparser.h"
#include "animationsmdparser.h"
#include "compiler_math.h"
#include "vtaparser.h"
#include "constants.h"
#include "bmp.h"
#include "tga.h"
#include "trimeshbuilder.h"
#include "filefuncs.h"

// Default normal merge treshold
#define DEFAULT_NORMAL_MERGE_TRESHOLD SDL_cos(2.0 * (M_PI / 180.0f))
// Defines a texture render mode 
#define DEFINE_RENDERMODE(name, bitflag) smdl::rendermode_definition_t(#name, bitflag)

// Default allocation size for MDL file buffer
const Uint32 CStudioModelCompiler::DEFAULT_MDL_ALLOCATION_SIZE = 1024*1024;
// Default minimum weight treshold
const Float CStudioModelCompiler::MINIMUM_WEIGHT_TRESHOLD = 1.0 / 1000.0;
// Default texture gamma
const Float CStudioModelCompiler::DEFAULT_TEXTURE_GAMMA_VALUE = 1.8;
// Default FPS for sequences
const Float CStudioModelCompiler::DEFAULT_SEQUENCE_FPS = 30;
// Rendermode definitions file name
const Char CStudioModelCompiler::RENDERMODE_DEFINITIONS_FILENAME[] = "rendermode_defs.txt";
// Hitgroup definitions file
const Char CStudioModelCompiler::HITGROUP_DEFINITIONS_FILENAME[] = "hitgroup_defs.txt";
// Hitgroup->bone automapping file
const Char CStudioModelCompiler::BONE_HITGROUP_AUTOMAP_FILENAME[] = "bone_hitgroup_automap.txt";

// Default rendermode definitions
const smdl::rendermode_definition_t CStudioModelCompiler::DEFAULT_RENDERMODE_DEFINITIONS[] {
	DEFINE_RENDERMODE(flatshade, STUDIO_NF_FLATSHADE),
	DEFINE_RENDERMODE(chrome, STUDIO_NF_CHROME),
	DEFINE_RENDERMODE(additive, STUDIO_NF_ADDITIVE),
	DEFINE_RENDERMODE(alphatest, STUDIO_NF_ALPHATEST),
	DEFINE_RENDERMODE(masked, STUDIO_NF_ALPHATEST)
};

//===============================================
// @brief Constructor for CStudioModelCompiler class
//
//===============================================
CStudioModelCompiler::CStudioModelCompiler( void ):
	m_numSkinFamilies(0),
	m_numSkinRefs(0),
	m_pFileOutputBuffer(nullptr),
	m_pStudioHeader(nullptr),
	m_defaultScale(0),
	m_scaleUpValue(0),
	m_defaultZRotation(0),
	m_defaultMovementScale(0),
	m_movementScale(0),
	m_studioFlags(0),
	m_weightTreshold(0),
	m_normalMergeTreshold(0),
	m_textureGamma(0)
{
	Uint32 nbRenderModes = sizeof(DEFAULT_RENDERMODE_DEFINITIONS) / sizeof(smdl::rendermode_definition_t);
	for(Uint32 i = 0; i < nbRenderModes; i++)
		m_renderModeDefinitionsArray.push_back(DEFAULT_RENDERMODE_DEFINITIONS[i]);
}

//===============================================
// @brief Destructor for CStudioModelCompiler class
//
//===============================================
CStudioModelCompiler::~CStudioModelCompiler( void )
{
	Clear();
}

//===============================================
// @brief Processes the input data into finalized data
//
// @return TRUE if successful, FALSE if failed
//===============================================
bool CStudioModelCompiler::ProcessInputData( void )
{
	// Check for basic errors
	if(m_pBodyPartsArray.empty())
	{
		ErrorMsg("No bodygroups were defined.\n");
		return false;
	}
	
	if(m_pSubmodelsArray.empty())
	{
		ErrorMsg("No submodels were defined.\n");
		return false;
	}

	if(m_pSequencesArray.empty())
	{
		ErrorMsg("No sequences were defined.\n");
		return false;
	}

	// Now, load and process each submodel
	for(Uint32 i = 0; i < m_pSubmodelsArray.size(); i++)
	{
		smdl::submodel_t* psubmodel = m_pSubmodelsArray[i];

		// Do not process any submodels marked "blank"
		if(!qstrcicmp(psubmodel->name, "blank"))
			continue;

		// Set any adjustment values specific to submodel
		m_scaleUpValue = psubmodel->scale;

		// Parse the SMD file
		CGeometrySMDParser smdParser((*this), psubmodel);
		if(!smdParser.ProcessFile(psubmodel->name.c_str()))
			return false;

		// Only process VTAs if we're generating a VBM file
		if(!psubmodel->vtaname.empty() && !g_options.isFlagSet(CMP_FL_DISABLE_VBM_GENERATION))
		{
			// Parse VTA file
			CVTAParser vtaParser((*this), psubmodel, smdParser.GetBoneTransformInfoArray());
			if(!vtaParser.ProcessFile(psubmodel->vtaname.c_str()))
				return false;
		}
	}

	// Check for LODs as well
	for(Uint32 i = 0; i < m_pLODsArray.size(); i++)
	{
		smdl::lod_t* plod = m_pLODsArray[i];

		Uint32 j = 0;
		for(; j < m_pSubmodelsArray.size(); j++)
		{
			smdl::submodel_t* psubmodel = m_pSubmodelsArray[j];
			if(!qstrcicmp(psubmodel->name, plod->submodelname))
			{
				plod->plodmodel = new smdl::submodel_t();
				plod->plodmodel->name = plod->lodfilename;
				plod->plodmodel->reverseTriangles = plod->reverseTriangles;
				plod->plodmodel->scale = plod->scale;

				// Parse the SMD file
				CGeometrySMDParser smdParser((*this), plod->plodmodel);
				if(!smdParser.ProcessFile(plod->plodmodel->name.c_str()))
					return false;

				psubmodel->plods.push_back(plod);
				break;
			}
		}

		if(j == m_pSubmodelsArray.size())
		{
			ErrorMsg("Submodel '%s' not found for LOD '%s.'\n", plod->submodelname.c_str(), plod->lodfilename.c_str());
			return false;
		}
	}

	// Load all textures used and set up other stuff
	if(!SetupTextures())
	{
		ErrorMsg("Failed to load textures for model.\n");
		return false;
	}

	// Now, load and process each sequence
	for(Uint32 i = 0; i < m_pSequencesArray.size(); i++)
	{
		smdl::sequence_t* psequence = m_pSequencesArray[i];

		for(Uint32 j = 0; j < psequence->smdfilenames.size(); j++)
		{
			smdl::animation_t *pnewanim = new smdl::animation_t();
			m_pAnimationsArray.push_back(pnewanim);

			pnewanim->startframe = psequence->startframe;
			pnewanim->endframe = psequence->endframe;
			psequence->panims.push_back(pnewanim);

			m_adjustmentVector = psequence->adjust;
			m_scaleUpValue = psequence->scaleup;

			CAnimationSMDParser smdParser((*this), pnewanim, psequence->zrotation);
			if(!smdParser.ProcessFile(psequence->smdfilenames[j].c_str()))
				return false;

			// Check that we have actual frames in the animation
			smdl::animation_t* pbaseanim = psequence->panims[0];
			Uint32 baseFrameNb = pbaseanim->endframe - pbaseanim->startframe + 1;
			if(!baseFrameNb)
			{
				ErrorMsg("Sequence '%s' has no animation frames.\n", psequence->name.c_str());
				return false;
			}

			// Ensure consistency with animation frame counts
			if(j > 0)
			{
				if(pbaseanim->nodes.size() != pnewanim->nodes.size())
				{
					ErrorMsg("In sequence '%s', blend animation '%s' has different number of bones compared to base animation.\n", psequence->name.c_str(), psequence->smdfilenames[j].c_str());
					return false;
				}

				Uint32 blendFrameNb = pnewanim->endframe - pnewanim->startframe + 1;
				if(baseFrameNb != blendFrameNb)
				{
					ErrorMsg("In sequence '%s', blend animation '%s' has different number of frames compared to base animation.\n", psequence->name.c_str(), psequence->smdfilenames[j].c_str());
					return false;
				}

				for(Uint32 k = 0; k < pbaseanim->nodes.size(); k++)
				{
					smdl::bone_node_t& baseanimnode = pbaseanim->nodes[k];
					smdl::bone_node_t& animnode = pnewanim->nodes[k];
					if(qstrcicmp(baseanimnode.bonename, animnode.bonename))
					{
						ErrorMsg("In sequence '%s', blend animation '%s' has a different bone at index %d: Base anim has '%s', blend anim has '%s'.\n", psequence->name.c_str(), psequence->smdfilenames[j].c_str(), k, baseanimnode.bonename.c_str(), animnode.bonename.c_str());
						return false;
					}

					if(baseanimnode.parentindex != animnode.parentindex)
					{
						ErrorMsg("In sequence '%s', blend animation '%s' has a different parent for bone '%s' at index %d: Base anim has %d, blend anim has %d.\n", psequence->name.c_str(), psequence->smdfilenames[j].c_str(), baseanimnode.bonename.c_str(), k, baseanimnode.parentindex, animnode.parentindex);
						return false;
					}
				}		
			}
		}

		smdl::animation_t *pbaseanimation = psequence->panims[0];

		psequence->numframes = pbaseanimation->endframe - pbaseanimation->startframe + 1;
		psequence->startframe = pbaseanimation->startframe;
		psequence->endframe = pbaseanimation->endframe;
	}

	// Do checks on our animations
	CheckAnimations();
	// Extract motion from animation data
	ExtractSequenceMotion();
	// Create transition mappings
	CreateNodeTransitionMappings();
	// Mark any used bones in the meshes
	MarkUsedBones();
	// Rename any bones
	RenameBones();

	// Build the global bone table
	if(!CreateBoneTable())
	{
		ErrorMsg("Error while building bone table.\n");
		return false;
	}

	// Remap sequence bones to global table
	if(!RemapSequenceBones())
	{
		ErrorMsg("Error remapping sequence bones.\n");
		return false;
	}

	// Set final bone indexes on all submodel vertexes, normals,
	// flex vertexes and vertex weights
	SetFinalBoneIndexes();

	// Link bone controllers with their bones
	if(!LinkBoneControllers())
	{
		ErrorMsg("Error linking bone controllers to their bones.\n");
		return false;
	}

	// Link attachments to their bones
	if(!LinkAttachments())
	{
		ErrorMsg("Error linking bone attachments to their bones.\n");
		return false;
	}

	// Set hitgroups
	if(!SetupHitGroups())
	{
		ErrorMsg("Error when setting up hitgroups.\n");
		return false;
	}

	// Sets up Hitboxes
	if(!SetupHitBoxes())
	{
		ErrorMsg("Error when setting up hitboxes.\n");
		return false;
	}

	// Set final bone positions on animations
	SetFinalSequenceBonePositions();
	// Calculate bone scale values
	CalculateBoneScales();
	// Calculate bounding boxes
	CalculateSequenceBoundingBoxes();
	// Compress animation data
	CompressAnimationData();

	return true;
}

//===============================================
// @brief Writes MDL data to the output file
//
// @return TRUE if successful, FALSE if failed
//===============================================
bool CStudioModelCompiler::WriteMDLFile( void )
{
	m_pFileOutputBuffer = new CBuffer(DEFAULT_MDL_ALLOCATION_SIZE);

	// Get ptr to header data
	void* pBuffer = m_pFileOutputBuffer->getbufferdata();
	m_pStudioHeader = reinterpret_cast<studiohdr_t*>(pBuffer);
	m_pFileOutputBuffer->addpointer(reinterpret_cast<void**>(&m_pStudioHeader));
	
	// Mark offset in buffer
	m_pFileOutputBuffer->append(nullptr, sizeof(studiohdr_t));

	sprintf(m_pStudioHeader->name, "%s.mdl", g_options.outputname.c_str());
	m_pStudioHeader->id = IDSTUDIOHEADER;
	m_pStudioHeader->version = STUDIO_VERSION;
	m_pStudioHeader->flags = m_studioFlags;
	m_pStudioHeader->eyeposition = m_eyePosition;
	m_pStudioHeader->min = m_bBoxMins;
	m_pStudioHeader->max = m_bBoxMaxs;
	m_pStudioHeader->bbmin = m_cBoxMins;
	m_pStudioHeader->bbmax = m_cBoxMaxs;

	// Write all bone related data, like bones themselves,
	// controllers, hitboxes, attachments, etc
	if(!WriteBoneData())
	{
		ErrorMsg("Error while writing bone data.\n");
		return false;
	}

	// Write all sequence data, including animation values
	if(!WriteAnimationData())
	{
		ErrorMsg("Error while writing sequence data.\n");
		return false;
	}

	// Write all submodels and meshes
	if(!WriteGeometryData())
	{
		ErrorMsg("Error while writing geometry data.\n");
		return false;
	}

	// Lastly, write the texture data
	// Note: This NEEDS to be the LAST write to the MDL file
	// buffer, as both Pathos and Goldsrc rely on texturedataindex
	// in the studio header to set the final size of the studio data
	// buffer after loading a model.
	WriteTextures();

	// Set final size in the file
	m_pStudioHeader->length = m_pFileOutputBuffer->getsize();

	// Build output filepath
	CString outputPath;
	if(g_options.outputname.find(0, ":") != CString::CSTRING_NO_POSITION)
		outputPath << g_options.outputname << ".mdl";
	else
		outputPath << g_options.basedirectory << PATH_SLASH_CHAR << g_options.outputname << ".mdl";

	outputPath = Common::CleanupPath(outputPath.c_str());

	const byte* pdata = reinterpret_cast<const byte*>(m_pFileOutputBuffer->getbufferdata());
	if(!g_fileInterface.pfnWriteFile(pdata, m_pStudioHeader->length, outputPath.c_str(), false))
	{
		ErrorMsg("Failed to open %s for writing: %s.\n", outputPath.c_str());
		return false;
	}

	Msg("Wrote MDL file '%s', %.2f mbytes.\n", outputPath.c_str(), Common::BytesToMegaBytes(m_pStudioHeader->length));

	// Release the buffer
	m_pFileOutputBuffer->removepointer(reinterpret_cast<void**>(&m_pStudioHeader));

	delete m_pFileOutputBuffer;
	m_pFileOutputBuffer = nullptr;

	return true;
}
 
//===============================================
// @brief Writes all texture data
//
// Notes: This NEEDS to be called LAST before
// writing the output, as "texturedataindex" is
// used to resize the buffer in the engine to
// just before the texture data blocks.
//===============================================
void CStudioModelCompiler::WriteTextures( void )
{
	Uint32 prevLoad = m_pFileOutputBuffer->getsize();

	// Save skinrefs array
	Uint32 dataSize = m_numSkinFamilies * m_numSkinRefs * sizeof(Int16);
	m_pStudioHeader->skinindex = m_pFileOutputBuffer->getsize();
	m_pStudioHeader->numskinref = m_numSkinRefs;
	m_pStudioHeader->numskinfamilies = m_numSkinFamilies;
	m_pFileOutputBuffer->append(nullptr, dataSize);

	Int16* pdestskinrefs = reinterpret_cast<Int16*>(reinterpret_cast<byte*>(m_pStudioHeader) + m_pStudioHeader->skinindex);
	for(Uint32 i = 0; i < m_numSkinFamilies; i++)
	{
		for(Uint32 j = 0; j < m_numSkinRefs; j++)
		{
			Int32 index = m_numSkinRefs * i + j;
			pdestskinrefs[index] = m_skinRefsArray[i][j];
		}
	}

	// Save the textures themselves
	dataSize = m_pTexturesArray.size() * sizeof(mstudiotexture_t);
	m_pStudioHeader->numtextures = m_pTexturesArray.size();
	m_pStudioHeader->textureindex = m_pFileOutputBuffer->getsize();
	m_pFileOutputBuffer->append(nullptr, dataSize);

	// Mark texture data index, needs to be last
	m_pStudioHeader->texturedataindex = m_pFileOutputBuffer->getsize();

	mstudiotexture_t* pdesttextures = reinterpret_cast<mstudiotexture_t*>(reinterpret_cast<byte*>(m_pStudioHeader) + m_pStudioHeader->textureindex);
	m_pFileOutputBuffer->addpointer(reinterpret_cast<void**>(&pdesttextures));

	for(Uint32 i = 0; i < m_pTexturesArray.size(); i++)
	{
		const smdl::texture_t* psrctexture = m_pTexturesArray[i];
		mstudiotexture_t* pdesttexture = &pdesttextures[i];

		Uint32 maxSize = sizeof(mstudiotexture_t::name);
		qstrcpy_s(pdesttexture->name, psrctexture->name.c_str(), maxSize);

		pdesttexture->flags = psrctexture->flags;
		pdesttexture->height = psrctexture->height;
		pdesttexture->width = psrctexture->width;

		pdesttexture->index = m_pFileOutputBuffer->getsize();

		// Save palette and texture data
		dataSize = sizeof(byte) * psrctexture->width * psrctexture->height;
		m_pFileOutputBuffer->append(psrctexture->ptexturedata, dataSize);

		dataSize = sizeof(color24_t) * 256;
		m_pFileOutputBuffer->append(psrctexture->ppalette, dataSize);
	}

	m_pFileOutputBuffer->removepointer(reinterpret_cast<void**>(&pdesttextures));

	dataSize = m_pFileOutputBuffer->getsize() - prevLoad;
	Msg("Wrote %d textures, %d skin families\n\t- Size written: %.2f megabytes.\n", 
		m_pTexturesArray.size(), m_pStudioHeader->numskinfamilies, Common::BytesToMegaBytes(dataSize));
}

//===============================================
// @brief Writes all model geometry data
//
// @return TRUE if everything went well, FALSE on failure
//===============================================
bool CStudioModelCompiler::WriteGeometryData( void )
{
	Uint32 prevLoad = m_pFileOutputBuffer->getsize();

	// Allocate bodyparts
	Uint32 dataSize = m_pBodyPartsArray.size() * sizeof(mstudiobodyparts_t);
	m_pStudioHeader->bodypartindex = m_pFileOutputBuffer->getsize();
	m_pStudioHeader->numbodyparts = m_pBodyPartsArray.size();
	m_pFileOutputBuffer->append(nullptr, dataSize);

	Uint32 submodelsIndex = m_pFileOutputBuffer->getsize();
	dataSize = m_pSubmodelsArray.size() * sizeof(mstudiomodel_t);
	m_pFileOutputBuffer->append(nullptr, dataSize);

	// Get ptr to destination submodel data
	mstudiomodel_t* pdestsubmodels = reinterpret_cast<mstudiomodel_t*>(reinterpret_cast<byte*>(m_pStudioHeader) + submodelsIndex);
	m_pFileOutputBuffer->addpointer(reinterpret_cast<void**>(&pdestsubmodels));

	// Fill in bodypart data
	mstudiobodyparts_t* pdestbodyparts = reinterpret_cast<mstudiobodyparts_t*>(reinterpret_cast<byte*>(m_pStudioHeader) + m_pStudioHeader->bodypartindex);
	m_pFileOutputBuffer->addpointer(reinterpret_cast<void**>(&pdestbodyparts));

	// Set up bodypart info
	for(Uint32 i = 0, j = 0; i < m_pBodyPartsArray.size(); i++)
	{
		const smdl::bodypart_t* psrcbodypart = m_pBodyPartsArray[i];
		mstudiobodyparts_t* pdestbodypart = &pdestbodyparts[i];

		Uint32 maxSize = sizeof(pdestbodypart->name);
		qstrcpy_s(pdestbodypart->name, psrcbodypart->name.c_str(), maxSize);
		pdestbodypart->base = psrcbodypart->base;
		pdestbodypart->nummodels = psrcbodypart->psubmodels.size();
		pdestbodypart->modelindex = (reinterpret_cast<byte*>(&pdestsubmodels[j]) - reinterpret_cast<byte*>(m_pStudioHeader));

		j += psrcbodypart->psubmodels.size();
	}

	Uint32 totalMeshes = 0;
	Uint32 totalTriangles = 0;
	Uint32 totalVertexes = 0;
	Uint32 totalNormals = 0;
	for(Uint32 i = 0; i < m_pSubmodelsArray.size(); i++)
	{
		smdl::submodel_t* psrcsubmodel = m_pSubmodelsArray[i];
		mstudiomodel_t* pdestsubmodel = &pdestsubmodels[i];

		Uint32 maxSize = sizeof(pdestsubmodel->name);
		qstrcpy_s(pdestsubmodel->name, psrcsubmodel->name.c_str(), maxSize);

		// If "blank" or the strip model data option is set, 
		// just set the name and leave everything as zeroes
		if(!qstrcicmp(psrcsubmodel->name, "blank") 
			|| g_options.isFlagSet(CMP_FL_STRIP_STUDIO_TRI_DATA))
			continue;

		m_pFileOutputBuffer->addpointer(reinterpret_cast<void**>(&pdestsubmodel));

		// Save vertex data
		pdestsubmodel->vertinfoindex = m_pFileOutputBuffer->getsize();
		dataSize = psrcsubmodel->numvertexes * sizeof(byte);
		m_pFileOutputBuffer->append(nullptr, dataSize);

		pdestsubmodel->vertindex = m_pFileOutputBuffer->getsize();
		dataSize = psrcsubmodel->numvertexes * sizeof(Vector);
		m_pFileOutputBuffer->append(nullptr, dataSize);

		byte* pdestvertinfos = reinterpret_cast<byte*>(m_pStudioHeader) + pdestsubmodel->vertinfoindex;
		Vector* pdestvertexes = reinterpret_cast<Vector*>(reinterpret_cast<byte*>(m_pStudioHeader) + pdestsubmodel->vertindex);

		for(Uint32 j = 0; j < psrcsubmodel->numvertexes; j++)
		{
			smdl::vertex_t& vertex = psrcsubmodel->vertexes[j];
			pdestvertinfos[j] = vertex.boneindex;
			pdestvertexes[j] = vertex.position;
		}

		pdestsubmodel->numverts = psrcsubmodel->numvertexes;
		totalVertexes += psrcsubmodel->numvertexes;

		// Save normal data
		pdestsubmodel->norminfoindex = m_pFileOutputBuffer->getsize();
		dataSize = psrcsubmodel->numnormals * sizeof(byte);
		m_pFileOutputBuffer->append(nullptr, dataSize);

		pdestsubmodel->normindex = m_pFileOutputBuffer->getsize();
		dataSize = psrcsubmodel->numnormals * sizeof(Vector);
		m_pFileOutputBuffer->append(nullptr, dataSize);

		byte* pdestnorminfos = reinterpret_cast<byte*>(m_pStudioHeader) + pdestsubmodel->norminfoindex;
		Vector* pdestnormals = reinterpret_cast<Vector*>(reinterpret_cast<byte*>(m_pStudioHeader) + pdestsubmodel->normindex);

		for(Uint32 j = 0; j < psrcsubmodel->numnormals; j++)
		{
			smdl::normal_t& normal = psrcsubmodel->normals[j];
			pdestnorminfos[j] = normal.boneindex;
			pdestnormals[j] = normal.normal;
		}

		pdestsubmodel->numnorms = psrcsubmodel->numnormals;
		totalNormals += psrcsubmodel->numnormals;

		// Set mesh data
		pdestsubmodel->meshindex = m_pFileOutputBuffer->getsize();
		pdestsubmodel->nummesh = psrcsubmodel->pmeshes.size();
		dataSize = psrcsubmodel->pmeshes.size() * sizeof(mstudiomesh_t);
		m_pFileOutputBuffer->append(nullptr, dataSize);

		mstudiomesh_t* pdestmeshes = reinterpret_cast<mstudiomesh_t*>(reinterpret_cast<byte*>(m_pStudioHeader) + pdestsubmodel->meshindex);
		m_pFileOutputBuffer->addpointer(reinterpret_cast<void**>(&pdestmeshes));

		for(Uint32 j = 0; j < psrcsubmodel->pmeshes.size(); j++)
		{
			const smdl::mesh_t* psrcmesh = psrcsubmodel->pmeshes[j];
			mstudiomesh_t* pdestmesh = &pdestmeshes[j];

			pdestmesh->numnorms = psrcmesh->numnorms;
			pdestmesh->numtris = psrcmesh->trivertexes.size() / 3;
			totalTriangles += pdestmesh->numtris;

			if(g_options.isFlagSet(CMP_FL_STRIP_MDL_TEXTURES))
				pdestmesh->skinref = 0;
			else
				pdestmesh->skinref = psrcmesh->skinref;

			byte* ptricmds = nullptr;
			CTriangleMeshBuilder builder(psrcmesh);
			dataSize = builder.BuildTriangleMesh(ptricmds);

			pdestmesh->triindex = m_pFileOutputBuffer->getsize();
			m_pFileOutputBuffer->append(ptricmds, dataSize);
		}

		totalMeshes += psrcsubmodel->pmeshes.size();

		m_pFileOutputBuffer->removepointer(reinterpret_cast<void**>(&pdestsubmodel));
		m_pFileOutputBuffer->removepointer(reinterpret_cast<void**>(&pdestmeshes));
	}

	m_pFileOutputBuffer->removepointer(reinterpret_cast<void**>(&pdestsubmodels));
	m_pFileOutputBuffer->removepointer(reinterpret_cast<void**>(&pdestbodyparts));

	dataSize = m_pFileOutputBuffer->getsize() - prevLoad;
	Msg("Wrote %d bodyparts, %d submodels, %d meshes, %d triangles, %d vertexes and %d normals.\n\t- Size written: %.2f megabytes.\n", 
		m_pStudioHeader->numbodyparts, m_pSubmodelsArray.size(), totalMeshes, totalTriangles, totalVertexes, totalNormals, Common::BytesToMegaBytes(dataSize));

	return true;
}

//===============================================
// @brief Writes all data defining the basic bone setup of the model
// and related structures like controllers, attachments, hitboxes
//
// @return TRUE if everything went well, FALSE on failure
//===============================================
bool CStudioModelCompiler::WriteBoneData( void )
{
	Uint32 prevLoad = m_pFileOutputBuffer->getsize();

	// Mark bone data beginning in studio header
	m_pStudioHeader->boneindex = m_pFileOutputBuffer->getsize();
	m_pStudioHeader->numbones = m_pBoneTableArray.size();

	Uint32 dataSize = sizeof(mstudiobone_t)*m_pBoneTableArray.size();
	m_pFileOutputBuffer->append(nullptr, dataSize);

	mstudiobone_t* pdestbones = reinterpret_cast<mstudiobone_t*>(reinterpret_cast<byte*>(m_pStudioHeader) + m_pStudioHeader->boneindex);
	m_pFileOutputBuffer->addpointer(reinterpret_cast<void**>(&pdestbones));

	for(Uint32 i = 0; i < m_pBoneTableArray.size(); i++)
	{
		mstudiobone_t* pdestbone = &pdestbones[i];
		const smdl::boneinfo_t* psrcbone = m_pBoneTableArray[i];

		Uint32 nameMax = sizeof(pdestbone->name);
		qstrcpy_s(pdestbone->name, psrcbone->name.c_str(), nameMax);

		pdestbone->parent = psrcbone->parent_index;
		pdestbone->flags = psrcbone->flags;

		for(Uint32 j = 0; j < 3; j++)
			pdestbone->value[j] = psrcbone->position[j];

		for(Uint32 j = 0; j < 3; j++)
			pdestbone->value[3+j] = psrcbone->rotation[j];

		for(Uint32 j = 0; j < 3; j++)
			pdestbone->scale[j] = psrcbone->position_scale[j];

		for(Uint32 j = 0; j < 3; j++)
			pdestbone->scale[3+j] = psrcbone->rotation_scale[j];

		// Set bone controllers to default
		for(Uint32 j = 0; j < 6; j++)
			pdestbone->bonecontroller[j] = NO_POSITION;
	}

	// Add bone controllers
	m_pStudioHeader->bonecontrollerindex = m_pFileOutputBuffer->getsize();
	m_pStudioHeader->numbonecontrollers = m_boneControllersArray.size();

	dataSize = sizeof(mstudiobonecontroller_t)*m_boneControllersArray.size();
	m_pFileOutputBuffer->append(nullptr, dataSize);

	mstudiobonecontroller_t *pcontrollers = reinterpret_cast<mstudiobonecontroller_t*>(reinterpret_cast<byte*>(m_pStudioHeader) + m_pStudioHeader->bonecontrollerindex);
	m_pFileOutputBuffer->addpointer(reinterpret_cast<void**>(&pcontrollers));

	// Set controllers indexes
	for(Uint32 i = 0; i < m_boneControllersArray.size(); i++)
	{
		// Set index in bone
		const smdl::bonecontroller_t& controller = m_boneControllersArray[i];
		Int32 boneIndex = controller.boneindex;
		mstudiobone_t* pstudiobone = &pdestbones[boneIndex];

		Int32 controllerIndex = NO_POSITION;
		switch(controller.type & STUDIO_TYPES)
		{
		case STUDIO_X:
			controllerIndex = 0;
			break;
		case STUDIO_Y:
			controllerIndex = 1;
			break;
		case STUDIO_Z:
			controllerIndex = 2;
			break;
		case STUDIO_XR:
			controllerIndex = 3;
			break;
		case STUDIO_YR:
			controllerIndex = 4;
			break;
		case STUDIO_ZR:
			controllerIndex = 5;
			break;
		}

		if(controllerIndex == NO_POSITION)
		{
			m_pFileOutputBuffer->removepointer(reinterpret_cast<void**>(&pdestbones));
			m_pFileOutputBuffer->removepointer(reinterpret_cast<void**>(&pcontrollers));
			ErrorMsg("Uknown bone controller type for bone '%s'.\n", pstudiobone->name);
			return false;
		}

		pstudiobone->bonecontroller[controllerIndex] = i;

		// Set bone controller info
		mstudiobonecontroller_t* pdestcontroller = &pcontrollers[i];
		pdestcontroller->bone = controller.boneindex;
		pdestcontroller->index = controller.controllerindex;
		pdestcontroller->type = controller.type;
		pdestcontroller->start = controller.start_value;
		pdestcontroller->end = controller.end_value;
	}

	// Save attachments
	m_pStudioHeader->attachmentindex = m_pFileOutputBuffer->getsize();
	m_pStudioHeader->numattachments = m_attachmentsArray.size();

	dataSize = sizeof(mstudioattachment_t)*m_attachmentsArray.size();
	m_pFileOutputBuffer->append(nullptr, dataSize);

	mstudioattachment_t *pattachments = reinterpret_cast<mstudioattachment_t*>(reinterpret_cast<byte*>(m_pStudioHeader) + m_pStudioHeader->attachmentindex);
	m_pFileOutputBuffer->addpointer(reinterpret_cast<void**>(&pattachments));

	for(Uint32 i = 0; i < m_attachmentsArray.size(); i++)
	{
		smdl::attachment_t& srcattachment = m_attachmentsArray[i];
		mstudioattachment_t* pdestattachment = &pattachments[i];

		pdestattachment->bone = srcattachment.bone_index;
		pdestattachment->org = srcattachment.origin;
	}

	// Save hitbox data
	m_pStudioHeader->hitboxindex = m_pFileOutputBuffer->getsize();
	m_pStudioHeader->numhitboxes = m_hitBoxesArray.size();

	dataSize = sizeof(mstudiobbox_t)*m_hitBoxesArray.size();
	m_pFileOutputBuffer->append(nullptr, dataSize);

	mstudiobbox_t *phitboxes = reinterpret_cast<mstudiobbox_t*>(reinterpret_cast<byte*>(m_pStudioHeader) + m_pStudioHeader->hitboxindex);
	m_pFileOutputBuffer->addpointer(reinterpret_cast<void**>(&phitboxes));

	for(Uint32 i = 0; i < m_hitBoxesArray.size(); i++)
	{
		const smdl::hitbox_t& hitbox = m_hitBoxesArray[i];
		mstudiobbox_t* pdesthitbox = &phitboxes[i];

		pdesthitbox->bone = hitbox.boneindex;
		pdesthitbox->group = hitbox.hitgroup;
		pdesthitbox->bbmin = hitbox.mins;
		pdesthitbox->bbmax = hitbox.maxs;
	}

	dataSize = m_pFileOutputBuffer->getsize() - prevLoad;
	Msg("Wrote %d bones, %d controllers, %d attachments, %d hit boxes.\n\t- Size written: %.2f megabytes.\n", 
		m_pStudioHeader->numbones, m_pStudioHeader->numbonecontrollers, m_pStudioHeader->numattachments, 
		m_pStudioHeader->numhitboxes, Common::BytesToMegaBytes(dataSize));

	// Clear out list of pointers
	m_pFileOutputBuffer->removepointer(reinterpret_cast<void**>(&pdestbones));
	m_pFileOutputBuffer->removepointer(reinterpret_cast<void**>(&pcontrollers));
	m_pFileOutputBuffer->removepointer(reinterpret_cast<void**>(&pattachments));
	m_pFileOutputBuffer->removepointer(reinterpret_cast<void**>(&phitboxes));

	return true;
}

//===============================================
// @brief Writes all animation data
//
// @return TRUE if everything went well, FALSE on failure
//===============================================
bool CStudioModelCompiler::WriteAnimationData( void )
{
	// attachments, controllers, hitboxes, etc
	Uint32 startBufferLoad = m_pFileOutputBuffer->getsize();

	mstudioanim_t* panimdata = nullptr;
	m_pFileOutputBuffer->addpointer(reinterpret_cast<void**>(&panimdata));

	mstudioanimvalue_t* panimvalue = nullptr;
	m_pFileOutputBuffer->addpointer(reinterpret_cast<void**>(&panimvalue));

	// Now save the data
	// Note: Fuck this
	for(Uint32 i = 0; i < m_pSequencesArray.size(); i++)
	{
		smdl::sequence_t* psequence = m_pSequencesArray[i];
		
		// Check if buffer needs to be expanded
		Uint32 dataSize = psequence->panims.size() * m_pBoneTableArray.size() * sizeof(mstudioanim_t);

		// Set animation data pointer
		psequence->animindex = m_pFileOutputBuffer->getsize();
		panimdata = reinterpret_cast<mstudioanim_t*>(reinterpret_cast<byte*>(m_pStudioHeader) + psequence->animindex);
		m_pFileOutputBuffer->append(nullptr, dataSize);

		panimvalue = reinterpret_cast<mstudioanimvalue_t*>(reinterpret_cast<byte*>(m_pStudioHeader) + psequence->animindex + dataSize);

		for(Uint32 j = 0; j < psequence->panims.size(); j++)
		{
			smdl::animation_t* panimation = psequence->panims[j];

			for(Uint32 k = 0; k < m_pBoneTableArray.size(); k++)
			{
				Int32 boneIndex = k; // Kept for use in debugging
				for(Uint32 l = 0; l < 6; l++)
				{
					if(panimation->numanims[boneIndex].counts[l] == 0)
					{
						panimdata->offset[l] = 0;
					}
					else
					{
						Uint32 dataSize = panimation->numanims[boneIndex].counts[l] * sizeof(mstudioanimvalue_t);
						m_pFileOutputBuffer->append(nullptr, dataSize);

						Uint32 offset = reinterpret_cast<byte*>(panimvalue) - reinterpret_cast<byte*>(panimdata);
						panimdata->offset[l] = offset;

						for(Uint32 m = 0; m < panimation->numanims[boneIndex].counts[l]; m++)
						{
							panimvalue->value = panimation->animdata[boneIndex].pvalues[l][m].value;
							panimvalue++;
						}
					}
				}

				Uint32 animationSize = reinterpret_cast<byte*>(panimvalue) - reinterpret_cast<byte*>(panimdata);
				if(animationSize > 65535)
				{
					ErrorMsg("Sequence '%s' animation '%s' exceeds 65 kbytes.\n", psequence->name.c_str(), panimation->name.c_str());
					return false;
				}

				panimdata++;
			}
		}
	}

	Uint32 animationDataSize = m_pFileOutputBuffer->getsize() - startBufferLoad;
	Uint32 eventCount = 0;
	Uint32 pivotCount = 0;

	// Mark sequence entry data beginning in studio header
	m_pStudioHeader->seqindex = m_pFileOutputBuffer->getsize();
	m_pStudioHeader->numseq = m_pSequencesArray.size();

	Uint32 dataSize = sizeof(mstudioseqdesc_t)*m_pSequencesArray.size();
	m_pFileOutputBuffer->append(nullptr, dataSize);

	mstudioseqdesc_t* pdestsequences = reinterpret_cast<mstudioseqdesc_t*>(reinterpret_cast<byte*>(m_pStudioHeader) + m_pStudioHeader->seqindex);
	m_pFileOutputBuffer->addpointer(reinterpret_cast<void**>(&pdestsequences));

	for(Uint32 i = 0; i < m_pSequencesArray.size(); i++)
	{
		smdl::sequence_t* psrcsequence = m_pSequencesArray[i];
		mstudioseqdesc_t* pdestsequence = &pdestsequences[i];

		Uint32 nameMax = sizeof(pdestsequence->label);
		qstrcpy_s(pdestsequence->label, psrcsequence->name.c_str(), nameMax);

		pdestsequence->activity = psrcsequence->activity;
		pdestsequence->actweight = psrcsequence->actweight;
		pdestsequence->animindex = psrcsequence->animindex;
		pdestsequence->bbmin = psrcsequence->bboxmins;
		pdestsequence->bbmax = psrcsequence->bboxmaxs;

		for(Uint32 j = 0; j < 2; j++)
			pdestsequence->blendstart[j] = psrcsequence->blendstart[j];

		for(Uint32 j = 0; j < 2; j++)
			pdestsequence->blendend[j] = psrcsequence->blendend[j];

		for(Uint32 j = 0; j < 2; j++)
			pdestsequence->blendtype[j] = psrcsequence->blendtype[j];

		pdestsequence->motionbone = psrcsequence->movementboneindex;
		pdestsequence->motiontype = psrcsequence->motiontype;
		pdestsequence->entrynode = psrcsequence->entrynode;
		pdestsequence->exitnode = psrcsequence->exitnode;
		pdestsequence->flags = psrcsequence->flags;
		pdestsequence->fps = psrcsequence->fps;
		pdestsequence->numframes = psrcsequence->numframes;
		pdestsequence->numblends = psrcsequence->panims.size();
		pdestsequence->nodeflags = psrcsequence->nodeflags;
		pdestsequence->seqgroup = 0;

		Math::VectorScale(psrcsequence->linearmovement, m_movementScale, pdestsequence->linearmovement);

		// Add events
		if(!psrcsequence->events.empty())
		{
			pdestsequence->eventindex = m_pFileOutputBuffer->getsize();
			pdestsequence->numevents = psrcsequence->events.size();
			dataSize = sizeof(mstudioevent_t)*psrcsequence->events.size();
			m_pFileOutputBuffer->append(nullptr, dataSize);

			mstudioevent_t* pdestevents = reinterpret_cast<mstudioevent_t*>(reinterpret_cast<byte*>(m_pStudioHeader) + pdestsequence->eventindex);
			for(Uint32 j = 0; j < psrcsequence->events.size(); j++)
			{
				smdl::animevent_t& srcevent = psrcsequence->events[j];
				mstudioevent_t* pdestevent = &pdestevents[j];

				pdestevent->event = srcevent.eventid;
				pdestevent->frame = srcevent.frame;

				Uint32 optionMax = sizeof(pdestevent->options);
				qstrcpy_s(pdestevent->options, srcevent.params.c_str(), optionMax);
			}

			eventCount += psrcsequence->events.size();
		}

		// Add pivots
		if(!psrcsequence->pivots.empty())
		{
			pdestsequence->pivotindex = m_pFileOutputBuffer->getsize();
			pdestsequence->numpivots = psrcsequence->pivots.size();
			dataSize = sizeof(mstudiopivot_t)*psrcsequence->pivots.size();
			m_pFileOutputBuffer->append(nullptr, dataSize);

			mstudiopivot_t* pdestpivots = reinterpret_cast<mstudiopivot_t*>(reinterpret_cast<byte*>(m_pStudioHeader) + pdestsequence->pivotindex);
			for(Uint32 j = 0; j < psrcsequence->pivots.size(); j++)
			{
				smdl::pivot_t& srcpivot = psrcsequence->pivots[j];
				mstudiopivot_t* pdestpivot = &pdestpivots[j];

				pdestpivot->org = srcpivot.origin;
				pdestpivot->start = srcpivot.start - psrcsequence->frameoffset;
				pdestpivot->end = srcpivot.end - psrcsequence->frameoffset;
			}

			pivotCount += psrcsequence->pivots.size();
		}
	}

	// Add a dummy sequence group entry. We don't use this crap in Pathos, but
	// Half-Life/GoldSrc still needs this structure present in the studiohdr
	// to not shit the bed(I think so anyway, didn't double-check).
	m_pStudioHeader->seqgroupindex = m_pFileOutputBuffer->getsize();
	m_pStudioHeader->numseqgroups = 1;

	dataSize = sizeof(mstudioseqgroup_t)*m_pStudioHeader->numseqgroups;
	m_pFileOutputBuffer->append(nullptr, dataSize);

	mstudioseqgroup_t* pdefaultgroup = reinterpret_cast<mstudioseqgroup_t*>(reinterpret_cast<byte*>(m_pStudioHeader) + m_pStudioHeader->seqgroupindex);
	qstrcpy(pdefaultgroup->label, "default");

	// Save transitions(What are these again exactly? Should figure it out.)
	if(!m_transitionNodesArray.empty())
	{
		m_pStudioHeader->transitionindex = m_pFileOutputBuffer->getsize();
		m_pStudioHeader->numtransitions = m_transitionNodesArray.size();

		dataSize = sizeof(byte) * m_pStudioHeader->numtransitions * m_pStudioHeader->numtransitions;
		m_pFileOutputBuffer->append(nullptr, dataSize);

		byte* pdesttransitiondata = reinterpret_cast<byte*>(m_pStudioHeader) + m_pStudioHeader->transitionindex;
		for(Uint32 i = 0; i < m_transitionNodesArray.size(); i++)
		{
			for(Uint32 j = 0; j < m_transitionNodesArray.size(); j++)
			{
				(*pdesttransitiondata) = m_transitionNodesArray[i][j];
				pdesttransitiondata++;
			}
		}
	}

	dataSize = m_pFileOutputBuffer->getsize() - startBufferLoad;
	Msg("Wrote %d sequences, %d events, %d pivots, %d bytes of compressed animation data.\n\t- Size written: %.2f megabytes.\n", 
		m_pSequencesArray.size(), eventCount, pivotCount, animationDataSize, Common::BytesToMegaBytes(dataSize));

	// Remove the pointers we allocated
	m_pFileOutputBuffer->removepointer(reinterpret_cast<void**>(&panimdata));
	m_pFileOutputBuffer->removepointer(reinterpret_cast<void**>(&panimvalue));
	m_pFileOutputBuffer->removepointer(reinterpret_cast<void**>(&pdestsequences));

	return true;
}

//===============================================
// @brief Extract motion from sequences, based on the movement bone specified
//
//===============================================
void CStudioModelCompiler::ExtractSequenceMotion( void )
{
	for(Uint32 i = 0; i < m_pSequencesArray.size(); i++)
	{
		smdl::sequence_t* psequence = m_pSequencesArray[i];

		if(!(psequence->motiontype & (STUDIO_LX|STUDIO_LY|STUDIO_LZ)))
			continue;

		smdl::animation_t* pbaseanimation = psequence->panims[0];

		if(psequence->numframes <= 1)
		{
			// Just clear any linear movement
			psequence->linearmovement.Clear();
			continue;
		}

		if(!m_movementBoneName.empty())
		{
			Uint32 j = 0;
			for(; j < pbaseanimation->nodes.size(); j++)
			{
				smdl::bone_node_t& boneNode = pbaseanimation->nodes[j];
				if(!qstrcicmp(boneNode.bonename, m_movementBoneName))
				{
					psequence->movementboneindex = j;
					break;
				}
			}
			
			if(j == pbaseanimation->nodes.size())
				WarningMsg("Couldn't find movement bone '%s' in sequence '%s'.\n", m_movementBoneName.c_str(), psequence->name.c_str());
		}
		else
			psequence->movementboneindex = 0;

		Int32 lastFrameIndex = psequence->numframes - 1;
		Vector& boneFirstPosition = (*pbaseanimation->pos_values[0])[psequence->movementboneindex];
		Vector& boneLastPosition = (*pbaseanimation->pos_values[lastFrameIndex])[psequence->movementboneindex];

		Vector& motion = psequence->linearmovement;
		motion.Clear();

		if(psequence->motiontype & STUDIO_LX)
			motion.x = boneLastPosition.x - boneFirstPosition.x;
		if(psequence->motiontype & STUDIO_LY)
			motion.y = boneLastPosition.y - boneFirstPosition.y;
		if(psequence->motiontype & STUDIO_LZ)
			motion.z = boneLastPosition.z - boneFirstPosition.z;

		for(Uint32 j = 0; j < psequence->numframes; j++)
		{
			for(Uint32 k = 0; k < pbaseanimation->nodes.size(); k++)
			{
				smdl::bone_node_t& node = pbaseanimation->nodes[k];
				if(node.parentindex != NO_POSITION)
					continue;

				// Currently, we use linear motion only
				Vector scaledMotion;
				Float motionScale = (j * 1.0) / (psequence->numframes - 1);
				Math::VectorScale(motion, motionScale, scaledMotion);

				for(Uint32 l = 0; l < psequence->panims.size(); l++)
				{
					smdl::animation_t* panimation = psequence->panims[l];
					Vector& frameBonePosition = (*panimation->pos_values[j])[k];
					Math::VectorSubtract(frameBonePosition, scaledMotion, frameBonePosition);
				}
			}
		}
	}

	// Extract unused rotational motion from sequences
	for(Uint32 i = 0; i < m_pSequencesArray.size(); i++)
	{
		smdl::sequence_t* psequence = m_pSequencesArray[i];
		smdl::animation_t* pbaseanimation = psequence->panims[0];

		for(Uint32 j = 0; j < pbaseanimation->nodes.size(); j++)
		{
			smdl::bone_node_t& node = pbaseanimation->nodes[j];
			if(node.parentindex != NO_POSITION)
				continue;

			for(Uint32 k = 0; k < psequence->panims.size(); k++)
			{
				smdl::animation_t* panimation = psequence->panims[k];
				Vector firstRotationalMotion = (*panimation->rot_values[0])[j];

				for(Uint32 l = 0; l < psequence->numframes; l++)
				{
					Vector& frameRotationalMotion = (*panimation->rot_values[l])[j];

					if(psequence->motiontype & STUDIO_XR)
						frameRotationalMotion.x = firstRotationalMotion.x;
					if(psequence->motiontype & STUDIO_YR)
						frameRotationalMotion.y = firstRotationalMotion.y;
					if(psequence->motiontype & STUDIO_ZR)
						frameRotationalMotion.z = firstRotationalMotion.z;
				}
			}
		}
	}

	// Extract auto motion
	for(Uint32 i = 0; i < m_pSequencesArray.size(); i++)
	{
		smdl::sequence_t* psequence = m_pSequencesArray[i];
		smdl::animation_t* pbaseanimation = psequence->panims[0];

		Int32 boneIndex = 0;
		if(!m_movementBoneName.empty())
		{
			Uint32 j = 0;
			for(; j < pbaseanimation->nodes.size(); j++)
			{
				smdl::bone_node_t& boneNode = pbaseanimation->nodes[j];
				if(!qstrcicmp(boneNode.bonename, m_movementBoneName))
				{
					boneIndex = j;
					break;
				}
			}
			
			if(j == pbaseanimation->nodes.size())
				WarningMsg("Couldn't find movement bone '%s' in sequence '%s'.\n", m_movementBoneName.c_str(), psequence->name.c_str());
		}

		psequence->automoveangles.resize(psequence->numframes);
		psequence->automovepositions.resize(psequence->numframes);

		Int32 motionType = psequence->motiontype;

		Vector& boneFirstPosition = (*pbaseanimation->pos_values[0])[boneIndex];
		Vector& boneFirstRotation = (*pbaseanimation->rot_values[0])[boneIndex];

		for(Int32 j = 0; j < psequence->numframes; j++)
		{
			Vector& frameBonePosition = (*pbaseanimation->pos_values[j])[boneIndex];
			Vector& frameBoneRotation = (*pbaseanimation->rot_values[j])[boneIndex];

			Vector& motion = psequence->automovepositions[j];
			Vector& angles = psequence->automoveangles[j];

			if(motionType & STUDIO_AX)
				motion.x = frameBonePosition.x - boneFirstPosition.x;
			if(motionType & STUDIO_AY)
				motion.y = frameBonePosition.y - boneFirstPosition.y;
			if(motionType & STUDIO_AZ)
				motion.z = frameBonePosition.z - boneFirstPosition.z;

			if(motionType & STUDIO_AXR)
				angles.x = frameBoneRotation.x - boneFirstRotation.x;
			if(motionType & STUDIO_AYR)
				angles.y = frameBoneRotation.y - boneFirstRotation.y;
			if(motionType & STUDIO_AZR)
				angles.z = frameBoneRotation.z - boneFirstRotation.z;
		}
	}
}

//===============================================
// @brief Create node transition mappings
//
//===============================================
void CStudioModelCompiler::CreateNodeTransitionMappings( void )
{
	Uint32 numTransitionNodes = 0;
	for(Uint32 i = 0; i < m_pSequencesArray.size(); i++)
	{
		smdl::sequence_t* psequence = m_pSequencesArray[i];
		if(psequence->entrynode > numTransitionNodes)
			numTransitionNodes = psequence->entrynode;
	}

	if(!numTransitionNodes)
		return;

	// Allocate our array
	m_transitionNodesArray.resize(numTransitionNodes+1);
	for(Uint32 i = 0; i < numTransitionNodes; i++)
		m_transitionNodesArray[i].resize(numTransitionNodes+1);

	// Add direct node transitions
	for(Uint32 i = 0; i < m_pSequencesArray.size(); i++)
	{
		smdl::sequence_t* psequence = m_pSequencesArray[i];
		if(psequence->entrynode != psequence->exitnode)
		{
			Int32 entryIndex = psequence->entrynode - 1;
			Int32 exitIndex = psequence->exitnode - 1;
			m_transitionNodesArray[entryIndex][exitIndex] = psequence->exitnode;

			if(psequence->nodeflags)
				m_transitionNodesArray[exitIndex][entryIndex] = psequence->entrynode;
		}
	}

	// Add multi-stage transitions
	while(true)
	{
		bool wasFound = false;
		for(Uint32 i = 1; i <= numTransitionNodes; i++)
		{
			for(Uint32 j = 1; j <= numTransitionNodes; j++)
			{
				if(i != j && m_transitionNodesArray[i-1][j-1] == 0)
				{
					for(Uint32 k = 1; k < numTransitionNodes; k++)
					{
						if(m_transitionNodesArray[k-1][j-1] > 0 && m_transitionNodesArray[i-1][k-1] > 0)
						{
							m_transitionNodesArray[i-1][j-1] = -m_transitionNodesArray[i-1][k-1];
							wasFound = true;
							break;
						}
					}
				}
			}
		}

		for(Uint32 i = 1; i <= numTransitionNodes; i++)
		{
			for(Uint32 j = 1; j <= numTransitionNodes; j++)
				m_transitionNodesArray[i-1][j-1] = SDL_abs(m_transitionNodesArray[i-1][j-1]);
		}

		if(!wasFound)
			break;
	}
}

//===============================================
// @brief Mark all used bones on meshes
//
//===============================================
void CStudioModelCompiler::MarkUsedBones( void )
{
	for(Uint32 i = 0; i < m_pSubmodelsArray.size(); i++)
	{
		smdl::submodel_t* psubmodel = m_pSubmodelsArray[i];
		if(psubmodel->pmeshes.empty())
			continue;

		MarkSubmodelUsedBones(psubmodel);

		// Check LOD too if it's present
		if(!psubmodel->plods.empty())
		{
			for(Uint32 j = 0; j < psubmodel->plods.size(); j++)
				MarkSubmodelUsedBones(psubmodel->plods[i]->plodmodel);
		}
	}
}

//===============================================
// @brief Mark used bones in a submodel
//
//===============================================
void CStudioModelCompiler::MarkSubmodelUsedBones( smdl::submodel_t* psubmodel )
{
	// Check hard-weight vertexes first
	for(Uint32 i = 0; i < psubmodel->numvertexes; i++)
	{
		smdl::vertex_t& vertex = psubmodel->vertexes[i];
		smdl::bone_t& bone = psubmodel->bones[vertex.boneindex];

		// Mark as used
		bone.refcounter++;
	}

	// Now weights
	for(Uint32 i = 0; i < psubmodel->numweightinfos; i++)
	{
		smdl::vertex_weightinfo_t& weightinfo = psubmodel->weightinfos[i];
		for(Uint32 j = 0; j < weightinfo.numweights; j++)
		{
			smdl::bone_t& bone = psubmodel->bones[weightinfo.boneindexes[j]];

			// Mark as used
			bone.refcounter++;
		}
	}

	// Check whether bone is protected
	CStringList_t::iterator itProtectedList = m_protectedBonesList.begin();
	while(itProtectedList != m_protectedBonesList.end())
	{
		for(Uint32 i = 0; i < psubmodel->nodes.size(); i++)
		{
			smdl::bone_node_t& node = psubmodel->nodes[i];
			if(!qstrcicmp(node.bonename, (*itProtectedList)))
			{
				smdl::bone_t& bone = psubmodel->bones[i];
				bone.refcounter++;

				Msg("Bone '%s' in submodel '%s' marked as protected.\n", node.bonename.c_str(), psubmodel->name.c_str());
				break;
			}
		}

		itProtectedList++;
	}
	
	// Check attachments also
	for(Uint32 i = 0; i < m_attachmentsArray.size(); i++)
	{
		smdl::attachment_t& attachment = m_attachmentsArray[i];
		for(Uint32 j = 0; j < psubmodel->nodes.size(); j++)
		{
			smdl::bone_node_t& node = psubmodel->nodes[j];
			if(!qstrcicmp(node.bonename, attachment.bonename))
			{
				smdl::bone_t& bone = psubmodel->bones[j];
				bone.refcounter++;
			}
		}
	}

	// Mark any parents that have a zero reference counter
	for(Uint32 i = 0; i < psubmodel->bones.size(); i++)
	{
		smdl::bone_t* pbone = &psubmodel->bones[i];
		if(pbone->refcounter > 0)
		{
			Int32 parentIndex = psubmodel->nodes[i].parentindex;
			while(parentIndex != NO_POSITION)
			{
				// Get ref counter of current
				pbone = &psubmodel->bones[parentIndex];
				if(pbone->refcounter > 0)
					break;

				// Mark as used
				pbone->refcounter++;

				// Proceed to our parent
				smdl::bone_node_t& node = psubmodel->nodes[parentIndex];
				parentIndex = node.parentindex;
			}
		}
	}
}

//===============================================
// @brief Rename any bones that were specified to be renamed
//
//===============================================
void CStudioModelCompiler::RenameBones( void )
{
	for(Uint32 i = 0; i < m_pSubmodelsArray.size(); i++)
	{
		smdl::submodel_t* psubmodel = m_pSubmodelsArray[i];

		for(Uint32 j = 0; j < psubmodel->bones.size(); j++)
		{
			smdl::bone_node_t& node = psubmodel->nodes[j];

			CStringMap_t::iterator it = m_boneRenameMap.find(node.bonename);
			if(it != m_boneRenameMap.end())
			{
				Msg("Bone '%s' in submodel '%s' was renamed to '%s'.\n", node.bonename.c_str(), psubmodel->name.c_str(), it->second.c_str());
				node.bonename = it->second;
			}
		}

		if(!psubmodel->plods.empty())
		{
			for(Uint32 j = 0; j < psubmodel->plods.size(); j++)
			{
				smdl::lod_t* plod = psubmodel->plods[j];

				for(Uint32 k = 0; k < plod->plodmodel->bones.size(); k++)
				{
					smdl::bone_node_t& node = plod->plodmodel->nodes[k];

					CStringMap_t::iterator it = m_boneRenameMap.find(node.bonename);
					if(it != m_boneRenameMap.end())
					{
						Msg("Bone '%s' in submodel '%s' lod '%s' was renamed to '%s'.\n", node.bonename.c_str(), psubmodel->name.c_str(), plod->plodmodel->name.c_str(), it->second.c_str());
						node.bonename = it->second;
					}
				}
			}
		}
	}
}

//===============================================
// @brief Set up/load data for textures used by the model
//
// @return TRUE if successful, FALSE on failure
//===============================================
bool CStudioModelCompiler::SetupTextures( void )
{
	// First of all, load the textures
	for(Uint32 i = 0; i < m_pTexturesArray.size(); i++)
	{
		smdl::texture_t* ptexture = m_pTexturesArray[i];
		if(!LoadTexture(ptexture))
			return false;
	}

	// Now process all the models, calculate their s/t coords
	for(Uint32 i = 0; i < m_pSubmodelsArray.size(); i++)
	{
		smdl::submodel_t* psubmodel = m_pSubmodelsArray[i];
		CalculateSubmodelSTCoords(psubmodel);

		if(!psubmodel->plods.empty())
		{
			for(Uint32 j = 0; j < psubmodel->plods.size(); j++)
			{
				smdl::lod_t* plod = psubmodel->plods[j];
				CalculateSubmodelSTCoords(plod->plodmodel);
			}
		}
	}

	// Set texture flags on final list of textures
	if(!SetTextureFlags())
	{
		ErrorMsg("Error when assigning texture rendermode flags.\n");
		return false;
	}

	// Set up skinrefs
	m_skinRefsArray.resize(m_pTexturesArray.size());
	for(Uint32 i = 0; i < m_pTexturesArray.size(); i++)
	{
		m_skinRefsArray[i].resize(m_pTexturesArray.size());
		for(Uint32 j = 0; j < m_pTexturesArray.size(); j++)
			m_skinRefsArray[i][j] = j;
	}

	// Build texture groups
	CArray<Uint32> textureReplacementNbsArray(m_textureGroupsArray.size());
	CArray<Uint32> textureLayersNbsArray(m_textureGroupsArray.size());
	CArray<CArray<CArray<Int32>>> textureGroupSkinrefsArray(m_textureGroupsArray.size());

	// This needs to be set BEFORE we load the textures
	// for the different texture groups!
	m_numSkinRefs = m_pTexturesArray.size();

	// Set up texture groups
	for(Uint32 i = 0; i < m_textureGroupsArray.size(); i++)
	{
		smdl::texturegroup_t& grp = m_textureGroupsArray[i];

		// Resize the skinref array for this group to fit
		Uint32 midArraySize = grp.replacements.size()+1;
		textureGroupSkinrefsArray[i].resize(midArraySize);
		for(Uint32 j = 0; j < midArraySize; j++)
			textureGroupSkinrefsArray[i][j].resize(grp.originals.size());

		for(Uint32 j = 0; j < grp.originals.size(); j++)
		{
			// First get the indexes of the originals
			smdl::grouptexture_t& origtexture = grp.originals[j];
			smdl::texture_t* porigtexture = GetTextureForName(origtexture.name.c_str(), true);
			if(!porigtexture)
			{
				ErrorMsg("Couldn't get base texture '%s' for texture group '%s'.\n", origtexture.name.c_str(), grp.groupname.c_str());
				return false;
			}

			if(!LoadTexture(porigtexture))
				return false;

			// Resize array and set value
			textureGroupSkinrefsArray[i][0][j] = porigtexture->skinref;

			// Now the indexes of the replacements
			for(Uint32 k = 0; k < grp.replacements.size(); k++)
			{
				smdl::grouptexture_t& reptexture = grp.replacements[k][j];
				smdl::texture_t* preptexture = GetTextureForName(reptexture.name.c_str());
				if(!preptexture)
				{
					ErrorMsg("Couldn't get replacement texture '%s' for texture group '%s'.\n", reptexture.name.c_str(), grp.groupname.c_str());
					return false;
				}

				if(!LoadTexture(preptexture))
					return false;

				// Set values
				preptexture->parent = porigtexture->skinref;
				textureGroupSkinrefsArray[i][k+1][j] = preptexture->skinref;
			}

			// Set layer count
			Uint32 nbLayers = grp.replacements.size() + 1;
			if(nbLayers > textureLayersNbsArray[i])
				textureLayersNbsArray[i] = nbLayers;
		}

		// Set number of textures that have replacements
		textureReplacementNbsArray[i] = grp.originals.size();
	}

	if(!textureLayersNbsArray.empty())
	{
		// Set skinref values
		// TODO: Move this to VBM in a way that supprts multiple group settings
		// For now we only support one group...
		for(Uint32 i = 0; i < textureLayersNbsArray[0]; i++)
		{
			for(Uint32 j = 0; j < textureReplacementNbsArray[0]; j++)
			{
				Int32 insertindex = textureGroupSkinrefsArray[0][0][j];
				m_skinRefsArray[i][insertindex] = textureGroupSkinrefsArray[0][i][j];
			}
		}
	}

	// Set skin family count
	if(!textureLayersNbsArray.empty() && textureLayersNbsArray[0] > 0)
		m_numSkinFamilies = textureLayersNbsArray[0];
	else
		m_numSkinFamilies = 1;

	return true;
}

//===============================================
// @brief Load a texuture used
//
// @param ptexture Pointer to texture object
// @return TRUE if successful, FALSE on failure
//===============================================
bool CStudioModelCompiler::LoadTexture( smdl::texture_t* ptexture )
{
	if(g_options.isFlagSet(CMP_FL_STRIP_MDL_TEXTURES))
	{
		// Set basics
		ptexture->width = ptexture->filewidth = 32;
		ptexture->height = ptexture->fileheight = 32;
		
		// Create dummy pure white texture
		Uint32 paletteSize = 256 * sizeof(color24_t);
		ptexture->ppalette = new byte[paletteSize];
		memset(ptexture->ppalette, 255, sizeof(byte)*paletteSize);

		Uint32 textureDataSize = ptexture->width * ptexture->height;
		ptexture->ptexturedata = new byte[textureDataSize];
		memset(ptexture->ptexturedata, 0, sizeof(byte)*textureDataSize);
	}
	else
	{
		// TODO: Support TGAs
		// TODO: Add a downsampling algorythm
		// Note: Tried coming up with my own, nope.mov
		CString textureName = ptexture->name;
		Common::Basename(ptexture->name.c_str(), textureName);
		
		CString filePath;
		filePath << textureName << ".bmp";

		Uint32 fileSize = 0;
		const byte* pfile = nullptr;
		if(!OpenFile(filePath.c_str(), fileSize, pfile))
		{
			for(Uint32 i = 0; i < m_textureFoldersArray.size(); i++)
			{
				filePath.clear();
				filePath << m_textureFoldersArray[i] << PATH_SLASH_CHAR << ptexture->name;
				if(OpenFile(filePath.c_str(), fileSize, pfile))
					break;
			}
		}

		if(!pfile)
		{
			ErrorMsg("Failed to open texture '%s'.\n", ptexture->name.c_str());
			return false;
		}

		Uint32 srcWidth = 0;
		Uint32 srcHeight = 0;
		Uint32 dataSize = 0;

		byte* ppalette = nullptr;
		byte* ptexturedata = nullptr;

		// Figure out sizes
		texture_compression_t compression = TX_COMPRESSION_NONE;
		if(!BMP_Load8Bit(filePath.c_str(), pfile, ppalette, ptexturedata, srcWidth, srcHeight, dataSize, compression, ErrorMsg))
		{
			ErrorMsg("Texture '%s' is not a valid 8-bit BMP file.\n", ptexture->name.c_str());
			FreeFile(pfile);
			return false;
		}

		// Release file data
		FreeFile(pfile);

		ptexture->width = CompilerMath::GetBestPowerOfTwo(MIN_TEXTURE_RESOLUTION, srcWidth);
		if(ptexture->width > g_options.max_texture_resolution)
		{
			ErrorMsg("Texture '%s' width %d exceeds maximum texture width '%d'.\n", ptexture->name.c_str(), ptexture->width, g_options.max_texture_resolution);
			return false;
		}

		ptexture->height = CompilerMath::GetBestPowerOfTwo(MIN_TEXTURE_RESOLUTION, srcHeight);
		if(ptexture->height > g_options.max_texture_resolution)
		{
			ErrorMsg("Texture '%s' height %d exceeds maximum texture height '%d'.\n", ptexture->name.c_str(), ptexture->height, g_options.max_texture_resolution);
			return false;
		}

		// Set palette ptr and source file sizes
		ptexture->ppalette = ppalette;
		ptexture->filewidth = srcWidth;
		ptexture->fileheight = srcHeight;

		// Allocate data
		ptexture->ptexturedata = new byte[ptexture->width * ptexture->height];
		byte* pdest = ptexture->ptexturedata;

		// Now perform any padding
		Uint32 trueWidth = (srcWidth % 4) != 0 ? ((srcWidth / 4) * 4 + 4) : srcWidth;

		// Paste in the texels, leaving a blank of srcres - skinres
		for(Uint32 i = 0; i < srcHeight; i++)
		{
			for(Uint32 j = 0; j < srcWidth; j++)
			{
				(*pdest) = *(ptexturedata + i * trueWidth + j);
				pdest++;
			}

			if(srcWidth != ptexture->width)
			{
				// Fill empty spaces with the index of the color at index 255
				for(Uint32 j = srcWidth; j < ptexture->width; j++)
				{
					if(j <= srcWidth + g_options.border_padding)
						(*pdest) = *(ptexturedata + i * trueWidth + srcWidth - 1);
					else
						(*pdest) = 0xFF;
					
					pdest++;
				}
			}
		}

		if(srcHeight != ptexture->height)
		{
			// Fill empty spaces with the index of the color at index 255
			for(Uint32 i = srcHeight; i < ptexture->height; i++)
			{
				for(Uint32 j = 0; j < ptexture->width; j++)
				{
					if(i <= srcHeight + g_options.border_padding && j <= (srcWidth+g_options.border_padding))
					{
						if(j < srcWidth)
							(*pdest) = *(ptexturedata + (srcHeight-1) * trueWidth + j);
						else
							(*pdest) = *(ptexturedata + (srcHeight-1) * trueWidth + (srcWidth-1));
					}
					else
						(*pdest) = 255;

					pdest++;
				}
			}
		}
		
		// Turn BGR into RGB
		for(Uint32 i = 0; i < 256; i++)
		{
			byte* pPaletteColor = ptexture->ppalette + (i * 3);
			byte tmp = pPaletteColor[0];
			pPaletteColor[0] = pPaletteColor[2];
			pPaletteColor[2] = tmp;
		}	

		// Apply any gamma if set
		if(m_textureGamma != 1.8)
		{
			Float gamma = m_textureGamma / 1.8;

			for(Uint32 i = 0; i < 256; i++)
			{
				byte* pPaletteColor = ptexture->ppalette + (i * 3);
				for(Uint32 j = 0; j < 3; j++)
				{
					Float value = static_cast<Float>(pPaletteColor[j]) / 255.0f;
					pPaletteColor[j] = SDL_pow(value, gamma) * 255;
				}
			}
		}

		// Release temp data
		delete[] ptexturedata;
	}

	Uint32 textureDataSize = 256 * sizeof(byte) + ptexture->width * ptexture->height * sizeof(byte);
	Float megaBytes = Common::BytesToMegaBytes(textureDataSize);
	Msg("Loaded texture '%s'(%d x %d) with skinref '%d', %.2f mbytes.\n", ptexture->name.c_str(), ptexture->width, ptexture->height, ptexture->skinref, megaBytes);

	return true;
}

//===============================================
// @brief Checks animations for looping and event validity
//
//===============================================
void CStudioModelCompiler::CheckAnimations( void )
{
	for(Uint32 i = 0; i < m_pSequencesArray.size(); i++)
	{
		smdl::sequence_t* psequence = m_pSequencesArray[i];

		if(psequence->flags & STUDIO_LOOPING)
		{
			for(Uint32 j = 0; j < psequence->panims[0]->nodes.size(); j++)
			{
				for(Uint32 k = 0; k < psequence->panims.size(); k++)
				{
					// Get end and start pos and rot values
					Int32 endframeindex = (psequence->numframes - 1);
					CArray<Vector>* pstartrotations = psequence->panims[k]->rot_values[0];
					CArray<Vector>* pendrotations = psequence->panims[k]->rot_values[endframeindex];

					CArray<Vector>* pstartpositions = psequence->panims[k]->pos_values[0];
					CArray<Vector>* pendpositions = psequence->panims[k]->pos_values[endframeindex];

					// Ensure motions are matched to start unless set to move
					if(!(psequence->motiontype & STUDIO_LX))
						(*pendpositions)[j].x = (*pstartpositions)[j].x;

					if(!(psequence->motiontype & STUDIO_LY))
						(*pendpositions)[j].y = (*pstartpositions)[j].y;

					if(!(psequence->motiontype & STUDIO_LZ))
						(*pendpositions)[j].z = (*pstartpositions)[j].z;

					// Ensure position matches also
					(*pendrotations)[j] = (*pstartrotations)[j];
				}
			}

			// Check that events are valid
			if(!psequence->events.empty())
			{
				for(Uint32 j = 0; j < psequence->events.size(); j++)
				{
					smdl::animevent_t& event = psequence->events[j];
					if(event.frame < psequence->startframe)
					{
						WarningMsg("Sequence '%s' has an event with id %d that is set before the first valid frame, which is %d.\n", psequence->name.c_str(), event.eventid, psequence->startframe);
						event.frame = psequence->startframe;
					}

					if(event.frame > psequence->endframe)
					{
						WarningMsg("Sequence '%s' has an event with id %d that is set after the last valid frame, which is %d.\n", psequence->name.c_str(), event.eventid, psequence->endframe);
						event.frame = psequence->startframe;
					}
				}
			}
		}

		// Mark frame offset value
		psequence->frameoffset = psequence->panims[0]->startframe;
	}
}

//===============================================
// @brief Poplates the bone table with the union of all bones used
//
// @return TRUE if successful, FALSE if failed
//===============================================
bool CStudioModelCompiler::CreateBoneTable( void )
{
	// Get bones from all submodels and LODs
	bool result = true;
	for(Uint32 i = 0; i < m_pSubmodelsArray.size(); i++)
	{
		smdl::submodel_t* psubmodel = m_pSubmodelsArray[i];
		if(!AddSubmodelBonesToTable(psubmodel))
			result = false;

		if(!psubmodel->plods.empty())
		{
			for(Uint32 j = 0; j < psubmodel->plods.size(); j++)
			{
				smdl::lod_t* plod = psubmodel->plods[j];
				if(!AddSubmodelBonesToTable(plod->plodmodel))
					result = false;
			}
		}
	}

	//
	// Perform validity checks on all blend list, pivot, etc stuff now,
	// because we can only really do this reliably AFTER the global bone
	// table has been created.

	// Now mark all bones that are set not to blend
	if(result && !m_noBlendBonesList.empty())
	{
		CStringList_t::iterator it = m_noBlendBonesList.begin();
		while(it != m_noBlendBonesList.end())
		{
			Int32 nodeIndex = GetBoneIndex((*it).c_str());
			if(nodeIndex != NO_POSITION)
			{
				smdl::boneinfo_t* pBone = m_pBoneTableArray[nodeIndex];
				pBone->flags |= STUDIO_DONT_BLEND;
				it++;
				continue;
			}
			else
			{
				ErrorMsg("Couldn't find '$noblend' bone '%s' in global table.\n", (*it).c_str());
				result = false;
			}

			it++;
		}
	}

	// Check that pivots are valid
	for(Uint32 i = 0; i < m_pivotNamesArray.size(); i++)
	{
		const CString& pivotName = m_pivotNamesArray[i];
		Int32 boneIndex = GetBoneIndex(pivotName.c_str());
		if(boneIndex == NO_POSITION)
			WarningMsg("Pivot bone '%s' not found in global bone table.\n", pivotName.c_str());
	}

	// Check that the protected bones list's bones are valid
	{
		CStringList_t::iterator it = m_protectedBonesList.begin();
		while(it != m_protectedBonesList.end())
		{
			Int32 nodeIndex = GetBoneIndex((*it).c_str());
			if(nodeIndex == NO_POSITION)
				WarningMsg("Protected bone '%s' not found in global bone table.\n", (*it).c_str());

			it++;
		}
	}

	// Ensure movement bone is valid
	if(!m_movementBoneName.empty())
	{
		Int32 nodeIndex = GetBoneIndex(m_movementBoneName.c_str());
		if(nodeIndex == NO_POSITION)
			WarningMsg("Movement bone '%s' not found in global bone table.\n", m_movementBoneName.c_str());
	}

	// Check if we've gone over the bones limit
	if(!g_options.isFlagSet(CMP_FL_NO_STUDIOMDL_BONE_LIMIT))
	{
		if(m_pBoneTableArray.size() >= MAXSTUDIOBONES)
		{
			ErrorMsg("Final bones list exceeded MAXSTUDIOBONES(%d > %d).\n", m_pBoneTableArray.size(), static_cast<Int32>(MAXSTUDIOBONES));
			return false;
		}
	}

	// Check against format type limitation
	if(m_pBoneTableArray.size() >= MAX_TOTAL_BONES)
	{
		ErrorMsg("Final bones list exceeded MAX_TOTAL_BONES(%d > %d) byte datatype limit.\n", m_pBoneTableArray.size(), static_cast<Int32>(MAX_TOTAL_BONES));
		return false;
	}	

	return result;
}

//===============================================
// @brief Renames sequence bones based on the bone rename settings
//
//===============================================
void CStudioModelCompiler::RenameSequenceBones( void )
{
	for(Uint32 i = 0; i < m_pSequencesArray.size(); i++)
	{
		smdl::sequence_t* psequence = m_pSequencesArray[i];
		smdl::animation_t* pbaseanim = psequence->panims[0];

		for(Uint32 j = 0; j < pbaseanim->nodes.size(); j++)
		{
			smdl::bone_node_t& node = pbaseanim->nodes[j];

			CStringMap_t::iterator it = m_boneRenameMap.find(node.bonename);
			if(it != m_boneRenameMap.end())
				node.bonename = it->second;
		}
	}
}

//===============================================
// @brief Maps each sequence bone to the global list
//
// @return TRUE if successful, FALSE if failed
//===============================================
bool CStudioModelCompiler::RemapSequenceBones( void )
{
	// Apply renamings to all bones first
	RenameSequenceBones();

	bool result = true;
	for(Uint32 i = 0; i < m_pSequencesArray.size(); i++)
	{
		smdl::sequence_t* psequence = m_pSequencesArray[i];
		smdl::animation_t* pbaseanim = psequence->panims[0];

		// Resize the mapping arrays
		pbaseanim->bonemap_inverse.resize(m_pBoneTableArray.size());
		for(Uint32 j = 0; j < pbaseanim->bonemap_inverse.size(); j++)
			pbaseanim->bonemap_inverse[j] = NO_POSITION;

		pbaseanim->bonemap.resize(pbaseanim->nodes.size());
		for(Uint32 j = 0; j < pbaseanim->bonemap.size(); j++)
			pbaseanim->bonemap[j] = NO_POSITION;

		for(Uint32 j = 0; j < pbaseanim->nodes.size(); j++)
		{
			smdl::bone_node_t& node = pbaseanim->nodes[j];
			Int32 tableBoneIndex = GetBoneIndex(node.bonename.c_str());
			if(tableBoneIndex == NO_POSITION)
			{
				// Not a bone that's being used
				pbaseanim->bonemap[j] = NO_POSITION;
				continue;
			}
			else
			{
				const Char* pstrAnimationBoneName;
				if(node.parentindex != NO_POSITION)
				{
					smdl::bone_node_t& parentNode = pbaseanim->nodes[node.parentindex];
					pstrAnimationBoneName = parentNode.bonename.c_str();
				}
				else
					pstrAnimationBoneName = "ROOT";

				const Char* pstrGlobalBoneName;
				smdl::boneinfo_t* pGlobalNode = m_pBoneTableArray[tableBoneIndex];
				if(pGlobalNode->parent_index != NO_POSITION)
				{
					smdl::boneinfo_t* pGlobalParentNode = m_pBoneTableArray[pGlobalNode->parent_index];
					pstrGlobalBoneName = pGlobalParentNode->name.c_str();
				}
				else
					pstrGlobalBoneName = "ROOT";

				if(qstrcicmp(pstrAnimationBoneName, pstrGlobalBoneName))
				{
					Msg("Parent bone of '%s' in animation '%s' does not match with global table.\n", node.bonename.c_str(), pbaseanim->name.c_str());
					Msg("\tParent in '%s' is named as '%s'.\n", pbaseanim->name.c_str(), pstrAnimationBoneName);
					Msg("\tParent in global table is named as '%s'.\n", pstrGlobalBoneName);
					result = false;
				}

				// Set final mappings
				pbaseanim->bonemap[j] = tableBoneIndex;
				pbaseanim->bonemap_inverse[tableBoneIndex] = j;
			}
		}
	}

	return result;
}

//===============================================
// @brief Adds a submodel's bones to the bone table
//
// @param psubmodel Pointer to submodel to process
// @return TRUE if successful, FALSE if failed
//===============================================
bool CStudioModelCompiler::AddSubmodelBonesToTable( smdl::submodel_t* psubmodel )
{
	// Process each bone into the global array, ONLY if
	// they have a refcount above 0
	for(Uint32 i = 0; i < psubmodel->bones.size(); i++)
	{
		smdl::bone_t& bone = psubmodel->bones[i];
		if(bone.refcounter <= 0)
			continue;

		smdl::bone_node_t& node = psubmodel->nodes[i];
		Int32 boneIndex = GetBoneIndex(node.bonename.c_str());
		if(boneIndex == NO_POSITION)
		{
			boneIndex = m_pBoneTableArray.size();
			smdl::boneinfo_t* pnewbone = new smdl::boneinfo_t();
			pnewbone->name = node.bonename;
			pnewbone->index = boneIndex;

			if(node.parentindex != NO_POSITION)
			{
				smdl::bone_node_t& parentNode = psubmodel->nodes[node.parentindex];
				pnewbone->parent_index = GetBoneIndex(parentNode.bonename.c_str());
				if(pnewbone->parent_index == NO_POSITION)
				{
					ErrorMsg("Bone '%s' parent '%s' not found in global bone table.\n", node.bonename.c_str(), parentNode.bonename.c_str());
					return false;
				}
			}
			else
			{
				// No parent
				pnewbone->parent_index = NO_POSITION;
			}

			pnewbone->controller_index = NO_POSITION;
			pnewbone->position = bone.position;
			pnewbone->rotation = bone.rotation;

			
			m_pBoneTableArray.push_back(pnewbone);
		}
		else
		{
			Int32 tableParentIndex = NO_POSITION;
			if(node.parentindex != NO_POSITION)
			{
				smdl::bone_node_t& parentNode = psubmodel->nodes[node.parentindex];
				tableParentIndex = GetBoneIndex(parentNode.bonename.c_str());
			}

			smdl::boneinfo_t* pTableNode = m_pBoneTableArray[boneIndex];
			Int32 localParentIndex = pTableNode->parent_index;

			if(tableParentIndex != localParentIndex)
			{
				const Char* pstrTableParentName;
				if(tableParentIndex != NO_POSITION)
					pstrTableParentName = m_pBoneTableArray[tableParentIndex]->name.c_str();
				else
					pstrTableParentName = "ROOT";

				const Char* pstrModelParentName;
				if(localParentIndex != NO_POSITION)
					pstrModelParentName = psubmodel->nodes[localParentIndex].bonename.c_str();
				else
					pstrModelParentName = "ROOT";

				Msg("Parent bone of '%s' in submodel '%s' does not match with global table.\n", node.bonename.c_str(), psubmodel->name.c_str());
				Msg("\tParent in '%s' is named as '%s'.\n", psubmodel->name.c_str(), pstrModelParentName);
				Msg("\tParent in global table is named as '%s'.\n", pstrTableParentName);
				return false;
			}
		}

		// Add required mappings
		if(boneIndex >= psubmodel->boneimap.size())
			psubmodel->boneimap.resize(psubmodel->boneimap.size() + 1);

		psubmodel->boneimap[boneIndex] = i;
		bone.globalindex = boneIndex;
	}

	return true;
}

//===============================================
// @brief Link bone controllers with their bones
//
// @return TRUE if successful, FALSE if failed
//===============================================
bool CStudioModelCompiler::LinkBoneControllers( void )
{
	for(Uint32 i = 0; i < m_boneControllersArray.size(); i++)
	{
		smdl::bonecontroller_t& controller = m_boneControllersArray[i];
		controller.boneindex = GetBoneIndex(controller.bonename.c_str());
		if(controller.boneindex == NO_POSITION)
		{
			ErrorMsg("Could not find bone '%s' in global table for bone controller with index '%d'.\n", controller.bonename.c_str(), controller.controllerindex);
			return false;
		}
	}

	return true;
}

//===============================================
// @brief Link attachments to their bones
//
// @return TRUE if successful, FALSE if failed
//===============================================
bool CStudioModelCompiler::LinkAttachments( void )
{
	for(Uint32 i = 0; i < m_attachmentsArray.size(); i++)
	{
		smdl::attachment_t& attachment = m_attachmentsArray[i];
		attachment.bone_index = GetBoneIndex(attachment.bonename.c_str());
		if(attachment.bone_index == NO_POSITION)
		{
			ErrorMsg("Could not find bone '%s' in global table for bone attachment with index '%d'.\n", attachment.bonename.c_str(), attachment.attachment_index);
			return false;
		}
	}

	return true;
}

//===============================================
// @brief Calculate submodel s/t integer coordinates 
// for GoldSrc model data. These are integer based
// texture coordinates, so they're rounded to the
// nearest integer value.
//
// @param psubmodel Pointer to submodel to process
//===============================================
void CStudioModelCompiler::CalculateSubmodelSTCoords( smdl::submodel_t* psubmodel )
{
	for(Uint32 i = 0; i < psubmodel->pmeshes.size(); i++)
	{
		smdl::mesh_t* pmesh = psubmodel->pmeshes[i];
		smdl::texture_t* ptexture = m_pTexturesArray[pmesh->skinref];
		Uint32 sizes[2] = {ptexture->filewidth, ptexture->fileheight};

		for(Uint32 j = 0; j < pmesh->trivertexes.size(); j++)
		{
			smdl::triangle_vertex_t& vertex = pmesh->trivertexes[j];

			for(Uint32 k = 0; k < 2; k++)
			{
				Float value = vertex.texcoords[k] * sizes[k];
				if(value - SDL_floor(value) > 0.5)
					value = SDL_ceil(value);
				else
					value = SDL_floor(value);

				vertex.int_texcoords[k] = value;
			}
		}
	}
}

//===============================================
// @brief Sets final(global) bone indexes on all 
// elements of all submodels
//
//===============================================
void CStudioModelCompiler::SetFinalBoneIndexes( void )
{
	// Get bones from all submodels and LODs
	for(Uint32 i = 0; i < m_pSubmodelsArray.size(); i++)
	{
		smdl::submodel_t* psubmodel = m_pSubmodelsArray[i];
		SetFinalSubmodelBoneIndexes(psubmodel);

		if(!psubmodel->plods.empty())
		{
			for(Uint32 j = 0; j < psubmodel->plods.size(); j++)
			{
				smdl::lod_t* plod = psubmodel->plods[j];
				SetFinalSubmodelBoneIndexes(plod->plodmodel);
			}
		}
	}
}

//===============================================
// @brief Sets final bone indexes on all elements of a submodel
//
// @param psubmodel Pointer to submodel to process
//===============================================
void CStudioModelCompiler::SetFinalSubmodelBoneIndexes( smdl::submodel_t* psubmodel )
{
	// Set final bone indexes on vertexes
	for(Uint32 i = 0; i < psubmodel->numvertexes; i++)
	{
		smdl::vertex_t& vertex = psubmodel->vertexes[i];
		vertex.boneindex = psubmodel->bones[vertex.boneindex].globalindex;
	}

	// Set final bone indexes on normals
	for(Uint32 i = 0; i < psubmodel->numnormals; i++)
	{
		smdl::normal_t& normal = psubmodel->normals[i];
		normal.boneindex = psubmodel->bones[normal.boneindex].globalindex;
	}

	// Set final bone indexes on vertex weights
	for(Uint32 i = 0; i < psubmodel->numweightinfos; i++)
	{
		smdl::vertex_weightinfo_t& weight = psubmodel->weightinfos[i];
		for(Uint32 j = 0; j < weight.numweights; j++)
			weight.boneindexes[j] = psubmodel->bones[weight.boneindexes[j]].globalindex;
	}

	if(psubmodel->pflexmodel)
	{
		// Set it on the flexes also
		for(Uint32 i = 0; i < psubmodel->pflexmodel->pflexes.size(); i++)
		{
			smdl::flexframe_t* pflexframe = psubmodel->pflexmodel->pflexes[i];
			for(Uint32 j = 0; j < pflexframe->numvertexes; j++)
			{
				smdl::flexvertex_t& flexvert = pflexframe->vertexes[j];
				flexvert.boneindex = psubmodel->bones[flexvert.boneindex].globalindex;
			}
		}
	}
}

//===============================================
// @brief Set hitgroup data based on what was defined
// in the QC file
//
// @return TRUE if successful, FALSE if failed
//===============================================
bool CStudioModelCompiler::SetupHitGroups( void )
{
	for(Uint32 i = 0; i < m_hitGroupArray.size(); i++)
	{
		smdl::hitgroup_t& grp = m_hitGroupArray[i];
		if(grp.script)
			continue;

		Uint32 j = 0;
		for(; j < m_pBoneTableArray.size(); j++)
		{
			smdl::boneinfo_t* pbone = m_pBoneTableArray[j];
			if(!qstrcicmp(pbone->name, grp.name))
			{
				pbone->hitgroup = grp.hitgroup;
				pbone->hitgroupset = true;
				break;
			}
		}

		if(j == m_pBoneTableArray.size())
		{
			ErrorMsg("Could not find bone for hitgroup '%s' with id %d.\n", grp.name.c_str(), grp.hitgroup);
			return false;
		}
	}

	for(Uint32 i = 0; i < m_pBoneTableArray.size(); i++)
	{
		smdl::boneinfo_t* pbone = m_pBoneTableArray[i];
		if(!pbone->hitgroupset)
		{
			if(pbone->parent_index != NO_POSITION)
			{
				smdl::boneinfo_t* pparentbone = m_pBoneTableArray[pbone->parent_index];
				pbone->hitgroup = pparentbone->hitgroup;
			}
			else
				pbone->hitgroup = HITGROUP_GENERIC;
		}
	}

	return true;
}

//===============================================
// @brief Sets up hitbox data based on what is in the QC
// file, or generate new data.
//
// @return TRUE if successful, FALSE if failed
//===============================================
bool CStudioModelCompiler::SetupHitBoxes( void )
{
	// If there were none, then build a default list
	if(m_hitBoxesArray.empty())
	{
		BuildHitBoxes();
		return true;
	}

	// Just link up hitboxes with their bones
	for(Uint32 i = 0; i < m_hitBoxesArray.size(); i++)
	{
		smdl::hitbox_t& hbox = m_hitBoxesArray[i];
		hbox.boneindex = GetBoneIndex(hbox.bonename.c_str());
		if(hbox.boneindex == NO_POSITION)
		{
			ErrorMsg("Could not find bone '%s' for hitbox.\n", hbox.bonename.c_str());
			return false;
		}

		if(g_options.isFlagSet(CMP_FL_DUMP_HITBOX_DATA))
		{
			CString hitgroupname;
			if(!m_hitGroupArray.empty())
			{
				Uint32 j = 0;
				for(; j < m_hitGroupArray.size(); j++)
				{
					smdl::hitgroup_t group = m_hitGroupArray[j];
					if(group.hitgroup == hbox.hitgroup)
					{
						hitgroupname = group.name;
						break;
					}
				}

				if(j == m_hitGroupArray.size())
				{
					WarningMsg("Couldn't find hitgroup with ID %d used by hitbox for bone '%s'.\n", hbox.hitgroup, hbox.bonename.c_str());
					hitgroupname = "UNKNOWN";
				}
			}

			if(!hitgroupname.empty())
				Msg("Hitbox with hitgroup named '%s' for bone '%s':\n", hitgroupname.c_str(), hbox.bonename.c_str());
			else
				Msg("Hitbox with hitgroup ID %d for bone '%s':\n", hbox.hitgroup, hbox.bonename.c_str());

			Msg("\tMins: %.2f %.2f %.2f.\n", hbox.mins.x, hbox.mins.y, hbox.mins.z);
			Msg("\tMaxs: %.2f %.2f %.2f.\n", hbox.maxs.x, hbox.maxs.y, hbox.maxs.z);
		}
	}

	return true;
}

//===============================================
// @brief Auto-creates hitboxes if there were none defined
//
// @return TRUE if successful, FALSE if failed
//===============================================
void CStudioModelCompiler::BuildHitBoxes( void )
{
	for(Uint32 i = 0; i < m_pBoneTableArray.size(); i++)
	{
		smdl::boneinfo_t* pbone = m_pBoneTableArray[i];

		pbone->mins = NULL_MINS;
		pbone->maxs = NULL_MAXS;
	}

	// Build a bbox based on the vertices that belong to a bone
	for(Uint32 i = 0; i < m_pSubmodelsArray.size(); i++)
	{
		const smdl::submodel_t* psubmodel = m_pSubmodelsArray[i];
		if(!psubmodel->numvertexes)
			continue;

		for(Uint32 j = 0; j < psubmodel->numvertexes; j++)
		{
			const smdl::vertex_t& vertex = psubmodel->vertexes[j];
			Int32 boneIndex = vertex.boneindex;
			smdl::boneinfo_t* pbone = m_pBoneTableArray[boneIndex];

			for(Uint32 k = 0; k < 3; k++)
			{
				if(vertex.position[k] < pbone->mins[k])
					pbone->mins[k] = vertex.position[k];

				if(vertex.position[k] > pbone->maxs[k])
					pbone->maxs[k] = vertex.position[k];
			}
		}
	}

	// Also count in child bone positions
	for(Uint32 j = 0; j < m_pBoneTableArray.size(); j++)
	{
		smdl::boneinfo_t* pbone = m_pBoneTableArray[j];
		if(pbone->parent_index != NO_POSITION)
		{
			// Get this bone's parent bone
			smdl::boneinfo_t* pparentbone = m_pBoneTableArray[pbone->parent_index];

			for(Uint32 k = 0; k < 3; k++)
			{
				if(pbone->position[k] < pparentbone->mins[k])
					pparentbone->mins[k] = pbone->position[k];

				if(pbone->position[k] > pparentbone->maxs[k])
					pparentbone->maxs[k] = pbone->position[k];
			}
		}
	}

	// Define the hit boxes
	for(Uint32 i = 0; i < m_pBoneTableArray.size(); i++)
	{
		smdl::boneinfo_t* pbone = m_pBoneTableArray[i];
		
		Uint32 j = 0;
		for(; j < 3; j++)
		{
			if(pbone->mins[j] < pbone->maxs[j] - 1)
				break;
		}

		if(j == 3)
			continue;

		// Filter on this regardless of whether the QC specified a hitgroup
		Int32 hitGroupType = DetermineBoneHitGroupType(pbone->name.c_str());
		if(hitGroupType == HITGROUP_NONE)
			continue;

		smdl::hitbox_t newHitBox;
		newHitBox.boneindex = i;
		newHitBox.mins = pbone->mins;
		newHitBox.maxs = pbone->maxs;
		newHitBox.bonename = pbone->name;

		if(pbone->hitgroupset)
			newHitBox.hitgroup = static_cast<hitgroups_t>(pbone->hitgroup);
		else
			newHitBox.hitgroup = hitGroupType;

		if(g_options.isFlagSet(CMP_FL_DUMP_HITBOX_DATA))
		{
			CString hitgroupname;
			if(!m_hitGroupArray.empty())
			{
				Uint32 j = 0;
				for(; j < m_hitGroupArray.size(); j++)
				{
					smdl::hitgroup_t group = m_hitGroupArray[j];
					if(group.hitgroup == newHitBox.hitgroup)
					{
						hitgroupname = group.name;
						break;
					}
				}

				if(j == m_hitGroupArray.size())
				{
					WarningMsg("Couldn't find hitgroup with ID %d used by hitbox for bone '%s'.\n", newHitBox.hitgroup, newHitBox.bonename.c_str());
					hitgroupname = "UNKNOWN";
				}
			}

			if(!hitgroupname.empty())
				Msg("Generated hitbox with hitgroup named '%s' for bone '%s':\n", hitgroupname.c_str(), pbone->name.c_str());
			else
				Msg("Generated hitbox with hitgroup ID %d for bone '%s':\n", newHitBox.hitgroup, pbone->name.c_str());

			Msg("\tMins: %.2f %.2f %.2f.\n", newHitBox.mins.x, newHitBox.mins.y, newHitBox.mins.z);
			Msg("\tMaxs: %.2f %.2f %.2f.\n", newHitBox.maxs.x, newHitBox.maxs.y, newHitBox.maxs.z);
		}

		m_hitBoxesArray.push_back(newHitBox);
	}
}

//===============================================
// @brief Sets final animation bone positions
//
//===============================================
void CStudioModelCompiler::SetFinalSequenceBonePositions( void )
{
	for(Uint32 i = 0; i < m_pSequencesArray.size(); i++)
	{
		smdl::sequence_t* psequence = m_pSequencesArray[i];
		smdl::animation_t* pbaseanimation = psequence->panims[0];

		for(Uint32 j = 0; j < psequence->panims.size(); j++)
		{
			smdl::animation_t* panimation = psequence->panims[j];

			for(Uint32 k = 0; k < psequence->numframes; k++)
			{
				CArray<Vector>* pPositionsArray = (panimation->pos_values[k]);
				if(pPositionsArray->size() < m_pBoneTableArray.size())
					pPositionsArray->resize(m_pBoneTableArray.size());

				CArray<Vector>* pRotationsArray = (panimation->rot_values[k]);
				if(pRotationsArray->size() < m_pBoneTableArray.size())
					pRotationsArray->resize(m_pBoneTableArray.size());

				// Store original positions and rotations, DO NOT make
				// these references by mistake, as we need the original
				// unaltered data here!
				CArray<Vector> originalPositions = (*pPositionsArray);
				CArray<Vector> originalRotations = (*pRotationsArray);

				for(Uint32 l = 0; l < m_pBoneTableArray.size(); l++)
				{
					smdl::boneinfo_t* pbone = m_pBoneTableArray[l];
					Int32 originalIndex = pbaseanimation->bonemap_inverse[l];
					if(originalIndex != NO_POSITION)
					{
						// Fill in from array of originals
						(*pPositionsArray)[l] = originalPositions[originalIndex];
						(*pRotationsArray)[l] = originalRotations[originalIndex];
					}
					else
					{
						// Fill in from base position and rotation
						(*pPositionsArray)[l] = pbone->position;
						(*pRotationsArray)[l] = pbone->rotation;
					}
				}
			}
		}
	}
}

//===============================================
// @brief Determine bone scale values for final
// animation data optimization.
//
//===============================================
void CStudioModelCompiler::CalculateBoneScales( void )
{
	for(Uint32 i = 0; i < m_pBoneTableArray.size(); i++)
	{
		smdl::boneinfo_t* pbone = m_pBoneTableArray[i];

		for(Uint32 j = 0; j < 6; j++)
		{
			Float minValue;
			Float maxValue;
			if(j < 3)
			{
				minValue = -128;
				maxValue = 128;
			}
			else
			{
				minValue = -M_PI / 8.0f;
				maxValue = M_PI / 8.0f;
			}

			for(Uint32 k = 0; k < m_pSequencesArray.size(); k++)
			{
				smdl::sequence_t* psequence = m_pSequencesArray[k];
				
				for(Uint32 l = 0; l < psequence->panims.size(); l++)
				{
					smdl::animation_t* panimation = psequence->panims[l];

					for(Uint32 m = 0; m < psequence->numframes; m++)
					{
						Float value;
						if(j < 3)
						{
							// Positional values
							CArray<Vector>* pPositionValues = (panimation->pos_values[m]);
							value = (*pPositionValues)[i][j] - pbone->position[j];
						}
						else
						{
							// Rotational values
							CArray<Vector>* pRotationValues = (panimation->rot_values[m]);
							value = (*pRotationValues)[i][j-3] - pbone->rotation[j-3];
							value = CompilerMath::AngleModRadians(value);
						}

						if(value < minValue)
							minValue = value;

						if(value > maxValue)
							maxValue = value;
					}
				}
			}

			Float scale;
			if(minValue < maxValue)
			{
				if((-minValue) > maxValue)
					scale = minValue / -32768.0;
				else
					scale = maxValue / 32767;
			}
			else
			{
				scale = 1.0 / 32.0;
			}

			if(j < 3)
				pbone->position_scale[j] = scale;
			else
				pbone->rotation_scale[j-3] = scale;
		}
	}
}

//===============================================
// @brief Determine bounding box sizes for each sequence
//
//===============================================
void CStudioModelCompiler::CalculateSequenceBoundingBoxes( void )
{
	// Allocate temporary array
	CArray<smdl::bonematrix_t> boneTransformArray;
	boneTransformArray.resize(m_pBoneTableArray.size());

	// Local transform matrix
	Float boneMatrix[3][4];

	for(Uint32 i = 0; i < m_pSequencesArray.size(); i++)
	{
		smdl::sequence_t* psequence = m_pSequencesArray[i];
		
		Vector& mins = psequence->bboxmins;
		mins = NULL_MINS;

		Vector& maxs = psequence->bboxmaxs;
		maxs = NULL_MAXS;

		for(Uint32 j = 0; j < psequence->panims.size(); j++)
		{
			smdl::animation_t* panimation = psequence->panims[j];

			for(Uint32 k = 0; k < psequence->numframes; k++)
			{
				for(Uint32 l = 0; l < m_pBoneTableArray.size(); l++)
				{
					CArray<Vector>* pRotationValues = (panimation->rot_values[k]);
					CArray<Vector>* pPositionValues = (panimation->pos_values[k]);

					// Convert angle to degrees
					Vector angle;
					for(Uint32 m = 0; m < 3; m++)
						angle[m] = (*pRotationValues)[l][m] * (180.0 / M_PI);

					CompilerMath::AngleMatrix(angle, boneMatrix);
					for(Uint32 m = 0; m < 3; m++)
						boneMatrix[m][3] = (*pPositionValues)[l][m];

					const smdl::boneinfo_t* pbone = m_pBoneTableArray[l];
					if(pbone->parent_index == NO_POSITION)
						Math::CopyMatrix(boneMatrix, boneTransformArray[l].matrix);
					else
						Math::ConcatTransforms(boneTransformArray[pbone->parent_index].matrix, boneMatrix, boneTransformArray[l].matrix);
				}

				for(Uint32 k = 0; k < m_pSubmodelsArray.size(); k++)
				{
					const smdl::submodel_t* psubmodel = m_pSubmodelsArray[k];

					for(Uint32 l = 0; l < psubmodel->numvertexes; l++)
					{
						const smdl::vertex_t& vertex = psubmodel->vertexes[l];
						Int32 boneIndex = vertex.boneindex;

						Vector vertexPosition;
						Math::VectorTransform(vertex.position, boneTransformArray[boneIndex].matrix, vertexPosition);

						for(Uint32 m = 0; m < 3; m++)
						{
							if(vertexPosition[m] < mins[m])
								mins[m] = vertexPosition[m];

							if(vertexPosition[m] > maxs[m])
								maxs[m] = vertexPosition[m];
						}
					}
				}
			}
		}
	}
}

//===============================================
// @brief Compresses animation data using RLE compression
//
//===============================================
void CStudioModelCompiler::CompressAnimationData( void )
{
	CArray<Int16> animationValuesArray;
	CArray<mstudioanimvalue_t> animationDataArray;

	Uint32 nbTotal = 0;
	Uint32 nbChanges = 0;

	for(Uint32 i = 0; i < m_pSequencesArray.size(); i++)
	{
		smdl::sequence_t* psequence = m_pSequencesArray[i];

		for(Uint32 j = 0; j < psequence->panims.size(); j++)
		{
			smdl::animation_t* panimation = psequence->panims[j];

			// Allocate animation counters array
			panimation->numanims.resize(m_pBoneTableArray.size());
			panimation->animdata.resize(m_pBoneTableArray.size());

			for(Uint32 k = 0; k < m_pBoneTableArray.size(); k++)
			{
				const smdl::boneinfo_t* pbone = m_pBoneTableArray[k];

				// Ensure array sizes are adequate
				if(animationValuesArray.size() < psequence->numframes)
					animationValuesArray.resize(psequence->numframes);

				if(animationDataArray.size() < (psequence->numframes*2))
					animationDataArray.resize(psequence->numframes*2);

				for(Uint32 l = 0; l < 6; l++)
				{
					// Fill in the animation data
					for(Uint32 m = 0; m < psequence->numframes; m++)
					{
						if(l < 3)
						{
							CArray<Vector>* pPositionValues = (panimation->pos_values[m]);
							animationValuesArray[m] = ((*pPositionValues)[k][l] - pbone->position[l]) / pbone->position_scale[l];
						}
						else
						{
							CArray<Vector>* pRotationValues = (panimation->rot_values[m]);
							Float value = ((*pRotationValues)[k][l-3] - pbone->rotation[l-3]);
							animationValuesArray[m] = CompilerMath::AngleModRadians(value) / pbone->rotation_scale[l-3];
						}
					}

					// Reset everything
					panimation->numanims[k].counts[l] = 0;

					for(Uint32 m = 0; m < psequence->numframes; m++)
						animationDataArray[m].value = 0;

					mstudioanimvalue_t* pcount = &animationDataArray[0];
					pcount->num.valid = 1;
					pcount->num.total = 1;

					mstudioanimvalue_t* pvalue = pcount + 1;
					pvalue->value = animationValuesArray[0];
					pvalue++;

					for(Uint32 m = 1, p = 0; m < psequence->numframes; m++)
					{
						if(SDL_abs(animationValuesArray[p] - animationValuesArray[m]) > 1600)
						{
							nbChanges++;
							p = m;
						}
					}

					// Fucking hated writing and debugging this
					for(Uint32 m = 1; m < psequence->numframes; m++)
					{
						Float prevValue = animationValuesArray[m-1];
						Float curValue = animationValuesArray[m];

						// Check if we've reached max
						if(pcount->num.total == 255)
						{
							// Advance pointers
							pcount = pvalue;
							pvalue = pcount + 1;
							pcount->num.valid++;

							pvalue->value = curValue;
							pvalue++;
						}
						else if((curValue != prevValue)
							|| ((pcount->num.total == pcount->num.valid) && (m < (psequence->numframes - 1) && curValue != animationValuesArray[m+1])))
						{
							nbTotal++;
							if(pcount->num.total != pcount->num.valid)
							{
								pcount = pvalue;
								pvalue = pcount + 1;
							}

							pcount->num.valid++;
							pvalue->value = curValue;
							pvalue++;
						}

						pcount->num.total++;
					}

					Uint32& animCount = panimation->numanims[k].counts[l];
					animCount = pvalue - &animationDataArray[0];

					if(animCount == 2 && animationValuesArray[0] == 0)
					{
						// Reset it to 0 and don't bother
						animCount = 0;
					}
					else
					{
						panimation->animdata[k].pvalues[l] = new mstudioanimvalue_t[animCount];
						memcpy(panimation->animdata[k].pvalues[l], &animationDataArray[0], sizeof(mstudioanimvalue_t)*animCount);
					}
				}
			}
		}
	}
}

//===============================================
// @brief Set flags for textures
//
// @return TRUE if successful, FALSE if an error was encountered
//===============================================
bool CStudioModelCompiler::SetTextureFlags( void )
{
	for(Uint32 i = 0; i < m_textureRenderModesArray.size(); i++)
	{
		smdl::rendermode_t& rmode = m_textureRenderModesArray[i];

		Uint32 j = 0;
		for(; j < m_pTexturesArray.size(); j++)
		{
			smdl::texture_t* ptexture = m_pTexturesArray[j];
			if(!qstrcicmp(ptexture->name.c_str(), rmode.texturename))
			{
				ptexture->flags |= rmode.texflags;
				break;
			}
		}

		if(j == m_pTexturesArray.size())
		{
			ErrorMsg("Couldn't find texture '%s' specified in '$texrendermode' command.\n", rmode.texturename.c_str());
			return false;
		}
	}

	return true;
}

//===============================================
// @brief Determine hitbox type based on bone name
//
// @param pstrBoneName Name of the bone to check for
// @return Hitgroup type based on mappings
//===============================================
Int32 CStudioModelCompiler::DetermineBoneHitGroupType( const Char* pstrBoneName )
{
	for(Uint32 i = 0; i < m_hitgroupBoneDiscardList.size(); i++)
	{
		CString token = m_hitgroupBoneDiscardList[i];
		if(qstrstr(pstrBoneName, token.c_str()))
		{
			Msg("Bone '%s' discarded from hitbox auto-creation.\n", pstrBoneName);
			return NO_POSITION;
		}
	}

	// First look for full, specific matches in the list
	for(Uint32 i = 0; i < m_boneHitgroupAutoMappingsArray.size(); i++)
	{
		smdl::hitgroup_bone_mapping_t& mapping = m_boneHitgroupAutoMappingsArray[i];
		if(!qstrcicmp(pstrBoneName, mapping.partialname.c_str()))
			return mapping.hitgroupindex;
	}

	// Now look for matching tokens
	for(Uint32 i = 0; i < m_boneHitgroupAutoMappingsArray.size(); i++)
	{
		smdl::hitgroup_bone_mapping_t& mapping = m_boneHitgroupAutoMappingsArray[i];
		if(qstrstr(pstrBoneName, mapping.partialname.c_str()))
			return mapping.hitgroupindex;
	}

	return HITGROUP_GENERIC;
}

//===============================================
// @brief Returns the index of a bone in the bone table array
//
// @param pstrBoneName Name of the bone to find the index of
// @return Index of the bone in the table, or NO_POSITION(-1)
//===============================================
Int32 CStudioModelCompiler::GetBoneIndex( const Char* pstrBoneName )
{
	for(Int32 i = 0; i < m_pBoneTableArray.size(); i++)
	{
		smdl::boneinfo_t*pbone = m_pBoneTableArray[i];
		if(!qstrcicmp(pbone->name, pstrBoneName))
			return i;
	}

	return NO_POSITION;
}

//===============================================
// @brief Initializes the basics of the class
//
// @param pstrModuleFileName Module file path
// @return TRUE if successful, FALSE if failed
//===============================================
bool CStudioModelCompiler::Init( const Char* pstrModuleFileName )
{
	// Set defaults
	SetDefaultValues();

	// Try loading custom definitions from the application path
	CString folderPath;
	Common::GetDirectoryPath(pstrModuleFileName, folderPath);

	CString filePath;
	filePath << folderPath << PATH_SLASH_CHAR << RENDERMODE_DEFINITIONS_FILENAME;

	if(!LoadRenderModeDefinitions(filePath.c_str()))
		return false;

	// Load hitgroup defs
	filePath.clear();
	filePath << folderPath << PATH_SLASH_CHAR << HITGROUP_DEFINITIONS_FILENAME;

	if(!LoadHitGroupDefinitions(filePath.c_str()))
		return false;

	// Load automappings file
	filePath.clear();
	filePath << folderPath << PATH_SLASH_CHAR << BONE_HITGROUP_AUTOMAP_FILENAME;

	if(!LoadBoneHitGroupAutoMappings(filePath.c_str()))
		return false;

	return true;
}

//===============================================
// @brief Loads the custom rendermode definitions from a file
//
// @param pstrFilePath Path to file to load
// @return TRUE if successful, FALSE if failed
//===============================================
bool CStudioModelCompiler::LoadRenderModeDefinitions( const Char* pstrFilePath )
{
	CString filePath = Common::CleanupPath(pstrFilePath);

	Uint32 fileSize = 0;
	const byte* pFile = g_fileInterface.pfnLoadFile(filePath.c_str(), &fileSize);
	if(!pFile)
	{
		WarningMsg("Could not load custom definitions file '%s'.\n", filePath.c_str());
		return true; // Not a failure
	}

	Msg("Loading custom render modes from file '%s'.\n", filePath.c_str());

	// Begin parsing
	Char token[MAX_PARSE_LENGTH];

	// Parse first token, needs to be '{'
	const Char* pstr = Common::Parse(reinterpret_cast<const Char*>(pFile), token);
	if(!pstr)
	{
		ErrorMsg("Short read on file.\n");
		g_fileInterface.pfnFreeFile(pFile);
		return false;
	}

	if(qstrcmp(token, "{"))
	{
		ErrorMsg("Expected '{', got '%s' instead.\n", token);
		g_fileInterface.pfnFreeFile(pFile);
		return false;
	}

	Uint32 nbAdded = 0;
	Uint32 lineNb = 0;
	Char lineStr[MAX_LINE_LENGTH];
	while(true)
	{
		// Parse next line
		pstr = Common::ReadLine(pstr, lineStr);
		if(qstrlen(lineStr) == 0)
		{
			ErrorMsg("Short read on line %d.\n", lineNb);
			g_fileInterface.pfnFreeFile(pFile);
			return false;
		}

		const Char* plinestr = Common::Parse(lineStr, token);
		
		// Skip over empty lines
		if(qstrlen(token) <= 0 && !plinestr)
		{
			lineNb++;
			continue;
		}

		// Stop if we reached the file end
		if(!qstrcmp(token, "}"))
			break;

		// Check for error
		if(!pstr)
		{
			ErrorMsg("Short read on line %d.\n", lineNb);
			g_fileInterface.pfnFreeFile(pFile);
			return false;
		}

		// First token of the line needs to be another '{'
		if(qstrcmp(token, "{"))
		{
			ErrorMsg("Expected '{', got '%s' instead on line %d.\n", token, lineNb);
			g_fileInterface.pfnFreeFile(pFile);
			return false;
		}

		// Now read the name of the rendermode
		plinestr = Common::Parse(plinestr, token);
		if(!plinestr)
		{
			ErrorMsg("Short read on line %d.\n", lineNb);
			g_fileInterface.pfnFreeFile(pFile);
			return false;
		}

		CString renderModeName = token;

		// Now read the value
		plinestr = Common::Parse(plinestr, token);
		if(!plinestr)
		{
			ErrorMsg("Short read on line %d.\n", lineNb);
			g_fileInterface.pfnFreeFile(pFile);
			return false;
		}

		if(!Common::IsNumber(token))
		{
			ErrorMsg("Expected a numerical value, got '%s' instead on line %d.\n", token, lineNb);
			g_fileInterface.pfnFreeFile(pFile);
			return false;
		}

		Int32 flagValue = SDL_atoi(token);

		// Now read the closing bracket
		plinestr = Common::Parse(plinestr, token);
		if(qstrcmp(token, "}"))
		{
			WarningMsg("Expected '{', got '%s' instead on line %d.\n", token, lineNb);
			g_fileInterface.pfnFreeFile(pFile);
			return false;
		}

		Uint32 i = 0;
		for(; i < m_renderModeDefinitionsArray.size(); i++)
		{
			smdl::rendermode_definition_t& def = m_renderModeDefinitionsArray[i];
			if(!qstrcicmp(def.rendermodename, renderModeName))
			{
				WarningMsg("Render mode '%s' already present, discarding.\n", renderModeName.c_str());
				break;
			}
			else if(def.flag == flagValue)
			{
				WarningMsg("Render mode flag value %d already used by '%s', discarding '%s'.\n", flagValue, def.rendermodename.c_str(), renderModeName.c_str());
				break;
			}
		}

		if(i == m_renderModeDefinitionsArray.size())
		{
			smdl::rendermode_definition_t newDef;
			newDef.flag = flagValue;
			newDef.rendermodename = renderModeName;

			m_renderModeDefinitionsArray.push_back(newDef);
			nbAdded++;
		}

		lineNb++;
	}

	Msg("Added %d new render mode definition(s).\n", nbAdded);

	// Release the file contents
	g_fileInterface.pfnFreeFile(pFile);
	return true;
}

//===============================================
// @brief Loads hitgroup definitions file
//
// @param pstrFilePath Path to file to load
// @return TRUE if successful, FALSE if failed
//===============================================
bool CStudioModelCompiler::LoadHitGroupDefinitions( const Char* pstrFilePath )
{
	CString filePath = Common::CleanupPath(pstrFilePath);

	Uint32 fileSize = 0;
	const byte* pFile = g_fileInterface.pfnLoadFile(filePath.c_str(), &fileSize);
	if(!pFile)
	{
		WarningMsg("Could not load custom definitions file '%s'.\n", filePath.c_str());
		return true; // Not a failure
	}

	Msg("Loading custom hitgroups from file '%s'.\n", filePath.c_str());

	// Begin parsing
	Char token[MAX_PARSE_LENGTH];

	// Parse first token, needs to be '{'
	const Char* pstr = Common::Parse(reinterpret_cast<const Char*>(pFile), token);
	if(!pstr)
	{
		ErrorMsg("Short read on file.\n");
		g_fileInterface.pfnFreeFile(pFile);
		return false;
	}

	if(qstrcmp(token, "{"))
	{
		ErrorMsg("Expected '{', got '%s' instead.\n", token);
		g_fileInterface.pfnFreeFile(pFile);
		return false;
	}

	Uint32 nbAdded = 0;
	Uint32 lineNb = 0;
	Char lineStr[MAX_LINE_LENGTH];
	while(true)
	{
		// Parse next line
		pstr = Common::ReadLine(pstr, lineStr);
		if(qstrlen(lineStr) == 0)
		{
			ErrorMsg("Short read on line %d.\n", lineNb);
			g_fileInterface.pfnFreeFile(pFile);
			return false;
		}

		const Char* plinestr = Common::Parse(lineStr, token);
		
		// Skip over empty lines
		if(qstrlen(token) <= 0 && !plinestr)
		{
			lineNb++;
			continue;
		}

		// Stop if we reached the file end
		if(!qstrcmp(token, "}"))
			break;

		// Check for error
		if(!pstr)
		{
			ErrorMsg("Short read on line %d.\n", lineNb);
			g_fileInterface.pfnFreeFile(pFile);
			return false;
		}

		// First token of the line needs to be another '{'
		if(qstrcmp(token, "{"))
		{
			ErrorMsg("Expected '{', got '%s' instead on line %d.\n", token, lineNb);
			g_fileInterface.pfnFreeFile(pFile);
			return false;
		}

		// Now read the ID of the hitgroup
		plinestr = Common::Parse(plinestr, token);
		if(!plinestr)
		{
			ErrorMsg("Short read on line %d.\n", lineNb);
			g_fileInterface.pfnFreeFile(pFile);
			return false;
		}

		if(!Common::IsNumber(token))
		{
			ErrorMsg("Expected a numerical value, got '%s' instead on line %d.\n", token, lineNb);
			g_fileInterface.pfnFreeFile(pFile);
			return false;
		}

		Int32 hitgroupId = SDL_atoi(token);

		// Now read the name of the hitgroup
		plinestr = Common::Parse(plinestr, token);
		if(!plinestr)
		{
			ErrorMsg("Short read on line %d.\n", lineNb);
			g_fileInterface.pfnFreeFile(pFile);
			return false;
		}

		CString hitgroupName = token;

		// Now read the closing bracket
		plinestr = Common::Parse(plinestr, token);
		if(qstrcmp(token, "}"))
		{
			WarningMsg("Expected '{', got '%s' instead on line %d.\n", token, lineNb);
			g_fileInterface.pfnFreeFile(pFile);
			return false;
		}

		if(hitgroupId == HITGROUP_GENERIC)
		{
			WarningMsg("Hitgroup ID 0 is reserved for generic hitgroup, definition with name '%s' discarded.\n", hitgroupName.c_str());
			continue;
		}

		Uint32 i = 0;
		for(; i < m_hitGroupArray.size(); i++)
		{
			smdl::hitgroup_t& def = m_hitGroupArray[i];
			if(def.script)
				continue;

			if(!qstrcicmp(def.name, hitgroupName))
			{
				WarningMsg("Hitgroup '%s' already present, discarding.\n", hitgroupName.c_str());
				break;
			}
			else if(def.hitgroup == hitgroupId)
			{
				WarningMsg("Hitgroup id %d already used by hitgroup '%s', discarding '%s'.\n", hitgroupId, def.name.c_str(), hitgroupName.c_str());
				break;
			}
		}

		if(i == m_hitGroupArray.size())
		{
			smdl::hitgroup_t newDef;
			newDef.hitgroup = hitgroupId;
			newDef.name = hitgroupName;
			newDef.script = true;

			m_hitGroupArray.push_back(newDef);
			nbAdded++;
		}

		lineNb++;
	}

	Msg("Added %d new hitgroup definition(s).\n", nbAdded);

	// Release the file contents
	g_fileInterface.pfnFreeFile(pFile);
	return true;
}

//===============================================
// @brief Loads hitgroup definitions file
//
// @param pstrFilePath Path to file to load
// @return TRUE if successful, FALSE if failed
//===============================================
bool CStudioModelCompiler::LoadBoneHitGroupAutoMappings( const Char* pstrFilePath )
{
	CString filePath = Common::CleanupPath(pstrFilePath);

	Uint32 fileSize = 0;
	const byte* pFile = g_fileInterface.pfnLoadFile(filePath.c_str(), &fileSize);
	if(!pFile)
	{
		WarningMsg("Could not load bone-hitgroup auto mappings file '%s'.\n", filePath.c_str());
		return true; // Not a failure
	}

	Msg("Loading bone-hitgroup mappings from file '%s'.\n", filePath.c_str());

	// Begin parsing
	Char token[MAX_PARSE_LENGTH];

	// Parse first token, needs to be '{'
	const Char* pstr = Common::Parse(reinterpret_cast<const Char*>(pFile), token);
	if(!pstr)
	{
		ErrorMsg("Short read on file.\n");
		g_fileInterface.pfnFreeFile(pFile);
		return false;
	}

	if(qstrcmp(token, "{"))
	{
		ErrorMsg("Expected '{', got '%s' instead.\n", token);
		g_fileInterface.pfnFreeFile(pFile);
		return false;
	}

	Uint32 nbAdded = 0;
	Uint32 nbDiscard = 0;
	Uint32 lineNb = 0;
	Char lineStr[MAX_LINE_LENGTH];
	while(true)
	{
		// Parse next line
		pstr = Common::ReadLine(pstr, lineStr);
		if(qstrlen(lineStr) == 0)
		{
			ErrorMsg("Short read on line %d.\n", lineNb);
			g_fileInterface.pfnFreeFile(pFile);
			return false;
		}

		const Char* plinestr = Common::Parse(lineStr, token);
		
		// Skip over empty lines
		if(qstrlen(token) <= 0 && !plinestr)
		{
			lineNb++;
			continue;
		}

		// Stop if we reached the file end
		if(!qstrcmp(token, "}"))
			break;

		// Check for error
		if(!pstr)
		{
			ErrorMsg("Short read on line %d.\n", lineNb);
			g_fileInterface.pfnFreeFile(pFile);
			return false;
		}

		// First token of the line needs to be another '{'
		if(qstrcmp(token, "{"))
		{
			ErrorMsg("Expected '{', got '%s' instead on line %d.\n", token, lineNb);
			g_fileInterface.pfnFreeFile(pFile);
			return false;
		}

		// Now read the name of the bone
		plinestr = Common::Parse(plinestr, token);
		if(!plinestr)
		{
			ErrorMsg("Short read on line %d.\n", lineNb);
			g_fileInterface.pfnFreeFile(pFile);
			return false;
		}

		CString boneNameToken = token;

		// Now read the name of the hitgroup
		plinestr = Common::Parse(plinestr, token);
		if(!plinestr)
		{
			ErrorMsg("Short read on line %d.\n", lineNb);
			g_fileInterface.pfnFreeFile(pFile);
			return false;
		}

		CString hitgroupName = token;

		// Now read the closing bracket
		plinestr = Common::Parse(plinestr, token);
		if(qstrcmp(token, "}"))
		{
			WarningMsg("Expected '{', got '%s' instead on line %d.\n", token, lineNb);
			g_fileInterface.pfnFreeFile(pFile);
			return false;
		}

		if(!qstrcicmp(hitgroupName, "discard"))
		{
			Uint32 i = 0;
			for(; i < m_hitgroupBoneDiscardList.size(); i++)
			{
				CString entry = m_hitgroupBoneDiscardList[i];
				if(!qstrcicmp(entry, boneNameToken))
				{
					WarningMsg("Bone name token '%s' already in discard list, ignoring.\n", boneNameToken.c_str());
					break;
				}
			}

			if(i == m_hitgroupBoneDiscardList.size())
			{
				m_hitgroupBoneDiscardList.push_back(boneNameToken);
				nbDiscard++;
			}
		}
		else
		{
			Uint32 i = 0;
			for(; i < m_boneHitgroupAutoMappingsArray.size(); i++)
			{
				smdl::hitgroup_bone_mapping_t& def = m_boneHitgroupAutoMappingsArray[i];
				if(!qstrcicmp(def.partialname, boneNameToken))
				{
					WarningMsg("Bone-hitgroup mapping with token '%s' and hitgroup ID %d already present, discarding.\n", def.partialname.c_str(), def.hitgroupindex);
					break;
				}
			}

			// If we found a duplicate, then skip processing of this
			if(i != m_boneHitgroupAutoMappingsArray.size())
				continue;

			i = 0;
			for(; i < m_hitGroupArray.size(); i++)
			{
				smdl::hitgroup_t& def = m_hitGroupArray[i];
				if(!def.script)
					continue;

				if(!qstrcicmp(def.name, hitgroupName))
					break;
			}

			if(i == m_hitGroupArray.size())
			{
				WarningMsg("Couldn't find hitgroup named '%s' for bone token '%s'.\n", hitgroupName.c_str(), boneNameToken.c_str());
			}
			else
			{
				smdl::hitgroup_bone_mapping_t newDef;
				newDef.hitgroupindex = m_hitGroupArray[i].hitgroup;
				newDef.partialname = boneNameToken;

				m_boneHitgroupAutoMappingsArray.push_back(newDef);
				nbAdded++;
			}
		}

		lineNb++;
	}

	Msg("Added %d bone-hitgroup mapping definition(s), %d bone names to discard.\n", nbAdded, nbDiscard);

	// Release the file contents
	g_fileInterface.pfnFreeFile(pFile);
	return true;
}

//===============================================
// @brief Clears up memory used by this class
//
//===============================================
void CStudioModelCompiler::Clear( void )
{
	//
	// Release data from all CArray objects that store pointers
	// to the elements
	//

	if(!m_pBoneTableArray.empty())
	{
		for(Uint32 i = 0; i < m_pBoneTableArray.size(); i++)
			delete m_pBoneTableArray[i];

		m_pBoneTableArray.clear();
	}

	if(!m_pAnimationsArray.empty())
	{
		for(Uint32 i = 0; i < m_pAnimationsArray.size(); i++)
			delete m_pAnimationsArray[i];

		m_pAnimationsArray.clear();
	}

	if(!m_pSequencesArray.empty())
	{
		for(Uint32 i = 0; i < m_pSequencesArray.size(); i++)
			delete m_pSequencesArray[i];

		m_pSequencesArray.clear();
	}

	if(!m_pSubmodelsArray.empty())
	{
		for(Uint32 i = 0; i < m_pSubmodelsArray.size(); i++)
			delete m_pSubmodelsArray[i];

		m_pSubmodelsArray.clear();
	}

	if(!m_pBodyPartsArray.empty())
	{
		for(Uint32 i = 0; i < m_pBodyPartsArray.size(); i++)
			delete m_pBodyPartsArray[i];

		m_pBodyPartsArray.clear();
	}

	if(!m_pTexturesArray.empty())
	{
		for(Uint32 i = 0; i < m_pTexturesArray.size(); i++)
			delete m_pTexturesArray[i];

		m_pTexturesArray.clear();
	}

	if(!m_pLODsArray.empty())
	{
		for(Uint32 i = 0; i < m_pLODsArray.size(); i++)
			delete m_pLODsArray[i];

		m_pLODsArray.clear();
	}

	// Reset everything else to no values
	// This is in case I want a batch compile functionality
	// added in the future

	if(!m_boneRenameMap.empty())
		m_boneRenameMap.clear();

	if(!m_mirroredBonesList.empty())
		m_mirroredBonesList.clear();

	if(!m_noBlendBonesList.empty())
		m_noBlendBonesList.clear();

	if(!m_transitionNodesArray.empty())
		m_transitionNodesArray.clear();

	if(!m_boneControllersArray.empty())
		m_boneControllersArray.clear();

	if(!m_attachmentsArray.empty())
		m_attachmentsArray.clear();

	if(!m_hitBoxesArray.empty())
		m_hitBoxesArray.clear();

	if(!m_hitGroupArray.empty())
		m_hitGroupArray.clear();

	if(!m_textureFoldersArray.empty())
		m_textureFoldersArray.clear();

	if(!m_protectedBonesList.empty())
		m_protectedBonesList.clear();

	if(!m_pivotNamesArray.empty())
		m_pivotNamesArray.clear();

	if(!m_skinRefsArray.empty())
		m_skinRefsArray.clear();

	if(!m_textureGroupsArray.empty())
		m_textureGroupsArray.clear();

	if(!m_renamedTextureMap.empty())
		m_renamedTextureMap.clear();

	if(!m_textureRenderModesArray.empty())
		m_textureRenderModesArray.clear();

	if(!m_movementBoneName.empty())
		m_movementBoneName.clear();

	if(m_pFileOutputBuffer)
	{
		delete m_pFileOutputBuffer;
		m_pFileOutputBuffer = nullptr;
	}

	m_numSkinFamilies = 0;
	m_scaleUpValue = 0;
	m_movementScale = 0;
	m_studioFlags = 0;

	m_adjustmentVector.Clear();
	m_defaultAdjustVector.Clear();
	m_eyePosition.Clear();
	m_bBoxMins.Clear();
	m_bBoxMaxs.Clear();
	m_cBoxMins.Clear();
	m_cBoxMaxs.Clear();

	m_pStudioHeader = nullptr;

	// Do not clear list:
	// These will be set by SetDefaultValues
	/*
	* m_defaultMovementScale
	* m_defaultScale
	* m_defaultZRotation
	* m_normalMergeTreshold
	* m_weightTreshold
	* m_textureGamma
	*/

	// Reset these to base values
	SetDefaultValues();
}

//===============================================
// @brief Set default values for scales, etc
//
//===============================================
void CStudioModelCompiler::SetDefaultValues( void )
{
	m_defaultMovementScale = m_movementScale = 1.0;
	m_defaultScale = 1.0;
	m_defaultZRotation = M_PI / 2.0;
	if(g_options.normal_merge_treshold != 0)
		m_normalMergeTreshold = g_options.normal_merge_treshold;
	else
		m_normalMergeTreshold = DEFAULT_NORMAL_MERGE_TRESHOLD;

	m_weightTreshold = MINIMUM_WEIGHT_TRESHOLD;
	m_textureGamma = DEFAULT_TEXTURE_GAMMA_VALUE;
}

//===============================================
// @brief Add texture folder to the list
//
// @param pstrPath Path to add to the list
//===============================================
void CStudioModelCompiler::AddTextureFolderPath( const Char* pstrPath )
{
	for(Uint32 i = 0; i < m_textureFoldersArray.size(); i++)
	{
		if(!qstrcicmp(m_textureFoldersArray[i], pstrPath))
			return;
	}

	m_textureFoldersArray.push_back(pstrPath);
}

//===============================================
// @brief Sets the scale up value
//
// @param value Value to set
//===============================================
void CStudioModelCompiler::SetScaleUpValue( Float value )
{
	m_scaleUpValue = value;
}

//===============================================
// @brief Sets the default scale value
//
// @param value Value to set
//===============================================
void CStudioModelCompiler::SetDefaultScaleValue( Float value )
{
	m_defaultScale = value;
}

//===============================================
// @brief Add pivot bone name
//
// @param pstrBoneName Name of the bone to set for
//===============================================
void CStudioModelCompiler::AddPivot( const Char* pstrBoneName )
{
	for(Uint32 i = 0; i < m_pivotNamesArray.size(); i++)
	{
		if(!qstrcicmp(m_pivotNamesArray[i], pstrBoneName))
			return;
	}

	m_pivotNamesArray.push_back(pstrBoneName);
}

//===============================================
// @brief Set root pivot bone name
//
// @param pstrBoneName Name of the bone to set for
//===============================================
void CStudioModelCompiler::SetRootPivot( const Char* pstrBoneName )
{
	if(m_pivotNamesArray.empty())
		m_pivotNamesArray.push_back(pstrBoneName);
	else
		m_pivotNamesArray[0] = pstrBoneName;
}

//===============================================
// @brief Sets CBox(How is this different from "bbox"?)
//
// @param mins Mins coordinate values
// @param maxs Maxs coordinate values
//===============================================
void CStudioModelCompiler::SetCBox( const Vector& mins, const Vector& maxs )
{
	Math::VectorScale(mins, m_scaleUpValue, m_cBoxMins);
	Math::VectorScale(maxs, m_scaleUpValue, m_cBoxMaxs);
}

//===============================================
// @brief Sets BBox(How is this different from "cbox"?)
//
// @param mins Mins coordinate values
// @param maxs Maxs coordinate values
//===============================================
void CStudioModelCompiler::SetBBox( const Vector& mins, const Vector& maxs )
{
	Math::VectorScale(mins, m_scaleUpValue, m_bBoxMins);
	Math::VectorScale(maxs, m_scaleUpValue, m_bBoxMaxs);
}

//===============================================
// @brief Sets eye position of the model
//
// @param eyePosition Eye position vector
//===============================================
void CStudioModelCompiler::SetEyePosition( const Vector& eyePosition )
{
	m_eyePosition[1] = eyePosition[0];
	m_eyePosition[0] = -eyePosition[1];
	m_eyePosition[2] = eyePosition[2];
}

//===============================================
// @brief Set default adjustment value
//
// @param adjustmentValue Adjustment value vector
//===============================================
void CStudioModelCompiler::SetDefaultAdjustVector( const Vector& adjustmentVector )
{
	m_defaultAdjustVector = adjustmentVector;
}

//===============================================
// @brief Set default z rotation value
//
// @param value Default Z rotation value
//===============================================
void CStudioModelCompiler::SetDefaultZRotation( Float value )
{
	m_defaultZRotation = (value + 90) * (M_PI / 180.0);
}

//===============================================
// @brief Returns the default Z rotation value
//
// @param value Default Z rotation value
//===============================================
Float CStudioModelCompiler::GetDefaultZRotation( void )
{
	return m_defaultZRotation;
}

//===============================================
// @brief Add a bone to the list of mirrored bones
//
// @param pstrBoneName Name of the bone to mirror
//===============================================
void CStudioModelCompiler::AddMirroredBone( const Char* pstrBoneName )
{
	CStringList_t::iterator it = m_mirroredBonesList.begin();
	while(it != m_mirroredBonesList.end())
	{
		if(!qstrcicmp((*it), pstrBoneName))
		{
			WarningMsg("Bone '%s' already in list of mirrored bones.\n", pstrBoneName);
			return;
		}

		it++;
	}

	m_mirroredBonesList.push_back(pstrBoneName);
}
 
//===============================================
// @brief Tells if this bone is marked to be mirrored
//
// @param pstrBoneName Name of the bone to check for
// @return TRUE if bone is mirrored, FALSE if not
//===============================================
bool CStudioModelCompiler::IsMirroredBone( const Char* pstrBoneName )
{
	CStringList_t::iterator it = m_mirroredBonesList.begin();
	while(it != m_mirroredBonesList.end())
	{
		if(!qstrcicmp((*it), pstrBoneName))
			return true;

		it++;
	}

	return false;
}

//===============================================
// @brief Add a bone to the list of protected bones
//
// @param pstrBoneName Name of the bone to keep protected
//===============================================
void CStudioModelCompiler::AddProtectedBone( const Char* pstrBoneName )
{
	CString str(pstrBoneName);
	CStringList_t::iterator it = m_protectedBonesList.begin();
	while(it != m_protectedBonesList.end())
	{
		if(!qstrcicmp((*it), str))
		{
			WarningMsg("Bone '%s' already in list of protected bones.\n", str.c_str());
			return;
		}

		it++;
	}

	Msg("Bone '%s' marked as protected.\n", pstrBoneName);
	m_protectedBonesList.push_back(str);
}

//===============================================
// @brief Add a bone to the list of no-blend bones
//
// @param pstrBoneName Name of the bone to disable 
// sequence blending for
//===============================================
void CStudioModelCompiler::AddNoBlendBone( const Char* pstrBoneName )
{
	CString str(pstrBoneName);
	CStringList_t::iterator it = m_noBlendBonesList.begin();
	while(it != m_noBlendBonesList.end())
	{
		if(!qstrcicmp((*it), str))
		{
			WarningMsg("Bone '%s' already in list of 'no-blend' bones.\n", str.c_str());
			return;
		}

		it++;
	}

	Msg("Bone '%s' marked not to have animation change blending applied.\n", pstrBoneName);
	m_noBlendBonesList.push_back(str);
}

//===============================================
// @brief Adds a texture rendermode setting
//
// @param pstrTextureName Name of the texture to set for
// @param pstrRenderMode Name of the render mode
// @return TRUE if successful, FALSE if there was an error
//===============================================
bool CStudioModelCompiler::AddTextureRenderMode( const Char* pstrTextureName, const Char* pstrRenderMode )
{
	smdl::rendermode_definition_t* pdef = nullptr;
	for(Uint32 i = 0; i < m_renderModeDefinitionsArray.size(); i++)
	{
		smdl::rendermode_definition_t& rdef = m_renderModeDefinitionsArray[i];
		if(!qstrcicmp(rdef.rendermodename, pstrRenderMode))
		{
			pdef = &rdef;
			break;
		}
	}

	if(pdef == nullptr)
	{
		ErrorMsg("Render mode '%s' not defined.\n", pstrRenderMode);
		return false;
	}

	for(Uint32 i = 0; i < m_textureRenderModesArray.size(); i++)
	{
		smdl::rendermode_t& rmode = m_textureRenderModesArray[i];
		if(!qstrcicmp(rmode.texturename, pstrTextureName))
		{
			if(rmode.texflags & pdef->flag)
			{
				WarningMsg("Render mode '%s' already set for texture '%s'.\n", pdef->rendermodename.c_str(), rmode.texturename.c_str());
			}
			else
			{
				Msg("Render mode '%s' set for texture '%s'.\n", pdef->rendermodename.c_str(), rmode.texturename.c_str());
				rmode.texflags |= pdef->flag;
			}

			return true;
		}
	}

	smdl::rendermode_t newMode;
	newMode.texturename = pstrTextureName;
	newMode.texflags = pdef->flag;
	m_textureRenderModesArray.push_back(newMode);

	Msg("Render mode '%s' set for texture '%s'.\n", pdef->rendermodename.c_str(), pstrTextureName);
	return true;
}

//===============================================
// @brief Set gamma value
//
// @param gamma Gamma value to use
//===============================================
void CStudioModelCompiler::SetGamma( Float gamma )
{
	if(m_textureGamma == gamma)
		return;

	m_textureGamma = gamma;
	Msg("Gamma set to '%.2f'.\n", gamma);
}

//===============================================
// @brief Sets a studio flag value
//
// @param flag Flag to set
//===============================================
void CStudioModelCompiler::SetModelFlag( Int32 flag )
{
	m_studioFlags |= flag;
}
 
//===============================================
// @brief Returns a bone info from the table
//
// @param pstrBoneName Name of the bone
// @return Pointer to bone info structure
//===============================================
const smdl::boneinfo_t* CStudioModelCompiler::GetBone( const Char* pstrBoneName ) const
{
	for(Uint32 i = 0; i < m_pBoneTableArray.size(); i++)
	{
		const smdl::boneinfo_t* pbone = m_pBoneTableArray[i];
		if(!qstrcicmp(pbone->name, pstrBoneName))
			return pbone;
	}

	return nullptr;
}

//===============================================
// @brief Sets the movement bone
//
// @param pstrBoneName Name of the bone to keep protected
// @return TRUE if successful, FALSE if there was an error
//===============================================
bool CStudioModelCompiler::SetMovementBone( const Char* pstrBoneName )
{
	if(!m_movementBoneName.empty())
	{
		ErrorMsg("'$movementbone' set more than once.\n");
		return false;
	}

	m_movementBoneName = pstrBoneName;
	Msg("Bone '%s' forced as movement bone.\n", pstrBoneName);
	return true;
}

//===============================================
// @brief Adds a renamed bone to the mappings
//
// @param pstrOriginalName Original name of the bone to change
// @param pstrNewName New name of the bone to change to
// @return TRUE if successful, FALSE if there was an error
//===============================================
bool CStudioModelCompiler::AddRenamedBone( const Char* pstrOriginalName, const Char* pstrNewName )
{
	CStringMap_t::iterator it = m_boneRenameMap.find(pstrOriginalName);
	if(it != m_boneRenameMap.end())
	{
		ErrorMsg("Bone '%s' already present in bone rename map with target name '%s'.\n", it->first.c_str(), it->second.c_str());
		return false;
	}

	m_boneRenameMap.insert(std::pair<CString, CString>(pstrOriginalName, pstrNewName));
	Msg("Bone '%s' will be renamed to '%s' during processing.\n", pstrOriginalName, pstrNewName);
	return true;
}

//===============================================
// @brief Adds a new attachment
//
// @param pstrBoneName Name of the bone this attachment belongs to
// @param index Index of attachment
// @param origin Offset from bone
// @return TRUE if successful, FALSE if there was an error
//===============================================
bool CStudioModelCompiler::AddAttachment( const Char* pstrBoneName, Int32 index, const Vector& origin )
{
	for(Uint32 i = 0; i < m_attachmentsArray.size(); i++)
	{
		const smdl::attachment_t attachment = m_attachmentsArray[i];
		if(attachment.attachment_index == index)
		{
			ErrorMsg("Attachment with index %d already exists.\n", index);
			return false;
		}
	}

	smdl::attachment_t newAttachment;
	newAttachment.attachment_index = index;
	newAttachment.bonename = pstrBoneName;
	newAttachment.bone_index = NO_POSITION;
	newAttachment.origin = origin;

	m_attachmentsArray.push_back(newAttachment);
	return true;
}

//===============================================
// @brief Adds a hitbox to the list
//
// @param hitgroup Hitgroup ID
// @param pstrBoneName Name of the bone this attachment belongs to
// @param mins BBox mins value
// @param maxs BBox maxs value
// @return TRUE if successful, FALSE if there was an error
//===============================================
bool CStudioModelCompiler::AddHitBox( Int32 hitgroup, const Char* pstrBoneName, const Vector& mins, const Vector& maxs )
{
	smdl::hitbox_t newHb;
	newHb.bonename = pstrBoneName;
	newHb.boneindex = NO_POSITION;
	newHb.hitgroup = hitgroup;
	newHb.mins = mins;
	newHb.maxs = maxs;

	m_hitBoxesArray.push_back(newHb);
	return true;
}
 
//===============================================
// @brief Adds a hitgroup to the list
//
// @param hitgroup Hitgroup ID
// @param pstrGroupName Name of the hit group
// @return TRUE if successful, FALSE if there was an error
//===============================================
bool CStudioModelCompiler::AddHitGroup( Int32 hitgroup, const Char* pstrGroupName )
{
	for(Uint32 i = 0; i < m_hitGroupArray.size(); i++)
	{
		smdl::hitgroup_t& group = m_hitGroupArray[i];
		if(group.script)
			continue;

		if(group.hitgroup == hitgroup)
		{
			ErrorMsg("Hit group with ID %d already exists under the name '%s'.\n", hitgroup, group.name.c_str());
			return false;
		}
		else if(!qstrcicmp(group.name, pstrGroupName))
		{
			ErrorMsg("Hit group with name '%s' already exists with ID.\n", group.name.c_str(), group.hitgroup);
			return false;
		}
	}

	smdl::hitgroup_t newGrp;
	newGrp.hitgroup = hitgroup;
	newGrp.name = pstrGroupName;

	m_hitGroupArray.push_back(newGrp);
	return true;
}
 
//===============================================
// @brief Adds a texture group to the list
//
// @param pstrGroupName Name of the texture group
// @param textureGroupsArray Array of texture group listings
// @return TRUE if successful, FALSE if there was an error
//===============================================
bool CStudioModelCompiler::AddTextureGroup( const Char* pstrGroupName, CArray<CArray<CString>>& textureGroupsArray )
{
	// Make sure the array is valid
	if(textureGroupsArray.size() <= 1)
	{
		ErrorMsg("No alternate groupings in texture group '%s'.\n", pstrGroupName);
		return false;
	}

	// Perform checks for duplicacy
	for(Uint32 i = 0; i < m_textureGroupsArray.size(); i++)
	{
		smdl::texturegroup_t& grp = m_textureGroupsArray[i];
		if(!qstrcicmp(grp.groupname, pstrGroupName))
		{
			ErrorMsg("Texture group '%s' already declared.\n", pstrGroupName);
			return false;
		}

		for(Uint32 j = 0; j < grp.originals.size(); j++)
		{
			for(Uint32 k = 0; k < textureGroupsArray[0].size(); k++)
			{
				if(!qstrcicmp(grp.originals[j].name, textureGroupsArray[0][k]))
				{
					ErrorMsg("While parsing group '%s': Base texture '%s' already referenced in texture group '%s'.\n", pstrGroupName, grp.originals[j].name.c_str(), grp.groupname.c_str());
					return false;
				}
			}
		}
	}

	smdl::texturegroup_t newGrp;
	newGrp.groupname = pstrGroupName;

	// Add base textures
	for(Uint32 i = 0; i < textureGroupsArray[0].size(); i++)
	{
		smdl::grouptexture_t newTexture;
		newTexture.name = textureGroupsArray[0][i];
		newTexture.skinref = NO_POSITION;

		newGrp.originals.push_back(newTexture);
	}

	newGrp.replacements.resize(textureGroupsArray.size()-1);

	// Add all the rest
	for(Uint32 i = 1; i < textureGroupsArray.size(); i++)
	{
		if(textureGroupsArray[0].size() != textureGroupsArray[i].size())
		{
			ErrorMsg("Inconsistent group sizes in texture group '%s'.\n", pstrGroupName);
			return false;
		}

		for(Uint32 j = 0; j < textureGroupsArray[i].size(); j++)
		{
			smdl::grouptexture_t newTexture;
			newTexture.name = textureGroupsArray[i][j];
			newTexture.skinref = NO_POSITION;
			newGrp.replacements[i-1].push_back(newTexture);
		}
	}

	m_textureGroupsArray.push_back(newGrp);
	return true;
}

//===============================================
// @brief Adds a new bone controller
//
// @param controllerIndex Index of the controller
// @param pstrBoneName Name of the bone that this controller influences
// @param typeFlags Marks the type of controller, and if it's wrap-around
// @param startValue Minimum value of the controller
// @param endValue Maximum value of the controller
// @return TRUE if successful, FALSE if there was an error
//===============================================
bool CStudioModelCompiler::AddBoneController( Int32 controllerIndex, const Char* pstrBoneName, Int32 typeFlags, Float startValue, Float endValue )
{
	for(Uint32 i = 0; i < m_boneControllersArray.size(); i++)
	{
		smdl::bonecontroller_t& controller = m_boneControllersArray[i];
		if(controller.controllerindex == controllerIndex)
		{
			ErrorMsg("Controller at index %d already exists for bone '%s'.\n", controller.controllerindex, controller.bonename.c_str());
			return false;
		}
		else if(!qstrcicmp(controller.bonename, pstrBoneName) && controller.type == typeFlags)
		{
			ErrorMsg("Controller for bone '%s' of type '%s' already exists.\n", pstrBoneName, typeFlags);
			return false;
		}
	}

	smdl::bonecontroller_t newController;
	newController.controllerindex = controllerIndex;
	newController.boneindex = NO_POSITION;
	newController.bonename = pstrBoneName;
	newController.type = typeFlags;
	newController.start_value = startValue;
	newController.end_value = endValue;

	m_boneControllersArray.push_back(newController);
	return true;
}

//===============================================
// @brief Adds a new LOD definition
//
// @param pstrSubmodelName Name of target submodel
// @param pstrLODFileName Name of LOD filename
// @param lodType Type of LOD
// @param distance Distance value
// @param reverseTriangles Reverse triangles flag
// @param scale Scale value for the LOD
// @return TRUE if successful, FALSE if there was an error
//===============================================
bool CStudioModelCompiler::AddLOD( const Char* pstrSubmodelName, const Char* pstrLODFileName, vbmlod_type_t lodType, Float distance, bool reverseTriangles, Float scale )
{
	for(Uint32 i = 0; i < m_pLODsArray.size(); i++)
	{
		const smdl::lod_t* plod = m_pLODsArray[i];
		if(!qstrcicmp(plod->submodelname, pstrSubmodelName) && plod->lodtype == lodType)
		{
			if(plod->lodtype == VBM_LOD_DISTANCE)
			{
				if(plod->distance == distance)
				{
					ErrorMsg("Distance LOD for submodel '%s' of type %d and distance %f already defined.\n", pstrSubmodelName, lodType, distance);
					return false;
				}
			}
			else
			{
				ErrorMsg("LOD for submodel '%s' of type %d already defined.\n", pstrSubmodelName, lodType);
				return false;
			}
		}
	}

	smdl::lod_t* pNewLOD = new smdl::lod_t();
	pNewLOD->submodelname = pstrSubmodelName;
	pNewLOD->lodfilename = pstrLODFileName;
	pNewLOD->lodtype = lodType;
	pNewLOD->distance = distance;
	pNewLOD->plodmodel = nullptr;
	pNewLOD->reverseTriangles = reverseTriangles;
	if(scale == -1)
		pNewLOD->scale = m_defaultScale;
	else
		pNewLOD->scale = scale;

	m_pLODsArray.push_back(pNewLOD);
	return true;
}

//===============================================
// @brief Adds a new bodygroup
//
// @param pstrGroupName Name of the bodygroup
// @return New bodygroup pointer
//===============================================
smdl::bodypart_t* CStudioModelCompiler::AddBodyGroup( const Char* pstrGroupName )
{
	for(Uint32 i = 0; i < m_pBodyPartsArray.size(); i++)
	{
		smdl::bodypart_t* pbodypart = m_pBodyPartsArray[i];
		if(!qstrcicmp(pbodypart->name, pstrGroupName))
			WarningMsg("Bodypart with name '%s' already exists at index %d.\n", pstrGroupName, i);
	}

	smdl::bodypart_t* pNew = new smdl::bodypart_t();
	pNew->name = pstrGroupName;

	if(m_pBodyPartsArray.empty())
	{
		pNew->base = 1;
	}
	else
	{
		smdl::bodypart_t* pPrev = m_pBodyPartsArray[m_pBodyPartsArray.size()-1];
		pNew->base = (pPrev->base * pPrev->psubmodels.size());
	}

	m_pBodyPartsArray.push_back(pNew);
	return pNew;
}

//===============================================
// @brief Adds a new submodel
//
// @param pBodyGroup Pointer to bodygroup to add this submodel to
// @param pstrSubmodelName Name of the submodel
// @param pstrVTAName VTA file name
// @param reverseTriangles Reverse triangles flag
// @param scale Scale value for the LOD
// @return TRUE if successful, FALSE otherwise
//===============================================
bool CStudioModelCompiler::AddSubmodel( smdl::bodypart_t* pBodyGroup, const Char* pstrSubmodelName, const Char* pstrVTAName, bool reverseTriangles, Float scale )
{
	if(!qstrcicmp(pstrSubmodelName, "blank"))
	{
		smdl::submodel_t* pNew = new smdl::submodel_t();
		pNew->name = "blank";
		m_pSubmodelsArray.push_back(pNew);
		pBodyGroup->psubmodels.push_back(pNew);
	}
	else
	{
		for(Uint32 i = 0; i < pBodyGroup->psubmodels.size(); i++)
		{
			smdl::submodel_t* psubmodel = pBodyGroup->psubmodels[i];
			if(!qstrcicmp(psubmodel->name, pstrSubmodelName))
				WarningMsg("Submodel with name '%s' already defined for bodygroup '%s' at position %d.\n", pstrSubmodelName, pBodyGroup->name.c_str(), i);
		}

		smdl::submodel_t* pNew = new smdl::submodel_t();
		pNew->name = pstrSubmodelName;
		pNew->vtaname = pstrVTAName;
		pNew->reverseTriangles = reverseTriangles;
		if(scale == -1)
			pNew->scale = m_defaultScale;
		else
			pNew->scale = scale;

		m_pSubmodelsArray.push_back(pNew);
		pBodyGroup->psubmodels.push_back(pNew);

		if(pstrVTAName && qstrlen(pstrVTAName) > 0)
			Msg("Added submodel '%s' with VTA '%s' to bodygroup '%s'.\n", pstrSubmodelName, pstrVTAName, pBodyGroup->name.c_str());
		else
			Msg("Added submodel '%s' to bodygroup '%s'.\n", pstrSubmodelName, pBodyGroup->name.c_str());
	}

	return true;
}

//===============================================
// @brief Adds a new sequence and returns it's pointer
//
// @param pstrGroupName Name of the sequence
// @return New sequence pointer
//===============================================
smdl::sequence_t* CStudioModelCompiler::AddSequence( const Char* pstrSequenceName )
{
	for(Uint32 i = 0; i < m_pSequencesArray.size(); i++)
	{
		smdl::sequence_t* pseq = m_pSequencesArray[i];
		if(!qstrcicmp(pseq->name, pstrSequenceName))
		{
			WarningMsg("Duplicate sequence name '%s' found at position %d.\n", pstrSequenceName, i);
			break;
		}
	}

	smdl::sequence_t* pNewSeq = new smdl::sequence_t();
	pNewSeq->name = pstrSequenceName;
	pNewSeq->fps = DEFAULT_SEQUENCE_FPS; 
	pNewSeq->blendend[0] = 1.0;

	// Set defaults for the adjustable values
	pNewSeq->scaleup = m_defaultScale;
	pNewSeq->movementscale = m_defaultMovementScale;
	pNewSeq->adjust = m_defaultAdjustVector;
	pNewSeq->endframe = -1;

	m_pSequencesArray.push_back(pNewSeq);
	return pNewSeq;
}

//===============================================
// @brief Adds a renamed texture
//
// @param pstrOriginalName Original texture name
// @param pstrNewName New name for texture
//===============================================
void CStudioModelCompiler::AddRenamedTexture( const Char* pstrOriginalName, const Char* pstrNewName )
{
	CStringMap_t::iterator it = m_renamedTextureMap.find(pstrOriginalName);
	if(it != m_renamedTextureMap.end())
	{
		WarningMsg("Texture '%s' already marked to be renamed.\n", pstrOriginalName);
		return;
	}

	m_renamedTextureMap.insert(std::pair<CString, CString>(pstrOriginalName, pstrNewName));
}

//===============================================
// @brief Get rename for an original texture name
//
// @param pstrOriginalName Original texture name
// @param pstrNewName New name for texture
// @return TRUE if a result was found, FALSE if not
//===============================================
bool CStudioModelCompiler::GetTextureRename( const Char* pstrOriginalName, const Char*& pstrNewName )
{
	CStringMap_t::iterator it = g_options.renamedtexturemap.find(pstrOriginalName);
	if(it != g_options.renamedtexturemap.end())
	{
		pstrNewName = it->second.c_str();
		return true;
	}
	else
	{
		CStringMap_t::iterator it = m_renamedTextureMap.find(pstrOriginalName);
		if(it != m_renamedTextureMap.end())
		{
			pstrNewName = it->second.c_str();
			return true;
		}
		else
		{
			pstrNewName = nullptr;
			return false;
		}
	}
}

//===============================================
// @brief Return a skinref for an existing texture, 
// or add a new one and return that
//
// @param pstrTexture Texture name
// @return Pointer to texture entry
//===============================================
smdl::texture_t* CStudioModelCompiler::GetTextureForName( const Char* pstrTexture, bool noCreate )
{
	for(Uint32 i = 0; i < m_pTexturesArray.size(); i++)
	{
		smdl::texture_t* ptexture = m_pTexturesArray[i];
		if(!qstrcicmp(pstrTexture, ptexture->name))
			return ptexture;
	}

	if(noCreate)
	{
		return nullptr;
	}
	else
	{
		smdl::texture_t* pnewtexture = new smdl::texture_t();
		pnewtexture->name = pstrTexture;
		pnewtexture->skinref = m_pTexturesArray.size();

		// Stupid HL legacy stuff
		if(pnewtexture->name.find(0, "chrome") != CString::CSTRING_NO_POSITION)
			pnewtexture->flags |= STUDIO_NF_CHROME;

		m_pTexturesArray.push_back(pnewtexture);
		return pnewtexture;
	}
}

//===============================================
// @brief Applies offsets to a vector
//
// @param input Input vector
// @return Result vector
//===============================================
Vector CStudioModelCompiler::ApplyOffset( const Vector& input )
{
	Vector result;
	Math::VectorSubtract(input, m_adjustmentVector, result);
	return result;
}

//===============================================
// @brief Applies scaling on a coordinate
//
// @param input Input vector
// @return Result vector
//===============================================
Vector CStudioModelCompiler::ApplyScaling( const Vector& input )
{
	Vector result;
	Math::VectorScale(input, m_scaleUpValue, result);
	return result;
}
 
//===============================================
// @brief Clips values on a rotation vector
//
// @param input Input vector
// @return Result vector
//===============================================
Vector CStudioModelCompiler::ClipRotations( const Vector& input )
{
	Vector result;
	for(Uint32 i = 0; i < 3; i++)
		result[i] = CompilerMath::AngleModRadians(input[i]);

	return result;
}