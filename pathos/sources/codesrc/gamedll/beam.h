/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef BEAM_H
#define BEAM_H

#include "delayentity.h"
#include "beam_shared.h"

//=============================================
//
//=============================================
class CBeam : public CBaseEntity
{
public:
	enum
	{
		FL_START_ON			= (1<<0),
		FL_TOGGLE			= (1<<1),
		FL_RANDOM			= (1<<2),
		FL_RING				= (1<<3),
		FL_START_SPARKS		= (1<<4),
		FL_END_SPARKS		= (1<<5),
		FL_DECAL_END		= (1<<6),
		FL_SHADE_START		= (1<<7),
		FL_SHADE_END		= (1<<8),
		FL_TEMPORARY		= (1<<9)
	};
public:
	explicit CBeam( edict_t* pedict );
	virtual ~CBeam( void );

public:
	// Spawns the entity
	virtual bool Spawn( void ) override;
	// Called after save-restoring an entity
	virtual bool Restore( void ) override;
	// Performs precache functions
	virtual void Precache( void ) override;
	// Calls for classes and their children
	virtual void DeclareSaveFields( void ) override;
	// Returns entity's flags
	virtual Int32 GetEntityFlags( void ) override;
	// Returns the entity's center
	virtual Vector GetCenter( void ) override;
	// Tells if entity should set bounds on restore
	virtual bool ShouldSetBoundsOnRestore( void ) override { return false; }

public:
	// Called when touched
	void EXPORTFN TriggerTouch( CBaseEntity* pOther );
	// Relinks the beam to the world
	void RelinkBeam( void );

	// Creates random sparks
	void BeamSparks( const Vector& start, const Vector& end );
	// Picks a random entity with the targetname
	CBaseEntity* GetRandomTargetName( const Char* pstrName );
	// Deals beam damage
	void BeamDamage( trace_t& tr );
	// Initializes a beam
	bool BeamInit( const Char* pstrSpriteName, Float width );
	// Initializes a points beam
	void BeamInitPoints( const Vector& start, const Vector& end );
	// Initializes a beam connecting two entities
	void BeamInitEntities( CBaseEntity* pStartEntity, CBaseEntity* pEndEntity );
	// Initializes a beam connecting an entity and a point
	void BeamInitPointEntity( CBaseEntity* pStartEntity, const Vector& end );

public:
	// Sets the beam type
	inline void SetBeamType( beam_msgtype_t type );
	// Returns the beam type
	inline beam_msgtype_t GetBeamType( void ) const;
	// Sets a beam flag
	inline void SetBeamFlags( Int32 flags );
	// Returns the beam flags
	inline Int32 GetBeamFlags( void ) const;
	// Sets the start position
	inline void SetBeamStartPosition( const Vector& position );
	// Returns the beam's starting position
	inline const Vector& GetBeamStartPosition( void ) const;
	// Sets the end position
	inline void SetBeamEndPosition( const Vector& position );
	// Returns the beam's starting position
	inline const Vector& GetBeamEndPosition( void ) const;

	// Sets the start entity attachment
	inline void SetBeamStartEntityAttachment( Int32 attachmentindex );
	// Returns the start entity attachment
	inline Int32 GetBeamStartEntityAttachment( void );
	// Sets the end entity attachment
	inline void SetBeamEndEntityAttachment( Int32 attachmentindex );
	// Returns the end entity attachment
	inline Int32 GetBeamEndEntityAttachment( void );

	// Sets the start entity
	inline void SetBeamStartEntity( CBaseEntity* pEntity );
	// Returns the start entity index
	inline CBaseEntity* GetBeamStartEntity( void );
	// Sets the end entity
	inline void SetBeamEndEntity( CBaseEntity* pEntity );
	// Returns the start entity index
	inline CBaseEntity* GetBeamEndEntity( void );

	// Sets the texture of the beam
	inline bool SetBeamTexture( Int32 spriteindex );
	// Sets the texture of the beam
	inline bool SetBeamTexture( const Char* pstrSpriteName );
	// Returns the sprite index
	inline Int32 GetBeamTexture( void ) const;
	// Sets the width of the beam
	inline void SetBeamWidth( Float width );
	// Returns the beam width
	inline Float GetBeamWidth( void ) const;
	// Sets the amplitude of the beam
	inline void SetBeamAmplitude( Float amplitude );
	// Returns the beam amplitude
	inline Float GetBeamAmplitude( void ) const;
	// Sets the color of the beam
	inline void SetBeamColor( Int32 r, Int32 g, Int32 b );
	// Returns the beam color
	inline const Vector& GetBeamColor( void ) const;
	// Sets the brightness of the beam
	inline void SetBeamBrightness( Float brightness );
	// Returns the beam brightness
	inline Float GetBeamBrightness( void ) const;
	// Sets the current frame
	inline void SetBeamFrame( Float frame );
	// Returns the beam frame
	inline Float GetBeamFrame( void ) const;
	// Sets the scroll rate
	inline void SetBeamScrollRate( Float speed );
	// Returns the scroll rate
	inline Float GetBeamScrollRate( void ) const;
	// Sets the noise speed
	inline void SetBeamNoiseSpeed( Float speed );
	// Returns the noise speed
	inline Float GetBeamNoiseSpeed( void ) const;

	// Sets the beam to die after the given amount of time
	inline void LiveForTime( Float time );
	// Deal instant damage
	inline void BeamDamageInstant( trace_t& tr, Float damage );

public:
	// Creates a beam object
	static CBeam* CreateBeam( const Char* pstrSpriteName, Float width );

protected:
	// Last time we were animated
	Double m_lastTime;
	// Max frame count
	Float m_maxFrame;
	// Damage dealt
	Float m_beamDamage;
	// Last damage time
	Double m_dmgTime;
	// Start entity
	CEntityHandle m_startEntity;
	// End entity
	CEntityHandle m_endEntity;
	// Beam end position
	Vector m_endPosition;
	// Start entity attachment index
	Int32 m_attachment1Index;
	// Start entity attachment index
	Int32 m_attachment2Index;
};
#include "beam_inline.hpp"
#endif //BEAM_H