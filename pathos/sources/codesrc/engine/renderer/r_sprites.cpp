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
#include "sprite.h"
#include "modelcache.h"
#include "cache_model.h"
#include "cl_main.h"
#include "r_main.h"
#include "r_common.h"
#include "trace.h"
#include "trace_shared.h"
#include "cl_pmove.h"
#include "cvar.h"
#include "r_sprites.h"
#include "system.h"
#include "console.h"
#include "texturemanager.h"
#include "com_math.h"
#include "file.h"
#include "cl_utils.h"
#include "r_glqueries.h"

// Max rendered sprites
const Uint32 CSpriteRenderer::MAX_RENDERED_SPRITES = 4096;

// Glow interpolation speed
const Float CSpriteRenderer::GLOW_INTERP_SPEED = 2;

// Glow minimum distance
const Float CSpriteRenderer::GLOW_MINDIST = 128;
// Glow maximum distance
const Float CSpriteRenderer::GLOW_MAXDIST = 1024;
// Glow halo distance
const Float CSpriteRenderer::GLOW_HALODIST = 2048;


// Class definition
CSpriteRenderer gSpriteRenderer;

//====================================
//
//====================================
CSpriteRenderer::CSpriteRenderer( void ) :
	m_pCvarDrawSprites(nullptr),
	m_pShader(nullptr),
	m_pVBO(nullptr),
	m_numIndexes(0),
	m_pVertexes(nullptr),
	m_occlusionQueryVBOBufferOffset(0),
	m_numVertexes(0),
	m_promptedLimitsThisFrame(false),
	m_glowStep(0)
{
	memset(m_viewMatrix, 0, sizeof(m_viewMatrix));

	// Allocate vertex array
	Uint32 vertexBufferSize = MAX_RENDERED_SPRITES*4;
	m_occlusionQueryVBOBufferOffset = vertexBufferSize;
	vertexBufferSize += GLOW_NUM_TRACES;

	m_pVertexes = new sprite_vertex_t[vertexBufferSize];
}

//====================================
//
//====================================
CSpriteRenderer::~CSpriteRenderer( void ) 
{
	Shutdown();
}

//====================================
//
//====================================
bool CSpriteRenderer::Init( void ) 
{
	m_pCvarDrawSprites = gConsole.CreateCVar(CVAR_FLOAT, FL_CV_CLIENT, "r_drawsprites", "1", "Toggles sprite rendering." );

	return true;
}

//====================================
//
//====================================
void CSpriteRenderer::Shutdown( void ) 
{
	ClearGame();
	ClearGL();

	if(m_pVertexes)
	{
		delete[] m_pVertexes;
		m_pVertexes = nullptr;
	}
}

//====================================
//
//====================================
bool CSpriteRenderer::InitGL( void ) 
{
	if(!m_pShader)
	{
		Int32 shaderFlags = CGLSLShader::FL_GLSL_SHADER_NONE;
		if(R_IsExtensionSupported("GL_ARB_get_program_binary"))
			shaderFlags |= CGLSLShader::FL_GLSL_BINARY_SHADER_OPS;

		m_pShader = new CGLSLShader(FL_GetInterface(), gGLExtF, "sprites.bss", shaderFlags, VID_ShaderCompileCallback);
		if(m_pShader->HasError())
		{
			Sys_ErrorPopup("%s - Failed to compile shader: %s.", __FUNCTION__, m_pShader->GetError());
			return false;
		}

		m_attribs.a_color = m_pShader->InitAttribute("in_color", 4, GL_FLOAT, sizeof(sprite_vertex_t), OFFSET(sprite_vertex_t, color));
		m_attribs.a_origin = m_pShader->InitAttribute("in_position", 4, GL_FLOAT, sizeof(sprite_vertex_t), OFFSET(sprite_vertex_t, origin));
		m_attribs.a_texcoord = m_pShader->InitAttribute("in_texcoord", 2, GL_FLOAT, sizeof(sprite_vertex_t), OFFSET(sprite_vertex_t, texcoord));

		if(!R_CheckShaderVertexAttribute(m_attribs.a_color, "in_color", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderVertexAttribute(m_attribs.a_origin, "in_position", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderVertexAttribute(m_attribs.a_texcoord, "in_texcoord", m_pShader, Sys_ErrorPopup))
			return false;

		m_attribs.u_fogcolor = m_pShader->InitUniform("fogcolor", CGLSLShader::UNIFORM_FLOAT3);
		m_attribs.u_fogparams = m_pShader->InitUniform("fogparams", CGLSLShader::UNIFORM_FLOAT2);
		m_attribs.u_modelview = m_pShader->InitUniform("modelview", CGLSLShader::UNIFORM_MATRIX4);
		m_attribs.u_projection = m_pShader->InitUniform("projection", CGLSLShader::UNIFORM_MATRIX4);
		m_attribs.u_texture = m_pShader->InitUniform("texture0", CGLSLShader::UNIFORM_INT1);

		if(!R_CheckShaderUniform(m_attribs.u_fogcolor, "fogcolor", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_fogparams, "fogparams", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_modelview, "modelview", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_projection, "projection", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_texture, "texture0", m_pShader, Sys_ErrorPopup))
			return false;

		m_attribs.d_fog = m_pShader->GetDeterminatorIndex("fog");
		m_attribs.d_solid = m_pShader->GetDeterminatorIndex("solid");

		if(!R_CheckShaderDeterminator(m_attribs.d_fog, "fog", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderDeterminator(m_attribs.d_solid, "solid", m_pShader, Sys_ErrorPopup))
			return false;
	}

	if(!m_pVBO)
	{
		Uint32 *pindexes = new Uint32[MAX_RENDERED_SPRITES*6];
		for(Uint32 i = 0, j = 0; i < MAX_RENDERED_SPRITES; i++, j += 6)
		{
			Uint32 baseval = (i*4);
			pindexes[j] = baseval; pindexes[j+1] = baseval+1; pindexes[j+2] = baseval+2;
			pindexes[j+3] = baseval; pindexes[j+4] = baseval+2; pindexes[j+5] = baseval+3;
		}

		Uint32 vertexBufferSize = MAX_RENDERED_SPRITES*4 + GLOW_NUM_TRACES;
		m_pVBO = new CVBO(gGLExtF, m_pVertexes, sizeof(sprite_vertex_t)*vertexBufferSize, pindexes, sizeof(Uint32)*MAX_RENDERED_SPRITES*6);
		m_pShader->SetVBO(m_pVBO);

		delete[] pindexes;
	}

	if(CL_IsGameActive())
	{
		// Reload any sprite textures
		InitGame();
	}

	return true;
}

//====================================
//
//====================================
void CSpriteRenderer::ClearGL( void ) 
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
bool CSpriteRenderer::InitGame( void ) 
{
	// Go through the model cache
	for(Uint32 i = 0; i < gModelCache.GetNbCachedModels(); i++)
	{
		cache_model_t* pmodel = gModelCache.GetModelByIndex(i+1);
		if(pmodel->type != MOD_SPRITE)
			continue;

		if(pmodel->isloaded)
			continue;

		R_LoadSprite(pmodel);
	}
	
	return true;
}

//====================================
//
//====================================
void CSpriteRenderer::ClearGame( void ) 
{
	if(!m_staticSpritesArray.empty())
		m_staticSpritesArray.clear();

	for(Uint32 i = 0; i < MAX_TEMP_SPRITES; i++)
		m_tempSpritesArray[i] = temp_sprite_t();
}

//====================================
//
//====================================
cl_entity_t* CSpriteRenderer::AllocStaticSprite( void ) 
{
	m_staticSpritesArray.resize(m_staticSpritesArray.size()+1);
	return &m_staticSpritesArray[m_staticSpritesArray.size()-1];
}

//====================================
//
//====================================
cl_entity_t* CSpriteRenderer::AllocTempSprite( Int32 key, Float life )
{
	// Find one with this key
	for(Uint32 i = 0; i < MAX_TEMP_SPRITES; i++)
	{
		if(m_tempSpritesArray[i].key == key)
		{
			m_tempSpritesArray[i].life = cls.cl_time + life;
			m_tempSpritesArray[i].key = key;
			m_tempSpritesArray[i].entity.entindex = key;
			m_tempSpritesArray[i].entity.curstate.entindex = key;
			m_tempSpritesArray[i].entity.prevstate.entindex = key;
			return &m_tempSpritesArray[i].entity;
		}
	}

	// Try to find an exhausted one
	for(Uint32 i = 0; i < MAX_TEMP_SPRITES; i++)
	{
		if( m_tempSpritesArray[i].life >= rns.time )
			continue;

		m_tempSpritesArray[i].entity.pmodel = nullptr;
		m_tempSpritesArray[i].life = cls.cl_time + life;
		m_tempSpritesArray[i].entity.curstate.renderfx = RenderFx_None;

		m_tempSpritesArray[i].key = key;
		m_tempSpritesArray[i].entity.entindex = key;
		m_tempSpritesArray[i].entity.curstate.entindex = key;
		m_tempSpritesArray[i].entity.prevstate.entindex = key;
		return &m_tempSpritesArray[i].entity;
	}

	// Just return the first one
	m_tempSpritesArray[0].entity.pmodel = nullptr;
	m_tempSpritesArray[0].life = cls.cl_time + life;
	m_tempSpritesArray[0].entity.curstate.renderfx = RenderFx_None;
	m_tempSpritesArray[0].key = key;
	m_tempSpritesArray[0].entity.entindex = key;
	m_tempSpritesArray[0].entity.curstate.entindex = key;
	m_tempSpritesArray[0].entity.prevstate.entindex = key;
	return &m_tempSpritesArray[0].entity;
}

//====================================
//
//====================================
void CSpriteRenderer::BatchSprites( cl_entity_t* entitiesArray, Uint32 numEntities )
{
	cl_entity_t* pplayer = CL_GetLocalPlayer();

	// Then engine sprites
	for(Uint32 i = 0; i < numEntities; i++)
	{
		cl_entity_t *pEntity = &entitiesArray[i];

		if(pEntity->curstate.renderamt == 0 || pEntity->curstate.rendermode == RENDER_NORMAL)
			continue;

		if(!pEntity->pmodel)
			continue;

		if(pEntity->pmodel->type != MOD_SPRITE)
			continue;

		if(!pEntity->curstate.scale)
			pEntity->curstate.scale = 1;

		if(pEntity->curstate.renderfx == RenderFx_SkyEnt && !rns.water_skydraw
			|| pEntity->curstate.renderfx != RenderFx_SkyEnt && rns.water_skydraw
			|| pEntity->curstate.renderfx == RenderFx_SkyEntScaled && !rns.water_skydraw
			|| pEntity->curstate.renderfx != RenderFx_SkyEntScaled && rns.water_skydraw
			|| pEntity->curstate.renderfx == RenderFx_SkyEntNC
			|| pEntity->curstate.renderfx == RenderFx_InPortalEntity && !rns.portalpass
			|| pEntity->curstate.renderfx != RenderFx_InPortalEntity && rns.portalpass)
			continue;

		if(R_IsSpecialRenderEntity(*pEntity))
			continue;

		// Manage parented sprites
		if(pEntity->curstate.movetype == MOVETYPE_FOLLOW && pEntity->curstate.aiment != NO_ENTITY_INDEX)
		{
			cl_entity_t* paiment = CL_GetEntityByIndex(pEntity->curstate.aiment);
			if(paiment->curstate.msg_num == pplayer->curstate.msg_num 
				&& paiment->pmodel && paiment->pmodel->type == MOD_VBM)
			{
				if(pEntity->curstate.body > 0 && pEntity->curstate.body <= MAX_ATTACHMENTS)
				{
					Int32 attachmentindex = pEntity->curstate.body - 1;
					pEntity->curstate.origin = paiment->getAttachment(attachmentindex);
				}
				else
				{
					// Just use the origin
					pEntity->curstate.origin = pEntity->curstate.origin;
				}
			}
			else
			{
				// Don't draw if entity is not visible
				continue;
			}
		}

		Vector vdir;
		Math::VectorSubtract(pEntity->curstate.origin, rns.view.v_origin, vdir);
		Math::VectorNormalize(vdir);

		// z clipped
		if(Math::DotProduct(vdir, rns.view.v_forward) < 0)
		{
			if(pEntity->glowstate.lastfrac.size() > rns.renderpassidx)
				pEntity->glowstate.lastfrac[rns.renderpassidx] = 0;
			continue;
		}

		// Check for recently loaded sprites
		if(!pEntity->pmodel->isloaded)
		{
			cache_model_t* pcache = gModelCache.GetModelByIndex(pEntity->curstate.modelindex);
			R_LoadSprite(pcache);
		}

		// Set this before batching
		pEntity->curstate.iuser1 = m_numIndexes;

		// Batch the particle
		if(BatchSprite(pEntity))
			pEntity->curstate.iuser2 = rns.framecount;
	}
}

//====================================
//
//====================================
bool CSpriteRenderer::DrawSpriteArrays( cl_entity_t* entitiesArray, Uint32 numEntities )
{
	cl_entity_t* pplayer = CL_GetLocalPlayer();
	if(!pplayer)
		return true;

	for(Uint32 i = 0; i < numEntities; i++)
	{
		cl_entity_t *pEntity = &entitiesArray[i];

		if(!pEntity->pmodel)
			continue;

		if(pEntity->pmodel->type != MOD_SPRITE)
			continue;

		if(pEntity->curstate.iuser2 != rns.framecount)
			continue;

		if (pEntity->curstate.rendermode == RENDER_TRANSADDITIVE || pEntity->curstate.rendermode == RENDER_TRANSGLOW)
		{
			glBlendFunc( GL_SRC_ALPHA, GL_ONE );
			if(rns.fog.settings.active)
				m_pShader->SetUniform3f(m_attribs.u_fogcolor, 0, 0, 0);
		}
		else
		{
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
			if(rns.fog.settings.active)
				m_pShader->SetUniform3f(m_attribs.u_fogcolor, rns.fog.settings.color[0], rns.fog.settings.color[1], rns.fog.settings.color[2]);
		}

		if(pEntity->curstate.rendermode == RENDER_TRANSGLOW)
		{
			glDisable(GL_DEPTH_TEST);
			if(rns.fog.settings.active && !m_pShader->SetDeterminator(m_attribs.d_fog, 0))
				return false;
		}

		const mspriteframe_t *frame = Sprite_GetFrame(pEntity->pmodel->getSprite(), pEntity->curstate.frame, rns.time);
		R_Bind2DTexture(GL_TEXTURE0, frame->ptexture->palloc->gl_index);
		R_ValidateShader(m_pShader);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, BUFFER_OFFSET(pEntity->curstate.iuser1));

		if(pEntity->curstate.rendermode == RENDER_TRANSGLOW)
		{
			glEnable(GL_DEPTH_TEST);
			if(rns.fog.settings.active && !m_pShader->SetDeterminator(m_attribs.d_fog, 1))
				return false;
		}
	}

	return true;
}

//====================================
//
//====================================
bool CSpriteRenderer::DrawSprites( void ) 
{
	if(m_pCvarDrawSprites->GetValue() <= 0)
	{
		R_ReleaseRenderPassQueryObjects(rns.renderpassidx, GL_QUERY_SPRITES);
		return true;
	}

	m_promptedLimitsThisFrame = false;

	// Bind now for glows
	m_pVBO->Bind();

	if(!m_pShader->EnableShader())
	{
		Sys_ErrorPopup("Shader error: %s.", __FUNCTION__);
		return false;
	}

	if(g_pCvarOcclusionQueries->GetValue() >= 1)
	{
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDepthMask(GL_FALSE);
		glPointSize(1.0);
	}

	m_pShader->EnableAttribute(m_attribs.a_origin);
	m_pShader->EnableAttribute(m_attribs.a_color);
	m_pShader->EnableAttribute(m_attribs.a_texcoord);

	m_pShader->SetUniform1i(m_attribs.u_texture, 0);

	m_pShader->SetUniformMatrix4fv(m_attribs.u_modelview, rns.view.modelview.GetMatrix());
	m_pShader->SetUniformMatrix4fv(m_attribs.u_projection, rns.view.projection.GetMatrix());

	if(!m_pShader->SetDeterminator(m_attribs.d_solid, TRUE))
	{
		Sys_ErrorPopup("Shader error: %s.", __FUNCTION__);
		m_pShader->DisableShader();
		m_pVBO->UnBind();
		return false;
	}

	// Reset these two
	m_numIndexes = m_numVertexes = 0;

	// Set the view matrix
	Math::AngleMatrix(rns.view.v_angles, m_viewMatrix);

	// Batch clientside sprites
	if(!m_staticSpritesArray.empty())
		BatchSprites(&m_staticSpritesArray[0], m_staticSpritesArray.size());

	// Now batch engine sprites
	for(Uint32 i = 0; i < rns.objects.numvisents; i++)
		BatchSprites(rns.objects.pvisents[i], 1);

	// Add tempent sprites
	for(Uint32 i = 0; i < MAX_TEMP_SPRITES; i++)
	{
		if(m_tempSpritesArray[i].life < rns.time)
			continue;

		if(!m_tempSpritesArray[i].entity.pmodel)
			continue;

		if(m_tempSpritesArray[i].entity.pmodel->type != MOD_SPRITE)
		{
			Con_EPrintf("%s - VBM model specified for sprite tempentity.\n", __FUNCTION__);
			continue;
		}

		BatchSprites(&m_tempSpritesArray[i].entity, 1);
	}

	if(g_pCvarOcclusionQueries->GetValue() >= 1)
	{
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);
	}

	if(!m_numVertexes)
	{
		R_ReleaseRenderPassQueryObjects(rns.renderpassidx, GL_QUERY_SPRITES);
		m_pShader->DisableShader();
		m_pVBO->UnBind();
		return true;
	}

	if(!m_pShader->SetDeterminator(m_attribs.d_solid, FALSE))
	{
		Sys_ErrorPopup("Shader error: %s.", __FUNCTION__);
		m_pShader->DisableShader();
		m_pVBO->UnBind();
		return false;
	}

	// Upload data
	m_pVBO->VBOSubBufferData(0, m_pVertexes, sizeof(sprite_vertex_t)*m_numVertexes);

	bool result = true;

	if(rns.fog.settings.active)
	{
		result = m_pShader->SetDeterminator(m_attribs.d_fog, 1);
		m_pShader->SetUniform3f(m_attribs.u_fogcolor, rns.fog.settings.color[0], rns.fog.settings.color[1], rns.fog.settings.color[2]);
		m_pShader->SetUniform2f(m_attribs.u_fogparams, rns.fog.settings.end, 1.0f/(static_cast<Float>(rns.fog.settings.end)- static_cast<Float>(rns.fog.settings.start)));
	}
	else
	{
		result = m_pShader->SetDeterminator(m_attribs.d_fog, 0);
	}

	if(!result)
	{
		Sys_ErrorPopup("Shader error: %s.", __FUNCTION__);
		m_pShader->DisableShader();
		m_pVBO->UnBind();
		return false;
	}

	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);

	// Draw clientside sprites
	if(!m_staticSpritesArray.empty())
		result = DrawSpriteArrays( &m_staticSpritesArray[0], m_staticSpritesArray.size() );

	// Now draw engine sprites
	if(result)
	{
		for(Uint32 i = 0; i < rns.objects.numvisents; i++)
		{
			result = DrawSpriteArrays( rns.objects.pvisents[i], 1 );
			if(!result)
				break;
		}
	}

	// Draw temp sprites
	if(result)
	{
		for(Uint32 i = 0; i < MAX_TEMP_SPRITES; i++)
		{
			if(m_tempSpritesArray[i].life < rns.time)
				continue;

			if(!m_tempSpritesArray[i].entity.pmodel)
				continue;

			if(m_tempSpritesArray[i].entity.pmodel->type != MOD_SPRITE)
			{
				Con_EPrintf("%s - VBM model specified for sprite tempentity.\n", __FUNCTION__);
				continue;
			}

			result = DrawSpriteArrays( &m_tempSpritesArray[i].entity, 1 );
			if(!result)
				break;
		}
	}

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);

	m_pShader->DisableShader();
	m_pVBO->UnBind();

	// Clear any binds
	R_ClearBinds();

	if(!result)
		Sys_ErrorPopup("Shader error: %s.", __FUNCTION__);

	R_ReleaseRenderPassQueryObjects(rns.renderpassidx, GL_QUERY_SPRITES);
	return result;
}

//====================================
//
//====================================
bool CSpriteRenderer::BatchSprite( cl_entity_t *pEntity ) 
{
	if(m_numVertexes >= MAX_RENDERED_SPRITES*4)
	{
		if(!m_promptedLimitsThisFrame)
		{
			Con_Printf("%s - Exceeded MAX_RENDERED_SPRITES*4.\n", __FUNCTION__);
			m_promptedLimitsThisFrame = true;
		}

		return false;
	}

	vec4_t vColor;
	static Vector vForward, vRight, vUp;

	const cache_model_t* pcache = pEntity->pmodel;
	const msprite_t* psprite = pcache->getSprite();
	const mspriteframe_t *pframe = Sprite_GetFrame(psprite, pEntity->curstate.frame, rns.time);

	Float spriteSize = (pframe->width > pframe->height) ? pframe->width : pframe->height;

	Vector mins;
	Vector maxs;
	for(Uint32 i = 0; i < 3; i++)
	{
		mins[i] = pEntity->curstate.origin[i] - spriteSize * pEntity->curstate.scale;
		maxs[i] = pEntity->curstate.origin[i] + spriteSize * pEntity->curstate.scale;
	}

	if(rns.view.frustum.CullBBox(mins, maxs))
	{
		if(pEntity->glowstate.lastfrac.size() > rns.renderpassidx)
			pEntity->glowstate.lastfrac[rns.renderpassidx] = 0;
		return false;
	}

	Float flScale = pEntity->curstate.scale;
	Float flAlpha = R_RenderFxBlend(pEntity)/255.0f;
	
	if(flAlpha <= 0) 
		flAlpha = 1.0f;

	if(pEntity->curstate.rendermode == RENDER_TRANSGLOW && pEntity->curstate.renderfx != RenderFx_NoDissipation)
	{
		Float flDist = (pEntity->curstate.origin - rns.view.v_origin).Length();
		if(flDist > GLOW_MAXDIST)
			flDist = GLOW_MAXDIST;

		Float flBrightness = ((GLOW_MAXDIST - flDist)/(GLOW_MAXDIST-GLOW_MINDIST));
		if(flBrightness < 0.25) flBrightness = 0.25;
		if(flBrightness > 1) flBrightness = 1;
		flAlpha *= flBrightness;
	}

	if(pEntity->curstate.rendermode == RENDER_TRANSGLOW)
	{
		if(pEntity->curstate.renderfx != RenderFx_NoDissipation)
		{
			Float flDist = (pEntity->curstate.origin - rns.view.v_origin).Length();
		
			if(flDist > GLOW_MAXDIST)
				flDist = GLOW_MAXDIST;

			flScale = flScale+2.0*(flDist/GLOW_MAXDIST);
		}
		
		bool useQueries = (g_pCvarOcclusionQueries->GetValue() >= 1) ? true : false;
		bool traceAll = (g_pCvarTraceGlow->GetValue() >= 1 || pEntity->curstate.renderfx == RenderFx_TraceGlow) ? true : false;
		Float occlusionFactor = R_CalcOcclusionFactor(pEntity->curstate.origin,
			pEntity->entindex, pframe->width,
			flScale, GLOW_INTERP_SPEED,
			GLOW_NUM_TRACES, GL_QUERY_SPRITES, pEntity->glowstate,
			useQueries, traceAll, false, false,
			m_viewMatrix, this,
			nullptr, SPR_DrawFunction);

		Float dst = Math::DotProduct(rns.view.v_forward, pEntity->curstate.origin - rns.view.v_origin);
		dst = clamp(dst, 0, 64);
		dst = dst / 64;

		flAlpha = flAlpha * occlusionFactor * dst;

		// Add in fog fade
		if(rns.fog.settings.active)
		{
			Float f = 1.0f/(static_cast<Float>(rns.fog.settings.end)- static_cast<Float>(rns.fog.settings.start));

			Float length = (rns.view.v_origin - pEntity->curstate.origin).Length();
			Float fogfactor = (rns.fog.settings.end - length)*f;
			flAlpha *= clamp(fogfactor, 0, 1);
		}

		// Save last time this was visible
		pEntity->glowstate.lastrendertime = rns.time;
	}

	if(pEntity->curstate.renderfx == RenderFx_AngularSprite)
	{
		Vector angles, forward;
		Math::VectorCopy(pEntity->curstate.angles, angles);
		Common::FixVector(angles);

		Math::AngleVectors( angles, &forward, nullptr, nullptr );

		if(!pEntity->curstate.fuser1)
			pEntity->curstate.fuser1 = 90;

		Vector vDirLight = pEntity->curstate.origin - rns.view.v_origin;
		Math::VectorNormalize(vDirLight);

		Float spotcos = cos((pEntity->curstate.fuser1)*(M_PI/360));
		Float atten = 1.0 - Math::DotProduct( forward, vDirLight );
		Float multiplier = (atten - spotcos)/(1 - spotcos);
		if(multiplier > 1)
			multiplier = 1;
		else if(multiplier < 0)
			multiplier = 0;

		flAlpha *= multiplier;

		if(flAlpha < 0)
			flAlpha = 0;
	}

	if (psprite->type == SPR_ORIENTED)
	{
		Math::AngleVectors (pEntity->curstate.angles, &vForward, &vRight, &vUp);
	}
	else if (psprite->type == SPR_FACING_UPRIGHT)
	{
		vUp[0] = vUp[1] = 0;
		vUp[2] = 1;
		vRight[0] = pEntity->curstate.origin[1] - rns.view.v_origin[1];
		vRight[1] = -(pEntity->curstate.origin[0] - rns.view.v_origin[0]);
		vRight[2] = 0;
		Math::VectorNormalize(vRight);
		Math::VectorCopy (rns.view.v_forward, vForward);
	}
	else if (psprite->type == SPR_VP_PARALLEL_UPRIGHT)
	{
		vUp[0] = vUp[1] = 0;
		vUp[2] = 1;
		Math::VectorCopy (rns.view.v_right, vRight);
		Math::VectorCopy (rns.view.v_forward, vForward);
	}
	else
	{     // normal sprite
		Math::VectorCopy (rns.view.v_up, vUp);
		Math::VectorCopy (rns.view.v_right, vRight);
		Math::VectorCopy (rns.view.v_forward, vForward);
	}

	if (pEntity->curstate.rendercolor.x || pEntity->curstate.rendercolor.y || pEntity->curstate.rendercolor.z)
	{
		vColor[0] = static_cast<Float>(pEntity->curstate.rendercolor.x)/255.0f;
		vColor[1] = static_cast<Float>(pEntity->curstate.rendercolor.y)/255.0f;
		vColor[2] = static_cast<Float>(pEntity->curstate.rendercolor.z)/255.0f;
		vColor[3] = flAlpha;
	}
	else
	{
		vColor[0] = 1.0;
		vColor[1] = 1.0;
		vColor[2] = 1.0;
		vColor[3] = flAlpha;
	}

	Vector vPoint = pEntity->curstate.origin - vForward * 0.01 + vUp * pframe->up * flScale;
	vPoint = vPoint + vRight * pframe->left * flScale;
	BatchVertex( vPoint, vColor, 0, 0 );

	vPoint = pEntity->curstate.origin - vForward * 0.01 + vUp * pframe->up * flScale;
	vPoint = vPoint + vRight * pframe->right * flScale;
	BatchVertex( vPoint, vColor, 1, 0 );

	vPoint = pEntity->curstate.origin - vForward * 0.01 + vUp * pframe->down * flScale;
	vPoint = vPoint + vRight * pframe->right * flScale;
	BatchVertex( vPoint, vColor, 1, 1 );

	vPoint = pEntity->curstate.origin - vForward * 0.01 + vUp * pframe->down * flScale;
	vPoint = vPoint + vRight * pframe->left * flScale;
	BatchVertex( vPoint, vColor, 0, 1 );

	m_numIndexes += 6;

	return true;
}

//====================================
//
//====================================
void CSpriteRenderer::BatchVertex( const Vector& vertex, const vec4_t& color, Float texcoords, Float texcoordt ) 
{
	sprite_vertex_t *pVertex = &m_pVertexes[m_numVertexes];
	m_numVertexes++;

	for(Uint32 j = 0; j < 3; j++)
		pVertex->origin[j] = vertex[j];
	pVertex->origin[3] = 1.0;

	pVertex->color[0] = color[0];
	pVertex->color[1] = color[1];
	pVertex->color[2] = color[2];
	pVertex->color[3] = color[3];

	pVertex->texcoord[0] = texcoords;
	pVertex->texcoord[1] = texcoordt;
}

//====================================
//
//====================================
void CSpriteRenderer::BatchVertex( Uint32 position, const Vector& vertex, Float r, Float g, Float b, Float a, Float texcoords, Float texcoordt )
{
	sprite_vertex_t& bufferVertex = m_pVertexes[position];

	Math::VectorCopy(vertex, bufferVertex.origin);
	bufferVertex.origin[3] = 1.0;

	bufferVertex.color[0] = r;
	bufferVertex.color[1] = g;
	bufferVertex.color[2] = b;
	bufferVertex.color[3] = a;

	bufferVertex.texcoord[0] = texcoords;
	bufferVertex.texcoord[1] = texcoordt;
}

//====================================
//
//====================================
void CSpriteRenderer::Animate( void ) 
{
	for(Uint32 i = 0; i < m_staticSpritesArray.size(); i++)
	{
		cl_entity_t *pEntity = &m_staticSpritesArray[i];
		if(!pEntity->pmodel)
			continue;

		if(pEntity->pmodel->type != MOD_SPRITE)
		{
			Con_EPrintf("%s - VBM model specified for sprite tempentity.\n", __FUNCTION__);
			continue;
		}

		const msprite_t *pSprite = pEntity->pmodel->getSprite();

		if(pSprite->frames.size() == 1)
			continue;

		if(!pEntity->curstate.framerate)
			pEntity->curstate.framerate = 1;

		if(!pEntity->curstate.animtime)
		{
			pEntity->curstate.animtime = rns.time;
			continue;
		}
		
		Float flDelta = rns.time-pEntity->curstate.animtime;
		pEntity->curstate.frame = static_cast<Int32>((pEntity->curstate.framerate*15)*flDelta) % (pSprite->frames.size()-1);
	}
	
	for(Uint32 i = 0; i < MAX_TEMP_SPRITES; i++)
	{
		if(m_tempSpritesArray[i].life < rns.time)
		{
			// Clear this entity
			if(m_tempSpritesArray[i].entity.pmodel)
				m_tempSpritesArray[i].entity.pmodel = nullptr;

			continue;
		}

		if(!m_tempSpritesArray[i].entity.pmodel)
			continue;

		if(m_tempSpritesArray[i].entity.pmodel->type != MOD_SPRITE)
		{
			Con_EPrintf("%s - VBM model specified for sprite tempentity.\n", __FUNCTION__);
			continue;
		}

		cl_entity_t *pEntity = &m_tempSpritesArray[i].entity;
		const msprite_t *pSprite = pEntity->pmodel->getSprite();

		if(pSprite->frames.size() == 1)
			continue;

		if(!pEntity->curstate.framerate)
			pEntity->curstate.framerate = 1;

		if(!pEntity->curstate.animtime)
		{
			pEntity->curstate.animtime = rns.time;
			continue;
		}
		
		Float flDelta = rns.time-pEntity->curstate.animtime;
		pEntity->curstate.frame = static_cast<Int32>((pEntity->curstate.framerate*15)*flDelta) % (pSprite->frames.size()-1);
	}
}

//====================================
//
//====================================
void CSpriteRenderer::DrawFunction( const Vector& origin )
{
	BatchVertex(m_occlusionQueryVBOBufferOffset, origin, 1.0, 1.0, 1.0, 1.0, 0, 0);

	sprite_vertex_t* pVertex = m_pVertexes + m_occlusionQueryVBOBufferOffset;
	m_pVBO->VBOSubBufferData(sizeof(sprite_vertex_t)*m_occlusionQueryVBOBufferOffset, pVertex, sizeof(sprite_vertex_t));

	glDrawArrays(GL_POINTS, m_occlusionQueryVBOBufferOffset, 1);
}

//====================================
//
//====================================
void SPR_DrawFunction( void* pContext, const Vector& origin )
{
	static_cast<CSpriteRenderer*>(pContext)->DrawFunction(origin);
}