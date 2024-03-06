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
#include "r_main.h"
#include "r_cables.h"
#include "com_math.h"
#include "system.h"
#include "brushmodel.h"
#include "enginestate.h"
#include "file.h"
#include "r_common.h"
#include "cvar.h"

// Object definition
CCableRenderer gCableRenderer;

//====================================
//
//====================================
CCableRenderer::CCableRenderer( void ):
	m_pShader(nullptr),
	m_pVBO(nullptr)
{
}

//====================================
//
//====================================
CCableRenderer::~CCableRenderer( void )
{
	Shutdown();
}

//====================================
//
//====================================
bool CCableRenderer::Init( void )
{
	return true;
}

//====================================
//
//====================================
void CCableRenderer::Shutdown( void )
{
	ClearGL();
	ClearGame();
}

//====================================
//
//====================================
bool CCableRenderer::InitGL( void )
{
	if(!m_pShader)
	{
		Int32 shaderFlags = CGLSLShader::FL_GLSL_SHADER_NONE;
		if(R_IsExtensionSupported("GL_ARB_get_program_binary"))
			shaderFlags |= CGLSLShader::FL_GLSL_BINARY_SHADER_OPS;

		m_pShader = new CGLSLShader(FL_GetInterface(), gGLExtF, "cable.bss", shaderFlags, VID_ShaderCompileCallback);
		if(m_pShader->HasError())
		{
			Sys_ErrorPopup("%s - Failed to compile shader: %s.", __FUNCTION__, m_pShader->GetError());
			return false;
		}

		m_attribs.a_color = m_pShader->InitAttribute("in_color", 4, GL_FLOAT, sizeof(cable_vertex_t), OFFSET(cable_vertex_t, color));
		m_attribs.a_origin = m_pShader->InitAttribute("in_position", 4, GL_FLOAT, sizeof(cable_vertex_t), OFFSET(cable_vertex_t, origin));
		m_attribs.a_texcoord = m_pShader->InitAttribute("in_texcoord", 2, GL_FLOAT, sizeof(cable_vertex_t), OFFSET(cable_vertex_t, texcoord));
		m_attribs.a_width = m_pShader->InitAttribute("in_width", 1, GL_FLOAT, sizeof(cable_vertex_t), OFFSET(cable_vertex_t, width));
		m_attribs.a_vpoint = m_pShader->InitAttribute("in_vpoint", 3, GL_FLOAT, sizeof(cable_vertex_t), OFFSET(cable_vertex_t, vpoint));

		if(!R_CheckShaderVertexAttribute(m_attribs.a_color, "in_color", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderVertexAttribute(m_attribs.a_origin, "in_position", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderVertexAttribute(m_attribs.a_texcoord, "in_texcoord", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderVertexAttribute(m_attribs.a_width, "in_width", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderVertexAttribute(m_attribs.a_vpoint, "in_vpoint", m_pShader, Sys_ErrorPopup))
			return false;

		m_attribs.u_fogcolor = m_pShader->InitUniform("fogcolor", CGLSLShader::UNIFORM_NOSYNC);
		m_attribs.u_fogparams = m_pShader->InitUniform("fogparams", CGLSLShader::UNIFORM_NOSYNC);
		m_attribs.u_modelview = m_pShader->InitUniform("modelview", CGLSLShader::UNIFORM_NOSYNC);
		m_attribs.u_projection = m_pShader->InitUniform("projection", CGLSLShader::UNIFORM_NOSYNC);

		m_attribs.u_vorigin = m_pShader->InitUniform("vorigin", CGLSLShader::UNIFORM_NOSYNC);
		m_attribs.u_start = m_pShader->InitUniform("start", CGLSLShader::UNIFORM_NOSYNC);

		if(!R_CheckShaderUniform(m_attribs.u_fogcolor, "fogcolor", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_fogparams, "fogparams", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_modelview, "modelview", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_projection, "projection", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_vorigin, "vorigin", m_pShader, Sys_ErrorPopup)
			|| !R_CheckShaderUniform(m_attribs.u_start, "start", m_pShader, Sys_ErrorPopup))
			return false;

		m_attribs.d_fog = m_pShader->GetDeterminatorIndex("fog");

		if(!R_CheckShaderDeterminator(m_attribs.d_fog, "fog", m_pShader, Sys_ErrorPopup))
			return false;
	}

	if(m_pVBO)
	{
		// Rebind us
		m_pVBO->RebindGL();
		m_pShader->SetVBO(m_pVBO);
	}

	return true;
}

//====================================
//
//====================================
void CCableRenderer::ClearGL( void )
{
	if(m_pShader)
	{
		delete m_pShader;
		m_pShader = nullptr;
	}

	if(m_pVBO)
		m_pVBO->ClearGL();
}

//====================================
//
//====================================
bool CCableRenderer::InitGame( void )
{
	if(!m_pVBO)
	{
		m_pVBO = new CVBO(gGLExtF, true, false);
		m_pShader->SetVBO(m_pVBO);
	}

	for(Uint32 i = 0; i < m_cablesArray.size(); i++)
		InitCableVBOData(m_cablesArray[i]);

	return true;
}

//====================================
//
//====================================
void CCableRenderer::ClearGame( void )
{
	if(!m_cablesArray.empty())
		m_cablesArray.clear();

	if(m_pShader)
	{
		m_pShader->SetVBO(nullptr);
		m_pShader->ResetShader();
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
bool CCableRenderer::DrawCables( void )
{
	if(m_cablesArray.empty())
		return true;

	m_pVBO->Bind();
	if(!m_pShader->EnableShader())
	{
		Sys_ErrorPopup("Shader error: %s.\n", m_pShader->GetError());
		return false;
	}

	m_pShader->EnableAttribute(m_attribs.a_origin);
	m_pShader->EnableAttribute(m_attribs.a_color);
	m_pShader->EnableAttribute(m_attribs.a_texcoord);
	m_pShader->EnableAttribute(m_attribs.a_width);
	m_pShader->EnableAttribute(m_attribs.a_vpoint);

	if(rns.fog.settings.active)
	{
		m_pShader->SetDeterminator(m_attribs.d_fog, 1);
		m_pShader->SetUniform3f(m_attribs.u_fogcolor, rns.fog.settings.color[0], rns.fog.settings.color[1], rns.fog.settings.color[2]);
		m_pShader->SetUniform2f(m_attribs.u_fogparams, rns.fog.settings.end, 1.0f/((Float)rns.fog.settings.end-(Float)rns.fog.settings.start));
	}
	else
	{
		m_pShader->SetDeterminator(m_attribs.d_fog, 0);
	}

	m_pShader->SetUniformMatrix4fv(m_attribs.u_modelview, rns.view.modelview.GetMatrix());
	m_pShader->SetUniformMatrix4fv(m_attribs.u_projection, rns.view.projection.GetMatrix());

	m_pShader->SetUniform3f(m_attribs.u_vorigin, rns.view.v_origin[0], rns.view.v_origin[1], rns.view.v_origin[2]);

	glDisable(GL_CULL_FACE);

	for(Uint32 i = 0; i < m_cablesArray.size(); i++)
	{
		cable_object_t *pcable = &m_cablesArray[i];

		if(!Common::CheckVisibility(pcable->leafnums, rns.pvisbuffer))
			continue;

		if(rns.view.frustum.CullBBox(pcable->vmins, pcable->vmaxs))
			continue;

		m_pShader->SetUniform3f(m_attribs.u_start, pcable->start[0], pcable->start[1], pcable->start[2]);

		R_ValidateShader(m_pShader);

		glDrawArrays(GL_TRIANGLES, pcable->start_vertex, pcable->num_vertexes);
	}

	glEnable(GL_CULL_FACE);

	m_pShader->DisableShader();
	m_pVBO->UnBind();

	return true;
}

//====================================
//
//====================================
void CCableRenderer::InitCableVBOData( cable_object_t& cable )
{
	if(!cable.numsegments)
		return;

	Vector vbottom, vpoint;
	Math::VectorMA(cable.start, 0.5, (cable.end-cable.start), vbottom);
	vbottom[2] -= cable.falldepth;

	cable_vertex_t pvertexes[4];
	
	// set first segment
	Float f = 1.0f/(Float)cable.numsegments;

	for(Uint32 i = 0; i < 3; i++)
		vpoint[i] = cable.start[i]*((1-f)*(1-f))+vbottom[i]*((1-f)*f*2)+cable.end[i]*(f*f);

	for(Uint32 i = 0; i < 3; i++)
		pvertexes[0].origin[i] = cable.start[i];

	Math::VectorCopy(cable.start, pvertexes[0].origin);
	Math::VectorCopy(vpoint, pvertexes[0].vpoint);
	pvertexes[0].width = -(Float)cable.width;

	Math::VectorCopy(cable.start, pvertexes[1].origin);
	Math::VectorCopy(vpoint, pvertexes[1].vpoint);
	pvertexes[1].width = (Float)cable.width;
	
	Uint32 numverts = cable.numsegments*6;
	cable_vertex_t *pverts = new cable_vertex_t[numverts];
	Uint32 curvert = 0;

	for(Int32 i = 1; i < (cable.numsegments+1); i++)
	{
		f = (Float)i/(Float)cable.numsegments;
		for(Uint32 j = 0; j < 3; j++)
			vpoint[j] = cable.start[j]*((1-f)*(1-f))+vbottom[j]*((1-f)*f*2)+cable.end[j]*(f*f);

		// Set reference array
		Math::VectorCopy(vpoint, pvertexes[2].origin);
		Math::VectorCopy(vpoint, pvertexes[2].vpoint);
		pvertexes[2].width = (Float)cable.width;

		Math::VectorCopy(vpoint, pvertexes[3].origin);
		Math::VectorCopy(vpoint, pvertexes[3].vpoint);
		pvertexes[3].width = -(Float)cable.width;

		memcpy(&pverts[curvert], &pvertexes[0], sizeof(cable_vertex_t)); curvert++;
		memcpy(&pverts[curvert], &pvertexes[1], sizeof(cable_vertex_t)); curvert++;
		memcpy(&pverts[curvert], &pvertexes[2], sizeof(cable_vertex_t)); curvert++;

		memcpy(&pverts[curvert], &pvertexes[0], sizeof(cable_vertex_t)); curvert++;
		memcpy(&pverts[curvert], &pvertexes[2], sizeof(cable_vertex_t)); curvert++;
		memcpy(&pverts[curvert], &pvertexes[3], sizeof(cable_vertex_t)); curvert++;

		memcpy(&pvertexes[0], &pvertexes[3], sizeof(cable_vertex_t));
		memcpy(&pvertexes[1], &pvertexes[2], sizeof(cable_vertex_t));
	}

	cable.start_vertex = m_pVBO->GetVBOSize()/sizeof(cable_vertex_t);
	cable.num_vertexes = numverts;

	m_pVBO->Append(pverts, sizeof(cable_vertex_t)*numverts, nullptr, 0);
	delete[] pverts;
}
//====================================
//
//====================================
void CCableRenderer::AddCable( const Vector& start, const Vector& end, Uint32 depth, Uint32 width, Uint32 numsegments )
{
	cable_object_t newcable;
	newcable.start = start;
	newcable.end = end;
	newcable.falldepth = depth;
	newcable.width = width;
	newcable.numsegments = numsegments;

	Vector vbottom;
	Math::VectorMA(start, 0.5, (end-start), vbottom);
	vbottom[2] -= depth;

	Vector vmins = NULL_MINS;
	Vector vmaxs = NULL_MAXS;
	for(Uint32 i = 0; i < (numsegments+1); i++)
	{
		Float f = (Float)i/(Float)numsegments;

		Vector vpoint;
		for(Uint32 j = 0; j < 3; j++)
			vpoint[j] = start[j]*((1-f)*(1-f))+vbottom[j]*((1-f)*f*2)+end[j]*(f*f);

		for(Uint32 j = 0; j < 3; j++)
		{
			if(vpoint[j] > vmaxs[j])
				vmaxs[j] = vpoint[j];

			if(vpoint[j] < vmins[j])
				vmins[j] = vpoint[j];
		}
	}

	// prevent intersection
	Math::VectorSubtract(vmins, Vector(1, 1, 1), newcable.vmins);
	Math::VectorAdd(vmaxs, Vector(1, 1, 1), newcable.vmaxs);

	Mod_FindTouchedLeafs(ens.pworld, newcable.leafnums, newcable.vmins, newcable.vmaxs, ens.pworld->pnodes);

	// Add to the list
	m_cablesArray.push_back(newcable);
}
