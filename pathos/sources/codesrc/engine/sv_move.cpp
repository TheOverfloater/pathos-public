/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "sv_main.h"
#include "sv_physics.h"
#include "sv_world.h"
#include "com_math.h"
#include "edictmanager.h"
#include "enginestate.h"

//=============================================
//
//=============================================
bool SV_NPC_MoveTest( edict_t* pedict, const Vector& move, bool relink )
{
	Vector oldorigin = pedict->state.origin;
	Vector neworigin = pedict->state.origin + move;

	// Test for slopes, etc
	neworigin[2] += g_psv_stepsize->GetValue();
	Vector end = neworigin;
	end[2] -= g_psv_stepsize->GetValue() * 2;

	// Test the origin
	trace_t tr;
	SV_MoveNoEntities(tr, neworigin, pedict->state.mins, pedict->state.maxs, end, FL_TRACE_NORMAL, pedict);
	if(tr.allSolid())
		return false;

	if(tr.startSolid())
	{
		// Try moving down origin a bit
		neworigin[2] -= g_psv_stepsize->GetValue();
		SV_MoveNoEntities(tr, neworigin, pedict->state.mins, pedict->state.maxs, end, FL_TRACE_NORMAL, pedict);

		if(tr.allSolid() || tr.startSolid())
			return false;
	}

	if(tr.noHit())
	{
		// If npc had the ground pulled out, fall
		if(pedict->state.flags & FL_PARTIALGROUND)
		{
			Math::VectorAdd(pedict->state.origin, move, pedict->state.origin);

			if(relink)
				SV_LinkEdict(pedict, true);

			pedict->state.flags &= ~FL_ONGROUND;
			return true;
		}
		
		// Walked off an edge
		return false;
	}

	// Set endpos from trace as origin
	pedict->state.origin = tr.endpos;

	if(!SV_CheckBottom(pedict))
	{
		if(!(pedict->state.flags & FL_PARTIALGROUND))
		{
			pedict->state.origin = oldorigin;
			return false;
		}
	}
	else
	{
		// Set to fully grounded if partial
		if(pedict->state.flags & FL_PARTIALGROUND)
			pedict->state.flags &= ~FL_PARTIALGROUND;

		pedict->state.groundent = tr.hitentity;
	}

	if(relink)
		SV_LinkEdict(pedict, true);

	return true;
}

//=============================================
//
//=============================================
bool SV_NPC_MoveStep( edict_t* pedict, const Vector& move, bool relink, bool nonpcs )
{
	Vector oldorigin = pedict->state.origin;
	Vector neworigin = pedict->state.origin + move;

	Int32 traceflags = FL_TRACE_NORMAL;
	if(pedict->state.flags & FL_NPC_CLIP)
		traceflags |= FL_TRACE_NPC_CLIP;
	if(nonpcs)
		traceflags |= FL_TRACE_NO_NPCS;

	// Flying npcs don't step up
	if(pedict->state.flags & (FL_FLY|FL_SWIM))
	{
		for(Uint32 i = 0; i < 2; i++)
		{
			neworigin = pedict->state.origin + move;

			if(i == 0 && pedict->state.enemy != NO_ENTITY_INDEX)
			{
				edict_t* penemy = gEdicts.GetEdict(pedict->state.enemy);
				if(penemy)
				{
					Float dz = pedict->state.origin[2] - penemy->state.origin[2];
					if(dz > 40)
						neworigin[2] -= 8.0f;
					else
						neworigin[2] += 8.0f;
				}
			}

			trace_t tr;
			SV_Move(tr, pedict->state.origin, pedict->state.mins, pedict->state.maxs, neworigin, traceflags, pedict);

			if(tr.noHit())
			{
				ens.tr_groupmask = pedict->state.groupinfo;

				// Check if the NPC left the water
				if((pedict->state.flags & FL_SWIM) && SV_PointContents(tr.endpos) == CONTENTS_EMPTY)
					return false;

				pedict->state.origin = tr.endpos;

				if(relink)
					SV_LinkEdict(pedict, true);

				return true;
			}

			if(pedict->state.enemy == NO_ENTITY_INDEX)
				break;
		}

		return false;
	}
	else
	{
		// Test for slopes, etc
		neworigin[2] += g_psv_stepsize->GetValue();
		Vector end = neworigin;
		end[2] -= g_psv_stepsize->GetValue() * 2;

		// Test the origin
		trace_t tr;
		SV_Move(tr, neworigin, pedict->state.mins, pedict->state.maxs, end, traceflags, pedict);
		if(tr.allSolid())
			return false;

		if(tr.startSolid())
		{
			// Try moving down origin a bit
			neworigin[2] -= g_psv_stepsize->GetValue();
			SV_Move(tr, neworigin, pedict->state.mins, pedict->state.maxs, end, traceflags, pedict);

			if(tr.allSolid() || tr.startSolid())
				return false;
		}

		if(tr.noHit())
		{
			// If npc had the ground pulled out, fall
			if(pedict->state.flags & FL_PARTIALGROUND)
			{
				Math::VectorAdd(pedict->state.origin, move, pedict->state.origin);

				if(relink)
					SV_LinkEdict(pedict, true);

				pedict->state.flags &= ~FL_ONGROUND;
				return true;
			}
		
			// Walked off an edge
			pedict->state.origin = oldorigin;
			return false;
		}
		else
		{
			// Test a line between the two points
			if(!move.IsZero())
			{
				// If we're on the same plane
				Float zdiff = SDL_fabs(oldorigin.z - tr.endpos.z);
				if(zdiff < 0.1f)
				{
					trace_t trmove;
					Vector trEnd = tr.endpos;
					Vector trStart = oldorigin;
					
					// Avoid issues due to really tiny elevation
					// differences
					if(zdiff > 0)
					{
						if(trEnd.z > trStart.z)
							trStart.z = trEnd.z;
						else if(trStart.z > trEnd.z)
							trEnd.z = trStart.z;
					}

					SV_Move(trmove, trStart, pedict->state.mins, pedict->state.maxs, trEnd, traceflags, pedict);
					if(trmove.allSolid() || trmove.startSolid() || !trmove.noHit())
						return false;
				}
				else
				{
					Vector moveDirection = move;
					moveDirection.Normalize();

					Vector goalPosition = tr.endpos;
					Float stepSize = g_psv_stepsize->GetValue();
					Float distanceToGoal = (goalPosition - oldorigin).Length2D();

					Vector verticalDirection = (oldorigin.z < goalPosition.z) ? Vector(0, 0, 1) : Vector(0, 0, -1);
					Vector currentPosition = oldorigin;

					while(distanceToGoal > 0)
					{
						trace_t trmove;
						Vector testPosition;

						Float testDistance = distanceToGoal > stepSize ? stepSize : distanceToGoal;
						if(verticalDirection.z > 0)
						{
							Vector offsetPosition = currentPosition + verticalDirection * stepSize;
							testPosition = offsetPosition + moveDirection * testDistance;

							SV_Move(trmove, offsetPosition, pedict->state.mins, pedict->state.maxs, testPosition, traceflags, pedict);
							if(trmove.allSolid() || trmove.startSolid())
								return false;							
							
							if(trmove.noHit())
							{
								trmove = trace_t();
								Vector newTestPosition = testPosition - verticalDirection * stepSize;
								SV_Move(trmove, testPosition, pedict->state.mins, pedict->state.maxs, newTestPosition, traceflags, pedict);
							}
						}
						else if(verticalDirection.z < 0)
						{
							Vector offsetPosition = currentPosition + moveDirection * testDistance;
							
							SV_Move(trmove, currentPosition, pedict->state.mins, pedict->state.maxs, offsetPosition, traceflags, pedict);
							if(trmove.allSolid() || trmove.startSolid() || !trmove.noHit())
								return false;							

							testPosition = offsetPosition + verticalDirection * stepSize;

							trmove = trace_t();
							SV_Move(trmove, offsetPosition, pedict->state.mins, pedict->state.maxs, testPosition, traceflags, pedict);
						}

						// We should always have hit something at this point
						if(trmove.allSolid() || trmove.startSolid() || trmove.noHit())
							return false;

						distanceToGoal -= testDistance;
						if(distanceToGoal < 0)
							distanceToGoal = 0;

						currentPosition = trmove.endpos;
					}
				}
			}

			// Set endpos from trace as origin
			pedict->state.origin = tr.endpos;

			trace_t savedtr = tr;
			if(!SV_CheckBottom(pedict))
			{
				if(pedict->state.flags & FL_PARTIALGROUND)
				{
					if(relink)
						SV_LinkEdict(pedict, true);

					return true;
				}

				// Set endpos from trace as origin
				pedict->state.origin = tr.endpos;
				svs.gamevars.globaltrace = savedtr;
				return false;
			}
			else
			{
				// Set to fully grounded if partial
				pedict->state.flags &= ~FL_PARTIALGROUND;
				pedict->state.groundent = tr.hitentity;

				if(relink)
					SV_LinkEdict(pedict, true);

				svs.gamevars.globaltrace = savedtr;
				return true;
			}
		}
	}
}

//=============================================
//
//=============================================
bool SV_NPC_StepDirection( edict_t* pedict, Float yaw, Float dist )
{
	Float _yaw = yaw * (M_PI*2.0f)/360.0f;

	Vector move;
	move[0] = SDL_cos(_yaw)*dist;
	move[1] = SDL_sin(_yaw)*dist;

	// Get the result of this move
	bool result = SV_NPC_MoveStep(pedict, move, false, false);

	// Relink either way
	SV_LinkEdict(pedict, true);

	return result;
}

//=============================================
//
//=============================================
bool SV_NPC_FlyDirection( edict_t* pedict, const Vector& direction )
{
	bool result = SV_NPC_MoveStep(pedict, direction, false, false);

	SV_LinkEdict(pedict, true);
	return result;
}

//=============================================
//
//=============================================
void SV_NPC_FixCheckBottom( edict_t* pedict )
{
	pedict->state.flags |= FL_PARTIALGROUND;
}

//=============================================
//
//=============================================
bool SV_NPC_CloseEnough( edict_t* pedict, edict_t* pgoal, Float dist )
{
	for(Uint32 i = 0; i < 3; i++)
	{
		if(pgoal->state.absmin[i] > (pedict->state.absmax[i] + dist))
			return false;

		if(pgoal->state.absmax[i] < (pedict->state.absmin[i] - dist))
			return false;
	}

	return true;
}

//=============================================
//
//=============================================
bool SV_NPC_ReachedGoal( edict_t* pedict, const Vector& goalPosition, Float dist )
{
	for(Uint32 i = 0; i < 3; i++)
	{
		if(goalPosition[i] > (pedict->state.absmax[i] + dist))
			return false;

		if(goalPosition[i] < (pedict->state.absmin[i] - dist))
			return false;
	}

	return true;
}

//=============================================
//
//=============================================
void SV_NPC_NewChaseDir( edict_t* pedict, const Vector& goalPosition, Float dist )
{
	Float olddir = (45.0f*SDL_floor(pedict->state.idealyaw/45.0f));
	Float turnaround = Math::AngleMod(olddir - 180.0f);

	Float deltax = goalPosition[0] - pedict->state.origin[0];
	Float deltay = goalPosition[1] - pedict->state.origin[1];

	Vector dir;
	Float tempdir;

	if(deltax > 10.0f)
		dir[1] = 0;
	else if(deltax < 10.0f)
		dir[1] = 180.0f;
	else
		dir[1] = DIRECTION_NODIR;

	if(deltay < -10.0f)
		dir[2] = 270;
	else if(deltay > 10.0f)
		dir[2] = 90;
	else
		dir[2] = DIRECTION_NODIR;

	if(dir[1] != DIRECTION_NODIR && dir[2] != DIRECTION_NODIR)
	{
		if(!dir[1])
			tempdir = (dir[2] == 90.0f) ? 45.0f : 315.0f;
		else
			tempdir = (dir[2] == 90.0f) ? 135.0f : 215.0f;

		if(tempdir != turnaround && SV_NPC_StepDirection(pedict, tempdir, dist))
			return;
	}

	if(Common::RandomLong(0, 1) || SDL_fabs(deltay) > SDL_fabs(deltax))
	{
		tempdir = dir[1];
		dir[1] = dir[2];
		dir[2] = tempdir;
	}

	if(dir[1] != DIRECTION_NODIR && dir[1] != turnaround && SV_NPC_StepDirection(pedict, dir[1], dist)
		|| dir[2] != DIRECTION_NODIR && dir[2] != turnaround && SV_NPC_StepDirection(pedict, dir[2], dist))
		return;

	if(olddir != DIRECTION_NODIR && SV_NPC_StepDirection(pedict, olddir, dist))
		return;

	if(Common::RandomLong(0, 1))
	{
		for(tempdir = 0; tempdir <= 315; tempdir += 45)
		{
			if(tempdir != turnaround && SV_NPC_StepDirection(pedict, tempdir, dist))
				return;
		}
	}
	else
	{
		for(tempdir = 315; tempdir >= 0; tempdir -= 45)
		{
			if(tempdir != turnaround && SV_NPC_StepDirection(pedict, tempdir, dist))
				return;
		}
	}

	if(turnaround != DIRECTION_NODIR && SV_NPC_StepDirection(pedict, turnaround, dist))
		return;

	// Stuck
	pedict->state.idealyaw = olddir;

	if(!SV_CheckBottom(pedict))
		SV_NPC_FixCheckBottom(pedict);
}

//=============================================
//
//=============================================
void SV_NPC_MoveToOrigin( edict_t* pedict, const Vector& goalPosition, Float moveyaw, Float dist, npc_movetype_t movetype )
{
	if(pedict->state.flags & (FL_FLY|FL_SWIM|FL_ONGROUND))
	{
		if(movetype == MOVE_NORMAL)
		{
			// Why the hell did this rely on idealyaw to begin with?
			if(!SV_NPC_StepDirection(pedict, moveyaw, dist))
				SV_NPC_NewChaseDir(pedict, goalPosition, dist);
		}
		else
		{
			Vector direction;
			Math::VectorSubtract(goalPosition, pedict->state.origin, direction);

			if(!(pedict->state.flags & (FL_SWIM|FL_FLY)))
				direction[2] = 0;

			direction.Normalize();
			Math::VectorScale(direction, dist, direction);
			SV_NPC_FlyDirection(pedict, direction);
		}
	}
}