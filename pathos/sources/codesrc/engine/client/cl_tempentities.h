/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef CL_TEMPENTITIES_H
#define CL_TEMPENTITIES_H

#include "tempentity.h"

class CCVar;

/*
=================================
-Class: CTempEntityManager
-Description:

=================================
*/
class CTempEntityManager
{
public:
	// Tempentity allocation size
	static const Uint32 TEMPENT_ALLOC_SIZE;
	// One shard every n^3 units
	static const Float SHARD_VOLUME;

public:
	CTempEntityManager( void );
	~CTempEntityManager( void );

public:
	// Initializes the class
	bool Init( void );
	// Shuts down the class
	void Shutdown( void );

	// Inits game objects
	bool InitGame( void );
	// Clears game objects
	void ClearGame( void );

public:
	// Allocates a tempentity
	tempentity_t* AllocTempEntity( const Vector& origin, const cache_model_t* pmodel );
	// Updates temporary entities
	void UpdateTempEntities( void );

	// Creates a sprite funnel effect
	void CreateFunnel( const Vector& origin, const Vector& color, Float alpha, Uint32 modelindex, bool reverse );
	// Creates a box of gib models
	void CreateBreakModel( const Vector& origin, const Vector& size, const Vector& velocity, Uint32 random, Float life, Uint32 num, Uint32 modelindex, Int32 sound, Float buoyancy, Float waterfriction, Int32 flags );
	// Creates a box of bubbles
	void CreateBubbles( const Vector& mins, const Vector& maxs, Float height, Uint32 modelindex, Uint32 num, Float speed );
	// Creates a trail of bubbles
	void CreateBubbleTrail( const Vector& start, const Vector& end, Float height, Uint32 modelindex, Uint32 num, Float speed );
	// Creates a spherical shower of models
	void CreateSphereModel( const Vector& origin, Float speed, Float life, Uint32 num, Uint32 modelindex, Float buoyancy, Float waterfriction, Int32 sound );
	// Creates a temporary model
	tempentity_t* CreateTempModel( const Vector& origin, const Vector& velocity, const Vector& angles, Float life, Uint32 modelindex, Int32 sound, Float buoyancy, Float waterfriction, Int32 flags );
	// Creates a temporary sprite
	tempentity_t* CreateTempSprite( const Vector& origin, const Vector& velocity, Float scale, Uint32 modelindex, Int32 rendermode, Int32 renderfx, Float alpha, Float life, Int32 sound, Int32 flags );

private:
	// Allocates new tempentity block
	void AllocateTempEntities( void );
	// Frees all active tempentities
	void FreeActiveTempEntities( void );
	// Frees a single tempentity
	void FreeTempEntity( tempentity_t* pfree );
	// Deletes all tempentities allocated
	void DeleteAllTempEntities( void );
	
	// Updates a single tempentity
	bool UpdateTempEntity( tempentity_t* ptemp ) const;

	// Plays a tempentity sound
	static void PlayTempEntitySound( tempentity_t *ptempentity, Float volume );

private:
	// Linked list of free tempentities
	tempentity_t* m_pFreeTempEntityHeader;
	// Linked list of acitve tempentities
	tempentity_t* m_pActiveTempEntityHeader;

	// Gravity CVar
	CCVar* m_pCvarGravity;
};
extern CTempEntityManager gTempEntities;
#endif //CL_TEMPENTITY_H