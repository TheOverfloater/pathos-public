/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef COLLISION_SHARED_H
#define COLLISION_SHARED_H

namespace CollisionShared
{
	// Test if a point-size traceline intersects a BVH node
	bool IntersectBVHNodePoint( const Vector& start, const Vector& end, const Vector& bbmins, const Vector& bbmaxs, const Vector& normalDirection );
	// Test if an AABB intersects a bounding box
	bool IntersectBBoxAABB( const Vector& center, const Vector& boxmins, const Vector& boxmaxs, const Vector& extents );
	// Test if an AABB traceline intersects a bounding box
	bool IntersectBBoxSweptAABB( const Vector& start, const Vector& end, const Vector& boxmins, const Vector& boxmaxs, const Vector& extents );
};
#endif COLLISION_SHARED_H