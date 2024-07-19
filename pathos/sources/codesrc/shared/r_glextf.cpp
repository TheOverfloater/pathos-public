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
	glMapBufferRange(nullptr),
	glUnmapBuffer(nullptr),
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
	glGetUniformBlockIndex(nullptr),
	glUniformBlockBinding(nullptr),
	glBindBufferBase(nullptr),
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
	glActiveTexture							= reinterpret_cast<PFNGLACTIVETEXTUREPROC>(GetFunction("glActiveTexture"));
	glClientActiveTexture					= reinterpret_cast<PFNGLCLIENTACTIVETEXTUREARBPROC>(GetFunction("glClientActiveTextureARB"));

	glBindBuffer							= reinterpret_cast<PFNGLBINDBUFFERPROC>(GetFunction("glBindBuffer"));
	glGenBuffers							= reinterpret_cast<PFNGLGENBUFFERSPROC>(GetFunction("glGenBuffers"));
	glBufferData							= reinterpret_cast<PFNGLBUFFERDATAPROC>(GetFunction("glBufferData"));
	glDeleteBuffers							= reinterpret_cast<PFNGLDELETEBUFFERSPROC>(GetFunction("glDeleteBuffers"));
	glBufferSubData							= reinterpret_cast<PFNGLBUFFERSUBDATAPROC>(GetFunction("glBufferSubData"));
	glMapBufferRange						= reinterpret_cast<PFNGLMAPBUFFERRANGEPROC>(GetFunction("glMapBufferRange"));
	glUnmapBuffer							= reinterpret_cast<PFNGLUNMAPBUFFERPROC>(GetFunction("glUnmapBuffer"));

	glGenVertexArrays						= reinterpret_cast<PFNGLGENVERTEXARRAYSPROC>(GetFunction("glGenVertexArrays"));
	glBindVertexArray						= reinterpret_cast<PFNGLBINDVERTEXARRAYPROC>(GetFunction("glBindVertexArray"));
	glDeleteVertexArrays					= reinterpret_cast<PFNGLDELETEVERTEXARRAYSPROC>(GetFunction("glDeleteVertexArrays"));

	glVertexAttribPointer					= reinterpret_cast<PFNGLVERTEXATTRIBPOINTERPROC>(GetFunction("glVertexAttribPointer"));
	glEnableVertexAttribArray				= reinterpret_cast<PFNGLENABLEVERTEXATTRIBARRAYPROC>(GetFunction("glEnableVertexAttribArray"));
	glDisableVertexAttribArray				= reinterpret_cast<PFNGLDISABLEVERTEXATTRIBARRAYPROC>(GetFunction("glDisableVertexAttribArray"));

	glGenRenderbuffers						= reinterpret_cast<PFNGLGENRENDERBUFFERSPROC>(GetFunction("glGenRenderbuffers"));
	glBindRenderbuffer						= reinterpret_cast<PFNGLBINDRENDERBUFFERPROC>(GetFunction("glBindRenderbuffer"));
	glRenderbufferStorage					= reinterpret_cast<PFNGLRENDERBUFFERSTORAGEPROC>(GetFunction("glRenderbufferStorage"));
	glRenderbufferStorageMultisample 		= reinterpret_cast<PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC>(GetFunction("glRenderbufferStorageMultisample"));
	glFramebufferRenderbuffer				= reinterpret_cast<PFNGLFRAMEBUFFERRENDERBUFFERPROC>(GetFunction("glFramebufferRenderbuffer"));
	glFramebufferTexture2D					= reinterpret_cast<PFNGLFRAMEBUFFERTEXTURE2DPROC>(GetFunction("glFramebufferTexture2D"));
	glCheckFramebufferStatus				= reinterpret_cast<PFNGLCHECKFRAMEBUFFERSTATUSPROC>(GetFunction("glCheckFramebufferStatus"));
	glBindFramebuffer						= reinterpret_cast<PFNGLBINDFRAMEBUFFERPROC>(GetFunction("glBindFramebuffer"));
	glGenFramebuffers						= reinterpret_cast<PFNGLGENFRAMEBUFFERSPROC>(GetFunction("glGenFramebuffers"));
	glDeleteRenderbuffers					= reinterpret_cast<PFNGLDELETERENDERBUFFERSPROC>(GetFunction("glDeleteRenderbuffers"));
	glDeleteFramebuffers					= reinterpret_cast<PFNGLDELETEFRAMEBUFFERSPROC>(GetFunction("glDeleteFramebuffers"));
	glGetFramebufferAttachmentParameteriv	= reinterpret_cast<PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC>(GetFunction("glGetFramebufferAttachmentParameteriv"));
	glGetRenderbufferParameteriv			= reinterpret_cast<PFNGLGETRENDERBUFFERPARAMETERIVPROC>(GetFunction("glGetRenderbufferParameteriv"));
	glBlitFramebuffer						= reinterpret_cast<PFNGLBLITFRAMEBUFFERPROC>(GetFunction("glBlitFramebuffer"));

	glLinkProgram							= reinterpret_cast<PFNGLLINKPROGRAMARBPROC>(GetFunction("glLinkProgram"));
	glShaderSource							= reinterpret_cast<PFNGLSHADERSOURCEPROC>(GetFunction("glShaderSource"));
	glUseProgram							= reinterpret_cast<PFNGLUSEPROGRAMPROC>(GetFunction("glUseProgram"));
	glUniform1f								= reinterpret_cast<PFNGLUNIFORM1FPROC>(GetFunction("glUniform1f"));
	glUniform2f								= reinterpret_cast<PFNGLUNIFORM2FPROC>(GetFunction("glUniform2f"));
	glUniform3f								= reinterpret_cast<PFNGLUNIFORM3FPROC>(GetFunction("glUniform3f"));
	glUniform4f								= reinterpret_cast<PFNGLUNIFORM4FPROC>(GetFunction("glUniform4f"));
	glUniform1i								= reinterpret_cast<PFNGLUNIFORM1IPROC>(GetFunction("glUniform1i"));
	glUniform2i								= reinterpret_cast<PFNGLUNIFORM2IPROC>(GetFunction("glUniform2i"));
	glUniform3i								= reinterpret_cast<PFNGLUNIFORM3IPROC>(GetFunction("glUniform3i"));
	glUniform4i								= reinterpret_cast<PFNGLUNIFORM4IPROC>(GetFunction("glUniform4i"));
	glUniform1fv							= reinterpret_cast<PFNGLUNIFORM1FVPROC>(GetFunction("glUniform1fv"));
	glUniform2fv							= reinterpret_cast<PFNGLUNIFORM2FVPROC>(GetFunction("glUniform2fv"));
	glUniform3fv							= reinterpret_cast<PFNGLUNIFORM3FVPROC>(GetFunction("glUniform3fv"));
	glUniform4fv							= reinterpret_cast<PFNGLUNIFORM4FVPROC>(GetFunction("glUniform4fv"));
	glUniform1iv							= reinterpret_cast<PFNGLUNIFORM1IVPROC>(GetFunction("glUniform1iv"));
	glUniform2iv							= reinterpret_cast<PFNGLUNIFORM2IVPROC>(GetFunction("glUniform2iv"));
	glUniform3iv							= reinterpret_cast<PFNGLUNIFORM3IVPROC>(GetFunction("glUniform3iv"));
	glUniform4iv							= reinterpret_cast<PFNGLUNIFORM4IVPROC>(GetFunction("glUniform4iv"));
	glUniformMatrix2fv						= reinterpret_cast<PFNGLUNIFORMMATRIX2FVPROC>(GetFunction("glUniformMatrix2fv"));
	glUniformMatrix3fv						= reinterpret_cast<PFNGLUNIFORMMATRIX3FVPROC>(GetFunction("glUniformMatrix3fv"));
	glUniformMatrix4fv						= reinterpret_cast<PFNGLUNIFORMMATRIX4FVPROC>(GetFunction("glUniformMatrix4fv"));
	glValidateProgram						= reinterpret_cast<PFNGLVALIDATEPROGRAMPROC>(GetFunction("glValidateProgram"));
	glCompileShader							= reinterpret_cast<PFNGLCOMPILESHADERPROC>(GetFunction("glCompileShader"));
	glCreateProgram							= reinterpret_cast<PFNGLCREATEPROGRAMPROC>(GetFunction("glCreateProgram"));
	glCreateShader							= reinterpret_cast<PFNGLCREATESHADERPROC>(GetFunction("glCreateShader"));
	glDeleteProgram							= reinterpret_cast<PFNGLDELETEPROGRAMPROC>(GetFunction("glDeleteProgram"));
	glDeleteShader							= reinterpret_cast<PFNGLDELETESHADERPROC>(GetFunction("glDeleteShader"));
	glDetachShader							= reinterpret_cast<PFNGLDETACHSHADERPROC>(GetFunction("glDetachShader"));
	glAttachShader							= reinterpret_cast<PFNGLATTACHSHADERPROC>(GetFunction("glAttachShader"));
	glGetProgramiv							= reinterpret_cast<PFNGLGETPROGRAMIVPROC>(GetFunction("glGetProgramiv"));
	glGetProgramInfoLog						= reinterpret_cast<PFNGLGETPROGRAMINFOLOGPROC>(GetFunction("glGetProgramInfoLog"));
	glGetShaderiv							= reinterpret_cast<PFNGLGETSHADERIVPROC>(GetFunction("glGetShaderiv"));
	glGetShaderInfoLog						= reinterpret_cast<PFNGLGETSHADERINFOLOGPROC>(GetFunction("glGetShaderInfoLog"));
	glGetUniformLocation					= reinterpret_cast<PFNGLGETUNIFORMLOCATIONPROC>(GetFunction("glGetUniformLocation"));
	glGetAttribLocation						= reinterpret_cast<PFNGLGETATTRIBLOCATIONPROC>(GetFunction("glGetAttribLocation"));		
	glProgramBinary							= reinterpret_cast<PFNGLPROGRAMBINARYPROC>(GetFunction("glProgramBinary"));
	glGetProgramBinary						= reinterpret_cast<PFNGLGETPROGRAMBINARYPROC>(GetFunction("glGetProgramBinary"));
	glGetUniformBlockIndex					= reinterpret_cast<PFNGLGETUNIFORMBLOCKINDEXPROC>(GetFunction("glGetUniformBlockIndex"));
	glUniformBlockBinding					= reinterpret_cast<PFNGLUNIFORMBLOCKBINDINGPROC>(GetFunction("glUniformBlockBinding"));
	glBindBufferBase						= reinterpret_cast<PFNGLBINDBUFFERBASEPROC>(GetFunction("glBindBufferBase"));

	glGenerateMipmap						= reinterpret_cast<PFNGLGENERATEMIPMAPPROC>(GetFunction("glGenerateMipmap"));

	glGenQueriesARB							= reinterpret_cast<PFNGLGENQUERIESARBPROC>(GetFunction("glGenQueriesARB"));
	glDeleteQueriesARB						= reinterpret_cast<PFNGLDELETEQUERIESARBPROC>(GetFunction("glDeleteQueriesARB"));
	glBeginQueryARB							= reinterpret_cast<PFNGLBEGINQUERYARBPROC>(GetFunction("glBeginQueryARB"));
	glEndQueryARB							= reinterpret_cast<PFNGLENDQUERYARBPROC>(GetFunction("glEndQueryARB"));
	glGetQueryivARB							= reinterpret_cast<PFNGLGETQUERYIVARBPROC>(GetFunction("glGetQueryivARB"));
	glGetQueryObjectivARB					= reinterpret_cast<PFNGLGETQUERYOBJECTIVARBPROC>(GetFunction("glGetQueryObjectivARB"));
	glGetQueryObjectuivARB					= reinterpret_cast<PFNGLGETQUERYOBJECTUIVARBPROC>(GetFunction("glGetQueryObjectuivARB"));

	glCompressedTexImage2D					= reinterpret_cast<PFNGLCOMPRESSEDTEXIMAGE2DPROC>(GetFunction("glCompressedTexImage2D"));
	glGenerateMipmap						= reinterpret_cast<PFNGLGENERATEMIPMAPPROC>(GetFunction("glGenerateMipmap"));

	glGetStringi							= reinterpret_cast<PFNGLGETSTRINGIPROC>(GetFunction("glGetStringi", false));
	glDebugMessageCallback					= reinterpret_cast<PFNGLDEBUGMESSAGECALLBACKPROC>(GetFunction("glDebugMessageCallback", false));
	glSampleCoverage						= reinterpret_cast<PFNGLSAMPLECOVERAGEPROC>(GetFunction("glSampleCoverage", false));
	glClampColor 							= reinterpret_cast<PFNGLCLAMPCOLORPROC>(GetFunction("glClampColor", false));

	return m_bSuccessful;
}