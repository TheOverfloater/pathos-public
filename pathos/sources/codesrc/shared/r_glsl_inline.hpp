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
#include "r_common.h"

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
	assert(puniform->elementcount == 1);

	R_SetMatrixData(matrix, &m_uniformMatrix[0][0], transpose);

	if(puniform->type != UNIFORM_NOSYNC)
	{
		if(!memcmp(m_uniformMatrix, &puniform->currentvalues[0], sizeof(Float)*puniform->stride))
			return;

		memcpy(&puniform->currentvalues[0], m_uniformMatrix, sizeof(Float)*puniform->stride);
	}

	if(puniform->indexes[m_shaderIndex] == PROPERTY_UNAVAILABLE)
		return;

	if(puniform->type != UNIFORM_NOSYNC)
	{
		if(m_isActive)
		{
			const Float* pvalues = &puniform->currentvalues[0];
			Float* ptargetvalues = &puniform->shadervalues[0] + puniform->stride * m_shaderIndex;

			if(memcmp(ptargetvalues, pvalues, sizeof(Float)*puniform->stride) != 0)
			{
				m_glExtF.glUniformMatrix4fv(puniform->indexes[m_shaderIndex], 1, GL_FALSE, pvalues); 	
				memcpy(ptargetvalues, pvalues, sizeof(Float)*puniform->stride);
			}
		}
	}
	else if(m_isActive)
	{
		m_glExtF.glUniformMatrix4fv(puniform->indexes[m_shaderIndex], 1, GL_FALSE, &m_uniformMatrix[0][0]);
	}
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
	assert(puniform->elementcount == 1);

	// SetUniform1i cannot have an elementcount > 1
	if (puniform->elementcount > 1)
		return;

	if(puniform->type != UNIFORM_NOSYNC)
	{
		assert(puniform->stride == 1);
		Float* pvalues = &puniform->currentvalues[0];
		if((*pvalues) == x && puniform->sync)
			return;

		(*pvalues) = static_cast<Float>(x);
	}

	if(puniform->indexes[m_shaderIndex] == PROPERTY_UNAVAILABLE)
		return;

	if(m_isActive)
	{
		if(puniform->type != UNIFORM_NOSYNC)
		{
			const Float* pvalues = &puniform->currentvalues[0];
			Float* ptargetvalues = &puniform->shadervalues[0] + puniform->stride * m_shaderIndex;
			if(memcmp(ptargetvalues, pvalues, sizeof(Float)*puniform->stride) != 0)
			{
				m_glExtF.glUniform1i(puniform->indexes[m_shaderIndex], x);
				memcpy(ptargetvalues, pvalues, sizeof(Float)*puniform->stride);
			}
		}
		else
		{
			m_glExtF.glUniform1i(puniform->indexes[m_shaderIndex], x);
		}
	}
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
	assert(puniform->elementcount == 1);

	// Functions that don't use pointers as parameters cannot be of elementcount > 1
	if (puniform->elementcount > 1)
		return;

	if(puniform->type != UNIFORM_NOSYNC)
	{
		assert(puniform->stride == 1);
		Float* pvalues = &puniform->currentvalues[0];
		if((*pvalues) == x && puniform->sync)
			return;

		(*pvalues) = x;
	}

	if(puniform->indexes[m_shaderIndex] == PROPERTY_UNAVAILABLE)
		return;

	if(m_isActive)
	{
		if(puniform->type != UNIFORM_NOSYNC)
		{
			const Float* pvalues = &puniform->currentvalues[0];
			Float* ptargetvalues = &puniform->shadervalues[0] + puniform->stride * m_shaderIndex;
			if(memcmp(ptargetvalues, pvalues, sizeof(Float)*puniform->stride) != 0)
			{
				m_glExtF.glUniform1f(puniform->indexes[m_shaderIndex], x);
				memcpy(ptargetvalues, pvalues, sizeof(Float)*puniform->stride);
			}
		}
		else
		{
			m_glExtF.glUniform1f(puniform->indexes[m_shaderIndex], x);
		}
	}
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
	assert(puniform->elementcount == 1);

	// Functions that don't use pointers as parameters cannot be of elementcount > 1
	if (puniform->elementcount > 1)
		return;

	if(puniform->type != UNIFORM_NOSYNC)
	{
		assert(puniform->stride == 2);
		Float* pvalues = &puniform->currentvalues[0];
		if(pvalues[0] == x && pvalues[1] == y
			&& puniform->sync)
			return;

		pvalues[0] = x; 
		pvalues[1] = y;
	}

	if(puniform->indexes[m_shaderIndex] == PROPERTY_UNAVAILABLE)
		return;

	if(m_isActive)
	{
		if(puniform->type != UNIFORM_NOSYNC)
		{
			const Float* pvalues = &puniform->currentvalues[0];
			Float* ptargetvalues = &puniform->shadervalues[0] + puniform->stride * m_shaderIndex;
			if(memcmp(ptargetvalues, pvalues, sizeof(Float)*puniform->stride) != 0)
			{
				m_glExtF.glUniform2f(puniform->indexes[m_shaderIndex], x, y);
				memcpy(ptargetvalues, pvalues, sizeof(Float)*puniform->stride);
			}
		}
		else
		{
			m_glExtF.glUniform2f(puniform->indexes[m_shaderIndex], x, y);
		}
	}
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
	assert(puniform->elementcount == 1);

	// Functions that don't use pointers as parameters cannot be of elementcount > 1
	if (puniform->elementcount > 1)
		return;

	if(puniform->type != UNIFORM_NOSYNC)
	{
		assert(puniform->stride == 3);
		Float* pvalues = &puniform->currentvalues[0];
		if(pvalues[0] == x && pvalues[1] == y && pvalues[2] == z
			&& puniform->sync)
			return;

		pvalues[0] = x; 
		pvalues[1] = y; 
		pvalues[2] = z;
	}

	if(puniform->indexes[m_shaderIndex] == PROPERTY_UNAVAILABLE)
		return;

	if(m_isActive)
	{
		if(puniform->type != UNIFORM_NOSYNC)
		{
			const Float* pvalues = &puniform->currentvalues[0];
			Float* ptargetvalues = &puniform->shadervalues[0] + puniform->stride * m_shaderIndex;
			if(memcmp(ptargetvalues, pvalues, sizeof(Float)*puniform->stride) != 0)
			{
				m_glExtF.glUniform3f(puniform->indexes[m_shaderIndex], x, y, z);
				memcpy(ptargetvalues, pvalues, sizeof(Float)*puniform->stride);
			}
		}
		else
		{
			m_glExtF.glUniform3f(puniform->indexes[m_shaderIndex], x, y, z);
		}
	}
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
	assert(puniform->elementcount == 1);

	// Functions that don't use pointers as parameters cannot be of elementcount > 1
	if (puniform->elementcount > 1)
		return;

	if(puniform->type != UNIFORM_NOSYNC)
	{
		assert(puniform->stride == 4);
		Float* pvalues = &puniform->currentvalues[0];
		if(pvalues[0] == x && pvalues[1] == y
			&& pvalues[2] == z && pvalues[3] == w
			&& puniform->sync)
			return;

		pvalues[0] = x; 
		pvalues[1] = y; 
		pvalues[2] = z; 
		pvalues[3] = w; 
	}

	if(puniform->indexes[m_shaderIndex] == PROPERTY_UNAVAILABLE)
		return;

	if(m_isActive)
	{
		if(puniform->type != UNIFORM_NOSYNC)
		{
			const Float* pvalues = &puniform->currentvalues[0];
			Float* ptargetvalues = &puniform->shadervalues[0] + puniform->stride * m_shaderIndex;
			if(memcmp(ptargetvalues, pvalues, sizeof(Float)*puniform->stride) != 0)
			{
				m_glExtF.glUniform4f(puniform->indexes[m_shaderIndex], x, y, z, w);
				memcpy(ptargetvalues, pvalues, sizeof(Float)*puniform->stride);
			}
		}
		else
		{
			m_glExtF.glUniform4f(puniform->indexes[m_shaderIndex], x, y, z, w);
		}
	}
};

//=============================================
// @brief Sets the values of the uniform for num elements
//
// @param index Index of the uniform
// @param v Pointer to the array of floats
// @param num Number of float4s to upload
//=============================================
inline void CGLSLShader::SetUniform4fv(Int32 index, const Float* v, Uint32 num)
{
	if (index == PROPERTY_UNAVAILABLE)
		return;

	assert(index >= 0 && index < m_uniformsArray.size());
	glsl_uniform_t* puniform = &m_uniformsArray[index];

	if (num > puniform->elementcount)
		return;

	bool update = false;
	if (puniform->type != UNIFORM_NOSYNC)
	{
		assert(puniform->stride == 4);

		Uint32 i = 0;
		for (; i < puniform->elementcount; i++)
		{
			Float* pvalues = &puniform->currentvalues[puniform->stride * i];
			const Float* pinput = &v[puniform->stride * i];

			if (pvalues[0] != pinput[0] || pvalues[1] != pinput[1]
				|| pvalues[2] != pinput[2] || pvalues[3] != pinput[3])
				break;
		}

		if (i != puniform->elementcount)
		{
			memcpy(&puniform->currentvalues[0], v, sizeof(Float) * puniform->stride * puniform->elementcount);
			update = true;
		}
	}

	if (!update && puniform->sync)
		return;

	if (puniform->indexes[m_shaderIndex] == PROPERTY_UNAVAILABLE)
		return;

	if (m_isActive)
	{
		if (puniform->type != UNIFORM_NOSYNC)
		{
			const Float* pvalues = &puniform->currentvalues[0];
			Float* ptargetvalues = &puniform->shadervalues[0] + puniform->stride * puniform->elementcount * m_shaderIndex;
			if (memcmp(ptargetvalues, pvalues, sizeof(Float) * puniform->stride * puniform->elementcount) != 0)
			{
				m_glExtF.glUniform4fv(puniform->indexes[m_shaderIndex], num, v);
				memcpy(ptargetvalues, pvalues, sizeof(Float) * puniform->stride * puniform->elementcount);
			}
		}
		else
		{
			m_glExtF.glUniform4fv(puniform->indexes[m_shaderIndex], num, v);
		}
	}
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
	
	// Re-sync this uniform
	SyncUniform(m_uniformsArray[uniform]);
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
}

//=============================================
// @brief Sets the data for the UBO
//
// @param index Index of the uniform
//=============================================
inline void CGLSLShader :: SetUniformBufferObjectData( Int32 index, void* pBufferData, Uint32 dataSize )
{
	assert(index >= 0 && index < m_uniformBufferObjectsArray.size());
	const glsl_ubo_t& ubo = m_uniformBufferObjectsArray[index];

	m_glExtF.glBindBuffer(GL_UNIFORM_BUFFER, ubo.buffer_id);
	m_glExtF.glBufferSubData(GL_UNIFORM_BUFFER, 0, dataSize, pBufferData);
	m_glExtF.glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
#endif //R_GLSL_INLINE_H