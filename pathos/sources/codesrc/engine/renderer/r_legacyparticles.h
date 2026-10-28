/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef R_LEGACYPARTICLES_H
#define R_LEGACYPARTICLES_H

class CCVar;

/*
=================================
CLegacyParticles

=================================
*/
class CLegacyParticles
{
public:
	// alloc count for legacy particles
	static const Uint32 LEGACY_PARTICLE_ALLOC_COUNT = 256;
	// Path to particle texture
	static const Char PARTICLE_TEXTURE_FILEPATH[];
	// Path to palette file
	static const Char PALETTE_FILEPATH[];

	// Ramp 1 values
	static const Uint32 PARTICLE_RAMP1[8];
	// Ramp 2 values
	static const Uint32 PARTICLE_RAMP2[8];
	// Ramp 3 values
	static const Uint32 PARTICLE_RAMP3[8];
	// Spark ramp values
	static const Uint32 PARTICLE_SPARK_RAMP[9];

	// Particle fade out time
	static const Float PARTICLE_FADEOUT_TIME;
	// Light check distance
	static const Float PARTICLE_LIGHT_CHECK_DISTANCE;

	enum particle_type_t
	{
		PT_UNDEFINED = 0,
		PT_STATIC,
		PT_GRAVITY,
		PT_SLOWGRAVITY,
		PT_FIRE,
		PT_EXPLODE1,
		PT_EXPLODE2,
		PT_BLOB1,
		PT_BLOB2,
		PT_VOX_GRAVITY,
		PT_VOX_SLOWGRAVITY,
		PT_FLICKER1,
		PT_FLICKER2
	};

	struct particle_t
	{
		particle_t():
			color(0),
			ramp(0),
			die(0),
			spawntime(0),
			type(PT_UNDEFINED),
			lighting(false),
			collisions(false),
			bouncefactor(1.0),
			pprev(nullptr),
			pnext(nullptr)
			{}

		Vector origin;
		Uint32 color;

		Vector velocity;
		Float ramp;
		Double die;
		Double spawntime;
		particle_type_t type;

		bool lighting;
		Vector lastlightposition;
		Vector lightcolor;

		bool collisions;
		Float bouncefactor;

		particle_t* pprev;
		particle_t* pnext;
	};

	enum legacymode_t
	{
		LPM_EXTENDED = 0,
		LPM_FAITHFUL
	};

public:
	CLegacyParticles( void );
	~CLegacyParticles( void );

public:
	// Initializes the class
	bool Init( void );
	// Shuts down the class
	void Shutdown( void );

	// Initializes OpenGL objects
	bool InitGL( void );
	// Shuts down OpenGL objects
	void ClearGL( void );

	// Initializes game objects
	bool InitGame( void );
	// Shuts down game objects
	void ClearGame( void );

	// Draws the particles
	bool DrawParticles( void );
	// Updates particles
	void UpdateParticles( void );

	// Creates a particle explosion of type 1
	void CreateParticleExplosion1( const Vector& origin );
	// Creates a particle explosion of type 2
	void CreateParticleExplosion2( const Vector& origin, Uint32 colorstart, Uint32 colorlength );
	// Creates a particle explosion of type 3
	void CreateParticleExplosion3( const Vector& origin );
	// Creates a blob explosion of type 1
	void CreateBlobExplosion( const Vector& origin );
	// Creates a particle effect based on the inputs
	void CreateRocketExplosion( const Vector& origin );
	// Creates a particle effect based on the inputs
	void CreateParticleEffect( const Vector& origin, const Vector& velocity, Uint32 color, Uint32 count );
	// Creates a lavasplash effect
	void CreateLavaSplash( const Vector& origin );
	// Creates a teleport splash effect
	void CreateTeleportSplash( const Vector& origin );
	// Creates a rocket trail effect
	void CreateRocketTrail( const Vector& start, const Vector& end, Uint32 type );
	// Creates a funnel effect
	void CreateLargeFunnel( const Vector& origin, bool reverse );
	// Creates a blood stream effect
	void CreateBloodStream( const Vector& origin, const Vector& direction, Uint32 color, Float speed );
	// Creates blood particles
	void CreateBloodParticles( const Vector& origin, const Vector& direction, Uint32 color, Float speed );
	// Create flicker particles
	void CreateFlickerParticles( const Vector& origin, const Vector& direction );
	// Create bullet impact particles
	void CreateBulletImpactParticles( const Vector& position, const Vector& direction );
	// Create particle burst effect
	void CreateParticleBurst( const Vector& position, Int32 size, Uint32 color, Float life );

private:
	// Allocates some particles
	void AllocateParticles( void );
	// Frees all active particles
	void FreeActiveParticles( void );
	// Releases a single particle
	void FreeParticle( particle_t* pparticle );
	// Allocates a particle
	particle_t* AllocParticle( void );
	// Releases all particles
	void ReleaseAllParticles( void );

	// Get legacy particle mode used
	legacymode_t GetLegacyMode( void );

private:
	// Particle texture
	en_texture_t* m_pParticleTexture;

	// Free particles list header
	particle_t* m_pFreeParticleHeader;
	// Active particle header
	particle_t* m_pActiveParticleHeader;

	// Particle allocation block ptrs
	CArray<particle_t*> m_pParticleBlocksArray;

	// Array of rendered particles
	CArray<particle_t*> m_pSortedParticles;
	// Number of sorted particles
	Uint32 m_nbSortedParticles;

	// Cvar controlling rendering
	CCVar* m_pCvarDrawLegacyParticles;
	// Cvar controlling legacy particle mode
	CCVar* m_pCvarLegacyParticleMode;
	// Gravity cvar ptr
	CCVar* m_pCvarGravity;

	// Color palette
	color24_t* m_pColorPalette;
	// Tracer count
	Uint32 m_tracerCount;
};
extern CLegacyParticles gLegacyParticles;
#endif //R_LEGACYPARTICLES_H