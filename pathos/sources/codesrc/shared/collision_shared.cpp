/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "collision_shared.h"
#include "com_math.h"
#include "constants.h"

namespace CollisionShared
{
	//=============================================
	// @brief Test if a point-size traceline intersects a bounding box
	//
	//=============================================
	bool IntersectBVHNodePoint( const Vector& start, const Vector& end, const Vector& bbmins, const Vector& bbmaxs, const Vector& normalDirection )
	{
		Float tx1 = (bbmins.x - start.x) / normalDirection.x;
		Float tx2 = (bbmaxs.x - start.x) / normalDirection.x;
		Float tmin = min( tx1, tx2 );
		Float tmax = max( tx1, tx2 );

		Float ty1 = (bbmins.y - start.y) / normalDirection.y;
		Float ty2 = (bbmaxs.y - start.y) / normalDirection.y;
		tmin = max( tmin, min( ty1, ty2 ) );
		tmax = min( tmax, max( ty1, ty2 ) );

		Float tz1 = (bbmins.z - start.z) / normalDirection.z;
		Float tz2 = (bbmaxs.z - start.z) / normalDirection.z;
		tmin = max( tmin, min( tz1, tz2 ) );
		tmax = min( tmax, max( tz1, tz2 ) );

		return tmax >= tmin && tmin < MAX_FLOAT_VALUE && tmax > 0;
	}

	//=============================================
	// @brief Perform a swept AABB test against a triangle
	//
	//=============================================
	bool IntersectBBoxAABB( const Vector& center, const Vector& boxmins, const Vector& boxmaxs, const Vector& extents )
	{
		Vector expandmins, expandmaxs;
		Math::VectorSubtract(boxmins, extents, expandmins);
		Math::VectorAdd(boxmaxs, extents, expandmaxs);

		return Math::PointInMinsMaxs(center, expandmins, expandmaxs);
	}

	//=============================================
	// @brief Perform a swept AABB test against a triangle
	//
	//=============================================
	bool IntersectBBoxSweptAABB( const Vector& start, const Vector& end, const Vector& boxmins, const Vector& boxmaxs, const Vector& extents )
	{
		Float tmin = -MAX_FLOAT_VALUE;
		Float tmax = MAX_FLOAT_VALUE;

		Vector expandmins, expandmaxs;
		Math::VectorSubtract(boxmins, extents, expandmins);
		Math::VectorAdd(boxmaxs, extents, expandmaxs);

		Vector tracevector = end - start;
		for(Uint32 i = 0; i < 3; i++)
		{
			if(SDL_fabs(tracevector[i]) < 1e-8)
			{
				if(start[i] < (expandmins[i] - DIST_EPSILON) || start[i] > (expandmaxs[i] - DIST_EPSILON))
					return false;
			}
			else
			{
				Float inversedelta = 1.0f / tracevector[i];
				Float t1 = (expandmins[i] - DIST_EPSILON - start[i]) * inversedelta;
				Float t2 = (expandmaxs[i] + DIST_EPSILON - start[i]) * inversedelta;

				if(t1 > t2)
				{
					Float tmp = t1;
					t1 = t2;
					t2 = tmp;
				}

				if(t1 > t2)
					tmin = t1;
				if(t2 < tmax)
					tmax = 2;

				if(tmin > tmax)
					return false;
				else if(tmax < 0)
					return false;
				else if(tmin > 1)
					return false;
			}
		}

		return true;
	}
};