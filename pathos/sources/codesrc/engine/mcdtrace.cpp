/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "com_math.h"
#include "mcdtrace.h"
#include "enginestate.h"
#include "frustum_inline.hpp"

// Credits go to:
// j_bikker For his article on Bounding Volume Hierarchies, which this implementation is based off of,
// and I used his solution for line-triangle intersection testing and traversing the BVH.
// To Valve for the Source SDK, which I referenced to find a solution for testing AABB intersection
// and Swept AABB collision tests. I looked at their collision detection for displacements in order
// to implement collision testing between a bounding box and raw triangle meshes.

// Object declaration
CMCDTrace g_mcdTrace;

// Collision epsilon value
const Float CMCDTrace::COLLISION_EPSILON = 0.01;
// Distance epsilon value
const Float CMCDTrace::DISTANCE_EPSILON = 0.03125;
// On-plane epsilon value
const Float CMCDTrace::ONPLANE_EPSILON = 0.03125;
// Invalid fraction value
const Float CMCDTrace::INVALID_FRACTION = -MAX_FLOAT_VALUE;
// SA epsilon value
const Float CMCDTrace::SA_EPSILON = 0.0f;
// Triangle index alloc size
const Uint32 CMCDTrace::TRIANGLE_INDEX_ALLOC_SIZE = 64;

// Impact normal vectors
const Vector CMCDTrace::IMPACT_NORMAL_VECTORS[2][3] = {
	{ 
		Vector(-1, 0, 0),
		Vector(0, -1, 0),
		Vector(0, 0, -1),
	},
	{
		Vector(1, 0, 0),
		Vector(0, 1, 0),
		Vector(0, 0, 1)
	}
};

//=============================================
// @brief Default constructor
//
//=============================================
CMCDTrace::CMCDTrace( void ):
	m_triangleIndexesArray(TRIANGLE_INDEX_ALLOC_SIZE),
	m_triangleCount(0),
	m_baseDistance(0),
	m_distance(0),
	m_currentPlaneDistance(0),
	m_currentStartFraction(0),
	m_currentEndFraction(0),
	m_pMCDHeader(nullptr),
	m_pSubModel(nullptr),
	m_pSubModelTriangleMesh(nullptr),
	m_pSubModelBVHData(nullptr),
	m_hitSkinRef(NO_POSITION)
{
}

//=============================================
// @brief Destructor
//
//=============================================
CMCDTrace::~CMCDTrace( void )
{
}

//=============================================
// @brief Perform a point-sized traceline against the MCD data
//
//=============================================
bool CMCDTrace::TraceLinePoint( const Vector& start, const Vector& end, const mcdheader_t* pmcdheader, Uint64 bodyvalue, trace_t& tr )
{
	// Set basics
	m_mins = ZERO_VECTOR;
	m_maxs = ZERO_VECTOR;
	m_pMCDHeader = pmcdheader;

	// Set defaults
	m_traceResult.fraction = 1.0;
	m_traceResult.endpos = end;
	m_traceResult.flags = FL_TR_INOPEN;
	m_triangleCount = 0;

	m_hitSkinRef = NO_POSITION;
	m_traceHitTextureName.clear();

	Math::VectorSubtract(end, start, m_normDirection);
	m_baseDistance = m_distance = m_normDirection.Length();
	m_normDirection.Normalize();

	// Now go through each submodel and perform the trace
	for (Int32 i = 0; i < m_pMCDHeader->numbodyparts; i++)
	{
		SetupModel(i, bodyvalue);

		if(!m_pSubModel->numcollisiontypes)
			continue;

		if(!IntersectBVHNodePoint(start, end, m_pSubModel->mins, m_pSubModel->maxs))
			continue;

		// Get triangle mesh data
		m_pSubModelTriangleMesh = reinterpret_cast<const mcdtrimeshtype_t*>(m_pSubModel->getTypeData(m_pMCDHeader, MCD_COLLISION_TRIANGLES));
		if(!m_pSubModelTriangleMesh)
			continue;

		// Get BVH data
		m_pSubModelBVHData = reinterpret_cast<const mcdbvhtype_t*>(m_pSubModel->getTypeData(m_pMCDHeader, MCD_COLLISION_BVH));
		if(!m_pSubModelBVHData)
			continue;

		// Get the root node and collect the triangles we need for collision testing
		const mcdbvhnode_t* prootnode = m_pSubModelBVHData->getNode(m_pMCDHeader, 0);
		RecurseTreePointTrace(start, end, prootnode);
		if(!m_triangleCount)
			continue;

		const mcdtrimeshtriangle_t* ptriangles = m_pSubModelTriangleMesh->getTriangles(m_pMCDHeader);
		const mcdvertex_t* pvertexes = m_pSubModelTriangleMesh->getVertexes(m_pMCDHeader);

		for(Uint32 j = 0; j < m_triangleCount; j++)
		{
			Vector impactPosition;
			Vector impactNormal;
			Float planeDistance;
			Float fraction = 1.0;

			Int32 triangleIndex = m_triangleIndexesArray[j];
			const mcdtrimeshtriangle_t* ptriangle = &ptriangles[triangleIndex];

			if(TestLineTriangleIntersect(start, end, pvertexes, ptriangle, impactPosition, impactNormal, planeDistance, fraction))
			{
				if(fraction < m_traceResult.fraction)
				{
					m_traceResult.fraction = fraction;
					m_traceResult.endpos = impactPosition;
					m_traceResult.plane.normal = impactNormal;
					m_traceResult.plane.dist = planeDistance;
					m_hitSkinRef = ptriangle->skinref;
				}
			}
		}

		// Reset this
		m_triangleCount = 0;
	}

	// If we hit anything, then mark the texture
	if(m_traceResult.fraction < 1.0 && m_hitSkinRef != NO_POSITION)
	{
		const mcdtexture_t* ptexture = m_pMCDHeader->getTexture(m_hitSkinRef);
		Common::Basename(ptexture->name, m_traceHitTextureName);
	}

	// Set result
	tr = m_traceResult;

	// Tell if we hit anything
	return (m_traceResult.fraction == 1.0) ? false : true;
}

//=============================================
// @brief Perform an AABB traceline against the MCD data
//
//=============================================
bool CMCDTrace::TraceLineAABB( const Vector& start, const Vector& end, const Vector& clipHullMins, const Vector& clipHullMaxs, const mcdheader_t* pmcdheader, Uint64 bodyvalue, trace_t& tr )
{
	// Set basics
	m_mins = clipHullMins;
	m_maxs = clipHullMaxs;
	m_pMCDHeader = pmcdheader;

	// Set defaults
	m_traceResult.fraction = 1.0;
	m_traceResult.endpos = end;
	m_traceResult.flags = (FL_TR_INOPEN);
	m_triangleCount = 0;

	// Clear this as we won't set this here
	m_traceHitTextureName.clear();

	Math::VectorSubtract(end, start, m_normDirection);
	m_baseDistance = m_distance = m_normDirection.Length();
	m_normDirection.Normalize();

	bool intersectTest = !m_distance ? true : false;
	Vector extents = (clipHullMins - clipHullMins) * 0.5 - clipHullMins;

	// Now go through each submodel and perform the trace
	for (Int32 i = 0; i < m_pMCDHeader->numbodyparts; i++)
	{
		SetupModel(i, bodyvalue);

		if(!m_pSubModel->numcollisiontypes)
			continue;

		if(!intersectTest)
		{
			if(!IntersectBBoxSweptAABB(start, end, m_pSubModel->mins, m_pSubModel->maxs, extents))
				continue;
		}
		else
		{
			if(!IntersectBBoxAABB(start, m_pSubModel->mins, m_pSubModel->maxs, extents))
				continue;
		}

		// Get triangle mesh data
		m_pSubModelTriangleMesh = reinterpret_cast<const mcdtrimeshtype_t*>(m_pSubModel->getTypeData(m_pMCDHeader, MCD_COLLISION_TRIANGLES));
		if(!m_pSubModelTriangleMesh)
			continue;

		// Get BVH data
		m_pSubModelBVHData = reinterpret_cast<const mcdbvhtype_t*>(m_pSubModel->getTypeData(m_pMCDHeader, MCD_COLLISION_BVH));
		if(!m_pSubModelBVHData)
			continue;

		// Get the root node and collect the triangles we need for collision testing
		const mcdbvhnode_t* prootnode = m_pSubModelBVHData->getNode(m_pMCDHeader, 0);
		RecurseTreeAABBTrace(start, end, extents, prootnode, intersectTest);
		if(!m_triangleCount)
			continue;

		const mcdtrimeshtriangle_t* ptriangles = m_pSubModelTriangleMesh->getTriangles(m_pMCDHeader);
		const mcdvertex_t* pvertexes = m_pSubModelTriangleMesh->getVertexes(m_pMCDHeader);

		for(Uint32 j = 0; j < m_triangleCount; j++)
		{
			Int32 triangleIndex = m_triangleIndexesArray[j];

			Vector impactPosition;
			Vector impactNormal;
			Float planeDistance = 0;
			Float fraction = 1.0;

			bool startsolid = false;
			bool allsolid = false;

			bool result;
			if(intersectTest)
			{
				result = SeparatingAxisAABBTriangleTest(start, extents, &ptriangles[triangleIndex], pvertexes);
				if(result)
				{
					impactNormal = ptriangles[i].normal;
					planeDistance = ptriangles[i].distance;
					impactPosition = end;
					fraction = 0.0;
					startsolid = true;
					allsolid = true;
				}
			}
			else
			{
				result = SweptAABBTriangleTest(start, end, extents, pvertexes, &ptriangles[triangleIndex], impactPosition, impactNormal, planeDistance, fraction);
			}

			if(result)
			{
				if(fraction < m_traceResult.fraction)
				{
					m_traceResult.fraction = fraction;
					m_traceResult.endpos = impactPosition;
					m_traceResult.plane.normal = impactNormal;
					m_traceResult.plane.dist = planeDistance;

					if(startsolid)
					{
						m_traceResult.flags |= FL_TR_STARTSOLID;
						m_traceResult.flags &= ~FL_TR_INOPEN;
					}

					if(allsolid)
					{
						m_traceResult.flags |= FL_TR_ALLSOLID;
						m_traceResult.flags &= ~FL_TR_INOPEN;
					}
				}
			}
		}

		// Reset this
		m_triangleCount = 0;
	}

	// Set result
	tr = m_traceResult;

	// Tell if we hit anything
	return (m_traceResult.fraction == 1.0 && !m_traceResult.allSolid() && !m_traceResult.startSolid()) ? false : true;
}

//=============================================
// @brief Set up submodel
//
//=============================================
void CMCDTrace::SetupModel( Uint32 bodypart, Uint64 bodyvalue )
{
	const mcdbodypart_t *pbodypart = m_pMCDHeader->getBodyPart(bodypart);

	Uint32 index = bodyvalue / pbodypart->base;
	index = index % pbodypart->numsubmodels;

	m_pSubModel = pbodypart->getSubmodel(m_pMCDHeader, index);
}

//=============================================
// @brief Test if a point-size traceline intersects a bounding box
//
//=============================================
bool CMCDTrace::IntersectBVHNodePoint( const Vector& start, const Vector& end, const Vector& bbmins, const Vector& bbmaxs )
{
    Float tx1 = (bbmins.x - start.x) / m_normDirection.x;
	Float tx2 = (bbmaxs.x - start.x) / m_normDirection.x;
    Float tmin = min( tx1, tx2 );
	Float tmax = max( tx1, tx2 );

    Float ty1 = (bbmins.y - start.y) / m_normDirection.y;
	Float ty2 = (bbmaxs.y - start.y) / m_normDirection.y;
    tmin = max( tmin, min( ty1, ty2 ) );
	tmax = min( tmax, max( ty1, ty2 ) );

    Float tz1 = (bbmins.z - start.z) / m_normDirection.z;
	Float tz2 = (bbmaxs.z - start.z) / m_normDirection.z;
    tmin = max( tmin, min( tz1, tz2 ) );
	tmax = min( tmax, max( tz1, tz2 ) );

    return tmax >= tmin && tmin < MAX_FLOAT_VALUE && tmax > 0;
}

//=============================================
// @brief Perform a swept AABB test against a triangle
//
//=============================================
bool CMCDTrace::IntersectBBoxAABB( const Vector& center, const Vector& boxmins, const Vector& boxmaxs, const Vector& extents )
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
bool CMCDTrace::IntersectBBoxSweptAABB( const Vector& start, const Vector& end, const Vector& boxmins, const Vector& boxmaxs, const Vector& extents )
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

//=============================================
// @brief Add triangle to the list
//
//=============================================
void CMCDTrace::AddBVHNodeTriangles( const mcdbvhnode_t* pbvhnode )
{
	const Int32* ptriindexes = pbvhnode->getTriangleIndexes(m_pMCDHeader);
	for(Uint32 i = 0; i < pbvhnode->numtriangles; i++)
	{
		// Check if array needs to be extended
		if(m_triangleCount == m_triangleIndexesArray.size())
		{
			Uint32 arraySize = m_triangleIndexesArray.size()+TRIANGLE_INDEX_ALLOC_SIZE;
			m_triangleIndexesArray.resize(arraySize);
		}

		m_triangleIndexesArray[m_triangleCount] = ptriindexes[i];
		m_triangleCount++;
	}
}

//=============================================
// @brief Intersect test between AABB and triangle
//
//=============================================
bool CMCDTrace::SeparatingAxisAABBTriangleTest( const Vector& position, const Vector& extents, const mcdtrimeshtriangle_t* ptriangle, const mcdvertex_t* pvertexes )
{
	// Order differs here from Swept version
	Vector pt[3];
	const Vector* pvertarray[] = {
		&pvertexes[ptriangle->trivertexes[0]].origin,
		&pvertexes[ptriangle->trivertexes[2]].origin,
		&pvertexes[ptriangle->trivertexes[1]].origin
	};

	// Test axial planes first
	for(Uint32 i = 0; i < 3; i++)
	{
		for(Uint32 j = 0; j < 3; j++)
			pt[j][i] = (*pvertarray[j])[i] - position[i];

		Float min, max;
		Math::FindMinMaxValuesOf3(pt[0][i], pt[1][i], pt[2][i], min, max);
		if(min > (extents[i] + SA_EPSILON) || max < -(extents[i] + SA_EPSILON))
			return false;
	}

	// Test the 9 edge cases
	Vector edge;
	Vector absedge;

	// Test on edge 0
	edge = (*pvertarray[1]) - (*pvertarray[0]);

	for(Uint32 i = 0; i < 3; i++)
		absedge[i] = SDL_fabs(edge[i]);

	if(!AxisTestEdgeCross(AXIS_X, edge.z, edge.y, absedge.z, absedge.y, (*pvertarray[0]), (*pvertarray[2]), extents)
		|| !AxisTestEdgeCross(AXIS_Y, edge.z, edge.x, absedge.z, absedge.x, (*pvertarray[0]), (*pvertarray[2]), extents)
		|| !AxisTestEdgeCross(AXIS_Z, edge.y, edge.x, absedge.y, absedge.x, (*pvertarray[1]), (*pvertarray[2]), extents))
		return false;

	// Test on edge 1
	edge = (*pvertarray[2]) - (*pvertarray[1]);

	for(Uint32 i = 0; i < 3; i++)
		absedge[i] = SDL_fabs(edge[i]);

	if(!AxisTestEdgeCross(AXIS_X, edge.z, edge.y, absedge.z, absedge.y, (*pvertarray[0]), (*pvertarray[1]), extents)
		|| !AxisTestEdgeCross(AXIS_Y, edge.z, edge.x, absedge.z, absedge.x, (*pvertarray[0]), (*pvertarray[1]), extents)
		|| !AxisTestEdgeCross(AXIS_Z, edge.y, edge.x, absedge.y, absedge.x, (*pvertarray[0]), (*pvertarray[2]), extents))
		return false;

	// Test on edge 2
	edge = (*pvertarray[0]) - (*pvertarray[2]);

	for(Uint32 i = 0; i < 3; i++)
		absedge[i] = SDL_fabs(edge[i]);

	if(!AxisTestEdgeCross(AXIS_X, edge.z, edge.y, absedge.z, absedge.y, (*pvertarray[0]), (*pvertarray[1]), extents)
		|| !AxisTestEdgeCross(AXIS_Y, edge.z, edge.x, absedge.z, absedge.x, (*pvertarray[0]), (*pvertarray[1]), extents)
		|| !AxisTestEdgeCross(AXIS_Z, edge.y, edge.x, absedge.y, absedge.x, (*pvertarray[1]), (*pvertarray[2]), extents))
		return false;

	// Final test is whether we're impacting the face plane
	Vector mins, maxs;
	Math::VectorSubtract(position, extents, mins);
	Math::VectorAdd(position, extents, maxs);

	plane_t plane;
	plane.dist = ptriangle->distance;
	plane.type = ptriangle->planetype;
	plane.signbits = ptriangle->signbits;
	plane.normal = ptriangle->normal;

	if(BoxOnPlaneSide(mins, maxs, &plane) != 3)
		return false;
	else
		return true;
}

//=============================================
// @brief Test intersection between plane and AABB
//
//=============================================
bool CMCDTrace::TestOverlapPlaneAABB( const mcdtrimeshtriangle_t* ptriangle, const Vector& position, const Vector& extents )
{
	Vector min, max;
	for(Uint32 i = 0; i < 3; i++)
	{
		if(ptriangle->normal[i] < 0.0f)
		{
			min[i] = extents[i];
			max[i] = -extents[i];
		}
		else
		{
			min[i] = -extents[i];
			max[i] = extents[i];
		}
	}

	Float mindistance = ptriangle->distance - Math::DotProduct(ptriangle->normal, min);
	Float maxdistance = ptriangle->distance - Math::DotProduct(ptriangle->normal, max);

	if((Math::DotProduct(ptriangle->normal, position) - mindistance) > 0.0f)
		return false;
	else if((Math::DotProduct(ptriangle->normal, position) - maxdistance) >= 0.0f)
		return false;
	else
		return false;
}

//=============================================
// @brief Test cross-x edge axis
//
//=============================================
bool CMCDTrace::AxisTestEdgeCross( axis_t testaxis, Float edge1, Float edge2, Float absedge1, Float absedge2, const Vector& pt1, const Vector& pt2, const Vector& extents )
{
	Float distance1 = 0;
	Float distance2 = 0;
	Float boxdistance = 0;

	switch(testaxis)
	{
	case AXIS_X:
		distance1 = (edge1 * pt1.y) - (edge2 * pt1.z);
		distance2 = (edge1 * pt2.y) - (edge2 * pt2.z);
		boxdistance = (absedge1 * extents.y) + (absedge2 * extents.z);
		break;
	case AXIS_Y:
		distance1 = (-edge1 * pt1.x) + (edge2 * pt1.z);
		distance2 = (-edge1 * pt2.x) + (edge2 * pt2.z);
		boxdistance = (absedge1 * extents.x) + (absedge2 * extents.z);
		break;
	case AXIS_Z:
		distance1 = (edge1 * pt1.x) - (edge2 * pt1.y);
		distance2 = (edge1 * pt2.x) - (edge2 * pt2.y);
		boxdistance = (absedge1 * extents.x) + (absedge2 * extents.y);
		break;
	}

	if(distance1 < distance2)
	{
		if(distance1 > (boxdistance + SA_EPSILON) || distance2 < -(boxdistance + SA_EPSILON))
			return false;
	}
	else
	{
		if(distance2 > (boxdistance + SA_EPSILON) || distance1 < -(boxdistance + SA_EPSILON))
			return false;
	}

	return true;
}

//=============================================
// @brief Perform a swept AABB test against a triangle
//
//=============================================
bool CMCDTrace::SweptAABBTriangleTest( const Vector& start, const Vector& end, const Vector& extents, const mcdvertex_t* pvertexes, const mcdtrimeshtriangle_t* ptriangle, Vector& impactPosition, Vector& impactNormal, Float& planeDistance, Float& fraction )
{
	m_currentStartFraction = INVALID_FRACTION;
	m_currentEndFraction = 1.0f;

	// Get the 
	Vector traceVector = end - start;
	Vector dirNormalized = traceVector;
	dirNormalized.Normalize();

	// Check if the trace is even headed in the direction of this triangle
	Float dot = Math::DotProduct(ptriangle->normal, traceVector);
	if(dot > DISTANCE_EPSILON)
		return false;

	// Test against axial planes next
	if(!SweptAABBTestAxialPlanesXYZ(start, end, extents, traceVector, dirNormalized, pvertexes, ptriangle))
		return false;

	const Vector& v1 = pvertexes[ptriangle->trivertexes[0]].origin;
	const Vector& v2 = pvertexes[ptriangle->trivertexes[1]].origin;
	const Vector& v3 = pvertexes[ptriangle->trivertexes[2]].origin;

	// Test against edge 1
	Vector edge = v2 - v1;
	if(!EdgeCrossAxisTest(AXIS_X, start, end, traceVector, extents, edge, v1, v3)
		|| !EdgeCrossAxisTest(AXIS_Y, start, end, traceVector, extents, edge, v1, v3)
		|| !EdgeCrossAxisTest(AXIS_Z, start, end, traceVector, extents, edge, v1, v3))
		return false;

	// Test against edge 2
	edge = v3 - v2;
	if(!EdgeCrossAxisTest(AXIS_X, start, end, traceVector, extents, edge, v2, v1)
		|| !EdgeCrossAxisTest(AXIS_Y, start, end, traceVector, extents, edge, v2, v1)
		|| !EdgeCrossAxisTest(AXIS_Z, start, end, traceVector, extents, edge, v2, v1))
		return false;

	// Test against edge 3
	edge = v1 - v3;
	if(!EdgeCrossAxisTest(AXIS_X, start, end, traceVector, extents, edge, v3, v2)
		|| !EdgeCrossAxisTest(AXIS_Y, start, end, traceVector, extents, edge, v3, v2)
		|| !EdgeCrossAxisTest(AXIS_Z, start, end, traceVector, extents, edge, v3, v2))
		return false;

	// Test against face plane finally
	if(!SweptAABBTestFacePlane(start, end, extents, ptriangle))
		return false;

	// Set end result depending on whether we hit anything
	if(m_currentStartFraction < m_currentEndFraction 
		|| SDL_fabs(m_currentStartFraction - m_currentEndFraction) < 0.001)
	{
		if(m_currentStartFraction != INVALID_FRACTION 
			&& m_currentStartFraction < fraction)
		{
			if(m_currentStartFraction < 0)
				m_currentStartFraction = 0;

			fraction = m_currentStartFraction;
			impactPosition = start + traceVector * fraction;

			impactNormal = m_currentNormal;
			planeDistance = m_currentPlaneDistance;

			return true;
		}
	}

	return false;
}

//=============================================
// @brief Test swept AABB against face plane
//
//=============================================
bool CMCDTrace::SweptAABBTestFacePlane( const Vector& start, const Vector& end, const Vector& extents, const mcdtrimeshtriangle_t* ptriangle )
{
	// Calculate closest point on box to the plane
	Vector pointExtents = CalcClosestExtents(ptriangle->normal, extents);

	// Expand the plane to the box extents, so we can reduce this to a line-triangle test
	Float expanddistance = ptriangle->distance - Math::DotProduct(ptriangle->normal, pointExtents);
	Float startdistance = Math::DotProduct(ptriangle->normal, start) - expanddistance;
	Float enddistance = Math::DotProduct(ptriangle->normal, end) - expanddistance;

	// Test iwe actually hit the plane or not
	return ResolveLinePlaneIntersection(startdistance, enddistance, ptriangle->normal, ptriangle->distance);
}

//=============================================
// @brief Test swept AABB against axial XYZ planes
//
//=============================================
bool CMCDTrace::SweptAABBTestAxialPlanesXYZ( const Vector& start, const Vector& end, const Vector& extents, const Vector& rayvector, const Vector& raydirection, const mcdvertex_t* pvertexes, const mcdtrimeshtriangle_t* ptriangle )
{
	// Get the vertexes(do not modify the order here)
	const Vector& v1 = pvertexes[ptriangle->trivertexes[0]].origin;
	const Vector& v2 = pvertexes[ptriangle->trivertexes[1]].origin;
	const Vector& v3 = pvertexes[ptriangle->trivertexes[2]].origin;

	// Get closest box on plane and test against axial planes
	Vector boxpoint = CalcClosestBoxPoint(ptriangle->normal, start, extents);
	for(Int32 i = 2; i >= 0; i--)
	{
		Float raystart = start[i];
		Float extent = extents[i];
		Float delta = rayvector[i];

		Float distance = Math::FindMinValueOf3(v1[i], v2[i], v3[i]);
		Float expanddistance = distance - extent;
		Float startdistance = expanddistance - raystart;
		Float enddistance = startdistance - delta;
		
		if(!ResolveLinePlaneIntersection(startdistance, enddistance, IMPACT_NORMAL_VECTORS[0][i], distance))
			return false;

		distance = Math::FindMaxValueOf3(v1[i], v2[i], v3[i]);
		expanddistance = distance + extent;
		startdistance = raystart - expanddistance;
		enddistance = startdistance + delta;

		if(!ResolveLinePlaneIntersection(startdistance, enddistance, IMPACT_NORMAL_VECTORS[1][i], distance))
			return false;
	}

	return true;
}

//=============================================
// @brief Edge cross axis test
//
//=============================================
bool CMCDTrace::EdgeCrossAxisTest( axis_t testaxis, const Vector& start, const Vector& end, const Vector& rayvector, const Vector& extents, const Vector& edge, const Vector& pointonedge, const Vector& pointoffedge )
{
	// Tests on normal depend on which axis we're considering,
	// so this cannot be simplified into a simple IsZero check
	// on the normal after assigning values.
	Vector normal;
	switch(testaxis)
	{
	case AXIS_X:
		{
			normal = Vector(0.0f, edge.z, -edge.y);
			normal.Normalize();

			if(normal.y == 0 || normal.z == 0)
				return true;
		}
		break;
	case AXIS_Y:
		{
			normal = Vector(-edge.z, 0.0, edge.x);
			normal.Normalize();

			if(normal.x == 0 || normal.z == 0)
				return true;
		}
		break;
	case AXIS_Z:
		{
			normal = Vector(edge.y, -edge.x, 0.0f);
			normal.Normalize();

			if(normal.x == 0 || normal.y == 0)
				return true;
		}
		break;
	}

	Int32 axis1 = (testaxis+1) % 3;
	Int32 axis2 = (testaxis+2) % 3;

	Float distance = (normal[axis1] * pointonedge[axis1]) + (normal[axis2] * pointonedge[axis2]);
	Float offdistance = (normal[axis1] * pointoffedge[axis1]) + (normal[axis2] * pointoffedge[axis2]);
	if(!(SDL_fabs(offdistance - distance) < COLLISION_EPSILON) && (offdistance > distance))
	{
		Math::VectorScale(normal, -1, normal);
		distance = -distance;
	}

	Vector closestextents;
	closestextents[axis1] = (normal[axis1] < 0) ? extents[axis1] : -extents[axis1];
	closestextents[axis2] = (normal[axis2] < 0) ? extents[axis2] : -extents[axis2];

	Float expanddistance = distance - ((normal[axis1] * closestextents[axis1]) + (normal[axis2] * closestextents[axis2]));
	Float startdistance = ((normal[axis1] * start[axis1]) + (normal[axis2] * start[axis2])) - expanddistance;
	Float enddistance = ((normal[axis1] * end[axis1]) + (normal[axis2] * end[axis2])) - expanddistance;

	return ResolveLinePlaneIntersection(startdistance, enddistance, normal, distance);
}

//=============================================
// @brief Calculate closest extents
//
//=============================================
Vector CMCDTrace::CalcClosestExtents( const Vector& normal, const Vector& boxExtents )
{
	Vector closestExtents;
	for(Uint32 i = 0; i < 3; i++)
	{
		if(normal[i] < 0.0f)
			closestExtents[i] = boxExtents[i];
		else
			closestExtents[i] = -boxExtents[i];
	}

	return closestExtents;
}

//=============================================
// @brief Calculate closest box point
//
//=============================================
Vector CMCDTrace::CalcClosestBoxPoint( const Vector& normal, const Vector& start, const Vector& extents )
{
	Vector boxPoint;
	for(Uint32 i = 0; i < 3; i++)
	{
		if(normal[i] < 0.0f)
			boxPoint[i] = start[i] + extents[i];
		else
			boxPoint[i] = start[i] - extents[i];
	}

	return boxPoint;
}

//=============================================
// @brief Resolve ray-plane intersection
//
//=============================================
bool CMCDTrace::ResolveLinePlaneIntersection( Float startDistance, Float endDistance, const Vector& planeNormal, const Float& planeDistance )
{
	if(startDistance > 0.0f && endDistance > 0.0f)
		return false;

	if(startDistance < 0.0f && endDistance < 0.0f)
		return true;

	Float denominator = startDistance - endDistance;
	bool isdenominatorzero = (denominator == 0) ? true : false;

	if(startDistance >= 0.0f && endDistance <= 0.0f)
	{
		Float fraction = (!isdenominatorzero) ? ((startDistance - DIST_EPSILON) / denominator) : 0.0f;
		if(fraction > m_currentStartFraction)
		{
			m_currentStartFraction = fraction;
			m_currentNormal = planeNormal;
			m_currentPlaneDistance = planeDistance;
		}
	}
	else
	{
		Float fraction = (!isdenominatorzero) ? ((startDistance + DIST_EPSILON) / denominator) : 0.0f;
		if(fraction < m_currentEndFraction)
			m_currentEndFraction = fraction;		
	}

	return true;
}

//=============================================
// @brief Perform a line test against a triangle
//
//=============================================
bool CMCDTrace::TestLineTriangleIntersect( const Vector& start, const Vector& end, const mcdvertex_t* pvertexes, const mcdtrimeshtriangle_t* ptriangle, Vector& impactPosition, Vector& impactNormal, Float& planeDistance, Float& fraction )
{
	const Vector& vertex0 = pvertexes[ptriangle->trivertexes[0]].origin;
	const Vector& vertex1 = pvertexes[ptriangle->trivertexes[1]].origin;
	const Vector& vertex2 = pvertexes[ptriangle->trivertexes[2]].origin;

    Vector edge1 = vertex1 - vertex0;
    Vector edge2 = vertex2 - vertex0;

	Vector h;
    Math::CrossProduct( m_normDirection, edge2, h );

    Float a = Math::DotProduct( edge1, h );
    if (a > -0.0001f && a < 0.0001f) 
		return false; // ray parallel to triangle

    Float f = 1 / a;
    Vector s = start - vertex0;
    const Float u = f * Math::DotProduct( s, h );
    if (u < 0 || u > 1) 
		return false;

    Vector q;
	Math::CrossProduct( s, edge1, q );
    Float v = f * Math::DotProduct( m_normDirection, q );
    if (v < 0 || u + v > 1) 
		return false;

    const Float t = f * Math::DotProduct( edge2, q );
    if (t > 0.0001f && t < m_distance)
	{
		impactPosition = start + m_normDirection * t;
		impactNormal = ptriangle->normal;
		planeDistance = ptriangle->distance;
		fraction = t / m_baseDistance;
		m_distance = t;
		return true;
	}
	else
	{
		return false;
	}
}

//=============================================
// @brief Recurse down the tree with a point trace
//
//=============================================
void CMCDTrace::RecurseTreePointTrace( const Vector& start, const Vector& end, const mcdbvhnode_t* pbvhnode )
{
	if(!IntersectBVHNodePoint(start, end, pbvhnode->mins, pbvhnode->maxs))
		return;

	if(pbvhnode->isleaf)
	{
		// If leaf, add triangles to the list
		AddBVHNodeTriangles(pbvhnode);
	}
	else
	{
		const mcdbvhnode_t* pchildnode = m_pSubModelBVHData->getNode(m_pMCDHeader, pbvhnode->children[0]);
		RecurseTreePointTrace(start, end, pchildnode);

		pchildnode = m_pSubModelBVHData->getNode(m_pMCDHeader, pbvhnode->children[1]);
		RecurseTreePointTrace(start, end, pchildnode);	
	}
}

//=============================================
// @brief Recurse down the tree with a point trace
//
//=============================================
void CMCDTrace::RecurseTreeAABBTrace( const Vector& start, const Vector& end, const Vector& extents, const mcdbvhnode_t* pbvhnode, bool intersectTest )
{
	if(!intersectTest)
	{
		if(!IntersectBBoxSweptAABB(start, end, pbvhnode->mins, pbvhnode->maxs, extents))
			return;
	}
	else
	{
		if(!IntersectBBoxAABB(start, pbvhnode->mins, pbvhnode->maxs, extents))
			return;
	}

	if(pbvhnode->isleaf)
	{
		// If leaf, add triangles to the list
		AddBVHNodeTriangles(pbvhnode);
	}
	else
	{
		const mcdbvhnode_t* pchildnode = m_pSubModelBVHData->getNode(m_pMCDHeader, pbvhnode->children[0]);
		RecurseTreeAABBTrace(start, end, extents, pchildnode, intersectTest);

		pchildnode = m_pSubModelBVHData->getNode(m_pMCDHeader, pbvhnode->children[1]);
		RecurseTreeAABBTrace(start, end, extents, pchildnode, intersectTest);	
	}
}