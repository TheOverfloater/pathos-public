/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef MCDTRACE_H
#define MCDTRACE_H

#include "mcdformat.h"
#include "plane.h"
#include "trace.h"

/*
=======================
CMCDTrace

=======================
*/
class CMCDTrace
{
public:
	// Collision epsilon value
	static const Float COLLISION_EPSILON;
	// Distance epsilon value
	static const Float DISTANCE_EPSILON;
	// On-plane epsilon value
	static const Float ONPLANE_EPSILON;
	// Invalid fraction value
	static const Float INVALID_FRACTION;
	// SA epsilon value
	static const Float SA_EPSILON;
	// Triangle index alloc size
	static const Uint32 TRIANGLE_INDEX_ALLOC_SIZE;
	// Impact normal vectors
	static const Vector IMPACT_NORMAL_VECTORS[2][3];

	enum axis_t
	{
		AXIS_X = 0,
		AXIS_Y,
		AXIS_Z
	};

public:
	CMCDTrace( void );
	~CMCDTrace( void );

public:
	// Perform a point-sized traceline against the MCD data
	bool TraceLinePoint( const Vector& start, const Vector& end, const mcdheader_t* pmcdheader, Uint64 bodyvalue, trace_t& tr );
	// Perform an AABB traceline against the MCD data
	bool TraceLineAABB( const Vector& start, const Vector& end, const Vector& clipHullMins, const Vector& clipHullMaxs, const mcdheader_t* pmcdheader, Uint64 bodyvalue, trace_t& tr );
	// Return the hit texture name
	const Char* GetHitTextureName( void ) const { return m_traceHitTextureName.c_str(); }

private:
	// Set up submodel
	void SetupModel( Uint32 bodypart, Uint64 bodyvalue );
	// Add triangle to thelist
	void AddBVHNodeTriangles( const mcdbvhnode_t* pbvhnode );
	// Test if a point-size traceline intersects a BVH node
	bool IntersectBVHNodePoint( const Vector& start, const Vector& end, const Vector& bbmins, const Vector& bbmaxs );
	// Test if an AABB intersects a bounding box
	bool IntersectBBoxAABB( const Vector& center, const Vector& boxmins, const Vector& boxmaxs, const Vector& extents );
	// Test if an AABB traceline intersects a bounding box
	bool IntersectBBoxSweptAABB( const Vector& start, const Vector& end, const Vector& boxmins, const Vector& boxmaxs, const Vector& extents );
	// Perform a swept AABB test against a triangle
	bool SweptAABBTriangleTest( const Vector& start, const Vector& end, const Vector& extents, const mcdvertex_t* pvertexes, const mcdtrimeshtriangle_t* ptriangle, Vector& impactPosition, Vector& impactNormal, Float& planeDistance, Float& fraction );
	// Perform a line test against a triangle
	bool TestLineTriangleIntersect( const Vector& start, const Vector& end, const mcdvertex_t* pvertexes, const mcdtrimeshtriangle_t* ptriangle, Vector& impactNormal, Vector& impactPosition, Float& planeDistance, Float& fraction );
	// Recurse down the tree with a point trace
	void RecurseTreePointTrace( const Vector& start, const Vector& end, const mcdbvhnode_t* pbvhnode );
	// Recurse down the tree with a point trace
	void RecurseTreeAABBTrace( const Vector& start, const Vector& end, const Vector& extents, const mcdbvhnode_t* pbvhnode, bool intersectTest );

	// Test swept AABB against face plane
	bool SweptAABBTestFacePlane( const Vector& start, const Vector& end, const Vector& extents, const mcdtrimeshtriangle_t* ptriangle );
	// Test swept AABB against axial XYZ planes
	bool SweptAABBTestAxialPlanesXYZ( const Vector& start, const Vector& end, const Vector& extents, const Vector& rayvector, const Vector& raydirection, const mcdvertex_t* pvertexes, const mcdtrimeshtriangle_t* ptriangle );

	// Intersect test between AABB and triangle
	bool SeparatingAxisAABBTriangleTest( const Vector& position, const Vector& extents, const mcdtrimeshtriangle_t* ptriangle, const mcdvertex_t* pvertexes );
	// Test intersection between plane and AABB
	bool TestOverlapPlaneAABB( const mcdtrimeshtriangle_t* ptriangle, const Vector& position, const Vector& extents );
	// Test cross-x edge axis
	bool AxisTestEdgeCross( axis_t testaxis, Float edge1, Float edge2, Float absedge1, Float absedge2, const Vector& pt1, const Vector& pt2, const Vector& extents );

	// Calculate closest extents
	Vector CalcClosestExtents( const Vector& normal, const Vector& boxExtents );
	// Calculate closest box point
	Vector CalcClosestBoxPoint( const Vector& normal, const Vector& start, const Vector& extents );
	// Resolve ray-plane intersection
	bool ResolveLinePlaneIntersection( Float startDistance, Float endDistance, const Vector& planeNormal, const Float& planeDistance );

	// Edge cross axis test
	bool EdgeCrossAxisTest( axis_t testaxis, const Vector& start, const Vector& end, const Vector& rayvector, const Vector& extents, const Vector& edge, const Vector& pointonedge, const Vector& pointoffedge );

private:
	// Array of triangle indexes we touched
	CArray<Int32> m_triangleIndexesArray;
	// Number of triangles in array
	Uint32 m_triangleCount;

	// Object AABB mins
	Vector m_mins;
	// Object AABB maxs
	Vector m_maxs;
	// Normalized direction
	Vector m_normDirection;
	// Original distance
	Float m_baseDistance;
	// Distance travelled
	Float m_distance;

	// Current normal used
	Vector m_currentNormal;
	// Current plane distance used
	Float m_currentPlaneDistance;
	// Current start fraction
	Float m_currentStartFraction;
	// Current end fraction
	Float m_currentEndFraction;

	// MCD header pointer
	const mcdheader_t* m_pMCDHeader;
	// Current submodel
	const mcdsubmodel_t* m_pSubModel;
	// Triangle mesh
	const mcdtrimeshtype_t* m_pSubModelTriangleMesh;
	// BVH collision data
	const mcdbvhtype_t* m_pSubModelBVHData;

	// Traceline result
	trace_t m_traceResult;
	// Skinref we hit
	Int32 m_hitSkinRef;
	// Texture name we hit
	CString m_traceHitTextureName;
};
extern CMCDTrace g_mcdTrace;
#endif //MCDTRACE_H