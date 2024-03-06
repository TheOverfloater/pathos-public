/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.

===============================================
*/

#ifndef R_VBO_H
#define R_VBO_H

// External functions class
class CGLExtF;

// For setting the offset in the datatype
#define OFFSET(type, variable) ((const void*)&(((type*)nullptr)->variable))
// For calls to glDrawElements
#define BUFFER_OFFSET(i) ((Uint32 *)nullptr + (i))

// Maximum GLSL attributes
#define MAX_ATTRIBS 32

// <attrib_t>
struct attrib_t
{
	attrib_t():
		ptr(nullptr),
		active(false),
		size(0),
		stride(0)
	{
	}

	const void *ptr;
	bool active;

	Uint32 size;
	Uint32 stride;
};

//====================
// CVBO
// 
//====================
class CVBO
{
public:
	CVBO( const CGLExtF& glExtF, const void *pvbodata, Uint32 ivbodatasize, const void *pibodata, Uint32 iibodatasize, bool keepcache = false );
	CVBO ( const CGLExtF& glExtF, bool bvbo, bool bibo );
	~CVBO( void );
	void Clear( void );

	void SetAttribPointer( Int32 index, Uint32 size, Uint32 type, Uint32 stride, const void *pointer );
	void DisableAttribPointer( Int32 index );

	void VBOSubBufferData( Uint32 offset, const void *pdata, Uint32 size );
	void IBOSubBufferData( Uint32 offset, const void *pdata, Uint32 size );

	void Bind( void );
	void UnBind( void );

	void ClearGL( void );
	void RebindGL( void );

	Uint32 GetVBOSize( void ) const { return m_iVBOSize; }
	Uint32 GetIBOSize( void ) const { return m_iIBOSize; }

	bool Append( const void *pvbodata, Uint32 ivbodatasize, const void *pibodata, Uint32 iibodatasize );
	void DeleteCaches( void );

	bool IsValid( void ) const { return m_isValid; }

public:
	GLuint GetVBOIndex( void ) const { return m_uiVBOIndex; }
	GLuint GetIBOIndex( void ) const { return m_uiIBOIndex; }
#ifndef NO_VAO
	GLuint GetVAOIndex( void ) const { return m_uiVAOIndex; }
#endif
	const void* GetVBOData( void ) const { return m_pVBOData; }
	const Uint32 GetVBODataSize( void ) const { return m_iVBOSize; }

	const void* GetIBOData( void ) const { return m_pIBOData; }
	const Uint32 GetIBODataSize( void ) const { return m_iIBOSize; }

private:
	GLuint	m_uiVBOIndex;
	GLuint	m_uiIBOIndex;
#ifndef NO_VAO
	GLuint	m_uiVAOIndex;
#endif
	bool	m_bActive;
	bool	m_bCache;
	bool	m_isValid;

	void *m_pVBOData;
	Uint32 m_iVBOSize;

	void *m_pIBOData;
	Uint32 m_iIBOSize;

	attrib_t m_pAttribPointers[MAX_ATTRIBS];

	// External functions class
	const CGLExtF& m_glExtF;
};
#endif // R_VBO_H