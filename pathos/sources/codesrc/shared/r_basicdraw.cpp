/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.

===============================================
*/

#include "includes.h"
#include "common.h"
#include "com_math.h"
#include "file.h"

#include "r_vbo.h"
#include "r_glsl.h"
#include "r_basicdraw.h"
#include "r_glextf.h"
#include "r_common.h"

// Increment by 4096 vertexes
const Uint32 CBasicDraw::BASICDRAW_CACHE_SIZE = 4096;

// Current instance of the class
CBasicDraw* CBasicDraw::g_pInstance = nullptr;

//=============================================
// @brief Constructor
//
//=============================================
CBasicDraw::CBasicDraw( void ):
	m_isActive(false),
	m_numVertexes(0),
	m_primitiveType(GL_NONE),
	m_pVBO(nullptr),
	m_pShader(nullptr),
	m_brightness(1.0)
{
	for(Uint32 i = 0; i < 4; i++)
		m_vertexColor[i] = 1.0;
}

//=============================================
// @brief Destructor
//
//=============================================
CBasicDraw::~CBasicDraw( void )
{
	ClearGL();
}

//=============================================
// @brief Initializes OpenGL stuff
//
// @return Success status
//=============================================
bool CBasicDraw::InitGL( const CGLExtF& gGlExtF, const file_interface_t& fileFuncs, pfnErrorPopup_t pfnErrorPopup )
{
	// Allocate vertex array
	if(m_vertexesArray.empty())
		m_vertexesArray.resize(BASICDRAW_CACHE_SIZE);

	// Allocate VBO if needed
	if(!m_pVBO)
		m_pVBO = new CVBO(gGlExtF, &m_vertexesArray[0], sizeof(basic_vertex_t)*m_vertexesArray.size(), nullptr, 0, true);

	// Load shader in if needed
	if(!m_pShader)
	{
		m_pShader = new CGLSLShader(fileFuncs, gGlExtF, "basicdraw.bss");
		if(m_pShader->HasError())
		{
			pfnErrorPopup("%s - Could not compile shader: %s.", __FUNCTION__, m_pShader->GetError());
			return false;
		}

		// Get attrib info
		m_shaderAttribs.u_modelview = m_pShader->InitUniform("modelview", CGLSLShader::UNIFORM_MATRIX4);
		m_shaderAttribs.u_projection = m_pShader->InitUniform("projection", CGLSLShader::UNIFORM_MATRIX4);
		m_shaderAttribs.u_texture = m_pShader->InitUniform("texture0", CGLSLShader::UNIFORM_INT1);
		m_shaderAttribs.u_rectangle = m_pShader->InitUniform("rectangle0", CGLSLShader::UNIFORM_INT1);
		m_shaderAttribs.u_multiplier = m_pShader->InitUniform("multiplier", CGLSLShader::UNIFORM_FLOAT1);
		m_shaderAttribs.u_fogcolor = m_pShader->InitUniform("fogcolor", CGLSLShader::UNIFORM_FLOAT3);
		m_shaderAttribs.u_fogparams = m_pShader->InitUniform("fogparams", CGLSLShader::UNIFORM_FLOAT2);

		if(!R_CheckShaderUniform(m_shaderAttribs.u_modelview, "modelview", m_pShader, pfnErrorPopup)
			|| !R_CheckShaderUniform(m_shaderAttribs.u_projection, "projection", m_pShader, pfnErrorPopup)
			|| !R_CheckShaderUniform(m_shaderAttribs.u_texture, "texture0", m_pShader, pfnErrorPopup)
			|| !R_CheckShaderUniform(m_shaderAttribs.u_rectangle, "rectangle0", m_pShader, pfnErrorPopup)
			|| !R_CheckShaderUniform(m_shaderAttribs.u_fogcolor, "fogcolor", m_pShader, pfnErrorPopup)
			|| !R_CheckShaderUniform(m_shaderAttribs.u_fogparams, "fogparams", m_pShader, pfnErrorPopup))
			return false;

		m_shaderAttribs.a_position = m_pShader->InitAttribute("in_position", 4, GL_FLOAT, sizeof(basic_vertex_t), OFFSET(basic_vertex_t, origin));
		m_shaderAttribs.a_texcoord = m_pShader->InitAttribute("in_texcoord", 2, GL_FLOAT, sizeof(basic_vertex_t), OFFSET(basic_vertex_t, texcoords));
		m_shaderAttribs.a_color = m_pShader->InitAttribute("in_color", 4, GL_FLOAT, sizeof(basic_vertex_t), OFFSET(basic_vertex_t, color));

		if(!R_CheckShaderVertexAttribute(m_shaderAttribs.a_position, "position", m_pShader, pfnErrorPopup)
			|| !R_CheckShaderVertexAttribute(m_shaderAttribs.a_texcoord, "texcoord", m_pShader, pfnErrorPopup)
			|| !R_CheckShaderVertexAttribute(m_shaderAttribs.a_color, "color", m_pShader, pfnErrorPopup))
			return false;

		m_shaderAttribs.d_texture = m_pShader->GetDeterminatorIndex("texture");
		m_shaderAttribs.d_rectangle = m_pShader->GetDeterminatorIndex("rectangle");
		m_shaderAttribs.d_fog = m_pShader->GetDeterminatorIndex("fog");
		if(!R_CheckShaderDeterminator(m_shaderAttribs.d_texture, "texture", m_pShader, pfnErrorPopup)
			|| !R_CheckShaderDeterminator(m_shaderAttribs.d_rectangle, "rectangle", m_pShader, pfnErrorPopup)
			|| !R_CheckShaderDeterminator(m_shaderAttribs.d_fog, "fog", m_pShader, pfnErrorPopup))
			return false;

		m_pShader->SetVBO(m_pVBO);
	}

	return true;
}

//=============================================
// @brief Clears out the class
//
//=============================================
void CBasicDraw::ClearGL( void )
{
	if(m_pVBO)
	{
		delete m_pVBO;
		m_pVBO = nullptr;
	}

	if(m_pShader)
	{
		delete m_pShader;
		m_pShader = nullptr;
	}
}

//=============================================
// @brief Sets up for rendering a primitive
//
// @param primitiveType OpenGL primitive type
//=============================================
void CBasicDraw::Begin( GLint primitiveType )
{
	// Reset this
	m_numVertexes = 0;
	// Reset this also
	m_brightness = 1.0;

	// Set primitive type
	m_primitiveType = primitiveType;
}

//=============================================
// @brief Renders with the supplied vertex data
//
//=============================================
void CBasicDraw::End( bool clearData )
{
	if(!m_numVertexes)
		return;

	// Send data to opengl
	m_pVBO->VBOSubBufferData(0, &m_vertexesArray[0], sizeof(basic_vertex_t)*m_numVertexes);

	// Render the primitives
	glDrawArrays(m_primitiveType, 0, m_numVertexes);

	if(clearData)
		m_numVertexes = 0;
}

//=============================================
// @brief Enables texture use in the shader
//
//=============================================
bool CBasicDraw::EnableTexture( void )
{
	return m_pShader->SetDeterminator(m_shaderAttribs.d_texture, true);
}

//=============================================
// @brief Disables texture use in the shader
//
//=============================================
bool CBasicDraw::DisableTexture( void )
{
	return m_pShader->SetDeterminator(m_shaderAttribs.d_texture, false);
}

//=============================================
// @brief Disables rectangle texture use in the shader
//
//=============================================
bool CBasicDraw::EnableRectangleTexture( void )
{
	return m_pShader->SetDeterminator(m_shaderAttribs.d_rectangle, true);
}

//=============================================
// @brief Enables rectangle texture use in the shader
//
//=============================================
bool CBasicDraw::DisableRectangleTexture( void )
{
	return m_pShader->SetDeterminator(m_shaderAttribs.d_rectangle, false);
}

//=============================================
// @brief Binds VBO and shaders, etc
//
//=============================================
bool CBasicDraw::Enable( void )
{
	if(!m_pVBO)
		return false;

	m_pVBO->Bind();
	if(!m_pShader->EnableShader())
		return false;

	m_pShader->EnableAttribute(m_shaderAttribs.a_position);
	m_pShader->EnableAttribute(m_shaderAttribs.a_texcoord);
	m_pShader->EnableAttribute(m_shaderAttribs.a_color);

	m_pShader->SetUniform1i(m_shaderAttribs.u_texture, 0);
	m_pShader->SetUniform1i(m_shaderAttribs.u_rectangle, 0);

	// Reset to default value
	SetColorMultiplier(1.0);
	// Reset fog
	DisableFog();

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	m_isActive = true;
	return true;
}

//=============================================
// @brief Unbinds the VBO and the shader
//
//=============================================
void CBasicDraw::Disable( void )
{
	if(m_pShader)
		m_pShader->DisableShader();

	if(m_pVBO)
		m_pVBO->UnBind();

	m_isActive = false;
}

//=============================================
// @brief Sets the projection matrix in the shader
//
// @param pMatrix Pointer to array of matrix values
//=============================================
void CBasicDraw::SetProjection( const Float* pMatrix )
{
	m_pShader->SetUniformMatrix4fv(m_shaderAttribs.u_projection, pMatrix);
}

//=============================================
// @brief Sets the modelview matrix in the shader
//
// @param pMatrix Pointer to array of matrix values
//=============================================
void CBasicDraw::SetModelview( const Float* pMatrix )
{
	m_pShader->SetUniformMatrix4fv(m_shaderAttribs.u_modelview, pMatrix);
}

//=============================================
// @brief Sets the color value for a vertex
//
// @param r Red value in 0 - 1 range
// @param g Green value in 0 - 1 range
// @param b Blue value in 0 - 1 range
// @param a Alpha value in 0 - 1 range
//=============================================
void CBasicDraw::Color4f( Float r, Float g, Float b, Float a )
{
	m_vertexColor[0] = r;
	m_vertexColor[1] = g;
	m_vertexColor[2] = b;
	m_vertexColor[3] = a;
}

//=============================================
// @brief Sets the color value for a vertex
//
// @param pfc Pointer to color values
//=============================================
void CBasicDraw::Color4fv( const Float* pfc )
{
	for(Uint32 i = 0; i < 4; i++)
		m_vertexColor[i] = pfc[i];
}

//=============================================
// @brief Sets the brightness value
//
// @param brightness Brightness from 0-1
//=============================================
void CBasicDraw::Brightness1f( Float brightness )
{
	m_brightness = brightness;
}

//=============================================
// @brief Sets the color multiplier value
//
// @param brightness Brightness from 0-1
//=============================================
void CBasicDraw::SetColorMultiplier( Float multiplier )
{
	m_pShader->SetUniform1f(m_shaderAttribs.u_multiplier, multiplier);
}

//=============================================
// @brief Sets the color value for a vertex
//
// @param u U coordinate
// @param v V coordinate
//=============================================
void CBasicDraw::TexCoord2f( Float u, Float v )
{
	basic_vertex_t* pvertex = &m_vertexesArray[m_numVertexes];

	pvertex->texcoords[0] = u;
	pvertex->texcoords[1] = v;
}

//=============================================
// @brief Sets the color value for a vertex
//
// @param pfc Pointer to texcoords
//=============================================
void CBasicDraw::TexCoord2fv( const Float* ptc )
{
	basic_vertex_t* pvertex = &m_vertexesArray[m_numVertexes];

	pvertex->texcoords[0] = ptc[0];
	pvertex->texcoords[1] = ptc[1];
}

//=============================================
// @brief Sets the position of a vertex, and moves onto the next one
//
// @param x Coordinate on x axis
// @param y Coordinate on y axis
// @param z Coordinate on z axis
//=============================================
void CBasicDraw::Vertex3f( Float x, Float y, Float z )
{
	Vector vOrigin(x, y, z);
	Vertex3fv(vOrigin);
}

//=============================================
// @brief Sets the position of a vertex, and moves onto the next one
//
// @param pfv Pointer to coordinates
//=============================================
void CBasicDraw::Vertex3fv( const Float* pfv )
{
	basic_vertex_t* pvertex = &m_vertexesArray[m_numVertexes];
	m_numVertexes++;

	for(Uint32 i = 0; i < 4; i++)
		pvertex->color[i] = m_vertexColor[i] * m_brightness;

	Math::VectorCopy(pfv, pvertex->origin);
	pvertex->origin[3] = 1.0;

	if(m_numVertexes == m_vertexesArray.size())
	{
		// Append the vertex array
		m_vertexesArray.resize(m_vertexesArray.size()+BASICDRAW_CACHE_SIZE);
		// Resize VBO also
		m_pVBO->Append(&m_vertexesArray[0], sizeof(basic_vertex_t)*m_vertexesArray.size(), nullptr, 0);
	}
}

//=============================================
// @brief Enables fogging
//
// @return Error string pointer
//=============================================
bool CBasicDraw::EnableFog( void )
{
	return m_pShader->SetDeterminator(m_shaderAttribs.d_fog, true);
}

//=============================================
// @brief Sets fog parameters
//
//=============================================
void CBasicDraw::SetFogParams( const Vector& fogcolor, Float startdist, Float enddist )
{
	m_pShader->SetUniform3f(m_shaderAttribs.u_fogcolor, fogcolor[0], fogcolor[1], fogcolor[2]);
	m_pShader->SetUniform2f(m_shaderAttribs.u_fogparams, enddist, 1.0f/((Float)enddist-(Float)startdist));
}

//=============================================
// @brief Disables fogging
//
// @return false if error occurred, true otherwise
//=============================================
bool CBasicDraw::DisableFog( void )
{
	return m_pShader->SetDeterminator(m_shaderAttribs.d_fog, false);
}

//=============================================
// @brief Returns the shader's error message
//
// @return Error string pointer
//=============================================
const Char* CBasicDraw::GetShaderError( void ) const
{
	if(!m_pShader)
		return "";
	else
		return m_pShader->GetError();
}

//=============================================
// @brief Tells if the shader has an error
//
// @return TRUE if shader has an error, FALSE otherwise
//=============================================
bool CBasicDraw::HasError( void ) const
{
	if(!m_pShader)
		return false;
	else
		return m_pShader->HasError();
}

// 
//=============================================
// @brief Validates current shader setup
//
// @return TRUE if shader has an error, FALSE otherwise
//=============================================
void CBasicDraw::ValidateShaderSetup( void (*pfnConPrintfFnPtr)( const Char *fmt, ... ) )
{
	if(!pfnConPrintfFnPtr)
		return;

	m_pShader->ValidateProgram(pfnConPrintfFnPtr);
}

//=============================================
// @brief Creates an instance of this class
//
// @return Created instance
//=============================================
CBasicDraw* CBasicDraw::CreateInstance( void )
{
	if(!g_pInstance)
		g_pInstance = new CBasicDraw;

	return g_pInstance;
}

//=============================================
// @brief Returns the current instance of this class
//
// @return Current instance, or nullptr
//=============================================
CBasicDraw* CBasicDraw::GetInstance( void )
{
	return g_pInstance;
}

//=============================================
// @brief Deletes the current instance of this class
//
//=============================================
void CBasicDraw::DeleteInstance( void )
{
	if(!g_pInstance)
		return;

	delete g_pInstance;
	g_pInstance = nullptr;
}