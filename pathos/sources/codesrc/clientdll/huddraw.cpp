/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "fontset.h"
#include "huddraw.h"
#include "clientdll.h"
#include "r_glsl.h"
#include "r_vbo.h"
#include "matrix.h"
#include "textures_shared.h"
#include "r_common.h"

// Edge size 
const Float CHUDDraw::HUD_EDGE_SIZE = 4;
// Number of verts in VBO for body
const Uint32 CHUDDraw::NUM_BODY_VERTEXES = 30;
// Number of verts in VBO for a quad
const Uint32 CHUDDraw::NUM_QUAD_VERTEXES = 6;
// Number of verts in VBO
const Uint32 CHUDDraw::NUM_TOTAL_VERTEXES = CHUDDraw::NUM_BODY_VERTEXES + CHUDDraw::NUM_QUAD_VERTEXES;

// Class object definition
CHUDDraw gHUDDraw;

//=============================================
// @brief Constructor
//
//=============================================
CHUDDraw::CHUDDraw( void ):
	m_pShader(nullptr),
	m_pVBO(nullptr),
	m_screenWidth(0),
	m_screenHeight(0)
{
	for(Uint32 i = 0; i < 4; i++)
		m_alphaMod[i] = 0;
}

//=============================================
// @brief Destructor
//
//=============================================
CHUDDraw::~CHUDDraw( void )
{
}

//=============================================
//
//
//=============================================
bool CHUDDraw::Init( void )
{
	return true;
}

//=============================================
//
//
//=============================================
void CHUDDraw::Shutdown( void )
{
	ClearGL();
}

//=============================================
//
//
//=============================================
bool CHUDDraw::InitGL( void )
{
	// Get screen sizes
	cl_renderfuncs.pfnGetScreenSize(m_screenWidth, m_screenHeight);

	// Compile our shader
	m_pShader = new CGLSLShader(cl_filefuncs, cl_renderfuncs.pfnGetExportFunctionsClass(), "hud.bss");
	if(m_pShader->HasError())
	{
		cl_engfuncs.pfnErrorPopup("%s - Failed to compile shader: %s", __FUNCTION__, m_pShader->GetError());
		return false;
	}

	m_attribs.u_modelview = m_pShader->InitUniform("modelview", CGLSLShader::UNIFORM_MATRIX4);
	m_attribs.u_projection = m_pShader->InitUniform("projection", CGLSLShader::UNIFORM_MATRIX4);
	m_attribs.u_origin = m_pShader->InitUniform("origin", CGLSLShader::UNIFORM_FLOAT2);
	m_attribs.u_color = m_pShader->InitUniform("color", CGLSLShader::UNIFORM_FLOAT4);
	m_attribs.u_size = m_pShader->InitUniform("size", CGLSLShader::UNIFORM_FLOAT2);
	m_attribs.u_indent = m_pShader->InitUniform("indent", CGLSLShader::UNIFORM_FLOAT1);
	m_attribs.u_texture = m_pShader->InitUniform("texture0", CGLSLShader::UNIFORM_INT1);
	m_attribs.u_alphamod = m_pShader->InitUniform("alphamod", CGLSLShader::UNIFORM_FLOAT4);

	if(!R_CheckShaderUniform(m_attribs.u_modelview, "modelview", m_pShader, cl_engfuncs.pfnErrorPopup)
		|| !R_CheckShaderUniform(m_attribs.u_projection, "projection", m_pShader, cl_engfuncs.pfnErrorPopup)
		|| !R_CheckShaderUniform(m_attribs.u_origin, "origin", m_pShader, cl_engfuncs.pfnErrorPopup)
		|| !R_CheckShaderUniform(m_attribs.u_color, "color", m_pShader, cl_engfuncs.pfnErrorPopup)
		|| !R_CheckShaderUniform(m_attribs.u_size, "size", m_pShader, cl_engfuncs.pfnErrorPopup)
		|| !R_CheckShaderUniform(m_attribs.u_indent, "indent", m_pShader, cl_engfuncs.pfnErrorPopup)
		|| !R_CheckShaderUniform(m_attribs.u_texture, "texture0", m_pShader, cl_engfuncs.pfnErrorPopup)
		|| !R_CheckShaderUniform(m_attribs.u_alphamod, "alphamod", m_pShader, cl_engfuncs.pfnErrorPopup))
		return false;

	m_attribs.a_origin = m_pShader->InitAttribute("in_position", 4, GL_FLOAT, sizeof(hud_vertex_t), OFFSET(hud_vertex_t, origin));
	m_attribs.a_texcoord = m_pShader->InitAttribute("in_texcoord", 2, GL_FLOAT, sizeof(hud_vertex_t), OFFSET(hud_vertex_t, texcoord));
	m_attribs.a_sizemod = m_pShader->InitAttribute("in_sizemod", 2, GL_FLOAT, sizeof(hud_vertex_t), OFFSET(hud_vertex_t, sizemod));
	m_attribs.a_indent = m_pShader->InitAttribute("in_indent", 1, GL_FLOAT, sizeof(hud_vertex_t), OFFSET(hud_vertex_t, indent));
	m_attribs.a_alpha = m_pShader->InitAttribute("in_alpha", 1, GL_FLOAT, sizeof(hud_vertex_t), OFFSET(hud_vertex_t, alpha));
	m_attribs.a_alphamod = m_pShader->InitAttribute("in_alphamod", 4, GL_FLOAT, sizeof(hud_vertex_t), OFFSET(hud_vertex_t, alphamod));

	if(!R_CheckShaderVertexAttribute(m_attribs.a_origin, "in_position", m_pShader, cl_engfuncs.pfnErrorPopup)
		|| !R_CheckShaderVertexAttribute(m_attribs.a_texcoord, "in_texcoord", m_pShader, cl_engfuncs.pfnErrorPopup)
		|| !R_CheckShaderVertexAttribute(m_attribs.a_sizemod, "in_sizemod", m_pShader, cl_engfuncs.pfnErrorPopup)
		|| !R_CheckShaderVertexAttribute(m_attribs.a_indent, "in_indent", m_pShader, cl_engfuncs.pfnErrorPopup)
		|| !R_CheckShaderVertexAttribute(m_attribs.a_alpha, "in_alpha", m_pShader, cl_engfuncs.pfnErrorPopup)
		|| !R_CheckShaderVertexAttribute(m_attribs.a_alphamod, "in_alphamod", m_pShader, cl_engfuncs.pfnErrorPopup))
		return false;

	m_attribs.d_solid = m_pShader->GetDeterminatorIndex("solid");

	if(!R_CheckShaderDeterminator(m_attribs.d_solid, "solid", m_pShader, cl_engfuncs.pfnErrorPopup))
		return false;

	// Set up vbo
	hud_vertex_t* pverts = new hud_vertex_t[NUM_TOTAL_VERTEXES];

	// Construct body
	ConstructBody(pverts);
	// Construct quad
	ConstructQuad(pverts+NUM_BODY_VERTEXES);

	m_pVBO = new CVBO(cl_renderfuncs.pfnGetExportFunctionsClass(), pverts, sizeof(hud_vertex_t)*NUM_TOTAL_VERTEXES, 0, false);
	m_pShader->SetVBO(m_pVBO);

	delete[] pverts;

	return true;
}

//=============================================
//
//
//=============================================
void CHUDDraw::ClearGL( void )
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

//=============================================
//
//
//=============================================
void CHUDDraw::ConstructQuad( hud_vertex_t *pvertexes )
{
	//vertex 0
	pvertexes[0].texcoord[0] = 0; pvertexes[0].texcoord[1] = 1;
	pvertexes[0].sizemod[0] = 0; pvertexes[0].sizemod[1] = 1;
	pvertexes[0].alphamod[0] = 1.0;
	pvertexes[0].alpha = 1;

	//vertex 1
	pvertexes[1].texcoord[0] = 0; pvertexes[1].texcoord[1] = 0;
	pvertexes[1].sizemod[0] = 0; pvertexes[1].sizemod[1] = 0;
	pvertexes[1].alphamod[1] = 1.0;
	pvertexes[1].alpha = 1;

	//vertex 2
	pvertexes[2].texcoord[0] = 1; pvertexes[2].texcoord[1] = 0;
	pvertexes[2].sizemod[0] = 1; pvertexes[2].sizemod[1] = 0;
	pvertexes[2].alphamod[2] = 1.0;
	pvertexes[2].alpha = 1;

	//vertex 0
	pvertexes[3].texcoord[0] = 0; pvertexes[3].texcoord[1] = 1;
	pvertexes[3].sizemod[0] = 0; pvertexes[3].sizemod[1] = 1;
	pvertexes[3].alphamod[0] = 1.0;
	pvertexes[3].alpha = 1;

	//vertex 2
	pvertexes[4].texcoord[0] = 1; pvertexes[4].texcoord[1] = 0;
	pvertexes[4].sizemod[0] = 1; pvertexes[4].sizemod[1] = 0;
	pvertexes[4].alphamod[2] = 1.0;
	pvertexes[4].alpha = 1;

	//vertex 3
	pvertexes[5].texcoord[0] = 1; pvertexes[5].texcoord[1] = 1;
	pvertexes[5].sizemod[0] = 1; pvertexes[5].sizemod[1] = 1;
	pvertexes[5].alphamod[3] = 1.0;
	pvertexes[5].alpha = 1;
}

//=============================================
//
//
//=============================================
void CHUDDraw::ConstructBody( hud_vertex_t *pvertexes ) const
{
	Float flaspect = (Float)m_screenHeight/BASE_RESOLUTION_Y;

	// left border
	hud_vertex_t *pquad = pvertexes;

	// Vertex 0
	pquad[0].origin[0] = -(HUD_EDGE_SIZE*flaspect);
	pquad[0].origin[1] = (HUD_EDGE_SIZE*flaspect);
	pquad[0].sizemod[1] = 1; 
	pquad[0].indent = 1;

	// Vertex 1
	pquad[1].origin[0] = -(HUD_EDGE_SIZE*flaspect);
	pquad[1].origin[1] = -(HUD_EDGE_SIZE*flaspect);

	// Vertex 2
	pquad[2].alpha = 1;
	pquad[2].alphamod[0] = 1.0;

	// Vertex 3
	pquad[3].origin[0] = -(HUD_EDGE_SIZE*flaspect);
	pquad[3].origin[1] = (HUD_EDGE_SIZE*flaspect);
	pquad[3].sizemod[1] = 1; 
	pquad[3].indent = 1;

	// Vertex 4
	pquad[4].alpha = 1;
	pquad[4].alphamod[0] = 1.0;

	// Vertex 5
	pquad[5].alpha = 1; 
	pquad[5].indent = 1; 
	pquad[5].sizemod[1] = 1;
	pquad[5].alphamod[0] = 1.0;

	// top border
	pquad += 6;
	
	// Vertex 0
	pquad[0].alpha = 1;
	pquad[0].alphamod[0] = 1.0;

	// Vertex 1
	pquad[1].origin[0] = -(HUD_EDGE_SIZE*flaspect);
	pquad[1].origin[1] = -(HUD_EDGE_SIZE*flaspect);

	// Vertex 2
	pquad[2].origin[0] = (HUD_EDGE_SIZE*flaspect);
	pquad[2].origin[1] = -(HUD_EDGE_SIZE*flaspect);
	pquad[2].sizemod[0] = 1;

	// Vertex 3
	pquad[3].alpha = 1;
	pquad[3].alphamod[0] = 1.0;

	// Vertex 4
	pquad[4].origin[0] = (HUD_EDGE_SIZE*flaspect);
	pquad[4].origin[1] = -(HUD_EDGE_SIZE*flaspect);
	pquad[4].sizemod[0] = 1;

	// Vertex 5
	pquad[5].alpha = 1; 
	pquad[5].sizemod[0] = 1;
	pquad[5].alphamod[0] = 1.0;

	// right border
	pquad += 6;

	// Vertex 0
	pquad[0].alpha = 1;
	pquad[0].sizemod[0] = 1; 
	pquad[0].sizemod[1] = 1;
	pquad[0].indent = 1;
	pquad[0].alphamod[0] = 1.0;

	// Vertex 1
	pquad[1].alpha = 1;
	pquad[1].sizemod[0] = 1;
	pquad[1].alphamod[0] = 1.0;

	// Vertex 2
	pquad[2].origin[0] = (HUD_EDGE_SIZE*flaspect);
	pquad[2].origin[1] = -(HUD_EDGE_SIZE*flaspect);
	pquad[2].sizemod[0] = 1;

	// Vertex 3
	pquad[3].alpha = 1;
	pquad[3].sizemod[0] = 1; 
	pquad[3].sizemod[1] = 1;
	pquad[3].indent = 1;
	pquad[3].alphamod[0] = 1.0;

	// Vertex 4
	pquad[4].origin[0] = (HUD_EDGE_SIZE*flaspect);
	pquad[4].origin[1] = -(HUD_EDGE_SIZE*flaspect);
	pquad[4].sizemod[0] = 1;

	// Vertex 5
	pquad[5].origin[0] = (HUD_EDGE_SIZE*flaspect);
	pquad[5].origin[1] = (HUD_EDGE_SIZE*flaspect);
	pquad[5].sizemod[0] = 1; 
	pquad[5].sizemod[1] = 1; 
	pquad[5].indent = 1;

	// bottom border
	pquad += 6;

	// Vertex 0
	pquad[0].origin[0] = -(HUD_EDGE_SIZE*flaspect);
	pquad[0].origin[1] = (HUD_EDGE_SIZE*flaspect);
	pquad[0].sizemod[1] = 1; 
	pquad[0].indent = 1;

	// Vertex 1
	pquad[1].alpha = 1; 
	pquad[1].sizemod[1] = 1; 
	pquad[1].indent = 1;
	pquad[1].alphamod[0] = 1.0;

	// Vertex 2
	pquad[2].sizemod[0] = 1;
	pquad[2].sizemod[1] = 1;
	pquad[2].alpha = 1; 
	pquad[2].indent = 1;
	pquad[2].alphamod[0] = 1.0;

	// Vertex 3
	pquad[3].origin[0] = -(HUD_EDGE_SIZE*flaspect);
	pquad[3].origin[1] = (HUD_EDGE_SIZE*flaspect);
	pquad[3].sizemod[1] = 1; 
	pquad[3].indent = 1;

	// Vertex 4
	pquad[4].sizemod[0] = 1;
	pquad[4].sizemod[1] = 1;
	pquad[4].alpha = 1; 
	pquad[4].indent = 1;
	pquad[4].alphamod[0] = 1.0;

	// Vertex 5
	pquad[5].origin[0] = (HUD_EDGE_SIZE*flaspect);
	pquad[5].origin[1] = (HUD_EDGE_SIZE*flaspect);
	pquad[5].sizemod[0] = 1; 
	pquad[5].sizemod[1] = 1; 
	pquad[5].indent = 1;

	// body
	pquad += 6;

	// Vertex 0
	pquad[0].sizemod[1] = 1; 
	pquad[0].alpha = 1; 
	pquad[0].indent = 1;
	pquad[0].alphamod[0] = 1.0;

	// Vertex 1
	pquad[1].alpha = 1;
	pquad[1].alphamod[0] = 1.0;

	// Vertex 2
	pquad[2].sizemod[0] = 1; 
	pquad[2].alpha = 1;
	pquad[2].alphamod[0] = 1.0;

	// Vertex 3
	pquad[3].sizemod[1] = 1; 
	pquad[3].alpha = 1; 
	pquad[3].indent = 1;
	pquad[3].alphamod[0] = 1.0;

	// Vertex 4
	pquad[4].sizemod[0] = 1; 
	pquad[4].alpha = 1;
	pquad[4].alphamod[0] = 1.0;

	// Vertex 6
	pquad[5].sizemod[0] = 1; 
	pquad[5].sizemod[1] = 1; 
	pquad[5].alpha = 1; 
	pquad[5].indent = 1;
	pquad[5].alphamod[0] = 1.0;
}

//=============================================
//
//
//=============================================
bool CHUDDraw::SetupDraw( void )
{
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);

	CMatrix projection;
	projection.LoadIdentity();

	CMatrix modelview;
	modelview.LoadIdentity();
	modelview.Ortho(GL_ZERO, GL_ONE, GL_ONE, GL_ZERO, 0.1, 100);
	modelview.Scale(1.0f/(Float)m_screenWidth, 1.0f/(Float)m_screenHeight, 1.0);

	m_pVBO->Bind();

	if(!m_pShader->EnableShader())
		return false;

	m_pShader->SetUniformMatrix4fv(m_attribs.u_projection, projection.GetMatrix());
	m_pShader->SetUniformMatrix4fv(m_attribs.u_modelview, modelview.GetMatrix());
	m_pShader->SetUniform1i(m_attribs.u_texture, 0);

	m_pShader->EnableAttribute(m_attribs.a_origin);
	m_pShader->EnableAttribute(m_attribs.a_texcoord);
	m_pShader->EnableAttribute(m_attribs.a_sizemod);
	m_pShader->EnableAttribute(m_attribs.a_alpha);
	m_pShader->EnableAttribute(m_attribs.a_indent);
	m_pShader->EnableAttribute(m_attribs.a_alphamod);

	// Reset color
	m_color.r = m_color.g = m_color.b = m_color.a = 255;

	for(Uint32 i = 0; i < 4; i++)
		m_alphaMod[i] = 255;

	return true;
}

//=============================================
//
//
//=============================================
void CHUDDraw::FinishDraw( void )
{
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);

	m_pShader->DisableShader();
	m_pVBO->UnBind();
}

//=============================================
//
//
//=============================================
bool CHUDDraw::DrawBody( bool indent )
{
	if(!m_pShader->SetDeterminator(m_attribs.d_solid, true))
		return false;

	m_pShader->SetUniform4f(m_attribs.u_color, m_color.r/255.0f, m_color.g/255.0f, m_color.b/255.0f, m_color.a/255.0f);
	m_pShader->SetUniform4f(m_attribs.u_alphamod, 1.0, 1.0, 1.0, 1.0);

	if(indent)
		m_pShader->SetUniform1f(m_attribs.u_indent, 5.0f);
	else
		m_pShader->SetUniform1f(m_attribs.u_indent, 0);

	cl_renderfuncs.pfnValidateShader(m_pShader);

	glDrawArrays(GL_TRIANGLES, 0, NUM_BODY_VERTEXES);

	return true;
}

//=============================================
//
//
//=============================================
bool CHUDDraw::DrawQuad( en_texture_t *ptexture )
{
	m_pShader->SetUniform4f(m_attribs.u_color, m_color.r/255.0f, m_color.g/255.0f, m_color.b/255.0f, m_color.a/255.0f);
	m_pShader->SetUniform4f(m_attribs.u_alphamod, m_alphaMod[0]/255.0f,  m_alphaMod[1]/255.0f,  m_alphaMod[2]/255.0f,  m_alphaMod[3]/255.0f);

	if(ptexture)
	{
		if(!m_pShader->SetDeterminator(m_attribs.d_solid, false))
			return false;

		cl_renderfuncs.pfnBind2DTexture(GL_TEXTURE0_ARB, ptexture->palloc->gl_index, false);
	}
	else
	{
		if(!m_pShader->SetDeterminator(m_attribs.d_solid, true))
			return false;
	}

	cl_renderfuncs.pfnValidateShader(m_pShader);

	glDrawArrays(GL_TRIANGLES, NUM_BODY_VERTEXES, NUM_QUAD_VERTEXES);

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CHUDDraw::DrawText( const Vector& color, Float alpha, Float x, Float y, const Char *sztext, const font_set_t *pfont )
{
	FinishDraw();

	color32_t colorrgb(color[0], color[1], color[2], alpha);
	if(!cl_renderfuncs.pfnDrawSimpleString(colorrgb, x, y, sztext, pfont))
		return false;

	if(!SetupDraw())
		return false;

	return true;
}

//=============================================
//
//
//=============================================
void CHUDDraw::SetColor( byte r, byte g, byte b, byte a )
{
	m_color.r = r;
	m_color.g = g;
	m_color.b = b;
	m_color.a = a;
}

//=============================================
//
//
//=============================================
void CHUDDraw::SetColor( const Vector& rgb, byte a )
{
	m_color.r = rgb.x;
	m_color.g = rgb.y;
	m_color.b = rgb.z;
	m_color.a = a;
}

//=============================================
//
//
//=============================================
void CHUDDraw::SetAlphaMod( byte a1, byte a2, byte a3, byte a4 )
{
	m_alphaMod[0] = a1;
	m_alphaMod[1] = a2;
	m_alphaMod[2] = a3;
	m_alphaMod[3] = a4;
}

//=============================================
//
//
//=============================================
void CHUDDraw::SetOrigin( Float x, Float y ) 
{
	m_pShader->SetUniform2f(m_attribs.u_origin, x, y);
}

//=============================================
//
//
//=============================================
void CHUDDraw::SetSize( Float x, Float y )
{
	m_pShader->SetUniform2f(m_attribs.u_size, x, y);
}

//=============================================
// @brief
//
//=============================================
Float CHUDDraw::ScaleX( Float x ) const
{
	Float flaspect = (Float)m_screenHeight/(Float)BASE_RESOLUTION_Y;
	return x*flaspect;
}

//=============================================
// @brief
//
//=============================================
Float CHUDDraw::ScaleXRelative( Float x ) const
{
	Float flaspect = (Float)m_screenWidth/(Float)BASE_RESOLUTION_X;
	return x*flaspect;
}

//=============================================
// @brief
//
//=============================================
Float CHUDDraw::ScaleY( Float y ) const
{
	Float flaspect = (Float)m_screenHeight/(Float)BASE_RESOLUTION_Y;
	return y*flaspect;
}

//=============================================
// @brief Tells if the HUD renderer has any errors
//
//=============================================
bool CHUDDraw::HasError( void ) const
{
	if(!m_pShader)
		return true;
	else
		return m_pShader->HasError();
}

//=============================================
// @brief Returns the error message
//
//=============================================
const Char* CHUDDraw::GetError( void ) const
{
	if(!m_pShader)
		return "";
	else
		return m_pShader->GetError();
}

//=============================================
// @brief
//
//=============================================
void CHUDDraw::ManageErrorMessage( void ) const
{
	CString msg;
	msg << "Shader error: ";
	if(HasError())
		msg << GetError();
	else
		msg << cl_renderfuncs.pfnGetStringDrawError();

	cl_engfuncs.pfnErrorPopup(msg.c_str());
}