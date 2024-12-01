/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef R_CUBEMAPS_H
#define R_CUBEMAPS_H

// Number of cubemap resolutions
static constexpr Uint32 NUM_CUBEMAP_SIZES = 7;

// Encoded cubemap header
#define ECD_HEADER_ENCODED (('2'<<24)+('D'<<16)+('C'<<8)+'E')

// Cubemap file version
#define CUBEMAP_FILE_VERSION 0

// Flags managing data availability
enum cube_dataflags_t
{
	FL_ECD_NONE = 0,
	FL_ECD_HAS_DAYDATA = (1<<0),
	FL_ECD_HAS_NIGHTDATA = (1<<1)
};

// shader states
enum cubemapstate_t
{
	CUBEMAPS_OFF = 0,
	CUBEMAPS_ON,
	CUBEMAPS_INTERP
};

// cubemap compression type
enum cubemapcompression_t
{
	COMPRESSION_DXT1 = 0,
};

// environment cubemap data file
struct ecdheader_t
{
	ecdheader_t():
		id(0),
		version(0),
		flags(0),
		cubemapinfooffset(0),
		numcubemaps(0),
		length(0)
		{}

	Int32 id;
	Int32 version;
	Int32 flags;

	Int32 cubemapinfooffset;
	Int32 numcubemaps;

	Int32 length;
};

struct ecdcubemapface_t
{
	ecdcubemapface_t():
		dataoffset(0),
		datasize(0)
	{}

	Int32 dataoffset;
	Int32 datasize;
};

struct ecdsinglecubemap_t
{
	ecdsinglecubemap_t():
		daystage(0),
		facesoffset(0),
		dxtcompression(0)
	{}

	Int32 daystage;
	Int32 facesoffset;
	Int32 dxtcompression;
};

struct ecdcubemap_t
{
	ecdcubemap_t():
		entindex(0),
		cubemapindex(0),
		width(0),
		height(0),
		cubemapoffset(0),
		cubemapcount(0)
		{}

	entindex_t entindex;
	Int32 cubemapindex;

	Int32 width;
	Int32 height;

	Vector origin;

	Int32 cubemapoffset;
	Int32 cubemapcount;
};

struct cubemapinfo_t
{
	cubemapinfo_t():
		entindex(0),
		cubemapindex(0),
		palloc(nullptr),
		width(0),
		height(0),
		pimagedata(nullptr)
		{}

	// original entity index
	entindex_t entindex;
	// index in cubemap list
	Int32 cubemapindex;

	// opengl texture index
	struct en_texalloc_t* palloc;
	
	// width/height info
	Int32 width;
	Int32 height;

	// location in world
	Vector origin;

	// Only used during compile
	byte* pimagedata;
};

/*
====================
CCubemapManager

====================
*/
class CCubemapManager
{
public:
	// Cubemap interpolation time
	static const Float CUBEMAP_INTERP_TIME;

public:
	CCubemapManager( void );
	~CCubemapManager();

public:
	// Initializes the class
	bool Init( void );
	// Shuts down the class
	void Shutdown( void );

	// Initializes the game objects
	bool InitGame( void );
	// Clears the game objects
	void ClearGame( void );

	// Initializes OpenGL objects
	bool InitGL( void );
	// Clears OpenGL objects
	void ClearGL( void );

	// Updates ideal cubemap states
	void Update( const Vector& v_origin );
	// Builds the cubemaps
	void BuildCubemaps( void );
	
	// Gets the ideal cubemap
	cubemapinfo_t* GetIdealCubemap( void );
	// Gets the previous cubemap
	cubemapinfo_t* GetPrevCubemap( void );
	// Gets the interpoland value
	Float GetInterpolant( void ) const;
	// Adds a new cubemap
	void AddCubemap( entindex_t entindex, const Vector& origin, Uint32 resolution );

private:
	// Clears the class
	void Cleanup( bool iscleargame );
	// Builds the cubemap list
	void BuildCubemapList( const struct entitydata_t* pEntities, Uint32 numEntities );
	// Builds the entity list
	void BuildEntityList( cl_entity_t** pRenderEntities, Uint32 *pNumRenderEntities, const struct entitydata_t* pEntities, Uint32 numEntities );
	// Saves cubemaps to a file
	void SaveCubemapFile( void );
	// Reads the cubemap file
	bool RenderCubemaps( cl_entity_t* pRenderEntities, Uint32 numRenderEntities, bool dumpTGAs );
	// Verifies if an ECD file is valid
	bool VerifyECDFile( const ecdheader_t* pheader );

private:
	// Array of cubemaps
	CArray<cubemapinfo_t> m_cubemapsArray;

	// Ideal cubemap to use
	cubemapinfo_t *m_pIdealCubemap;
	// Previous cubemap used
	cubemapinfo_t *m_pPrevCubemap;

	// Interpolant for blending cubemaps
	Float m_flInterpolant;
	// Last time the ideal cubemap changed
	Float m_flLastChangeTime;

private:
	// Cubemap resolutions arrays
	static Uint32 g_cubemapResolutions[NUM_CUBEMAP_SIZES][2];
};
extern CCubemapManager gCubemaps;
#endif