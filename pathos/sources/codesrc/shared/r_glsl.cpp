/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include <ctime>

#include "includes.h"
#include "r_vbo.h"
#include "r_glsl.h"
#include "r_glextf.h"
#include "md5.h"
#include "cbuffer.h"

// Total duration of vertex shader compile calls
Double CGLSLShader::g_vertexShaderCompileTotalDuration = 0;
// Total duration of vertex shader verification calls
Double CGLSLShader::g_vertexShaderGetStatusCallTotalDuration = 0;
// Total duration of fragment shader compile calls
Double CGLSLShader::g_fragmentShaderCompileTotalDuration = 0;
// Total duration of fragment shader compile calls
Double CGLSLShader::g_fragmentShaderGetStatusCallTotalDuration = 0;
// Total duration of shader linking calls
Double CGLSLShader::g_shaderLinkTotalDuration = 0;
// Total duration of shader linking calls
Double CGLSLShader::g_shaderLinkGetStatusCallDuration = 0;
// Number of shader programs linked total
Uint32 CGLSLShader::g_numShaderProgramsLinked = 0;

//=============================================
// @brief Constructor
//
// @param szfile File path string
// @param flags Shader compile flags
//=============================================
CGLSLShader::CGLSLShader ( const file_interface_t& fileFuncs, const CGLExtF& glExtF, const Char *szfile, Int32 flags, pfnProgressUpdateFunction_t pfnCallback ):
	m_fileInterface(fileFuncs),
	m_glExtF( glExtF ),
	m_shaderIndex( 0 ),
	m_pVBO( nullptr ),
	m_lastIndex( -1 ),
	m_pVertexScript( nullptr ),
	m_pFragmentScript( nullptr ),
	m_pDeterminatorValues( nullptr ),
	m_pCSDHeader( nullptr ),
	m_reCheck( false ),
	m_onDemandLoad( false ),
	m_useBinaryShaders( false ),
	m_areUBOsBound( false ),
	m_isActive( false ),
	m_bFailed( false ),
	m_pProgressUpdateCallbackFn(*pfnCallback)
{
	memset(m_uniformMatrix, 0, sizeof(m_uniformMatrix));

	if(flags & FL_GLSL_ONDEMAND_LOAD)
		m_onDemandLoad = true;

	if(flags & FL_GLSL_BINARY_SHADER_OPS)
	{
		m_useBinaryShaders = true;
		if(m_onDemandLoad)
			m_onDemandLoad = false;
	}

	// Clear the entire class
	Clear();

	if(!Compile(szfile))
		m_bFailed = true;
}

//=============================================
// @brief Constructor
//
// @param flags Shader compile flags
//=============================================
CGLSLShader::CGLSLShader ( const file_interface_t& fileFuncs, const CGLExtF& glExtF, Int32 flags, pfnProgressUpdateFunction_t pfnCallback ):
	m_fileInterface(fileFuncs),
	m_glExtF( glExtF ),
	m_shaderIndex( 0 ),
	m_pVBO( nullptr ),
	m_lastIndex( -1 ),
	m_pVertexScript( nullptr ),
	m_pFragmentScript( nullptr ),
	m_pDeterminatorValues( nullptr ),
	m_pCSDHeader( nullptr ),
	m_reCheck( false ),
	m_onDemandLoad( false ),
	m_useBinaryShaders( false ),
	m_areUBOsBound( false ),
	m_isActive( false ),
	m_bFailed( false ),
	m_pProgressUpdateCallbackFn(*pfnCallback)
{
	memset(m_uniformMatrix, 0, sizeof(m_uniformMatrix));

	if(flags & FL_GLSL_ONDEMAND_LOAD)
		m_onDemandLoad = true;

	if(flags & FL_GLSL_BINARY_SHADER_OPS)
	{
		m_useBinaryShaders = true;
		if(m_onDemandLoad)
			m_onDemandLoad = false;
	}

	Clear();
}

//=============================================
// @brief Destructor
//
//=============================================
CGLSLShader::~CGLSLShader ( void )
{
	FreeData();
}

//=============================================
// @brief Clears the shader class's data
//
//=============================================
void CGLSLShader::Clear ( void )
{
	m_shaderFile.clear();
	m_errorString.clear();
}

//=============================================
// @brief Recurses through a chunk and it's children, and frees it's memory
//
// @param pchunk Pointer to chunk object to free
//=============================================
void CGLSLShader::RecursiveFreeChunks ( shader_chunk_t* pchunk )
{
	for(Uint32 i = 0; i < pchunk->numchunks; i++)
		RecursiveFreeChunks(&pchunk->pchunks[i]);

	if(pchunk->pdata)
		delete[] pchunk->pdata;

	if(pchunk->pchunks)
		delete[] pchunk->pchunks;

	if(pchunk->pconditionals)
		delete[] pchunk->pconditionals;
}

//=============================================
// @brief Frees compile time shader data
//
//=============================================
void CGLSLShader::FreeTempData ( void )
{
	if(m_pVertexScript)
	{
		for(Uint32 i = 0; i < m_pVertexScript->numchunks; i++)
			RecursiveFreeChunks(&m_pVertexScript->pchunks[i]);

		if(m_pVertexScript->pchunks)
			delete[] m_pVertexScript->pchunks;

		delete[] m_pVertexScript;
		m_pVertexScript = nullptr;
	}
		
	if(m_pFragmentScript)
	{
		for(Uint32 i = 0; i < m_pFragmentScript->numchunks; i++)
			RecursiveFreeChunks(&m_pFragmentScript->pchunks[i]);

		if(m_pFragmentScript->pchunks)
			delete[] m_pFragmentScript->pchunks;

		delete[] m_pFragmentScript;
		m_pFragmentScript = nullptr;
	}

	if(!m_disabledStatesArray.empty())
		m_disabledStatesArray.clear();
}

//=============================================
// @brief Frees all the OpenGL shader data
//
//=============================================
void CGLSLShader::FreeShaderData ( void )
{
	if(!m_determinatorArray.empty())
		m_determinatorArray.clear();

	if(!m_shadersArray.empty())
	{
		for(Uint32 i = 0; i < m_shadersArray.size(); i++)
		{
			if(m_shadersArray[i].program_id)
				m_glExtF.glDeleteProgram(m_shadersArray[i].program_id);
		}

		m_shadersArray.clear();
	}
}

//=============================================
// @brief Frees all the shader data
//
//=============================================
void CGLSLShader::FreeData ( void )
{
	if(!m_uniformsArray.empty())
		m_uniformsArray.clear();

	if(!m_uniformBufferObjectsArray.empty())
	{
		for(Uint32 i = 0; i < m_uniformBufferObjectsArray.size(); i++)
		{
			glsl_ubo_t& ubo = m_uniformBufferObjectsArray[i];
			
			if(ubo.buffer_id)
				m_glExtF.glDeleteBuffers(1, &ubo.buffer_id);
		}

		m_uniformBufferObjectsArray.clear();
	}

	if(m_pDeterminatorValues)
	{
		delete[] m_pDeterminatorValues;
		m_pDeterminatorValues = nullptr;
	}

	if(!m_vertexAttribsArray.empty())
		m_vertexAttribsArray.clear();

	if(!m_invalidStatesArray.empty())
		m_invalidStatesArray.clear();
	
	if(m_pCSDHeader)
	{
		delete[] m_pCSDHeader;
		m_pCSDHeader = nullptr;
	}

	FreeTempData();
	FreeShaderData();
}

//=============================================
// @brief Compiles a single unique shader
//
// @param index Index of the shader to compile
// @param pshader Shader information object
// @param pshaderdata Pointer to compiled shader data
// @return TRUE if the compile was successful, FALSE otherwise
//=============================================
bool CGLSLShader::CompileShader( Uint32 index, glsl_shader_t* pshader, csdshaderdata_t* pshaderdata )
{
	// Get start clock
	clock_t beginTime = clock();

	// Try to compile the vertex shader
	const Char *vp = reinterpret_cast<Char*>(reinterpret_cast<byte*>(m_pCSDHeader) + pshaderdata->vertexdataoffs);
	GLuint vertex_id = m_glExtF.glCreateShader(GL_VERTEX_SHADER);
	m_glExtF.glShaderSource(vertex_id, 1, &vp, &pshaderdata->vertexdatasize);
	m_glExtF.glCompileShader(vertex_id);

	// Now get elapsed time
	g_vertexShaderCompileTotalDuration += static_cast<Double>(clock() - beginTime) / CLOCKS_PER_SEC;

	CString basename;
	Common::Basename(m_shaderFile.c_str(), basename);

	CString vsOut;
	vsOut << "logs/" << basename << "_" << (Int32)index << "_vs";

	// Get start clock
	beginTime = clock();

	Int32 iStatus = FALSE;
	m_glExtF.glGetShaderiv(vertex_id, GL_COMPILE_STATUS, &iStatus);
	g_vertexShaderGetStatusCallTotalDuration += static_cast<Double>(clock() - beginTime) / CLOCKS_PER_SEC;
	Shader_PrintLog(vertex_id, vp, pshaderdata->vertexdatasize, vsOut.c_str(), (iStatus != TRUE) ? true : false);

	if(iStatus != TRUE)
	{
		m_errorString = "Vertex shader " + m_shaderFile + " failed to compile. Log file was written.";
		return false;
	}

	// Get start clock
	beginTime = clock();

	// Compile the fragment shader now
	const Char *fp = reinterpret_cast<Char*>(reinterpret_cast<byte*>(m_pCSDHeader) + pshaderdata->fragmentdataoffs);
	GLuint fragment_id = m_glExtF.glCreateShader(GL_FRAGMENT_SHADER);
	m_glExtF.glShaderSource(fragment_id, 1, &fp, &pshaderdata->fragmentdatasize);
	m_glExtF.glCompileShader(fragment_id);
		
	// Now get elapsed time
	g_fragmentShaderCompileTotalDuration += (Double)(clock() - beginTime) / CLOCKS_PER_SEC;

	CString fsOut;
	fsOut << "logs/" << basename << "_" << (Int32)index << "_fs";

	// Get start clock
	beginTime = clock();

	m_glExtF.glGetShaderiv(fragment_id, GL_COMPILE_STATUS, &iStatus);
	g_fragmentShaderGetStatusCallTotalDuration += (Double)(clock() - beginTime) / CLOCKS_PER_SEC;
	Shader_PrintLog(fragment_id, fp, pshaderdata->fragmentdatasize, fsOut.c_str(), (iStatus != TRUE) ? true : false);

	if(iStatus != TRUE)
	{
		m_errorString = "Fragment shader " + m_shaderFile + " failed to compile. Log file was written.";
		return false;
	}

	// Get start clock
	beginTime = clock();

	// Link the two into one program
	pshader->program_id = m_glExtF.glCreateProgram();
	m_glExtF.glAttachShader(pshader->program_id, vertex_id);
	m_glExtF.glAttachShader(pshader->program_id, fragment_id);
	m_glExtF.glLinkProgram(pshader->program_id);

	// Now get elapsed time
	g_shaderLinkTotalDuration += (Double)(clock() - beginTime) / CLOCKS_PER_SEC;

	CString progOut;
	progOut << "logs/" << basename << "_" << (Int32)index << "_prog";

	// Get start clock
	beginTime = clock();

	m_glExtF.glGetProgramiv(pshader->program_id, GL_LINK_STATUS, &iStatus);
	g_shaderLinkGetStatusCallDuration += (Double)(clock() - beginTime) / CLOCKS_PER_SEC;
	Program_PrintLog(pshader->program_id, progOut.c_str());

	if(iStatus != TRUE)
	{
		Shader_PrintLog(vertex_id, vp, pshaderdata->vertexdatasize, vsOut.c_str(), true);
		Shader_PrintLog(fragment_id, fp, pshaderdata->fragmentdatasize, fsOut.c_str(), true);

		m_errorString = "Program " + m_shaderFile + " failed to compile. Log file was written.";
	}

	m_glExtF.glDetachShader(pshader->program_id, vertex_id);
	m_glExtF.glDeleteShader(vertex_id);

	m_glExtF.glDetachShader(pshader->program_id, fragment_id);
	m_glExtF.glDeleteShader(fragment_id);

	if(iStatus == TRUE)
	{
		// Flag it as compiled
		pshader->compiled = true;
		g_numShaderProgramsLinked++;
	}

	return (iStatus == TRUE) ? true : false;
}

//=============================================
// @brief Validates a program, and if it's not ok, it'll write to the console
//
// @param msg CString to hold result
// @param return TRUE if no error was returned, FALSE otherwise
//=============================================
bool CGLSLShader::ValidateProgram( void (*pfnConPrintfFnPtr)( const Char *fmt, ... ) )
{
	if(!pfnConPrintfFnPtr)
		return true;

	GLint status;
	m_glExtF.glValidateProgram(m_shadersArray[m_shaderIndex].program_id);
	m_glExtF.glGetProgramiv(m_shadersArray[m_shaderIndex].program_id, GL_VALIDATE_STATUS, &status);

	if(status == GL_TRUE)
		return true;

	GLint logSize1;
	m_glExtF.glGetProgramiv(m_shadersArray[m_shaderIndex].program_id, GL_INFO_LOG_LENGTH, &logSize1);
	if(!logSize1)
		return true;

	GLsizei logSize2;
	Char* pstrTmp = new Char[logSize1+1];
	m_glExtF.glGetProgramInfoLog(m_shadersArray[m_shaderIndex].program_id, logSize1+1, &logSize2, pstrTmp);

	(*pfnConPrintfFnPtr)("%s - Validation error: %s\n", m_shaderFile.c_str(), pstrTmp);
	delete[] pstrTmp;

	return false;
}

//=============================================
// @brief Compiles the shader from a CSD file
//
// @return TRUE if successful, FLASE otherwise
//=============================================
bool CGLSLShader::CompileFromCSD( void )
{
	// Generate hash so we can verify
	CString scriptFilePath;
	scriptFilePath << "scripts/shaders/" << m_shaderFile;

	Uint32 scriptFileSize = 0;
	const byte* pScriptFile = m_fileInterface.pfnLoadFile(scriptFilePath.c_str(), &scriptFileSize);
	if(!pScriptFile)
	{
		m_errorString << "Failed to open '" << m_shaderFile << "' for reading.";
		m_bFailed = true;
		return false;
	}

	CMD5 md5Hash(pScriptFile, scriptFileSize);
	CString strHash = md5Hash.HexDigest();
	m_fileInterface.pfnFreeFile(pScriptFile);

	// Build the path
	CString basename;
	Common::Basename(m_shaderFile.c_str(), basename);

	CString csdFilePath;
	if(!m_rootDirectory.empty())
		csdFilePath << m_rootDirectory << PATH_SLASH_CHAR;

	csdFilePath << "scripts/shaders/" << basename << ".csd";

	// Open the file for reading
	Uint32 csdFileSize = 0;
	const byte *pFile = m_fileInterface.pfnLoadFile(csdFilePath.c_str(), &csdFileSize);
	if(!pFile)
		return false;

	CString bssFilePath;
	if(!m_rootDirectory.empty())
		bssFilePath << m_rootDirectory << PATH_SLASH_CHAR;

	bssFilePath << "scripts/shaders/" << m_shaderFile;

	// Retreive date for BSS file
	file_dateinfo_t bssdate, csddate;
	if(!m_fileInterface.pfnGetFileDate(bssFilePath.c_str(), bssdate) || !m_fileInterface.pfnGetFileDate(csdFilePath.c_str(), csddate))
	{
		m_fileInterface.pfnFreeFile(pFile);
		return false;
	}

	if(m_fileInterface.pfnCompareFileDates(bssdate, csddate) < 0)
	{
		m_fileInterface.pfnFreeFile(pFile);
		return false;
	}

	// Make sure the header is correct
	const csdheader_t* pFileCSDHeader = reinterpret_cast<const csdheader_t*>(pFile);
	if(pFileCSDHeader->id != CSD_HEADER_ENCODED)
	{
		m_fileInterface.pfnFreeFile(pFile);
		return false;
	}

	if(pFileCSDHeader->version != CSD_FILE_VERSION)
	{
		m_fileInterface.pfnFreeFile(pFile);
		return false;
	}

	// Check the hash
	if(memcmp(pFileCSDHeader->hash, strHash.c_str(), sizeof(Char)*32))
	{
		m_fileInterface.pfnFreeFile(pFile);
		return false;
	}

	// Set the basic data
	m_pCSDHeader = reinterpret_cast<csdheader_t*>(new byte[csdFileSize]);
	memcpy(m_pCSDHeader, pFile, csdFileSize);
	m_fileInterface.pfnFreeFile(pFile);

	// Set determinator values
	m_pDeterminatorValues = new Int32[m_pCSDHeader->numshaders];
	for(Uint32 i = 0; i < m_pCSDHeader->numshaders; i++)
		m_pDeterminatorValues[i] = 0;

	// Set determinators
	m_determinatorArray.resize(m_pCSDHeader->numdeterminators);
	csddeterminator_t* pindeterminators = reinterpret_cast<csddeterminator_t*>(reinterpret_cast<byte*>(m_pCSDHeader) + m_pCSDHeader->determinatoroffset);
	for(Uint32 i = 0; i < m_pCSDHeader->numdeterminators; i++)
	{
		m_determinatorArray[i].name = pindeterminators[i].name;
		m_determinatorArray[i].type = pindeterminators[i].type;
		m_determinatorArray[i].minval = pindeterminators[i].minval;
		m_determinatorArray[i].maxval = pindeterminators[i].maxval;

		// Copy values
		Int16* pinvalues = reinterpret_cast<Int16*>(reinterpret_cast<byte*>(m_pCSDHeader) + pindeterminators[i].valuesoffset);
		m_determinatorArray[i].values.resize(m_pCSDHeader->numshaders);
		memcpy(&m_determinatorArray[i].values[0], pinvalues, sizeof(Int16)*m_pCSDHeader->numshaders);
	}

	// Allocate the shaders
	m_shadersArray.resize(m_pCSDHeader->numshaders);

	for(Uint32 j = 0; j < m_disabledStatesArray.size(); j++)
	{
		disabled_state_t& state = m_disabledStatesArray[j];

		Uint32 k = 0;
		for(; k < m_determinatorArray.size(); k++)
		{
			if(!qstrcmp(state.dt_name, m_determinatorArray[k].name))
			{
				state.dt_index = k;
				break;
			}
		}

		if(k == m_determinatorArray.size())
		{
			m_errorString << "Unable to find determinator " << state.dt_name << " for disabled state in shader " << m_shaderFile;
			return false;
		}
	}

	// Load the BSD file if supported
	bool bsdFailed = false;
	if(m_useBinaryShaders)
	{
		if(LoadFromBSD())
			return true;
		else
			bsdFailed = true;
	}

	// Load the shader data if newly compiled, if not on-demand loading
	if(!m_onDemandLoad || bsdFailed)
	{
		if(!CompileCSDShaderData())
			return false;
	}

	return true;
}

//=============================================
// @brief Loads shaders from the BSD file
//
// @return TRUE if no errors occurred, FLASE otherwise
//=============================================
bool CGLSLShader::LoadFromBSD( void )
{
	// Build the path
	CString basename;
	Common::Basename(m_shaderFile.c_str(), basename);

	CString bsdFilePath;
	if(!m_rootDirectory.empty())
		bsdFilePath << m_rootDirectory << PATH_SLASH_CHAR;

	bsdFilePath << "scripts/shaders/";
		
#ifdef _64BUILD
	bsdFilePath << "binary_x64/";
#else
	bsdFilePath << "binary_x86/";
#endif

	bsdFilePath << basename << ".bsd";

	// Open the file for reading
	Uint32 bsdFileSize = 0;
	const byte *pFile = m_fileInterface.pfnLoadFile(bsdFilePath.c_str(), &bsdFileSize);
	if(!pFile)
		return false;

	CString bssFilePath;
	if(!m_rootDirectory.empty())
		bssFilePath << m_rootDirectory << PATH_SLASH_CHAR;

	bssFilePath << "scripts/shaders/" << m_shaderFile;

	// Retreive date for BSS file
	file_dateinfo_t bssdate, bsddate;
	if(!m_fileInterface.pfnGetFileDate(bssFilePath.c_str(), bssdate) || !m_fileInterface.pfnGetFileDate(bsdFilePath.c_str(), bsddate))
	{
		m_fileInterface.pfnFreeFile(pFile);
		return false;
	}

	if(m_fileInterface.pfnCompareFileDates(bssdate, bsddate) < 0)
	{
		m_fileInterface.pfnFreeFile(pFile);
		return false;
	}

	const bsd_header_t* pBSDHeader = reinterpret_cast<const bsd_header_t*>(pFile);
	if(pBSDHeader->id != BSD_HEADER_ENCODED)
	{
		m_fileInterface.pfnFreeFile(pFile);
		return false;
	}

	if(pBSDHeader->version != BSD_FILE_VERSION)
	{
		m_fileInterface.pfnFreeFile(pFile);
		return false;
	}

	if(memcmp(pBSDHeader->hash, m_pCSDHeader->hash, sizeof(Char)*32))
	{
		m_fileInterface.pfnFreeFile(pFile);
		return false;
	}

	if(pBSDHeader->numshaders != m_pCSDHeader->numshaders)
	{
		m_fileInterface.pfnFreeFile(pFile);
		return false;
	}

	// Verify all shader formats
	GLint nbBinaryFormats = 0;
	glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &nbBinaryFormats);

	// Shouldn't happen
	if(nbBinaryFormats <= 0)
	{
		m_fileInterface.pfnFreeFile(pFile);
		return false;
	}

	CArray<GLint> binaryFormats(nbBinaryFormats);
	glGetIntegerv(GL_PROGRAM_BINARY_FORMATS, &binaryFormats[0]);

	GLint iStatus = 0;
	const shader_binary_t* pbinaryshaders = reinterpret_cast<const shader_binary_t*>(reinterpret_cast<const byte*>(pBSDHeader) + pBSDHeader->shaderoffset);
	for(Uint32 i = 0; i < pBSDHeader->numshaders; i++)
	{
		// Do not compile disabled states
		if(!m_disabledStatesArray.empty())
		{
			Uint32 j = 0;
			for(; j < m_disabledStatesArray.size(); j++)
			{
				const disabled_state_t& state = m_disabledStatesArray[j];
				Int32 value = m_determinatorArray[state.dt_index].values[i];
				if( state.dt_setting == value )
					break;
			}

			if(j != m_disabledStatesArray.size())
				continue;
		}

		const shader_binary_t* pshaderinfo = &pbinaryshaders[i];
		
		Uint32 j = 0;
		for(; j < nbBinaryFormats; j++)
		{
			if((GLint)pshaderinfo->binaryformat == binaryFormats[j])
				break;
		}

		if(j == (Uint32)nbBinaryFormats)
		{
			m_fileInterface.pfnFreeFile(pFile);
			return false;
		}
	}

	Int32 promptSpacing = 1;
	if(pBSDHeader->numshaders > 16)
		promptSpacing = 16;
	else if(pBSDHeader->numshaders > 8)
		promptSpacing = 8;

	Int32 lastPrompt = 0;
	for(Uint32 i = 0; i < pBSDHeader->numshaders; i++)
	{
		// Do not compile disabled states
		if(!m_disabledStatesArray.empty())
		{
			Uint32 j = 0;
			for(; j < m_disabledStatesArray.size(); j++)
			{
				const disabled_state_t& state = m_disabledStatesArray[j];
				Int32 value = m_determinatorArray[state.dt_index].values[i];
				if( state.dt_setting == value )
					break;
			}

			if(j != m_disabledStatesArray.size())
				continue;
		}

		const shader_binary_t* pshaderinfo = &pbinaryshaders[i];
		const byte* pshaderdata = reinterpret_cast<const byte*>(pBSDHeader) + pshaderinfo->dataoffset;

		m_shadersArray[i].program_id = m_glExtF.glCreateProgram();
		m_glExtF.glProgramBinary(m_shadersArray[i].program_id, pshaderinfo->binaryformat, pshaderdata, pshaderinfo->datasize);
		m_glExtF.glGetProgramiv(m_shadersArray[i].program_id, GL_LINK_STATUS, &iStatus);
		if(iStatus != GL_TRUE)
		{
			for(Uint32 j = 0; j <= i; j++)
			{
				m_glExtF.glDeleteProgram(m_shadersArray[j].program_id);
				m_shadersArray[j].compiled = false;
			}

			break;
		}
		else
		{
			// Mark as succeeded
			m_shadersArray[i].compiled = true;

			// Update client about or progress if needed
			if(m_pProgressUpdateCallbackFn)
			{
				if(i - lastPrompt > promptSpacing || i == (pBSDHeader->numshaders-1) || i == 0)
				{
					m_pProgressUpdateCallbackFn(m_shaderFile.c_str(), pBSDHeader->numshaders, i+1, false);
					lastPrompt = i;
				}
			}
		}
	}

	m_fileInterface.pfnFreeFile(pFile);
	return (iStatus == GL_TRUE) ? true : false;
}

//=============================================
// @brief Compiles all CSD shaders
//
// @return TRUE if no errors occurred, FLASE otherwise
//=============================================
bool CGLSLShader::CompileCSDShaderData( void )
{
	CBuffer* pbuffer = nullptr;
	shader_binary_t* pbinaryshaders = nullptr;
	bsd_header_t* pbsdheader = nullptr;

	if(m_useBinaryShaders)
	{
		// Create buffer object
		pbuffer = new CBuffer(sizeof(byte)*TEMP_FILE_BUFFER_SIZE);
		pbsdheader = reinterpret_cast<bsd_header_t*>(pbuffer->getbufferdata());
		pbuffer->addpointer((void**)&pbsdheader);
		pbuffer->append(nullptr, sizeof(bsd_header_t));

		pbsdheader->id = BSD_HEADER_ENCODED;
		pbsdheader->version = BSD_FILE_VERSION;
		pbsdheader->numshaders = m_pCSDHeader->numshaders;
		pbsdheader->shaderoffset = pbuffer->getsize();
		qstrcpy(pbsdheader->hash, m_pCSDHeader->hash);

		// Get ptr to binary shaders
		Uint32 datasize = sizeof(shader_binary_t) * pbsdheader->numshaders;
		pbuffer->append(nullptr, datasize);
		pbinaryshaders = reinterpret_cast<shader_binary_t*>(reinterpret_cast<byte*>(pbsdheader) + pbsdheader->shaderoffset);
		pbuffer->addpointer((void**)&pbinaryshaders);
	}

	Int32 promptSpacing = 1;
	if(m_shadersArray.size() > 16)
		promptSpacing = 16;
	else if(m_shadersArray.size() > 8)
		promptSpacing = 8;

	Int32 lastPrompt = 0;

	// Always compile all shaders if the CSD was changed, to find any possible errors
	csdshaderdata_t* pinshaders = reinterpret_cast<csdshaderdata_t*>(reinterpret_cast<byte*>(m_pCSDHeader) + m_pCSDHeader->shaderdataoffset);
	for(Uint32 i = 0; i < m_shadersArray.size(); i++)
	{
		// Do not compile disabled states
		if(!m_disabledStatesArray.empty())
		{
			Uint32 j = 0;
			for(; j < m_disabledStatesArray.size(); j++)
			{
				const disabled_state_t& state = m_disabledStatesArray[j];
				Int32 value = m_determinatorArray[state.dt_index].values[i];
				if( state.dt_setting == value )
					break;
			}

			if(j != m_disabledStatesArray.size())
				continue;
		}

		if(!CompileShader(i, &m_shadersArray[i], &pinshaders[i]))
			return false;

		// Extract binary data
		if(pbinaryshaders)
		{
			shader_binary_t* pbinaryshader = &pbinaryshaders[i];
			pbuffer->addpointer((void**)&pbinaryshader);

			// Get binary data size
			GLint programSize = 0;
			m_glExtF.glGetProgramiv(m_shadersArray[i].program_id, GL_PROGRAM_BINARY_LENGTH, &programSize);

			pbinaryshader->dataoffset = pbuffer->getsize();
			pbinaryshader->datasize = programSize;

			pbuffer->append(nullptr, pbinaryshader->datasize);
			void *pbinarydest = reinterpret_cast<byte*>(pbsdheader) + pbinaryshader->dataoffset;
			m_glExtF.glGetProgramBinary(m_shadersArray[i].program_id, programSize, nullptr, &pbinaryshader->binaryformat, pbinarydest);

			pbuffer->removepointer((const void**)&pbinaryshader);
		}

		// Update client about or progress if needed
		if(m_pProgressUpdateCallbackFn)
		{
			if(i - lastPrompt > promptSpacing || i == (m_shadersArray.size()-1))
			{
				m_pProgressUpdateCallbackFn(m_shaderFile.c_str(), m_shadersArray.size(), i+1, true);
				lastPrompt = i;
			}
		}
	}

	if(m_useBinaryShaders)
	{
		// Set final stuff
		pbsdheader->size = pbuffer->getsize();

		CString basename;
		Common::Basename(m_shaderFile.c_str(), basename);

		// Save binary data to disk
		CString folderPath;
		folderPath << "scripts/shaders/";

#ifdef _64BUILD
		folderPath << "binary_x64/";
#else
		folderPath << "binary_x86/";
#endif
		if(!m_fileInterface.pfnCreateDirectory(folderPath.c_str()))
		{
			m_errorString << "Failed to create directory " << folderPath;
			delete[] pbsdheader;
			return false;
		}

		CString filePath;
		filePath << folderPath << basename << ".bsd";
		if(!m_fileInterface.pfnWriteFile(reinterpret_cast<byte*>(pbsdheader), pbsdheader->size, filePath.c_str(), false))
		{
			m_errorString << "Failed to open " << filePath << " for writing";
			delete[] pbsdheader;
			return false;
		}

		delete[] pbsdheader;
	}

	return true;
}

//=============================================
// @brief Reads in the script entry for a determinator
//
// @param ppstr Pointer to the current character's pointer
// @return TRUE if no errors occurred, FLASE otherwise
//=============================================
bool CGLSLShader::ReadDeterminator( const Char** ppstr )
{
	// Extract the token name
	CString szdttoken;
	const Char* pscan = Common::Parse((*ppstr), szdttoken);
	if(szdttoken[0] == '$')
	{
		m_errorString << "Unexpected " << szdttoken << " in " << m_shaderFile;
		return false;
	}

	glsl_determinator_t *pdeterminator = AddDeterminator(szdttoken.c_str());
	if(!pdeterminator)
		return false;

	pdeterminator->type = DETERMINATOR_SHARED;
			
	// Parse the min value
	pscan = Common::Parse(pscan, szdttoken);
	if(!Common::IsNumber(szdttoken))
	{
		m_errorString << "Expected an integer number, got " << szdttoken << " in " << m_shaderFile;
		return false;
	}

	pdeterminator->minval = SDL_atoi(szdttoken.c_str());

	// Parse the max value
	pscan = Common::Parse(pscan, szdttoken);
	if(!Common::IsNumber(szdttoken))
	{
		m_errorString << "Expected an integer number, got " << szdttoken << " in " << m_shaderFile;
		return false;
	}

	pdeterminator->maxval = SDL_atoi(szdttoken.c_str());

	// Change the pointer
	(*ppstr) = pscan;
	return true;
}

//=============================================
// @brief Reads in the script entry for invalid states
//
// @param ppstr Pointer to the current character's pointer
// @return TRUE if no errors occurred, FLASE otherwise
//=============================================
bool CGLSLShader :: ReadInvalidState( const Char** ppstr )
{
	CString sztoken;
	invalid_state_t newState;
	
	// Scan the next token
	const Char* pscan = Common::Parse((*ppstr), sztoken);
	if(!Common::IsNumber(sztoken))
	{
		m_errorString << "Expected an integer number, got " << sztoken << " in " << m_shaderFile;
		return false;
	}

	newState.numdts = SDL_atoi(sztoken.c_str());

	// Parse the relevant determinators
	for(Uint32 i = 0; i < newState.numdts; i++)
	{
		pscan = Common::Parse(pscan, sztoken);

		// Try to find this determinator
		Uint32 index = GetDeterminatorIndex(sztoken.c_str());
		if(index == DETERMINATOR_UNDEFINED)
		{
			m_errorString << "Unable to find " << sztoken << " in determinator list of shader " << m_shaderFile 
				<< " for an invalid state. Make sure to put invalid state declarations after determinator declarations.";
			return false;
		}
		else
			newState.dt_indexes[i] = index;

		// scan min value
		pscan = Common::Parse(pscan, sztoken);
		if(!Common::IsNumber(sztoken))
		{
			m_errorString << "Expected an integer number, got " << sztoken << " in " << m_shaderFile;
			return false;
		}

		newState.dt_minrange[i] = SDL_atoi(sztoken.c_str());

		// scan max value
		pscan = Common::Parse(pscan, sztoken);
		if(!Common::IsNumber(sztoken))
		{
			m_errorString << "Expected an integer number, got " << sztoken << " in " << m_shaderFile;
			return false;
		}

		newState.dt_maxrange[i] = SDL_atoi(sztoken.c_str());
	}

	// Add it to the list
	m_invalidStatesArray.push_back(newState);

	// Change the pointer
	(*ppstr) = pscan;
	return true;
}

//=============================================
// @brief Compiles the shader from a script
//
// @return TRUE if successful, FLASE otherwise
//=============================================
bool CGLSLShader :: CompileFromScript( void )
{
	CString filePath;
	if(!m_rootDirectory.empty())
		filePath << m_rootDirectory << PATH_SLASH_CHAR;

	filePath << "scripts/shaders/" << m_shaderFile;

	Uint32 isize = 0;
	const Char *pfile = reinterpret_cast<const Char*>(m_fileInterface.pfnLoadFile(filePath.c_str(), &isize));
	if(!pfile)
	{
		m_errorString << "Error: Failed to load shader script " << filePath << ".";
		return false;
	}

	// Allocate the temporary structures
	m_pVertexScript = new shader_script_t();
	m_pFragmentScript = new shader_script_t();

	// Set data types
	m_pVertexScript->type = SHADER_VERTEX;
	m_pFragmentScript->type = SHADER_FRAGMENT;

	// Parse the script data
	CString sztoken;
	const Char *pscan = pfile;
	while((pscan-pfile) < isize && pscan)
	{
		// Seek out global determinators
		while(1)
		{
			pscan = Common::Parse(pscan, sztoken);
			if(!pscan)
				break;

			if(qstrcmp("$global_determinator", sztoken))
				break;

			if(!ReadDeterminator(&pscan))
			{
				m_fileInterface.pfnFreeFile(pfile);
				return false;
			}
		}

		// Seek out invalid states
		while(1)
		{
			if(qstrcmp("$invalid_state", sztoken))
				break;

			if(!ReadInvalidState(&pscan))
			{
				m_fileInterface.pfnFreeFile(pfile);
				return false;
			}

			// parse next token
			pscan = Common::Parse(pscan, sztoken);
		}

		// See if we hit the end
		if(!qstrncmp("$end", sztoken, 4))
			break;

		// Determine shader type
		shader_script_t *pscript = nullptr;
		if(!qstrcmp("$shader_vertex", sztoken))
			pscript = m_pVertexScript;
		else if(!qstrcmp("$shader_fragment", sztoken))
			pscript = m_pFragmentScript;
		else
		{
			m_errorString << "Invalid token " << sztoken << ", was expecting either $shader_vertex or $shader_fragment in " << m_shaderFile;
			m_fileInterface.pfnFreeFile(pfile);
			return false;
		}

		// Parse the shader relevant stuff
		while(1)
		{
			pscan = Common::Parse(pscan, sztoken);
			if(!pscan)
			{
				m_errorString << "Unexpected EOF on " << m_shaderFile;
				m_fileInterface.pfnFreeFile(pfile);
				return false;
			}

			if(!qstrcmp("$begin", sztoken))
				break;

			if(!qstrcmp("$determinator", sztoken))
			{
				if(!ReadDeterminator(&pscan))
				{
					m_fileInterface.pfnFreeFile(pfile);
					return false;
				}
			}
			
			// Parse invalid states
			if(!qstrcmp("$invalid_state", sztoken))
			{
				if(!ReadInvalidState(&pscan))
				{
					m_fileInterface.pfnFreeFile(pfile);
					return false;
				}
			}
		}

		// Make sure the syntax is right
		if(qstrcmp("$begin", sztoken))
		{
			m_errorString << "Error: Invalid token, was expecting $begin in " << m_shaderFile;
			m_fileInterface.pfnFreeFile(pfile);
			return false;
		}

		// Break the script into chunks and sub-chunks
		if(!ReadChunks(&pscan, &pscript->pchunks, &pscript->numchunks, nullptr, 0))
		{
			m_fileInterface.pfnFreeFile(pfile);
			return false;
		}
	}

	// free contents
	bool result = ConstructBranches(pfile, isize);

	// Release file
	m_fileInterface.pfnFreeFile(pfile);

	// Perform the shader compile
	return result;
}

//=============================================
// @brief Compiles the shader from a given script
//
// @param szfile File path string
// @return TRUE if successful, FLASE otherwise
//=============================================
bool CGLSLShader :: Compile( const Char* szfile )
{
	m_shaderFile = szfile;

	if(!CompileFromCSD())
	{
		// Make sure this is disabled
		m_onDemandLoad = false;

		// Free any shader data we might've loaded
		FreeShaderData();

		if(!CompileFromScript())
		{
			// Failed, free everything
			FreeData();
			m_bFailed = true;
			return false;
		}

		// Free temporary structures
		FreeTempData();
	}

	// Free the CSD if we're not on-demand loading
	if(m_pCSDHeader && !m_onDemandLoad)
	{
		delete[] m_pCSDHeader;
		m_pCSDHeader = nullptr;
	}

	return true;
}

//=============================================
// @brief Reads a single chunk, and any child chunks from the script
//
// @param ppscan Pointer to current character in the script
// @param pchunkptr Pointer to chunk array
// @return TRUE if successful, FLASE otherwise
//=============================================
bool CGLSLShader::ReadChunks( const Char **ppscan, shader_chunk_t** pchunkptr, Uint32* numchunkptr, glsl_branchcondition_t* pconditionals, Uint32 numconditionals )
{
	CString szToken1;
	CString szToken2;
	CString szToken3;

	Uint32 chunkSize = 0;
	constexpr Uint32 chunkMaxSize = 65535;
	Char *pChunkBuffer = new Char[chunkMaxSize]();
	if(!pChunkBuffer)
	{
		m_errorString << "Failed to allocate " << (Int32)chunkMaxSize << " bytes for " << m_shaderFile;
		return false;
	}

	// Allocate the new chunk
	(*pchunkptr) = static_cast<shader_chunk_t*>(Common::ResizeArray((*pchunkptr), sizeof(shader_chunk_t), (*numchunkptr)));
	shader_chunk_t* pchunk = &(*pchunkptr)[(*numchunkptr)];
	(*numchunkptr)++;

	// Add conditionals if any
	if(pconditionals && numconditionals)
	{
		pchunk->pconditionals = new glsl_branchcondition_t[numconditionals];
		memcpy(pchunk->pconditionals, pconditionals, sizeof(glsl_branchcondition_t)*numconditionals);
		pchunk->numconditionals = numconditionals;
	}

	// Read the chunk in byte by byte
	while(TRUE)
	{
		if((**ppscan) == '$')
		{
			// Read the special token in
			(*ppscan) = Common::Parse((*ppscan), szToken1);
			
			// If it's a branch token, read in the conditionals
			if(!qstrcmp(szToken1, "$branch"))
			{
				// Conditionals to pass to the chunk
				glsl_branchcondition_t* pchunkconditionals = nullptr;
				Uint32 numchunkconditionals = 0;

				// determines how we relate to a previous conditional
				operator_e lastoperator = OPERATOR_NONE;

				while(TRUE)
				{
					// Read the determinator name
					(*ppscan) = Common::Parse((*ppscan), szToken1);
					if(!(*ppscan))
					{
						m_errorString << "Unexpected EOF in " << m_shaderFile;
						if(pchunkconditionals)
							delete[] pchunkconditionals;
						delete[] pChunkBuffer;
						return false;
					}

					// We need to use our own parser for this
					szToken2.clear();
					{
						while(isspace((**ppscan)) && (**ppscan))
							(*ppscan) = (*ppscan + 1);

						// Read the token in
						while(!isspace(**ppscan) && (**ppscan))
						{
							szToken2 += (**ppscan);
							(*ppscan) = (*ppscan + 1);
						}
					}

					// Read the determinator conditional value
					(*ppscan) = Common::Parse((*ppscan), szToken3);
					if(!(*ppscan))
					{
						m_errorString << "Unexpected EOF on " << m_shaderFile;
						if(pchunkconditionals)
							delete[] pchunkconditionals;
						delete[] pChunkBuffer;
						return false;
					}

					// Allocate the new conditional
					pchunkconditionals = static_cast<glsl_branchcondition_t*>(Common::ResizeArray(pchunkconditionals, sizeof(glsl_branchcondition_t), numchunkconditionals));
					glsl_branchcondition_t *pchunkconditional = &pchunkconditionals[numchunkconditionals];
					numchunkconditionals++;

					// Set the operator
					pchunkconditional->boperator = lastoperator;

					// Find the determinator
					Uint32 i = 0;
					for(; i < m_determinatorArray.size(); i++)
					{
						if(!qstrcmp(szToken1, m_determinatorArray[i].name))
						{
							pchunkconditional->determinatorindex = i;
							break;
						}
					}

					if(i == m_determinatorArray.size())
					{
						m_errorString << "Determinator " << szToken1 << " not found in listing for " << m_shaderFile;
						if(pchunkconditionals)
							delete[] pchunkconditionals;
						delete[] pChunkBuffer;
						return false;
					}

					// Set the conditional type
					if(!qstrcmp(szToken2, "=="))
						pchunkconditional->bcomparison = BRANCH_EQUAL;
					else if(!qstrcmp(szToken2, ">="))
						pchunkconditional->bcomparison = BRANCH_GREATER_OR_EQUAL;
					else if(!qstrcmp(szToken2, "<="))
						pchunkconditional->bcomparison = BRANCH_LESS_OR_EQUAL;
					else if(!qstrcmp(szToken2, ">"))
						pchunkconditional->bcomparison = BRANCH_GREATER;
					else if(!qstrcmp(szToken2, "<"))
						pchunkconditional->bcomparison = BRANCH_LESS;
					else if(!qstrcmp(szToken2, "!="))
						pchunkconditional->bcomparison = BRANCH_NOT_EQUAL;
					else
					{
						m_errorString << "Unrecognized comparison: " << szToken2;
						if(pchunkconditionals)
							delete[] pchunkconditionals;
						delete[] pChunkBuffer;
						return false;
					}

					// Assign the value
					pchunkconditional->value = SDL_atoi(szToken3.c_str());

					// Read the next token, but don't check for EOF errors
					(*ppscan) = Common::Parse((*ppscan), szToken1);

					// See if it's an operator token
					if(!qstrcmp(szToken1, "||"))
						lastoperator = OPERATOR_ELSE;
					else if(!qstrcmp(szToken1, "&&"))
						lastoperator = OPERATOR_AND;
					else
						break;
				}

				// Only a $begin token should be valid at this point
				if(qstrcmp(szToken1, "$begin"))
				{
					m_errorString << "Unexpected " << szToken1 << " token for $branch in " << m_shaderFile << ", $begin expected.";
					if(pchunkconditionals)
						delete[] pchunkconditionals;
					delete[] pChunkBuffer;
					return false;
				}

				// Read the contents into a sub-chunk
				if(!ReadChunks(ppscan, &pchunk->pchunks, &pchunk->numchunks, pchunkconditionals, numchunkconditionals))
				{
					if(pchunkconditionals)
						delete[] pchunkconditionals;
					delete[] pChunkBuffer;
					return false;
				}

				// Copy the data to the final buffer
				pchunk->isize = chunkSize;
				pchunk->pdata = new Char[chunkSize+1];
				memcpy(pchunk->pdata, pChunkBuffer, chunkSize);

				// Null terminate the string
				pchunk->pdata[pchunk->isize] = '\0';
				chunkSize = 0;

				// Free the conditionals array
				delete[] pchunkconditionals;

				// Allocate the new chunk, as parts coming after a sub chunk are a new chunk
				(*pchunkptr) = static_cast<shader_chunk_t*>(Common::ResizeArray((*pchunkptr), sizeof(shader_chunk_t), (*numchunkptr)));
				pchunk = &(*pchunkptr)[(*numchunkptr)];
				(*numchunkptr)++;

				// Add the conditionals if any were given
				if(pconditionals && numconditionals)
				{
					pchunk->pconditionals = new glsl_branchcondition_t[numconditionals];
					memcpy(pchunk->pconditionals, pconditionals, sizeof(glsl_branchcondition_t)*numconditionals);
					pchunk->numconditionals = numconditionals;
				}

				// Start from the beginning
				continue;
			}
			else if(!qstrcmp(szToken1, "$end"))
			{
				// End of chunk
				break;
			}
			else
			{
				m_errorString << "Unexpected " << szToken1 << " token for $branch in " << m_shaderFile;
				delete[] pChunkBuffer;
				return false;
			}
		}

		if(chunkSize == chunkMaxSize)
		{
			m_errorString << "Shader script " << m_shaderFile << " << too long, max is " << (Int32)chunkMaxSize << " characters";
			delete[] pChunkBuffer;
			return false;
		}

		// Read the script contents in char by char
		pChunkBuffer[chunkSize] = (**ppscan);
		(*ppscan) = (*ppscan + 1);
		chunkSize++;
	}

	// Copy the data to the final buffer
	pchunk->isize = chunkSize;
	pchunk->pdata = new Char[chunkSize+1];
	memcpy(pchunk->pdata, pChunkBuffer, chunkSize);

	// Null terminate the string
	pchunk->pdata[pchunk->isize] = '\0';
	delete[] pChunkBuffer;

	return true;
}

//=============================================
// @brief Recursively fills out all the possible determinator value combinations
//
// @param index Determinator index
// @param numShaders Reference to variable to hold the number of shaders
// @return Number of unique shaders
//=============================================
void CGLSLShader::RecursiveFillValues( Uint32 index, Uint32& numShaders )
{
	if(index == m_determinatorArray.size())
		return;

	glsl_determinator_t *pdt = &m_determinatorArray[index];
	for(Int32 i = pdt->minval; i < (pdt->maxval+1); i++)
	{
		// Set base value
		m_pDeterminatorValues[index] = i;

		// If we're at the very end, we've got to store it
		if(index == (m_determinatorArray.size()-1))
		{
			if(!m_invalidStatesArray.empty())
			{
				Uint32 j = 0;
				for(; j < m_invalidStatesArray.size(); j++)
				{
					Uint32 k = 0;
					for(; k < m_invalidStatesArray[j].numdts; k++)
					{
						invalid_state_t *pstate = &m_invalidStatesArray[j];
						Int32 value = m_pDeterminatorValues[pstate->dt_indexes[k]];

						if(value < pstate->dt_minrange[k] || value > pstate->dt_maxrange[k])
							break;
					}

					if(k == m_invalidStatesArray[j].numdts)
						break;
				}

				if(j != m_invalidStatesArray.size())
					continue;
			}

			for(Uint32 j = 0; j < m_determinatorArray.size(); j++)
				m_determinatorArray[j].values.push_back(m_pDeterminatorValues[j]);

			numShaders++;
		}

		// Recurse down the next
		RecursiveFillValues(index+1, numShaders);
	}

	// Reset this
	m_pDeterminatorValues[index] = pdt->minval;
}

//=============================================
// @brief Determines if a chunk should be included in a shader setup
//
// @param id Index of the shader to compile
// @param pchunk Pointer to the chunk object
// @return TRUE if the chuck should be added, FALSE otherwise
//=============================================
bool CGLSLShader::ShouldIncludeChunk( Uint32 id, shader_chunk_t *pchunk )
{
	if(!pchunk)
		return false;

	if(!pchunk->numconditionals)
		return true;

	bool bResult = false;
	for(Uint32 i = 0; i < pchunk->numconditionals; i++)
	{
		bool bComparisonResult = true;
		glsl_branchcondition_t* pconditional = &pchunk->pconditionals[i];
		glsl_determinator_t *pdt = &m_determinatorArray[pconditional->determinatorindex];

		if(pconditional->bcomparison == BRANCH_EQUAL)
		{
			if(pdt->values[id] != pconditional->value)
				bComparisonResult = false;
		}
		else if(pconditional->bcomparison == BRANCH_GREATER)
		{
			if(pdt->values[id] <= pconditional->value)
				bComparisonResult = false;
		}
		else if(pconditional->bcomparison == BRANCH_GREATER_OR_EQUAL)
		{
			if(pdt->values[id] < pconditional->value)
				bComparisonResult = false;
		}
		else if(pconditional->bcomparison == BRANCH_LESS)
		{
			if(pdt->values[id] >= pconditional->value)
				bComparisonResult = false;
		}
		else if(pconditional->bcomparison == BRANCH_LESS_OR_EQUAL)
		{
			if(pdt->values[id] > pconditional->value)
				bComparisonResult = false;
		}
		else if(pconditional->bcomparison == BRANCH_NOT_EQUAL)
		{
			if(pdt->values[id] == pconditional->value)
				bComparisonResult = false;
		}

		if(pconditional->boperator == OPERATOR_NONE && bComparisonResult
			|| pconditional->boperator == OPERATOR_AND && bResult && bResult == bComparisonResult
			|| pconditional->boperator == OPERATOR_ELSE && (bComparisonResult || bResult))
			bResult = true;
	}

	return bResult;
}

//=============================================
// @brief Recursively adds all the required chunks for a shader
//
// @param id Index of the shader to compile
// @param pchunk Pointer to the chunk object
// @param pstrbuf Pointer to the destination buffer
// @param size Pointer to variable holding the buffer load
// @param maxBufferSize Maximum size of the buffer
//=============================================
bool CGLSLShader::RecursiveAddChunks( Uint32 id, shader_chunk_t* pchunk, Char* pstrbuf, Uint32* size, Uint32 maxBufferSize )
{
	if((*size) + pchunk->isize >= maxBufferSize)
	{
		m_errorString << "Exceeded maxBufferSize(" << (Int32)maxBufferSize << ") in shader " << m_shaderFile;
		return false;
	}

	// Add the chunk's data to the destination buffer
	Char* pdestptr = pstrbuf + (*size);
	memcpy(pdestptr, pchunk->pdata, pchunk->isize);
	(*size) += pchunk->isize;

	// Add any sub-chunks
	for(Uint32 i = 0; i < pchunk->numchunks; i++)
	{
		if(!ShouldIncludeChunk(id, &pchunk->pchunks[i]))
			continue;

		if(!RecursiveAddChunks(id, &pchunk->pchunks[i], pstrbuf, size, maxBufferSize))
			return false;
	}

	return true;
}

//=============================================
// @brief Splices the scripts together for a shader
//
// @param id Index of the shader to compile
// @param vsptr Pointer to pointer to hold the vertex shader data
// @param fsptr Pointer to pointer to hold the fragment shader data
//=============================================
bool CGLSLShader::SpliceScripts( Uint32 id, Char **vsptr, Char **fsptr )
{
	// Buffer to write to
	constexpr Uint32 maxBufferSize = 131072;
	Char *pstrbuffer = new Char[maxBufferSize];
	Uint32 buffersize = 0;

	// Splice vertex script data
	for(Uint32 i = 0; i < m_pVertexScript->numchunks; i++)
	{
		if(!ShouldIncludeChunk(id, &m_pVertexScript->pchunks[i]))
			continue;

		if(!RecursiveAddChunks(id, &m_pVertexScript->pchunks[i], pstrbuffer, &buffersize, maxBufferSize))
			return false;
	}

	// Allocate the final buffer
	(*vsptr) = new Char[buffersize+1];
	memcpy((*vsptr), pstrbuffer, sizeof(Char)*buffersize);
	(*vsptr)[buffersize] = '\0';

	// Reset this
	buffersize = 0;

	// Splice fragment script data
	for(Uint32 i = 0; i < m_pFragmentScript->numchunks; i++)
	{
		if(!ShouldIncludeChunk(id, &m_pFragmentScript->pchunks[i]))
			continue;

		if(!RecursiveAddChunks(id, &m_pFragmentScript->pchunks[i], pstrbuffer, &buffersize, maxBufferSize))
			return false;
	}

	// Allocate the final buffer
	(*fsptr) = new Char[buffersize+1];
	memcpy((*fsptr), pstrbuffer, sizeof(Char)*buffersize);
	(*fsptr)[buffersize] = '\0';

	// Free the temporary buffer
	delete[] pstrbuffer;
	return true;
}

//=============================================
// @brief Adds a disabled state to the GLSL shader
//
// @param pstrName Name of the determinator to disable a state for
// @param state Value to be disabled on the determinator
//=============================================
void CGLSLShader::DisableDeterminatorState( const Char *pstrName, Int32 state )
{
	for(Uint32 i = 0; i < m_disabledStatesArray.size(); i++)
	{
		if(!qstrcmp(m_disabledStatesArray[i].dt_name, pstrName) 
			&& m_disabledStatesArray[i].dt_setting == state)
			return;
	}

	disabled_state_t newState;
	newState.dt_name = pstrName;
	newState.dt_setting = state;

	m_disabledStatesArray.push_back(newState);
}

//=============================================
// @brief Tells the total number of shader variations
//
//=============================================
Uint32 CGLSLShader::GetNbTotalShaderVariations( void ) const
{
	if(m_pCSDHeader)
		return m_pCSDHeader->numshaders;
	else
		return 0;
}

//=============================================
// @brief Tells if a specific shader variation has been compiled
//
//=============================================
bool CGLSLShader::IsShaderVariationCompiled( Uint32 index )
{
	if(!m_pCSDHeader)
		return false;

	return m_shadersArray[index].compiled;
}

//=============================================
// @brief Compiles a specific shader variation
//
//=============================================
bool CGLSLShader::CompileShaderVariation( Uint32 index )
{
	if(!m_pCSDHeader)
		return false;

	// Compile the shader
	csdshaderdata_t* pshaderdata = reinterpret_cast<csdshaderdata_t*>(reinterpret_cast<byte *>(m_pCSDHeader) + m_pCSDHeader->shaderdataoffset) + index;
	if(!CompileShader(index, &m_shadersArray[index], pshaderdata))
	{
		m_bFailed = true;
		return false;
	}

	// Find uniforms in this shader
	for(Uint32 i = 0; i < m_uniformsArray.size(); i++)
	{
		m_uniformsArray[i].indexes[index] = m_glExtF.glGetUniformLocation(m_shadersArray[index].program_id, m_uniformsArray[i].name.c_str());
		if(m_uniformsArray[i].indexes[index] == -1)
			m_uniformsArray[i].indexes[index] = PROPERTY_UNAVAILABLE;
		else
			assert(m_uniformsArray[i].indexes[index] >= 0);
	}

	// Find uniform blocks in this shader
	for(Uint32 i = 0; i < m_uniformBufferObjectsArray.size(); i++)
	{
		m_uniformBufferObjectsArray[i].blockindexes[index] = m_glExtF.glGetUniformBlockIndex(m_shadersArray[index].program_id, m_uniformBufferObjectsArray[i].name.c_str());
		if(m_uniformBufferObjectsArray[i].blockindexes[index] == -1)
			m_uniformBufferObjectsArray[i].blockindexes[index] = PROPERTY_UNAVAILABLE;
		else
			assert(m_uniformBufferObjectsArray[i].blockindexes[index] >= 0);
	}

	// Find attributes in the shader
	for(Uint32 i = 0; i < m_vertexAttribsArray.size(); i++)
	{
		m_vertexAttribsArray[i].indexes[index] = m_glExtF.glGetAttribLocation(m_shadersArray[index].program_id, m_vertexAttribsArray[i].name.c_str());
		if(m_vertexAttribsArray[i].indexes[index] == -1)
			m_vertexAttribsArray[i].indexes[index] = PROPERTY_UNAVAILABLE;
		else
			assert(m_vertexAttribsArray[i].indexes[index] >= 0);
	}

	return true;
}

//=============================================
// @brief Constructs all possible shader setups
//
//=============================================
bool CGLSLShader::ConstructBranches ( const Char* pSrc, Uint32 fileSize )
{
	// Determine all possible values
	Uint32 nbShaders = 1;
	if(!m_determinatorArray.empty())
	{
		m_pDeterminatorValues = new Int32[m_determinatorArray.size()];
		memset(m_pDeterminatorValues, 0, sizeof(Int32)*m_determinatorArray.size());

		nbShaders = 0;
		RecursiveFillValues(0, nbShaders);
	}

	if(nbShaders >= MAX_VARIATIONS)
	{
		m_errorString << CString("nbShaders > ") << (Int32)MAX_VARIATIONS << " in " << m_shaderFile;
		return false;
	}

	// Get the temp buffer for writing the file
	CBuffer csdBuffer(TEMP_FILE_BUFFER_SIZE);
	csdheader_t* pheader = reinterpret_cast<csdheader_t*>(csdBuffer.getbufferdata());
	csdBuffer.addpointer((void**)&pheader);

	// Set values
	csdBuffer.append(nullptr, sizeof(csdheader_t));
	pheader->id = CSD_HEADER_ENCODED;
	pheader->version = CSD_FILE_VERSION;

	CMD5 hash(reinterpret_cast<const byte*>(pSrc), fileSize);
	CString hashStr = hash.HexDigest();

	qstrcpy(pheader->hash, hashStr.c_str());

	// Allocate shaders
	m_shadersArray.resize(nbShaders);

	// Allocate in the output file too
	pheader->shaderdataoffset = csdBuffer.getsize();
	pheader->numshaders = nbShaders;
	csdBuffer.append(nullptr, sizeof(csdshaderdata_t)*nbShaders);

	csdshaderdata_t* poutshaders = reinterpret_cast<csdshaderdata_t*>(reinterpret_cast<byte*>(pheader) + pheader->shaderdataoffset);
	csdBuffer.addpointer((void**)&poutshaders);

	for(Uint32 i = 0; i < nbShaders; i++)
	{
		// Perform the script splices
		Char *vsscript, *fsscript;
		if(!SpliceScripts(i, &vsscript, &fsscript))
			return false;

		// Allocate in the output
		Uint32 vslength = qstrlen(vsscript);
		poutshaders[i].vertexdataoffs = csdBuffer.getsize();
		poutshaders[i].vertexdatasize = vslength;

		// Copy the vertex shader data
		csdBuffer.append(vsscript, sizeof(Char)*vslength);

		// Allocate in the output
		Uint32 fslength = qstrlen(fsscript);
		poutshaders[i].fragmentdataoffs = csdBuffer.getsize();
		poutshaders[i].fragmentdatasize = fslength;

		// Copy the vertex shader data
		csdBuffer.append(fsscript, sizeof(Char)*fslength);
		
		delete[] vsscript;
		delete[] fsscript;
	}

	// Save the determinator data
	if(!m_determinatorArray.empty())
	{
		pheader->determinatoroffset = csdBuffer.getsize();
		pheader->numdeterminators = m_determinatorArray.size();
		csdBuffer.append(nullptr, sizeof(csddeterminator_t)*pheader->numdeterminators);

		csddeterminator_t* pdeterminators = reinterpret_cast<csddeterminator_t*>(reinterpret_cast<byte *>(pheader) + pheader->determinatoroffset);
		csdBuffer.addpointer((void**)&pdeterminators);

		for(Uint32 i = 0; i < m_determinatorArray.size(); i++)
		{
			qstrcpy(pdeterminators[i].name, m_determinatorArray[i].name.c_str());
			pdeterminators[i].type = m_determinatorArray[i].type;
			pdeterminators[i].minval = m_determinatorArray[i].minval;
			pdeterminators[i].maxval = m_determinatorArray[i].maxval;

			// Save values
			pdeterminators[i].valuesoffset = csdBuffer.getsize();
			csdBuffer.append(&m_determinatorArray[i].values[0], sizeof(Int16)*nbShaders);
		}
	}

	// Set the final filesize
	pheader->size = csdBuffer.getsize();

	// Save the file to the output
	CString basename;
	Common::Basename(m_shaderFile.c_str(), basename);

	CString filePath;
	filePath << "scripts/shaders/" << basename << ".csd";

	if(!m_fileInterface.pfnWriteFile(reinterpret_cast<byte*>(pheader), csdBuffer.getsize(), filePath.c_str(), false))
	{
		m_errorString << "Failed to open " << filePath << " for writing";
		return false;
	}

	// Create the final CSD buffer
	m_pCSDHeader = reinterpret_cast<csdheader_t*>(new byte[pheader->size]);
	memcpy(m_pCSDHeader, pheader, pheader->size);

	// Link up disabled states to determinators
	for(Uint32 j = 0; j < m_disabledStatesArray.size(); j++)
	{
		disabled_state_t& state = m_disabledStatesArray[j];

		Uint32 k = 0;
		for(; k < m_determinatorArray.size(); k++)
		{
			if(!qstrcmp(state.dt_name, m_determinatorArray[k].name))
			{
				state.dt_index = k;
				break;
			}
		}

		if(k == m_determinatorArray.size())
		{
			m_errorString << "Unable to find determinator " << state.dt_name << " for disabled state in shader " << m_shaderFile;
			return false;
		}
	}

	return CompileCSDShaderData();
}

//=============================================
// @brief Prints the error log for a shader into a file
//
// @param shader_id GLSL id for the shader
// @param script Pointer to the GLSL script
// @param szoutpath Log file base name
//=============================================
void CGLSLShader::Shader_PrintLog ( GLuint shader_id, const Char *script, Uint32 length, const Char *szoutpath, bool dumpShaderCode )
{
	Int32 iLogSize = 0;
	m_glExtF.glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &iLogSize);

	if(iLogSize > 1)
	{
		Int32 iNumWritten;
		Char *pLog = new Char[iLogSize];
		m_glExtF.glGetShaderInfoLog(shader_id, iLogSize, &iNumWritten, pLog);

		Char *pScan = pLog;
		while(*pScan)
		{
			if(*pScan == '\n')
				iLogSize++;

			pScan++;
		}

		Char *pOut = new Char[iLogSize];
		Char *pDest = pOut;
		pScan = pLog;

		while(*pScan)
		{
			if(*pScan == '\n')
			{
				*pDest = '\r'; 
				pDest++;
			}

			*pDest = *pScan;
			pDest++; pScan++;
		}

		CString outputPath;
		outputPath << szoutpath << "_debug.txt";
		m_fileInterface.pfnWriteFile(reinterpret_cast<const byte *>(pOut), iLogSize-1, outputPath.c_str(), false);

		delete[] pLog;
		delete[] pOut;
	}

	if(dumpShaderCode)
	{
		CString outputPath;
		outputPath << szoutpath << "_source.txt";
		m_fileInterface.pfnWriteFile(reinterpret_cast<const byte *>(script), length, outputPath.c_str(), false);
	}
}

//=============================================
// @brief Prints the error log for an entire program into a log file
//
// @param program_id GLSL id for the program
// @param szoutpath Log file base name
//=============================================
void CGLSLShader :: Program_PrintLog ( GLuint program_id, const Char *szoutpath )
{
	Int32 iLogSize = 0;
	m_glExtF.glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &iLogSize);
	if(iLogSize <= 1)
		return;

	Int32 iNumWritten;
	Char *pLog = new Char[iLogSize];
	m_glExtF.glGetProgramInfoLog(program_id, iLogSize, &iNumWritten, pLog);

	Char *pScan = pLog;
	while(*pScan)
	{
		if(*pScan == '\n')
			iLogSize++;

		pScan++;
	}

	Char *pOut = new Char[iLogSize];
	Char *pDest = pOut;
	pScan = pLog;
	while(*pScan)
	{
		if(*pScan == '\n')
		{
			*pDest = '\r'; 
			pDest++;
		}

		*pDest = *pScan;
		pDest++; pScan++;
	}

	CString outputPath;
	outputPath << szoutpath << "_prog_debug.txt";
	m_fileInterface.pfnWriteFile(reinterpret_cast<byte *>(pOut), iLogSize-1, outputPath.c_str(), false);

	delete[] pLog;
	delete[] pOut;
}

//=============================================
// @brief Adds a new determinator to the array
//
// @param szname The name of the determinator
// @return Pointer to the new determinator
//=============================================
CGLSLShader::glsl_determinator_t *CGLSLShader::AddDeterminator ( const Char *szname )
{
	for(Uint32 i = 0; i < m_determinatorArray.size(); i++)
	{
		if(!qstrcmp(m_determinatorArray[i].name, szname))
			return &m_determinatorArray[i];
	}

	if(m_determinatorArray.size() == MAX_DETERMINATORS)
	{
		m_errorString << "Exceeded MAX_DETERMINATORS";
		return nullptr;
	}

	m_determinatorArray.resize(m_determinatorArray.size()+1);

	glsl_determinator_t* pnew = &m_determinatorArray[m_determinatorArray.size()-1];
	pnew->type = DETERMINATOR_UNDEFINED;
	pnew->name = szname;

	return pnew;
}

//=============================================
// @brief Binds the shader and sets the attributes, and uniform values
//
//=============================================
bool CGLSLShader::EnableShader ( void )
{
	// See if it's bound
	if(m_onDemandLoad && !m_shadersArray[m_shaderIndex].compiled)
	{
		if(!CompileShaderVariation(m_shaderIndex))
			return false;
	}

	m_glExtF.glUseProgram(m_shadersArray[m_shaderIndex].program_id);
	m_isActive = true;

	// Bind UBOs if any
	if(!m_areUBOsBound && !m_uniformBufferObjectsArray.empty())
	{
		for(Uint32 i = 0; i < m_uniformBufferObjectsArray.size(); i++)
		{
			glsl_ubo_t& ubo = m_uniformBufferObjectsArray[i];
			m_glExtF.glBindBufferBase(GL_UNIFORM_BUFFER, i+1, ubo.buffer_id);
		}

		m_areUBOsBound = true;
	}

	if(m_shaderIndex == m_lastIndex)
		return true;

	// resync attribs if previous shader was valid
	if(m_lastIndex != -1)
	{
		for(Uint32 i = 0; i < m_vertexAttribsArray.size(); i++)
		{
			glsl_attrib_t *pattrib = &m_vertexAttribsArray[i];

			if(pattrib->indexes[m_shaderIndex] == pattrib->indexes[m_lastIndex])
				continue;

			// Disable if not present here
			if(pattrib->indexes[m_shaderIndex] == PROPERTY_UNAVAILABLE)
			{
				m_pVBO->DisableAttribPointer(pattrib->indexes[m_lastIndex]);
			}
			else if(pattrib->indexes[m_lastIndex] != PROPERTY_UNAVAILABLE)
			{
				m_pVBO->DisableAttribPointer(pattrib->indexes[m_lastIndex]);
			}
		}
	}

	for(Uint32 i = 0; i < m_vertexAttribsArray.size(); i++)
	{
		glsl_attrib_t *pattrib = &m_vertexAttribsArray[i];

		if(m_lastIndex != -1 && pattrib->indexes[m_shaderIndex] == pattrib->indexes[m_lastIndex])
			continue;

		if(pattrib->indexes[m_shaderIndex] != PROPERTY_UNAVAILABLE && pattrib->active)
			m_pVBO->SetAttribPointer(pattrib->indexes[m_shaderIndex], pattrib->size, pattrib->type, pattrib->stride, pattrib->pointer);
	}

	// Resync values
	m_lastIndex = m_shaderIndex;

	for(Uint32 i = 0; i < m_uniformsArray.size(); i++)
	{
		glsl_uniform_t& uniform = m_uniformsArray[i];

		if(!uniform.sync)
			continue;

		if(uniform.type == UNIFORM_NOSYNC)
			continue;

		if(uniform.indexes[m_shaderIndex] == -1)
			continue;
		
		SyncUniform(uniform);
	}

	return true;
}

//=============================================
// @brief Synchronizes a shader uniform setting
//
//=============================================
void CGLSLShader::SyncUniform( glsl_uniform_t& uniform )
{
	Uint32 offset = uniform.stride * m_shaderIndex;
	Float* pshadervalue = &uniform.shadervalues[offset];
	Float* pcurrentvalue = &uniform.currentvalues[0];

	switch (uniform.type)
	{
		case UNIFORM_INT1:
		{
			if (memcmp(pshadervalue, pcurrentvalue, sizeof(Float) * uniform.stride) != 0)
			{
				m_glExtF.glUniform1i(uniform.indexes[m_shaderIndex], (GLint)(*pcurrentvalue));
				memcpy(pshadervalue, pcurrentvalue, sizeof(Float) * uniform.stride);
			}
		}
	break;
		case UNIFORM_FLOAT1:
		{
			if (memcmp(pshadervalue, pcurrentvalue, sizeof(Float) * uniform.stride) != 0)
			{
				m_glExtF.glUniform1f(uniform.indexes[m_shaderIndex], (*pcurrentvalue));
				memcpy(pshadervalue, pcurrentvalue, sizeof(Float) * uniform.stride);
			}
		}
	break;
		case UNIFORM_FLOAT2:
		{
			if (memcmp(pshadervalue, pcurrentvalue, sizeof(Float) * uniform.stride) != 0)
			{
				m_glExtF.glUniform2fv(uniform.indexes[m_shaderIndex], 1, pcurrentvalue);
				memcpy(pshadervalue, pcurrentvalue, sizeof(Float) * uniform.stride);
			}
		}
	break;
		case UNIFORM_FLOAT3:
		{
			if (memcmp(pshadervalue, pcurrentvalue, sizeof(Float) * uniform.stride) != 0)
			{
				m_glExtF.glUniform3fv(uniform.indexes[m_shaderIndex], 1, pcurrentvalue);
				memcpy(pshadervalue, pcurrentvalue, sizeof(Float) * uniform.stride);
			}
		}
	break;
		case UNIFORM_FLOAT4:
		{
			if (memcmp(pshadervalue, pcurrentvalue, sizeof(Float) * uniform.stride) != 0)
			{
				m_glExtF.glUniform4fv(uniform.indexes[m_shaderIndex], 1, pcurrentvalue);
				memcpy(pshadervalue, pcurrentvalue, sizeof(Float) * uniform.stride);
			}
		}
	break;
		case UNIFORM_MATRIX4:
		{
			if (memcmp(pshadervalue, pcurrentvalue, sizeof(Float) * uniform.stride) != 0)
			{
				m_glExtF.glUniformMatrix4fv(uniform.indexes[m_shaderIndex], 1, GL_FALSE, pcurrentvalue);
				memcpy(pshadervalue, pcurrentvalue, sizeof(Float) * uniform.stride);
			}
		}
	break;
	}
}

//=============================================
// @brief Unbinds the shader and disables the vertex attribs
//
//=============================================
void CGLSLShader::DisableShader ( void )
{
	// Bind UBOs if any
	if(m_areUBOsBound)
	{
		for(Uint32 i = 0; i < m_uniformBufferObjectsArray.size(); i++)
			m_glExtF.glBindBufferBase(GL_UNIFORM_BUFFER, 1+i, 0);

		m_areUBOsBound = false;
	}

	// Disable any attribs we might've had active
#ifdef DONT_USE_VAO
	for(Uint32 i = 0; i < m_vertexAttribsArray.size(); i++)
	{
		if(!m_vertexAttribsArray[i].active)
			continue;

		DisableAttribute(i);
	}
#endif

	m_glExtF.glUseProgram(0);
	m_isActive = false;
}

//=============================================
// @brief Unbinds the shader and disables the vertex attribs
//
//=============================================
void CGLSLShader::ResetShader ( void )
{
	for(Uint32 i = 0; i < m_vertexAttribsArray.size(); i++)
		DisableAttribute(i);

	m_lastIndex = -1;
}

//=============================================
// @brief Unbinds the shader and disables the vertex attribs
//
// @param szname Vertex attribute name
// @param size Size based on datatype
// @param type Datatype of the attribute
// @param stride Size of the vertex datatype
// @param pointer Byte offset into the vertex datatype
// @return Index of the attribute
//=============================================
Int32 CGLSLShader::InitAttribute( const Char *szname, Uint32 size, Int32 type, Uint32 stride, const void *pointer )
{
	if(stride % 32 != 0)
	{
		m_errorString.clear();
		m_errorString << "VBO vertex data size for shader " << m_shaderFile << " is not divisible by 32";
		return PROPERTY_UNAVAILABLE;
	}

	// See if it was already added
	for(Uint32 i = 0; i < m_vertexAttribsArray.size(); i++)
	{
		if(!qstrcmp(m_vertexAttribsArray[i].name, szname))
			return i;
	}

	glsl_attrib_t newAttrib;
	newAttrib.indexes.resize(m_shadersArray.size());

	if(!m_onDemandLoad)
	{
		for(Uint32 i = 0; i < m_shadersArray.size(); i++)
		{
			if (!m_shadersArray[i].program_id)
				continue;

			newAttrib.indexes[i] = m_glExtF.glGetAttribLocation(m_shadersArray[i].program_id, szname);
			if(newAttrib.indexes[i] == -1)
				newAttrib.indexes[i] = PROPERTY_UNAVAILABLE;
			else
				assert(newAttrib.indexes[i] >= 0);
		}

		Uint32 i = 0;
		for(; i < m_shadersArray.size(); i++)
		{
			if(newAttrib.indexes[i] != PROPERTY_UNAVAILABLE)
				break;
		}

		if(i == m_shadersArray.size())
		{
			m_errorString << "Invalid vertex attribute " << szname << " in shader " << m_shaderFile;
			return PROPERTY_UNAVAILABLE;
		}
	}
	else
	{
		// These should already be tested, as on-demand
		// loading only works after all shaders were
		// tested to begin with(when the BSS changes)
		for(Uint32 i = 0; i < m_shadersArray.size(); i++)
			newAttrib.indexes[i] = PROPERTY_UNINITIALIZED;
	}

	newAttrib.name = szname;
	newAttrib.active = false;
	newAttrib.pointer = pointer;
	newAttrib.stride = stride;
	newAttrib.type = type;
	newAttrib.size = size;

	m_vertexAttribsArray.push_back(newAttrib);
	return m_vertexAttribsArray.size()-1;
}

//=============================================
// @brief Sets an attribute's data pointer
//
// @param index Index of the vertex attribute
// @param pointer Pointer holding the offset of the attribute
//=============================================
void CGLSLShader::SetAttributePointer( Int32 index, const void *pointer )
{
	glsl_attrib_t *pattrib = &m_vertexAttribsArray[index];
	pattrib->pointer = pointer;

	if(pattrib->indexes[m_shaderIndex] != PROPERTY_UNAVAILABLE)
		m_pVBO->SetAttribPointer(pattrib->indexes[m_shaderIndex], pattrib->size, pattrib->type, pattrib->stride, pattrib->pointer);

	pattrib->active = true;
}

//=============================================
// @brief Enables an attibute
//
// @param index Index of the vertex attribute
//=============================================
void CGLSLShader::EnableAttribute( Int32 index )
{
	glsl_attrib_t *pattrib = &m_vertexAttribsArray[index];
	if(pattrib->active)
		return;

	if(pattrib->indexes[m_shaderIndex] != PROPERTY_UNAVAILABLE)
		m_pVBO->SetAttribPointer(pattrib->indexes[m_shaderIndex], pattrib->size, pattrib->type, pattrib->stride, pattrib->pointer);

	pattrib->active = true;
}

//=============================================
// @brief Disables an attibute
//
// @param index Index of the vertex attribute
//=============================================
void CGLSLShader::DisableAttribute( Int32 index )
{
	glsl_attrib_t *pattrib = &m_vertexAttribsArray[index];
	if(!pattrib->active)
		return;

	if(m_pVBO && pattrib->indexes[m_shaderIndex] != PROPERTY_UNAVAILABLE)
		m_pVBO->DisableAttribPointer(pattrib->indexes[m_shaderIndex]);

	pattrib->active = false;
}

//=============================================
// @brief Sets the current VBO to be used
//
// @param pVBO Pointer to CVBO object
//=============================================
void CGLSLShader::SetVBO( CVBO *pVBO )
{
	m_pVBO = pVBO;
}

//=============================================
// @brief Initializes a uniform and returns it's index
//
// @param szname Name of the uniform to initialize
// @param type Type of the uniform to initialize
// @return Index of the uniform in the array
//=============================================
Int32 CGLSLShader::InitUniform( const Char *szname, uniform_e type )
{
	for(Uint32 i = 0; i < m_uniformsArray.size(); i++)
	{
		if(!qstrcmp(m_uniformsArray[i].name, szname))
			return i;
	}

	glsl_uniform_t newUniform;
	newUniform.indexes.resize(m_shadersArray.size());
	
	switch(type)
	{
	case UNIFORM_INT1:
		newUniform.stride = 1;
		break;
	case UNIFORM_FLOAT1:
		newUniform.stride = 1;
		break;
	case UNIFORM_FLOAT2:
		newUniform.stride = 2;
		break;
	case UNIFORM_FLOAT3:
		newUniform.stride = 3;
		break;
	case UNIFORM_FLOAT4:
		newUniform.stride = 4;
		break;
	case UNIFORM_MATRIX4:
		newUniform.stride = 16;
		break;
	case UNIFORM_NOSYNC:
	default:
		break;
	}

	if(newUniform.stride)
	{
		newUniform.shadervalues.resize(newUniform.stride*m_shadersArray.size());
		newUniform.currentvalues.resize(newUniform.stride);
	}
	
	if(!m_onDemandLoad)
	{
		for(Uint32 i = 0; i < m_shadersArray.size(); i++)
		{
			if (!m_shadersArray[i].program_id)
				continue;

			newUniform.indexes[i] = m_glExtF.glGetUniformLocation(m_shadersArray[i].program_id, szname);
			if(newUniform.indexes[i] == -1)
				newUniform.indexes[i] = PROPERTY_UNAVAILABLE;
		}

		Uint32 i = 0;
		for(; i < m_shadersArray.size(); i++)
		{
			if(newUniform.indexes[i] != PROPERTY_UNAVAILABLE)
				break;
		}

		if(i == m_shadersArray.size())
		{
			m_errorString << "Uniform " << szname << " couldn't be found in shader " << m_shaderFile;
			return PROPERTY_UNAVAILABLE;
		}
	}
	else
	{
		// These should already be tested, as on-demand
		// loading only works after all shaders were
		// tested to begin with(when the BSS changes)
		for(Uint32 i = 0; i < m_shadersArray.size(); i++)
			newUniform.indexes[i] = PROPERTY_UNINITIALIZED;
	}

	newUniform.name = szname;
	newUniform.type = type;

	if(type != UNIFORM_NOSYNC)
		newUniform.sync = true;

	m_uniformsArray.push_back(newUniform);
	return (m_uniformsArray.size()-1);
}

//=============================================
// @brief Creates a uniform buffer object
//
// @param pstrName Name of the UBO
// @param bufferSize size of the UBO
// @return The index of the UBO entry
//=============================================
Int32 CGLSLShader :: InitUniformBufferObject( const Char* pstrName, Uint32 bufferSize )
{
	glsl_ubo_t newUBO;
	newUBO.name = pstrName;
	newUBO.blockindexes.resize(m_shadersArray.size());

	if(!m_onDemandLoad)
	{
		for(Uint32 i = 0; i < m_shadersArray.size(); i++)
		{
			newUBO.blockindexes[i] = m_glExtF.glGetUniformBlockIndex(m_shadersArray[i].program_id, newUBO.name.c_str());
			if(newUBO.blockindexes[i] == -1)
				newUBO.blockindexes[i] = PROPERTY_UNAVAILABLE;
		}

		Uint32 j = 0;
		for(; j < m_shadersArray.size(); j++)
		{
			if(newUBO.blockindexes[j] != PROPERTY_UNAVAILABLE)
				break;
		}

		if(j == m_shadersArray.size())
		{
			m_errorString << "Uniform buffer object " << newUBO.name.c_str() << " couldn't be found in shader " << m_shaderFile;
			return PROPERTY_UNAVAILABLE;
		}
	}
	else
	{
		// These should already be tested, as on-demand
		// loading only works after all shaders were
		// tested to begin with(when the BSS changes)
		for(Uint32 i = 0; i < m_shadersArray.size(); i++)
			newUBO.blockindexes[i] = PROPERTY_UNINITIALIZED;
	}

	// Create the object
	m_glExtF.glGenBuffers(1, &newUBO.buffer_id);
	m_glExtF.glBindBuffer(GL_UNIFORM_BUFFER, newUBO.buffer_id);
	m_glExtF.glBufferData(GL_UNIFORM_BUFFER, bufferSize, NULL, GL_STREAM_DRAW);
	m_glExtF.glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// Bind these now when we are creating the UBO
	Uint32 uboIndex = m_uniformBufferObjectsArray.size();
	m_glExtF.glBindBufferBase(GL_UNIFORM_BUFFER, uboIndex+1, newUBO.buffer_id);

	for(Uint32 j = 0; j < m_shadersArray.size(); j++)
	{
		if(newUBO.blockindexes[j] == -1)
			continue;

		m_glExtF.glUniformBlockBinding(m_shadersArray[j].program_id, newUBO.blockindexes[j], uboIndex+1);
	}

	m_glExtF.glBindBufferBase(GL_UNIFORM_BUFFER, uboIndex+1, 0);

	m_uniformBufferObjectsArray.push_back(newUBO);
	return uboIndex;
}

//=============================================
// @brief Sets the value of a determinator
//
// @param index Index of the determinator
// @param value Value to set
// @param update Tells if we should rebind shaders immediately
//=============================================
bool CGLSLShader :: SetDeterminator ( Int32 index, Int32 value, bool update )
{
	if(!m_reCheck)
	{
		if(m_pDeterminatorValues[index] == value)
			return true;
	}

	m_pDeterminatorValues[index] = value;

	if(update)
	{
		m_reCheck = false;
		if(!VerifyDeterminators())
			return false;
		else
			return true;
	}
	else
	{
		m_reCheck = true;
	}

	return true;
}

//=============================================
// @brief Rechecks determinator options and binds the appropriate shader
//
//=============================================
bool CGLSLShader :: VerifyDeterminators ( void )
{
	for(Uint32 k = 0; k < m_disabledStatesArray.size(); k++)
	{
		const disabled_state_t& state = m_disabledStatesArray[k];
		if(state.dt_setting == m_pDeterminatorValues[state.dt_index])
		{
			const glsl_determinator_t& dt = m_determinatorArray[state.dt_index];
			m_errorString << "Tried to use disabled state for determinator " << dt.name << " in shader " << m_shaderFile;
			return false;
		}
	}

	for(Uint32 i = 0; i < m_shadersArray.size(); i++)
	{
		Uint32 j = 0;
		for(; j < m_determinatorArray.size(); j++)
		{
			if(m_determinatorArray[j].values[i] != m_pDeterminatorValues[j])
				break;
		}

		if(j == m_determinatorArray.size())
		{
			if(m_shaderIndex == (Int32)i)
				return true;

			m_shaderIndex = i;
			if(m_isActive)
			{
				if(EnableShader())
					return true;
				else
					return false;
			}
			else
			{
				// Will be tested later
				return true;
			}
		}
	}

	CString buffer;
	buffer << "Invalid determinator values in shader " << m_shaderFile << ":\n";

	for(Uint32 i = 0; i < m_determinatorArray.size(); i++)
		buffer << "Name: " << m_determinatorArray[i].name << ", value: " << m_pDeterminatorValues[i] << "\n";

	m_errorString = buffer;
	return false;
}

//=============================================
// @brief Returns the index of a determinator
//
// @param szname Name of the determinator
// @return Index of the determinator
//=============================================
Int32 CGLSLShader :: GetDeterminatorIndex( const Char *szname )
{
	for(Uint32 i = 0; i < m_determinatorArray.size(); i++)
	{
		if(!qstrcmp(m_determinatorArray[i].name, szname))
			return i;
	}

	CString msg;
	msg << "Unable to get determinator " << szname << " for shader " << m_shaderFile << ".\n";
	m_errorString = msg;

	return DETERMINATOR_UNDEFINED;
}

//=============================================
// @brief Clears time counters
//
//=============================================
void CGLSLShader :: ClearTimeCounters( void )
{
	g_vertexShaderCompileTotalDuration = 0;
	g_vertexShaderGetStatusCallTotalDuration = 0;
	g_fragmentShaderCompileTotalDuration = 0;
	g_fragmentShaderGetStatusCallTotalDuration = 0;
	g_shaderLinkTotalDuration = 0;
	g_shaderLinkGetStatusCallDuration = 0;
	g_numShaderProgramsLinked = 0;
}

//=============================================
// @brief Returns the cumulative compile time for vertex shaders
//
// @return Cumulative compile time for vertex shaders
//=============================================
Double CGLSLShader :: GetTotalVertexShaderCompileTime( void )
{
	return g_vertexShaderCompileTotalDuration;
}

//=============================================
// @brief Returns the cumulative status get call time for vertex shaders
//
// @return Cumulative status get call time for vertex shaders
//=============================================
Double CGLSLShader :: GetTotalVertexShaderGetStatusCallTime( void )
{
	return g_vertexShaderGetStatusCallTotalDuration;
}

//=============================================
// @brief Returns the cumulative compile time for fragment shaders
//
// @return Cumulative compile time for fragment shaders
//=============================================
Double CGLSLShader :: GetTotalFragmentShaderCompileTime( void )
{
	return g_fragmentShaderCompileTotalDuration;
}

//=============================================
// @brief Returns the cumulative status get call time for fragment shaders
//
// @return Cumulative status get call time for fragment shaders
//=============================================
Double CGLSLShader :: GetTotalFragmentShaderGetStatusCallTime( void )
{
	return g_fragmentShaderGetStatusCallTotalDuration;
}

//=============================================
// @brief Returns the cumulative link time for shaders
//
// @return Cumulative link time for shaders
//=============================================
Double CGLSLShader :: GetTotalShaderLinkTime( void )
{
	return g_shaderLinkTotalDuration;
}

//=============================================
// @brief Returns the cumulative get status call time of shader programs linked total
//
// @return Cumulative get status call time of shader programs linked total
//=============================================
Double CGLSLShader :: GetTotalShaderLinkGetStatusCallTime( void )
{
	return g_shaderLinkGetStatusCallDuration;
}

//=============================================
// @brief Returns the number of shader programs linked total
//
// @return The number of shader programs linked total
//=============================================
Uint32 CGLSLShader :: GetNbTotalShaderProgramsLinked( void )
{
	return g_numShaderProgramsLinked;
}
