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
#include "ai_nodegraph.h"

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::IsTaskComplete( void ) const
{
	return (m_taskStatus == TASK_STATUS_COMPLETE) ? true : false;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::SetTaskFailed( bool allowRetry )
{
	SetConditions(AI_COND_TASK_FAILED);
	if(allowRetry)
		m_taskStatus = TASK_STATUS_FAILED;
	else
		m_taskStatus = TASK_STATUS_FAILED_NO_RETRY;
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::SetTaskCompleted( void )
{
	if(CheckConditions(AI_COND_TASK_FAILED))
		return;

	m_taskStatus = TASK_STATUS_COMPLETE;
}

//=============================================
// @brief
//
//=============================================
bool CBaseNPC::IsTaskRunning( void ) const
{
	if(m_taskStatus != TASK_STATUS_COMPLETE
		&& m_taskStatus != TASK_STATUS_RUNNING_MOVEMENT)
		return true;
	else
		return false;
}

//=============================================
// @brief
//
//=============================================
const ai_task_t* CBaseNPC::GetTask()
{
	if(m_scheduleTaskIndex < 0 || m_scheduleTaskIndex > (Int32)m_pSchedule->GetNumTasks())
		return nullptr;
	else
		return &m_pSchedule->GetTaskByIndex(m_scheduleTaskIndex);
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::TaskBegin( void )
{
	m_taskStatus = TASK_STATUS_RUNNING;
	m_updateYaw = false; // Reset this
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::StartTask( const ai_task_t* pTask )
{
	switch(pTask->task)
	{
	case AI_TASK_SET_CHECK_BEST_SOUND:
		{
			if(m_pBestSound)
			{
				m_currentCheckSound = (*m_pBestSound);
				m_checkSoundWasSet = true;
				SetTaskCompleted();
			}
			else
			{
				m_checkSoundWasSet = false;
				SetTaskFailed(false);
			}
		}
		break;
	case AI_TASK_WAIT_BLEND:
		{
			if(g_pGameVars->time - m_lastActivityTime >= pTask->param)
				SetTaskCompleted();
		}
		break;
	case AI_TASK_FIND_ENEMY_SEARCH_SPOT:
		{
			if(FindUnseenNode() || FindRandomSearchSpot())
				SetTaskCompleted();
			else
				SetTaskFailed(false);
		}
		break;
	case AI_TASK_CLEAR_BLOCKED_PATH:
		{
			if( !m_blockedNPC )
			{
				SetTaskFailed();
				break;
			}

			Vector vecBlockedMonsterDir = m_blockedNPCDestination - m_blockedNPC->GetOrigin();
			vecBlockedMonsterDir = vecBlockedMonsterDir.Normalize();
			Vector vecAngles = Math::VectorToAngles( vecBlockedMonsterDir );

			// Try to set up angles
			Vector forward, right;
			Math::AngleVectors( vecAngles, &forward, &right );
			Vector testDirections[] = { right, -right, forward };

			Float bestDist = 0;
			Vector bestLocation;

			for(Uint32 i = 0; i < 3; i++)
			{
				// Try to find an optimal distance
				Float travelDist;
				if( !m_blockedNPC->IsPlayer() && i == 2 )
				{
					travelDist = (m_blockedNPCDestination - m_blockedNPC->GetOrigin()).Length() + 40;
					if(travelDist < 64)
						travelDist = 64;
				}
				else
					travelDist = 512;

				// Try to trace to it
				Vector vecFinalDist;
				Float movedDistance = 0;
				WalkMoveTrace( m_pState->origin, testDirections[i], vecFinalDist, travelDist, movedDistance );

				// Make sure the distance is big enough
				if( movedDistance < 8 )
					continue;

				if( movedDistance > bestDist )
				{
					bestLocation = vecFinalDist;
					bestDist = travelDist;
				}
			}

			// Check if we can actually move there
			if(bestDist > 0 && MoveToLocation( ACT_RUN, 2, bestLocation ) || FindClearNode( nullptr ))
				SetTaskCompleted();
			else
				SetTaskFailed(false);
		}
		break;
	case AI_TASK_CLEAR_BLOCK_STATUS:
		{
			ClearConditions(AI_COND_BLOCKING_PATH);
			m_blockedNPC.reset();
			SetTaskCompleted();
		}
		break;
	case AI_TASK_TURN_RIGHT:
		{
			Float currentYaw = Math::AngleMod(m_pState->angles[YAW]);
			m_pState->idealyaw = Math::AngleMod(currentYaw - pTask->param);
			m_updateYaw = true;
			SetTurnActivity();
		}
		break;
	case AI_TASK_TURN_LEFT:
		{
			Float currentYaw = Math::AngleMod(m_pState->angles[YAW]);
			m_pState->idealyaw = Math::AngleMod(currentYaw + pTask->param);
			m_updateYaw = true;
			SetTurnActivity();
		}
		break;
	case AI_TASK_REMEMBER:
		{
			SetMemory((Uint64)pTask->param);
			SetTaskCompleted();
		}
		break;
	case AI_TASK_FORGET:
		{
			ClearMemory((Uint64)pTask->param);
			SetTaskCompleted();
		}
		break;
	case AI_TASK_FIND_HINT_NODE:
		{
			m_hintNodeIndex = FindHintNode();
			if(m_hintNodeIndex != NO_POSITION)
				SetTaskCompleted();
			else
				SetTaskFailed(false);
		}
		break;
	case AI_TASK_STORE_LAST_POSITION:
		{
			m_lastPosition = m_pState->origin;
			SetTaskCompleted();
		}
		break;
	case AI_TASK_CLEAR_LAST_POSITION:
		{
			m_lastPosition.Clear();
			SetTaskCompleted();
		}
		break;
	case AI_TASK_CLEAR_HINT_NODE:
		{
			m_hintNodeIndex = NO_POSITION;
			SetTaskCompleted();
		}
		break;
	case AI_TASK_STOP_MOVING:
		{
			if(GetIdealActivity() == m_movementActivity)
				StopMovement();

			ClearRoute();
			SetTaskCompleted();
		}
		break;
	case AI_TASK_PLAY_SEQUENCE_FACE_ENEMY:
	case AI_TASK_PLAY_SEQUENCE_FACE_TARGET:
		{
			m_updateYaw = true;
			SetIdealActivity((Int32)pTask->param);
		}
		break;
	case AI_TASK_PLAY_SEQUENCE:
		{
			SetIdealActivity((Int32)pTask->param);
		}
		break;
	case AI_TASK_PLAY_ACTIVE_IDLE:
		{
			if(m_hintNodeIndex == NO_POSITION)
			{
				SetTaskFailed(false);
				return;
			}

			const CAINodeGraph::node_t* pNode = gNodeGraph.GetNode(m_hintNodeIndex);
			if(!pNode)
			{
				SetTaskFailed(false);
				return;
			}

			SetIdealActivity((Int32)pNode->hintactivity);
		}
		break;
	case AI_TASK_SET_SCHEDULE:
		{
			const CAISchedule* pNewSchedule = GetScheduleByIndex((Int32)pTask->param);
			if(!pNewSchedule)
			{
				SetTaskFailed();
				return;
			}

			ChangeSchedule(pNewSchedule);
		}
		break;
	case AI_TASK_FIND_NEAR_NODE_COVER_FROM_ENEMY:
		{
			if(!m_enemy)
			{
				SetTaskFailed();
				return;
			}

			if(FindCover(m_enemy->GetOrigin(), m_enemy->GetViewOffset(), 0, pTask->param, m_enemy))
				SetTaskCompleted();
			else
				SetTaskFailed(false);
		}
		break;
	case AI_TASK_FIND_FAR_NODE_COVER_FROM_ENEMY:
		{
			if(!m_enemy)
			{
				SetTaskFailed();
				return;
			}

			if(FindCover(m_enemy->GetOrigin(), m_enemy->GetViewOffset(), pTask->param, GetCoverDistance(), m_enemy))
				SetTaskCompleted();
			else
				SetTaskFailed(false);
		}
		break;
	case AI_TASK_FIND_NODE_COVER_FROM_ENEMY:
		{
			if(!m_enemy)
			{
				SetTaskFailed();
				return;
			}

			if(FindCover(m_enemy->GetOrigin(), m_enemy->GetViewOffset(), 0, GetCoverDistance(), m_enemy))
				SetTaskCompleted();
			else
				SetTaskFailed(false);
		}
		break;
	case AI_TASK_FIND_COVER_FROM_ENEMY:
		{
			CBaseEntity* pCoverEntity = m_enemy ? m_enemy : this;

			Vector coverViewOffset = pCoverEntity->GetViewOffset();
			Vector coverOrigin = pCoverEntity->GetOrigin();
			if(FindLateralCover(coverOrigin, coverViewOffset) 
				&& (!m_dangerEntity || !m_dangerEntity->IsNPCDangerous())
				|| FindCover(coverOrigin, coverViewOffset, 0, GetCoverDistance(), m_enemy))
			{
				StartDangerCheck(coverOrigin, pCoverEntity, true);

				m_moveWaitFinishTime = g_pGameVars->time + pTask->param;
				SetTaskCompleted();
			}
			else
				SetTaskFailed(false);
		}
		break;
	case AI_TASK_FIND_RELOAD_COVER_SPOT:
		{
			CBaseEntity* pCoverEntity = m_enemy ? m_enemy : this;

			Vector coverViewOffset = pCoverEntity->GetViewOffset();
			Vector coverOrigin = pCoverEntity->GetOrigin();
			if(FindLateralCover(coverOrigin, coverViewOffset) 
				&& (!m_dangerEntity || !m_dangerEntity->IsNPCDangerous())
				|| FindCover(coverOrigin, coverViewOffset, 0, NPC_RELOAD_COVER_DISTANCE, m_enemy))
			{
				StartDangerCheck(coverOrigin, pCoverEntity, true);

				m_moveWaitFinishTime = g_pGameVars->time + pTask->param;
				SetTaskCompleted();
			}
			else
				SetTaskFailed(false);
		}
		break;
	case AI_TASK_FIND_COVER_FROM_ORIGIN:
		{
			if(FindCover(m_pState->origin, m_pState->view_offset, 0, GetCoverDistance(), nullptr))
			{
				StartDangerCheck(m_pState->origin, nullptr, true);

				m_moveWaitFinishTime = g_pGameVars->time + pTask->param;
				SetTaskCompleted();
			}
			else
				SetTaskFailed(false);
		}
		break;
	case AI_TASK_FIND_COVER_FROM_BEST_SOUND:
		{
			Vector destination; // FindCoverWithBestDistance will give a value to this
			if(m_pBestSound 
				&& FindCoverWithBestDistance(m_pBestSound->position, NPC_COVER_BESTSOUND_MIN_DISTANCE, NPC_COVER_BESTSOUND_MAX_DISTANCE, NPC_COVER_BESTSOUND_OPTIMAL_DISTANCE, destination) 
				&& MoveToLocation(ACT_RUN, 0, destination))
			{
				StartDangerCheck(m_pBestSound->position, m_pBestSound->emitter, true);

				m_moveWaitFinishTime = g_pGameVars->time + pTask->param;
				SetTaskCompleted();
			}
			else if(m_pBestSound
				&& FindCover(m_pBestSound->position, ZERO_VECTOR, m_pBestSound->radius, GetCoverDistance(), nullptr))
			{
				StartDangerCheck(m_pBestSound->position, m_pBestSound->emitter, true);

				m_moveWaitFinishTime = g_pGameVars->time + pTask->param;
				SetTaskCompleted();
			}
			else
				SetTaskFailed(false);
		}
		break;
	case AI_TASK_FACE_HINT_NODE:
		{
			if(m_hintNodeIndex == NO_POSITION)
			{
				SetTaskFailed();
				return;
			}

			const CAINodeGraph::node_t* pNode = gNodeGraph.GetNode(m_hintNodeIndex);
			if(!pNode)
			{
				SetTaskFailed();
				return;
			}

			m_updateYaw = true;
			m_pState->idealyaw = pNode->hintyaw;
			SetTurnActivity();
		}
		break;
	case AI_TASK_FACE_LAST_POSITION:
		{
			m_updateYaw = true;
			SetIdealYaw(m_lastPosition);
			SetTurnActivity();
		}
		break;
	case AI_TASK_FACE_TARGET:
		{
			if(!m_targetEntity)
			{
				SetTaskFailed();
				return;
			}

			m_updateYaw = true;
			SetIdealYaw(m_targetEntity->GetOrigin());
			SetTurnActivity();
		}
		break;
	case AI_TASK_FACE_ENEMY:
		{
			m_updateYaw = true;
			SetIdealYaw(m_enemyLastKnownPosition);
			SetTurnActivity();
		}
		break;
	case AI_TASK_FACE_PLAYER:
		{
			CBaseEntity* pPlayer = Util::GetHostPlayer();
			if(!pPlayer)
			{
				SetTaskFailed();
				return;
			}

			m_updateYaw = true;
			SetIdealYaw(pPlayer->GetOrigin());
			SetTurnActivity();
		}
		break;
	case AI_TASK_FACE_IDEAL:
		{
			m_updateYaw = true;
			SetTurnActivity();
		}
		break;
	case AI_TASK_FACE_ROUTE:
		{
			if(IsRouteClear())
			{
				Util::EntityConDPrintf(m_pEdict, "AI_TASK_FACE_ROUTE with no route!\n");
				SetTaskFailed();
				return;
			}

			m_updateYaw = true;
			SetIdealYaw(m_routePointsArray[m_routePointIndex].position);
			SetTurnActivity();
		}
		break;
	case AI_TASK_WAIT_PVS:
		break;
	case AI_TASK_WAIT_FACE_ENEMY_INDEFINITE:
		{
			m_updateYaw = true;
		}
		break;
	case AI_TASK_WAIT_INDEFINITE:
		break;
	case AI_TASK_WAIT:
		{
			m_waitFinishedTime = g_pGameVars->time + pTask->param;
		}
		break;
	case AI_TASK_WAIT_FACE_ENEMY:
		{
			m_updateYaw = true;
			m_waitFinishedTime = g_pGameVars->time + pTask->param;
		}
		break;
	case AI_TASK_WAIT_RANDOM:
		{
			m_waitFinishedTime = g_pGameVars->time + Common::RandomFloat(0.1, pTask->param);
		}
		break;
	case AI_TASK_MOVE_TO_TARGET_RANGE:
		{
			if(!m_targetEntity)
			{
				SetTaskFailed();
				return;
			}

			Vector targetPosition = m_targetEntity->GetNavigablePosition();
			Float targetDistance = (targetPosition - m_pState->origin).Length();
			if(targetDistance < pTask->param)
			{
				ClearConditions(AI_COND_FOLLOW_TARGET_TOO_FAR);
				SetTaskCompleted();
			}
			else
			{
				m_movementGoalPosition = targetPosition;
				if(!MoveToTarget(ACT_WALK, NPC_DEFAULT_MOVE_WAIT_TIME))
					SetTaskFailed(false);
			}
		}
		break;
	case AI_TASK_RUN_TO_TARGET:
	case AI_TASK_WALK_TO_TARGET:
		{
			if(!m_targetEntity)
			{
				SetTaskFailed();
				return;
			}

			Float targetDistance = (m_targetEntity->GetOrigin() - m_pState->origin).Length();
			if(targetDistance < 1.0f)
			{
				SetTaskCompleted();
				return;
			}

			Int32 activity;
			if(pTask->task == AI_TASK_WALK_TO_TARGET)
				activity = ACT_WALK;
			else
				activity = ACT_RUN;

			if(FindActivity(activity) == NO_SEQUENCE_VALUE)
			{
				SetTaskCompleted();
				return;
			}

			if(!m_targetEntity || !MoveToTarget((activity_t)activity, NPC_DEFAULT_MOVE_WAIT_TIME))
			{
				Util::EntityConDPrintf(m_pEdict, "Failed to reach target.\n");
				SetTaskFailed();
				ClearRoute();
			}
			else
			{
				// We have a path to the target
				SetTaskCompleted();
			}
		}
		break;
	case AI_TASK_CLEAR_MOVE_WAIT:
		{
			m_moveWaitFinishTime = g_pGameVars->time;
			SetTaskCompleted();
		}
		break;
	case AI_TASK_MELEE_ATTACK1_NO_TURN:
	case AI_TASK_MELEE_ATTACK1:
		{
			m_updateYaw = true;
			SetIdealActivity(ACT_MELEE_ATTACK1);
		}
		break;
	case AI_TASK_MELEE_ATTACK2_NO_TURN:
	case AI_TASK_MELEE_ATTACK2:
		{
			m_updateYaw = true;
			SetIdealActivity(ACT_MELEE_ATTACK2);
		}
		break;
	case AI_TASK_RANGE_ATTACK1_NO_TURN:
	case AI_TASK_RANGE_ATTACK1:
		{
			m_updateYaw = true;
			SetIdealActivity(ACT_RANGE_ATTACK1);
		}
		break;
	case AI_TASK_RANGE_ATTACK2_NO_TURN:
	case AI_TASK_RANGE_ATTACK2:
		{
			m_updateYaw = true;
			SetIdealActivity(ACT_RANGE_ATTACK2);
		}
		break;
	case AI_TASK_RELOAD_NO_TURN:
	case AI_TASK_RELOAD:
		{
			SetIdealActivity(ACT_RELOAD);
		}
		break;
	case AI_TASK_SPECIAL_ATTACK1:
		{
			SetIdealActivity(ACT_SPECIAL_ATTACK1);
		}
		break;
	case AI_TASK_SPECIAL_ATTACK2:
		{
			SetIdealActivity(ACT_SPECIAL_ATTACK2);
		}
		break;
	case AI_TASK_SET_ACTIVITY:
		{
			SetIdealActivity((Int32)pTask->param);
			SetTaskCompleted();
		}
		break;
	case AI_TASK_GET_PATH_TO_ENEMY:
		{
			if(!m_enemy)
			{
				SetTaskFailed();
				return;
			}

			Float minimumDistance = 0;
			bool disableMinDistance = (pTask->param == 1) ? true : false;
			if(!disableMinDistance)
				minimumDistance = GetMinimumRangeAttackDistance();

			if(!minimumDistance && BuildRoute(m_enemyLastKnownPosition, MF_TO_ENEMY, m_enemy))
			{
				SetTaskCompleted();
			}
			else
			{
				Float enemyLKPDistance = (m_enemyLastKnownPosition - m_pState->origin).Length();
				Vector enemyCenterOffset = (m_enemy->GetCenter()* 0.75 + m_enemy->GetEyePosition()*0.25) - m_enemy->GetOrigin();
				if(BuildNearestVisibleRoute(m_enemyLastKnownPosition, enemyCenterOffset, minimumDistance, enemyLKPDistance))
				{
					SetTaskCompleted();
				}
				else if(m_capabilityBits & (AI_CAP_RANGE_ATTACK1|AI_CAP_RANGE_ATTACK2)
					&& !CheckConditions(AI_COND_CAN_RANGE_ATTACK1|AI_COND_CAN_RANGE_ATTACK2)
					&& GetLateralShootingPosition(m_enemyLastKnownPosition + enemyCenterOffset))
				{
					Util::EntityConDPrintf(m_pEdict, "Found a lateral shooting position.\n");
					SetTaskCompleted();
				}
				else if(m_capabilityBits & (AI_CAP_RANGE_ATTACK1|AI_CAP_RANGE_ATTACK2)
					&& !CheckConditions(AI_COND_CAN_RANGE_ATTACK1|AI_COND_CAN_RANGE_ATTACK2)
					&& GetClosestShootingPosition(m_enemyLastKnownPosition + enemyCenterOffset))
				{
					Util::EntityConDPrintf(m_pEdict, "Found a closer shooting position.\n");
					SetTaskCompleted();
				}
				else
				{
					// Set enemy as not found it we can't see him
					if(!CheckConditions(AI_COND_SEE_ENEMY))
						SetConditions(AI_COND_ENEMY_NOT_FOUND);

					SetTaskFailed(false);
				}
			}
		}
		break;
	case AI_TASK_GET_PATH_TO_ENEMY_CORPSE:
		{
			Vector forward;
			Math::AngleVectors(m_pState->angles, &forward);

			Vector corpsePosition = m_enemyLastKnownPosition - forward * 64;
			if(BuildRoute(corpsePosition, MF_TO_LOCATION, nullptr))
				SetTaskCompleted();
			else
				SetTaskFailed(false);
		}
		break;
	case AI_TASK_GET_PATH_TO_SPOT:
		{
			CBaseEntity* pPlayer = Util::GetHostPlayer();
			if(BuildRoute(m_movementGoalPosition, MF_TO_LOCATION, pPlayer))
				SetTaskCompleted();
			else
				SetTaskFailed(false);
		}
		break;
	case AI_TASK_GET_PATH_TO_TARGET:
		{
			ClearRoute();
			if(m_targetEntity && MoveToTarget((activity_t)m_movementActivity, NPC_DEFAULT_MOVE_WAIT_TIME))
				SetTaskCompleted();
			else
				SetTaskFailed(false);
		}
		break;
	case AI_TASK_GET_PATH_TO_HINT_NODE:
		{
			if(m_hintNodeIndex == NO_POSITION)
			{
				SetTaskFailed();
				return;
			}

			const CAINodeGraph::node_t* pNode = gNodeGraph.GetNode(m_hintNodeIndex);
			if(!pNode)
			{
				SetTaskFailed();
				return;
			}

			if(MoveToLocation((activity_t)m_movementActivity, NPC_DEFAULT_MOVE_WAIT_TIME, pNode->origin))
				SetTaskCompleted();
			else
				SetTaskFailed(false);
		}
		break;
	case AI_TASK_GET_PATH_TO_LAST_POSITION:
		{
			// Set last position as movement goal
			m_movementGoalPosition = m_lastPosition;

			if(MoveToLocation((activity_t)m_movementActivity, NPC_DEFAULT_MOVE_WAIT_TIME, m_movementGoalPosition))
				SetTaskCompleted();
			else
				SetTaskFailed(false);
		}
		break;
	case AI_TASK_FACE_BEST_SOUND:
		{
			if(!m_checkSoundWasSet)
			{
				Util::EntityConPrintf(m_pEdict, "Best sound was not set for AI_TASK_FACE_BEST_SOUND.\n");
				SetTaskFailed();
				break;
			}

			if((m_currentCheckSound.position-m_pState->origin).Length2D() < 1.0f)
			{
				SetTaskCompleted();
				break;
			}

			m_updateYaw = true;
			SetIdealYaw(m_currentCheckSound.position);
			if(IsFacingIdealYaw())
				SetTaskCompleted();
		}
		break;
	case AI_TASK_GET_PATH_TO_BEST_SOUND:
		{
			if(!m_checkSoundWasSet)
			{
				Util::EntityConPrintf(m_pEdict, "Best sound was not set for AI_TASK_FACE_BEST_SOUND.\n");
				SetTaskFailed();
				break;
			}

			Float maxSoundDistance = (m_pState->origin - m_currentCheckSound.position).Length() * 2;
			if(BuildRoute(m_currentCheckSound.position, MF_TO_LOCATION, m_currentCheckSound.emitter)
				|| BuildNearestVisibleRoute(m_currentCheckSound.position, m_pState->view_offset, 0, maxSoundDistance)
				|| BuildNearestRoute(m_currentCheckSound.position, 0, maxSoundDistance))
				SetTaskCompleted();
			else
				SetTaskFailed(false);
		}
		break;
	case AI_TASK_RUN_PATH:
		{
			// Forget that we failed to dodge
			ClearMemory(AI_MEMORY_DODGE_ENEMY_FAILED);
			// Forget that we were in cover
			ClearMemory(AI_MEMORY_IN_COVER);
			// Forget about this too
			ClearMemory(AI_MEMORY_HIDING_SPOT_NOT_FOUND);

			// Set appropriate animation
			if(FindActivity(ACT_RUN) != NO_SEQUENCE_VALUE)
				m_movementActivity = ACT_RUN;
			else
				m_movementActivity = ACT_WALK;

			SetTaskCompleted();
		}
		break;
	case AI_TASK_WALK_PATH:
		{
			// Forget that we failed to dodge
			ClearMemory(AI_MEMORY_DODGE_ENEMY_FAILED);
			// Forget that we were in cover
			ClearMemory(AI_MEMORY_IN_COVER);
			// Forget about this too
			ClearMemory(AI_MEMORY_HIDING_SPOT_NOT_FOUND);

			// Set appropriate animation
			if(m_pState->movetype == MOVETYPE_FLY)
				m_movementActivity = ACT_FLY;
			else if(FindActivity(ACT_WALK) != NO_SEQUENCE_VALUE)
				m_movementActivity = ACT_WALK;
			else
				m_movementActivity = ACT_RUN;

			SetTaskCompleted();
		}
		break;
	case AI_TASK_WAIT_FOR_MOVEMENT:
		{
			if(IsRouteClear())
				SetTaskCompleted();
		}
		break;
	case AI_TASK_FLINCH:
		{
			SetIdealActivity(GetFlinchActivity());
		}
		break;
	case AI_TASK_DIE:
		{
			ClearRoute();
			SetIdealActivity(GetDeathActivity());
			m_pState->deadstate = DEADSTATE_DYING;

			// Make sure the bbox is of valid size
			SetSequenceBox(false);
		}
		break;
	case AI_TASK_SOUND_DIE:
		{
			switch(m_deathMode)
			{
			case DEATH_NORMAL:
				EmitDeathSound();
			case DEATH_DECAPITATED:
			default:
				break;
			}
			SetTaskCompleted();
		}
		break;
	case AI_TASK_SOUND_IDLE:
		{
			EmitIdleSound();
			SetTaskCompleted();
		}
		break;
	case AI_TASK_SOUND_PAIN:
		{
			EmitPainSound();
			SetTaskCompleted();
		}
		break;
	case AI_TASK_WAIT_FOR_SCRIPT:
		{
			if(m_pScriptedSequence->HasIdleAnimation())
			{
				m_pScriptedSequence->StartSequence(this, m_pScriptedSequence->GetIdleSequenceName(), false);

				// If play and idle anim names match, freeze animation
				const Char* pstrPlaySequenceName = m_pScriptedSequence->GetPlaySequenceName();
				const Char* pstrIdleSequenceName = m_pScriptedSequence->GetIdleSequenceName();
				if(pstrPlaySequenceName && pstrIdleSequenceName && 
					!qstrcmp(pstrPlaySequenceName, pstrIdleSequenceName))
					m_pState->framerate = 0;
			}
			else
			{
				// Just play the idle animation
				SetIdealActivity(ACT_IDLE);
			}
		}
		break;
	case AI_TASK_PLAY_SCRIPT:
		{
			m_pState->movetype = MOVETYPE_FLY;
			m_pState->flags &= ~FL_ONGROUND;
			m_scriptState = AI_SCRIPT_PLAYING;
		}
		break;
	case AI_TASK_ENABLE_SCRIPT:
		{
			m_pScriptedSequence->DelayStart(0);
			SetTaskCompleted();
		}
		break;
	case AI_TASK_PLANT_ON_SCRIPT:
		{
			if(m_targetEntity)
			{
				m_pState->origin = m_targetEntity->GetOrigin();
				GroundEntityNudge(true); // This will call SetOrigin
			}

			SetTaskCompleted();
		}
		break;
	case AI_TASK_FACE_SCRIPT:
		{
			if(m_targetEntity)
				m_pState->idealyaw = Math::AngleMod(m_targetEntity->GetAngles()[YAW]);

			SetTaskCompleted();
			SetIdealActivity(ACT_IDLE);
			ClearRoute();
		}
		break;
	case AI_TASK_DODGE_ENEMY:
		{
			Vector destination; // Will be filled by FindDodgeCover
			if(m_dangerousEnemy 
				&& FindDodgeCover(m_dangerousEnemy->GetOrigin(), NPC_DODGE_MIN_DISTANCE, NPC_DODGE_MAX_DISTANCE, destination) 
				&& MoveToLocation(ACT_RUN, 0, destination))
			{
				StartDangerCheck(m_dangerousEnemy->GetOrigin(), m_dangerousEnemy, true);
				SetTaskCompleted();
			}
			else
			{
				SetMemory(AI_MEMORY_DODGE_ENEMY_FAILED);
				SetTaskFailed(false);
			}
		}
		break;
	case AI_TASK_SUGGEST_STATE:
		{
			m_idealNPCState = (Int32)pTask->param;
			SetTaskCompleted();
		}
		break;
	case AI_TASK_SET_FAIL_SCHEDULE:
		{
			m_failureScheduleIndex = (Int32)pTask->param;
			SetTaskCompleted();
		}
		break;
	case AI_TASK_SET_NEXT_SCHEDULE:
		{
			m_nextScheduleIndex = (Int32)pTask->param;
			SetTaskCompleted();
		}
		break;
	case AI_TASK_CLEAR_FAIL_SCHEDULE:
		{
			m_failureScheduleIndex = AI_SCHED_NONE;
			SetTaskCompleted();
		}
		break;
	case AI_TASK_ALERT_SOUND:
		{
			EmitAlertSound();
			SetTaskCompleted();
		}
		break;
	case AI_TASK_ATTACK_REACTION_DELAY:
		{
			m_updateYaw = true;
			m_waitFinishedTime = g_pGameVars->time + GetReactionTime();
		}
		break;
	case AI_TASK_NPCPULLER_HOVER:
		{
			if(!m_npcPullerEntity)
			{
				Util::EntityConDPrintf(m_pEdict, "AI_TASK_NPCPULLER_HOVER in StartTask without puller set.\n");
				SetTaskFailed();
				break;
			}

			m_updateYaw = true;

			Vector vecFromPuller = (m_pState->origin - m_npcPullerPosition).Normalize();
			SetIdealYaw(vecFromPuller, false);
		}
		break;
	case AI_TASK_GET_PATH_AWAY_FROM_NPCPULLER:
		{
			if(!m_npcPullerEntity)
			{
				Util::EntityConDPrintf(m_pEdict, "AI_TASK_GET_PATH_AWAY_FROM_NPCPULLER without puller set.\n");
				SetTaskFailed();
				break;
			}

			Vector flatPullerPosition = m_npcPullerPosition;
			flatPullerPosition[2] = m_pState->origin[2];

			Vector targetDirection = (m_pState->origin - flatPullerPosition).Normalize();

			// Try to trace to it
			Vector finalPosition;
			Float movedDistance = 0;
			WalkMoveTrace( m_pState->origin, targetDirection, finalPosition, 1024, movedDistance );
			if(movedDistance < 64)
			{
				Util::EntityConDPrintf(m_pEdict, "Failure trying to move away from puller.\n");
				m_npcPullerEntity->RemovePulledNPC(this);
				m_npcPullerEntity.reset();
				SetTaskFailed();
				break;
			}

			if(MoveToLocation( ACT_RUN, 2, finalPosition ))
			{
				SetTaskCompleted();
			}
			else
			{
				m_npcPullerEntity->RemovePulledNPC(this);
				m_npcPullerEntity.reset();
				SetTaskFailed();
			}
		}
		break;
	default:
		{
			Util::EntityConPrintf(m_pEdict, "No StartTask case for '%d'.\n", (Int32)pTask->task);
		}
		break;
	}
}

//=============================================
// @brief
//
//=============================================
void CBaseNPC::RunTask( const ai_task_t* pTask )
{
	switch(pTask->task)
	{
	case AI_TASK_NPCPULLER_HOVER:
		{
			if(!m_npcPullerEntity)
			{
				SetTaskCompleted();
				break;
			}

			Vector vecFromPuller = (m_pState->origin - m_npcPullerPosition).Normalize();
			SetIdealYaw(vecFromPuller, false);
		}
		break;
	case AI_TASK_WAIT_BLEND:
		{
			if(g_pGameVars->time - m_lastActivityTime >= pTask->param)
				SetTaskCompleted();
		}
		break;
	case AI_TASK_TURN_RIGHT:
	case AI_TASK_TURN_LEFT:
		{
			if(IsFacingIdealYaw())
				SetTaskCompleted();
		}
		break;
	case AI_TASK_PLAY_SEQUENCE_FACE_ENEMY:
	case AI_TASK_PLAY_SEQUENCE_FACE_TARGET:
		{
			CBaseEntity* pTargetEntity;
			if(pTask->task == AI_TASK_PLAY_SEQUENCE_FACE_ENEMY)
				pTargetEntity = m_enemy;
			else
				pTargetEntity = m_targetEntity;

			if(pTargetEntity)
				SetIdealYaw(pTargetEntity->GetOrigin());

			if(m_isSequenceFinished)
				SetTaskCompleted();
		}
		break;
	case AI_TASK_PLAY_SEQUENCE:
	case AI_TASK_PLAY_ACTIVE_IDLE:
		{
			if(m_isSequenceFinished)
				SetTaskCompleted();
		}
		break;
	case AI_TASK_FACE_ENEMY:
		{
			SetIdealYaw(m_enemyLastKnownPosition);

			if(IsFacingIdealYaw())
				SetTaskCompleted();
		}
		break;
	case AI_TASK_FACE_PLAYER:
		{
			CBaseEntity* pPlayer = Util::GetHostPlayer();
			if(!pPlayer)
			{
				SetTaskFailed();
				return;
			}

			SetIdealYaw(pPlayer->GetOrigin());

			if(IsFacingIdealYaw())
				SetTaskCompleted();
		}
		break;
	case AI_TASK_FACE_HINT_NODE:
	case AI_TASK_FACE_LAST_POSITION:
	case AI_TASK_FACE_TARGET:
	case AI_TASK_FACE_IDEAL:
	case AI_TASK_FACE_ROUTE:
		{
			if(IsFacingIdealYaw())
				SetTaskCompleted();
		}
		break;
	case AI_TASK_FACE_BEST_SOUND:
		{
			if(!m_checkSoundWasSet)
			{
				Util::EntityConPrintf(m_pEdict, "Best sound was not set for AI_TASK_FACE_BEST_SOUND.\n");
				SetTaskFailed();
				break;
			}

			SetIdealYaw(m_currentCheckSound.position);
			if(IsFacingIdealYaw())
				SetTaskCompleted();
		}
		break;
	case AI_TASK_WAIT_PVS:
		{
			// Do not wait for PVS if following
			if(m_npcState == NPC_STATE_COMBAT || IsFollowing() || gd_engfuncs.pfnFindClientInPVS(m_pEdict) != nullptr)
				SetTaskCompleted();
		}
		break;
	case AI_TASK_WAIT_INDEFINITE:
		{
		}
		break;
	case AI_TASK_WAIT:
	case AI_TASK_WAIT_RANDOM:
		{
			if(m_waitFinishedTime <= g_pGameVars->time)
				SetTaskCompleted();
		}
		break;
	case AI_TASK_ATTACK_REACTION_DELAY:
	case AI_TASK_WAIT_FACE_ENEMY:
		{
			SetIdealYaw(m_enemyLastKnownPosition);

			if(m_waitFinishedTime <= g_pGameVars->time)
				SetTaskCompleted();
		}
		break;
	case AI_TASK_WAIT_FACE_ENEMY_INDEFINITE:
		{
			SetIdealYaw(m_enemyLastKnownPosition);
		}
		break;
	case AI_TASK_MOVE_TO_TARGET_RANGE:
		{
			if(IsMovementComplete())
			{
				ClearRoute();
				SetTaskCompleted();
				return;
			}

			if(!m_targetEntity)
			{
				ClearRoute();
				SetTaskFailed();
				return;
			}

			// Re-evaluate chase target
			Float targetDistance = GetRouteLength();
			Float targetGoalDistance = (m_movementGoalPosition - m_targetEntity->GetOrigin()).Length();
			if(targetDistance < pTask->param || targetGoalDistance > pTask->param*0.5)
			{
				Vector targetNavigableOrigin = m_targetEntity->GetNavigablePosition();

				if(!UpdateRoute(m_targetEntity, targetNavigableOrigin))
				{
					m_movementGoalPosition = targetNavigableOrigin;
					RefreshRoute();
				}
				else
				{
					// Show the updated path
					ShowRoute(false, MAX_ROUTE_POINTS, m_movementGoalPosition);
				}

				// Update this
				targetDistance = GetRouteLength();
			}

			// Check if we're in the range
			if(targetDistance < pTask->param)
			{
				ClearConditions(AI_COND_FOLLOW_TARGET_TOO_FAR);
				SetTaskCompleted();
				ClearRoute();
			}
			else if(targetDistance < NPC_FOLLOW_WALK_DISTANCE && m_movementActivity != ACT_WALK)
			{
				// If below the treshold, then walk
				m_movementActivity = ACT_WALK;
			}
			else if(targetDistance >= NPC_FOLLOW_RUN_DISTANCE && m_movementActivity != ACT_RUN)
			{
				// If above the treshold, then run
				m_movementActivity = ACT_RUN;
			}
		}
		break;
	case AI_TASK_WAIT_FOR_MOVEMENT:
		{
			if(IsMovementComplete())
			{
				SetTaskCompleted();
				ClearRoute();
			}
		}
		break;
	case AI_TASK_DIE:
		{
			if(m_isSequenceFinished && m_pState->frame >= 255)
			{
				m_pState->deadstate = DEADSTATE_DEAD;
				m_pState->flags |= FL_DEAD;

				SetThink(&CBaseNPC::CallNPCDeadThink);
				m_pState->nextthink = g_pGameVars->time + NPC_THINK_TIME;
				StopAnimation();
			}
		}
		break;
	case AI_TASK_RANGE_ATTACK1_NO_TURN:
	case AI_TASK_RANGE_ATTACK2_NO_TURN:
	case AI_TASK_MELEE_ATTACK1_NO_TURN:
	case AI_TASK_MELEE_ATTACK2_NO_TURN:
	case AI_TASK_RELOAD_NO_TURN:
		{
			if(m_isSequenceFinished)
			{
				m_currentActivity = ACT_RESET;
				SetTaskCompleted();
			}
		}
		break;
	case AI_TASK_RANGE_ATTACK1:
	case AI_TASK_RANGE_ATTACK2:
	case AI_TASK_MELEE_ATTACK1:
	case AI_TASK_MELEE_ATTACK2:
	case AI_TASK_SPECIAL_ATTACK1:
	case AI_TASK_SPECIAL_ATTACK2:
	case AI_TASK_RELOAD:
		{
			SetIdealYaw(m_enemyLastKnownPosition);

			if(m_isSequenceFinished)
			{
				m_currentActivity = ACT_RESET;
				SetTaskCompleted();
			}
		}
		break;
	case AI_TASK_FLINCH:
		{
			if(m_isSequenceFinished)
				SetTaskCompleted();
		}
		break;
	case AI_TASK_WAIT_FOR_SCRIPT:
		{
			if(m_pScriptedSequence->GetScriptDelay() <= 0 && !m_pScriptedSequence->IsWaitingToBeTriggered())
			{
				script_loop_t loopState = m_pScriptedSequence->GetLoopState();

				const Char* pstrSequenceName = nullptr;
				switch(loopState)
				{
				case SCRIPT_LOOP_PLAYING_LOOP:
					pstrSequenceName = m_pScriptedSequence->GetLoopSequenceName();
					break;
				case SCRIPT_LOOP_PLAYING_EXIT:
					pstrSequenceName = m_pScriptedSequence->GetExitSequenceName();
					break;
				default:
				case SCRIPT_LOOP_INACTIVE:
					pstrSequenceName = m_pScriptedSequence->GetPlaySequenceName();
					break;
				}

				SetTaskCompleted();
				m_pScriptedSequence->StartSequence(this, pstrSequenceName, true);
				if(m_isSequenceFinished)
					ClearSchedule();

				m_pState->framerate = 1.0;
			}
		}
		break;
	case AI_TASK_PLAY_SCRIPT:
		{
			if(m_isSequenceFinished)
				m_pScriptedSequence->SetSequenceDone(this);
		}
		break;
	}
}