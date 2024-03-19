/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef GL_IMPORTS_H
#define GL_IMPORTS_H

#include <SDL_opengl_glext.h>

/*
====================
CGLExtF

====================
*/
class CGLExtF
{
public:
	CGLExtF ( void );
	~CGLExtF( void );

	// Retrieves all function pointers
	bool GetFunctions( void );
	// Retrieves a single function's pointer
	PROC GetFunction( const Char* pstrName, bool setFailureCode = true );

	// Returns the error string
	const Char* GetError( void ) const { return m_errorString.c_str(); }

public:
	// OpenGL function declarations
	PFNGLACTIVETEXTUREPROC glActiveTexture;
	PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTexture;

	PFNGLBINDBUFFERPROC glBindBuffer;
	PFNGLGENBUFFERSPROC glGenBuffers;
	PFNGLBUFFERDATAPROC glBufferData;
	PFNGLDELETEBUFFERSPROC glDeleteBuffers;
	PFNGLBUFFERSUBDATAPROC glBufferSubData;

	PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
	PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
	PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;

	PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
	PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
	PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;

	PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
	PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer;
	PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage;
	PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;
	PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
	PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
	PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
	PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
	PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers;
	PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
	PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC glGetFramebufferAttachmentParameteriv;
	PFNGLGETRENDERBUFFERPARAMETERIVPROC glGetRenderbufferParameteriv;
	PFNGLBLITFRAMEBUFFERPROC glBlitFramebuffer;

	PFNGLLINKPROGRAMPROC glLinkProgram;
	PFNGLSHADERSOURCEPROC glShaderSource;
	PFNGLUSEPROGRAMPROC glUseProgram;
	PFNGLUNIFORM1FPROC glUniform1f;
	PFNGLUNIFORM2FPROC glUniform2f;
	PFNGLUNIFORM3FPROC glUniform3f;
	PFNGLUNIFORM4FPROC glUniform4f;
	PFNGLUNIFORM1IPROC glUniform1i;
	PFNGLUNIFORM2IPROC glUniform2i;
	PFNGLUNIFORM3IPROC glUniform3i;
	PFNGLUNIFORM4IPROC glUniform4i;
	PFNGLUNIFORM1FVPROC glUniform1fv;
	PFNGLUNIFORM2FVPROC glUniform2fv;
	PFNGLUNIFORM3FVPROC glUniform3fv;
	PFNGLUNIFORM4FVPROC glUniform4fv;
	PFNGLUNIFORM1IVPROC glUniform1iv;
	PFNGLUNIFORM2IVPROC glUniform2iv;
	PFNGLUNIFORM3IVPROC glUniform3iv;
	PFNGLUNIFORM4IVPROC glUniform4iv;
	PFNGLUNIFORMMATRIX2FVPROC glUniformMatrix2fv;
	PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fv;
	PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
	PFNGLVALIDATEPROGRAMPROC glValidateProgram;
	PFNGLCOMPILESHADERPROC glCompileShader;
	PFNGLCREATEPROGRAMPROC glCreateProgram;
	PFNGLCREATESHADERPROC glCreateShader;
	PFNGLDELETEPROGRAMPROC glDeleteProgram;
	PFNGLDELETESHADERPROC glDeleteShader;
	PFNGLDETACHSHADERPROC glDetachShader;
	PFNGLATTACHSHADERPROC glAttachShader;
	PFNGLGETPROGRAMIVPROC glGetProgramiv;
	PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
	PFNGLGETSHADERIVPROC glGetShaderiv;
	PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
	PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
	PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
	PFNGLPROGRAMBINARYPROC glProgramBinary;
	PFNGLGETPROGRAMBINARYPROC glGetProgramBinary;

	PFNGLGENQUERIESARBPROC glGenQueriesARB;
	PFNGLDELETEQUERIESARBPROC glDeleteQueriesARB;

	PFNGLBEGINQUERYARBPROC glBeginQueryARB;
	PFNGLENDQUERYARBPROC glEndQueryARB;

	PFNGLGETQUERYIVARBPROC glGetQueryivARB;
	PFNGLGETQUERYOBJECTIVARBPROC glGetQueryObjectivARB;
	PFNGLGETQUERYOBJECTUIVARBPROC glGetQueryObjectuivARB;

	PFNGLCOMPRESSEDTEXIMAGE2DPROC glCompressedTexImage2D;
	PFNGLGENERATEMIPMAPPROC glGenerateMipmap;

	PFNGLGETSTRINGIPROC glGetStringi;
	PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback;
	PFNGLSAMPLECOVERAGEPROC glSampleCoverage;

private:
	// Determines if all functions were loaded fine
	bool m_bSuccessful;
	// Error string if any
	CString m_errorString;
};
#endif