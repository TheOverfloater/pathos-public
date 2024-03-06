/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef AI_SCHEDULE_H
#define AI_SCHEDULE_H

#include "ai_tasks.h"

//=============================================
//
//=============================================
class CAISchedule
{
public:
	CAISchedule( ai_task_t* pTasks, Uint32 nbTasks, Uint64 conditionMask, Uint64 soundMask, const Char* pstrName );
	CAISchedule( ai_task_t* pTasks, Uint32 nbTasks, Uint64 conditionMask, Uint64 inverseConditionMask, Uint64 soundMask, const Char* pstrName );
	CAISchedule( ai_task_t* pTasks, Uint32 nbTasks, Uint64 conditionMask, Uint64 inverseConditionMask, Int32 specialInterruptSchedule, Uint64 specialInterruptConditionMask, Uint64 specialInterruptExceptionMask, Uint64 specialInterruptRequirementMask, Uint64 soundMask, const Char* pstrName );
	~CAISchedule();

public:
	// Returns the number of tasks
	inline Uint32 GetNumTasks( void ) const;
	// Returns a task for an index
	inline const ai_task_t& GetTaskByIndex( Uint32 index ) const;
	// Returns the condition interrupt mask
	inline Uint64 GetInterruptMask( void ) const;
	// Returns the inverse condition interrupt mask
	inline Uint64 GetInverseInterruptMask( void ) const;
	// Returns the special interrupt schedule index
	inline Int32 GetSpecialInterruptScheduleIndex( void ) const;
	// Returns the special condition interrupt mask
	inline Uint64 GetSpecialInterruptMask( void ) const;
	// Returns the special condition interrupt exception mask
	inline Uint64 GetSpecialInterruptExceptionMask( void ) const;
	// Returns the special condition interrupt exception mask
	inline Uint64 GetSpecialInterruptRequirementMask( void ) const;
	// Returns the smell mask
	inline Uint64 GetSoundMask( void ) const;
	// Returns the name for the task
	inline const Char* GetName( void ) const;

private:
	// Array of tasks
	CArray<ai_task_t> m_tasksArray;
	// AI condition interrupt mask
	Uint64 m_aiCondInterruptMask;
	// AI condition inverse interrupt mask
	Uint64 m_aiCondInverseInterruptMask;
	// Special interrupt schedule index
	Int32 m_specialInterruptSchedule;
	// Special interrupt schedule condition mask
	Uint64 m_specialInterruptConditionMask;
	// Special interrupt schedule exception mask
	Uint64 m_specialInterruptExceptionMask;
	// Special interrupt requirement mask
	Uint64 m_specialInterruptRequirementMask;
	// Sound mask
	Uint64 m_soundMask;
	// Name of the schedule
	CString m_scheduleName;
};

#include "ai_schedule_inline.hpp"
#endif //AI_SCHEDULE_H