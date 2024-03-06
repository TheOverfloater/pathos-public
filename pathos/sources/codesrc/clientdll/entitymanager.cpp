/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "cldll_interface.h"
#include "clientdll.h"
#include "cl_entity.h"
#include "entitymanager.h"
#include "cache_model.h"
#include "brushmodel_shared.h"
#include "com_math.h"
#include "entity_extrainfo.h"
#include "decallist.h"
#include "view.h"
#include "vbm_shared.h"

// Class definition
CEntityManager gEntityManager;

//=============================================
// @brief
//
//=============================================
CEntityManager::CEntityManager( void ) :
	m_pCvarDrawClientEntities( nullptr ),
	m_lastIdentifierUsed(0)
{
}

//=============================================
// @brief
//
//=============================================
CEntityManager::~CEntityManager( void )
{
}

//=============================================
// @brief
//
//=============================================
void CEntityManager::Init( void )
{
	m_pCvarDrawClientEntities = cl_engfuncs.pfnCreateCVar( CVAR_FLOAT, FL_CV_CLIENT, "r_client_entities", "1", "Toggles rendering of client-side only entities." );
}

//=============================================
// @brief
//
//=============================================
void CEntityManager::Clear( void )
{
	if(!m_entitiesArray.empty())
		m_entitiesArray.clear();

	FreeEntityData();

	m_promptHashList.clear();
}

//=============================================
// @brief
//
//=============================================
void CEntityManager::FreeEntityData( void )
{
	if(m_bspEntitiesArray.empty())
		return;

	m_bspEntitiesArray.clear();
}

//=============================================
// @brief
//
//=============================================
void CEntityManager::Shutdown( void )
{
	Clear();
}

//=============================================
// @brief
//
//=============================================
void CEntityManager::Setup ( void )
{
	// Reset this
	m_lastIdentifierUsed = ENTITY_IDENTIFIER_RESERVED_MIN;

	ParseEntities();
	LoadEntVars();
	FreeEntityData();
}

//=============================================
// @brief
//
//=============================================
void CEntityManager::ParseEntities ( void )
{
	Char token[MAX_PATH];

	// Retreive world model ptr
	const cache_model_t *pmodel = cl_engfuncs.pfnGetModel(WORLD_MODEL_INDEX);
	if(!pmodel || pmodel->type != MOD_BRUSH)
	{
		cl_engfuncs.pfnCon_EPrintf("%s - Couldn't get world model.\n", __FUNCTION__);
		return;
	}

	// Get brushmodel ptr
	const brushmodel_t* pbrushmodel = pmodel->getBrushmodel();

	// Count the number of entities
	Int32 depth = 0;
	Uint32 numentities = 0;
	const Char *pscan = pbrushmodel->pentdata;
	while(pscan && *pscan != '\0')
	{
		if(*pscan == '\"')
		{
			pscan++;
			while(*pscan && *pscan != '\"')
				pscan++;

			if(*pscan)
				pscan++;
		}

		if(*pscan == '{')
		{
			depth++;
			
			if(depth > 1)
			{
				cl_engfuncs.pfnCon_Printf("%s - Entity data is invalid.\n", __FUNCTION__);
				return;
			}

			numentities++;
		}
		else if(*pscan == '}')
		{
			depth--;
			if(depth < 0)
			{
				cl_engfuncs.pfnCon_Printf("%s - Entity data is invalid.\n", __FUNCTION__);
				return;
			}
		}

		pscan++;
	}

	// Resize array accordingly
	m_bspEntitiesArray.resize(numentities);

	entindex_t entindex = 0;
	pscan = pbrushmodel->pentdata;
	while(pscan && *pscan != '\0')
	{
		// Read first token
		pscan = Common::Parse(pscan, token);
		if(!pscan || qstrcmp(token, "{"))
		{
			cl_engfuncs.pfnCon_EPrintf("%s - Entity data for bsp is invalid.\n", __FUNCTION__);
			m_bspEntitiesArray.clear();
			return;
		}

		// Check for errors
		if(entindex >= (Int32)m_bspEntitiesArray.size())
		{
			cl_engfuncs.pfnCon_Printf("%s - Entity data is invalid.\n", __FUNCTION__);
			m_bspEntitiesArray.clear();
		}

		// new entity data struct
		entitydata_t& entity = m_bspEntitiesArray[entindex];
		entindex++;

		// Should suffice
		entity.values.reserve(64);

		while(true)
		{
			pscan = Common::Parse(pscan, token);
			if(!qstrcmp(token, "}"))
				break;

			if(!pscan)
			{
				cl_engfuncs.pfnCon_EPrintf("%s - Entity data for bsp is invalid.\n", __FUNCTION__);
				m_bspEntitiesArray.clear();
				return;
			}

			keyvalue_t* pkv = new keyvalue_t;
			qstrcpy(pkv->keyname, token);

			pscan = Common::Parse(pscan, token);
			if(!pscan)
			{
				cl_engfuncs.pfnCon_EPrintf("%s - Entity data for bsp is invalid.\n", __FUNCTION__);
				m_bspEntitiesArray.clear();
				return;
			}
			qstrcpy(pkv->value, token);

			entity.values.push_back(pkv);
		}

		// Resize it to the final size
		entity.values.resize(entity.values.size());
	}
}

//=============================================
// @brief
//
//=============================================
void CEntityManager::Entity_EnvCable( const entitydata_t& entity )
{
	// Get our origin
	const Char *pValue = ValueForKey(entity, "origin");
	if(!pValue)
		return;

	Vector vorigin1, vorigin2;
	Common::StringToVector(pValue, vorigin1);

	// Find our target entity
	pValue = ValueForKey(entity, "target");
	if(!pValue)
	{
		cl_engfuncs.pfnCon_Printf("%s - env_cable entity at %f %f %f has no target set.\n", __FUNCTION__, vorigin1.x, vorigin1.y, vorigin1.z);
		return;
	}

	const entitydata_t* pentity = FindEntityByTargetName("env_cable", pValue);
	if(!pentity)
	{
		pentity = FindEntityByTargetName("info_target", pValue);
		if(!pentity)
		{
			cl_engfuncs.pfnCon_Printf("%s - env_cable entity at %f %f %f can't find target '%s'.\n", __FUNCTION__, vorigin1.x, vorigin1.y, vorigin1.z, pValue);
			return;
		}
	}

	pValue = ValueForKey((*pentity), "origin");
	if(!pValue)
		return;

	Common::StringToVector(pValue, vorigin2);

	// Get our falling depth
	pValue = ValueForKey(entity, "falldepth");
	if(!pValue)
		return;

	Uint32 falldepth = atoi(pValue);

	// Get sprite width
	pValue = ValueForKey(entity, "spritewidth");
	if(!pValue)
		return;

	Float flwidth = atof(pValue);
	if(flwidth < 1) flwidth = 1;

	// Get segment count
	pValue = ValueForKey(entity, "segments");
	if(!pValue)
		return;

	Uint32 numsegments = atoi(pValue);
	cl_efxapi.pfnCreateCableEntity(vorigin1, vorigin2, falldepth, flwidth, numsegments);
}

//=============================================
// @brief
//
//=============================================
void CEntityManager::Entity_EnvDecal( const entitydata_t& entity )
{
	const Char *pvalue = ValueForKey(entity, "targetname");
	if(pvalue)
		return;

	Vector origin;
	pvalue = ValueForKey(entity, "origin");
	if (pvalue)
		Common::StringToVector(pvalue, origin);

	pvalue = ValueForKey(entity, "message");
	if (!pvalue || !qstrlen(pvalue))
		return;
	CString decalname = pvalue;

	// Default flags
	Int32 flags = (FL_DECAL_PERSISTENT|FL_DECAL_CL_ENTITY_MANAGER);
	Int32 spawnflags = 0;
	pvalue = ValueForKey(entity, "spawnflags");
	if (pvalue)
		spawnflags = SDL_atoi(pvalue);

	// Random decals must be server-handled so
	// the decal name is saved
	if(spawnflags & FL_ENVDECAL_RANDOM)
		return;

	if(!(spawnflags & FL_ENVDECAL_NO_VBM))
		flags |= FL_DECAL_VBM;

	if(spawnflags & FL_ENVDECAL_PERMISSIVE)
		flags |= FL_DECAL_NORMAL_PERMISSIVE;

	cl_engfuncs.pfnPrecacheDecal(decalname.c_str());
	cl_efxapi.pfnCreateGenericDecal(decalname.c_str(), origin, Vector(0, 0, 0), flags, NO_ENTITY_INDEX, 0, 0, 0);
}

//=============================================
// @brief
//
//=============================================
void CEntityManager::Entity_EnvSprite( const entitydata_t& entity, entindex_t& entindex )
{
	const Char *pvalue = ValueForKey(entity, "targetname");
	if(pvalue)
		return;

	pvalue = ValueForKey(entity, "model");
	if(!pvalue || !qstrstr(pvalue, ".spr"))
		return;

	const cache_model_t *pmodel = cl_engfuncs.pfnLoadModel(pvalue);
	if(!pmodel)
	{
		if(m_promptHashList.addhash(reinterpret_cast<const byte*>(pvalue), qstrlen(pvalue)))
			cl_engfuncs.pfnCon_Printf("%s - Failed to load '%s'.\n", __FUNCTION__, pvalue);
		return;
	}

	cl_entity_t *pEntity = cl_efxapi.pfnAllocStaticSpriteEntity(); 
	if(!pEntity)
		return;
			
	pEntity->entindex = entindex+1;
	entindex++;

	pEntity->identifier = m_lastIdentifierUsed;
	m_lastIdentifierUsed++;

	pEntity->pmodel = pmodel;
	pEntity->curstate.framerate = 1;

	pvalue = ValueForKey(entity, "origin");
	if (pvalue)
		Common::StringToVector(pvalue, pEntity->curstate.origin);

	pvalue = ValueForKey(entity, "angles");
	if (pvalue)
		Common::StringToVector(pvalue, pEntity->curstate.angles);

	pvalue = ValueForKey(entity, "framerate");
	if (pvalue)
		pEntity->curstate.framerate = SDL_atof(pvalue);

	pvalue = ValueForKey(entity, "rendermode");
	if (pvalue)
		pEntity->curstate.rendermode = (rendermode_t)SDL_atoi(pvalue);

	pvalue = ValueForKey(entity, "renderamt");
	if (pvalue)
		pEntity->curstate.renderamt = SDL_atoi(pvalue);

	pvalue = ValueForKey(entity, "renderfx");
	if (pvalue)
		pEntity->curstate.renderfx = SDL_atoi(pvalue);

	pvalue = ValueForKey(entity, "rendercolor");
	if (pvalue)
		Common::StringToVector(pvalue, pEntity->curstate.rendercolor);

	pvalue = ValueForKey(entity, "scale");
	if (pvalue)
		pEntity->curstate.scale = SDL_atof(pvalue);

	entity_extrainfo_t *pInfo = cl_engfuncs.pfnGetEntityExtraData(pEntity);
	Math::VectorAdd(pEntity->pmodel->mins, pEntity->curstate.origin, pInfo->absmin);
	Math::VectorAdd(pEntity->pmodel->maxs, pEntity->curstate.origin, pInfo->absmax);

	Math::VectorAdd(pInfo->absmax, Vector(1, 1, 1), pInfo->absmax);
	Math::VectorSubtract(pInfo->absmin, Vector(1, 1, 1), pInfo->absmin);

	const cache_model_t* pworld = cl_engfuncs.pfnGetModel(1);
	const brushmodel_t* pbrushmodel = pworld->getBrushmodel();

	cl_engfuncs.pfnFindTouchedLeafs(pbrushmodel, pInfo->leafnums, pInfo->absmin, pInfo->absmax, pbrushmodel->pnodes);
}

//=============================================
// @brief
//
//=============================================
void CEntityManager::Entity_EnvELight( const entitydata_t& entity, entindex_t& entindex )
{
	const Char *pvalue = ValueForKey(entity, "targetname");
	if(pvalue)
	{
		// Entities with targetname are handled by the engine
		return;
	}

	if(m_entitiesArray.size() == MAX_SERVER_ENTITIES)
	{
		cl_engfuncs.pfnCon_Printf("%s - Exceeded MAX_SERVER_ENTITIES.\n", __FUNCTION__);
		return;
	}

	m_entitiesArray.resize(m_entitiesArray.size()+1);
	cl_entity_t& newEntity = m_entitiesArray[m_entitiesArray.size()-1];

	// Set identification index
	newEntity.entindex = entindex+1;
	entindex++;

	newEntity.identifier = m_lastIdentifierUsed;
	m_lastIdentifierUsed++;

	pvalue = ValueForKey(entity, "origin");
	if (pvalue)
		Common::StringToVector(pvalue, newEntity.curstate.origin);

	pvalue = ValueForKey(entity, "renderamt");
	if (pvalue)
		newEntity.curstate.renderamt = SDL_atoi(pvalue);

	pvalue = ValueForKey(entity, "rendercolor");
	if (pvalue)
		Common::StringToVector(pvalue, newEntity.curstate.rendercolor);

	entity_extrainfo_t *pInfo = cl_engfuncs.pfnGetEntityExtraData(&newEntity);

	// Calculate mins/maxs
	for(Int32 i = 0; i < 3; i++)
	{
		pInfo->absmax[i] = newEntity.curstate.origin[i] + newEntity.curstate.renderamt*9.5 + 1;
		pInfo->absmin[i] = newEntity.curstate.origin[i] - newEntity.curstate.renderamt*9.5 - 1;
	}

	const cache_model_t* pworld = cl_engfuncs.pfnGetModel(1);
	const brushmodel_t* pbrushmodel = pworld->getBrushmodel();

	cl_engfuncs.pfnFindTouchedLeafs(pbrushmodel, pInfo->leafnums, pInfo->absmin, pInfo->absmax, pbrushmodel->pnodes);

	// Set renderfx
	newEntity.curstate.rendertype = RT_ENVELIGHT;
}

//=============================================
// @brief
//
//=============================================
void CEntityManager::Entity_EnvModel( const entitydata_t& entity, entindex_t& entindex )
{
	const Char *pvalue = ValueForKey(entity, "targetname");
	if(pvalue)
	{
		// Entities with a targetname are handled by the engine
		return;
	}

	pvalue = ValueForKey(entity, "model");
	if(!pvalue || !qstrstr(pvalue, ".mdl"))
		return;

	const cache_model_t *pmodel = cl_engfuncs.pfnLoadModel(pvalue);
	if (!pmodel)
	{
		if(m_promptHashList.addhash(reinterpret_cast<const byte*>(pvalue), qstrlen(pvalue)))
			cl_engfuncs.pfnCon_Printf("%s - Failed to load '%s'.\n", __FUNCTION__, pvalue);
		return;
	}

	if(m_entitiesArray.size() == MAX_SERVER_ENTITIES)
	{
		cl_engfuncs.pfnCon_Printf("%s - Exceeded MAX_SERVER_ENTITIES.\n", __FUNCTION__);
		return;
	}

	m_entitiesArray.resize(m_entitiesArray.size()+1);
	cl_entity_t& newEntity = m_entitiesArray[m_entitiesArray.size()-1];

	newEntity.entindex = entindex+1;
	entindex++;

	newEntity.identifier = m_lastIdentifierUsed;
	m_lastIdentifierUsed++;

	newEntity.visframe = -1;
	newEntity.pmodel = pmodel;
	newEntity.curstate.effects |= EF_CLIENTENT;
	newEntity.curstate.modelindex = pmodel->cacheindex;

	pvalue = ValueForKey(entity, "origin");
	if (pvalue)
	{
		Common::StringToVector(pvalue, newEntity.curstate.origin);
		newEntity.latched.origin = newEntity.curstate.origin;
		newEntity.prevstate.origin = newEntity.curstate.origin;
	}

	pvalue = ValueForKey(entity, "angles");
	if (pvalue)
	{
		Common::StringToVector(pvalue, newEntity.curstate.angles);
		newEntity.latched.angles = newEntity.curstate.angles;
		newEntity.prevstate.angles = newEntity.curstate.angles;
	}

	pvalue = ValueForKey(entity, "rendermode");
	if (pvalue)
		newEntity.curstate.rendermode = (rendermode_t)SDL_atoi(pvalue);

	pvalue = ValueForKey(entity, "renderamt");
	if (pvalue)
		newEntity.curstate.renderamt = SDL_atoi(pvalue);

	pvalue = ValueForKey(entity, "renderfx");
	if (pvalue)
		newEntity.curstate.renderfx = SDL_atoi(pvalue);

	pvalue = ValueForKey(entity, "rendercolor");
	if (pvalue)
		Common::StringToVector(pvalue, newEntity.curstate.rendercolor);

	pvalue = ValueForKey(entity, "sequence");
	if (pvalue)
		newEntity.curstate.sequence = SDL_atoi(pvalue);

	pvalue = ValueForKey(entity, "framerate");
	if (pvalue)
		newEntity.curstate.framerate = SDL_atof(pvalue);

	if(!newEntity.curstate.framerate)
		newEntity.curstate.framerate = 1;

	pvalue = ValueForKey(entity, "body");
	if (pvalue)
		newEntity.curstate.body = SDL_atoi(pvalue);

	pvalue = ValueForKey(entity, "skin");
	if (pvalue)
		newEntity.curstate.skin = SDL_atoi(pvalue);

	pvalue = ValueForKey(entity, "scale");
	if (pvalue)
		newEntity.curstate.scale = SDL_atof(pvalue);

	pvalue = ValueForKey(entity, "lightorigin");
	if(pvalue && qstrlen(pvalue))
	{
		const entitydata_t* pentity = FindEntityByTargetName("info_light_origin", pvalue);
		if(pentity)
		{
			pvalue = ValueForKey((*pentity), "origin");
			if(pvalue)
			{
				Common::StringToVector(pvalue, newEntity.curstate.lightorigin);
				newEntity.curstate.effects |= EF_ALTLIGHTORIGIN;
			}
		}
	}

	// Retreive studio cache object
	const vbmcache_t* pstudiocache = newEntity.pmodel->getVBMCache();
	const studiohdr_t* pstudiohdr = pstudiocache->pstudiohdr;

	// seqname overrides sequence parameter
	pvalue = ValueForKey(entity, "seqname");
	if(pvalue && qstrlen(pvalue))
	{
		int i = 0;
		for(; i < pstudiohdr->numseq; i++)
		{
			const mstudioseqdesc_t *pseqdesc = pstudiohdr->getSequence(i);
			if(!strcmp(pseqdesc->label, pvalue))
				break;
		}

		if(i != pstudiohdr->numseq)
			newEntity.curstate.sequence = i;
		else
			cl_engfuncs.pfnCon_Printf("%s - env_model '%s' at %.0f %.0f %.0f - no such sequence '%s'.\n", __FUNCTION__, newEntity.pmodel->name.c_str(), newEntity.curstate.origin.x, newEntity.curstate.origin.y, newEntity.curstate.origin.z, pvalue);
	}

	// Cap sequence index
	if (newEntity.curstate.sequence >=  pstudiohdr->numseq) 
		newEntity.curstate.sequence = 0;

	Vector vtemp;
	Vector vbounds[8];
	const mstudioseqdesc_t *pseqdesc = pstudiohdr->getSequence(newEntity.curstate.sequence);
	for (int i = 0; i < 8; i++)
	{
		if ( i & 1 ) vtemp[0] = pseqdesc->bbmin[0];
		else vtemp[0] = pseqdesc->bbmax[0];
		if ( i & 2 ) vtemp[1] = pseqdesc->bbmin[1];
		else vtemp[1] = pseqdesc->bbmax[1];
		if ( i & 4 ) vtemp[2] = pseqdesc->bbmin[2];
		else vtemp[2] = pseqdesc->bbmax[2];
		Math::VectorCopy( vtemp, vbounds[i] );
	}
		
	Float anglemarix[3][4];
	Math::AngleMatrix(newEntity.curstate.angles, anglemarix);

	for (int i = 0; i < 8; i++ )
	{
		Math::VectorCopy(vbounds[i], vtemp);
		Math::VectorRotate(vtemp, anglemarix, vbounds[i]);
	}

	// Set the bounding box
	Vector vmins = NULL_MINS;
	Vector vmaxs = NULL_MAXS;
	for(Uint32 i = 0; i < 8; i++)
	{
		// Mins
		if(vbounds[i][0] < vmins[0]) vmins[0] = vbounds[i][0];
		if(vbounds[i][1] < vmins[1]) vmins[1] = vbounds[i][1];
		if(vbounds[i][2] < vmins[2]) vmins[2] = vbounds[i][2];

		// Maxs
		if(vbounds[i][0] > vmaxs[0]) vmaxs[0] = vbounds[i][0];
		if(vbounds[i][1] > vmaxs[1]) vmaxs[1] = vbounds[i][1];
		if(vbounds[i][2] > vmaxs[2]) vmaxs[2] = vbounds[i][2];
	}

	entity_extrainfo_t *pInfo = cl_engfuncs.pfnGetEntityExtraData(&newEntity);

	Math::VectorAdd(vmaxs, newEntity.curstate.origin, pInfo->absmax);
	Math::VectorAdd(vmins, newEntity.curstate.origin, pInfo->absmin);

	Math::VectorAdd(pInfo->absmax, Vector(1, 1, 1), pInfo->absmax);
	Math::VectorSubtract(pInfo->absmin, Vector(1, 1, 1), pInfo->absmin);

	const cache_model_t* pworld = cl_engfuncs.pfnGetModel(1);
	const brushmodel_t* pbrushmodel = pworld->getBrushmodel();

	cl_engfuncs.pfnFindTouchedLeafs(pbrushmodel, pInfo->leafnums, pInfo->absmin, pInfo->absmax, pbrushmodel->pnodes);

	// If it's on a non-looping sequence, calculate the bones only once
	if(!(pseqdesc->flags & STUDIO_LOOPING))
		newEntity.curstate.effects |= EF_STATICENTITY;

	// Set the bone conteollers
	for(Uint32 i = 0; i < pstudiohdr->numbonecontrollers; i++)
		VBM_SetController(pmodel, i, 0, newEntity.curstate.controllers, cl_engfuncs.pfnCon_Printf);
}

//=============================================
// @brief
//
//=============================================
const entitydata_t* CEntityManager::FindEntityByTargetName( const Char* pstrClassName, const Char* pstrTargetName )
{
	if(!pstrClassName || !pstrTargetName)
		return nullptr;

	for(Uint32 i = 0; i < m_bspEntitiesArray.size(); i++)
	{
		const Char* pvalue = ValueForKey(m_bspEntitiesArray[i], "classname");

		if (strcmp(pvalue, pstrClassName))
			continue;
					
		pvalue = ValueForKey(m_bspEntitiesArray[i], "targetname");	
		if(!pvalue)
			continue;

		if(!qstrcmp(pvalue, pstrTargetName))
			return &m_bspEntitiesArray[i];
	}

	return nullptr;
}


//=============================================
// @brief
//
//=============================================
const CArray<entitydata_t>& CEntityManager::GetEntityList( void ) const
{
	return m_bspEntitiesArray;
}

//=============================================
// @brief
//
//=============================================
void CEntityManager::LoadEntVars( void )
{
	// Start index from max renderents
	Int32 entityindex = CL_ENTITY_INDEX_BASE;

	// Create the client-side entities
	for(Uint32 i = 0; i < m_bspEntitiesArray.size(); i++)
	{
		const Char *pValue = ValueForKey(m_bspEntitiesArray[i], "classname");
		if(!pValue)
			continue;

		// Check for overrun
		if(entityindex >= CL_ENTITY_INDEX_BASE+MAX_CLIENTSIDE_ENTITIES)
		{
			cl_engfuncs.pfnCon_Printf("%s - Exceeded MAX_CLIENTSIDE_ENTITIES, not adding further client entities.\n", __FUNCTION__);
			break;
		}

		if(!strcmp( pValue, "env_elight"))
		{
			Entity_EnvELight(m_bspEntitiesArray[i], entityindex);
		}
		else if(!strcmp( pValue, "env_cable"))
		{
			Entity_EnvCable(m_bspEntitiesArray[i]);
		}
		else if(!strcmp( pValue, "env_decal"))
		{
			Entity_EnvDecal(m_bspEntitiesArray[i]);
		}
		else if(!strcmp( pValue, "env_sprite") || !strcmp( pValue, "env_glow"))
		{
			Entity_EnvSprite(m_bspEntitiesArray[i], entityindex);
		}
		else if(!strcmp( pValue, "env_model"))
		{
			Entity_EnvModel(m_bspEntitiesArray[i], entityindex);
		}
	}
}

//=============================================
// @brief
//
//=============================================
void CEntityManager::Frame( void )
{
	if(m_entitiesArray.empty())
		return;

	if(m_pCvarDrawClientEntities->GetValue() < 1)
		return;

	// Get player ptr
	cl_entity_t* pplayer = cl_engfuncs.pfnGetLocalPlayer();
	if(!pplayer)
		return;

	// Get view angles and origin
	Vector vorigin, vangles;
	V_GetViewInfo(vorigin, vangles);

	// Get pvs data for view origin
	const mleaf_t* pleaf = cl_engfuncs.pfnPointInLeaf(vorigin);
	const byte* ppvs = cl_engfuncs.pfnLeafPVS(*pleaf);

	// Collect all camera entities
	CArray<cl_entity_t*> cameraEntitiesArray;
	for(Uint32 i = 0; i < MAX_RENDER_ENTITIES; i++)
	{
		cl_entity_t* pentity = cl_engfuncs.pfnGetEntityByIndex(i);
		if(pentity && pentity->curstate.rendertype == RT_MONITORENTITY)
		{
			if(pentity->curstate.aiment == NO_ENTITY_INDEX)
				continue;

			cl_entity_t* pcamera = cl_engfuncs.pfnGetEntityByIndex(pentity->curstate.aiment);
			if(pcamera && pcamera->curstate.msg_num == pplayer->curstate.msg_num)
				cameraEntitiesArray.push_back(pcamera);
		}
	}

	for(Uint32 i = 0; i < m_entitiesArray.size(); i++)
	{
		cl_entity_t *pentity = &m_entitiesArray[i];

		// model light, add to the stack
		if( pentity->curstate.rendertype == RT_ENVELIGHT )
		{
			entity_extrainfo_t *pInfo = cl_engfuncs.pfnGetEntityExtraData(pentity);
			if( !pInfo )
				continue;

			if(Common::CheckVisibility(pInfo->leafnums, ppvs))
			{
				cl_engfuncs.pfnAddEntity(pentity);
				continue;
			}
		}

		if(!pentity->pmodel)
			continue;

		if(pentity->pmodel->type != MOD_VBM)
			continue;

		entity_extrainfo_t *pInfo = cl_engfuncs.pfnGetEntityExtraData(pentity);
		if(!pInfo)
			continue;

		if(pentity->curstate.renderfx != RenderFx_SkyEnt && pentity->curstate.renderfx != RenderFx_SkyEntNC)
		{
			if(!Common::CheckVisibility(pInfo->leafnums, ppvs))
			{
				Uint32 j = 0;
				for(; j < cameraEntitiesArray.size(); j++)
				{
					entity_extrainfo_t* pCameraInfo = cl_engfuncs.pfnGetEntityExtraData(cameraEntitiesArray[j]);
					if(!pCameraInfo || !pCameraInfo->ppvsdata)
						continue;

					if(Common::CheckVisibility(pInfo->leafnums, pCameraInfo->ppvsdata))
						break;
				}

				if(j == cameraEntitiesArray.size())
					continue;
			}
		}

		cl_engfuncs.pfnAddEntity(pentity);
	}
}

//=============================================
// @brief
//
//=============================================
void CEntityManager::DecalExternalEntities( const Vector& vpos, const Vector& vnorm, decalgroupentry_t *texptr, Int32 flags )
{
	if(!texptr)
		return;

	Float radius = (texptr->xsize > texptr->ysize) ? texptr->xsize : texptr->ysize;

	Vector vdecalmins, vdecalmaxs;
	for(Uint32 i = 0; i < 3; i++)
	{
		vdecalmins[i] = vpos[i] - radius;
		vdecalmaxs[i] = vpos[i] + radius;
	}

	for(Uint32 i = 0; i < m_entitiesArray.size(); i++)
	{
		cl_entity_t *pentity = &m_entitiesArray[i];

		if(!pentity->pmodel)
			continue;

		if(pentity->pmodel->type != MOD_VBM)
			continue;

		// grab vmins/maxs
		entity_extrainfo_t *pextradata = cl_engfuncs.pfnGetEntityExtraData(pentity);
		if(Math::CheckMinsMaxs(pextradata->absmin, pextradata->absmax, vdecalmins, vdecalmaxs))
			continue;

		cl_efxapi.pfnDecalVBMEntity(vpos, vnorm, texptr, pentity, flags);
	}
}