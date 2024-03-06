/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef AI_SCHEDULE_INLINE_HPP
#define AI_SCHEDULE_INLINE_HPP

//=============================================
// @brief
//
//=============================================
inline Uint32 CAISchedule::GetNumTasks( void ) const
{
	return m_tasksArray.size();
}

//=============================================
// @brief
//
//=============================================
inline const ai_task_t& CAISchedule::GetTaskByIndex( Uint32 index ) const
{
	assert(index < m_tasksArray.size());
	return m_tasksArray[index];
}

//=============================================
// @brief
//
//=============================================
inline Uint64 CAISchedule::GetInterruptMask( void ) const
{
	return m_aiCondInterruptMask;
}

//=============================================
// @brief
//
//=============================================
inline Uint64 CAISchedule::GetInverseInterruptMask( void ) const
{
	return m_aiCondInverseInterruptMask;
}

//=============================================
// @brief
//
//=============================================
inline Int32 CAISchedule::GetSpecialInterruptScheduleIndex( void ) const
{
	return m_specialInterruptSchedule;
}

//=============================================
// @brief
//
//=============================================
inline Uint64 CAISchedule::GetSpecialInterruptMask( void ) const
{
	return m_specialInterruptConditionMask;
}

//=============================================
// @brief
//
//=============================================
inline Uint64 CAISchedule::GetSpecialInterruptExceptionMask( void ) const
{
	return m_specialInterruptExceptionMask;
}

//=============================================
// @brief
//
//=============================================
inline Uint64 CAISchedule::GetSpecialInterruptRequirementMask( void ) const
{
	return m_specialInterruptRequirementMask;
}


//=============================================
// @brief
//
//=============================================
inline Uint64 CAISchedule::GetSoundMask( void ) const
{
	return m_soundMask;
}

//=============================================
// @brief
//
//=============================================
inline const Char* CAISchedule::GetName( void ) const
{
	return m_scheduleName.c_str();
}
#endif //AI_SCHEDULE_INLINE_HPP