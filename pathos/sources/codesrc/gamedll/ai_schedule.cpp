/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "ai_basenpc.h"
#include "ai_schedule.h"

//=============================================
// @brief
//
//=============================================
CAISchedule::CAISchedule( ai_task_t* pTasks, Uint32 nbTasks, Uint64 conditionMask, Uint64 soundMask, const Char* pstrName ):
	m_aiCondInterruptMask(conditionMask),
	m_aiCondInverseInterruptMask(AI_COND_NONE),
	m_specialInterruptSchedule(AI_SCHED_NONE),
	m_specialInterruptConditionMask(AI_COND_NONE),
	m_specialInterruptExceptionMask(AI_COND_NONE),
	m_soundMask(soundMask),
	m_scheduleName(pstrName)
{
	for(Uint32 i = 0; i < nbTasks; i++)
		m_tasksArray.push_back(pTasks[i]);
}

//=============================================
// @brief
//
//=============================================
CAISchedule::CAISchedule( ai_task_t* pTasks, Uint32 nbTasks, Uint64 conditionMask, Uint64 inverseConditionMask, Uint64 soundMask, const Char* pstrName ):
	m_aiCondInterruptMask(conditionMask),
	m_aiCondInverseInterruptMask(inverseConditionMask),
	m_specialInterruptSchedule(AI_SCHED_NONE),
	m_specialInterruptConditionMask(AI_COND_NONE),
	m_specialInterruptExceptionMask(AI_COND_NONE),
	m_soundMask(soundMask),
	m_scheduleName(pstrName)
{
	for(Uint32 i = 0; i < nbTasks; i++)
		m_tasksArray.push_back(pTasks[i]);
}

//=============================================
// @brief
//
//=============================================
CAISchedule::CAISchedule( ai_task_t* pTasks, Uint32 nbTasks, Uint64 conditionMask, Uint64 inverseConditionMask, Int32 specialInterruptSchedule, Uint64 specialInterruptConditionMask, Uint64 specialInterruptExceptionMask, Uint64 specialInterruptRequirementMask, Uint64 soundMask, const Char* pstrName ):
	m_aiCondInterruptMask(conditionMask),
	m_aiCondInverseInterruptMask(inverseConditionMask),
	m_specialInterruptSchedule(specialInterruptSchedule),
	m_specialInterruptConditionMask(specialInterruptConditionMask),
	m_specialInterruptExceptionMask(specialInterruptExceptionMask),
	m_specialInterruptRequirementMask(specialInterruptRequirementMask),
	m_soundMask(soundMask),
	m_scheduleName(pstrName)
{
	for(Uint32 i = 0; i < nbTasks; i++)
		m_tasksArray.push_back(pTasks[i]);
}

//=============================================
// @brief
//
//=============================================
CAISchedule::~CAISchedule( void )
{
}
