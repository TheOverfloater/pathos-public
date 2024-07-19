/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef R_GLSL_H
#define R_GLSL_H

#include "file_interface.h"

class CGLExtF;

#define MAX_DETERMINATORS		16
#define MAX_VARIATIONS			4096

#define SHADER_VERTEX			0
#define SHADER_FRAGMENT			1

#define CSD_HEADER_ENCODED		(('1'<<24)+('D'<<16)+('S'<<8)+'C')
#define CSD_FILE_VERSION		1

#define BSD_HEADER_ENCODED		(('1'<<24)+('D'<<16)+('S'<<8)+'B')
#define BSD_FILE_VERSION		1

#define TEMP_FILE_BUFFER_SIZE	1024*1024*4 // 4 mb

//=====================
//	CGLSLShader
//
//=====================
class CGLSLShader
{
public:
	// Typedef for progress update function
	typedef void (*pfnProgressUpdateFunction_t)( const Char* pstrShaderName, Uint32 totalCount, Uint32 completedCount, bool buildingCache );

public:
	enum shaderflags_t
	{
		FL_GLSL_SHADER_NONE			= 0,
		FL_GLSL_ONDEMAND_LOAD		= (1<<0),
		FL_GLSL_BINARY_SHADER_OPS	= (1<<1)
	};

	enum uniformindex_t
	{
		PROPERTY_UNINITIALIZED = -2,
		PROPERTY_UNAVAILABLE = -1
	};

	enum uniform_e
	{
		UNIFORM_UNDEFINED = -1,
		UNIFORM_NOSYNC,
		UNIFORM_INT1,
		UNIFORM_FLOAT1,
		UNIFORM_FLOAT2,
		UNIFORM_FLOAT3,
		UNIFORM_FLOAT4,
		UNIFORM_MATRIX4
	};

	enum determinator_e
	{
		DETERMINATOR_UNDEFINED = -1,
		DETERMINATOR_VERTEX,
		DETERMINATOR_FRAGMENT,
		DETERMINATOR_SHARED
	};

	enum comparison_e
	{
		BRANCH_UNDEFINED = -1,
		BRANCH_EQUAL,
		BRANCH_GREATER,
		BRANCH_GREATER_OR_EQUAL,
		BRANCH_LESS,
		BRANCH_LESS_OR_EQUAL,
		BRANCH_NOT_EQUAL,
	};

	enum operator_e
	{
		OPERATOR_NONE = 0,
		OPERATOR_AND,
		OPERATOR_ELSE
	};

	// <glsl_branch_value_t>
	struct glsl_branchcondition_t
	{
		glsl_branchcondition_t():
			boperator(OPERATOR_NONE),
			bcomparison(BRANCH_UNDEFINED),
			determinatorindex(0),
			value(0)
		{}

		operator_e boperator;
		comparison_e bcomparison;
		Int32 determinatorindex;
		Int32 value;
	};

	// <glsl_determinator_t>
	struct glsl_determinator_t
	{
		glsl_determinator_t():
			type(DETERMINATOR_UNDEFINED),
			minval(0),
			maxval(0)
		{}

		CString name;
		CArray<Int16> values;
		determinator_e type;

		Int32 minval;
		Int32 maxval;
	};

	// <glsl_uniform_t>
	struct glsl_uniform_t
	{
		glsl_uniform_t():
			type(UNIFORM_UNDEFINED),
			stride(0),
			sync(false)
			{}

		CString name;
		uniform_e type;

		CArray<Float> currentvalues;
		CArray<Float> shadervalues;

		CArray<Int16> indexes;
		Uint32 stride;

		bool sync;
	};

	// <glsl_ubo_t>
	struct glsl_ubo_t
	{
		glsl_ubo_t():
			buffer_id(0)
		{};

		CString name;
		GLuint buffer_id;
		CArray<Int32> blockindexes;
	};

	// <glsl_shader_t>
	struct glsl_shader_t
	{
		glsl_shader_t():
			compiled(false),
			program_id(0)
		{}

		bool compiled;

		GLuint program_id;
	};

	// <glsl_attrib_t>
	struct glsl_attrib_t
	{
		glsl_attrib_t():
			size(0),
			stride(0),
			type(0),
			pointer(nullptr),
			active(false)
		{}

		CString name;
		CArray<Int16> indexes;

		Uint32 size;
		Uint32 stride;
		Int32 type;

		const void *pointer;
		bool active;
	};

	// <shader_chunk_t>
	struct shader_chunk_t
	{
		shader_chunk_t():
			pdata(nullptr),
			isize(0),
			pchunks(nullptr),
			numchunks(0),
			pconditionals(nullptr),
			numconditionals(0)
		{}

		Char *pdata;
		Uint32 isize;

		shader_chunk_t* pchunks;
		Uint32 numchunks;

		glsl_branchcondition_t *pconditionals;
		Uint32 numconditionals;
	};

	// <shader_script_t>
	struct shader_script_t
	{
		shader_script_t():
			pchunks(nullptr),
			numchunks(0),
			type(0)
		{}

		shader_chunk_t* pchunks;
		Uint32 numchunks;

		Uint32 type;
	};

	// <invalid_state_t>
	struct invalid_state_t
	{
		invalid_state_t():
			numdts(0)
		{
			memset(dt_indexes, 0, sizeof(dt_indexes));
			memset(dt_minrange, 0, sizeof(dt_minrange));
			memset(dt_maxrange, 0, sizeof(dt_maxrange));
		}

		Uint32 dt_indexes[MAX_DETERMINATORS];
		Int32 dt_minrange[MAX_DETERMINATORS];
		Int32 dt_maxrange[MAX_DETERMINATORS];

		Uint32 numdts;
	};

	// <disabled_state_t>
	struct disabled_state_t
	{
		disabled_state_t():
			dt_setting(0),
			dt_index(0)
		{}

		CString dt_name;
		Int32 dt_setting;
		Int32 dt_index;
	};

	// <csdheader_t>
	struct csddeterminator_t
	{
		csddeterminator_t():
			type(DETERMINATOR_UNDEFINED),
			valuesoffset(0),
			minval(0),
			maxval(0)
		{
			memset(name, 0, sizeof(name));
		}

		Char name[32];
		determinator_e type;

		Int32 valuesoffset;

		Int32 minval;
		Int32 maxval;
	};

	// <csdshaderdata_t>
	struct csdshaderdata_t
	{
		csdshaderdata_t():
			vertexdataoffs(0),
			vertexdatasize(0),
			fragmentdataoffs(0),
			fragmentdatasize(0)
		{}

		Int32 vertexdataoffs;
		Int32 vertexdatasize;

		Int32 fragmentdataoffs;
		Int32 fragmentdatasize;
	};

	// <csddisabled_state_t>
	struct csddisabled_state_t
	{
		csddisabled_state_t():
			value(0)
		{
			memset(determinator, 0, sizeof(determinator));
		}

		Char determinator[64];
		Int32 value;
	};

	// <csdheader_t>
	struct csdheader_t
	{
		csdheader_t():
			id(0),
			version(0),
			determinatoroffset(0),
			numdeterminators(0),
			shaderdataoffset(0),
			numshaders(0),
			size(0)
		{
			memset(hash, 0, sizeof(hash));
		}

		Int32 id;
		Int32 version;

		// md5 hash
		Char hash[33];

		Uint32 determinatoroffset;
		Uint32 numdeterminators;

		Uint32 shaderdataoffset;
		Uint32 numshaders;

		Uint32 size;
	};

	struct shader_binary_t
	{
		shader_binary_t():
			dataoffset(0),
			datasize(0),
			binaryformat(0)
		{
		}

		Int32 dataoffset;
		Uint32 datasize;
		GLenum binaryformat;
	};

	struct bsd_header_t
	{
		bsd_header_t():
			id(0),
			version(0),
			shaderoffset(0),
			numshaders(0),
			size(0)
		{
			memset(hash, 0, sizeof(hash));
		}

		Int32 id;
		Int32 version;

		// md5 hash
		Char hash[33];

		Int32 shaderoffset;
		Uint32 numshaders;

		Uint32 size;
	};

public:
	CGLSLShader( const file_interface_t& fileFuncs, const CGLExtF& glExtF, const Char *szfile, Int32 flags = FL_GLSL_SHADER_NONE, pfnProgressUpdateFunction_t pfnCallback = nullptr );
	CGLSLShader( const file_interface_t& fileFuncs, const CGLExtF& glExtF, Int32 flags = FL_GLSL_SHADER_NONE, pfnProgressUpdateFunction_t pfnCallback = nullptr );
	~CGLSLShader( void );

	// Clears all data for the class
	void Clear( void );
	// Compiles the GLSL script
	bool Compile( const Char* szfile );

public:
	// Uniform float assignment variations
	inline void SetUniform4f( Int32 index, Float x, Float y, Float z, Float w );
	inline void SetUniform4fv( Int32 index, const Float *v, Uint32 num );
	inline void SetUniform3f( Int32 index, Float x, Float y, Float z );
	inline void SetUniform2f( Int32 index, Float x, Float y );
	inline void SetUniform1f( Int32 index, Float x );
	// Uniform int assignment variations
	inline void SetUniform1i( Int32 index, Int32 x );
	// Uniform matrix assignment variations
	inline void SetUniformMatrix4fv( Int32 index, const Float *matrix, bool transpose = false );

	// Enables syncing on a uniform
	inline void EnableSync( Int32 uniform );
	// Disables syncing on a uniform
	inline void DisableSync( Int32 uniform );

	// Validates a program, and if it's not ok, it'll write to the console
	bool ValidateProgram( void (*pfnConPrintfFnPtr)( const Char *fmt, ... ) );

	// Sets the data for the UBO
	inline void SetUniformBufferObjectData( Int32 index, void* pBufferData, Uint32 dataSize );

public:
	// Initializes a uniform
	Int32 InitUniform( const Char *szname, uniform_e type );
	// Creates a uniform buffer object
	Int32 InitUniformBufferObject( const Char* pstrName, Uint32 bufferSize );
	// Initializes a vertex attribute
	Int32 InitAttribute( const Char *szname, Uint32 size, Int32 type, Uint32 stride, const void *pointer );
	// Sets a vertex attribute's data pointer
	void SetAttributePointer( Int32 index, const void *pointer );
	// Enables a vertex attribute
	void EnableAttribute( Int32 index );
	// Disables a vertex attribute
	void DisableAttribute( Int32 index );

	// Creates a new GLSL vertex attribute, and returns it's pointer
	glsl_determinator_t *AddDeterminator( const Char *szname );
	// Sets a determinator's value
	bool SetDeterminator( Int32 index, Int32 value, bool update = true );
	// Returns a determinator's index for a name
	Int32 GetDeterminatorIndex( const Char *szname );
	// Checks all determinators and binds the appropriate shader
	bool VerifyDeterminators( void );

	// Compiles a single shader object
	bool CompileShader( Uint32 index, glsl_shader_t* pshader, csdshaderdata_t* pshaderdata );
	// Reads in a determinator's script data
	bool ReadDeterminator( const Char** ppstr );
	// Reads in the script entry for invalid states
	bool ReadInvalidState( const Char** ppstr );

	// Binds the shader in OpenGL
	bool EnableShader( void );
	// Unbinds the shader
	void DisableShader( void );
	// Resets all the attribs
	void ResetShader( void );

	// Sets the associated VBO
	void SetVBO( class CVBO *pVBO );
	
	// Returns the shader script's name
	const Char* GetShaderScriptName( void ) const { return m_shaderFile.c_str(); }
	// Retrieves the error string if any
	const Char* GetError( void ) const { return m_errorString.c_str(); }
	// Tells if there was an error
	bool HasError( void ) const { return m_bFailed; }

	// Disables a determinator from being compiled
	void DisableDeterminatorState( const Char *pstrName, Int32 state );

	// Tells the total number of shader variations
	Uint32 GetNbTotalShaderVariations( void ) const;
	// Tells if a specific shader variation has been compiled
	bool IsShaderVariationCompiled( Uint32 index );
	// Compiles a specific shader variation
	bool CompileShaderVariation( Uint32 index );

public:
	// Clears time counters
	static void ClearTimeCounters( void );
	// Returns the cumulative compile time for vertex shaders
	static Double GetTotalVertexShaderCompileTime( void );
	// Returns the cumulative status get call time for vertex shaders
	static Double GetTotalVertexShaderGetStatusCallTime( void );
	// Returns the cumulative compile time for fragment shaders
	static Double GetTotalFragmentShaderCompileTime( void );
	// Returns the cumulative status get call time for fragment shaders
	static Double GetTotalFragmentShaderGetStatusCallTime( void );
	// Returns the cumulative link time for shaders
	static Double GetTotalShaderLinkTime( void );
	// Returns the cumulative get call time time for shaders
	static Double GetTotalShaderLinkGetStatusCallTime( void );
	// Returns the number of shader programs linked total
	static Uint32 GetNbTotalShaderProgramsLinked( void );

private:
	// Free all data used by the shader class
	void FreeData( void );
	// Free all compile time data
	void FreeTempData( void );
	// Free shader binds
	void FreeShaderData( void );
	// Loads a CSD file
	bool CompileFromCSD( void );
	// Compiles from a raw shader script
	bool CompileFromScript( void );
	// Loads shaders from the BSD file
	bool LoadFromBSD( void );
	// Compiles all CSD shaders
	bool CompileCSDShaderData( void );

	// Constructs all the possible variations
	bool ConstructBranches( const Char* pSrc, Uint32 fileSize );
	// Recursively fill all the possible determinator variations
	void RecursiveFillValues( Uint32 index, Uint32& numShaders );

	// Parses a single token from the shader file
	Char *ParseToken( Char *pfile, Char *token );
	// Reads a chunk from the shader script
	bool ReadChunks( const Char **ppscan, shader_chunk_t** pchunkptr, Uint32* numchunkptr, glsl_branchcondition_t* pconditionals, Uint32 numconditionals );

	// Splices the vertex and fragment scripts for a shader
	bool SpliceScripts( Uint32 id, Char **vsptr, Char **fsptr );
	// Tells if a chunk should be included in this variation
	bool ShouldIncludeChunk( Uint32 id, shader_chunk_t *pchunk );
	// Recursively adds all usable shader chunks to a shader
	bool RecursiveAddChunks( Uint32 id, shader_chunk_t* pchunk, Char* pstrbuf, Uint32* size, Uint32 maxBufferSize );
	// Recursively frees all GLSL chunks
	void RecursiveFreeChunks( shader_chunk_t* pchunk );

	// Prints error information to a log file
	void Shader_PrintLog( GLuint shader_id, const Char *script, Uint32 length, const Char *szoutpath, bool dumpShaderCode );
	// Prints error information for the entire program to a log file
	void Program_PrintLog( GLuint program_id, const Char *szoutpath );

	// Re-syncs a uniform
	void SyncUniform( glsl_uniform_t& uniform );

private:
	// TRUE if the shader is active
	bool m_isActive;
	// TRUE if we need to re-check shader binds
	bool m_reCheck;
	// TRUE if on-demand load is used
	bool m_onDemandLoad;
	// TRUE if binary shader funcs are supported
	bool m_useBinaryShaders;
	// TRUE if the UBOs got bound
	bool m_areUBOsBound;

	// Currently bound shader's index
	Int32 m_shaderIndex;
	// Index of the last shader bound
	Int32 m_lastIndex;

	// Array of determinators
	CArray<glsl_determinator_t> m_determinatorArray;
	// Array of GLSL uniforms
	CArray<glsl_uniform_t> m_uniformsArray;
	// Array of uniform buffer objects
	CArray<glsl_ubo_t> m_uniformBufferObjectsArray;
	// Array of unique GLSL shaders
	CArray<glsl_shader_t> m_shadersArray;
	// Array of vertex attributes
	CArray<glsl_attrib_t> m_vertexAttribsArray;
	// All invalid determinator states
	CArray<invalid_state_t> m_invalidStatesArray;
	// Array of disabled determinator states
	CArray<disabled_state_t> m_disabledStatesArray;

	// Used to store permutation arrays
	Int32 *m_pDeterminatorValues;

	// Pointer to VBO associated with this shader
	CVBO *m_pVBO;

	// Compile-time vertex script data
	shader_script_t *m_pVertexScript;
	// Compile-time fragment script data
	shader_script_t *m_pFragmentScript;
	// Pointer to shader data header
	csdheader_t* m_pCSDHeader;

	// Matrix uniform for non-sync matrices
	Float m_uniformMatrix[4][4];

private:
	// Compile status
	bool m_bFailed;

	// Shader filename with path
	CString m_shaderFile;
	// Error string
	CString m_errorString;
	// Root directory
	CString m_rootDirectory;

	// External functions class
	const CGLExtF& m_glExtF;

	// External file load interface funcs
	file_interface_t m_fileInterface;

private:
	// Progress update function
	pfnProgressUpdateFunction_t m_pProgressUpdateCallbackFn;

private:
	// Total duration of vertex shader compile calls
	static Double g_vertexShaderCompileTotalDuration;
	// Total duration of vertex shader verification calls
	static Double g_vertexShaderGetStatusCallTotalDuration;
	// Total duration of fragment shader compile calls
	static Double g_fragmentShaderCompileTotalDuration;
	// Total duration of fragment shader compile calls
	static Double g_fragmentShaderGetStatusCallTotalDuration;
	// Total duration of shader linking calls
	static Double g_shaderLinkTotalDuration;
	// Total duration of shader linking calls
	static Double g_shaderLinkGetStatusCallDuration;
	// Number of shader programs linked total
	static Uint32 g_numShaderProgramsLinked;
};
#include "r_glsl_inline.hpp"
#endif // R_GLSL_H