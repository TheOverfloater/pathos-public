/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef AI_SOUNDS_H
#define AI_SOUNDS_H

class CBaseEntity;

enum ai_soundtypes_t
{
	AI_SOUND_NONE		= 0,
	AI_SOUND_COMBAT		= (1<<0),
	AI_SOUND_WORLD		= (1<<1),
	AI_SOUND_PLAYER		= (1<<2),
	AI_SOUND_DANGER		= (1<<3)
};

struct ai_sound_t
{
	ai_sound_t():
		radius(0),
		volume(0),
		life(0),
		typeflags(0),
		identifier(0)
	{}

	Vector position;
	CEntityHandle emitter;

	Float radius;
	Float volume;
	Double life;
	Int32 typeflags;
	Uint32 identifier;
};

/*
=======================
CAISounds

=======================
*/
class CAISounds
{
public:
	CAISounds( void );
	~CAISounds( void );

public:
	ai_sound_t* GetSoundForEmitter( CBaseEntity* pEmitterEntity );
	ai_sound_t* AddSound( CBaseEntity* pEmitterEntity, Int32 typebits, Float radius, Float volume, Float duration );
	ai_sound_t* AddSound( Int32 typebits, const Vector& origin, Float radius, Float volume, Float duration );
	void GetSoundList( const Vector& position, CLinkedList<ai_sound_t>& destList, Uint32 soundMask, Float sensitivity );
	void ClearEmitterSounds( CBaseEntity* pEmitterEntity );

public:
	void Think( void );
	void Clear( void );

private:
	// Linked list of active sounds
	CLinkedList<ai_sound_t> m_soundsList;
	// Next free identifier
	Uint32 m_currentIdentifier;
};
extern CAISounds gAISounds;
#endif //AI_SOUNDS_H