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
	// Max particles
	static const Uint32 MAX_LEGACY_PARTICLES = 4096;
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

	enum particle_type_t
	{
		pt_undefined = 0,
		pt_static,
		pt_gravity,
		pt_slowgravity,
		pt_fire,
		pt_explode1,
		pt_explode2,
		pt_blob1,
		pt_blob2
	};

	struct particle_t
	{
		particle_t():
			color(0),
			ramp(0),
			die(0),
			type(pt_undefined),
			pprev(nullptr),
			pnext(nullptr)
			{}

		Vector origin;
		Uint32 color;

		Vector velocity;
		Float ramp;
		Float die;
		particle_type_t type;

		particle_t* pprev;
		particle_t* pnext;
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
	// Creates a blob explosion of type 1
	void CreateBlobExplosion( const Vector& origin );
	// Creates a particle effect based on the inputs
	void CreateRocketExplosion( const Vector& origin, Uint32 color );
	// Creates a particle effect based on the inputs
	void CreateParticleEffect( const Vector& origin, const Vector& velocity, Uint32 color, Uint32 count );
	// Creates a lavasplash effect
	void CreateLavaSplash( const Vector& origin );
	// Creates a teleport splash effect
	void CreateTeleportSplash( const Vector& origin );
	// Creates a rocket trail effect
	void CreateRocketTrail( const Vector& start, const Vector& end, Uint32 type );

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

private:
	// Particle texture
	en_texture_t* m_pParticleTexture;

	// Free particles list header
	particle_t* m_pFreeParticleHeader;
	// Active particle header
	particle_t* m_pActiveParticleHeader;

	// Array of rendered particles
	CArray<particle_t*> m_pSortedParticles;
	// Number of sorted particles
	Uint32 m_nbSortedParticles;

	// Gravity cvar ptr
	CCVar* m_pCvarGravity;

	// Color palette
	color24_t* m_pColorPalette;
};
extern CLegacyParticles gLegacyParticles;
#endif //R_LEGACYPARTICLES_H