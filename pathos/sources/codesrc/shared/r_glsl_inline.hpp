/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef R_GLSL_INLINE_H
#define R_GLSL_INLINE_H

#include "r_glextf.h"

//=============================================
// @brief Sets the values of the uniform
//
// @param index Index of the uniform
// @param matrix Pointer to the uniform values
// @param transpose Tells if the values should be transposed
//=============================================
inline void CGLSLShader :: SetUniformMatrix4fv( Int32 index, const Float *matrix, bool transpose )
{
	if(index == PROPERTY_UNAVAILABLE)
		return;

	assert(index >= 0 && index < m_uniformsArray.size());

	glsl_uniform_t* puniform = &m_uniformsArray[index];

	if(transpose)
	{
		puniform->flvalue[0][0] = matrix[0]; puniform->flvalue[0][1] = matrix[4]; puniform->flvalue[0][2] = matrix[8]; puniform->flvalue[0][3] = matrix[12];
		puniform->flvalue[1][0] = matrix[1]; puniform->flvalue[1][1] = matrix[5]; puniform->flvalue[1][2] = matrix[9]; puniform->flvalue[1][3] = matrix[13];
		puniform->flvalue[2][0] = matrix[2]; puniform->flvalue[2][1] = matrix[6]; puniform->flvalue[2][2] = matrix[10]; puniform->flvalue[2][3] = matrix[14];
		puniform->flvalue[3][0] = matrix[3]; puniform->flvalue[3][1] = matrix[7]; puniform->flvalue[3][2] = matrix[11]; puniform->flvalue[3][3] = matrix[15];
	}
	else
	{
		puniform->flvalue[0][0] = matrix[0]; puniform->flvalue[0][1] = matrix[1]; puniform->flvalue[0][2] = matrix[2]; puniform->flvalue[0][3] = matrix[3];
		puniform->flvalue[1][0] = matrix[4]; puniform->flvalue[1][1] = matrix[5]; puniform->flvalue[1][2] = matrix[6]; puniform->flvalue[1][3] = matrix[7];
		puniform->flvalue[2][0] = matrix[8]; puniform->flvalue[2][1] = matrix[9]; puniform->flvalue[2][2] = matrix[10]; puniform->flvalue[2][3] = matrix[11];
		puniform->flvalue[3][0] = matrix[12]; puniform->flvalue[3][1] = matrix[13]; puniform->flvalue[3][2] = matrix[14]; puniform->flvalue[3][3] = matrix[15];
	}

	if(puniform->indexes[m_shaderIndex] == PROPERTY_UNAVAILABLE)
		return;
	
	if(m_isActive)
		m_glExtF.glUniformMatrix4fv(puniform->indexes[m_shaderIndex], 1, GL_FALSE, reinterpret_cast<const GLfloat *>(puniform->flvalue)); 

	puniform->reload = false;
};

//=============================================
// @brief Sets the values of the uniform
//
// @param index Index of the uniform
// @param x Value at x position
//=============================================
inline void CGLSLShader :: SetUniform1i ( Int32 index, Int32 x )
{
	if(index == PROPERTY_UNAVAILABLE)
		return;

	assert(index >= 0 && index < m_uniformsArray.size());

	glsl_uniform_t* puniform = &m_uniformsArray[index];
	if(puniform->type != UNIFORM_NOSYNC)
	{
		if(puniform->flvalue[0][0] == x && puniform->sync && !puniform->reload)
			return;

		puniform->flvalue[0][0] = (Float)x;
	}

	if(puniform->indexes[m_shaderIndex] == PROPERTY_UNAVAILABLE)
		return;

	if(m_isActive)
		m_glExtF.glUniform1i(puniform->indexes[m_shaderIndex], x);

	puniform->reload = false;
};

//=============================================
// @brief Sets the values of the uniform
//
// @param index Index of the uniform
// @param x Value at x position
//=============================================
inline void CGLSLShader :: SetUniform1f ( Int32 index, Float x )
{
	if(index == PROPERTY_UNAVAILABLE)
		return;

	assert(index >= 0 && index < m_uniformsArray.size());

	glsl_uniform_t* puniform = &m_uniformsArray[index];
	if(puniform->type != UNIFORM_NOSYNC)
	{
		if(puniform->flvalue[0][0] == x && puniform->sync && !puniform->reload)
			return;

		puniform->flvalue[0][0] = x;
	}

	if(puniform->indexes[m_shaderIndex] == PROPERTY_UNAVAILABLE)
		return;

	if(m_isActive)
		m_glExtF.glUniform1f(puniform->indexes[m_shaderIndex], x);

	puniform->reload = false;
};

//=============================================
// @brief Sets the values of the uniform
//
// @param index Index of the uniform
// @param x Value at x position
// @param y Value at y position
//=============================================
inline void CGLSLShader :: SetUniform2f ( Int32 index, Float x, Float y )
{
	if(index == PROPERTY_UNAVAILABLE)
		return;

	assert(index >= 0 && index < m_uniformsArray.size());

	glsl_uniform_t* puniform = &m_uniformsArray[index];
	if(puniform->type != UNIFORM_NOSYNC)
	{
		if(puniform->flvalue[0][0] == x
			&& puniform->flvalue[0][1] == y
			&& puniform->sync && !puniform->reload)
			return;

		puniform->flvalue[0][0] = x; 
		puniform->flvalue[0][1] = y;
	}

	if(puniform->indexes[m_shaderIndex] == PROPERTY_UNAVAILABLE)
		return;

	if(m_isActive)
		m_glExtF.glUniform2f(puniform->indexes[m_shaderIndex], x, y);

	puniform->reload = false;
};

//=============================================
// @brief Sets the values of the uniform
//
// @param index Index of the uniform
// @param x Value at x position
// @param y Value at y position
// @param z Value at z position
//=============================================
inline void CGLSLShader :: SetUniform3f ( Int32 index, Float x, Float y, Float z )
{
	if(index == PROPERTY_UNAVAILABLE)
		return;

	assert(index >= 0 && index < m_uniformsArray.size());

	glsl_uniform_t* puniform = &m_uniformsArray[index];
	if(puniform->type != UNIFORM_NOSYNC)
	{
		if(puniform->flvalue[0][0] == x
			&& puniform->flvalue[0][1] == y
			&& puniform->flvalue[0][2] == z
			&& puniform->sync && !puniform->reload)
			return;

		puniform->flvalue[0][0] = x; 
		puniform->flvalue[0][1] = y; 
		puniform->flvalue[0][2] = z;
	}

	if(puniform->indexes[m_shaderIndex] == PROPERTY_UNAVAILABLE)
		return;

	if(m_isActive)
		m_glExtF.glUniform3f(puniform->indexes[m_shaderIndex], x, y, z);

	puniform->reload = false;
};

//=============================================
// @brief Sets the values of the uniform
//
// @param index Index of the uniform
// @param x Value at x position
// @param y Value at y position
// @param z Value at z position
// @param w Value at w position
//=============================================
inline void CGLSLShader :: SetUniform4f ( Int32 index, Float x, Float y, Float z, Float w )
{
	if(index == PROPERTY_UNAVAILABLE)
		return;

	assert(index >= 0 && index < m_uniformsArray.size());

	glsl_uniform_t* puniform = &m_uniformsArray[index];
	if(puniform->type != UNIFORM_NOSYNC)
	{
		if(puniform->flvalue[0][0] == x
			&& puniform->flvalue[0][1] == y
			&& puniform->flvalue[0][2] == z
			&& puniform->flvalue[0][3] == w
			&& puniform->sync && !puniform->reload)
			return;

		puniform->flvalue[0][0] = x; 
		puniform->flvalue[0][1] = y; 
		puniform->flvalue[0][2] = z; 
		puniform->flvalue[0][3] = w; 
	}

	if(puniform->indexes[m_shaderIndex] == PROPERTY_UNAVAILABLE)
		return;

	if(m_isActive)
		m_glExtF.glUniform4f(puniform->indexes[m_shaderIndex], x, y, z, w);

	puniform->reload = false;
};

//=============================================
// @brief Sets the values of the uniform for num elements
//
// @param index Index of the uniform
// @param v Pointer to the array of floats
// @param num Number of float4s to upload
//=============================================
inline void CGLSLShader :: SetUniform4fv ( Int32 index, const Float *v, Uint32 num )
{
	if(index == PROPERTY_UNAVAILABLE)
		return;

	assert(index >= 0 && index < m_uniformsArray.size());

	glsl_uniform_t* puniform = &m_uniformsArray[index];
	if(puniform->type != UNIFORM_NOSYNC)
	{
		if(puniform->flvalue[0][0] == v[0]
			&& puniform->flvalue[0][1] == v[1]
			&& puniform->flvalue[0][2] == v[2]
			&& puniform->flvalue[0][3] == v[3]
			&& puniform->sync && !puniform->reload)
			return;

		puniform->flvalue[0][0] = v[0]; 
		puniform->flvalue[0][1] = v[1]; 
		puniform->flvalue[0][2] = v[2]; 
		puniform->flvalue[0][3] = v[3]; 
	}

	if(puniform->indexes[m_shaderIndex] == PROPERTY_UNAVAILABLE)
		return;

	if(m_isActive)
		m_glExtF.glUniform4fv(puniform->indexes[m_shaderIndex], num, v);

	puniform->reload = false;
}

//=============================================
// @brief Enables syncing on a uniform
//
// @param index Index of the uniform
//=============================================
inline void CGLSLShader :: EnableSync ( Int32 uniform )
{
	if(uniform == PROPERTY_UNAVAILABLE)
		return;

	assert(uniform >= 0 && uniform < m_uniformsArray.size());

	m_uniformsArray[uniform].sync = true;
}

//=============================================
// @brief Disables syncing on a uniform
//
// @param index Index of the uniform
//=============================================
inline void CGLSLShader :: DisableSync ( Int32 uniform )
{
	if(uniform == PROPERTY_UNAVAILABLE)
		return;

	assert(uniform >= 0 && uniform < m_uniformsArray.size());

	m_uniformsArray[uniform].sync = false;
	m_uniformsArray[uniform].reload = true;
}
#endif //R_GLSL_INLINE_H