/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.

===============================================
*/

#include "includes.h"
#include "r_glextf.h"
#include "common.h"

//=============================================
// @brief Constructor
//
//=============================================
CGLExtF::CGLExtF ( void ):
	glActiveTexture(nullptr),
	glClientActiveTexture(nullptr),
	glBindBuffer(nullptr),
	glGenBuffers(nullptr),
	glBufferData(nullptr),
	glDeleteBuffers(nullptr),
	glBufferSubData(nullptr),
	glGenVertexArrays(nullptr),
	glBindVertexArray(nullptr),
	glDeleteVertexArrays(nullptr),
	glVertexAttribPointer(nullptr),
	glEnableVertexAttribArray(nullptr),
	glDisableVertexAttribArray(nullptr),
	glGenRenderbuffers(nullptr),
	glBindRenderbuffer(nullptr),
	glRenderbufferStorage(nullptr),
	glFramebufferRenderbuffer(nullptr),
	glFramebufferTexture2D(nullptr),
	glCheckFramebufferStatus(nullptr),
	glBindFramebuffer(nullptr),
	glGenFramebuffers(nullptr),
	glDeleteRenderbuffers(nullptr),
	glDeleteFramebuffers(nullptr),
	glGetFramebufferAttachmentParameteriv(nullptr),
	glGetRenderbufferParameteriv(nullptr),
	glBlitFramebuffer(nullptr),
	glLinkProgram(nullptr),
	glShaderSource(nullptr),
	glUseProgram(nullptr),
	glUniform1f(nullptr),
	glUniform2f(nullptr),
	glUniform3f(nullptr),
	glUniform4f(nullptr),
	glUniform1i(nullptr),
	glUniform2i(nullptr),
	glUniform3i(nullptr),
	glUniform4i(nullptr),
	glUniform1fv(nullptr),
	glUniform2fv(nullptr),
	glUniform3fv(nullptr),
	glUniform4fv(nullptr),
	glUniform1iv(nullptr),
	glUniform2iv(nullptr),
	glUniform3iv(nullptr),
	glUniform4iv(nullptr),
	glUniformMatrix2fv(nullptr),
	glUniformMatrix3fv(nullptr),
	glUniformMatrix4fv(nullptr),
	glValidateProgram(nullptr),
	glCompileShader(nullptr),
	glCreateProgram(nullptr),
	glCreateShader(nullptr),
	glDeleteProgram(nullptr),
	glDeleteShader(nullptr),
	glDetachShader(nullptr),
	glAttachShader(nullptr),
	glGetProgramiv(nullptr),
	glGetProgramInfoLog(nullptr),
	glGetShaderiv(nullptr),
	glGetShaderInfoLog(nullptr),
	glGetUniformLocation(nullptr),
	glGetAttribLocation(nullptr),
	glProgramBinary(nullptr),
	glGetProgramBinary(nullptr),
	glGenQueriesARB(nullptr),
	glDeleteQueriesARB(nullptr),
	glBeginQueryARB(nullptr),
	glEndQueryARB(nullptr),
	glGetQueryivARB(nullptr),
	glGetQueryObjectivARB(nullptr),
	glGetQueryObjectuivARB(nullptr),
	glCompressedTexImage2D(nullptr),
	glGenerateMipmap(nullptr),
	glGetStringi(nullptr),
	glDebugMessageCallback(nullptr),
	glSampleCoverage(nullptr),
	m_bSuccessful(true)
{
}

//=============================================
// @brief Destructor
//
//=============================================
CGLExtF::~CGLExtF ( void )
{
}

//=============================================
// @brief Loads an OpenGL function's address
//
// @param pstrName Function name
// @return TRUE if all functions were loaded, FALSE otherwise
//=============================================
PROC CGLExtF::GetFunction( const Char* pstrName, bool setFailureCode )
{
	PROC pfn = wglGetProcAddress(pstrName);
	if(setFailureCode && !pfn)
	{
		if(m_errorString.empty())
			m_errorString << "The following functions failed to be loaded: ";
		else
			m_errorString << ", ";

		m_errorString << pstrName;
		m_bSuccessful = false;
		return nullptr;
	}

	return pfn;
}

//=============================================
// @brief Loads the external GL functions
//
// @return TRUE if all functions were loaded, FALSE otherwise
//=============================================
bool CGLExtF::GetFunctions ( void )
{
	glActiveTexture = (PFNGLACTIVETEXTUREPROC)GetFunction("glActiveTexture");
	glClientActiveTexture = (PFNGLCLIENTACTIVETEXTUREARBPROC)GetFunction("glClientActiveTextureARB");

	glBindBuffer = (PFNGLBINDBUFFERPROC)GetFunction("glBindBuffer");
	glGenBuffers = (PFNGLGENBUFFERSPROC)GetFunction("glGenBuffers");
	glBufferData = (PFNGLBUFFERDATAPROC)GetFunction("glBufferData");
	glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)GetFunction("glDeleteBuffers");
	glBufferSubData = (PFNGLBUFFERSUBDATAPROC)GetFunction("glBufferSubData");

	glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)GetFunction("glGenVertexArrays");
	glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)GetFunction("glBindVertexArray");
	glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)GetFunction("glDeleteVertexArrays");

	glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)GetFunction("glVertexAttribPointer");
	glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)GetFunction("glEnableVertexAttribArray");
	glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)GetFunction("glDisableVertexAttribArray");

	glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)GetFunction("glGenRenderbuffers");
	glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)GetFunction("glBindRenderbuffer");
	glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC)GetFunction("glRenderbufferStorage");
	glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)GetFunction("glFramebufferRenderbuffer");
	glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)GetFunction("glFramebufferTexture2D");
	glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)GetFunction("glCheckFramebufferStatus");
	glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)GetFunction("glBindFramebuffer");
	glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)GetFunction("glGenFramebuffers");
	glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC)GetFunction("glDeleteRenderbuffers");
	glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)GetFunction("glDeleteFramebuffers");
	glGetFramebufferAttachmentParameteriv = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC)GetFunction("glGetFramebufferAttachmentParameteriv");
	glGetRenderbufferParameteriv = (PFNGLGETRENDERBUFFERPARAMETERIVPROC)GetFunction("glGetRenderbufferParameteriv");
	glBlitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC)GetFunction("glBlitFramebuffer");

	glLinkProgram = (PFNGLLINKPROGRAMARBPROC)GetFunction("glLinkProgramARB");
	glShaderSource = (PFNGLSHADERSOURCEPROC)GetFunction("glShaderSource");
	glUseProgram = (PFNGLUSEPROGRAMPROC)GetFunction("glUseProgram");
	glUniform1f = (PFNGLUNIFORM1FPROC)GetFunction("glUniform1f");
	glUniform2f = (PFNGLUNIFORM2FPROC)GetFunction("glUniform2f");
	glUniform3f = (PFNGLUNIFORM3FPROC)GetFunction("glUniform3f");
	glUniform4f = (PFNGLUNIFORM4FPROC)GetFunction("glUniform4f");
	glUniform1i = (PFNGLUNIFORM1IPROC)GetFunction("glUniform1i");
	glUniform2i = (PFNGLUNIFORM2IPROC)GetFunction("glUniform2i");
	glUniform3i = (PFNGLUNIFORM3IPROC)GetFunction("glUniform3i");
	glUniform4i = (PFNGLUNIFORM4IPROC)GetFunction("glUniform4i");
	glUniform1fv = (PFNGLUNIFORM1FVPROC)GetFunction("glUniform1fv");
	glUniform2fv = (PFNGLUNIFORM2FVPROC)GetFunction("glUniform2fv");
	glUniform3fv = (PFNGLUNIFORM3FVPROC)GetFunction("glUniform3fv");
	glUniform4fv = (PFNGLUNIFORM4FVPROC)GetFunction("glUniform4fv");
	glUniform1iv = (PFNGLUNIFORM1IVPROC)GetFunction("glUniform1iv");
	glUniform2iv = (PFNGLUNIFORM2IVPROC)GetFunction("glUniform2iv");
	glUniform3iv = (PFNGLUNIFORM3IVPROC)GetFunction("glUniform3iv");
	glUniform4iv = (PFNGLUNIFORM4IVPROC)GetFunction("glUniform4iv");
	glUniformMatrix2fv = (PFNGLUNIFORMMATRIX2FVPROC)GetFunction("glUniformMatrix2fv");
	glUniformMatrix3fv = (PFNGLUNIFORMMATRIX3FVPROC)GetFunction("glUniformMatrix3fv");
	glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)GetFunction("glUniformMatrix4fv");
	glValidateProgram = (PFNGLVALIDATEPROGRAMPROC)GetFunction("glValidateProgram");
	glCompileShader = (PFNGLCOMPILESHADERPROC)GetFunction("glCompileShader");
	glCreateProgram = (PFNGLCREATEPROGRAMPROC)GetFunction("glCreateProgram");
	glCreateShader = (PFNGLCREATESHADERPROC)GetFunction("glCreateShader");
	glDeleteProgram = (PFNGLDELETEPROGRAMPROC)GetFunction("glDeleteProgram");
	glDeleteShader = (PFNGLDELETESHADERPROC)GetFunction("glDeleteShader");
	glDetachShader = (PFNGLDETACHSHADERPROC)GetFunction("glDetachShader");
	glAttachShader = (PFNGLATTACHSHADERPROC)GetFunction("glAttachShader");
	glGetProgramiv = (PFNGLGETPROGRAMIVPROC)GetFunction("glGetProgramiv");
	glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)GetFunction("glGetProgramInfoLog");
	glGetShaderiv = (PFNGLGETSHADERIVPROC)GetFunction("glGetShaderiv");
	glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)GetFunction("glGetShaderInfoLog");
	glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)GetFunction("glGetUniformLocation");
	glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)GetFunction("glGetAttribLocation");		
	glProgramBinary = (PFNGLPROGRAMBINARYPROC)GetFunction("glProgramBinary");
	glGetProgramBinary = (PFNGLGETPROGRAMBINARYPROC)GetFunction("glGetProgramBinary");

	glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)GetFunction("glGenerateMipmap");

	glGenQueriesARB = (PFNGLGENQUERIESARBPROC)GetFunction("glGenQueriesARB");
	glDeleteQueriesARB = (PFNGLDELETEQUERIESARBPROC)GetFunction("glDeleteQueriesARB");
	glBeginQueryARB = (PFNGLBEGINQUERYARBPROC)GetFunction("glBeginQueryARB");
	glEndQueryARB = (PFNGLENDQUERYARBPROC)GetFunction("glEndQueryARB");
	glGetQueryivARB = (PFNGLGETQUERYIVARBPROC)GetFunction("glGetQueryivARB");
	glGetQueryObjectivARB = (PFNGLGETQUERYOBJECTIVARBPROC)GetFunction("glGetQueryObjectivARB");
	glGetQueryObjectuivARB = (PFNGLGETQUERYOBJECTUIVARBPROC)GetFunction("glGetQueryObjectuivARB");

	glCompressedTexImage2D = (PFNGLCOMPRESSEDTEXIMAGE2DPROC)GetFunction("glCompressedTexImage2D");
	glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)GetFunction("glGenerateMipmap");

	glGetStringi = (PFNGLGETSTRINGIPROC)GetFunction("glGetStringi", false);
	glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC)GetFunction("glDebugMessageCallback", false);
	glSampleCoverage = (PFNGLSAMPLECOVERAGEPROC)GetFunction("glSampleCoverage", false);

	return m_bSuccessful;
}