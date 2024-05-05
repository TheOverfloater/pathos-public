/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "com_math.h"
#include "file.h"
#include "texturemanager.h"
#include "brushmodel.h"
#include "modelcache.h"
#include "pbspv1file.h"
#include "bspv30.h"
#include "pbspv1.h"
#include "system.h"
#include "texturemanager.h"
#include "studio.h"
#include "sprite.h"
#include "r_vbm.h"
#include "enginestate.h"
#include "commands.h"

// Object declaration
CModelCache gModelCache;

//=============================================
// @brief Dumps all models in-cache
//
//=============================================
void Mod_DumpCacheModels( void )
{
	Con_Printf("Model cache contents:\n");

	Uint32 nbCache = gModelCache.GetNbCachedModels();
	for(Uint32 i = 0; i < nbCache; i++)
	{
		cache_model_t* pmodel = gModelCache.GetModelByIndex(i+1);
		if(!pmodel)
			break;

		Con_Printf("%d - %s.\n", i, pmodel->name.c_str());
	}
}

//=============================================
// @brief Default constructor
//
//=============================================
CModelCache::CModelCache( void )
{
}

//=============================================
// @brief Destructor
//
//=============================================
CModelCache::~CModelCache( void )
{
	ClearCache();
}

//=============================================
// @brief
//
//=============================================
void CModelCache::Init( void )
{
	gCommands.CreateCommand("dumpmodelcache", Mod_DumpCacheModels, "Dumps all models currently loaded in the cache");
}

//=============================================
// @brief
//
//=============================================
void CModelCache::ClearGL( void )
{
	for(Uint32 i = 0; i < m_modelCacheArray.size(); i++)
	{
		cache_model_t* pmodel = m_modelCacheArray[i];
		if(pmodel->type == MOD_VBM || pmodel->type == MOD_SPRITE)
			pmodel->isloaded = false;
	}
}

//=============================================
// @brief
//
//=============================================
void CModelCache::ClearCache( void )
{
	if(m_modelCacheArray.empty())
		return;

	for(Uint32 i = 0; i < m_modelCacheArray.size(); i++)
	{
		cache_model_t* pmodel = m_modelCacheArray[i];

		if(pmodel->pcachedata)
		{
			switch(pmodel->type)
			{
			case MOD_BRUSH:
				{
					brushmodel_t* pdata = pmodel->getBrushmodel();
					delete pdata;
				}
				break;
			case MOD_VBM:
				{
					vbmcache_t* pcache = pmodel->getVBMCache();
					delete pcache;
				}
				break;
			case MOD_SPRITE:
				{
					msprite_t* psprite = pmodel->getSprite();
					delete psprite;
				}
				break;
			default:
				Con_Printf("%s - Unknown model type %d for '%s'.\n", __FUNCTION__, (Int32)pmodel->type, pmodel->name.c_str());
				break;
			}
		}
			
		delete m_modelCacheArray[i];
	}

	m_modelCacheArray.clear();
}

//=============================================
// @brief
//
//=============================================
cache_model_t* CModelCache::LoadModel( const Char* pstrFilename )
{
	// Try to find it in the cache first
	cache_model_t* pmodel = FindModelByName(pstrFilename);
	if(pmodel)
		return pmodel;

	Uint32 filesize = 0;
	const byte* pfile = FL_LoadFile(pstrFilename, &filesize);
	if(!pfile)
		return nullptr;

	// Determine file type
	Int32 fileId = Common::ByteToInt32(pfile);
	switch(fileId)
	{
	case IDSTUDIOHEADER:
		pmodel = LoadVBMModel(pstrFilename, pfile, filesize);
		break;
	case IDSPRITEHEADER:
		pmodel = LoadSpriteModel(pstrFilename, pfile, filesize);
		break;
	default:
		// Loads BSP files by default
		pmodel = LoadBSPModel(pstrFilename, pfile);
		break;
	}

	FL_FreeFile(pfile);
	return pmodel;
}
//=============================================
// @brief
//
//=============================================
cache_model_t* CModelCache::LoadSpriteModel( const Char* pstrFilename, const byte* pfile, Uint32 filesize )
{
	msprite_t* psprite = Sprite_Load(pfile, filesize);
	if(!psprite)
	{
		Con_EPrintf("%s - Failure while loading '%'.\n", __FUNCTION__, pstrFilename);
		return nullptr;
	}

	// Create a new model entry
	Uint32 modelindex = m_modelCacheArray.size();
	cache_model_t* pnew = new cache_model_t;
	m_modelCacheArray.push_back(pnew);

	// Fill data
	pnew->cacheindex = modelindex+1;
	pnew->mins = Vector(-psprite->radius, -psprite->radius, -psprite->radius);
	pnew->maxs = Vector(psprite->radius, psprite->radius, psprite->radius);
	pnew->radius = psprite->radius;
	pnew->pcachedata = psprite;
	pnew->type = MOD_SPRITE;
	pnew->name = pstrFilename;

	// needs to be loaded to gpu
	pnew->isloaded = false;

	return pnew;
}

//=============================================
// @brief
//
//=============================================
cache_model_t* CModelCache::LoadVBMModel( const Char* pstrFilename, const byte* pfile, Uint32 filesize )
{
	// Get studio header
	const studiohdr_t* pstudiohdr = reinterpret_cast<const studiohdr_t*>(pfile);
	if(pstudiohdr->id != IDSTUDIOHEADER)
	{
		Con_EPrintf("%s - '%s' is not a valid studio model.\n", __FUNCTION__, pstrFilename);
		return nullptr;
	}

	if(pstudiohdr->version != STUDIO_VERSION)
	{
		Con_EPrintf("%s - '%s' has wrong version(%d).\n", __FUNCTION__, pstrFilename, pstudiohdr->version);
		return nullptr;
	}

	// Now load the VBM file
	CString filepath = pstrFilename;
	Uint32 begin = filepath.find(0, ".mdl");
	if(begin != -1)
		filepath.erase(begin, 4);
	filepath << ".vbm";

	Uint32 vbmfilesize = 0;
	const byte* pvbmfile = FL_LoadFile(filepath.c_str(), &vbmfilesize);
	if(!pvbmfile)
	{
		Con_EPrintf("%s - Failed to load file '%s'.\n", __FUNCTION__, filepath.c_str());
		return nullptr;
	}

	// Check header
	const vbmheader_t* pvbmheader = reinterpret_cast<const vbmheader_t*>(pvbmfile);
	if(pvbmheader->id != VBM_HEADER)
	{
		Con_EPrintf("%s - '%s' is not a valid vbm file.\n", __FUNCTION__, filepath.c_str());
		FL_FreeFile(pvbmfile);
		return nullptr;
	}

	// Copy the data
	Uint32 studiodatasize = 0;
	if(pstudiohdr->texturedataindex)
		studiodatasize = pstudiohdr->texturedataindex;
	else
		studiodatasize = filesize;

	byte* pstudiodata = new byte[studiodatasize];
	memcpy(pstudiodata, pfile, sizeof(byte)*studiodatasize);

	// Copy VBM data
	byte* pvbmdata = new byte[vbmfilesize];
	memcpy(pvbmdata, pvbmfile, sizeof(byte)*vbmfilesize);
	FL_FreeFile(pvbmfile);

	// Create studio cache object
	vbmcache_t* pcache = new vbmcache_t();
	pcache->pstudiohdr = reinterpret_cast<studiohdr_t *>(pstudiodata);
	pcache->pvbmhdr = reinterpret_cast<vbmheader_t *>(pvbmdata);

	// Create a new model entry
	Uint32 modelindex = m_modelCacheArray.size();
	cache_model_t* pnew = new cache_model_t;
	m_modelCacheArray.push_back(pnew);

	Vector mins;
	Vector maxs;
	
	for(Int32 i = 0; i < pcache->pstudiohdr->numseq; i++)
	{
		const mstudioseqdesc_t* psequence = pstudiohdr->getSequence(i);
		for(Uint32 j = 0; j < 3; j++)
		{
			if(psequence->bbmax[j] > maxs[j])
				maxs[j] = psequence->bbmax[j];

			if(psequence->bbmin[j] < mins[j])
				mins[j] = psequence->bbmin[j];
		}
	}

	// Fill data
	pnew->cacheindex = modelindex+1;
	pnew->mins = mins;
	pnew->maxs = maxs;
	pnew->pcachedata = pcache;
	pnew->type = MOD_VBM;
	pnew->name = pstrFilename;
	pnew->flags = pstudiohdr->flags;

	// needs to be loaded to gpu
	pnew->isloaded = false;

	// Determine radius
	pnew->radius = 0;
	for(Uint32 i = 0; i < 3; i++)
	{
		if(SDL_fabs(pstudiohdr->bbmin[i]) > pnew->radius)
			pnew->radius = pstudiohdr->bbmin[i];

		if(SDL_fabs(pstudiohdr->bbmax[i]) > pnew->radius)
			pnew->radius = pstudiohdr->bbmax[i];
	}

	return pnew;
}

//=============================================
// @brief
//
//=============================================
cache_model_t* CModelCache::LoadBSPModel( const Char* pstrFilename, const byte* pfile )
{
	brushmodel_t* pmodel = nullptr;

	// First, check if it's a Pathos BSP
	const dpbspv1header_t* pbspheader = reinterpret_cast<const dpbspv1header_t*>(pfile);
	if(pbspheader->id == PBSP_HEADER)
	{
		switch(pbspheader->version)
		{
		case PBSP_VERSION:
			pmodel = PBSPV1_Load(pfile, pbspheader, pstrFilename);
			break;
		default:
			Con_EPrintf("%s - PBSP file '%s' has wrong version number '%d', which should be '%d'.\n", __FUNCTION__, pstrFilename, pbspheader->version, PBSP_VERSION);
			return nullptr;
			break;
		}
	}
	else
	{
		// Get header info
		const dheader_t* pheader = reinterpret_cast<const dheader_t*>(pfile);
	
		// Determine bsp version to load
		switch(pheader->version)
		{
		case BSPV30_VERSION:
			pmodel = BSPV30_Load(pfile, pheader, pstrFilename);
			break;
		default:
			Con_EPrintf("%s - BSP file '%s' has wrong version number '%d', which should be '%d'.\n", __FUNCTION__, pstrFilename, pheader->version, BSPV30_VERSION);
			return nullptr;
			break;
		}
	}

	// See if the load failed
	if(!pmodel)
		return nullptr;

	// Setup the submodels too
	SetupBSPSubmodels(*pmodel, pstrFilename);

	// Free up the temp model
	pmodel->freedata = false;
	delete pmodel;

	// Set up PAS after loading submodel data
	cache_model_t* pcache = m_modelCacheArray[0];
	pmodel = pcache->getBrushmodel();
	BSPV30_SetupPAS((*pmodel));

	return pcache;
}

//=============================================
// @brief
//
//=============================================
void CModelCache::SetupBSPSubmodels( brushmodel_t& model, const Char* loadName )
{
	// Skip first model because it's the world
	for(Uint32 i = 0; i < model.numsubmodels; i++)
	{
		mmodel_t* psubmodel = &model.psubmodels[i];

		// Allocate the new model
		brushmodel_t* pnewmodel = new brushmodel_t();

		// Cheap: Copy data from the base model
		pnewmodel->psubmodels = model.psubmodels;
		pnewmodel->numsubmodels = model.numsubmodels;
		pnewmodel->psurfaces = model.psurfaces;
		pnewmodel->numsurfaces = model.numsurfaces;
		pnewmodel->pvisdata = model.pvisdata;
		pnewmodel->visdatasize = model.visdatasize;
		pnewmodel->ppasdata = model.ppasdata;
		pnewmodel->pasdatasize = model.pasdatasize;
		pnewmodel->plightdata = model.plightdata;
		pnewmodel->lightdatasize = model.lightdatasize;
		pnewmodel->pclipnodes = model.pclipnodes;
		pnewmodel->numclipnodes = model.numclipnodes;
		pnewmodel->pedges = model.pedges;
		pnewmodel->numedges = model.numedges;
		pnewmodel->psurfedges = model.psurfedges;
		pnewmodel->numsurfedges = model.numsurfedges;
		pnewmodel->pnodes = model.pnodes;
		pnewmodel->numnodes = model.numnodes;
		pnewmodel->pleafs = model.pleafs;
		pnewmodel->numleafs = model.numleafs;
		pnewmodel->ptexinfos = model.ptexinfos;
		pnewmodel->numtexinfos = model.numtexinfos;
		pnewmodel->ptextures = model.ptextures;
		pnewmodel->numtextures = model.numtextures;
		pnewmodel->pvertexes = model.pvertexes;
		pnewmodel->numvertexes = model.numvertexes;
		pnewmodel->pplanes = model.pplanes;
		pnewmodel->numplanes = model.numplanes;
		pnewmodel->pmarksurfaces = model.pmarksurfaces;
		pnewmodel->nummarksurfaces = model.nummarksurfaces;
		pnewmodel->pentdata = model.pentdata;
		pnewmodel->entdatasize = model.entdatasize;

		memcpy(pnewmodel->hulls, model.hulls, sizeof(hull_t)*MAX_MAP_HULLS);
		pnewmodel->freedata = (i == 0) ? true : false;

		pnewmodel->hulls[0].firstclipnode = psubmodel->headnode[0];
		for(Uint32 j = 1; j < MAX_MAP_HULLS; j++)
		{
			pnewmodel->hulls[j].firstclipnode = psubmodel->headnode[j];
			pnewmodel->hulls[j].lastclipnode = model.numclipnodes-1;
		}

		pnewmodel->firstmodelsurface = psubmodel->firstface;
		pnewmodel->nummodelsurfaces = psubmodel->numfaces;

		Math::VectorCopy(psubmodel->mins, pnewmodel->mins);
		Math::VectorCopy(psubmodel->maxs, pnewmodel->maxs);

		// Calculate radius
		pnewmodel->radius = 0;
		for(Uint32 j = 0; j < 3; j++)
		{
			if(SDL_fabs(pnewmodel->mins[j]) > pnewmodel->radius)
				pnewmodel->radius = SDL_fabs(pnewmodel->mins[j]);

			if(pnewmodel->maxs[j] > pnewmodel->radius)
				pnewmodel->radius = pnewmodel->maxs[j];
		}

		pnewmodel->numleafs = psubmodel->visleafs;

		// Add it to the cache
		CString modelname;
		if(i == 0)
			modelname = loadName;
		else
			modelname << '*' << (Int32)(i);

		// Set in brushmodel_t too
		pnewmodel->name = modelname;

		// Load was successful, so add it to the cache
		Uint32 modelindex = m_modelCacheArray.size();
		cache_model_t* pnew = new cache_model_t;
		m_modelCacheArray.push_back(pnew);

		pnew->name = modelname;
		pnew->pcachedata = pnewmodel;
		pnew->type = MOD_BRUSH;
		pnew->cacheindex = modelindex+1;
		pnew->mins = pnewmodel->mins;
		pnew->maxs = pnewmodel->maxs;
		pnew->radius = pnewmodel->radius;
		pnew->isloaded = true;
	}
}

//=============================================
// @brief
//
//=============================================
cache_model_t* CModelCache::FindModelByName( const Char* pstrFilename )
{
	if(m_modelCacheArray.empty())
		return nullptr;

	for(Uint32 i = 0; i < m_modelCacheArray.size(); i++)
	{
		if(!qstrcmp(m_modelCacheArray[i]->name, pstrFilename))
			return m_modelCacheArray[i];
	}

	return nullptr;
}

//=============================================
// @brief
//
//=============================================
cache_model_t* CModelCache::GetModelByIndex( Uint32 index )
{
	// Verify if the index is correct
	Int32 realIndex = (Uint32)(index - 1);
	if(realIndex >= (Int32)m_modelCacheArray.size() || realIndex < 0)
	{
		Con_DPrintf("CModelCache::GetModelByIndex - Invalid index: %d.\n", index);
		return nullptr;
	}

	return m_modelCacheArray[realIndex];
}

//=============================================
// @brief
//
//=============================================
cmodel_type_t CModelCache::GetType( Uint32 index )
{
	cache_model_t* pmodel = GetModelByIndex(index);
	if(!pmodel)
		return MOD_NONE;

	return pmodel->type;
}

//=============================================
// @brief
//
//=============================================
void CModelCache::GatherModelResources( const Char* pstrFilename, CArray<maptexturematerial_t>& mapTextureLinks, CArray<CString>& outMaterialsArray, CArray<CString>& outTexturesArray )
{
	// Try to find it in the cache first
	cache_model_t* pcache = FindModelByName(pstrFilename);
	if(!pcache)
	{
		Con_Printf("%s - Model '%s' not found.\n", __FUNCTION__, pstrFilename);
		return;
	}

	CTextureManager* pTextureManager = CTextureManager::GetInstance();

	if(pcache->type == MOD_BRUSH)
	{
		brushmodel_t* pmodel = pcache->getBrushmodel();
		for(Uint32 i = 0; i < pmodel->numtextures; i++)
		{
			mtexture_t* ptexture = &pmodel->ptextures[i];
			
			// Find the material file linked to this map texture
			CString materialfilepath;
			for(Uint32 j = 0; j < mapTextureLinks.size(); j++)
			{
				if(!qstrcmp(ptexture->name, mapTextureLinks[j].maptexturename))
				{
					materialfilepath = mapTextureLinks[j].materialfilepath;
					break;
				}
			}

			if(materialfilepath.empty())
				continue;

			en_material_t* pmaterialscript = pTextureManager->LoadMaterialScript(materialfilepath.c_str(), RS_GAME_LEVEL);
			if(!pmaterialscript)
				continue;
			
			// Gather resources for this material script
			GatherMaterialResources(pmaterialscript, outMaterialsArray, outTexturesArray);
		}
	}
	else if(pcache->type == MOD_VBM)
	{
		vbmcache_t* pstudiocache = pcache->getVBMCache();
		if(!pstudiocache->pvbmhdr)
			return;

		CString modelname;
		Common::Basename(pstrFilename, modelname);

		for(Int32 i = 0; i < pstudiocache->pvbmhdr->numtextures; i++)
		{
			const vbmtexture_t* pvbmtexture = pstudiocache->pvbmhdr->getTexture(i);

			CString textureName;
			Common::Basename(pvbmtexture->name, textureName);

			CString materialscriptpath;
			materialscriptpath << MODEL_MATERIALS_BASE_PATH << modelname << PATH_SLASH_CHAR << textureName.c_str() << PMF_FORMAT_EXTENSION;
			en_material_t* pmaterialscript = pTextureManager->LoadMaterialScript(materialscriptpath.c_str(), RS_GAME_LEVEL);
			if(!pmaterialscript)
				continue;

			// Gather resources for this material script
			GatherMaterialResources(pmaterialscript, outMaterialsArray, outTexturesArray);
		}
	}
	else
	{
		Con_Printf("%s - Unknown type %d for model '%s'.\n", __FUNCTION__, pstrFilename, pcache->type);
		return;
	}
}

//=============================================
// @brief
//
//=============================================
void CModelCache::GatherMaterialResources( en_material_t* pmaterialscript, CArray<CString>& outMaterialsArray, CArray<CString>& outTexturesArray )
{
	if(!outMaterialsArray.empty())
	{
		Uint32 j = 0;
		for(; j < outMaterialsArray.size(); j++)
		{
			if(!qstrcmp(outMaterialsArray[j], pmaterialscript->filepath))
				break;
		}

		// Check if already present
		if(j != outMaterialsArray.size())
			return;
	}

	// Add to the list
	outMaterialsArray.push_back(pmaterialscript->filepath);

	// Now add each texture
	for(Uint32 j = 0; j < NB_MT_TX; j++)
	{
		if(!pmaterialscript->containername.empty() && j == MT_TX_DIFFUSE)
			continue;
		
		en_texture_t* ptexture = pmaterialscript->ptextures[j];
		if(!ptexture)
			continue;

		if(!outTexturesArray.empty())
		{
			Uint32 k = 0;
			for(; k < outTexturesArray.size(); k++)
			{
				if(!qstrcmp(outTexturesArray[k], ptexture->filepath))
					break;
			}

			// Check if already present
			if(k != outTexturesArray.size())
				continue;
		}
					
		outTexturesArray.push_back(ptexture->filepath);
	}
}

//=============================================
//
//=============================================
void Cache_GetModelBounds( const cache_model_t& model, Vector& mins, Vector& maxs )
{
	if(model.type == MOD_BRUSH)
	{
		const brushmodel_t* pmodel = model.getBrushmodel();
		Math::VectorCopy(pmodel->mins, mins);
		Math::VectorCopy(pmodel->maxs, maxs);
	}
	else
	{
		Con_EPrintf("%s - Bogus model type %d.\n", __FUNCTION__, model.type);
		Math::VectorClear(mins);
		Math::VectorClear(maxs);
	}
}

//=============================================
//
//=============================================
cmodel_type_t Cache_GetModelType( const cache_model_t& model )
{
	return model.type;
}

//=============================================
//
//=============================================
const cache_model_t* Cache_GetModel( Int32 modelindex )
{
	if(!modelindex)
		return nullptr;

	const cache_model_t* pmodel = gModelCache.GetModelByIndex(modelindex);
	return pmodel;
}

//=============================================
//
//=============================================
Uint64 Cache_GetModelFrameCount( const cache_model_t& model )
{
	Uint64 count = 0;
	switch(model.type)
	{
	case MOD_SPRITE:
		{
			const msprite_t *psprite = model.getSprite();
			count = psprite->frames.size();
		}
		break;
	case MOD_VBM:
		{
			const vbmcache_t* pvbmcache = model.getVBMCache();
			vbmheader_t* pvbmheader = pvbmcache->pvbmhdr;

			count = 1;
			
			for(Int32 i = 0; i < pvbmheader->numbodyparts; i++)
			{
				vbmbodypart_t* pbodypart = pvbmheader->getBodyPart(i);
				count *= pbodypart->numsubmodels;
			}
		}
		break;
	}

	if(!count)
		count = 1;

	return count;
}

//=============================================
//
//=============================================
Uint32 Cache_GetNbModels( void )
{
	return gModelCache.GetNbCachedModels();
}

//=============================================
//
//=============================================
const cache_model_t* Cache_GetModelByName( const Char* pstrModelName )
{
	return gModelCache.FindModelByName(pstrModelName);
}