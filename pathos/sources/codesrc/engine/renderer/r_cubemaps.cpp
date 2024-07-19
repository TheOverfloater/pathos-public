/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "entitydata.h"
#include "cache_model.h"
#include "modelcache.h"
#include "cl_main.h"
#include "r_main.h"
#include "r_cubemaps.h"
#include "commands.h"
#include "texturemanager.h"
#include "system.h"
#include "cl_utils.h"
#include "file.h"
#include "r_glextf.h"
#include "tga.h"
#include "enginestate.h"
#include "enginefuncs.h"
#include "r_fbocache.h"

// Console commands
void Cmd_BuildCubemaps( void ) { gCubemaps.BuildCubemaps(); }

// Cubemap resolutions available
Uint32 CCubemapManager::g_cubemapResolutions[NUM_CUBEMAP_SIZES][2] =
{ 
	{ 16, 16 },
	{ 32, 32 },
	{ 64, 64 },
	{ 128, 128 },
	{ 256, 256 },
	{ 512, 512 }
};

// Cubemap interpolation time
const Float CCubemapManager::CUBEMAP_INTERP_TIME = 0.5;

// Class definition
CCubemapManager gCubemaps;

//=============================================
// @brief
//
//=============================================
CCubemapManager::CCubemapManager( void ) :
	m_pIdealCubemap(nullptr),
	m_pPrevCubemap(nullptr),
	m_flInterpolant(0),
	m_flLastChangeTime(0)
{
}

//=============================================
// @brief
//
//=============================================
CCubemapManager::~CCubemapManager()
{
}

//=============================================
// @brief
//
//=============================================
bool CCubemapManager::Init( void )
{
	gCommands.CreateCommand("r_buildcubemaps", Cmd_BuildCubemaps, "Generates cubemaps for a map.");
	return true;
}

//=============================================
// @brief
//
//=============================================
void CCubemapManager::Shutdown( void )
{
	ClearGame();
}

//=============================================
// @brief
//
//=============================================
bool CCubemapManager::InitGL( void )
{
	if(CL_IsGameActive())
	{
		// Just call this to re-load cubemaps
		if(!InitGame())
			return false;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CCubemapManager::ClearGL( void )
{
	Cleanup(false);
}

//=============================================
// @brief
//
//=============================================
bool CCubemapManager::InitGame( void )
{
	// Build file path
	CString filename = Engine_GetLevelName();
	Uint32 begin = filename.find(0, ".bsp");
	if(begin != -1)
		filename.erase(begin, 4);
	filename << ".ecd";

	if(!m_cubemapsArray.empty())
		Cleanup(false);

	const byte* pf = FL_LoadFile(filename.c_str(), nullptr);
	if(!pf)
		return true;

	// Load the file
	const ecdheader_t* pheader = reinterpret_cast<const ecdheader_t*>(pf);
	if(pheader->id != ECD_HEADER_ENCODED)
	{
		Con_EPrintf("%s - %s is not a valid ECD file.\n", __FUNCTION__, filename.c_str());
		FL_FreeFile(pf);
		return true; // don't fail fatally
	}

	if(rns.daystage == DAYSTAGE_NIGHTSTAGE && !(pheader->flags & FL_ECD_HAS_NIGHTDATA)
		|| !rns.daystage != DAYSTAGE_NIGHTSTAGE && !(pheader->flags & FL_ECD_HAS_DAYDATA))
	{
		FL_FreeFile(pf);
		return true; // don't fail fatally
	}

	// allocate cubemaps
	m_cubemapsArray.resize(pheader->numcubemaps);

	CTextureManager* pTextureManager = CTextureManager::GetInstance();

	// Fill in the data
	for(Int32 i = 0; i < pheader->numcubemaps; i++)
	{
		const ecdcubemap_t* pcubemap = reinterpret_cast<const ecdcubemap_t*>(reinterpret_cast<const byte*>(pheader) + pheader->cubemapinfooffset)+i;

		m_cubemapsArray[i].cubemapindex = pcubemap->cubemapindex;
		m_cubemapsArray[i].entindex = pcubemap->entindex;
		m_cubemapsArray[i].width = pcubemap->width;
		m_cubemapsArray[i].height = pcubemap->height;
		Math::VectorCopy(pcubemap->origin, m_cubemapsArray[i].origin);

		m_cubemapsArray[i].palloc = pTextureManager->GenTextureIndex(RS_GAME_LEVEL);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemapsArray[i].palloc->gl_index);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_SEAMLESS, GL_TRUE);

		for (int j = 0; j < 6; j++)
		{
			const byte* psrc = (reinterpret_cast<const byte*>(pheader) + (rns.daystage == DAYSTAGE_NIGHTSTAGE ? pcubemap->nightdataoffset : pcubemap->dataoffset));
			psrc += j * pcubemap->width*pcubemap->height*3;

			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, 0, GL_RGB, m_cubemapsArray[i].width, m_cubemapsArray[i].height, 0, GL_RGB, GL_UNSIGNED_BYTE, psrc);
		}

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}

	FL_FreeFile(pf);

	return true;
}

//=============================================
// @brief
//
//=============================================
void CCubemapManager::ClearGame( void )
{
	Cleanup(true);
}

//=============================================
// @brief
//
//=============================================
void CCubemapManager::Cleanup( bool iscleargame )
{
	if(!iscleargame)
	{
		CTextureManager* pTextureManager = CTextureManager::GetInstance();

		// we need to delete these manually
		for(Uint32 i = 0; i < m_cubemapsArray.size(); i++)
		{
			if(m_cubemapsArray[i].palloc)
			{
				pTextureManager->DeleteAllocation(m_cubemapsArray[i].palloc);
				m_cubemapsArray[i].palloc = nullptr;
			}
		}
	}

	if(!m_cubemapsArray.empty())
		m_cubemapsArray.clear();

	m_pIdealCubemap = nullptr;
	m_pPrevCubemap = nullptr;
}

//=============================================
// @brief
//
//=============================================
void CCubemapManager::Update( const Vector& v_origin )
{
	if(rns.cubemapdraw)
	{
		m_pPrevCubemap = nullptr;
		m_flInterpolant = 1.0;
		m_pIdealCubemap = nullptr;
	}

	if(m_cubemapsArray.empty())
		return;

	// Calculate interpolation
	if(m_flInterpolant != 1.0 && m_flLastChangeTime != rns.time)
	{
		if(m_flLastChangeTime + CUBEMAP_INTERP_TIME > rns.time)
		{
			Float timeFrac = rns.time - m_flLastChangeTime;
			m_flInterpolant = timeFrac/CUBEMAP_INTERP_TIME;
		}
		else
			m_flInterpolant = 1.0;
	}

	// Don't bother if we're still interpolating
	if(m_flInterpolant < 1.0)
		return;

	// Find the closest cubemap
	Int32 closestIndex = -1;
	Float closestDistance = 0;

	for(Uint32 i = 0; i < m_cubemapsArray.size(); i++)
	{
		Float distance = (m_cubemapsArray[i].origin - v_origin).Length();
		if(closestIndex == -1 || distance < closestDistance)
		{
			closestDistance = distance;
			closestIndex = i;
		}
	}

	// if nothing changed, don't bother
	if(m_pIdealCubemap && m_pIdealCubemap->cubemapindex == closestIndex)
		return;

	// Change the ideal cubemap
	m_pPrevCubemap = m_pIdealCubemap;
	m_pIdealCubemap = &m_cubemapsArray[closestIndex];

	m_flLastChangeTime = rns.time;
	m_flInterpolant = 0.0;
}

//=============================================
// @brief
//
//=============================================
void CCubemapManager::BuildCubemaps( void )
{
	if(!CL_IsGameActive())
	{
		Con_Printf("Cannot build cubemaps unless a game is loaded.\n");
		return;
	}

	if(!m_cubemapsArray.empty())
		Cleanup(false);

	Uint32 numEntities = 0;
	const entitydata_t* pEntities = nullptr;

	// Call the entity manager to rebuild the ent list
	cls.dllfuncs.pfnParseEntityList();
	cls.dllfuncs.pfnGetClientEntityList(pEntities, numEntities);

	if(!numEntities || !pEntities)
	{
		cls.dllfuncs.pfnFreeEntityData();
		return;
	}

	// Load them from the ent manager
	BuildCubemapList(pEntities, numEntities);

	// Now build the list of renderable entities
	Uint32 numRenderEntities = 0;
	cl_entity_t *pRenderEntities = new cl_entity_t[MAX_RENDER_ENTITIES];
	BuildEntityList(&pRenderEntities, &numRenderEntities, pEntities, numEntities);

	// Now draw the cubemap faces for all cubemaps
	bool result = RenderCubemaps(pRenderEntities, numRenderEntities);

	// Free render data
	cls.dllfuncs.pfnFreeEntityData();
	delete[] pRenderEntities;

	if(!result)
	{
		Sys_ErrorPopup("Rendering error while drawing cubemaps.\n");
		CL_Disconnect();
		ens.exit = true;
		return;
	}

	// Save the cubemap data
	SaveCubemapFile();
}

//=============================================
// @brief
//
//=============================================
cubemapinfo_t* CCubemapManager::GetIdealCubemap( void )
{
	return m_pIdealCubemap;
}

//=============================================
// @brief
//
//=============================================
cubemapinfo_t* CCubemapManager::GetPrevCubemap( void )
{
	return m_pPrevCubemap;
}

//=============================================
// @brief
//
//=============================================
Float CCubemapManager::GetInterpolant( void ) const
{
	return m_flInterpolant;
}

//=============================================
// @brief
//
//=============================================
void CCubemapManager::AddCubemap( entindex_t entindex, const Vector& origin, Uint32 resolution )
{
	m_cubemapsArray.resize(m_cubemapsArray.size()+1);
	cubemapinfo_t* pcubemap = &m_cubemapsArray[m_cubemapsArray.size()-1];

	pcubemap->entindex = entindex;
	pcubemap->cubemapindex = m_cubemapsArray.size()-1;
	pcubemap->width = g_cubemapResolutions[resolution][0];
	pcubemap->height = g_cubemapResolutions[resolution][1];
	Math::VectorCopy(origin, pcubemap->origin);
}

//=============================================
// @brief
//
//=============================================
void CCubemapManager::BuildCubemapList( const entitydata_t* pEntities, Uint32 numEntities )
{
	// Parse all the entities
	for(Uint32 i = 0; i < numEntities; i++)
	{
		// Check for the classname
		const Char *pkeyvalue = ValueForKey(pEntities[i], "classname");
		if(strcmp(pkeyvalue, "env_cubemap"))
			continue;

		// Grab origin
		Vector origin;
		pkeyvalue = ValueForKey(pEntities[i], "origin");
		if (pkeyvalue)
			Common::StringToVector(pkeyvalue, origin);

		// Grab size
		Uint32 cubemapSize = 0;
		pkeyvalue = ValueForKey(pEntities[i], "size");
		if (pkeyvalue)
			cubemapSize = SDL_atof(pkeyvalue);

		// Make sure it's correct
		if(cubemapSize >= NUM_CUBEMAP_SIZES)
		{
			Con_Printf("Warning: Invalid size %d for cubemap at %f %f %f\n", cubemapSize, origin[0], origin[1], origin[2]);
			cubemapSize = 0;
		}

		// Add the new cubemap
		AddCubemap( i, origin, cubemapSize );
	}
}

//=============================================
// @brief
//
//=============================================
void CCubemapManager::BuildEntityList( cl_entity_t** pRenderEntities, Uint32 *pNumRenderEntities, const entitydata_t* pEntities, Uint32 numEntities )
{
	for(Uint32 i = 0; i < numEntities; i++)
	{
		// Check for the classname
		const Char *pkeyvalue = ValueForKey(pEntities[i], "classname");
		if(strcmp(pkeyvalue, "func_detail") 
			&& strcmp(pkeyvalue, "func_wall") 
			&& strcmp(pkeyvalue, "func_illusionary")
			&& strcmp(pkeyvalue, "env_model"))
			continue;

		// Grab model
		pkeyvalue = ValueForKey(pEntities[i], "model");
		if (!pkeyvalue)
			continue;

		cache_model_t* pmodel = nullptr;
		if(pkeyvalue[0] == '*')
		{
			// Find the brush model for this entity
			Uint32 modelindex = atoi(&pkeyvalue[1]);
			pmodel = gModelCache.GetModelByIndex(modelindex+1);
			if(!pmodel || pmodel->type != MOD_BRUSH)
				continue;
		}
		else if(qstrstr(pkeyvalue, "mdl"))
		{
			pmodel = gModelCache.LoadModel(pkeyvalue);
			if(!pmodel || pmodel->type != MOD_VBM)
				continue;
		}
		else
		{
			// Don't bother with sprites
			continue;
		}

		// Grab origin
		Vector origin;
		Math::VectorClear(origin);
		pkeyvalue = ValueForKey(pEntities[i], "origin");
		if (pkeyvalue)
			Common::StringToVector(pkeyvalue, origin);

		// Grab angles
		Vector angles;
		Math::VectorClear(angles);
		pkeyvalue = ValueForKey(pEntities[i], "angles");
		if (pkeyvalue)
			Common::StringToVector(pkeyvalue, angles);

		// Grab rendermode
		Uint32 renderMode = RENDER_NORMAL;
		pkeyvalue = ValueForKey(pEntities[i], "rendermode");
		if (pkeyvalue)
			renderMode = SDL_atoi(pkeyvalue);

		// Grab renderamt
		Int32 renderamt = 0;
		pkeyvalue = ValueForKey(pEntities[i], "renderamt");
		if (pkeyvalue)
			renderamt = SDL_atoi(pkeyvalue);

		// Grab body
		Int32 body = 0;
		pkeyvalue = ValueForKey(pEntities[i], "body");
		if (pkeyvalue)
			body = SDL_atoi(pkeyvalue);

		// Grab skin
		Int32 skin = 0;
		pkeyvalue = ValueForKey(pEntities[i], "skin");
		if (pkeyvalue)
			skin = SDL_atoi(pkeyvalue);

		// Add the entity to the list
		cl_entity_t* pEntity = &(*pRenderEntities)[(*pNumRenderEntities)];
		(*pNumRenderEntities)++;

		// Set values
		Math::VectorCopy(origin, pEntity->curstate.origin);
		Math::VectorCopy(angles, pEntity->curstate.angles);

		pEntity->curstate.rendermode = static_cast<rendermode_t>(renderMode);
		pEntity->curstate.renderamt = renderamt;
		pEntity->curstate.body = body;
		pEntity->curstate.skin = skin;

		// Set model
		pEntity->pmodel = pmodel;
	}
}

//=============================================
// @brief
//
//=============================================
bool CCubemapManager::VerifyECDFile( const ecdheader_t* pheader )
{
	if(pheader->id != ECD_HEADER_ENCODED)
		return false;

	if(pheader->version != CUBEMAP_FILE_VERSION)
		return false;

	if(pheader->numcubemaps != static_cast<Int32>(m_cubemapsArray.size()))
		return false;

	for(Int32 i = 0; i < pheader->numcubemaps; i++)
	{
		const ecdcubemap_t* pcubemap = reinterpret_cast<const ecdcubemap_t*>(reinterpret_cast<const byte *>(pheader) + pheader->cubemapinfooffset)+i;
		if(pcubemap->cubemapindex != m_cubemapsArray[i].cubemapindex
			|| pcubemap->entindex != m_cubemapsArray[i].entindex
			|| pcubemap->width != m_cubemapsArray[i].width
			|| pcubemap->height != m_cubemapsArray[i].height
			|| pcubemap->origin != m_cubemapsArray[i].origin)
			return false;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CCubemapManager::SaveCubemapFile( void )
{
	if(m_cubemapsArray.empty())
		return;

	// Try to load the original file first
	CString filename = Engine_GetLevelName();
	Uint32 begin = filename.find(0, ".bsp");
	if(begin != -1)
		filename.erase(begin, 4);
	filename << ".ecd";

	const ecdheader_t* ploadheader = nullptr;

	Uint32 size = 0;
	const byte* pf = FL_LoadFile(filename.c_str(), &size);
	if(pf)
	{
		ploadheader = reinterpret_cast<const ecdheader_t*>(pf);
		if(!VerifyECDFile(ploadheader))
		{
			// Not a valid format or file, discard it
			FL_FreeFile(pf);
			ploadheader = nullptr; 
			size = 0;
		}
	}

	// Allocate temporary buffer
	CBuffer fileBuffer;

	// Copy existing data
	if(ploadheader && size)
	{
		fileBuffer.append(ploadheader, size);
		FL_FreeFile(pf);
	}
	else
	{
		// Create space for header
		fileBuffer.append(nullptr, sizeof(ecdheader_t));
	}

	// Retreive data ptr
	void*& dataptr = fileBuffer.getbufferdata();
	ecdheader_t* pheader = reinterpret_cast<ecdheader_t*>(dataptr);
	fileBuffer.addpointer(reinterpret_cast<void**>(&pheader));

	// Save basic info
	pheader->id = ECD_HEADER_ENCODED;
	pheader->version = CUBEMAP_FILE_VERSION;
	
	if(rns.daystage == DAYSTAGE_NIGHTSTAGE)
		pheader->flags |= FL_ECD_HAS_NIGHTDATA;
	else
		pheader->flags |= FL_ECD_HAS_DAYDATA;

	if(!pheader->numcubemaps || !pheader->cubemapinfooffset)
	{
		pheader->numcubemaps = m_cubemapsArray.size();
		pheader->cubemapinfooffset = fileBuffer.getsize();
		fileBuffer.append(nullptr, sizeof(ecdcubemap_t)*pheader->numcubemaps);
	}

	// Save cubemap data
	for(Uint32 i = 0; i < m_cubemapsArray.size(); i++)
	{
		ecdcubemap_t* pcubemap = reinterpret_cast<ecdcubemap_t*>(reinterpret_cast<byte*>(pheader) + pheader->cubemapinfooffset)+i;
		pcubemap->cubemapindex = m_cubemapsArray[i].cubemapindex;
		pcubemap->entindex = m_cubemapsArray[i].entindex;
		pcubemap->width = m_cubemapsArray[i].width;
		pcubemap->height = m_cubemapsArray[i].height;
		Math::VectorCopy(m_cubemapsArray[i].origin, pcubemap->origin);
		
		Int32 offset = fileBuffer.getsize();
		Int32 imagesize = pcubemap->width*pcubemap->height*3;

		if(rns.daystage != DAYSTAGE_NIGHTSTAGE)
		{
			if(!pcubemap->dataoffset)
				pcubemap->dataoffset = offset;
			else
				offset = pcubemap->dataoffset;
		}
		else
		{
			if(!pcubemap->nightdataoffset)
				pcubemap->nightdataoffset = offset;
			else
				offset = pcubemap->nightdataoffset;
		}

		// Write the data and reset pointer after reallocation
		fileBuffer.append(m_cubemapsArray[i].pimagedata, static_cast<Uint32>(imagesize*6));

		// Free the image data
		delete[] m_cubemapsArray[i].pimagedata;
		m_cubemapsArray[i].pimagedata = nullptr;
	}

	// Set length
	pheader->length = fileBuffer.getsize();

	const byte *pwritedata = reinterpret_cast<const byte*>(fileBuffer.getbufferdata());
	if(!FL_WriteFile(pwritedata, fileBuffer.getsize(), filename.c_str()))
	{
		Con_Printf("Error: Failed to open %s for writing.\n", filename.c_str());
		return;
	}
}

//=============================================
// @brief
//
//=============================================
bool CCubemapManager::RenderCubemaps( cl_entity_t* pRenderEntities, Uint32 numRenderEntities )
{
	// Holds rendering info
	ref_params_t viewParams;

	// Angles used for the faces
	Vector faceAngles[] = {
		Vector(0, -90, 180),
		Vector(0, 90, 180),
		Vector(-90, 0, 0),
		Vector(90, 0, 0),
		Vector(0, 180, 180),
		Vector(0, 0, 180)
	};

	// Error tracking
	bool result = true;

	CTextureManager* pTextureManager = CTextureManager::GetInstance();

	// Draw the faces for each cubemap
	for(Uint32 i = 0; i < m_cubemapsArray.size(); i++)
	{
		// Allocate texture data reservoir
		Uint32 imagedatasize = m_cubemapsArray[i].width*m_cubemapsArray[i].height*3;
		m_cubemapsArray[i].pimagedata = new byte[imagedatasize*6];

		// Delete any existing textures
		if(m_cubemapsArray[i].palloc)
			pTextureManager->DeleteAllocation(m_cubemapsArray[i].palloc);

		// Allocate the GL texture
		m_cubemapsArray[i].palloc = pTextureManager->GenTextureIndex(RS_GAME_LEVEL);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemapsArray[i].palloc->gl_index);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_SEAMLESS, GL_TRUE);

		for (Uint32 j = 0; j < 6; j++)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, 0, GL_RGB, m_cubemapsArray[i].width, m_cubemapsArray[i].height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		R_ResetFrameStates();

		// Set the render list up with our ents
		for(Uint32 j = 0; j < numRenderEntities; j++)
			R_AddEntity(&pRenderEntities[j]);

		// Set view params
		viewParams.time = rns.time;
		viewParams.viewsize = 90;

		Math::VectorCopy(m_cubemapsArray[i].origin, viewParams.v_origin);

		// Perform basic frame setups
		R_Update();
		// Get VIS
		R_MarkLeaves(viewParams.v_origin);

		// So we don't draw sprites/particles and don't update the projection matrix
		rns.cubemapdraw = true;

		// Set projection matrix
		Float flSize = tan((M_PI/360) * viewParams.viewsize);
		rns.view.projection.PushMatrix();
		rns.view.projection.LoadIdentity();
		rns.view.projection.SetFrustum(-flSize, flSize, -flSize, flSize, 1, 16384);

		viewParams.screenwidth = m_cubemapsArray[i].width;
		viewParams.screenheight = m_cubemapsArray[i].height;

		glViewport(0, 0, viewParams.screenwidth, viewParams.screenheight);

		// Draw each individual face
		for(Uint32 j = 0; j < 6; j++)
		{
			// Set view angles
			Math::VectorCopy(faceAngles[j], viewParams.v_angles);
			Math::VectorCopy(faceAngles[j], viewParams.pl_viewangles);

			glCullFace(GL_FRONT);
			glDisable(GL_BLEND);

			CFBOCache::cache_fbo_t* pCubemapFBO = nullptr;
			if (rns.fboused && rns.usehdr)
			{
				pCubemapFBO = gFBOCache.Alloc(m_cubemapsArray[i].width, m_cubemapsArray[i].height, true);
				if (!pCubemapFBO)
				{
					Con_Printf("%s - Failed to get FBO for cubemap rendering with width %d, height %d.\n", __FUNCTION__, m_cubemapsArray[i].width, m_cubemapsArray[i].height);
					result = false;
					break;
				}

				R_BindFBO(&pCubemapFBO->fbo);
			}

			// Draw everything
			result = R_Draw(viewParams);
			if (!result)
			{
				gFBOCache.Free(pCubemapFBO);
				break;
			}

			if (rns.fboused && rns.usehdr)
			{
				assert(pCubemapFBO != nullptr);
				gGLExtF.glBindFramebuffer(GL_READ_FRAMEBUFFER, pCubemapFBO->fbo.fboid);
				glReadBuffer(GL_COLOR_ATTACHMENT0);
			}

			// Save it into the buffer
			byte* pdest = m_cubemapsArray[i].pimagedata + imagedatasize * j;
			glReadPixels(0, 0, m_cubemapsArray[i].width, m_cubemapsArray[i].height, GL_RGB, GL_UNSIGNED_BYTE, pdest);

			CString filepath;
			filepath << "cubemap_" << static_cast<Int32>(i) << "_" << static_cast<Int32>(j) << ".tga";
			TGA_Write(pdest, 3, m_cubemapsArray[i].width, m_cubemapsArray[i].height, filepath.c_str(), FL_GetInterface(), Con_EPrintf);

			// Save it to the OGL texture too
			gGLExtF.glActiveTexture(GL_TEXTURE0_ARB);
			glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemapsArray[i].palloc->gl_index);

			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, 0, GL_RGB, m_cubemapsArray[i].width, m_cubemapsArray[i].height, FALSE, GL_RGB, GL_UNSIGNED_BYTE, pdest);

			if (rns.fboused && rns.usehdr)
			{
				// Unbind FBO and free it
				R_BindFBO(nullptr);
				gFBOCache.Free(pCubemapFBO);
			}
		}

		// Restore projection
		rns.view.projection.PopMatrix();

		if(!result)
			break;
	}

	// Restore view size
	glViewport(0, 0, rns.screenwidth, rns.screenheight);
	return result;
}