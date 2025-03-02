/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef R_TRACERS_H
#define R_TRACERS_H

#include "tracer.h"

/*
=================================
CTracerRenderer

=================================
*/
class CTracerRenderer
{
public:
	// Allocation size for tracers
	static const Uint32 TRACER_ALLOC_SIZE;
	// Tracer texture file path
	static const Char TRACER_TEXTURE_PATH[];

public:
	CTracerRenderer( void );
	~CTracerRenderer( void );

public:
	// Initializes the class
	bool Init( void );
	// Shut down tracer system
	void Shutdown( void );

	// Initializes game objects
	bool InitGame( void );
	// Shuts down game objects
	void ClearGame( void );

public:
	// Draws the tracers
	bool DrawTracers( void );
	// Updates active tracers
	void Update( void );

	// Creates a tracer
	tracer_t* CreateTracer( const Vector& origin, const Vector& velocity, const Vector& color, Float alpha, Float width, Float length, Float life, tracer_type_t type );

	// Creates an implosion effect
	void CreateImplosionEffect( const Vector& destination, Float radius, Uint32 count, Float life, const Vector& color, Float alpha, Float width, Float length, bool reverse );
	// Creates a spark streak effect
	void CreateSparkStreak( const Vector& origin, Uint32 count, const Vector& color, Float alpha, Float width, Float length, Float minLifetime, Float maxLifetime, Float minVelocity, Float maxVelocity );
	// Creates a streak splash effect
	void CreateStreakSplash( const Vector& origin, const Vector& direction, const Vector& color, Float alpha, Float width, Float length, Uint32 count, Float speed, Float minLifetime, Float maxLifetime, Float minVelocity, Float maxVelocity );

private:
	// Allocates tracers
	void AllocTracers( void );
	// Allocate a single tracer
	tracer_t* AllocTracer( void );
	// Releases a tracer
	void FreeTracer( tracer_t* ptracer );

private:
	// Free particles header
	tracer_t* m_pFreeTracersHeader;
	// Active particles header
	tracer_t* m_pActiveTracersHeader;

	// Draw tracers cvar
	CCVar* m_pCvarDrawTracers;
	// Gravity cvar ptr
	CCVar* m_pCvarGravity;

	// Tracer texture
	en_texture_t* m_pTracerTexture;
};
extern CTracerRenderer gTracers;
#endif //R_TRACERS_H