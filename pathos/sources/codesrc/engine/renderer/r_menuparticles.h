/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef R_MENUPARTICLES_H
#define R_MENUPARTICLES_H

struct en_texture_t;
class Vector;

/*
====================
CMenuParticles

====================
*/
class CMenuParticles
{
private:
	// Max particles live at once
	static const Uint32 MAX_PARTICLES = 512;
	// Max particles live at once
	static const Uint32 MAX_PARTICLE_VERTEXES = MAX_PARTICLES*6;
	// Number of frames in particle texture on x axis
	static const Uint32 NUM_PARTICLE_FRAMES_X;
	// Number of frames in particle texture on y axis
	static const Uint32 NUM_PARTICLE_FRAMES_Y;
	// Menu particle max life
	static const Float PARTICLE_MAX_LIFE;
	// Particle spawn base frequency
	static const Uint32 PARTICLE_SPAWN_FREQ;
	// Particle spawn frequency variation
	static const Uint32 PARTICLE_SPAWN_VARIATION;
	// Particle fade in/fade out time
	static const Float PARTICLE_FADE_TIME;

private:
	struct mparticle_t
	{
		mparticle_t():
			alpha(0),
			scale(0),
			gravity(0),
			wind(0),
			width(0),
			height(0),
			die(0),
			spawntime(0),
			mainalpha(0),
			noinfade(false)
			{
				memset(texcoords, 0, sizeof(texcoords));
			}

		Vector origin;
		Vector velocity;

		Vector color;
		Float alpha;
		Float scale;

		Float gravity;
		Float wind;

		Float texcoords[2];

		Uint32 width;
		Uint32 height;

		Float die;
		Double spawntime;
		Float mainalpha;
		bool noinfade;
	};

	struct mparticle_vertex_t
	{
		mparticle_vertex_t()
			{
				memset(origin, 0, sizeof(origin));
				memset(color, 0, sizeof(color));
				memset(texcoords, 0, sizeof(texcoords));
				memset(pad, 0, sizeof(pad));
			}

		vec4_t origin;
		vec4_t color;
		Float texcoords[2];
		byte pad[24];
	};

	struct mparticle_attribs_t
	{
		mparticle_attribs_t():
			a_vertex(CGLSLShader::PROPERTY_UNAVAILABLE),
			a_color(CGLSLShader::PROPERTY_UNAVAILABLE),
			a_texcoord(CGLSLShader::PROPERTY_UNAVAILABLE),
			u_modelview(CGLSLShader::PROPERTY_UNAVAILABLE),
			u_projection(CGLSLShader::PROPERTY_UNAVAILABLE),
			u_texture1(CGLSLShader::PROPERTY_UNAVAILABLE),
			u_texture2(CGLSLShader::PROPERTY_UNAVAILABLE)
			{
			}

		Int32 a_vertex;
		Int32 a_color;
		Int32 a_texcoord;

		Int32 u_modelview;
		Int32 u_projection;

		Int32 u_texture1;
		Int32 u_texture2;
	};

public:
	CMenuParticles( void );
	~CMenuParticles( void );

public:
	// Initializes the class
	bool Init( void );

	// Initializes the class
	bool InitGL( void );
	// Shuts down the class
	void ClearGL( void );

	// Draws the menu
	bool Draw( void );
	// Think function
	void Think( void );

	// Spawns menu particles
	void StartParticles( void );
	// Shuts down particles
	void KillParticles( void );

	// Returns any errors reported by the shader
	const Char* GetShaderError( void );

private:
	// Spawns a new particle
	bool SpawnParticle( Float xcoord, Float ycoord, bool noinfade = false );
	// Batches a vertex into the array
	void BatchVertex( const Vector& origin, const Vector& color, Float alpha, Float tcx, Float tcy);

private:
	// Linked list of particles
	CLinkedList<mparticle_t*> m_particlesList;

	// Alpha texture
	en_texture_t* m_pBgAlphaTexture;
	// Particle texture
	en_texture_t* m_pParticleTexture;

	// TRUE if particles are active
	bool m_isActive;

	// Next particle spawn time
	Double m_lastSpawnTime;

public:
	// GLSL shader
	class CGLSLShader *m_pShader;
	// VBO
	class CVBO *m_pVBO;

	// Black hole shader attributes
	mparticle_attribs_t m_attribs;

	// Vertex array for particles
	mparticle_vertex_t m_vertexesArray[MAX_PARTICLE_VERTEXES];
	// Number of batched vertexes
	Uint32 m_numBatchedVertexes;
};
extern CMenuParticles gMenuParticles;
#endif