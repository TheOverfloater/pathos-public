/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef R_PARTICLES_H
#define R_PARTICLES_H

class CCVar;
struct en_texalloc_t;

#include "r_glsl.h"

// Distance between light checks
static const Uint32 LIGHTCHECK_DISTANCE			= 16;

// Max active particles
static const Uint32 MAX_ACTIVE_PARTICLES		= 32768;

// Max lights for projective type
static const Uint32 MAX_PARTICLE_POINT_LIGHTS	= 4;
// Max lights for projective type
static const Uint32 MAX_PARTICLE_PROJ_LIGHTS	= 4;

enum particle_lightflags_t
{
	PARTICLE_LIGHTCHECK_NONE			= 0,
	PARTICLE_LIGHTCHECK_NORMAL			= (1<<0),
	PARTICLE_LIGHTCHECK_SCOLOR			= (1<<1),
	PARTICLE_LIGHTCHECK_MIXP			= (1<<2),
	PARTICLE_LIGHTCHECK_INTENSITY		= (1<<3),
	PARTICLE_LIGHTCHECK_ONLYONCE		= (1<<4),
	PARTICLE_LIGHTCHECK_NO_DYNLIGHTS	= (1<<5)
};

enum prt_shader_type_e
{
	SHADER_PRT_NORMAL = 0,
	SHADER_PRT_DISTORT
};

enum prt_render_pass_e
{
	PARTICLES_NORMAL = 0,
	PARTICLES_SKY,
	PARTICLES_VIEWMODEL,
};

enum prt_system_shape_e
{
	shape_point = 0,
	shape_box,
	shape_playerplane
};

enum prt_alignment_e
{
	align_tiled = 0,
	align_parallel,
	align_normal,
	align_tracer
};

enum prt_rendermode_e
{
	render_additive = 0,
	render_alpha,
	render_alphatest,
	render_distort,
};

enum prt_collision_e
{
	collide_none = 0,
	collide_die,
	collide_bounce,
	collide_decal,
	collide_stuck,
	collide_new_system
};

enum prt_windtype_e
{
	wind_none = 0,
	wind_linear,
	wind_sine
};

enum system_flags_t
{
	SYSTEM_FL_NONE			= 0,
	SYSTEM_FL_RANDOM_DIR	= (1<<0),
	SYSTEM_FL_GLOBS			= (1<<1),
	SYSTEM_FL_SOFTOFF		= (1<<2)
};

enum collision_flags_t
{
	COLLISION_FL_NONE		= 0,
	COLLISION_FL_PRECISE	= (1<<0),
	COLLISION_FL_BMODELS	= (1<<1),
	COLLISION_FL_WATER		= (1<<2)
};

enum render_flags_t
{
	RENDER_FL_NONE			= 0,
	RENDER_FL_OVERBRIGHT	= (1<<0),
	RENDER_FL_SKYBOX		= (1<<1),
	RENDER_FL_NOCULL		= (1<<2),
	RENDER_FL_NOFOG			= (1<<3)
};

struct script_definition_t
{
	script_definition_t():
		shapetype(shape_point),
		flags(SYSTEM_FL_NONE),
		globsize(0),
		numglobparticles(0),
		minvel(0),
		maxvel(0),
		maxofs(0),
		fadeintime(0),
		fadeoutdelay(0),
		velocitydamp(0),
		stuckdie(0),
		tracerdist(0),
		maxheight(0),
		windx(0),
		windy(0),
		windvar(0),
		windmult(0),
		minwindmult(0),
		windmultvar(0),
		windtype(wind_none),
		attachflags(PARTICLE_ATTACH_NONE),
		maxlife(0),
		maxlifevar(0),
		systemsize(0),
		transitiondelay(0),
		transitiontime(0),
		transitionvar(0),
		rotationvar(0),
		rotationvel(0),
		rotationdamp(0),
		rotationdampdelay(0),
		rotxvar(0),
		rotxvel(0),
		rotxdamp(0),
		rotxdampdelay(0),
		rotyvar(0),
		rotyvel(0),
		rotydamp(0),
		rotydampdelay(0),
		scale(0),
		scalevar(0),
		scaledampdelay(0),
		scaledampfactor(0),
		veldampdelay(0),
		gravity(0),
		particlefreq(0),
		impactdamp(0),
		mainalpha(0),
		minlight(0),
		maxlight(0),
		startparticles(0),
		maxparticles(0),
		maxparticlevar(0),
		lighting_flags(PARTICLE_LIGHTCHECK_NONE),
		collision(collide_none),
		collision_flags(COLLISION_FL_NONE),
		alignment(align_tiled),
		rendermode(render_additive),
		render_flags(RENDER_FL_NONE),
		spawnchance(0),
		softofftime(0),
		fadedistfar(0),
		fadedistnear(0),
		numframes(0),
		framesizex(0),
		framesizey(0),
		framerate(0),
		decallife(0),
		decalfade(0),
		decalgrowthtime(0),
		ptexture(nullptr)
		{
		}

	CString scriptpath;

	Int16 shapetype;
	Int16 flags;

	Int16 globsize;
	Int16 numglobparticles;

	Float minvel;
	Float maxvel;
	Float maxofs;

	Float fadeintime;
	Float fadeoutdelay;
	Float velocitydamp;
	Float stuckdie;
	Float tracerdist;

	Float maxheight;

	Float windx;
	Float windy;
	Float windvar;
	Float windmult;
	Float minwindmult;
	Float windmultvar;
	Int16 windtype;

	Int16 attachflags;

	Float maxlife;
	Float maxlifevar;
	Float systemsize;

	Vector primarycolor;
	Vector secondarycolor;
	Float transitiondelay;
	Float transitiontime;
	Float transitionvar;
	
	Float rotationvar;
	Float rotationvel;
	Float rotationdamp;
	Float rotationdampdelay;

	Float rotxvar;
	Float rotxvel;
	Float rotxdamp;
	Float rotxdampdelay;

	Float rotyvar;
	Float rotyvel;
	Float rotydamp;
	Float rotydampdelay;

	Float scale;
	Float scalevar;
	Float scaledampdelay;
	Float scaledampfactor;
	Float veldampdelay;
	Float gravity;
	Float particlefreq;
	Float impactdamp;
	Float mainalpha;
	Float minlight;
	Float maxlight;

	Uint16 startparticles;
	Int16 maxparticles;
	Uint16 maxparticlevar;

	Int16 lighting_flags;
	Int16 collision;
	Int16 collision_flags;
	Int16 alignment;
	Int16 rendermode;
	Int16 render_flags;
	Int16 spawnchance;
	Float softofftime;

	Int16 fadedistfar;
	Int16 fadedistnear;

	Int16 numframes;
	Int16 framesizex;
	Int16 framesizey;
	Int16 framerate;

	CString create;
	CString deathcreate;
	CString watercreate;

	Float decallife;
	Float decalfade;
	Float decalgrowthtime;

	en_texture_t *ptexture;
};

struct script_cache_t
{
	script_cache_t():
		pdefinition(nullptr)
		{}
	~script_cache_t()
	{
		if(pdefinition)
			delete pdefinition;
	}

	CString name;

	script_definition_t* pdefinition;
	CArray<CString> clusterscripts;
};

struct part_msg_cache_t
{
	part_msg_cache_t():
		id(0),
		entindex(0),
		attachment(0),
		msgtype(0),
		scripttype(0),
		boneindex(NO_POSITION),
		attachflags(PARTICLE_ATTACH_NONE),
		keepcached(false)
		{}

	Int32 id;
	entindex_t entindex;
	CString file;
	byte attachment;

	Vector origin;
	Vector direction;

	byte msgtype;
	byte scripttype;

	Int32 boneindex;
	Int32 attachflags;

	bool keepcached;
};

struct particle_system_t
{
	particle_system_t():
		entindex(0),
		id(0),
		radius(0),
		parententity(nullptr),
		attachment(0),
		skyheight(0),
		spawntime(0),
		softoffbegintime(0),
		maxparticles(0),
		particlefreq(0),
		attachflags(PARTICLE_ATTACH_NONE),
		numspawns(0),
		createsystem(nullptr),
		watersystem(nullptr),
		parentsystem(nullptr),
		ptexture(nullptr),
		pparticleheader(nullptr),
		indexoffset(0),
		numindexes(0),
		visframe(0),
		spawned(false),
		pdefinition(nullptr),
		numdlights(0),
		numspotlights(0),
		boneindex(0)
		{
			memset(pdlights, 0, sizeof(pdlights));
			memset(pspotlights, 0, sizeof(pspotlights));
		}

	entindex_t entindex;
	Int16 id;

	Vector origin;
	Vector dir;
	Float radius;

	cl_entity_t *parententity;
	byte attachment;

	Float skyheight;

	Double spawntime;
	Double softoffbegintime;

	Int16 maxparticles;
	Float particlefreq;

	Int16 attachflags;

	Uint64 numspawns;

	particle_system_t *createsystem;
	particle_system_t *watersystem;
	particle_system_t *parentsystem;

	en_texture_t *ptexture;

	struct cl_particle_t *pparticleheader;

	Int32 indexoffset;
	Int32 numindexes;

	Int32 visframe;

	bool spawned;
	
	const script_definition_t* pdefinition;

	CArray<Uint32> leafnums;

	cl_dlight_t *pdlights[MAX_PARTICLE_POINT_LIGHTS];
	Uint32 numdlights;

	cl_dlight_t *pspotlights[MAX_PARTICLE_PROJ_LIGHTS];
	Uint32 numspotlights;

	Int16 boneindex;
};

struct cl_particle_t
{
	cl_particle_t():
		spawntime(0),
		life(0),
		scale(0),
		alpha(0),
		fadeoutdelay(0),
		scaledampdelay(0),
		secondarydelay(0),
		secondarytime(0),
		rotationvel(0),
		rotation(0),
		rotx(0),
		rotxvel(0),
		roty(0),
		rotyvel(0),
		windxvel(0),
		windyvel(0),
		windmult(0),
		fadein(0),
		frame(0),
		psystem(nullptr),
		next(nullptr),
		prev(nullptr)
	{
		memset(texcoords, 0, sizeof(texcoords));
		memset(pad, 0, sizeof(pad));
	}

	Vector velocity;
	Vector origin;
	Vector color;
	Vector scolor;
	Vector lastspawn;
	Vector normal;

	Vector lightcol;

	Vector last_light;
	Vector lightmap;

	Double spawntime;
	Float life;
	Float scale;
	Float alpha;

	Float fadeoutdelay;

	Float scaledampdelay;
	Float secondarydelay;
	Float secondarytime;

	Float rotationvel;
	Float rotation;

	Float rotx;
	Float rotxvel;

	Float roty;
	Float rotyvel;

	Float windxvel;
	Float windyvel;
	Float windmult;
	Float fadein;

	Float texcoords[4][2];

	Int32 frame;

	particle_system_t *psystem;

	cl_particle_t	*next;
	cl_particle_t	*prev;

	byte pad[27];
};

struct particle_vertex_t
{
	particle_vertex_t()
	{
		memset(origin, 0, sizeof(origin));
		memset(color, 0, sizeof(color));
		memset(texcoord, 0, sizeof(texcoord));
		memset(padding, 0, sizeof(padding));
	}

	vec4_t origin;
	vec4_t color;

	Float texcoord[2];
	byte padding[24];
};

struct p_proj_light
{
	p_proj_light():
		u_origin(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_color(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_radius(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_texture(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_matrix(CGLSLShader::PROPERTY_UNAVAILABLE)
		{}

	Int32 u_origin;
	Int32 u_color;
	Int32 u_radius;

	Int32 u_texture;
	Int32 u_matrix;
};

struct p_point_light
{
	p_point_light():
		u_origin(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_color(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_radius(CGLSLShader::PROPERTY_UNAVAILABLE)
		{}

	Int32 u_origin;
	Int32 u_color;
	Int32 u_radius;
};

struct particle_attribs
{
	particle_attribs():
		a_origin(CGLSLShader::PROPERTY_UNAVAILABLE),
		a_color(CGLSLShader::PROPERTY_UNAVAILABLE),
		a_texcoord(CGLSLShader::PROPERTY_UNAVAILABLE),
		d_numprojlights(CGLSLShader::PROPERTY_UNAVAILABLE),
		d_numpointlights(CGLSLShader::PROPERTY_UNAVAILABLE),
		d_fog(CGLSLShader::PROPERTY_UNAVAILABLE),
		d_type(CGLSLShader::PROPERTY_UNAVAILABLE),
		d_alphatest(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_modelview(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_projection(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_fogparams(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_fogcolor(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_overbright(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_scrsize(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_zfar(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_znear(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_texture0(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_rtexture0(CGLSLShader::PROPERTY_UNAVAILABLE),
		u_rtexture1(CGLSLShader::PROPERTY_UNAVAILABLE)
		{}

	Int32 a_origin;
	Int32 a_color;
	Int32 a_texcoord;

	p_proj_light proj_lights[MAX_PARTICLE_POINT_LIGHTS];
	Int32 d_numprojlights;

	p_point_light point_lights[MAX_PARTICLE_PROJ_LIGHTS];
	Int32 d_numpointlights;
	
	Int32 d_fog;
	Int32 d_type;
	Int32 d_alphatest;

	Int32 u_modelview;
	Int32 u_projection;

	Int32 u_fogparams;
	Int32 u_fogcolor;
	Int32 u_overbright;
	Int32 u_scrsize;

	Int32 u_zfar;
	Int32 u_znear;

	Int32 u_texture0;

	Int32 u_rtexture0;
	Int32 u_rtexture1;
};

/*
====================
CParticleEngine

====================
*/
class CParticleEngine
{
public:
	CParticleEngine( void );
	~CParticleEngine( void );

public:
	// Initializes the class
	bool Init( void );
	// Shuts down the class
	void Shutdown( void );

	// Initializes game objects
	bool InitGame( void );
	// Clears game objects
	void ClearGame( void );

	// Initializes OpenGL objects
	bool InitGL( void );
	// Clear OpenGL objects
	void ClearGL( void );

public:
	// Updates particle states
	void Update( void );

public:
	// Renders particles
	bool DrawParticles( prt_render_pass_e pass );
	
public:
	// Caches a particle engine system creation event
	void CacheCreateSystem( const Vector& origin, const Vector& direction, part_script_type_t scripttype, const Char* pstrFilepath, Uint32 id, entindex_t entindex, Int32 attachment, Int32 boneindex = NO_POSITION, Int32 attachflags = PARTICLE_ATTACH_NONE );
	// Caches a particle removal event
	void CacheRemoveSystem( Int32 id, entindex_t entindex, bool keepcached );
	// Parses a particle script
	const script_cache_t* PrecacheScript( Int32 type, const Char *name, CArray<CString>* pLoadList );
	// Loads a system script
	bool LoadSystemScript( script_cache_t* pCache, const Char* pstrData );
	// Loads a cluster script
	bool LoadClusterScript( script_cache_t* pCache, const Char* pstrData );
	// Reads a field in from a particle script
	bool ReadField( script_definition_t* pdefinition, const Char* pstrField, const Char* pstrNextRead );

	// Removes particle systems tied to an entity
	void KillEntityParticleSystems( Int32 entindex );

private:
	// Allocates a particle system object
	particle_system_t *AllocSystem( void );
	// Allocates a particle
	__forceinline cl_particle_t *AllocParticle( particle_system_t *psystem );
	// Releases a particle system
	void ReleaseSystem( particle_system_t* psystem );

private:
	// Creates partile systems from a cluster script
	void CreateCluster( const Char *szPath, const Vector& origin, const Vector& dir, Uint32 iId, cl_entity_t *pentity = 0, entindex_t entindex = 0, Uint32 attachment = 0, Int32 boneindex = NO_POSITION, Int32 attachflags = PARTICLE_ATTACH_NONE );
	// Creates a particle system object
	particle_system_t *CreateSystem( const Char *szPath, const Vector& origin, const Vector& dir, Uint32 iId, particle_system_t *parent = nullptr, cl_entity_t *entity = 0, entindex_t entindex = 0, Uint32 attachment = 0, Int32 boneindex = NO_POSITION, Int32 attachflags = PARTICLE_ATTACH_NONE );
	// Removes a particle system
	void RemoveSystem( entindex_t entindex, Int32 iId, bool keepcached );

private:
	// Updates particle systems
	void UpdateSystems( void );
	
	// Creates a particle
	cl_particle_t *CreateParticle( particle_system_t *psystem, Float *pflorigin = nullptr, Float *pflnormal = nullptr );
	// Creates environment particle first-time objects
	void EnvironmentCreateFirst( particle_system_t *psystem );

	// Removes a particle
	__forceinline void RemoveParticle( cl_particle_t *particle );
	// Updates a particle
	bool UpdateParticle( cl_particle_t *pparticle );
	// Checks collisions for a particle
	bool CheckCollision( Vector& vecOrigin, Vector& vecVelocity, particle_system_t* psystem, cl_particle_t *pparticle ) ;
	// Retreives lighting info for a particle
	__forceinline Vector LightForParticle( cl_particle_t *pparticle );

	// Retreives dynamic lights for a particle
	static void GetLights( particle_system_t *psystem, cl_dlight_t **plights, Uint32 *numlights, bool spotlight, Uint32 max );

private:
	// Checks water state for a particle
	__forceinline static Int32 CheckWater( const Vector& origin );
	// Clips a tracer particle
	__forceinline static bool ClipTracer( const Vector &start, const Vector &delta, Vector &clippedStart, Vector &clippedDelta );
	// Batches vertexes for a particle
	__forceinline void BatchVertex( cl_particle_t *pparticle, const Vector& vertex, Float alpha,  Int32 tc );
	// Batches a particle
	void BatchParticle( cl_particle_t *pparticle, Float flup, Float flright, const Float *pfltranspose );
	
	// Returns a position transformed by an entity's bone
	void GetBoneTransformedPosition( const Vector& baseposition, cl_entity_t* pentity, Int32 boneindex, Vector& outposition, bool reverse );
	// Returns a vector rotated by an entity's bone
	void GetBoneRotatedVector( const Vector& basevector, cl_entity_t* pentity, Int32 boneindex, Vector& outvector, bool reverse );
	// Returns relative vector
	void TransformRelativeVector( const Vector& basevector, particle_system_t *psystem, Vector& outvector, bool iscoordinate, bool inverse );

private:
	// Controls rendering of particles
	CCVar *m_pCvarDrawParticles;
	// Controls particle debug info printing
	CCVar *m_pCvarParticleDebug;
	// Pointer to gravity cvar
	CCVar *m_pCvarGravity;
	// Controls weather intensity
	CCVar *m_pWeatherDensity;

	// Linked list of particle systems
	CLinkedList<particle_system_t*> m_particleSystemsList;

private:
	// Array of active particle allocations
	cl_particle_t* m_pParticles;
	// Linked list of free particles
	cl_particle_t *m_pFreeParticles;

	// Number of freed particles
	Uint32 m_iNumFreedParticles;
	// Number of created particles
	Uint32 m_iNumCreatedParticles;

	// Number of freed systems
	Uint32 m_iNumFreedSystems;
	// Number of created systems
	Uint32 m_iNumCreatedSystems;

private:
	// Linked list of cached messages
	CLinkedList<part_msg_cache_t> m_msgCache;

	// Array of precached particle scripts
	CArray<script_cache_t*> m_scriptCache;

private:
	// Pointer to GLSL shader
	class CGLSLShader *m_pShader;
	// Pointer to VBO
	class CVBO *m_pVBO;

	// Shader attributes
	particle_attribs m_attribs;
	// Start vertex of screen rectangle
	Uint32 m_screenRectangleBase;

private:
	// Array holding particle vertexes
	particle_vertex_t* m_pVertexes;

	// Number of vertexes
	Int32 m_numVertexes;
	// Number of indexes
	Int32 m_numIndexes;
	// Number of particles
	Int32 m_numParticles;

	// View right vector
	Vector m_vRRight;
	// View up vector
	Vector m_vRUp;
};
extern CParticleEngine gParticleEngine;
#endif