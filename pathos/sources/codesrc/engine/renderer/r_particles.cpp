/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "r_vbo.h"
#include "r_glsl.h"
#include "cl_entity.h"
#include "cl_main.h"
#include "r_main.h"
#include "cache_model.h"
#include "studio.h"
#include "cl_utils.h"
#include "r_common.h"
#include "cvar.h"
#include "console.h"
#include "r_particles.h"
#include "textures_shared.h"
#include "texturemanager.h"
#include "system.h"
#include "file.h"
#include "brushmodel.h"
#include "enginestate.h"
#include "cl_pmove.h"
#include "r_decals.h"
#include "r_bsp.h"
#include "r_dlights.h"
#include "r_rttcache.h"
#include "networking.h"
#include "r_vbm.h"
#include "r_blackhole.h"

// Class definition
CParticleEngine gParticleEngine;

// Particle texture base path
static const Char PARTICLE_TEXTURES_PATH[] = "particles/";

//====================================
//
//====================================
CParticleEngine::CParticleEngine( void ):
	m_pCvarDrawParticles(nullptr),
	m_pCvarParticleDebug(nullptr),
	m_pCvarGravity(nullptr),
	m_pWeatherDensity(nullptr),
	m_pParticles(nullptr),
	m_pFreeParticles(nullptr),
	m_iNumFreedParticles(0),
	m_iNumCreatedParticles(0),
	m_iNumFreedSystems(0),
	m_iNumCreatedSystems(0),
	m_pShader(nullptr),
	m_pVBO(nullptr),
	m_screenRectangleBase(0),
	m_pVertexes(nullptr),
	m_numVertexes(0),
	m_numIndexes(0),
	m_numParticles(0)
{
}

//====================================
//
//====================================
CParticleEngine::~CParticleEngine( void )
{
	Shutdown();
}

//====================================
//
//====================================
bool CParticleEngine::Init( void ) 
{
	m_pCvarDrawParticles = gConsole.CreateCVar( CVAR_FLOAT, FL_CV_CLIENT, "r_particles", "1", "Toggles particle rendering." );
	m_pCvarParticleDebug = gConsole.CreateCVar( CVAR_FLOAT, FL_CV_CLIENT, "r_particles_debug", "0", "Toggle particle debug info printing." );
	m_pWeatherDensity = gConsole.CreateCVar( CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), "r_weather_density", "1", "Controls weather particle density." );

	m_pCvarGravity = gConsole.GetCVar("sv_gravity");

	// Initialize free linked list
	m_pParticles = new cl_particle_t[MAX_ACTIVE_PARTICLES];

	// Link up the particles
	m_pFreeParticles = m_pParticles;
	for(Uint32 i = 0; i < (MAX_ACTIVE_PARTICLES-1); i++)
	{
		m_pParticles[i].next = &m_pParticles[i+1];
		m_pParticles[i+1].prev = &m_pParticles[i];
	}

	// Create vertexes array
	m_pVertexes = new particle_vertex_t[MAX_ACTIVE_PARTICLES*4+6];

	return true;
};

//====================================
//
//====================================
void CParticleEngine::Shutdown( void ) 
{
	if(m_pVertexes)
	{
		delete[] m_pVertexes;
		m_pVertexes = nullptr;
	}

	if(m_pParticles)
	{
		delete[] m_pParticles;
		m_pParticles = nullptr;
	}

	ClearGame();
	ClearGL();
}

//====================================
//
//====================================
bool CParticleEngine::InitGL( void ) 
{
	if(!m_pShader)
	{
		Int32 shaderFlags = CGLSLShader::FL_GLSL_SHADER_NONE;
		if(R_IsExtensionSupported("GL_ARB_get_program_binary"))
			shaderFlags |= CGLSLShader::FL_GLSL_BINARY_SHADER_OPS;

		m_pShader = new CGLSLShader(FL_GetInterface(), gGLExtF, "particles.bss", shaderFlags, VID_ShaderCompileCallback);
		if(m_pShader->HasError())
		{
			Sys_ErrorPopup("%s - Failed to compile shader: %s.", __FUNCTION__, m_pShader->GetError());
			return false;
		}

		m_attribs.a_origin = m_pShader->InitAttribute("in_position", 4, GL_FLOAT, sizeof(particle_vertex_t), OFFSET(particle_vertex_t, origin));
		m_attribs.a_color = m_pShader->InitAttribute("in_color", 4, GL_FLOAT, sizeof(particle_vertex_t), OFFSET(particle_vertex_t, color));
		m_attribs.a_texcoord =  m_pShader->InitAttribute("in_texcoord", 2, GL_FLOAT, sizeof(particle_vertex_t), OFFSET(particle_vertex_t, texcoord));

		if(!R_CheckShaderVertexAttribute(m_attribs.a_origin, "in_position", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderVertexAttribute(m_attribs.a_color, "in_color", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderVertexAttribute(m_attribs.a_texcoord, "in_texcoord", m_pShader, Sys_ErrorPopup))
			return false;

		m_attribs.u_modelview = m_pShader->InitUniform("modelview", CGLSLShader::UNIFORM_MATRIX4);
		m_attribs.u_projection = m_pShader->InitUniform("projection", CGLSLShader::UNIFORM_MATRIX4);
		m_attribs.u_texture0 = m_pShader->InitUniform("texture0", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_rtexture0 = m_pShader->InitUniform("rtexture0", CGLSLShader::UNIFORM_INT1);
		m_attribs.u_rtexture1 = m_pShader->InitUniform("rtexture1", CGLSLShader::UNIFORM_INT1);

		m_attribs.u_fogcolor = m_pShader->InitUniform("fogcolor", CGLSLShader::UNIFORM_FLOAT3);
		m_attribs.u_fogparams = m_pShader->InitUniform("fogparams", CGLSLShader::UNIFORM_FLOAT2);
		m_attribs.u_overbright = m_pShader->InitUniform("overbright", CGLSLShader::UNIFORM_FLOAT1);
		m_attribs.u_scrsize = m_pShader->InitUniform("scrsize", CGLSLShader::UNIFORM_FLOAT2);

		if(!R_CheckShaderUniform(m_attribs.u_modelview, "modelview", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_projection, "projection", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_texture0, "texture0", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_rtexture0, "rtexture0", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_rtexture1, "rtexture1", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_fogcolor, "fogcolor", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_fogparams, "fogparams", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_overbright, "overbright", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_scrsize, "scrsize", m_pShader, Sys_ErrorPopup))
			return false;

		m_attribs.d_fog = m_pShader->GetDeterminatorIndex("fog");
		m_attribs.d_type = m_pShader->GetDeterminatorIndex("type");
		m_attribs.d_alphatest = m_pShader->GetDeterminatorIndex("alphatest");

		if(!R_CheckShaderDeterminator(m_attribs.d_fog, "fog", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderDeterminator(m_attribs.d_type, "type", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderDeterminator(m_attribs.d_alphatest, "alphatest", m_pShader, Sys_ErrorPopup))
			return false;

		for(Uint32 i = 0; i < MAX_PARTICLE_POINT_LIGHTS; i++)
		{
			CString field;
			field << "point_lights_" << (Int32)i << "_color";
			m_attribs.point_lights[i].u_color = m_pShader->InitUniform(field.c_str(), CGLSLShader::UNIFORM_FLOAT3);
			if(!R_CheckShaderUniform(m_attribs.point_lights[i].u_color, field.c_str(), m_pShader, Sys_ErrorPopup))
				return false;

			field.clear();
			field << "point_lights_" << (Int32)i << "_origin";
			m_attribs.point_lights[i].u_origin = m_pShader->InitUniform(field.c_str(), CGLSLShader::UNIFORM_FLOAT3);
			if(!R_CheckShaderUniform(m_attribs.point_lights[i].u_origin, field.c_str(), m_pShader, Sys_ErrorPopup))
				return false;

			field.clear();
			field << "point_lights_" << (Int32)i << "_radius";
			m_attribs.point_lights[i].u_radius = m_pShader->InitUniform(field.c_str(), CGLSLShader::UNIFORM_FLOAT1);
			if(!R_CheckShaderUniform(m_attribs.point_lights[i].u_radius, field.c_str(), m_pShader, Sys_ErrorPopup))
				return false;
		}

		m_attribs.d_numpointlights = m_pShader->GetDeterminatorIndex("num_point_lights");

		for(Uint32 i = 0; i < MAX_PARTICLE_PROJ_LIGHTS; i++)
		{
			CString field;
			field << "proj_lights_" << (Int32)i << "_origin";
			m_attribs.proj_lights[i].u_origin = m_pShader->InitUniform(field.c_str(), CGLSLShader::UNIFORM_FLOAT3);
			if(!R_CheckShaderUniform(m_attribs.proj_lights[i].u_origin, field.c_str(), m_pShader, Sys_ErrorPopup))
				return false;

			field.clear();
			field << "proj_lights_" << (Int32)i << "_color";
			m_attribs.proj_lights[i].u_color = m_pShader->InitUniform(field.c_str(), CGLSLShader::UNIFORM_FLOAT3);
			if(!R_CheckShaderUniform(m_attribs.proj_lights[i].u_color, field.c_str(), m_pShader, Sys_ErrorPopup))
				return false;

			field.clear();
			field << "proj_lights_" << (Int32)i << "_radius";
			m_attribs.proj_lights[i].u_radius = m_pShader->InitUniform(field.c_str(), CGLSLShader::UNIFORM_FLOAT1);
			if(!R_CheckShaderUniform(m_attribs.proj_lights[i].u_radius, field.c_str(), m_pShader, Sys_ErrorPopup))
				return false;

			field.clear();
			field << "proj_lights_" << (Int32)i << "_matrix";
			m_attribs.proj_lights[i].u_matrix = m_pShader->InitUniform(field.c_str(), CGLSLShader::UNIFORM_MATRIX4);
			if(!R_CheckShaderUniform(m_attribs.proj_lights[i].u_matrix, field.c_str(), m_pShader, Sys_ErrorPopup))
				return false;

			field.clear();
			field << "proj_lights_" << (Int32)i << "_texture";
			m_attribs.proj_lights[i].u_texture = m_pShader->InitUniform(field.c_str(), CGLSLShader::UNIFORM_INT1);
			if(!R_CheckShaderUniform(m_attribs.proj_lights[i].u_texture, field.c_str(), m_pShader, Sys_ErrorPopup))
				return false;
		}

		m_attribs.d_numprojlights = m_pShader->GetDeterminatorIndex("num_proj_lights");
		if(!R_CheckShaderDeterminator(m_attribs.d_numprojlights, "num_proj_lights", m_pShader, Sys_ErrorPopup))
			return false;
	}

	if(!m_pVBO)
	{
		// Initialize the indexes
		Uint32 *indexes = new Uint32[MAX_ACTIVE_PARTICLES*6];
		for(Uint32 i = 0, j = 0; i < MAX_ACTIVE_PARTICLES; i++, j += 6)
		{
			Uint32 baseindex = (i*4);
			indexes[j] = baseindex; indexes[j+1] = baseindex+1; indexes[j+2] = baseindex+2;
			indexes[j+3] = baseindex; indexes[j+4] = baseindex+2; indexes[j+5] = baseindex+3;
		}

		Uint32 numvert = MAX_ACTIVE_PARTICLES*4+6;
		for(Uint32 i = 0; i < numvert; i++)
			m_pVertexes[i] = particle_vertex_t();

		// Screen rectangle comes after particles
		m_screenRectangleBase = MAX_ACTIVE_PARTICLES*4;
		Uint32 base = m_screenRectangleBase;

		m_pVertexes[base].origin[0] = 0; m_pVertexes[base].origin[1] = 1; 
		m_pVertexes[base].origin[2] = -1; m_pVertexes[base].origin[3] = 1;
		m_pVertexes[base].texcoord[0] = 0; m_pVertexes[base].texcoord[1] = 0;
		base++;

		m_pVertexes[base].origin[0] = 0; m_pVertexes[base].origin[1] = 0; 
		m_pVertexes[base].origin[2] = -1; m_pVertexes[base].origin[3] = 1;
		m_pVertexes[base].texcoord[0] = 0; m_pVertexes[base].texcoord[1] = rns.screenheight;
		base++;

		m_pVertexes[base].origin[0] = 1; m_pVertexes[base].origin[1] = 0; 
		m_pVertexes[base].origin[2] = -1; m_pVertexes[base].origin[3] = 1;
		m_pVertexes[base].texcoord[0] = rns.screenwidth; m_pVertexes[base].texcoord[1] = rns.screenheight;
		base++;

		m_pVertexes[base].origin[0] = 0; m_pVertexes[base].origin[1] = 1; 
		m_pVertexes[base].origin[2] = -1; m_pVertexes[base].origin[3] = 1;
		m_pVertexes[base].texcoord[0] = 0; m_pVertexes[base].texcoord[1] = 0;
		base++;

		m_pVertexes[base].origin[0] = 1; m_pVertexes[base].origin[1] = 0; 
		m_pVertexes[base].origin[2] = -1; m_pVertexes[base].origin[3] = 1;
		m_pVertexes[base].texcoord[0] = rns.screenwidth; m_pVertexes[base].texcoord[1] = rns.screenheight;
		base++;

		m_pVertexes[base].origin[0] = 1; m_pVertexes[base].origin[1] = 1; 
		m_pVertexes[base].origin[2] = -1; m_pVertexes[base].origin[3] = 1;
		m_pVertexes[base].texcoord[0] = rns.screenwidth; m_pVertexes[base].texcoord[1] = 0;

		Uint32 vertexcount = MAX_ACTIVE_PARTICLES*4+6;
		m_pVBO = new CVBO(gGLExtF, m_pVertexes, sizeof(particle_vertex_t)*vertexcount, indexes, sizeof(Uint32)*MAX_ACTIVE_PARTICLES*6);

		for(Uint32 i = 0; i < MAX_ACTIVE_PARTICLES*4; i++)
			m_pVertexes[i].origin[3] = 1.0;

		delete[] indexes;

		m_pShader->SetVBO(m_pVBO);
	}

	return true;
}

//====================================
//
//====================================
void CParticleEngine::ClearGL( void ) 
{
	if(m_pShader)
	{
		delete m_pShader;
		m_pShader = nullptr;
	}

	if(m_pVBO)
	{
		delete m_pVBO;
		m_pVBO = nullptr;
	}
}

//====================================
//
//====================================
bool CParticleEngine::InitGame( void ) 
{
	// Nothing here yet
	return true;
}

//====================================
//
//====================================
void CParticleEngine::ClearGame( void ) 
{
	if(!m_particleSystemsList.empty())
	{
		m_particleSystemsList.begin();
		while(!m_particleSystemsList.end())
		{
			particle_system_t *pfree = m_particleSystemsList.get();

			cl_particle_t *pparticle = pfree->pparticleheader;
			while(pparticle)
			{
				cl_particle_t *pfreeparticle = pparticle;
				pparticle = pfreeparticle->next;

				m_iNumFreedParticles++;
				RemoveParticle(pfreeparticle);
			}

			m_iNumFreedSystems++;
			delete pfree;

			m_particleSystemsList.remove(m_particleSystemsList.get_link());
			m_particleSystemsList.next();
		}

		m_particleSystemsList.clear();
	}

	if(!m_scriptCache.empty())
	{
		for(Uint32 i = 0; i < m_scriptCache.size(); i++)
			delete m_scriptCache[i];

		m_scriptCache.clear();
	}

	if(!m_msgCache.empty())
		m_msgCache.clear();
};

//====================================
//
//====================================
particle_system_t *CParticleEngine::AllocSystem( void ) 
{
	// Allocate memory
	particle_system_t *psystem = new particle_system_t;

	// Add system into pointer array
	m_particleSystemsList.add(psystem);

	m_iNumCreatedSystems++;
	return psystem;
}

//====================================
//
//====================================
__forceinline cl_particle_t *CParticleEngine::AllocParticle( particle_system_t *psystem ) 
{
	if(!m_pFreeParticles)
		return nullptr;

	cl_particle_t *pparticle = m_pFreeParticles;
	m_pFreeParticles = pparticle->next;
	(*pparticle) = cl_particle_t();

	// Add system into pointer array
	if(psystem->pparticleheader)
	{
		psystem->pparticleheader->prev = pparticle;
		pparticle->next = psystem->pparticleheader;
	}

	m_iNumCreatedParticles++;
	psystem->pparticleheader = pparticle;
	return pparticle;
}

//====================================
//
//====================================
void CParticleEngine::CreateCluster( const Char *szPath, const Vector& origin, const Vector& dir, Uint32 iId, cl_entity_t *pentity, entindex_t entindex, Uint32 attachment, Int32 boneindex, Int32 attachflags ) 
{
	const script_cache_t* pFile = PrecacheScript(PART_SCRIPT_CLUSTER, szPath, nullptr);
	if(!pFile)
	{
		Con_Printf("%s - Could not load cluster script file '%s'.\n", __FUNCTION__, szPath);
		return;
	}

	for(Uint32 i = 0; i < pFile->clusterscripts.size(); i++)
		CreateSystem(pFile->clusterscripts[i].c_str(), origin, dir, iId, nullptr, pentity, entindex, attachment, boneindex, attachflags);
}

//====================================
//
//====================================
particle_system_t *CParticleEngine::CreateSystem( const Char *szPath, const Vector& origin, const Vector& dir, Uint32 iId, particle_system_t *parent, cl_entity_t *entity, entindex_t entindex, Uint32 attachment, Int32 boneindex, Int32 attachflags ) 
{
	if(!qstrlen(szPath))
		return nullptr;

	if(entity && (!entity->pmodel || entity->pmodel->type != MOD_VBM))
	{
		Con_Printf("%s - Entity %d has no model, or model is not of VBM type!\n", __FUNCTION__, entindex);
		return nullptr;
	}

	const script_cache_t* pcache = PrecacheScript(PART_SCRIPT_SYSTEM, szPath, nullptr);
	if(!pcache)
	{
		Con_Printf("%s - Failed to spawn particle system.\n", __FUNCTION__);
		return nullptr;
	}

	particle_system_t *psystem = AllocSystem();
	if(!psystem)
	{
		Con_Printf("%s - Warning: Exceeded max number of particle systems.\n", __FUNCTION__);
		return nullptr;
	}

	// For short
	const script_definition_t* pdefinition = pcache->pdefinition;

	// Fill in default values
	psystem->entindex = entindex;
	psystem->id = iId;
	psystem->spawntime = rns.time;
	psystem->pdefinition = pdefinition;

	psystem->attachflags |= (attachflags | pdefinition->attachflags);
	psystem->particlefreq = pdefinition->particlefreq;
	psystem->maxparticles = pdefinition->maxparticles;
	psystem->ptexture = pdefinition->ptexture;

	if(entity)
	{
		psystem->parententity = entity;
		psystem->attachment = attachment;
		psystem->boneindex = boneindex;
	}

	// Determine bounds
	psystem->radius = pdefinition->systemsize;
	if(pdefinition->maxlife != -1)
		psystem->radius += pdefinition->maxvel+pdefinition->scale*pdefinition->scalevar;

	if(pdefinition->scaledampfactor < 0)
		psystem->radius += pdefinition->scale*(1.0-(pdefinition->maxlife+pdefinition->maxlifevar-pdefinition->scaledampdelay)*pdefinition->scaledampfactor);

	if(psystem->radius < pdefinition->systemsize)
		psystem->radius = pdefinition->systemsize;
	
	Vector systemorigin;
	Vector systemdir;

	if(entity)
	{
		// add in any flags supplied by user
		psystem->attachflags |= attachflags;

		if((psystem->attachflags & PARTICLE_ATTACH_TO_PARENT))
		{
			TransformRelativeVector(origin, psystem, systemorigin, true, true);
			TransformRelativeVector(dir, psystem, systemdir, false, true);
		}
		else
		{
			// Take defaults
			Math::VectorCopy(origin, systemorigin);
			Math::VectorCopy(dir, systemdir);
		}
	}
	else
	{
		// Take defaults
		Math::VectorCopy(origin, systemorigin);
		Math::VectorCopy(dir, systemdir);
	}

	if(pdefinition->shapetype != shape_playerplane)
	{
		Math::VectorCopy(systemorigin, psystem->origin);
		Math::VectorCopy(systemdir, psystem->dir);

		if(!parent && !entity)
		{
			Vector vmins, vmaxs;
			for(Uint32 i = 0; i < 3; i++)
			{
				vmins[i] = psystem->origin[i]-psystem->radius;
				vmaxs[i] = psystem->origin[i]+psystem->radius;
			}

			// Reserve this many positions to speed up processing
			psystem->leafnums.reserve(MAX_EDICT_LEAFNUMS);

			// Find touched nodes
			Mod_FindTouchedLeafs(ens.pworld, psystem->leafnums, vmins, vmaxs, ens.pworld->pnodes);

			// Resize to final size
			psystem->leafnums.resize(psystem->leafnums.size());
		}
		else if(parent)
		{
			// Just copy from parent
			psystem->leafnums = parent->leafnums;
		}
	}
	else
	{
		// Playerplane needs to find sky brush above
		trace_t tr;
		CL_PlayerTrace(origin, origin + Vector(0, 0, 8496), FL_TRACE_WORLD_ONLY, HULL_POINT, NO_ENTITY_INDEX, tr);

		if( tr.fraction == 1.0 || CL_PointContents(tr.endpos, nullptr) != CONTENTS_SKY )
		{
			m_particleSystemsList.remove(psystem);
			return nullptr;
		}

		psystem->skyheight = tr.endpos.z;
	}

	if(!parent)
	{
		// Create a slave system to hold particles spawned
		if(pdefinition->collision != collide_decal)
		{
			if(!pdefinition->create.empty())
				psystem->createsystem = CreateSystem(pdefinition->create.c_str(), psystem->origin, psystem->dir, iId, psystem, entity, entindex);
		}

		// Create a slave system for water impact particles
		if(!pdefinition->watercreate.empty())
			psystem->watersystem = CreateSystem(pdefinition->watercreate.c_str(), psystem->origin, psystem->dir, iId, psystem, entity, entindex);
	}
	else
	{
		// Child systems cannot spawn on their own
		psystem->parentsystem = parent;
		psystem->maxparticles = 0;
		psystem->particlefreq = 0;
		psystem->spawned = true;
	}

	return psystem;
}

//====================================
//
//====================================
bool CParticleEngine::ReadField( script_definition_t* pdefinition, const Char* pstrField, const Char* pstrNextRead )
{
	Char token[MAX_PARSE_LENGTH];
	token[0] = '\0';

	if(!qstrcmp(pstrField, "$shape"))
	{
		if(!pstrNextRead)
		{
			Con_Printf("%s - Incomplete field '%s'.\n", __FUNCTION__, pstrField);
			return false;
		}

		// Read in token
		Common::Parse(pstrNextRead, token);

		if(!qstrcmp(token, "point") || !qstrcmp(token, "default"))
			pdefinition->shapetype = shape_point;
		else if(!qstrcmp(token, "box"))
			pdefinition->shapetype = shape_box;
		else if(!qstrcmp(token, "playerplane"))
			pdefinition->shapetype = shape_playerplane;
		else
		{
			Con_Printf("%s - Invalid value '%s' for field '%s'.\n", __FUNCTION__, token, pstrField);
			return false;
		}

		return true;
	}
	else if(!qstrcmp(pstrField, "$flags"))
	{
		if(!pstrNextRead)
		{
			Con_Printf("%s - Incomplete field '%s'.\n", __FUNCTION__, pstrField);
			return false;
		}

		const Char* pstr = pstrNextRead;
		while(pstr)
		{
			pstr = Common::Parse(pstr, token);

			if(!qstrcmp(token, "random_direction"))
				pdefinition->flags |= SYSTEM_FL_RANDOM_DIR;
			else if(!qstrcmp(token, "globs"))
				pdefinition->flags |= SYSTEM_FL_GLOBS;
			else if(!qstrcmp(token, "softoff"))
				pdefinition->flags |= SYSTEM_FL_SOFTOFF;
			else
				Con_Printf("%s - Unrecognized flag '%s' for field '%s'.\n", __FUNCTION__, token, pstrField);
		}

		return true;
	}
	else if(!qstrcmp(pstrField, "$primary_color")
		|| !qstrcmp(pstrField, "$secondary_color"))
	{
		if(!pstrNextRead)
		{
			Con_Printf("%s - Incomplete field '%s'.\n", __FUNCTION__, pstrField);
			return false;
		}

		Vector color;
		// Parse R color element
		const Char* pstr = Common::Parse(pstrNextRead, token);
		if(!pstr)
		{
			Con_Printf("%s - Incomplete definition for field '%s'.\n", __FUNCTION__, pstrField);
			return false;
		}

		color.x = (Float)SDL_atoi(token) / 255.0f;

		// Parse G color element
		pstr = Common::Parse(pstr, token);
		if(!pstr)
		{
			Con_Printf("%s - Incomplete definition for field '%s'.\n", __FUNCTION__, pstrField);
			return false;
		}

		color.y = (Float)SDL_atoi(token) / 255.0f;

		// Parse B color element
		pstr = Common::Parse(pstr, token);

		color.z = (Float)SDL_atoi(token) / 255.0f;

		if(!qstrcmp(pstrField, "$primary_color"))
			pdefinition->primarycolor = color;
		else
			pdefinition->secondarycolor = color;

		return true;
	}
	else if(!qstrcmp(pstrField, "$lighting"))
	{
		if(!pstrNextRead)
		{
			Con_Printf("%s - Incomplete field '%s'.\n", __FUNCTION__, pstrField);
			return false;
		}

		const Char* pstr = pstrNextRead;
		while(pstr)
		{
			pstr = Common::Parse(pstr, token);

			if(!qstrcmp(token, "normal"))
				pdefinition->lighting_flags |= PARTICLE_LIGHTCHECK_NORMAL;
			else if(!qstrcmp(token, "mix_primary_color"))
				pdefinition->lighting_flags |= PARTICLE_LIGHTCHECK_MIXP;
			else if(!qstrcmp(token, "use_intensity"))
				pdefinition->lighting_flags |= PARTICLE_LIGHTCHECK_INTENSITY;
			else if(!qstrcmp(token, "only_once"))
				pdefinition->lighting_flags |= PARTICLE_LIGHTCHECK_ONLYONCE;
			else if(!qstrcmp(token, "none"))
				pdefinition->lighting_flags = PARTICLE_LIGHTCHECK_NONE;
			else if(!qstrcmp(token, "nodynlights"))
				pdefinition->lighting_flags |= PARTICLE_LIGHTCHECK_NO_DYNLIGHTS;
			else
				Con_Printf("%s - Unrecognized flag '%s' for field '%s'.\n", __FUNCTION__, token, pstrField);
		}

		return true;
	}
	else if(!qstrcmp(pstrField, "$collision"))
	{
		if(!pstrNextRead)
		{
			Con_Printf("%s - Incomplete field '%s'.\n", __FUNCTION__, pstrField);
			return false;
		}

		// Read in token
		Common::Parse(pstrNextRead, token);

		if(!qstrcmp(token, "none") || !qstrcmp(token, "default"))
			pdefinition->collision = collide_none;
		else if(!qstrcmp(token, "die"))
			pdefinition->collision = collide_die;
		else if(!qstrcmp(token, "bounce"))
			pdefinition->collision = collide_bounce;
		else if(!qstrcmp(token, "decal"))
			pdefinition->collision = collide_decal;
		else if(!qstrcmp(token, "stuck"))
			pdefinition->collision = collide_stuck;
		else if(!qstrcmp(token, "create_system"))
			pdefinition->collision = collide_new_system;
		else
		{
			Con_Printf("%s - Invalid value '%s' for field '%s'.\n", __FUNCTION__, token, pstrField);
			return false;
		}

		return true;
	}
	else if(!qstrcmp(pstrField, "$collision_flags"))
	{
		if(!pstrNextRead)
		{
			Con_Printf("%s - Incomplete field '%s'.\n", __FUNCTION__, pstrField);
			return false;
		}

		const Char* pstr = pstrNextRead;
		while(pstr)
		{
			pstr = Common::Parse(pstr, token);

			if(!qstrcmp(token, "precise"))
				pdefinition->collision_flags |= COLLISION_FL_PRECISE;
			else if(!qstrcmp(token, "collide_brushmodels"))
				pdefinition->collision_flags |= COLLISION_FL_BMODELS;
			else if(!qstrcmp(token, "collide_water"))
				pdefinition->collision_flags |= COLLISION_FL_WATER;
			else
				Con_Printf("%s - Unrecognized flag '%s' for field '%s'.\n", __FUNCTION__, token, pstrField);
		}

		return true;
	}
	else if(!qstrcmp(pstrField, "$rendermode"))
	{
		if(!pstrNextRead)
		{
			Con_Printf("%s - Incomplete field '%s'.\n", __FUNCTION__, pstrField);
			return false;
		}

		// Read in token
		Common::Parse(pstrNextRead, token);

		if(!qstrcmp(token, "additive") || !qstrcmp(token, "default"))
			pdefinition->rendermode = render_additive;
		else if(!qstrcmp(token, "alphablend"))
			pdefinition->rendermode = render_alpha;
		else if(!qstrcmp(token, "refractive"))
			pdefinition->rendermode = render_distort;
		else if(!qstrcmp(token, "alphatest"))
			pdefinition->rendermode = render_alphatest;
		else
		{
			Con_Printf("%s - Invalid value '%s' for field '%s'.\n", __FUNCTION__, token, pstrField);
			return false;
		}

		return true;
	}
	else if(!qstrcmp(pstrField, "$alignment"))
	{
		if(!pstrNextRead)
		{
			Con_Printf("%s - Incomplete field '%s'.\n", __FUNCTION__, pstrField);
			return false;
		}

		// Read in token
		Common::Parse(pstrNextRead, token);

		if(!qstrcmp(token, "tiled") || !qstrcmp(token, "default"))
			pdefinition->alignment = align_tiled;
		else if(!qstrcmp(token, "parallel"))
			pdefinition->alignment = align_parallel;
		else if(!qstrcmp(token, "to_normal"))
			pdefinition->alignment = align_normal;
		else if(!qstrcmp(token, "tracer"))
			pdefinition->alignment = align_tracer;
		else
		{
			Con_Printf("%s - Invalid value '%s' for field '%s'.\n", __FUNCTION__, token, pstrField);
			return false;
		}

		return true;
	}
	else if(!qstrcmp(pstrField, "$render_flags"))
	{
		if(!pstrNextRead)
		{
			Con_Printf("%s - Incomplete field '%s'.\n", __FUNCTION__, pstrField);
			return false;
		}

		const Char* pstr = pstrNextRead;
		while(pstr)
		{
			pstr = Common::Parse(pstr, token);

			if(!qstrcmp(token, "overbright"))
				pdefinition->render_flags |= RENDER_FL_OVERBRIGHT;
			else if(!qstrcmp(token, "skybox"))
				pdefinition->render_flags |= RENDER_FL_SKYBOX;
			else if(!qstrcmp(token, "nocull"))
				pdefinition->render_flags |= RENDER_FL_NOCULL;
			else if(!qstrcmp(token, "nofog"))
				pdefinition->render_flags |= RENDER_FL_NOFOG;
			else
				Con_Printf("%s - Unrecognized flag '%s' for field '%s'.\n", __FUNCTION__, token, pstrField);
		}

		return true;
	}
	else if(!qstrcmp(pstrField, "$wind_type"))
	{
		if(!pstrNextRead)
		{
			Con_Printf("%s - Incomplete field '%s'.\n", __FUNCTION__, pstrField);
			return false;
		}

		// Read in token
		Common::Parse(pstrNextRead, token);

		if(!qstrcmp(token, "linear"))
			pdefinition->windtype = wind_linear;
		else if(!qstrcmp(token, "sine"))
			pdefinition->windtype = wind_sine;
		else if(!qstrcmp(token, "none"))
			pdefinition->windtype = wind_none;
		else
		{
			Con_Printf("%s - Invalid value '%s' for field '%s'.\n", __FUNCTION__, token, pstrField);
			return false;
		}

		return true;
	}
	else if(!qstrcmp(pstrField, "$attachment_flags"))
	{
		if(!pstrNextRead)
		{
			Con_Printf("%s - Incomplete field '%s'.\n", __FUNCTION__, pstrField);
			return false;
		}

		const Char* pstr = pstrNextRead;
		while(pstr)
		{
			pstr = Common::Parse(pstr, token);

			if(!qstrcmp(token, "to_parent"))
				pdefinition->attachflags |= PARTICLE_ATTACH_TO_PARENT;
			else if(!qstrcmp(token, "relative"))
				pdefinition->attachflags |= PARTICLE_ATTACH_RELATIVE;
			else if(!qstrcmp(token, "to_attachment"))
				pdefinition->attachflags |= PARTICLE_ATTACH_TO_ATTACHMENT;
			else if(!qstrcmp(token, "attachment_vector"))
				pdefinition->attachflags |= PARTICLE_ATTACH_ATTACHMENT_VECTOR;
			else if(!qstrcmp(token, "to_bone"))
				pdefinition->attachflags |= PARTICLE_ATTACH_TO_BONE;
			else if(!qstrcmp(token, "none"))
				pdefinition->attachflags = PARTICLE_ATTACH_NONE;
			else
				Con_Printf("%s - Unrecognized flag '%s' for field '%s'.\n", __FUNCTION__, token, pstrField);
		}

		return true;
	}
	else
	{
		if(!pstrNextRead)
		{
			Con_Printf("%s - Incomplete field '%s'.\n", __FUNCTION__, pstrField);
			return false;
		}

		// Read in token
		Common::Parse(pstrNextRead, token);

		// All fields from here on are single-token fields
		if(!qstrcmp(pstrField, "$min_velocity"))
			pdefinition->minvel = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$max_velocity"))
			pdefinition->maxvel = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$max_offset"))
			pdefinition->maxofs = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$fade_in_time"))
			pdefinition->fadeintime = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$fade_out_delay"))
			pdefinition->fadeoutdelay = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$alpha"))
			pdefinition->mainalpha = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$velocity_damping"))
			pdefinition->velocitydamp = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$velocity_damping_delay"))
			pdefinition->veldampdelay = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$lifetime"))
			pdefinition->maxlife = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$lifetime_variation"))
			pdefinition->maxlifevar = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$color_transition_delay"))
			pdefinition->transitiondelay = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$color_transition_duration"))
			pdefinition->transitiontime = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$color_transition_variance"))
			pdefinition->transitionvar = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$scale"))
			pdefinition->scale = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$scale_variation"))
			pdefinition->scalevar = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$scale_damping_delay"))
			pdefinition->scaledampdelay = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$scale_damping"))
			pdefinition->scaledampfactor = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$gravity"))
			pdefinition->gravity = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$system_size"))
			pdefinition->systemsize = SDL_atoi(token);
		else if(!qstrcmp(pstrField, "$max_particles"))
			pdefinition->maxparticles = SDL_atoi(token);
		else if(!qstrcmp(pstrField, "$particle_frequency"))
			pdefinition->particlefreq = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$particles_on_spawn"))
			pdefinition->startparticles = SDL_atoi(token);
		else if(!qstrcmp(pstrField, "$particle_frequency_variation"))
			pdefinition->maxparticlevar = SDL_atoi(token);
		else if(!qstrcmp(pstrField, "$impact_velocity_dampening"))
			pdefinition->impactdamp = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$rotate_z_variation"))
			pdefinition->rotationvar = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$rotate_z_speed"))
			pdefinition->rotationvel = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$rotate_z_dampening"))
			pdefinition->rotationdamp = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$rotate_z_dampening_delay"))
			pdefinition->rotationdampdelay = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$rotate_x_variation"))
			pdefinition->rotxvar = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$rotate_x_speed"))
			pdefinition->rotxvel = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$rotate_x_dampening"))
			pdefinition->rotxdamp = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$rotate_x_dampening_delay"))
			pdefinition->rotxdampdelay = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$rotate_y_variation"))
			pdefinition->rotyvar = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$rotate_y_speed"))
			pdefinition->rotyvel = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$rotate_y_dampening"))
			pdefinition->rotydamp = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$rotate_y_dampening_delay"))
			pdefinition->rotydampdelay = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$create"))
			pdefinition->create = token;
		else if(!qstrcmp(pstrField, "$create_on_death"))
			pdefinition->deathcreate = token;
		else if(!qstrcmp(pstrField, "$create_on_water_impact"))
			pdefinition->watercreate = token;
		else if(!qstrcmp(pstrField, "$wind_x_velocity"))
			pdefinition->windx = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$wind_y_velocity"))
			pdefinition->windy = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$wind_velocity_variance"))
			pdefinition->windvar = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$wind_sine_variance_speed_multiplier"))
			pdefinition->windmult = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$wind_sine_min_variance"))
			pdefinition->minwindmult = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$wind_sine_variance"))
			pdefinition->windmultvar = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$stuck_death_time"))
			pdefinition->stuckdie = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$playerplane_max_height"))
			pdefinition->maxheight = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$tracer_distance"))
			pdefinition->tracerdist = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$fade_near_distance"))
			pdefinition->fadedistnear = SDL_atoi(token);
		else if(!qstrcmp(pstrField, "$fade_far_distance"))
			pdefinition->fadedistfar = SDL_atoi(token);
		else if(!qstrcmp(pstrField, "$num_frames"))
			pdefinition->numframes = SDL_atoi(token);
		else if(!qstrcmp(pstrField, "$frame_width"))
			pdefinition->framesizex = SDL_atoi(token);
		else if(!qstrcmp(pstrField, "$frame_height"))
			pdefinition->framesizey = SDL_atoi(token);
		else if(!qstrcmp(pstrField, "$framerate"))
			pdefinition->framerate = SDL_atoi(token);
		else if(!qstrcmp(pstrField, "$chance_to_create"))
			pdefinition->spawnchance = SDL_atoi(token);
		else if(!qstrcmp(pstrField, "$min_light_value"))
			pdefinition->minlight = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$max_light_value"))
			pdefinition->maxlight = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$glob_size"))
			pdefinition->globsize = SDL_atoi(token);
		else if(!qstrcmp(pstrField, "$num_glob_particles"))
			pdefinition->numglobparticles = SDL_atoi(token);
		else if(!qstrcmp(pstrField, "$soft_turnoff_duration"))
			pdefinition->softofftime = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$decal_lifetime"))
			pdefinition->decallife = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$decal_fade_time"))
			pdefinition->decalfade = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$decal_growth_time"))
			pdefinition->decalgrowthtime = SDL_atof(token);
		else if(!qstrcmp(pstrField, "$texture"))
		{
			CString texturepath;
			texturepath << PARTICLE_TEXTURES_PATH << token << ".dds";

			pdefinition->ptexture = CTextureManager::GetInstance()->LoadTexture(texturepath.c_str(), RS_GAME_LEVEL);
			if(!pdefinition->ptexture)
			{
				// Load failed
				return false;
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, pdefinition->ptexture->palloc->gl_index);
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
				glBindTexture(GL_TEXTURE_2D, 0);
			}
		}
		else
		{
			Con_Printf("%s - Unrecognized field '%s' in '%s'\n", __FUNCTION__, pstrField, pstrField);
		}

		return true;
	}
}

//====================================
//
//====================================
bool CParticleEngine::LoadSystemScript( script_cache_t* pCache, const Char* pstrData )
{
	pCache->pdefinition = new script_definition_t;

	// Fill in default values
	pCache->pdefinition->mainalpha = 1.0;

	// Holds the currently read line
	static Char line[MAX_LINE_LENGTH];
	// Token we've read
	static Char token[MAX_PARSE_LENGTH];

	// First token must be "$particlescript"
	const Char *pstr = Common::Parse(pstrData, token);
	if(!pstr)
	{
		Con_Printf("%s - Particle script '%s' is incomplete after '%s'.\n", 
			__FUNCTION__, pCache->name.c_str(), token);
		return nullptr;
	}

	if(qstrcmp(token, "$particlescript"))
	{
		Con_Printf("%s - Expected '$particlescript' token at beginning of script '%s', got '%s' instead.\n", 
			__FUNCTION__, pCache->name.c_str(), token);
		return nullptr;
	}

	// Second token must be '{'
	pstr = Common::Parse(pstr, token);
	if(!pstr)
	{
		Con_Printf("%s - Particle script '%s' is incomplete after '%s'.\n", 
			__FUNCTION__, pCache->name.c_str(), token);
		return nullptr;
	}

	if(qstrcmp(token, "{"))
	{
		Con_Printf("%s - Expected '{' token at beginning of script '%s', got '%s' instead.\n", 
			__FUNCTION__, pCache->name.c_str(), token);
		return nullptr;
	}

	// Skip ahead to next non-space
	while(SDL_isspace(*pstr))
		pstr++;

	// Begin reading the lines one by one
	while(true)
	{
		if(!pstr)
		{
			Con_Printf("%s - Particle script '%s' is incomplete, missing '}' closing bracket.\n", 
				__FUNCTION__, pCache->name.c_str());
			return nullptr;
		}

		// Read this line
		pstr = Common::ReadLine(pstr, line);

		// Skip if line is empty
		if(!qstrlen(line))
			continue;

		// Read first token
		const Char* plstr = Common::Parse(line, token);

		// Stop if reached end
		if(!qstrcmp(token, "}"))
			break;

		// Skip any comments
		if(!qstrncmp(token, "//", 2))
			continue;

		// Token must always have '$' at beginning
		if(token[0] != '$')
		{
			Con_Printf("%s - Invalid field definition for '%s' in script '%s', fields must begin with '$'.\n", 
				__FUNCTION__, token, pCache->name.c_str());
			continue;
		}

		// Read in the field
		if(!ReadField(pCache->pdefinition, token, plstr))
		{
			Con_Printf("%s - Error reading field '%s' in script '%s'.\n", 
				__FUNCTION__, token, pCache->name.c_str());
			continue;
		}
	}

	// Disable light checking on distortion particles
	script_definition_t* pdefinition = pCache->pdefinition;
	if(pdefinition->rendermode == render_distort 
		&& pdefinition->lighting_flags != PARTICLE_LIGHTCHECK_NONE)
	{
		pdefinition->lighting_flags = PARTICLE_LIGHTCHECK_NONE;
		pdefinition->primarycolor = Vector(1, 1, 1);
	}

	// Avoid invalid states
	if(pdefinition->maxparticles != -1 && pdefinition->maxparticles < 0)
	{
		Con_Printf("%s - Invalid value '%d' for '$max_particles' in script '%s'.\n", __FUNCTION__,
			pdefinition->maxparticles, pCache->name.c_str());
		return false;
	}

	// Avoid invalid state for particle frequency
	if(pdefinition->particlefreq < 0)
	{
		Con_Printf("%s - Invalid value '%f' for '$particle_frequency' in script '%s'.\n", __FUNCTION__,
			pdefinition->particlefreq, pCache->name.c_str());
		return false;
	}

	return true;
}

//====================================
//
//====================================
bool CParticleEngine::LoadClusterScript( script_cache_t* pCache, const Char* pstrData )
{
	// Holds the currently read line
	static Char line[MAX_LINE_LENGTH];
	// Token we've read
	static Char token[MAX_PARSE_LENGTH];

	// First token must be "$clusterscript"
	const Char *pstr = Common::Parse(pstrData, token);
	if(!pstr)
	{
		Con_Printf("%s - Particle script '%s' is incomplete after '%s'.\n", 
			__FUNCTION__, pCache->name.c_str(), token);
		return nullptr;
	}

	if(qstrcmp(token, "$clusterscript"))
	{
		Con_Printf("%s - Expected '$clusterscript' token at beginning of script '%s', got '%s' instead.\n", 
			__FUNCTION__, pCache->name.c_str(), token);
		return nullptr;
	}

	// Second token must be '{'
	pstr = Common::Parse(pstr, token);
	if(!pstr)
	{
		Con_Printf("%s - Particle script '%s' is incomplete after '%s'.\n", 
			__FUNCTION__, pCache->name.c_str(), token);
		return nullptr;
	}

	if(qstrcmp(token, "{"))
	{
		Con_Printf("%s - Expected '{' token at beginning of script '%s', got '%s' instead.\n", 
			__FUNCTION__, pCache->name.c_str(), token);
		return nullptr;
	}

	// Skip ahead to next non-space
	while(SDL_isspace(*pstr))
		pstr++;

	// Begin reading the lines one by one
	while(true)
	{
		if(!pstr)
		{
			Con_Printf("%s - Particle script '%s' is incomplete, missing '}' closing bracket.\n", 
				__FUNCTION__, pCache->name.c_str());
			return nullptr;
		}

		// Read this line
		pstr = Common::ReadLine(pstr, line);

		// Skip if line is empty
		if(!qstrlen(line))
			continue;

		// Read first token
		const Char* plstr = Common::Parse(line, token);

		// Stop if reached end
		if(!qstrcmp(token, "}"))
			break;

		// Skip any comments
		if(!qstrncmp(token, "//", 2))
			continue;

		if(!plstr)
		{
			Con_Printf("%s - Incomplete cluster script definition for '%s'.\n", __FUNCTION__, token);
			continue;
		}

		// Token must always have '$' at beginning
		if(token[0] != '$')
		{
			Con_Printf("%s - Invalid field definition for '%s' in script '%s', fields must begin with '$'.\n", 
				__FUNCTION__, token, pCache->name.c_str());
			continue;
		}

		if(qstrcmp(token, "$script"))
		{
			Con_Printf("%s - Expected '%script' token in '%s', got '%s' instead.\n", 
				__FUNCTION__, pCache->name.c_str(), token);
			continue;
		}

		// Read script name
		Common::Parse(plstr, token);

		// Read file and add to cache
		CArray<CString> loadList;
		const script_cache_t* pScript = PrecacheScript(PART_SCRIPT_SYSTEM, token, &loadList);
		if(!pScript)
		{
			Con_Printf("%s - Failed to load particle script '%s' for cluster script '%s'.\n",
				__FUNCTION__, token, pCache->name.c_str());
			continue;
		}
		
		// Add it to the list
		pCache->clusterscripts.push_back(token);
	}

	return pCache->clusterscripts.empty() ? false : true;
}

//====================================
//
//====================================
const script_cache_t *CParticleEngine::PrecacheScript( Int32 type, const Char *name, CArray<CString>* pLoadList ) 
{
	if(type != PART_SCRIPT_SYSTEM && type != PART_SCRIPT_CLUSTER)
	{
		Con_Printf("%s - Unknown script type %d for particle script '%s'.\n", __FUNCTION__, type, name);
		return nullptr;
	}

	CString filepath;
	filepath << PARTICLE_SCRIPT_PATH << name;

	for(Uint32 i = 0; i < m_scriptCache.size(); i++)
	{
		if(!qstrcmp(m_scriptCache[i]->name, filepath))
			return m_scriptCache[i];
	}

	if(pLoadList)
	{
		// Avoid loops
		for(Uint32 i = 0; i < pLoadList->size(); i++)
		{
			if(!qstrcmp((*pLoadList)[i], name))
				return nullptr;
		}

		// Remember this
		pLoadList->push_back(name);
	}

	if(rns.isgameready)
		Con_Printf("%s - Late precache of '%s'.\n", __FUNCTION__, filepath.c_str());

	const Char *pFile = reinterpret_cast<const Char *>(FL_LoadFile(filepath.c_str()));
	if(!pFile)
	{
		Con_Printf("%s - Could not load script '%s'.\n", __FUNCTION__, filepath.c_str());
		return nullptr;
	}

	script_cache_t *pcache = new script_cache_t;
	pcache->name = filepath;

	bool result = false;
	switch(type)
	{
	case PART_SCRIPT_CLUSTER:
		{
			result = LoadClusterScript(pcache, pFile);
		}
		break;
	default:
	case PART_SCRIPT_SYSTEM:
		{
			result = LoadSystemScript(pcache, pFile);

			CArray<CString> loadList;
			if(result && !pcache->pdefinition->create.empty()
				&& (pcache->pdefinition->collision == collide_new_system
				|| pcache->pdefinition->tracerdist > 0))
			{
				result = (PrecacheScript(PART_SCRIPT_SYSTEM, pcache->pdefinition->create.c_str(), &loadList) != nullptr) ? true : false;
				if(!result)
					Con_Printf("%s - Failed to load collision create script '%s'.\n", __FUNCTION__, pcache->pdefinition->create.c_str());
			}

			if(result && !pcache->pdefinition->watercreate.empty()
				&& (pcache->pdefinition->collision_flags & COLLISION_FL_WATER))
			{
				result = (PrecacheScript(PART_SCRIPT_SYSTEM, pcache->pdefinition->watercreate.c_str(), &loadList) != nullptr) ? true : false;
				if(!result)
					Con_Printf("%s - Failed to load water collision create script '%s'.\n", __FUNCTION__, pcache->pdefinition->watercreate.c_str());
			}

			if(result && !pcache->pdefinition->deathcreate.empty())
			{
				result = (PrecacheScript(PART_SCRIPT_SYSTEM, pcache->pdefinition->deathcreate.c_str(), &loadList) != nullptr) ? true : false;
				if(!result)
					Con_Printf("%s - Failed to load death create script '%s'.\n", __FUNCTION__, pcache->pdefinition->deathcreate.c_str());
			}
		}
		break;
	}

	FL_FreeFile(pFile);

	if(!result)
	{
		delete pcache;
		return nullptr;
	}

	// Add to cache
	m_scriptCache.push_back(pcache);
	return pcache;
}

//====================================
//
//====================================
void CParticleEngine::EnvironmentCreateFirst( particle_system_t *psystem ) 
{
	cl_entity_t* pplayer = CL_GetLocalPlayer();
	if(!pplayer)
		return;

	Vector vorigin;
	Vector vplayer = pplayer->curstate.origin;
	Uint32 inumparticles = psystem->particlefreq*2;

	// Pointer to script definition
	assert(psystem->pdefinition != nullptr);
	const script_definition_t* pdefinition = psystem->pdefinition;

	// Spawn particles inbetween the view origin and maxheight
	for(Uint32 i = 0; i < inumparticles; i++)
	{
		vorigin[0] = vplayer[0] + Common::RandomLong(-pdefinition->systemsize, pdefinition->systemsize);
		vorigin[1] = vplayer[1] + Common::RandomLong(-pdefinition->systemsize, pdefinition->systemsize);
		
		if(pdefinition->maxheight)
		{
			vorigin[2] = vplayer[2] + pdefinition->maxheight;

			if(vorigin[2] > psystem->skyheight)
				vorigin[2] = psystem->skyheight;
		}
		else
		{
			vorigin[2] = psystem->skyheight;
		}

		vorigin[2] = Common::RandomLong(vplayer[2], vorigin[2]);

		trace_t trace;
		CL_PlayerTrace(vorigin, Vector(vorigin[0], vorigin[1], psystem->skyheight-8), FL_TRACE_WORLD_ONLY, HULL_POINT, NO_ENTITY_INDEX, trace);

		if((trace.flags & FL_TR_ALLSOLID) || trace.fraction != 1.0)
			continue;

		CreateParticle(psystem, vorigin);
	}
}

//====================================
//
//====================================
cl_particle_t *CParticleEngine::CreateParticle( particle_system_t *psystem, Float *pflorigin, Float *pflnormal ) 
{
	static trace_t tr;
	Vector vbaseorigin, vtest;
	Vector vforward, vright, vup;

	// Pointer to script definition
	assert(psystem->pdefinition != nullptr);
	const script_definition_t* pdefinition = psystem->pdefinition;

	if(pdefinition->shapetype == shape_playerplane)
	{
		// Origin supplied as parameter can override
		// particle origin
		if(!pflorigin)
		{
			Vector vOffset;
			cl_entity_t *pplayer = CL_GetLocalPlayer();
			vOffset[0] = Common::RandomLong(-pdefinition->systemsize, pdefinition->systemsize);
			vOffset[1] = Common::RandomLong(-pdefinition->systemsize, pdefinition->systemsize);

			if(pdefinition->maxheight)
			{
				vOffset[2] = pplayer->curstate.origin[2] + pdefinition->maxheight;
				if(vOffset[2] > psystem->skyheight) vOffset[2] = psystem->skyheight;
			}
			else
			{
				vOffset[2] = psystem->skyheight;
			}

			vtest[0] = pplayer->curstate.origin[0]+vOffset[0];
			vtest[1] = pplayer->curstate.origin[1]+vOffset[1];
			vtest[2] = psystem->skyheight+16;

			Float flFrac = (vOffset.Length2D()/pdefinition->systemsize);
			if(flFrac > 1) 
				flFrac = 1;
			
			vOffset[2] = (1-(flFrac))*(vOffset[2]-pplayer->curstate.origin[2]);

			Vector testPosition = pplayer->curstate.origin + vOffset;
			CL_PlayerTrace(testPosition, vtest, FL_TRACE_WORLD_ONLY, HULL_POINT, NO_ENTITY_INDEX, tr);
			if(tr.fraction != 1.0 && CL_PointContents(tr.endpos, nullptr) != CONTENTS_SKY)
				return nullptr;

			Math::VectorCopy(testPosition, vbaseorigin);
		}
		else
		{
			Math::VectorCopy(pflorigin, vbaseorigin);
		}
	}
	else
	{
		if(psystem->parententity && (psystem->attachflags & PARTICLE_ATTACH_TO_PARENT))
		{
			if(psystem->attachflags & (PARTICLE_ATTACH_TO_ATTACHMENT|PARTICLE_ATTACH_ATTACHMENT_VECTOR))
				Math::VectorCopy(psystem->parententity->getAttachment(psystem->attachment), vbaseorigin);
			else if(psystem->attachflags & PARTICLE_ATTACH_TO_BONE)
				GetBoneTransformedPosition(psystem->origin, psystem->parententity, psystem->boneindex, vbaseorigin, false);
			else
				Math::VectorCopy(psystem->parententity->curstate.origin, vbaseorigin);
		}
		else if(pflorigin)
		{
			Math::VectorCopy(pflorigin, vbaseorigin);

			if(pflnormal)
				Math::VectorMA(vbaseorigin, 0.1, pflnormal, vbaseorigin);
		}
		else
		{
			Math::VectorCopy(psystem->origin, vbaseorigin);
		}

		if(pdefinition->shapetype == shape_box)
		{
			vbaseorigin[0] += Common::RandomLong(-pdefinition->systemsize, pdefinition->systemsize);
			vbaseorigin[1] += Common::RandomLong(-pdefinition->systemsize, pdefinition->systemsize);
			vbaseorigin[2] += Common::RandomLong(-pdefinition->systemsize, pdefinition->systemsize);
		}

		// Transform origin back to relative space
		if(psystem->parententity 
			&& (psystem->attachflags & PARTICLE_ATTACH_TO_PARENT)
			&& (psystem->attachflags & PARTICLE_ATTACH_RELATIVE))
			TransformRelativeVector(vbaseorigin, psystem, vbaseorigin, true, true);
	}

	cl_particle_t *pparticle = nullptr;
	Uint32 numspawn = (pdefinition->flags & SYSTEM_FL_GLOBS) ? pdefinition->numglobparticles : 1;

	for(Uint32 i = 0; i < numspawn; i++)
	{
		pparticle = AllocParticle(psystem);
		if(!pparticle)
			return nullptr;

		pparticle->spawntime = rns.time;
		pparticle->psystem = psystem;
		pparticle->frame = -1;

		if(pdefinition->flags & SYSTEM_FL_GLOBS)
		{
			vbaseorigin[0] += Common::RandomLong(-pdefinition->globsize, pdefinition->globsize);
			vbaseorigin[1] += Common::RandomLong(-pdefinition->globsize, pdefinition->globsize);
			vbaseorigin[2] += Common::RandomLong(-pdefinition->globsize, pdefinition->globsize);
		}

		Math::VectorCopy(vbaseorigin, pparticle->origin);

		if(pdefinition->shapetype == shape_playerplane && !psystem->spawned)
		{
			pparticle->fadein = 0;
		}
		else
		{
			pparticle->fadein = pdefinition->fadeintime;
		}

		if(pdefinition->shapetype == shape_playerplane)
		{
			vforward[0] = 0;
			vforward[1] = 0;
			vforward[2] = -1;
		}
		else if((psystem->attachflags & PARTICLE_ATTACH_TO_PARENT)
			&& (psystem->attachflags & PARTICLE_ATTACH_ATTACHMENT_VECTOR) 
			&& psystem->parententity)
		{
			cl_entity_t *pParent = psystem->parententity;
			Vector attach1 = pParent->getAttachment(psystem->attachment);
			Vector attach2 = pParent->getAttachment(psystem->attachment+1);

			vforward = (attach2-attach1).Normalize();
		}
		else if((psystem->attachflags & PARTICLE_ATTACH_TO_PARENT) 
			&& (psystem->attachflags & PARTICLE_ATTACH_TO_BONE)
			&& psystem->parententity)
		{
			GetBoneRotatedVector(psystem->dir, psystem->parententity, psystem->boneindex, vforward, false);
		}
		else if(pdefinition->flags & SYSTEM_FL_RANDOM_DIR)
		{
			vforward[0] = Common::RandomFloat(-1, 1);
			vforward[1] = Common::RandomFloat(-1, 1);
			vforward[2] = Common::RandomFloat(-1, 1);
		}
		else if(pflorigin && pflnormal)
		{
			vforward[0] = pflnormal[0];
			vforward[1] = pflnormal[1];
			vforward[2] = pflnormal[2];
		}
		else
		{
			vforward[0] = psystem->dir[0];
			vforward[1] = psystem->dir[1];
			vforward[2] = psystem->dir[2];
		}

		if(pflnormal)
		{
			pparticle->normal[0] = pflnormal[0];
			pparticle->normal[1] = pflnormal[1];
			pparticle->normal[2] = pflnormal[2];
		}
		else
		{
			pparticle->normal[0] = vforward[0];
			pparticle->normal[1] = vforward[1];
			pparticle->normal[2] = vforward[2];
		}

		Math::VectorClear(vup);
		Math::VectorClear(vright);

		vforward.Normalize();
		Math::GetUpRight(vforward, vup, vright);

		Math::VectorMA(pparticle->velocity, Common::RandomFloat(pdefinition->minvel, pdefinition->maxvel), vforward, pparticle->velocity);
		Math::VectorMA(pparticle->velocity, Common::RandomFloat(-pdefinition->maxofs, pdefinition->maxofs), vright, pparticle->velocity);
		Math::VectorMA(pparticle->velocity, Common::RandomFloat(-pdefinition->maxofs, pdefinition->maxofs), vup, pparticle->velocity);

		// Rotate velocity back to base vector
		if(psystem->parententity 
			&& (psystem->attachflags & PARTICLE_ATTACH_TO_PARENT)
			&& (psystem->attachflags & PARTICLE_ATTACH_RELATIVE))
			TransformRelativeVector(pparticle->velocity, psystem, pparticle->velocity, false, true);

		if(pdefinition->maxlife == -1)
			pparticle->life = pdefinition->maxlife;
		else
			pparticle->life = rns.time + pdefinition->maxlife + Common::RandomFloat(-pdefinition->maxlifevar, pdefinition->maxlifevar);
	
		pparticle->scale = pdefinition->scale + Common::RandomFloat(-pdefinition->scalevar, pdefinition->scalevar);
		pparticle->rotationvel = pdefinition->rotationvel + Common::RandomFloat(-pdefinition->rotationvar, pdefinition->rotationvar);
		pparticle->rotxvel = pdefinition->rotxvel + Common::RandomFloat(-pdefinition->rotxvar, pdefinition->rotxvar);
		pparticle->rotyvel = pdefinition->rotyvel + Common::RandomFloat(-pdefinition->rotyvar, pdefinition->rotyvar);

		if(pparticle->rotationvel)
			pparticle->rotation = Common::RandomFloat(0, 360);

		if(pparticle->rotxvel)
			pparticle->rotx = Common::RandomFloat(0, 360);

		if(pparticle->rotyvel)
			pparticle->roty = Common::RandomFloat(0, 360);

		if(!pparticle->fadein)
			pparticle->alpha = 1;

		if(pdefinition->fadeoutdelay)
			pparticle->fadeoutdelay = pdefinition->fadeoutdelay;

		if(pdefinition->scaledampdelay)
			pparticle->scaledampdelay = rns.time + pdefinition->scaledampdelay + Common::RandomFloat(-pdefinition->scalevar, pdefinition->scalevar);
	
		if(pdefinition->transitiondelay && pdefinition->transitiontime)
		{
			pparticle->secondarydelay = rns.time + pdefinition->transitiondelay + Common::RandomFloat(-pdefinition->transitionvar, pdefinition->transitionvar);
			pparticle->secondarytime = pdefinition->transitiontime + Common::RandomFloat(-pdefinition->transitionvar, pdefinition->transitionvar);
		}

		if(pdefinition->windtype)
		{
			pparticle->windmult = pdefinition->windmult + Common::RandomFloat(-pdefinition->windmultvar, pdefinition->windmultvar);
			pparticle->windxvel = pdefinition->windx + Common::RandomFloat(-pdefinition->windvar, pdefinition->windvar);
			pparticle->windyvel = pdefinition->windy + Common::RandomFloat(-pdefinition->windvar, pdefinition->windvar);
		}

		if(!pdefinition->numframes)
		{
			pparticle->texcoords[0][0] = 0; pparticle->texcoords[0][1] = 0;
			pparticle->texcoords[1][0] = 1; pparticle->texcoords[1][1] = 0;
			pparticle->texcoords[2][0] = 1; pparticle->texcoords[2][1] = 1;
			pparticle->texcoords[3][0] = 0; pparticle->texcoords[3][1] = 1;
		}
		else
		{
			Int32 iframe;
			if(!pdefinition->framerate)
			{
				iframe = pparticle->frame = Common::RandomLong(0, pdefinition->numframes-1);
			}
			else
			{
				// Get desired frame
				iframe = ((Int32)((rns.time - pparticle->spawntime)*pdefinition->framerate));
				iframe = iframe % pdefinition->numframes;
			}

			en_texture_t *pTexture = psystem->ptexture;

			Int32	numframesx = pTexture->width/pdefinition->framesizex;
			Int32	numframesy = pTexture->height/pdefinition->framesizey;

			Int32 column = iframe%numframesx;
			Int32 row = (iframe/numframesx)%numframesy;

			// Calculate these only once
			Float fracwidth = (Float)pdefinition->framesizex/(Float)pTexture->width;
			Float fracheight = (Float)pdefinition->framesizey/(Float)pTexture->height;

			// Calculate top left coordinate
			pparticle->texcoords[0][0] = (column+1)*fracwidth;
			pparticle->texcoords[0][1] = row*fracheight;

			// Calculate top right coordinate
			pparticle->texcoords[1][0] = column*fracwidth;
			pparticle->texcoords[1][1] = row*fracheight;

			// Calculate bottom right coordinate
			pparticle->texcoords[2][0] = column*fracwidth;
			pparticle->texcoords[2][1] = (row+1)*fracheight;

			// Calculate bottom left coordinate
			pparticle->texcoords[3][0] = (column+1)*fracwidth;
			pparticle->texcoords[3][1] = (row+1)*fracheight;

			// Fill in current frame
			pparticle->frame = iframe;
		}

		Math::VectorCopy(pdefinition->primarycolor, pparticle->color);
		Math::VectorCopy(pdefinition->secondarycolor, pparticle->scolor);
		Math::VectorCopy(pparticle->origin, pparticle->lastspawn);

		for(Uint32 j = 0; j < 3; j++)
		{
			if(pparticle->scolor[j] == -1)
				pparticle->scolor[j] = Common::RandomFloat(0, 1);
		}

		if(pdefinition->lighting_flags != PARTICLE_LIGHTCHECK_NONE)
		{
			Mod_RecursiveLightPoint(ens.pworld, ens.pworld->pnodes, pparticle->origin, pparticle->origin - Vector(0, 0, 2048), pparticle->lightcol);
			Math::VectorCopy(pparticle->origin, pparticle->last_light);
			Math::VectorCopy(pparticle->lightcol, pparticle->lightmap);

			if(pdefinition->maxlight || pdefinition->minlight)
			{
				Float illum = (pparticle->lightcol[0] + pparticle->lightcol[1] + pparticle->lightcol[2])/3;

				if(pdefinition->minlight)
				{
					if(illum < pdefinition->minlight)
						Math::VectorScale(pparticle->lightcol, pdefinition->minlight/illum, pparticle->lightcol);
				}

				if(pdefinition->maxlight)
				{
					if(illum > pdefinition->maxlight)
						Math::VectorScale(pparticle->lightcol, pdefinition->maxlight/illum, pparticle->lightcol);
				}
			}
		}
	}

	return (pdefinition->flags & SYSTEM_FL_GLOBS) ? nullptr : pparticle;
}

//====================================
//
//====================================
void CParticleEngine::Update( void ) 
{
	if(!m_msgCache.empty())
	{
		m_msgCache.begin();
		while(!m_msgCache.end())
		{
			part_msg_cache_t *pcache = &m_msgCache.get();

			cl_entity_t *pentity = nullptr;
			if(pcache->entindex)
			{
				pentity = CL_GetEntityByIndex(pcache->entindex);
				if(!pentity || !pentity->pmodel)
					pentity = nullptr;
			}

			if(pcache->msgtype == PART_MSG_REMOVE)
			{
				RemoveSystem(pcache->entindex, pcache->id, pcache->keepcached); 
			}
			else
			{
				switch(pcache->scripttype)
				{
				case PART_SCRIPT_CLUSTER: 
					CreateCluster(pcache->file.c_str(), pcache->origin, pcache->direction, pcache->id, pentity, pcache->entindex, pcache->attachment, pcache->boneindex, pcache->attachflags); 
					break;
				case PART_SCRIPT_SYSTEM: 
					CreateSystem(pcache->file.c_str(), pcache->origin, pcache->direction, pcache->id, nullptr, pentity, pcache->entindex, pcache->attachment, pcache->boneindex, pcache->attachflags); 
					break;
				}
			}

			m_msgCache.next();
		}

		m_msgCache.clear();
	}

	if(m_pCvarParticleDebug->GetValue() > 0)
	{
		Con_Printf("Created Particles: %i, Freed Particles %i, Active Particles: %i\nCreated Systems: %i, Freed Systems: %i, Active Systems: %i\n\n", 
			m_iNumCreatedParticles, m_iNumFreedParticles,m_iNumCreatedParticles-m_iNumFreedParticles, m_iNumCreatedSystems, m_iNumFreedSystems, m_iNumCreatedSystems-m_iNumFreedSystems);
	}

	if(m_pCvarDrawParticles->GetValue() < 1)
		return;

	// No systems to check on
	if(m_particleSystemsList.empty())
		return;

	UpdateSystems();

	// Update all particles
	m_particleSystemsList.begin();
	while(!m_particleSystemsList.end())
	{
		particle_system_t *psystem = m_particleSystemsList.get();
		cl_particle_t *pparticle = psystem->pparticleheader;

		while(pparticle)
		{
			if(!UpdateParticle(pparticle))
			{
				cl_particle_t *pfree = pparticle;
				pparticle = pfree->next;

				m_iNumFreedParticles++;
				RemoveParticle(pfree);
				continue;
			}
			cl_particle_t *pnext = pparticle->next;
			pparticle = pnext;
		}

		m_particleSystemsList.next();
	}
}

//====================================
//
//====================================
void CParticleEngine::UpdateSystems( void ) 
{
	// Handle parented ones first
	m_particleSystemsList.begin();
	while(!m_particleSystemsList.end())
	{
		particle_system_t *psystem = m_particleSystemsList.get();
		if(!psystem->parententity)
		{
			m_particleSystemsList.next();
			continue;
		}

		// something happened to the parent
		if(!psystem->parententity->pmodel)
		{
			// Remove all related particles
			cl_particle_t *pparticle = psystem->pparticleheader;
			while(pparticle)
			{
				cl_particle_t *pfree = pparticle;
				pparticle = pfree->next;

				m_iNumFreedParticles++;
				RemoveParticle(pfree);
			}

			// Unparent these and let the engine handle them
			if(psystem->createsystem)
				psystem->createsystem->parentsystem = nullptr;

			if(psystem->watersystem)
				psystem->watersystem->parentsystem = nullptr;

			// Delete from memory
			m_iNumFreedSystems++;
			m_particleSystemsList.remove(m_particleSystemsList.get_link());
			delete psystem;

			m_particleSystemsList.next();
			continue;
		}

		// Refresh attachment
		if(psystem->parententity->pmodel && psystem->parententity->pmodel->type == MOD_VBM)
			gVBMRenderer.UpdateAttachments(psystem->parententity);

		m_particleSystemsList.next();
	}

	//Update systems
	m_particleSystemsList.begin();
	while(!m_particleSystemsList.end())
	{
		particle_system_t *psystem = m_particleSystemsList.get();
#if 0
		if(!psystem->leafnums.empty())
		{
			if(Common::CheckVisibility(psystem->leafnums, rns.pvisbuffer))
				psystem->visframe = rns.visframe;
		}
#endif
		// Parented systems cannot spawn particles themselves
		if(psystem->parentsystem)
		{
			m_particleSystemsList.next();
			continue;
		}

		// Pointer to script definition
		assert(psystem->pdefinition != nullptr);
		const script_definition_t* pdefinition = psystem->pdefinition;

		if(psystem->spawned == false)
		{
			if(pdefinition->shapetype != shape_playerplane)
			{
				// create all starting particles
				for(Uint32 i = 0; i < pdefinition->startparticles; i++)
					CreateParticle(psystem);
			}
			else
			{
				// Create particles at random heights
				EnvironmentCreateFirst(psystem);
			}

			// Mark system as having spawned
			psystem->spawned = true;
		}
		
		if(!psystem->particlefreq)
		{
			m_particleSystemsList.next();
			continue;
		}

		// Get base frequency for potential modification
		Float particlefreq = psystem->particlefreq;

		// Manage soft turnoff
		if(psystem->softoffbegintime)
		{
			if((psystem->softoffbegintime + pdefinition->softofftime) <= rns.time)
			{
				psystem->maxparticles = 0;
				m_particleSystemsList.next();
				continue;
			}

			Float fraction = 1.0 - ((rns.time - psystem->softoffbegintime) / pdefinition->softofftime);
			fraction = clamp(fraction, 0.0, 1.0);

			particlefreq *= fraction;
		}

		// Determine how many we've spawned
		Float life = rns.time - psystem->spawntime;
		Float freq = 1/(Float)particlefreq;
		Uint64 itimesspawn = life/freq;

		if(itimesspawn <= psystem->numspawns)
		{
			m_particleSystemsList.next();
			continue;
		}

		Int32 inumspawn = itimesspawn - psystem->numspawns;

		// cap if finite
		if(psystem->maxparticles != -1)
		{
			if(psystem->maxparticles < inumspawn)
				inumspawn = psystem->maxparticles;
		}

		if(pdefinition->maxparticlevar)
		{
			// Calculate variation
			Uint32 variedamount = inumspawn+abs((sin(rns.time*0.2)/M_PI)*pdefinition->maxparticlevar);
			if(pdefinition->shapetype == shape_playerplane)
				variedamount *= m_pWeatherDensity->GetValue();

			// Create new particles
			for(Uint32 j = 0; j < variedamount; j++)
				CreateParticle(psystem);

			// Add to counter
			psystem->numspawns += inumspawn;

			// don't take off for infinite systems
			if(psystem->maxparticles != -1)
				psystem->maxparticles -= inumspawn;
		}
		else
		{
			Uint32 finalamount = inumspawn;
			if(pdefinition->shapetype == shape_playerplane)
				finalamount *= m_pWeatherDensity->GetValue();

			// Create new particles
			for(Uint32 j = 0; j < finalamount; j++)
				CreateParticle(psystem);

			// Add to counter
			psystem->numspawns += inumspawn;

			// don't take off for infinite systems
			if(psystem->maxparticles != -1)
				psystem->maxparticles -= inumspawn;
		}

		m_particleSystemsList.next();
	}

	// check if any systems are available for removal
	m_particleSystemsList.begin();
	while(!m_particleSystemsList.end())
	{
		particle_system_t *psystem = m_particleSystemsList.get();

		if(psystem->maxparticles != 0)
		{
			m_particleSystemsList.next();
			continue;
		}

		if(psystem->parentsystem)
		{
			m_particleSystemsList.next();
			continue;
		}
		
		// Has related particles
		if(psystem->pparticleheader || !psystem->spawned)
		{
			m_particleSystemsList.next();
			continue;
		}

		// Unparent these and let the engine handle them
		if(psystem->createsystem)
			psystem->createsystem->parentsystem = nullptr;

		if(psystem->watersystem)
			psystem->watersystem->parentsystem = nullptr;

		// Delete from memory
		m_iNumFreedSystems++;
		m_particleSystemsList.remove(m_particleSystemsList.get_link());
		delete psystem;

		m_particleSystemsList.next();
	}
}

//====================================
//
//====================================
void CParticleEngine::TransformRelativeVector( const Vector& basevector, particle_system_t *psystem, Vector& outvector, bool iscoordinate, bool inverse )
{
	if(psystem->attachflags & PARTICLE_ATTACH_TO_ATTACHMENT)
	{
		// Only do anything here if it's a coordinate
		if(iscoordinate)
		{
			if(inverse)
				Math::VectorSubtract(basevector, psystem->parententity->getAttachment(psystem->attachment), outvector);
			else
				Math::VectorAdd(basevector, psystem->parententity->getAttachment(psystem->attachment), outvector);
		}
		else
		{
			outvector = basevector;
		}
	}
	else if(psystem->attachflags & PARTICLE_ATTACH_ATTACHMENT_VECTOR)
	{
		Vector attach1 = psystem->parententity->getAttachment(psystem->attachment);
		Vector attach2 = psystem->parententity->getAttachment(psystem->attachment+1);
		Vector forward = (attach2-attach1).Normalize();
		Vector angles = Math::VectorToAngles(forward);
		
		if(iscoordinate)
		{
			if(inverse)
			{
				Math::VectorSubtract(basevector, attach1, outvector);
				Math::RotateToEntitySpace(angles, outvector);
			}
			else
			{
				Math::RotateFromEntitySpace(angles, outvector);
				Math::VectorAdd(basevector, attach1, outvector);
			}
		}
		else
		{
			Math::VectorCopy(basevector, outvector);

			if(inverse)
				Math::RotateToEntitySpace(angles, outvector);
			else
				Math::RotateFromEntitySpace(angles, outvector);
		}
	}
	else if(psystem->attachflags & PARTICLE_ATTACH_TO_BONE)
	{
		if(iscoordinate)
			GetBoneTransformedPosition(basevector, psystem->parententity, psystem->boneindex, outvector, inverse);
		else
			GetBoneRotatedVector(basevector, psystem->parententity, psystem->boneindex, outvector, inverse);
	}
	else
	{
		// Only do anything here if it's a coordinate
		if(iscoordinate)
		{
			if(inverse)
				Math::VectorSubtract(basevector, psystem->parententity->curstate.origin, outvector);
			else
				Math::VectorAdd(basevector, psystem->parententity->curstate.origin, outvector);
		}
		else
			outvector = basevector;
	}
}

//====================================
//
//====================================
Vector CParticleEngine::LightForParticle( cl_particle_t *pparticle ) 
{
	Vector vorigin;
	Vector color;

	// Pointer to script definition
	const script_definition_t* pdefinition = pparticle->psystem->pdefinition;

	if(pdefinition->lighting_flags & PARTICLE_LIGHTCHECK_ONLYONCE)
		return pparticle->lightcol;

	Math::VectorCopy(pparticle->origin, vorigin);

	if((pparticle->psystem->attachflags & PARTICLE_ATTACH_TO_PARENT)  
		&& (pparticle->psystem->attachflags & PARTICLE_ATTACH_RELATIVE)
		&& pparticle->psystem->parententity)
	{
		// Transform from local space to world space
		TransformRelativeVector(vorigin, pparticle->psystem, vorigin, true, false);
	}

	Float fldist = (pparticle->origin - pparticle->last_light).Length2D();
	if(fldist > LIGHTCHECK_DISTANCE)
	{
		if(!Mod_RecursiveLightPoint(ens.pworld, ens.pworld->pnodes, vorigin, vorigin - Vector(0, 0, 2048), color))
			color = ZERO_VECTOR;

		Math::VectorCopy(color, pparticle->lightmap);
		Math::VectorCopy(pparticle->origin, pparticle->last_light);
	}
	else
		Math::VectorCopy(pparticle->lightmap, color);

	if(pdefinition->maxlight || pdefinition->minlight)
	{
		Float illum = (color[0] + color[1] + color[2])/3;

		if(pdefinition->minlight)
		{
			if(illum < pdefinition->minlight)
				Math::VectorScale(color, pdefinition->minlight/illum, color);
		}

		if(pdefinition->maxlight)
		{
			if(illum > pdefinition->maxlight)
				Math::VectorScale(color, pdefinition->maxlight/illum, color);
		}
	}

	Math::VectorCopy(color, pparticle->lightcol);
	return color;
}

//====================================
//
//====================================
__forceinline Int32 CParticleEngine::CheckWater( const Vector& origin ) 
{
	for(Uint32 i = 0; i < rns.objects.numvisents; i++)
	{
		if(rns.objects.pvisents[i]->curstate.rendertype != RT_WATERSHADER)
			continue;

		Int32 contents = CL_PointContents( rns.objects.pvisents[i], origin );
		if(contents != CONTENTS_EMPTY)
			return i;
	}

	return NO_ENTITY_INDEX;
}

//====================================
//
//====================================
bool CParticleEngine::CheckCollision( Vector& vecOrigin, Vector& vecVelocity, particle_system_t* psystem, cl_particle_t *pparticle ) 
{
	// Pointer to script definition
	const script_definition_t* pdefinition = psystem->pdefinition;

	Vector testPosition;
	Math::VectorMA(vecOrigin, rns.frametime, vecVelocity, testPosition);

	// If TRUE, check collisions precisely
	bool checkPreciseCollision = false;

	// Do a non-precise collision check if script is not set to be precise
	if(!(pdefinition->collision_flags & COLLISION_FL_PRECISE))
	{
		Int32 contents = CL_PointContents(nullptr, testPosition);
		if(contents != CONTENTS_EMPTY)
			checkPreciseCollision = true;
	}
	else
	{
		// Always check for precise if it's set
		checkPreciseCollision = true;
	}

	// Check for water if we haven't collided with anything
	Int32 waterEntity = NO_ENTITY_INDEX;
	if((pdefinition->collision_flags & COLLISION_FL_WATER)
		&& ((pdefinition->collision_flags & COLLISION_FL_PRECISE) || !checkPreciseCollision))
		waterEntity = CheckWater(testPosition);

	if(!checkPreciseCollision && waterEntity == NO_ENTITY_INDEX)
	{
		Math::VectorCopy(vecVelocity, pparticle->velocity);
		return true;
	}

	trace_t tr;

	// If we collided with water, then fill out traceline
	if(waterEntity != NO_ENTITY_INDEX)
	{
		cl_entity_t *pEntity = rns.objects.pvisents[waterEntity];

		// Determine impact z coord
		Float impactZCoordinate = pEntity->curstate.origin.z + pEntity->pmodel->maxs.z + 0.1;
		// Calculate fraction based on this
		Float distanceFraction = (SDL_fabs(impactZCoordinate - vecOrigin.z)) / (SDL_fabs(testPosition.z - vecOrigin.z));
		
		Math::VectorScale(vecOrigin, (1.0 - distanceFraction), tr.endpos);
		Math::VectorMA(tr.endpos, distanceFraction, testPosition, tr.endpos);

		tr.plane.normal = Vector(0, 0, 1);
		tr.fraction = distanceFraction;
		tr.hitentity = pEntity->entindex;
	}
	else
	{
		// Just do a normal traceline
		Int32 traceFlags = (pdefinition->collision_flags & COLLISION_FL_BMODELS) ? FL_TRACE_NORMAL : FL_TRACE_WORLD_ONLY;
		CL_PlayerTrace(vecOrigin, testPosition, traceFlags, HULL_POINT, NO_ENTITY_INDEX, tr);
	}

	if(tr.flags & FL_TR_ALLSOLID)
		return false; // Probably spawned inside a solid

	// Didn't hit anything
	if(tr.fraction == 1.0)
	{
		Math::VectorCopy(vecVelocity, pparticle->velocity);
		return true;
	}

	if(pdefinition->collision == collide_stuck)
	{
		if(CL_PointContents(tr.endpos, nullptr) == CONTENTS_SKY)
			return false;

		if(pparticle->life == -1 && pdefinition->stuckdie)
		{
			pparticle->life = rns.time + pdefinition->stuckdie;
			pparticle->fadeoutdelay = rns.time - pparticle->spawntime;
		}
		Math::VectorMA( vecOrigin, tr.fraction*rns.frametime, vecVelocity, vecOrigin );

		pparticle->rotationvel = 0;
		pparticle->rotxvel = 0;
		pparticle->rotyvel = 0;

		Math::VectorClear(pparticle->velocity);
		Math::VectorClear(vecVelocity);
		return true;
	}
	else if(pdefinition->collision == collide_bounce)
	{
		Float fProj = Math::DotProduct(vecVelocity, tr.plane.normal);
		Math::VectorMA(vecVelocity, -fProj*2, tr.plane.normal, pparticle->velocity);
		Math::VectorScale(pparticle->velocity, pdefinition->impactdamp, pparticle->velocity);
		Math::VectorScale(vecVelocity, tr.fraction, vecVelocity);

		if(pparticle->rotationvel)
			pparticle->rotationvel *= pdefinition->impactdamp;

		if(pparticle->rotxvel)
			pparticle->rotxvel *= pdefinition->impactdamp;

		if(pparticle->rotyvel)
			pparticle->rotyvel *= pdefinition->impactdamp;

		if(pdefinition->alignment == align_tracer)
		{
			if(vecVelocity.Length() < 80)
				return false;
		}

		return true;
	}
	else if(pdefinition->collision == collide_decal)
	{
		CDecalList& decalList = gDecals.GetDecalList();

		decalgroupentry_t *pentry = decalList.GetRandom(pdefinition->create.c_str());
		if(pentry)
			gBSPRenderer.CreateDecal(tr.endpos, tr.plane.normal, pentry, FL_DECAL_NONE, psystem->pdefinition->decallife, psystem->pdefinition->decalfade, psystem->pdefinition->decalgrowthtime);

		return false;
	}
	else if(pdefinition->collision == collide_new_system)
	{
		if(pdefinition->spawnchance)
		{
			if(Common::RandomLong(1, pdefinition->spawnchance) == 1)
			{
				if(waterEntity != -1 && !pdefinition->watercreate.empty() && psystem->watersystem != nullptr)
				{
					for(Uint32 i = 0; i < psystem->watersystem->pdefinition->startparticles; i++)
						CreateParticle(psystem->watersystem, tr.endpos, tr.plane.normal);
				}

				if(CL_PointContents(tr.endpos, nullptr) != CONTENTS_SKY && !pdefinition->create.empty() && psystem->createsystem != nullptr)
				{
					for(Uint32 i = 0; i < psystem->createsystem->pdefinition->startparticles; i++)
						CreateParticle(psystem->createsystem, tr.endpos, tr.plane.normal);
				}
			}
		}
		else
		{
			if(waterEntity != -1 && !pdefinition->watercreate.empty() && psystem->watersystem != nullptr)
			{
				for(Uint32 i = 0; i < psystem->watersystem->pdefinition->startparticles; i++)
					CreateParticle(psystem->watersystem, tr.endpos, tr.plane.normal);
			}

			if(CL_PointContents(tr.endpos, nullptr) != CONTENTS_SKY && !pdefinition->create.empty() && psystem->createsystem != nullptr)
			{
				for(Uint32 i = 0; i < psystem->createsystem->pdefinition->startparticles; i++)
					CreateParticle(psystem->createsystem, tr.endpos, tr.plane.normal);
			}
		}
		return false;
	}

	// Kill it
	return false;
}

//====================================
//
//====================================
void CParticleEngine::GetBoneTransformedPosition( const Vector& baseposition, cl_entity_t* pentity, Int32 boneindex, Vector& outposition, bool reverse ) 
{
	if(!pentity->pmodel || pentity->pmodel->type != MOD_VBM)
	{
		Math::VectorAdd(baseposition, pentity->curstate.origin, outposition);
		return;
	}

	outposition = baseposition;
	gVBMRenderer.TransformVectorByBoneMatrix(pentity, boneindex, outposition, reverse); 
}

//====================================
//
//====================================
void CParticleEngine::GetBoneRotatedVector( const Vector& basevector, cl_entity_t* pentity, Int32 boneindex, Vector& outvector, bool reverse )
{
	outvector = basevector;

	if(!pentity->pmodel || pentity->pmodel->type != MOD_VBM)
		return;

	gVBMRenderer.RotateVectorByBoneMatrix(pentity, boneindex, outvector, reverse); 
}

//====================================
//
//====================================
bool CParticleEngine::UpdateParticle( cl_particle_t *pparticle ) 
{
	Vector vvelocity;
	Vector vbaseorigin;

	particle_system_t *psystem = pparticle->psystem;

	// Pointer to script definition
	assert(psystem->pdefinition != nullptr);
	const script_definition_t* pdefinition = psystem->pdefinition;

	Math::VectorCopy(pparticle->velocity, vvelocity);
	Math::VectorCopy(pparticle->origin, vbaseorigin);

	if((pparticle->psystem->attachflags & PARTICLE_ATTACH_TO_PARENT)  
		&& (pparticle->psystem->attachflags & PARTICLE_ATTACH_RELATIVE)
		&& pparticle->psystem->parententity)
	{
		// Transform origin and velocity
		TransformRelativeVector(vbaseorigin, psystem, vbaseorigin, true, false);
		TransformRelativeVector(vvelocity, psystem, vvelocity, false, false);
	}

	if(pdefinition->shapetype != shape_playerplane)
	{
		if(!(pdefinition->render_flags & RENDER_FL_SKYBOX))
		{
#if 0
			if(!psystem->leafnums.empty())
			{
				if(psystem->visframe != rns.visframe)
					return false;
			}

			if(!psystem->nocull && psystem->id == 0 && !psystem->parententity)
			{
				Vector vmins, vmaxs;
				for(Uint32 i = 0; i < 3; i++)
				{
					vmins[i] = vbaseorigin[i]-pparticle->scale;
					vmaxs[i] = vbaseorigin[i]+pparticle->scale;
				}

				if(rns.view.frustum.CullBBox(vmins, vmaxs))
					return false;
			}
#endif
		}
	}

	//
	// Check if the particle is ready to die
	//
	if(pparticle->life != -1)
	{
		if(pparticle->life <= rns.time)
		{
			if(!pdefinition->deathcreate.empty() && !psystem->parentsystem)
			{
				Vector direction = pparticle->velocity;
				direction.Normalize();

				CreateSystem(pdefinition->deathcreate.c_str(), vbaseorigin, direction, 0);
			}

			return false; // remove
		}
	}

	//
	// Damp velocity
	//
	if(pdefinition->velocitydamp && (pparticle->spawntime + pdefinition->veldampdelay) < rns.time)
		Math::VectorScale(vvelocity, (1.0 - pdefinition->velocitydamp*rns.frametime), vvelocity);

	//
	// Add gravity before collision test
	//
	vvelocity.z -= m_pCvarGravity->GetValue()*pdefinition->gravity*rns.frametime;

	//
	// Add in wind on either axes
	//
	if(pdefinition->windtype)
	{
		if(pparticle->windxvel)
		{
			if(pdefinition->windtype == wind_linear) vvelocity.x += pparticle->windxvel*rns.frametime;
			else vvelocity.x += (pdefinition->minwindmult+abs(sin((rns.time*pparticle->windmult))))*pparticle->windxvel*rns.frametime;
		}
		if(pparticle->windyvel)
		{
			if(pdefinition->windtype == wind_linear) vvelocity.y += pparticle->windyvel*rns.frametime;
			else vvelocity.y += (pdefinition->minwindmult+abs(sin((rns.time*pparticle->windmult))))*pparticle->windxvel*rns.frametime;
		}
	}

	//
	// Calculate rotation on all axes
	//
	if(pdefinition->rotationvel)
	{
		if(pdefinition->rotationdamp && pparticle->rotationvel)
		{
			if((pdefinition->rotationdampdelay + pparticle->spawntime) < rns.time)
				pparticle->rotationvel = pparticle->rotationvel*(1.0 - pdefinition->rotationdamp);
		}

		pparticle->rotation += pparticle->rotationvel*rns.frametime;
	
		if(pparticle->rotation < 0)
			pparticle->rotation += 360;
		if(pparticle->rotation > 360)
			pparticle->rotation -= 360;
	}
	if(pdefinition->rotxvel)
	{
		if(pdefinition->rotxdamp && pparticle->rotxvel)
		{
			if((pdefinition->rotxdampdelay + pparticle->spawntime) < rns.time)
				pparticle->rotxvel = pparticle->rotxvel*(1.0 - pdefinition->rotxdamp);
		}

		pparticle->rotx += pparticle->rotxvel*rns.frametime;
	
		if(pparticle->rotx < 0)
			pparticle->rotx += 360;
		if(pparticle->rotx > 360)
			pparticle->rotx -= 360;
	}
	if(pdefinition->rotyvel)
	{
		if(pdefinition->rotydamp && pparticle->rotyvel)
		{
			if((pdefinition->rotydampdelay + pparticle->spawntime) < rns.time)
				pparticle->rotyvel = pparticle->rotyvel*(1.0 - pdefinition->rotydamp);
		}

		pparticle->roty += pparticle->rotyvel*rns.frametime;
	
		if(pparticle->roty < 0)
			pparticle->roty += 360;
		if(pparticle->roty > 360)
			pparticle->roty -= 360;
	}

	// Factor in black holes
	if(pdefinition->gravity)
	{
		if(!gBlackHoleRenderer.AffectObject(pparticle->origin, vvelocity, pdefinition->gravity))
			return false;
	}

	//
	// Collision detection
	//
	if(pdefinition->collision)
	{
		if(!CheckCollision(vbaseorigin, vvelocity, psystem, pparticle))
			return false;
	}
	else
	{
		// Just set the velocity
		Math::VectorCopy(vvelocity, pparticle->velocity);
	}

	//
	// Add in the final velocity
	//
	Math::VectorMA(vbaseorigin, rns.frametime, vvelocity, vbaseorigin);

	//
	// Subtract back if necessary
	if((psystem->attachflags & PARTICLE_ATTACH_TO_PARENT) 
		&& (psystem->attachflags & PARTICLE_ATTACH_RELATIVE)
		&& psystem->parententity)
	{
		// Transform origin and velocity
		TransformRelativeVector(vbaseorigin, psystem, pparticle->origin, true, true);
		TransformRelativeVector(pparticle->velocity, psystem, pparticle->velocity, false, true);
	}
	else
	{
		Math::VectorCopy(vbaseorigin, pparticle->origin);
	}

	//
	// Always reset to 1.0
	//
	pparticle->alpha = 1.0;

	//
	// Fading in
	//
	if(pparticle->fadein)
	{
		if((pparticle->spawntime + pparticle->fadein) > rns.time)
		{
			Double fadetime = pparticle->spawntime + pparticle->fadein;
			Double timetofade = fadetime - rns.time;

			pparticle->alpha = 1.0 - (timetofade/pparticle->fadein);
		}
	}

	//
	// Fade out
	//
	if(pparticle->fadeoutdelay)
	{
		if((pparticle->fadeoutdelay + pparticle->spawntime) < rns.time)
		{
			Float deathtime = pparticle->life - rns.time;
			Float fadetime = pparticle->fadeoutdelay + pparticle->spawntime;
			Float fadedelta = pparticle->life - fadetime;

			pparticle->alpha = deathtime/fadedelta;
		}
	}

	//
	// Minimum and maximum distance fading
	//
	if(pdefinition->fadedistfar && pdefinition->fadedistnear)
	{
		Float dist = (vbaseorigin - Vector(rns.view.params.v_origin)).Length();
		Float alpha = 1.0-((pdefinition->fadedistfar - dist)/(pdefinition->fadedistfar-pdefinition->fadedistnear));
	
		if( alpha < 0 ) alpha = 0;
		if( alpha > 1 ) alpha = 1;

		pparticle->alpha *= alpha;
	}

	//
	// Dampen scale
	//
	if(pdefinition->scaledampfactor && (pparticle->scaledampdelay < rns.time))
		pparticle->scale = pparticle->scale - rns.frametime*pdefinition->scaledampfactor;

	if(pparticle->scale <= 0)
		return false;

	//
	// See if we need to blend colors
	// 
	if(!(pdefinition->lighting_flags & PARTICLE_LIGHTCHECK_NORMAL))
	{
		if((pparticle->secondarydelay < rns.time) && (rns.time < (pparticle->secondarydelay + pparticle->secondarytime)))
		{
			Float fulltime = (pparticle->secondarydelay+pparticle->secondarytime) - rns.time;
			Float coldelta = fulltime/pparticle->secondarytime;

			pparticle->color[0] = pparticle->scolor[0]*(1.0 - coldelta) + pdefinition->primarycolor[0]*coldelta;
			pparticle->color[1] = pparticle->scolor[1]*(1.0 - coldelta) + pdefinition->primarycolor[1]*coldelta;
			pparticle->color[2] = pparticle->scolor[2]*(1.0 - coldelta) + pdefinition->primarycolor[2]*coldelta;
		}
	}


	//
	// Spawn tracer particles
	//
	if(pdefinition->tracerdist)
	{
		Vector vdistance, vorigin;
		Math::VectorSubtract(vbaseorigin, pparticle->lastspawn, vdistance);

		if(vdistance.Length() > pdefinition->tracerdist)
		{
			Vector vdirection = vbaseorigin - pparticle->lastspawn;
			Uint32 inumtraces = vdistance.Length()/pdefinition->tracerdist;

			for(Uint32 i = 0; i < inumtraces; i++)
			{
				Float fraction = (i+1)/(Float)inumtraces;
				Math::VectorMA(pparticle->lastspawn, fraction, vdirection, vorigin);

				Vector direction = pparticle->velocity;
				direction.Normalize();

				CreateParticle(psystem->createsystem, vorigin, direction);
			}

			Math::VectorCopy(vbaseorigin, pparticle->lastspawn);
		}
	}

	//
	// Calculate texcoords
	//
	if(pdefinition->numframes && pdefinition->framerate)
	{
		// Get desired frame
		Int32 iframe = ((Int32)((rns.time - pparticle->spawntime)*pdefinition->framerate));
		iframe = iframe % pdefinition->numframes;

		// Check if we actually have to set the frame
		if(iframe != pparticle->frame)
		{
			en_texture_t *pTexture = psystem->ptexture;

			Int32	numframesx = pTexture->width/pdefinition->framesizex;
			Int32	numframesy = pTexture->height/pdefinition->framesizey;

			Int32 column = iframe%numframesx;
			Int32 row = (iframe/numframesx)%numframesy;

			// Calculate these only once
			Float fracwidth = (Float)pdefinition->framesizex/(Float)pTexture->width;
			Float fracheight = (Float)pdefinition->framesizey/(Float)pTexture->height;

			// Calculate top left coordinate
			pparticle->texcoords[0][0] = (column+1)*fracwidth;
			pparticle->texcoords[0][1] = row*fracheight;

			// Calculate top right coordinate
			pparticle->texcoords[1][0] = column*fracwidth;
			pparticle->texcoords[1][1] = row*fracheight;

			// Calculate bottom right coordinate
			pparticle->texcoords[2][0] = column*fracwidth;
			pparticle->texcoords[2][1] = (row+1)*fracheight;

			// Calculate bottom left coordinate
			pparticle->texcoords[3][0] = (column+1)*fracwidth;
			pparticle->texcoords[3][1] = (row+1)*fracheight;

			// Fill in current frame
			pparticle->frame = iframe;
		}
	}

	// All went well, particle is still active
	return true;
}

//====================================
//
//====================================
void CParticleEngine::GetLights( particle_system_t *psystem, cl_dlight_t **plights, Uint32 *numlights, bool spotlight, Uint32 max )
{
	// Clear first
	for(Uint32 i = 0; i < max; i++)
		plights[i] = nullptr;

	(*numlights) = 0;

	if(g_pCvarDynamicLights->GetValue() < 1)
		return;

	// Pointer to script definition
	assert(psystem->pdefinition != nullptr);
	const script_definition_t* pdefinition = psystem->pdefinition;

	Vector vsysmins, vsysmaxs, vsysorigin;
	if(pdefinition->shapetype == shape_playerplane)
	{
		cl_entity_t *pplayer = CL_GetLocalPlayer();
		for(Uint32 i = 0; i < 3; i++)
		{
			vsysmins[i] = pplayer->curstate.origin[i]-psystem->radius;
			vsysmaxs[i] = pplayer->curstate.origin[i]+psystem->radius;
		}
		Math::VectorCopy(pplayer->curstate.origin, vsysorigin);
	}
	else
	{
		if(!psystem->parententity)
		{
			for(Uint32 i = 0; i < 3; i++)
			{
				vsysmins[i] = psystem->origin[i]-psystem->radius;
				vsysmaxs[i] = psystem->origin[i]+psystem->radius;
			}
			Math::VectorCopy(psystem->origin, vsysorigin);
		}
		else
		{
			Vector vattachorigin;
			Math::VectorAdd(psystem->origin, psystem->parententity->getAttachment(psystem->attachment), vattachorigin);

			for(Uint32 i = 0; i < 3; i++)
			{
				vsysmins[i] = vattachorigin[i]-psystem->radius;
				vsysmaxs[i] = vattachorigin[i]+psystem->radius;
			}
			Math::VectorCopy(vattachorigin, vsysorigin);
		}

	}

	for(Uint32 i = 0; i < max; i++)
	{
		cl_dlight_t *lastnearest = nullptr;
		Float lastnearestdist = 0;

		CLinkedList<cl_dlight_t*>& dlList = gDynamicLights.GetLightList();

		dlList.begin();
		while(!dlList.end())
		{
			cl_dlight_t* pdl = dlList.get();

			if(pdl->cone_size && !spotlight)
			{
				dlList.next();
				continue;
			}

			if(!pdl->cone_size && spotlight)
			{
				dlList.next();
				continue;
			}

			Vector vmins, vmaxs;
			for(Uint32 j = 0; j < 3; j++)
			{
				vmins[j] = pdl->origin[j]-pdl->radius;
				vmaxs[j] = pdl->origin[j]+pdl->radius;
			}

			if(!DL_IsLightVisible(rns.view.frustum, vmins, vmaxs, pdl))
			{
				dlList.next();
				continue;
			}

			if(Math::CheckMinsMaxs(vsysmins, vsysmaxs, vmins, vmaxs))
			{
				dlList.next();
				continue;
			}

			Float fldistance = (pdl->origin - vsysorigin).Length();
			if(!lastnearest || lastnearestdist > fldistance)
			{
				if((*numlights) > 0)
				{
					Uint32 j = 0;
					for(; j < (*numlights); j++)
					{
						if(plights[j] == pdl)
							break;
					}

					if(j != (*numlights))
					{
						dlList.next();
						continue;
					}
				}

				lastnearest = pdl;
				lastnearestdist = fldistance;
			}

			dlList.next();
		}

		if(!lastnearest)
		{
			plights[*numlights] = nullptr;
			return;
		}

		plights[*numlights] = lastnearest;
		(*numlights)++;
	}
}

//====================================
//
//====================================
__forceinline void CParticleEngine::BatchVertex( cl_particle_t *pparticle, const Vector& vertex, Float alpha,  Int32 tc )
{
	particle_vertex_t *pvert = &m_pVertexes[m_numVertexes];
	m_numVertexes++;

	Math::VectorCopy(vertex, pvert->origin);
	pvert->origin[3] = 1.0;

	Math::VectorCopy(pparticle->color, pvert->color);
	pvert->color[3] = alpha;

	pvert->texcoord[0] = pparticle->texcoords[tc][0];
	pvert->texcoord[1] = pparticle->texcoords[tc][1];
}

//====================================
//
//====================================
__forceinline bool CParticleEngine::ClipTracer( const Vector &start, const Vector &delta, Vector &clippedStart, Vector &clippedDelta )
{
	Float dist1 = -start[2];
	Float dist2 = dist1 - delta[2];
	
	// Clipped, skip this tracer
	if ( dist1 <= 0 && dist2 <= 0 )
		return true;

	clippedStart = start;
	clippedDelta = delta;
	
	// Needs to be clipped
	if ( dist1 <= 0 || dist2 <= 0 )
	{
		Float fraction = dist2 - dist1;

		// Too close to clipping plane
		if ( fraction < 1e-3 && fraction > -1e-3 )
			return true;

		fraction = -dist1 / fraction;

		if ( dist1 <= 0 )
			Math::VectorMA( start, fraction, delta, clippedStart );
		else
			Math::VectorScale( delta, fraction, clippedDelta );
	}

	return false;
}

//====================================
//
//====================================
void CParticleEngine::BatchParticle( cl_particle_t *pparticle, Float flup, Float flright, const Float *pfltranspose ) 
{
	// make these static, might save on performance
	Vector start, delta, clippedStart, clippedDelta;
	Vector tracerverts[4];

	Vector vorigin, vangles, normal, vpoint;
	Vector vmins, vmaxs;

	Float sqLength;		

	if(m_numParticles == MAX_ACTIVE_PARTICLES)
	{
		Con_Printf("%s - Overflow on maximum rendered particles.\n", __FUNCTION__);
		return;
	}

	Math::VectorCopy(pparticle->origin, vorigin);

	if((pparticle->psystem->attachflags & PARTICLE_ATTACH_TO_PARENT) 
		&& (pparticle->psystem->attachflags & PARTICLE_ATTACH_RELATIVE)
		&& pparticle->psystem->parententity )
	{
		// Transform origin to world space
		TransformRelativeVector(vorigin, pparticle->psystem, vorigin, true, false);
	}

	// Pointer to script definition
	assert(pparticle->psystem->pdefinition != nullptr);
	const script_definition_t* pdefinition = pparticle->psystem->pdefinition;
	if(!(pdefinition->render_flags & RENDER_FL_SKYBOX))
	{
		for(Uint32 i = 0; i < 3; i++)
		{
			vmins[i] = vorigin[i]-pparticle->scale;
			vmaxs[i] = vorigin[i]+pparticle->scale;
		}

		if(rns.view.frustum.CullBBox(vmins, vmaxs))
			return;
	}

	//
	// Check if lighting is required
	//
	if(pdefinition->lighting_flags)
	{
		if(pdefinition->lighting_flags & PARTICLE_LIGHTCHECK_NORMAL)
		{
			pparticle->color = LightForParticle(pparticle);
		}
		else if(pdefinition->lighting_flags & PARTICLE_LIGHTCHECK_SCOLOR)
		{
			LightForParticle(pparticle);
			Math::VectorCopy(pparticle->lightcol, pparticle->scolor);
		}

		if(pdefinition->lighting_flags & PARTICLE_LIGHTCHECK_MIXP)
		{
			pparticle->color = LightForParticle(pparticle);
			pparticle->color.x = pparticle->color.x*pdefinition->primarycolor.x;
			pparticle->color.y = pparticle->color.y*pdefinition->primarycolor.y;
			pparticle->color.z = pparticle->color.z*pdefinition->primarycolor.z;
		}

		if(pdefinition->lighting_flags & PARTICLE_LIGHTCHECK_INTENSITY)
		{
			Vector color = LightForParticle(pparticle);
			Float avg = (color[0]+color[1]+color[2])/3;
			pparticle->color.x = pdefinition->primarycolor.x*avg;
			pparticle->color.y = pdefinition->primarycolor.y*avg;
			pparticle->color.z = pdefinition->primarycolor.z*avg;
		}
	}

	if(pdefinition->alignment == align_normal)
	{
		Math::GetUpRight(pparticle->normal, m_vRUp, m_vRRight);
	}
	else if(pparticle->rotation || pparticle->rotx || pparticle->roty)
	{
		Math::VectorCopy(rns.view.v_angles, vangles);

		if(pparticle->rotx) vangles[0] = pparticle->rotx;
		if(pparticle->roty) vangles[1] = pparticle->roty;
		if(pparticle->rotation) vangles[2] = pparticle->rotation;

		Math::AngleVectors(vangles, nullptr, &m_vRRight, &m_vRUp);
	}

	Float alpha = pparticle->alpha*pdefinition->mainalpha;
	if(pdefinition->alignment == align_parallel)
	{
		vpoint = vorigin + m_vRUp * flup * pparticle->scale * 2;
		vpoint = vpoint + m_vRRight * flright * (-pparticle->scale);
		BatchVertex(pparticle, vpoint, alpha, 0);

		vpoint = vorigin + m_vRUp * flup * pparticle->scale * 2;
		vpoint = vpoint + m_vRRight * flright * pparticle->scale;
		BatchVertex(pparticle, vpoint, alpha, 1);

		vpoint = vorigin + m_vRRight * flright * pparticle->scale;
		BatchVertex(pparticle, vpoint, alpha, 2);

		vpoint = vorigin + m_vRRight * flright * (-pparticle->scale);
		BatchVertex(pparticle, vpoint, alpha, 3);
	}
	else if(pdefinition->alignment == align_tracer)
	{
		Math::MatMultPosition(pfltranspose, vorigin, &start);
		Math::MatMult(pfltranspose, pparticle->velocity, &delta);
		delta = delta.Normalize()*flup*pparticle->scale*2;

		// Clip the tracer
		if ( ClipTracer( start, delta, clippedStart, clippedDelta ) )
			return;

		// Figure out direction in camera space of the normal
		Math::CrossProduct( clippedDelta, clippedStart, normal );
						  
		// don't draw if they are parallel
		sqLength = Math::DotProduct( normal, normal );
		if (sqLength < 1e-3)
			return;

		// Resize the normal to be appropriate based on the width
		Math::VectorScale( normal, 0.5f * flright*pparticle->scale / sqrt(sqLength), normal );

		Math::VectorSubtract( clippedStart, normal, tracerverts[0] );
		Math::VectorAdd( clippedStart, normal, tracerverts[1] );

		Math::VectorAdd( tracerverts[0], clippedDelta, tracerverts[2] );
		Math::VectorAdd( tracerverts[1], clippedDelta, tracerverts[3] );

		BatchVertex(pparticle, tracerverts[0],  alpha, 0);
		BatchVertex(pparticle, tracerverts[1],  alpha, 1);
		BatchVertex(pparticle, tracerverts[3],  alpha, 2);
		BatchVertex(pparticle, tracerverts[2],  alpha, 3);
	}	
	else
	{
		vpoint = vorigin + m_vRUp * flup * pparticle->scale;
		vpoint = vpoint + m_vRRight * flright * (-pparticle->scale);
		BatchVertex(pparticle, vpoint, alpha, 0);

		vpoint = vorigin + m_vRUp * flup * pparticle->scale;
		vpoint = vpoint + m_vRRight * flright * pparticle->scale;
		BatchVertex(pparticle, vpoint, alpha, 1);

		vpoint = vorigin + m_vRUp * flup * (-pparticle->scale);
		vpoint = vpoint + m_vRRight * flright * pparticle->scale;
		BatchVertex(pparticle, vpoint, alpha, 2);

		vpoint = vorigin + m_vRUp * flup * (-pparticle->scale);
		vpoint = vpoint + m_vRRight * flright * (-pparticle->scale);
		BatchVertex(pparticle, vpoint, alpha, 3);
	}

	m_numIndexes += 6;
	m_numParticles++;

	rns.counters.particles++;
}

//====================================
//
//====================================
bool CParticleEngine::DrawParticles( prt_render_pass_e pass ) 
{
	if(pass == PARTICLES_VIEWMODEL && !rns.mainframe)
		return true;

	if(m_pCvarDrawParticles->GetValue() <= 0)
		return true;

	CMatrix matrix;
	CMatrix modelview(rns.view.modelview.GetMatrix());
	const Float *fltranspose = modelview.GetMatrix();

	m_numParticles = m_numVertexes = m_numIndexes = 0;

	m_particleSystemsList.begin();
	while(!m_particleSystemsList.end())
	{
		particle_system_t* psystem = m_particleSystemsList.get();
		psystem->numindexes = 0; // Always set this

		// Pointer to script definition
		const script_definition_t* pdefinition = psystem->pdefinition;

		if(pass == PARTICLES_SKY)
		{
			if(!psystem->pparticleheader 
				|| !(pdefinition->render_flags & RENDER_FL_SKYBOX))
			{
				m_particleSystemsList.next();
				continue;
			}
		}
		else if(pass == PARTICLES_VIEWMODEL)
		{
			if(!psystem->pparticleheader 
				|| psystem->parententity != cls.dllfuncs.pfnGetViewModel())
			{
				m_particleSystemsList.next();
				continue;
			}
		}
		else
		{
			if(!psystem->pparticleheader 
				|| (pdefinition->render_flags & RENDER_FL_SKYBOX) 
				|| psystem->parententity == cls.dllfuncs.pfnGetViewModel())
			{
				m_particleSystemsList.next();
				continue;
			}
		}

		// Don't render distortions in other renderpasses
		if(!rns.mainframe && pdefinition->rendermode == render_distort)
		{
			m_particleSystemsList.next();
			continue;
		}

		// Check if it's in VIS
		if(!psystem->parententity)
		{
			if(!psystem->leafnums.empty())
			{
				if(!Common::CheckVisibility(psystem->leafnums, rns.pvisbuffer))
				{
					m_particleSystemsList.next();
					continue;
				}
			}
		}
		else if(psystem->parententity != cls.dllfuncs.pfnGetViewModel())
		{
			cl_entity_t *pplayer = CL_GetLocalPlayer();
			if(psystem->parententity->curstate.msg_num != pplayer->curstate.msg_num)
			{
				m_particleSystemsList.next();
				continue;
			}
		}

		psystem->visframe = rns.visframe;

		if(pdefinition->lighting_flags != PARTICLE_LIGHTCHECK_NONE
			&& !(pdefinition->lighting_flags & PARTICLE_LIGHTCHECK_NO_DYNLIGHTS))
		{
			GetLights(psystem, psystem->pdlights, &psystem->numdlights, false, MAX_PARTICLE_POINT_LIGHTS);
			GetLights(psystem, psystem->pspotlights, &psystem->numspotlights, true, MAX_PARTICLE_PROJ_LIGHTS);
		}

		// Calculate up and right scalers
		Float up, right;
		if(pdefinition->numframes)
		{
			if(pdefinition->framesizex > pdefinition->framesizey)
			{
				up = (Float)pdefinition->framesizey/(Float)pdefinition->framesizex;
				right = 1.0;
			}
			else
			{
				right = (Float)pdefinition->framesizex/(Float)pdefinition->framesizey;
				up = 1.0;
			}
		}
		else
		{
			if(psystem->ptexture->width > psystem->ptexture->height)
			{
				up = (Float)psystem->ptexture->height/(Float)psystem->ptexture->width;
				right = 1.0;
			}
			else
			{
				right = (Float)psystem->ptexture->width/(Float)psystem->ptexture->height;
				up = 1.0;
			}
		}

		if(pdefinition->alignment == align_parallel)
		{
			Math::VectorCopy(rns.view.v_right, m_vRRight);
			Math::VectorClear(m_vRUp); m_vRUp[2] = 1;
		}
		else if(!pdefinition->rotationvel && !pdefinition->rotxvel && !pdefinition->rotyvel)
		{
			Math::VectorCopy(rns.view.v_right, m_vRRight);
			Math::VectorCopy(rns.view.v_up, m_vRUp);
		}

		// Set starting index
		psystem->indexoffset = m_numIndexes;

		// Batch all particles tied to this system
		cl_particle_t *pparticle = psystem->pparticleheader;
		while(pparticle)
		{
			BatchParticle(pparticle, up, right, fltranspose);
			cl_particle_t *pnext = pparticle->next;
			pparticle = pnext;
		}

		// Set number of indexes
		psystem->numindexes = m_numIndexes-psystem->indexoffset;

		m_particleSystemsList.next();
	}

	if(!m_numVertexes)
		return true;

	m_pVBO->Bind();
	if(!m_pShader->EnableShader())
	{
		Sys_ErrorPopup("Shader error: %s.", m_pShader->GetError());
		return false;
	}

	m_pShader->EnableAttribute(m_attribs.a_color);
	m_pShader->EnableAttribute(m_attribs.a_origin);
	m_pShader->EnableAttribute(m_attribs.a_texcoord);

	m_pShader->SetUniformMatrix4fv(m_attribs.u_projection, rns.view.projection.GetMatrix());
	m_pShader->SetUniformMatrix4fv(m_attribs.u_modelview, rns.view.modelview.GetMatrix());

	m_pShader->SetUniform1i(m_attribs.u_texture0, 0);

	bool result = true;

	if(rns.fog.settings.active)
	{
		result = m_pShader->SetDeterminator(m_attribs.d_fog, 1);
		m_pShader->SetUniform3f(m_attribs.u_fogcolor, rns.fog.settings.color[0], rns.fog.settings.color[1], rns.fog.settings.color[2]);
		m_pShader->SetUniform2f(m_attribs.u_fogparams, rns.fog.settings.end, 1.0f/((Float)rns.fog.settings.end-(Float)rns.fog.settings.start));
	}
	else
	{
		result = m_pShader->SetDeterminator(m_attribs.d_fog, 0);
	}

	if(!result)
	{
		Sys_ErrorPopup("Shader error: %s.", m_pShader->GetError());
		return false;
	}

	// Update the VBO
	m_pVBO->VBOSubBufferData(0, m_pVertexes, sizeof(particle_vertex_t)*m_numVertexes);

	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	// render-to texture, if it's needed
	rtt_texture_t* prttscreentexture = nullptr;
	rtt_texture_t* prttdistorttexture = nullptr;

	// Render systems
	m_particleSystemsList.begin();
	while(!m_particleSystemsList.end())
	{
		particle_system_t* psystem = m_particleSystemsList.get();
		if(!psystem->numindexes)
		{
			m_particleSystemsList.next();
			continue;
		}

		// Pointer to script definition
		const script_definition_t* pdefinition = psystem->pdefinition;

		if((pdefinition->render_flags & RENDER_FL_NOFOG) && rns.fog.settings.active)
			result = m_pShader->SetDeterminator(m_attribs.d_fog, 0);

		m_pShader->SetUniform1f(m_attribs.u_overbright, (pdefinition->render_flags & RENDER_FL_OVERBRIGHT) ? 1.0 : 0.0);

		// Alphatestm mode used
		Int32 alphatestMode = ALPHATEST_DISABLED;

		if(pdefinition->rendermode != render_distort)
		{
			if(!m_pShader->SetDeterminator(m_attribs.d_numpointlights, psystem->numdlights, false)
				|| !m_pShader->SetDeterminator(m_attribs.d_numprojlights, psystem->numspotlights, false))
			{
				result = false;
				break;
			}

			for(Uint32 i = 0; i < psystem->numdlights; i++)
			{
				Vector origin;
				Math::MatMultPosition(fltranspose, psystem->pdlights[i]->origin, &origin);

				m_pShader->SetUniform3f(m_attribs.point_lights[i].u_origin, origin[0], origin[1], origin[2]);
				m_pShader->SetUniform1f(m_attribs.point_lights[i].u_radius, psystem->pdlights[i]->radius);

				Vector vcolor;
				if(pdefinition->lighting_flags & PARTICLE_LIGHTCHECK_MIXP)
				{
					vcolor.x = psystem->pdlights[i]->color.x*pdefinition->primarycolor.x;
					vcolor.y = psystem->pdlights[i]->color.y*pdefinition->primarycolor.y;
					vcolor.z = psystem->pdlights[i]->color.z*pdefinition->primarycolor.z;
				}
				else if(pdefinition->lighting_flags & PARTICLE_LIGHTCHECK_INTENSITY)
				{
					Float avg = (psystem->pdlights[i]->color[0]+psystem->pdlights[i]->color[1]+psystem->pdlights[i]->color[2])/3;
					vcolor.x = psystem->pdlights[i]->color.x*avg;
					vcolor.y = psystem->pdlights[i]->color.y*avg;
					vcolor.z = psystem->pdlights[i]->color.z*avg;
				}
				else
				{
					Math::VectorCopy(psystem->pdlights[i]->color, vcolor);
				}

				// Apply lightstyles
				gDynamicLights.ApplyLightStyle(psystem->pdlights[i], vcolor);

				m_pShader->SetUniform3f(m_attribs.point_lights[i].u_color, vcolor[0], vcolor[1], vcolor[2]);
			}

			Int32 lastbind = 0;
			Int32 lastunit = 0;
			for(Uint32 i = 0; i < psystem->numspotlights; i++)
			{
				Vector origin;
				Math::MatMultPosition(fltranspose, psystem->pspotlights[i]->origin, &origin);

				m_pShader->SetUniform1f(m_attribs.proj_lights[i].u_radius, psystem->pspotlights[i]->radius);
				m_pShader->SetUniform3f(m_attribs.proj_lights[i].u_origin, origin[0], origin[1], origin[2]);

				Vector vcolor;
				if(pdefinition->lighting_flags & PARTICLE_LIGHTCHECK_MIXP)
				{
					vcolor.x = psystem->pspotlights[i]->color.x*pdefinition->primarycolor.x;
					vcolor.y = psystem->pspotlights[i]->color.y*pdefinition->primarycolor.y;
					vcolor.z = psystem->pspotlights[i]->color.z*pdefinition->primarycolor.z;
				}
				else if(pdefinition->lighting_flags & PARTICLE_LIGHTCHECK_INTENSITY)
				{
					Float avg = (psystem->pspotlights[i]->color[0]+psystem->pspotlights[i]->color[1]+psystem->pspotlights[i]->color[2])/3;
					vcolor.x = psystem->pspotlights[i]->color.x*avg;
					vcolor.y = psystem->pspotlights[i]->color.y*avg;
					vcolor.z = psystem->pspotlights[i]->color.z*avg;
				}
				else
				{
					Math::VectorCopy(psystem->pspotlights[i]->color, vcolor);
				}

				// Apply lightstyles
				gDynamicLights.ApplyLightStyle(psystem->pspotlights[i], vcolor);

				m_pShader->SetUniform3f(m_attribs.proj_lights[i].u_color, vcolor[0], vcolor[1], vcolor[2]);

				if(lastbind == (Int32)rns.objects.projective_textures[psystem->pspotlights[i]->textureindex]->palloc->gl_index)
				{
					m_pShader->SetUniform1i(m_attribs.proj_lights[i].u_texture, 2+lastunit);
				}
				else
				{
					m_pShader->SetUniform1i(m_attribs.proj_lights[i].u_texture, 2+i);
					R_Bind2DTexture(GL_TEXTURE2+i, rns.objects.projective_textures[psystem->pspotlights[i]->textureindex]->palloc->gl_index);

					lastunit = i;
					lastbind = rns.objects.projective_textures[psystem->pspotlights[i]->textureindex]->palloc->gl_index;
				}

				matrix.LoadIdentity();
				matrix.Translate(0.5, 0.5, 0.5);
				matrix.Scale(0.5, 0.5, 0.5);

				Float flsize = tan((M_PI/360) * psystem->pspotlights[i]->cone_size);
				matrix.SetFrustum(-flsize, flsize, -flsize, flsize, 1, psystem->pspotlights[i]->radius);

				Vector vforward, vtarget;
				Vector angles = psystem->pspotlights[i]->angles;
				Common::FixVector(angles);

				Math::AngleVectors(angles, &vforward, nullptr, nullptr);
				Math::VectorMA(psystem->pspotlights[i]->origin, psystem->pspotlights[i]->radius, vforward, vtarget);

				matrix.LookAt(psystem->pspotlights[i]->origin[0], psystem->pspotlights[i]->origin[1], psystem->pspotlights[i]->origin[2], vtarget[0], vtarget[1], vtarget[2], 0, 0, Common::IsPitchReversed(psystem->pspotlights[i]->angles[PITCH]) ? -1 : 1);
				matrix.MultMatrix(rns.view.modelview.GetInverse());

				m_pShader->SetUniformMatrix4fv(m_attribs.proj_lights[i].u_matrix, matrix.Transpose());
			}

			if(pdefinition->rendermode == render_alphatest)
			{
				glDisable(GL_BLEND);
				alphatestMode = (rns.msaa && rns.mainframe) ? ALPHATEST_COVERAGE : ALPHATEST_LESSTHAN;
				result = m_pShader->SetDeterminator(m_attribs.d_alphatest, alphatestMode);
			}
			else
			{
				result = m_pShader->SetDeterminator(m_attribs.d_alphatest, ALPHATEST_DISABLED);
			}

			if(!result)
				break;

			if(alphatestMode == ALPHATEST_COVERAGE)
			{
				glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
				gGLExtF.glSampleCoverage(0.5, GL_FALSE);
			}
		}
		else
		{
			if(!m_pShader->SetDeterminator(m_attribs.d_numpointlights, 0, false)
				|| !m_pShader->SetDeterminator(m_attribs.d_numprojlights, 0))
			{
				result = false;
				break;
			}

			// Make sure the RTT objects are present
			if(!prttscreentexture)
				prttscreentexture = gRTTCache.Alloc(rns.screenwidth, rns.screenheight, true);

			if(!prttdistorttexture)
				prttdistorttexture = gRTTCache.Alloc(rns.screenwidth, rns.screenheight, true);

			// Save the screen texture
			gGLExtF.glActiveTexture(GL_TEXTURE0_ARB);
			glEnable(GL_TEXTURE_RECTANGLE);

			R_BindRectangleTexture(GL_TEXTURE0_ARB, prttscreentexture->palloc->gl_index);
			glCopyTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, 0, 0, rns.screenwidth, rns.screenheight, 0);

			gGLExtF.glActiveTexture(GL_TEXTURE0_ARB);
			glEnable(GL_TEXTURE_RECTANGLE);

			// Clear the color buffer and begin rendering
			glClearColor(0.5, 0.5, 0.5, GL_ZERO);
			glClear(GL_COLOR_BUFFER_BIT);
		}

		// Set up blending
		if(pdefinition->rendermode != render_alphatest)
		{
			if(pdefinition->rendermode == render_alpha)
			{
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
			else
			{
				glBlendFunc(GL_SRC_ALPHA, GL_ONE);

				if(!(pdefinition->render_flags & RENDER_FL_NOFOG))
					m_pShader->SetUniform3f(m_attribs.u_fogcolor, 0, 0, 0);
			}
		}

		if(pdefinition->alignment == align_tracer)
		{
			rns.view.modelview.PushMatrix();
			rns.view.modelview.LoadIdentity();
			m_pShader->SetUniformMatrix4fv(m_attribs.u_modelview, rns.view.modelview.GetMatrix());
		}

		// Bind texture
		R_Bind2DTexture(GL_TEXTURE0, psystem->ptexture->palloc->gl_index);
		R_ValidateShader(m_pShader);

		// Render elements
		glDrawElements(GL_TRIANGLES, psystem->numindexes, GL_UNSIGNED_INT, BUFFER_OFFSET(psystem->indexoffset));

		if(pdefinition->alignment == align_tracer)
		{
			rns.view.modelview.PopMatrix();
			m_pShader->SetUniformMatrix4fv(m_attribs.u_modelview, rns.view.modelview.GetMatrix());
		}

		if(pdefinition->rendermode != render_alpha)
			m_pShader->SetUniform3f(m_attribs.u_fogcolor, rns.fog.settings.color[0], rns.fog.settings.color[1], rns.fog.settings.color[2]);

		if(pdefinition->rendermode == render_distort)
		{
			glDisable(GL_BLEND);

			// Grab the contents of the screen
			gGLExtF.glActiveTexture(GL_TEXTURE0_ARB);
			glEnable(GL_TEXTURE_RECTANGLE);

			R_BindRectangleTexture(GL_TEXTURE0_ARB, prttdistorttexture->palloc->gl_index);
			glCopyTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, 0, 0, rns.screenwidth, rns.screenheight, 0);

			// Disable everything special
			if(!m_pShader->SetDeterminator(m_attribs.d_numpointlights, 0, false)
				|| !m_pShader->SetDeterminator(m_attribs.d_numprojlights, 0, false)
				|| !m_pShader->SetDeterminator(m_attribs.d_fog, 0, false)
				|| !m_pShader->SetDeterminator(m_attribs.d_type, SHADER_PRT_DISTORT))
			{
				result = false;
				break;
			}

			rns.view.modelview.PushMatrix();
			rns.view.modelview.LoadIdentity();

			rns.view.projection.PushMatrix();
			rns.view.projection.LoadIdentity();
			rns.view.projection.Ortho(GL_ZERO, GL_ONE, GL_ONE, GL_ZERO, 0.1, 100);
			
			m_pShader->SetUniformMatrix4fv(m_attribs.u_projection, rns.view.projection.GetMatrix());
			m_pShader->SetUniformMatrix4fv(m_attribs.u_modelview, rns.view.modelview.GetMatrix());

			m_pShader->SetUniform1f(m_attribs.u_overbright, 0);
			m_pShader->SetUniform2f(m_attribs.u_scrsize, rns.screenwidth, rns.screenheight);

			rns.view.modelview.PopMatrix();
			rns.view.projection.PopMatrix();

			glDisable(GL_DEPTH_TEST);

			gGLExtF.glActiveTexture(GL_TEXTURE0_ARB);
			glEnable(GL_TEXTURE_RECTANGLE);
			R_BindRectangleTexture(GL_TEXTURE0_ARB, prttdistorttexture->palloc->gl_index);
			m_pShader->SetUniform1i(m_attribs.u_rtexture0, 0);

			gGLExtF.glActiveTexture(GL_TEXTURE1_ARB);
			glEnable(GL_TEXTURE_RECTANGLE);
			R_BindRectangleTexture(GL_TEXTURE1_ARB, prttscreentexture->palloc->gl_index);
			m_pShader->SetUniform1i(m_attribs.u_rtexture1, 1);

			R_ValidateShader(m_pShader);

			// Render elements
			glDrawArrays(GL_TRIANGLES, m_screenRectangleBase, 6);

			gGLExtF.glActiveTexture(GL_TEXTURE1_ARB);
			glDisable(GL_TEXTURE_RECTANGLE);

			gGLExtF.glActiveTexture(GL_TEXTURE0_ARB);
			glDisable(GL_TEXTURE_RECTANGLE);

			glEnable(GL_DEPTH_TEST);
			if(!m_pShader->SetDeterminator(m_attribs.d_type, SHADER_PRT_NORMAL, false)
				|| !m_pShader->SetDeterminator(m_attribs.d_fog, rns.fog.settings.active))
			{
				result = false;
				break;
			}

			m_pShader->SetUniformMatrix4fv(m_attribs.u_projection, rns.view.projection.GetMatrix());
			m_pShader->SetUniformMatrix4fv(m_attribs.u_modelview, rns.view.modelview.GetMatrix());

			glEnable(GL_BLEND);
		}
		else
		{
			if(pdefinition->rendermode == render_alphatest)
			{
				glEnable(GL_BLEND);

				if(alphatestMode == ALPHATEST_COVERAGE)
				{
					glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
					gGLExtF.glSampleCoverage(1.0, GL_FALSE);
				}
			}
		}


		if((pdefinition->render_flags & RENDER_FL_NOFOG) && rns.fog.settings.active)
			result = m_pShader->SetDeterminator(m_attribs.d_fog, 1);

		// advance
		m_particleSystemsList.next();
	}

	m_pShader->DisableAttribute(m_attribs.a_color);

	// Free render-to-texture objects
	if(prttscreentexture)
		gRTTCache.Free(prttscreentexture);

	if(prttdistorttexture)
		gRTTCache.Free(prttdistorttexture);

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);

	m_pShader->DisableShader();
	m_pVBO->UnBind();

	// Clear any binds
	R_ClearBinds();

	if(!result)
	{
		Sys_ErrorPopup("Shader error: %s.", m_pShader->GetError());
		return false;
	}

	return result;
}

//====================================
//
//====================================
void CParticleEngine::ReleaseSystem( particle_system_t* psystem )
{
	// Remove all related particles
	cl_particle_t *pparticle = psystem->pparticleheader;
	while(pparticle)
	{
		cl_particle_t *pfree = pparticle;
		pparticle = pfree->next;

		m_iNumFreedParticles++;
		RemoveParticle(pfree);
	}

	// Unlink this
	if(psystem->createsystem)
		psystem->createsystem->parentsystem = nullptr;

	// Unlink this
	if(psystem->watersystem)
		psystem->watersystem->parentsystem = nullptr;

	m_particleSystemsList.remove(m_particleSystemsList.get_link());
	m_iNumFreedSystems++;
	delete psystem;
}

//====================================
//
//====================================
void CParticleEngine::RemoveSystem( entindex_t entindex, Int32 iId, bool keepcache ) 
{
	if(m_particleSystemsList.empty())
		return;
		
	if(!iId)
		return;

	m_particleSystemsList.begin();
	while(!m_particleSystemsList.end())
	{
		particle_system_t *psystem = m_particleSystemsList.get();
		if(psystem->id != iId || psystem->entindex != entindex)
		{
			m_particleSystemsList.next();
			continue;
		}

		// Pointer to script definition
		const script_definition_t* pdefinition = psystem->pdefinition;

		if(!(pdefinition->flags & SYSTEM_FL_SOFTOFF))
		{
			// Just delete it
			ReleaseSystem(psystem);
		}
		else
		{
			// Reset these
			psystem->id = 0;
			psystem->entindex = 0;

			if(!pdefinition->softofftime)
			{
				// Do not spawn any more particles, so that
				// the code will delete this system once all
				// the particles have faded
				psystem->maxparticles = psystem->numspawns;
			}
			else
			{
				// Begin fading out
				psystem->softoffbegintime = cls.cl_time;
			}
		}

		m_particleSystemsList.next();
	}

	if(!keepcache)
	{
		m_msgCache.push_iterator();

		m_msgCache.begin();
		while(!m_msgCache.end())
		{
			const part_msg_cache_t& cache = m_msgCache.get();
			if(cache.entindex == entindex && cache.id == iId)
			{
				m_msgCache.remove(m_msgCache.get_link());
				continue;
			}

			m_msgCache.next();
		}

		m_msgCache.pop_iterator();
	}
}

//====================================
//
//====================================
__forceinline void CParticleEngine::RemoveParticle( cl_particle_t *particle ) 
{
	if(particle->prev) particle->prev->next = particle->next;
	else particle->psystem->pparticleheader = particle->next;
	if(particle->next) particle->next->prev = particle->prev;

	particle->next = m_pFreeParticles;
	m_pFreeParticles = particle;
}

//====================================
//
//====================================
void CParticleEngine::CacheCreateSystem( const Vector& origin, const Vector& direction, part_script_type_t scripttype, const Char* pstrFilepath, Uint32 id, entindex_t entindex, Int32 attachment, Int32 boneindex, Int32 attachflags )
{
	part_msg_cache_t newcache;

	newcache.origin = origin;
	newcache.direction = direction;
	newcache.msgtype = PART_MSG_SPAWN;
	newcache.scripttype = scripttype;
	newcache.file = pstrFilepath;
	newcache.id = id;
	newcache.entindex = entindex;
	newcache.attachment = attachment;
	newcache.boneindex = boneindex;
	newcache.attachflags = attachflags;

	m_msgCache.radd(newcache);
}

//====================================
//
//====================================
void CParticleEngine::CacheRemoveSystem( Int32 id, entindex_t entindex, bool keepcached )
{
	part_msg_cache_t newcache;
	newcache.id = id;
	newcache.entindex = entindex;
	newcache.msgtype = PART_MSG_REMOVE;
	newcache.keepcached = keepcached;

	m_msgCache.radd(newcache);
}

//====================================
//
//====================================
void CParticleEngine::KillEntityParticleSystems( Int32 entindex )
{
	if(m_particleSystemsList.empty())
		return;

	m_particleSystemsList.begin();
	while(!m_particleSystemsList.end())
	{
		particle_system_t *psystem = m_particleSystemsList.get();
		if(psystem->entindex == entindex && (psystem->attachflags & PARTICLE_ATTACH_TO_PARENT))
		{
			ReleaseSystem(psystem);
			m_particleSystemsList.next();
			continue;
		}

		m_particleSystemsList.next();
	}

	m_msgCache.begin();
	while(!m_msgCache.end())
	{
		part_msg_cache_t& cache = m_msgCache.get();
		if(cache.entindex == entindex && (cache.attachflags & PARTICLE_ATTACH_TO_PARENT))
		{
			m_msgCache.remove(m_msgCache.get_link());
			continue;
		}

		m_msgCache.next();
	}
}