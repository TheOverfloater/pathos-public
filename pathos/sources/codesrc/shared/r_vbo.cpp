/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.

===============================================
*/

#include "includes.h"
#include "r_vbo.h"
#include "common.h"
#include "r_glextf.h"

//=============================================
// @brief Constructor
//
// @param pvbodata Pointer to VBO data
// @param ivbodatasize Size of the VBO data in bytes
// @param pibodata Pointer to IBO data
// @param iibodatasize Size of the IBO data in bytes
// @param keepcache Determines whether data is freed after binding
//=============================================
CVBO::CVBO ( const CGLExtF& glExtF, const void *pvbodata, Uint32 ivbodatasize, const void *pibodata, Uint32 iibodatasize, bool keepcache ) :
	m_uiVBOIndex(0),
	m_uiIBOIndex(0),
#ifndef NO_VAO
	m_uiVAOIndex(0),
#endif
	m_bActive(false),
	m_bCache(keepcache),
	m_isValid(true),
	m_pVBOData(nullptr),
	m_iVBOSize(0),
	m_pIBOData(nullptr),
	m_iIBOSize(0),
	m_glExtF(glExtF)
{
	Clear();

	if(!pvbodata && !ivbodatasize && !pibodata && !iibodatasize)
	{
		m_isValid = false;
		return;
	}

#ifndef NO_VAO
	// Allocate the VAO
	m_glExtF.glGenVertexArrays(1, &m_uiVAOIndex);
	m_glExtF.glBindVertexArray(m_uiVAOIndex);
#endif

	if(pvbodata && ivbodatasize)
	{
		m_glExtF.glGenBuffers(1, &m_uiVBOIndex);
		m_glExtF.glBindBuffer(GL_ARRAY_BUFFER, m_uiVBOIndex);
		m_glExtF.glBufferData(GL_ARRAY_BUFFER, ivbodatasize, pvbodata, GL_DYNAMIC_DRAW);

		if(keepcache)
		{
			m_pVBOData = new byte[ivbodatasize];
			memcpy(m_pVBOData, pvbodata, sizeof(byte)*ivbodatasize);
			m_iVBOSize = ivbodatasize;
		}

		m_glExtF.glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	if(pibodata && iibodatasize)
	{
		m_glExtF.glGenBuffers(1, &m_uiIBOIndex);
		m_glExtF.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uiIBOIndex);
		m_glExtF.glBufferData(GL_ELEMENT_ARRAY_BUFFER, iibodatasize, pibodata, GL_DYNAMIC_DRAW);

		if(keepcache)
		{
			m_pIBOData = new byte[iibodatasize];
			memcpy(m_pIBOData, pibodata, sizeof(byte)*iibodatasize);
			m_iIBOSize = iibodatasize;
		}

		m_glExtF.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

#ifndef NO_VAO
	// Unbind the VAO
	m_glExtF.glBindVertexArray(0);
#endif
}

//=============================================
// @brief Constructor
//
// @param bvbo Determines if we expect VBO data
// @param bibo Determines if we expect IBO data
//=============================================
CVBO::CVBO ( const CGLExtF& glExtF, bool bvbo, bool bibo ) :
	m_uiVBOIndex(0),
	m_uiIBOIndex(0),
#ifndef NO_VAO
	m_uiVAOIndex(0),
#endif
	m_bActive(false),
	m_bCache(false),
	m_isValid(true),
	m_pVBOData(nullptr),
	m_iVBOSize(0),
	m_pIBOData(nullptr),
	m_iIBOSize(0),
	m_glExtF(glExtF)
{
	Clear();

#ifndef NO_VAO
	// Allocate the VAO
	m_glExtF.glGenVertexArrays(1, &m_uiVAOIndex);
	m_glExtF.glBindVertexArray(m_uiVAOIndex);
#endif

	if(bvbo)
	{
		// just generate an empty buffer
		m_glExtF.glGenBuffers(1, &m_uiVBOIndex);
	}

	if(bibo)
	{
		// just generate an empty buffer
		m_glExtF.glGenBuffers(1, &m_uiIBOIndex);
	}

#ifndef NO_VAO
	// Unbind the VAO
	m_glExtF.glBindVertexArray(0);
#endif
	m_bCache = true;
}

//=============================================
// @brief Destructor
//
//=============================================
CVBO::~CVBO ( void )
{
	ClearGL();

	if(m_pIBOData)
		delete[] m_pIBOData;

	if(m_pVBOData)
		delete[] m_pVBOData;
}

//=============================================
// @brief Unbinds the buffers
//
//=============================================
void CVBO::ClearGL( void )
{
	if(m_bActive)
	{
		if(m_uiVBOIndex)
			m_glExtF.glBindBuffer(GL_ARRAY_BUFFER, 0);

		if(m_uiIBOIndex)
			m_glExtF.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

#ifndef NO_VAO
		if(m_uiVAOIndex)
			m_glExtF.glBindVertexArray(0);
#endif
	}

	if(m_uiVBOIndex)
	{
		m_glExtF.glDeleteBuffers(1, &m_uiVBOIndex);
		m_uiVBOIndex = 0;
	}

	if(m_uiIBOIndex)
	{
		m_glExtF.glDeleteBuffers(1, &m_uiIBOIndex);
		m_uiIBOIndex = 0;
	}

#ifndef NO_VAO
	if(m_uiVAOIndex)
	{
		m_glExtF.glDeleteVertexArrays(1, &m_uiVAOIndex);
		m_uiVAOIndex = 0;
	}
#endif
	Clear();
}

//=============================================
// @brief Unbinds the buffers
//
//=============================================
void CVBO::RebindGL( void )
{
	assert(m_bCache);

#ifndef NO_VAO
	// Allocate the VAO
	m_glExtF.glGenVertexArrays(1, &m_uiVAOIndex);
	m_glExtF.glBindVertexArray(m_uiVAOIndex);
#endif

	if(m_pVBOData && m_iVBOSize)
	{
		m_glExtF.glGenBuffers(1, &m_uiVBOIndex);
		m_glExtF.glBindBuffer(GL_ARRAY_BUFFER, m_uiVBOIndex);
		m_glExtF.glBufferData(GL_ARRAY_BUFFER, m_iVBOSize, m_pVBOData, GL_DYNAMIC_DRAW);
		m_glExtF.glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	if(m_pIBOData && m_iIBOSize)
	{
		m_glExtF.glGenBuffers(1, &m_uiIBOIndex);
		m_glExtF.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uiIBOIndex);
		m_glExtF.glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_iIBOSize, m_pIBOData, GL_DYNAMIC_DRAW);
		m_glExtF.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

#ifndef NO_VAO
	// Unbind the VAO
	m_glExtF.glBindVertexArray(0);
#endif
}

//=============================================
// @brief Clears the CVBO class
//
//=============================================
void CVBO::Clear ( void  )
{
	for(Uint32 i = 0; i < MAX_ATTRIBS; i++)
		m_pAttribPointers[i] = attrib_t();
}

//=============================================
// @brief Appends data to the existing buffers
//
// @param pvbodata Pointer to VBO data
// @param ivbodatasize Size of the VBO data to append
// @param pibodata Pointer to IBO data
// @param iibodatasize Size of the IBO data to append
//=============================================
bool CVBO::Append ( const void *pvbodata, Uint32 ivbodatasize, const void *pibodata, Uint32 iibodatasize )
{
	if(!m_bCache)
		return false;

#ifndef NO_VAO
	if(!m_bActive)
		m_glExtF.glBindVertexArray(m_uiVAOIndex);
#endif

	if(pvbodata && ivbodatasize)
	{
		if(m_pVBOData)
		{
			byte *pnew = new byte[m_iVBOSize+ivbodatasize];
			memcpy(pnew, m_pVBOData, sizeof(byte)*m_iVBOSize);
			memcpy((pnew+m_iVBOSize), pvbodata, sizeof(byte)*ivbodatasize);

			delete[] m_pVBOData;
			m_pVBOData = pnew; 
			m_iVBOSize += ivbodatasize;
		}
		else
		{
			m_pVBOData = new byte[ivbodatasize];
			memcpy(m_pVBOData, pvbodata, sizeof(byte)*ivbodatasize);
			m_iVBOSize = ivbodatasize;
		}

		if(!m_bActive)
			m_glExtF.glBindBuffer(GL_ARRAY_BUFFER, m_uiVBOIndex);

		m_glExtF.glBufferData(GL_ARRAY_BUFFER, m_iVBOSize, m_pVBOData, GL_DYNAMIC_DRAW);

		if(!m_bActive)
			m_glExtF.glBindBuffer(GL_ARRAY_BUFFER, 0);
	}	

	if(pibodata && iibodatasize)
	{
		if(m_pIBOData)
		{
			byte *pnew = new byte[m_iIBOSize+iibodatasize];
			memcpy(pnew, m_pIBOData, sizeof(byte)*m_iIBOSize);
			memcpy(pnew+m_iIBOSize, pibodata, sizeof(byte)*iibodatasize);

			delete[] m_pIBOData;
			m_pIBOData = pnew;
			m_iIBOSize += iibodatasize;
		}
		else
		{
			m_pIBOData = new byte[iibodatasize];
			memcpy(m_pIBOData, pibodata, sizeof(byte)*iibodatasize);
			m_iIBOSize = iibodatasize;
		}

		if(!m_bActive)
			m_glExtF.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uiIBOIndex);

		m_glExtF.glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_iIBOSize, m_pIBOData, GL_DYNAMIC_DRAW);

		if(!m_bActive)
			m_glExtF.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	if(!m_bActive)
		m_glExtF.glBindVertexArray(0);

	return true;
}

//=============================================
// @brief Deletes the caches from the RAM
//
//=============================================
void CVBO::DeleteCaches ( void )
{
	if(m_pVBOData)
	{
		delete[] m_pVBOData;
		m_pVBOData = nullptr;
		m_iVBOSize = 0;
	}

	if(m_pIBOData)
	{
		delete[] m_pIBOData;
		m_pIBOData = nullptr;
		m_iIBOSize = 0;
	}

	m_bCache = false;
}

//=============================================
// @brief Sets an attribute pointer's data
//
// @param index Index of vertex attribute
// @param size Size of the attribute in bytes
// @param type OpenGL datatype of the attribute
// @param stride Size of the vertex VBO datatype
// @param pointer Offset in bytes to the attribute in the datatype
//=============================================
void CVBO::SetAttribPointer ( Int32 index, Uint32 size, Uint32 type, Uint32 stride, const void *pointer )
{
	assert(index >= 0 && index < MAX_ATTRIBS);
	attrib_t *pattrib = &m_pAttribPointers[index];
	if(pattrib->ptr == pointer && pattrib->size == size && pattrib->stride == stride && pattrib->active)
		return;

	m_glExtF.glEnableVertexAttribArray(index); 
	m_glExtF.glVertexAttribPointer(index, size, type, GL_FALSE, stride, pointer);

	// Set our tracker
	pattrib->size = size;
	pattrib->stride = stride;
	pattrib->ptr = pointer;
	pattrib->active = true;
}

//=============================================
// @brief Disables and clears a vertex attribute
//
// @param index Index of vertex attribute
//=============================================
void CVBO::DisableAttribPointer ( Int32 index )
{
	if(index == -1)
		return;

	assert(index >= 0 && index < MAX_ATTRIBS);
	attrib_t *pattrib = &m_pAttribPointers[index];

	if(!pattrib->ptr && !pattrib->size && !pattrib->active)
		return;

	m_glExtF.glDisableVertexAttribArray(index);

	pattrib->size = pattrib->stride = 0;
	pattrib->ptr = nullptr;
	pattrib->active = false;
}

//=============================================
// @brief Replaces data at a given location in the VBO
//
// @param offset Offset into the VBO in bytes
// @param pdata Pointer to VBO data
// @param size Size of the data in bytes
//=============================================
void CVBO :: VBOSubBufferData ( Uint32 offset, const void *pdata, Uint32 size )
{
	if(!m_bActive)
	{
#ifndef NO_VAO
		m_glExtF.glBindVertexArray(m_uiVAOIndex);
#endif
		m_glExtF.glBindBuffer(GL_ARRAY_BUFFER, m_uiVBOIndex);
	}

	m_glExtF.glBufferSubData(GL_ARRAY_BUFFER, offset, size, pdata);

	if(m_pVBOData)
	{
		byte* pdest = reinterpret_cast<byte*>(m_pVBOData) + offset;
		memcpy(pdest, pdata, sizeof(byte)*size);
	}
	if(!m_bActive)
	{
		m_glExtF.glBindBuffer(GL_ARRAY_BUFFER, 0);
		m_glExtF.glBindVertexArray(0);
	}
}

//=============================================
// @brief Replaces data at a given location in the IBO
//
// @param offset Offset into the IBO in bytes
// @param pdata Pointer to IBO data
// @param size Size of the data in bytes
//=============================================
void CVBO::IBOSubBufferData ( Uint32 offset, const void *pdata, Uint32 size )
{
	if(!m_bActive)
	{
#ifndef NO_VAO
		m_glExtF.glBindVertexArray(m_uiVAOIndex);
#endif
		m_glExtF.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uiIBOIndex);
	}

	m_glExtF.glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, size, pdata);

	if(m_pVBOData)
	{
		byte* pdest = reinterpret_cast<byte*>(m_pIBOData) + offset;
		memcpy(pdest, pdata, sizeof(byte)*size);
	}

	if(!m_bActive)
	{
		m_glExtF.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		m_glExtF.glBindVertexArray(0);
	}
}

//=============================================
// @brief Binds the buffers for rendering
//
//=============================================
void CVBO::Bind ( void )
{
	if(m_bActive)
		return;
#ifndef NO_VAO
	// Bind the VAO
	m_glExtF.glBindVertexArray(m_uiVAOIndex);
#endif
	if(m_uiVBOIndex)
		m_glExtF.glBindBuffer(GL_ARRAY_BUFFER, m_uiVBOIndex);

	if(m_uiIBOIndex)
		m_glExtF.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uiIBOIndex);

	m_bActive = true;
}

//=============================================
// @brief Unbinds the buffers
//
//=============================================
void CVBO::UnBind ( void )
{
	if(!m_bActive)
		return;

	if(m_uiVBOIndex)
		m_glExtF.glBindBuffer(GL_ARRAY_BUFFER, 0);

	if(m_uiIBOIndex)
		m_glExtF.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

#ifndef NO_VAO
	// Unbind the VAO
	m_glExtF.glBindVertexArray(0);
#endif
	m_bActive = false;
}
