/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef ANIMATINGENTITY_H
#define ANIMATINGENTITY_H

#include "delayentity.h"

struct mstudioevent_t;

//=============================================
//
//=============================================
class CAnimatingEntity : public CDelayEntity
{
public:
	explicit CAnimatingEntity( edict_t* pedict );
	virtual ~CAnimatingEntity( void );

public:
	// Spawns the entity
	virtual bool Spawn( void ) override;
	// Precaches model
	virtual void Precache( void ) override;
	// Declares the save fields
	virtual void DeclareSaveFields( void ) override;

	// Sets a sequence index as the current sequence
	virtual bool SetSequence( Int32 sequenceIndex ) override;
	// Sets a sequence by name
	virtual bool SetSequence( const Char* pstrSequenceName ) override;
	// Sets a sequence index as the current sequence
	virtual Int32 GetSequence( void ) override { return m_pState->sequence; }
	// Tells if a sequence is looped
	virtual bool IsSequenceLooped( const Char* pstrSequenceName ) override;
	// Resets animation states
	virtual void ResetSequenceInfo( void ) override;
	// Precaches sounds for a sequence
	virtual void PrecacheSequenceSounds( Int32 sequenceIndex ) override;
	// Precaches sounds for a sequence
	virtual void PrecacheSequenceSounds( Int32 modelIndex, Int32 sequenceIndex ) override;
	// Precaches sounds for a sequence
	virtual void PrecacheSequenceSounds( const Char* pstrSequenceName ) override;

	// Sets sequence blending
	virtual Float SetBlending( Int32 blenderIndex, Float value ) override;
	// Returns the number of sequence blenders
	virtual Uint32 GetNbBlenders( void ) override;
	// TRUE if entity is an animating entity
	virtual bool IsAnimatingEntity( void ) override { return true; }

	// Returns the bodygroup index by name
	virtual Int32 GetBodyGroupIndexByName( const Char* pstrBodygroupName ) override;
	// Returns the submodel index in a bodygroup by name
	virtual Int32 GetSubmodelIndexByName( Int32 bodygroupindex, const Char* pstrSubmodelName ) override;
	// Sets bodygroup setting
	virtual void SetBodyGroup( Int32 groupindex, Int32 value ) override;
	// Finds a sequence by name
	virtual Int32 FindSequence( const char* pstrsequencename ) override;
	// Returns sequence duration
	virtual Float GetSequenceTime( Int32 sequenceIndex ) override;

public:
	// Advances the frame
	virtual Float FrameAdvance( Double interval = 0, Double* pAnimFinishTime = nullptr );
	// Returns a sequence for an activity type
	virtual Int32 FindActivity( Int32 activity );
	// Returns the sequence with the heaviest activity weight
	virtual Int32 FindHeaviestActivity( Int32 activity );
	// Finds a transition animation
	virtual Int32 FindTransition( Uint32 endingsequence, Uint32 goalsequence, Int32* pdirection );

	// Returns the sequence flags
	virtual Int32 GetSequenceFlags( void );
	// Returns the number of frames in a sequence
	virtual Uint32 GetNumFrames( void );
	// Returns the model flags
	virtual Int32 GetModelFlags( void );

	// Gets attachment position
	virtual void GetAttachment( Uint32 attachmentindex, Vector& origin );
	// Gets bone position by index
	virtual bool GetBonePosition( Uint32 boneindex, Vector& origin );
	// Gets bone position by name
	virtual bool GetBonePosition( const Char* pstrbonename, Vector& origin );

	// Returns the bodygroup setting
	virtual Int32 GetBodyGroupValue( Int32 groupindex );

	// Sets the sequence bbox as the bounding box
	virtual bool SetSequenceBox( bool makeflat );
	// Returns the sequence duration
	virtual Float GetSequenceTime( const Char* pstrsequencename );
	// Returns the nb of sequences
	virtual Uint32 GetSequenceNumber( void );

	// Initializes bone controllers
	virtual void InitBoneControllers( void );
	// Sets bone controller
	virtual Float SetBoneController( Int32 controllerindex, Float value );

	// Manages animation events
	virtual void ManageAnimationEvents( void );
	// Handles animation events
	virtual void HandleAnimationEvent( const mstudioevent_t* pevent ) { };

	// Precaches sounds for a model index
	virtual void PrecacheModelSounds( Int32 modelIndex );

public:
	// Returns the sequence bounding box
	static bool GetSequenceBox( entity_state_t& state, Vector& mins, Vector& maxs, bool makeflat );
	// Extracts the bounding box for a sequence
	static bool ExtractBoundingBox( entity_state_t& state, Uint32 sequence, Vector& mins, Vector& maxs );

protected:
	// Framerate
	Float m_frameRate;
	// Ground speed
	Float m_groundSpeed;
	// Last frame we checked events at
	Float m_lastEventCheckFrame;

	// Tells if sequence is finished playing
	bool m_isSequenceFinished;
	// Tell
	bool m_isSequenceLooped;
};
#endif //NULLENTITY_H