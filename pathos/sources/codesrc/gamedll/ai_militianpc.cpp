/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

// 2024 May 15th
// AI for hostile human NPCs who form militias, rely on some squad tactics,
// and speak among themselves

#include "includes.h"
#include "gd_includes.h"
#include "ai_militianpc.h"
#include "gamedll.h"
#include "sentencesfile.h"

// Array of clone soldier sentences
const Char* CMilitiaNPC::NPC_SENTENCE_POSTFIXES[NUM_NPC_SENTENCES] = 
{
	"_GRENADE",
	"_ALERT",
	"_COVER",
	"_THROW",
	"_CHARGE",
	"_TAUNT"
};

// Question asked
CMilitiaNPC::npc_question_types_t CMilitiaNPC::g_questionAsked = CMilitiaNPC::NPC_QUESTION_NONE;
// Time until we talk again
Double CMilitiaNPC::g_talkWaitTime = 0;
// NPC who asked the question
CBaseEntity* CMilitiaNPC::g_pAskerNPC = nullptr;

//==========================================================================
//
// SCHEDULES FOR CMILITIANPC CLASS
//
//==========================================================================

//=============================================
// @brief Establish line of fire
//
//=============================================
ai_task_t taskListScheduleMilitiaNPCEstablishLineOfFire[] = 
{
	AITASK(AI_TASK_SET_FAIL_SCHEDULE,						(Float)AI_SCHED_ELOF_FAIL),
	AITASK(AI_TASK_GET_PATH_TO_ENEMY,						0),
	AITASK(AI_MILITIANPC_TASK_SPEAK_SENTENCE,				0),
	AITASK(AI_TASK_RUN_PATH,								0),
	AITASK(AI_TASK_WAIT_FOR_MOVEMENT,						0),
	AITASK(AI_TASK_FACE_ENEMY,								0)
};

Uint32 interruptBitsScheduleMilitiaNPCEstablishLineOfFire[] =
{
	AI_COND_DANGEROUS_ENEMY_CLOSE,
	AI_COND_NEW_ENEMY,
	AI_COND_ENEMY_DEAD,
	AI_COND_CAN_RANGE_ATTACK1,
	AI_COND_CAN_RANGE_ATTACK2,
	AI_COND_CAN_MELEE_ATTACK1,
	AI_COND_CAN_MELEE_ATTACK2,
	AI_COND_CAN_SPECIAL_ATTACK1,
	AI_COND_CAN_SPECIAL_ATTACK2,
	AI_COND_IN_DANGER,
	AI_COND_HEARD_ENEMY_NEW_POSITION,
	AI_COND_HEAR_SOUND,
	AI_COND_SHOOT_VECTOR_VALID
};

Uint32 specialInterruptBitsScheduleMilitiaNPCEstablishLineOfFire[] =
{
	AI_COND_SHOOT_VECTOR_VALID
};

Uint32 specialInterruptExceptionBitsScheduleMilitiaNPCEstablishLineOfFire[] =
{
	AI_COND_DANGEROUS_ENEMY_CLOSE,
	AI_COND_NEW_ENEMY,
	AI_COND_ENEMY_DEAD,
	AI_COND_IN_DANGER,
	AI_COND_HEARD_ENEMY_NEW_POSITION,
	AI_COND_HEAR_SOUND
};

const CAISchedule scheduleMilitiaNPCEstablishLineOfFire(
	// Task list
	taskListScheduleMilitiaNPCEstablishLineOfFire, 
	// Number of tasks
	PT_ARRAYSIZE(taskListScheduleMilitiaNPCEstablishLineOfFire),
	// AI interrupt mask
	CBitSet(AI_COND_BITSET_SIZE, interruptBitsScheduleMilitiaNPCEstablishLineOfFire, PT_ARRAYSIZE(interruptBitsScheduleMilitiaNPCEstablishLineOfFire)),
	// Inverse interrupt mask
	CBitSet(AI_COND_BITSET_SIZE),
	// Special interrupt schedule
	AI_SCHED_FACE_ENEMY,
	// Special interrupt mask
	CBitSet(AI_COND_BITSET_SIZE, specialInterruptBitsScheduleMilitiaNPCEstablishLineOfFire, PT_ARRAYSIZE(specialInterruptBitsScheduleMilitiaNPCEstablishLineOfFire)),
	// Special interrupt exception mask
	CBitSet(AI_COND_BITSET_SIZE, specialInterruptExceptionBitsScheduleMilitiaNPCEstablishLineOfFire, PT_ARRAYSIZE(specialInterruptExceptionBitsScheduleMilitiaNPCEstablishLineOfFire)),
	// Special interrupt requirement mask
	CBitSet(AI_COND_BITSET_SIZE),
	// Sound mask
	AI_SOUND_DANGER, 
	// Name
	"Establish Line Of Fire for Militia NPC"
);

//==========================================================================
//
// SCHEDULES FOR CMILITIANPC CLASS
//
//==========================================================================

//=============================================
// @brief Constructor
//
//=============================================
CMilitiaNPC::CMilitiaNPC( edict_t* pedict ):
	CSquadNPC(pedict),
	m_sentence(0),
	m_voicePitch(PITCH_NORM)
{
}

//=============================================
// @brief Destructor
//
//=============================================
CMilitiaNPC::~CMilitiaNPC( void )
{
}

//=============================================
// @brief Declares save-restore fields
//
//=============================================
void CMilitiaNPC::DeclareSaveFields( void )
{
	CSquadNPC::DeclareSaveFields();

	DeclareSaveField(DEFINE_DATA_FIELD(CMilitiaNPC, m_voicePitch, EFIELD_UINT32));
	DeclareSaveField(DEFINE_DATA_FIELD(CMilitiaNPC, m_sentence, EFIELD_INT32));
}

//=============================================
// @brief Performs precache functions
//
//=============================================
void CMilitiaNPC::Precache( void )
{
	CSquadNPC::Precache();

	if(g_pSentencesFile)
	{
		for(Uint32 i = 0; i < NUM_NPC_SENTENCES; i++)
		{
			CString fullName;
			fullName << GetSentencePrefix() << NPC_SENTENCE_POSTFIXES[i];
			g_pSentencesFile->PrecacheGroup(fullName.c_str());
		}
	}
}

//=============================================
// @brief Starts a task
//
//=============================================
void CMilitiaNPC::StartTask( const ai_task_t* pTask )
{
	switch(pTask->task)
	{
	case AI_MILITIANPC_TASK_SPEAK_SENTENCE:
		{
			SpeakSentence();
			SetTaskCompleted();
		}
		break;
	default:
		CSquadNPC::StartTask(pTask);
		break;
	}
}

//=============================================
// @brief Plays idle sounds
//
//=============================================
void CMilitiaNPC::EmitIdleSound( void )
{
	if(!CanSpeak())
		return;
	
	if(g_questionAsked != NPC_QUESTION_NONE || Common::RandomLong(0, 1))
	{
		CString groupFullName;
		CString sentenceGroup;

		if(g_questionAsked == NPC_QUESTION_NONE)
		{
			switch(Common::RandomLong(0, 2))
			{
			case 0: // Check in request
				sentenceGroup = "_CHECK";
				g_questionAsked = NPC_QUESTION_CHECKIN;
				break;
			case 1: // Question
				sentenceGroup = "_QUEST";
				g_questionAsked = NPC_QUESTION_NORMAL;
				break;
			case 2: // Statement
				sentenceGroup = "_IDLE";
				g_questionAsked = NPC_QUESTION_IDLE;
				break;
			}

			groupFullName << GetSentencePrefix() << sentenceGroup;
			PlaySentence(groupFullName.c_str(), 0, VOL_NORM, ATTN_NORM, 0, true);

			// Mark who asked this
			g_pAskerNPC = this;
		}
		else if(g_pAskerNPC != this)
		{
			switch(g_questionAsked)
			{
			case NPC_QUESTION_CHECKIN: // Check in request
				sentenceGroup = "_CLEAR";
				break;
			case NPC_QUESTION_NORMAL: // Question
				sentenceGroup = "_ANSWER";
				break;
			}

			groupFullName << GetSentencePrefix() << sentenceGroup;
			PlaySentence(groupFullName.c_str(), 0, VOL_NORM, ATTN_NORM, 0, true);

			// Clear these
			g_questionAsked = NPC_QUESTION_NONE;
			g_pAskerNPC = nullptr;
		}

		Spoke();
	}
}

//=============================================
// @brief Speaks a sentence
//
//=============================================
void CMilitiaNPC::SpeakSentence( void )
{
	if(m_sentence == NPC_SENT_NONE || !CanSpeak())
		return;

	if(m_sentence < 0 || m_sentence >= NUM_NPC_SENTENCES)
	{
		Util::EntityConPrintf(m_pEdict, "Invalid sentence queued: %d.\n", m_sentence);
		return;
	}

	CString groupFullName;
	groupFullName << GetSentencePrefix() << NPC_SENTENCE_POSTFIXES[m_sentence];
	PlaySentence(groupFullName.c_str(), 0, VOL_NORM, ATTN_NORM, 0, true);
}

//=============================================
// @brief Tells if it's okay to speak
//
//=============================================
bool CMilitiaNPC::CanSpeak( void ) const
{
	if(g_talkWaitTime > g_pGameVars->time)
		return false;

	if(m_talkTime > g_pGameVars->time)
		return false;

	if(HasSpawnFlag(FL_NPC_GAG) && m_npcState != NPC_STATE_COMBAT)
		return false;

	return true;
}

//=============================================
// @brief Called when a clone soldier just spoke
//
//=============================================
void CMilitiaNPC::Spoke( void )
{
	g_talkWaitTime = Common::RandomFloat(1.5, 2.0);
	m_sentence = NPC_SENT_NONE;
}

//=============================================
// @brief Returns the voice pitch
//
//=============================================
Uint32 CMilitiaNPC::GetVoicePitch( void )
{
	return m_voicePitch;
}

//=============================================
// @brief Returns a schedule by it's index
//
//=============================================
const CAISchedule* CMilitiaNPC::GetScheduleByIndex( Int32 scheduleIndex )
{
	switch(scheduleIndex)
	{
	case AI_SCHED_ESTABLISH_LINE_OF_FIRE:
		{
			return &scheduleMilitiaNPCEstablishLineOfFire;
		}
		break;
	}

	return CSquadNPC::GetScheduleByIndex(scheduleIndex);
}

//=============================================
// @brief Resets talk
//
//=============================================
void CMilitiaNPC::Reset( void )
{
	g_talkWaitTime = 0;
	g_questionAsked = NPC_QUESTION_NONE;
}
