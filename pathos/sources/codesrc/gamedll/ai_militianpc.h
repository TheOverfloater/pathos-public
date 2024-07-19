/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef AI_MILITIANPC_H
#define AI_MILITIANPC_H

#include "ai_squadnpc.h"

enum militianpc_tasks_t
{
	AI_MILITIANPC_TASK_SPEAK_SENTENCE = LAST_SQUADNPC_TASK + 1,

	// Must be last!
	LAST_MILITIANPC_TASK
};

//=============================================
//
//=============================================
class CMilitiaNPC : public CSquadNPC
{
public:
	// Question types
	enum npc_question_types_t
	{
		NPC_QUESTION_NONE = 0,
		NPC_QUESTION_CHECKIN,
		NPC_QUESTION_NORMAL,
		NPC_QUESTION_IDLE
	};

	enum npc_sentences_t
	{
		NPC_SENT_NONE = -1,
		NPC_SENT_GREN = 0,
		NPC_SENT_ALERT,
		NPC_SENT_COVER,
		NPC_SENT_THROW,
		NPC_SENT_CHARGE,
		NPC_SENT_TAUNT,

		NUM_NPC_SENTENCES
	};

	// Array of clone soldier sentences
	static const Char* NPC_SENTENCE_POSTFIXES[NUM_NPC_SENTENCES];

public:
	explicit CMilitiaNPC( edict_t* pedict );
	virtual ~CMilitiaNPC( void );

public:
	// Performs precache functions
	virtual void Precache( void ) override;
	// Declares save-restore fields
	virtual void DeclareSaveFields( void ) override;

public:
	// Starts a task
	virtual void StartTask( const ai_task_t* pTask ) override;
	// Plays idle sounds
	virtual void EmitIdleSound( void ) override;
	// Returns the voice pitch
	virtual Uint32 GetVoicePitch( void ) override;

	// Returns a schedule by it's index
	virtual const CAISchedule* GetScheduleByIndex( Int32 scheduleIndex ) override;

public:
	// Speaks a sentence
	void SpeakSentence( void );
	// Called when a clone soldier just spoke
	void Spoke( void );
	// Tells if it's okay to speak
	bool CanSpeak( void ) const;

public:
	// Get the sentence prefix
	virtual const Char* GetSentencePrefix( void ) = 0;

public:
	// Resets global npc states
	static void Reset( void );

protected:
	// Last sentence said
	Int32 m_sentence;
	// Voice pitch
	Uint32 m_voicePitch;

protected:
	// Question asked
	static npc_question_types_t g_questionAsked;
	// Time until we talk again
	static Double g_talkWaitTime;
	// NPC who asked
	static CBaseEntity* g_pAskerNPC;
};

#endif