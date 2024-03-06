/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef R_BEAMS_H
#define R_BEAMS_H

#include "beam_shared.h"

//
// Beam rendering code
// Special Thanks to Enko for providing the sources I referenced to recode this
//

/*
=================================
CBeamRenderer

=================================
*/
class CBeamRenderer
{
public:
	// Number of beams allocated at once
	static const Uint32 BEAM_ALLOC_SIZE;
	// Number of beam positions allocated at once
	static const Uint32 BEAM_POSITIONS_ALLOC_SIZE;
	// Distance at which a new beam position is allocated for BEAMFOLLOW
	static const Float BEAM_POSITION_SEGMENT_DISTANCE;
	// Minimum number of beam segments
	static const Uint32 MIN_NB_BEAM_SEGMENTS;
	// Fraction of length at which end/start fades
	static const Float BEAM_FADE_FRACTION;

public:
	// Maximum number of segments for a beam
	static const Uint32 MAX_BEAM_SEGMENTS = 256;

public:
	CBeamRenderer( void );
	~CBeamRenderer( void );

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
	bool DrawBeams( void );
	// Updates active beams
	void Update( void );

private:
	// Allocates a block of beams
	void AllocateBeams( void );
	// Frees all active beams
	void FreeActiveBeams( void );
	// Releases a single beam
	void FreeBeam( beam_t* pbeam );
	// Allocates a beam
	beam_t* AllocBeam(  void );
	// Finds a beam by it's server entity a particle
	beam_t* FindBeamByEntity( const cl_entity_t* pentity );

	// Allocates a block of beam positions
	void AllocateBeamPositions( void );
	// Frees all active beam positions on a beam entity
	void FreeActiveBeamPositions( beam_t* pbeam );
	// Releases a single particle
	void FreeBeamPosition( beam_t* pbeam, beam_position_t* pposition );
	// Allocates a particle
	beam_position_t* AllocBeamPosition( beam_t* pbeam );

	// Does cull-testing on a beam
	bool CullBeam( const Vector& src, const Vector& end, bool pvsOnly );

	// Returns the sprite model handle
	cache_model_t* GetSpriteModel( Int32 modelindex );
	// Binds a sprite texture
	void BindSprite( cache_model_t* pmodel, beam_t* pbeam );

private:
	// Draws beam segments
	void DrawBeamSegments( const Vector& src, const Vector& delta, Float width, Float scale, Float frequency, Float speed, Float noisespeed, Uint32 numsegments, Int32 flags, Int32 beamindex );
	// Draws a beam torus
	void DrawBeamTorus( const Vector& src, const Vector& delta, Float width, Float scale, Float frequency, Float speed, Float noisespeed, Uint32 numsegments, Int32 beamindex, Int32 flags );
	// Draws a beam disk
	void DrawBeamDisk( const Vector& src, const Vector& delta, Float width, Float scale, Float frequency, Float speed, Float noisespeed, Uint32 numsegments );
	// Draws a beam cylinder
	void DrawBeamCylinder( const Vector& src, const Vector& delta, Float width, Float scale, Float frequency, Float speed, Float noisespeed, Uint32 numsegments );
	// Draws a beam ring
	void DrawBeamRing( const Vector& src, const Vector& delta, Float width, Float amplitude, Float frequency, Float speed, Float noisespeed, Uint32 numsegments, Int32 beamindex, Int32 flags );
	// Draws a tesla beam
	void DrawBeamTesla( const Vector& src, const Vector& delta, Float width, Float scale, Float frequency, Float speed, Float noisespeed, Float endwidth, Uint32 numsegments, Int32 flags, Int32 beamindex );
	// Draws a vapor trail beam
	void DrawBeamVaporTrail( beam_t* pbeam, Float fadealpha );
	// Draws a single beam
	void DrawBeam( beam_t* pbeam );
	// Draws a beam that follows an entity
	void DrawBeamFollow( beam_t* pbeam );

	// Calculates beam segment noise
	void ApplySegmentNoise( Vector& start, Uint32 beamindex, Uint32 i, Float scale, Uint32 numsegments, const Vector& up, const Vector& right, Int32 flags );

	// Draws vapor trail segments
	void DrawVaporTrailSegments( const CArray<beamsegment_t>& segments, Vector color, Float alpha, bool takelighting, Float fadealpha );

private:
	// Gets the entity this beam is tied to
	cl_entity_t* GetBeamAttachmentEntity( Int32 index );
	// Returns the beam's attachment point
	Vector GetBeamAttachmentPoint( cl_entity_t* pentity, Int32 attachment );
	// Sets up a single beam object
	bool BeamSetup( beam_t* pbeam, const Vector& src, const Vector& end, Int32 modelindex, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Int32 flags );
	// Sets a beam's attributes
	void SetBeamAttributes( beam_t* pbeam, Float r, Float g, Float b, Float framerate, Uint32 startframe );

public:
	// Sets up a lightning beam
	beam_t* BeamLightning( const Vector& src, const Vector& end, Int32 modelindex, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Int32 flags );
	// Sets up a beam which attaches to a random point in a circle
	beam_t* BeamCirclePoints( beam_types_t type, const Vector& src, const Vector& end, Int32 modelindex, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Uint32 startframe, Float framerate, Float r, Float g, Float b, Int32 flags );
	// Creates a beam between the starting entity and a point
	beam_t* BeamEntityPoint( entindex_t startentity, Int32 attachment, const Vector& end, Int32 modelindex, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Uint32 startframe, Float framerate, Float r, Float g, Float b, Int32 flags );
	// Creates a beam between two entities
	beam_t* BeamEntities( entindex_t startentity, entindex_t endentity, Int32 attachment1, Int32 attachment2, Int32 modelindex, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Uint32 startframe, Float framerate, Float r, Float g, Float b, Int32 flags );
	// Creates a beam that follows an entity
	beam_t* BeamFollow( entindex_t startentity, Int32 attachment, Int32 modelindex, Float life, Float width, Float brightness, Float r, Float g, Float b );
	// Creates a simple beam between two points
	beam_t* BeamPoints( const Vector& src, const Vector& end, Int32 modelindex, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Uint32 startframe, Float framerate, Float r, Float g, Float b, Int32 flags );
	// Creates a ring beam between two entities
	beam_t* BeamRing( entindex_t startentity, entindex_t endentity, Int32 attachment1, Int32 attachment2, Int32 modelindex, Float life, Float width, Float amplitude, Float brightness, Float speed, Float noisespeed, Uint32 startframe, Float framerate, Float r, Float g, Float b, Int32 flags );
	// Creates a vapor trail beam
	beam_t* BeamVaporTrail( const Vector& src, const Vector& end, Int32 modelindex1, Int32 modelindex2, Float colorfadedelay, Float colorfadetime, Float life, Float width, Float brightness, Float r1, Float g1, Float b1, Float r2, Float g2, Float b2, Int32 flags );

	// Kills beams on an entity
	void KillEntityBeams( entindex_t entityindex );
	// Adds a beam entity
	void AddBeamEntity( cl_entity_t* pentity );

private:
	// Free particles list header
	beam_t* m_pFreeBeamHeader;
	// Active particle header
	beam_t* m_pActiveBeamHeader;
	// Last beam index used
	Int32 m_lastBeamIndex;

	// Beam positions linked list, used by BEAMFOLLOW
	beam_position_t* m_pFreeBeamPositionsHeader;

	// BasicDraw instance
	CBasicDraw* m_pBasicDraw;

	// World to screen matrix
	CMatrix m_modelViewProjectionMatrix;

	// Cvar to toggle beam rendering
	CCVar* m_pCvarDrawBeams;
};
extern CBeamRenderer gBeamRenderer;
#endif //R_BEAMS_H