/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "winding.h"
#include "constants.h"
#include "com_math.h"

// Bogus range value
static const Float BOGUS_RANGE = 8192;

//===============================================
// @brief
//
//===============================================
CWinding::CWinding( void )
{
}

//===============================================
// @brief
//
//===============================================
CWinding::CWinding( Uint32 nbpoints )
{
	m_vertexArray.resize(nbpoints);
}

//===============================================
// @brief
//
//===============================================
CWinding::CWinding( const Vector* ppoints, Uint32 nbpoints )
{
	m_vertexArray.resize(nbpoints);

	for(Uint32 i = 0; i < nbpoints; i++)
		m_vertexArray[i] = ppoints[i];
}

//===============================================
// @brief
//
//===============================================
CWinding::CWinding( const Vector& normal, Float distance, Float planesize )
{
	// Find the major axis
	Float max = -BOGUS_RANGE;
	Int32 majorAxis = NO_POSITION;
	for(Uint32 i = 0; i < 3; i++)
	{
		Float val = SDL_fabs(normal[i]);
		if(val > max)
		{
			majorAxis = i;
			max = val;
		}
	}

	// Check if plane is valid
	if(majorAxis == NO_POSITION)
		return;

	Vector up;
	if(majorAxis < 2)
		up[2] = 1;
	else
		up[0] = 1;

	Float value = Math::DotProduct(up, normal);
	Math::VectorMA(up, -value, normal, up);
	up.Normalize();

	Vector origin, right;
	Math::VectorScale(normal, distance, origin);
	Math::CrossProduct(up, normal, right);

	up = up * planesize * 0.5;
	right = right * planesize * 0.5;

	// We create a huge plane with 4 points
	m_vertexArray.resize(4);
	
	m_vertexArray[0] = origin - right + up;
	m_vertexArray[1] = origin + right + up;
	m_vertexArray[2] = origin + right - up;
	m_vertexArray[3] = origin - right - up;
}

//===============================================
// @brief
//
//===============================================
CWinding::CWinding( const CWinding& src )
{
	m_planeNormal = src.m_planeNormal;
	m_planeDistance = src.m_planeDistance;
	m_vertexArray = src.m_vertexArray;
}

//===============================================
// @brief
//
//===============================================
CWinding::~CWinding( void )
{
}

//===============================================
// @brief
//
//===============================================
CWinding& CWinding::operator=( const CWinding& src )
{
	m_planeNormal = src.m_planeNormal;
	m_planeDistance = src.m_planeDistance;
	m_vertexArray = src.m_vertexArray;

	return *this;
}

//===============================================
// @brief
//
//===============================================
bool CWinding::operator==( const CWinding& src )
{
	if(m_planeNormal != src.m_planeNormal)
		return false;

	if(m_planeDistance != src.m_planeDistance)
		return false;

	if(m_vertexArray.size() != src.m_vertexArray.size())
		return false;

	for(Uint32 i = 0; i < m_vertexArray.size(); i++)
	{
		if(m_vertexArray[i] != src.m_vertexArray[i])
			return false;
	}

	return true;
}

//===============================================
// @brief Removes colinear points from winding
//
//===============================================
void CWinding::RemoveColinearPoints( void )
{
	CArray<Vector> newPoints;
	newPoints.reserve(m_vertexArray.size());

	for(Uint32 i = 0; i < m_vertexArray.size(); i++)
	{
		Uint32 j = (i+1) % m_vertexArray.size();
		Uint32 k = (i+m_vertexArray.size()-1) % m_vertexArray.size();

		Vector v1;
		Math::VectorSubtract(m_vertexArray[j], m_vertexArray[i], v1);
		v1.Normalize();

		Vector v2;
		Math::VectorSubtract(m_vertexArray[i], m_vertexArray[k], v2);
		v2.Normalize();

		if(Math::DotProduct(v1, v2) < 1.0 - ON_EPSILON)
			newPoints.push_back(m_vertexArray[i]);
	}

	if(m_vertexArray.size() == newPoints.size())
		return;

	m_vertexArray = newPoints;
}

//===============================================
// @brief Calculates plane of the winding
//
//===============================================
void CWinding::CalcPlane( void )
{
	if(m_vertexArray.size() < 3)
		return;

	Vector v1, v2;
	Math::VectorSubtract(m_vertexArray[1], m_vertexArray[0], v1);
	Math::VectorSubtract(m_vertexArray[2], m_vertexArray[0], v2);
	Math::CrossProduct(v2, v1, m_planeNormal);
	m_planeNormal.Normalize();

	m_planeDistance = Math::DotProduct(m_vertexArray[0], m_planeNormal);
}

//===============================================
// @brief Returns the area of the winding
//
//===============================================
Float CWinding::GetArea( void )
{
	Float total = 0;
	for(Uint32 i = 2; i < m_vertexArray.size(); i++)
	{
		Vector d1, d2, cross;
		Math::VectorSubtract(m_vertexArray[i-1], m_vertexArray[0], d1);
		Math::VectorSubtract(m_vertexArray[i], m_vertexArray[0], d2);
		Math::CrossProduct(d1, d2, cross);

		total += cross.Length() * 0.5;
	}

	return total;
}

//===============================================
// @brief Returns the bounding box of the winding
//
//===============================================
void CWinding::GetBounds( Vector& mins, Vector& maxs )
{
	mins = m_mins;
	maxs = m_maxs;
}

//===============================================
// @brief Returns the bounding box of the winding
//
//===============================================
void CWinding::CalcBounds( void )
{
	m_mins = NULL_MINS;
	m_maxs = NULL_MAXS;

	for(Uint32 i = 0; i < m_vertexArray.size(); i++)
	{
		Vector& v = m_vertexArray[i];
		for(Uint32 j = 0; j < 3; j++)
		{
			if(v[j] < m_mins[j])
				m_mins[j] = v[j];

			if(v[j] > m_maxs[j])
				m_maxs[j] = v[j];
		}
	}
}

//===============================================
// @brief Get winding center
//
//===============================================
Vector CWinding::GetCenter( void )
{
	Vector center;
	for(Uint32 i = 0; i < m_vertexArray.size(); i++)
		center += m_vertexArray[i];

	return center * (1.0 / static_cast<Float>(m_vertexArray.size()));
}

//===============================================
// @brief Clips winding
//
//===============================================
void CWinding::Clip( const Vector& normal, const Float& distance, CWinding*& ptrfront, CWinding*& ptrback )
{
	Uint32 counts[SIDE_NB] = { 0 };

	Uint32 mypointnb = m_vertexArray.size();
	CArray<Float> distances(mypointnb+1);
	CArray<planeside_t> sides(mypointnb+1);

	// Determine where on the splitting plane the points are located
	for(Uint32 i = 0; i < mypointnb; i++)
	{
		Float dp = Math::DotProduct(m_vertexArray[i], normal);
		dp -= distance;

		distances[i] = dp;
		if(dp > ON_EPSILON)
			sides[i] = SIDE_FRONT;
		else if(dp < -ON_EPSILON)
			sides[i] = SIDE_BACK;
		else
			sides[i] = SIDE_ON;

		counts[i]++;
	}

	// Close off the list
	sides[mypointnb] = sides[0];
	distances[mypointnb] = distances[0];

	if(!counts[SIDE_FRONT])
	{
		ptrback = new CWinding(*this);
		return;
	}
	else if(!counts[SIDE_BACK])
	{
		ptrfront = new CWinding(*this);
		return;
	}

	Uint32 pointsalloc = mypointnb + 4;
	Uint32 nbback = 0;
	CArray<Vector> backpoints(pointsalloc);

	Uint32 nbfront = 0;
	CArray<Vector> frontpoints(pointsalloc);

	for(Uint32 i = 0; i < mypointnb; i++)
	{
		Vector p1 = m_vertexArray[i];
		if(sides[i] == SIDE_ON)
		{
			// Add to back
			if(backpoints.size() == nbback)
				backpoints.resize(backpoints.size() + pointsalloc);

			backpoints[nbback] = p1;
			nbback++;

			// Add to front
			if(frontpoints.size() == nbfront)
				frontpoints.resize(frontpoints.size() + pointsalloc);

			frontpoints[nbfront] = p1;
			nbfront++;
		}
		else
		{
			if(sides[i] == SIDE_FRONT)
			{
				// Add to front
				if(frontpoints.size() == nbfront)
					frontpoints.resize(frontpoints.size() + pointsalloc);

				frontpoints[nbfront] = p1;
				nbfront++;
			}
			else if(sides[i] == SIDE_BACK)
			{
				// Add to back
				if(backpoints.size() == nbback)
					backpoints.resize(backpoints.size() + pointsalloc);

				backpoints[nbback] = p1;
				nbback++;
			}

			if(sides[i+1] == SIDE_ON || sides[i+1] == sides[i])
				continue;

			Uint32 index = (i + 1) % mypointnb;
			Vector p2 = m_vertexArray[index];
			Float dp = distances[i] / (distances[i] - distances[i+1]);

			Vector middle;
			for(Uint32 j = 0; j < 3; i++)
			{
				if(normal[j] == 1)
					middle[j] = distance;
				else if(normal[j] == -1)
					middle[j] = -distance;
				else
					middle[j] = p1[j] + dp*(p2[j] - p1[j]);
			}

			// Add to back
			if(backpoints.size() == nbback)
				backpoints.resize(backpoints.size() + pointsalloc);

			backpoints[nbback] = middle;
			nbback++;

			// Add to front
			if(frontpoints.size() == nbfront)
				frontpoints.resize(frontpoints.size() + pointsalloc);

			frontpoints[nbfront] = middle;
			nbfront++;
		}
	}

	// Allocate final results
	ptrfront = new CWinding(&frontpoints[0], nbfront);
	ptrback = new CWinding(&backpoints[0], nbback);
}

//===============================================
// @brief Chop winding by plane, and return the front part
//
//===============================================
CWinding* CWinding::Chop( const Vector& normal, Float distance )
{
	CWinding* pfront;
	CWinding* pback;

	Clip(normal, distance, pfront, pback);
	if(pback)
		delete pback;

	return pfront;
}

//===============================================
// @brief Check if winding is valid
//
//===============================================
bool CWinding::IsValid( void )
{
	if(m_vertexArray.size() < 3)
		return false;

	if(GetArea() < 1.0)
		return false;

	// Re-calculate plane
	CalcPlane();

	for(Uint32 i = 0; i < m_vertexArray.size(); i++)
	{
		Vector p1 = m_vertexArray[i];

		// Check if we're over the bogus range
		for(Uint32 j = 0; j < 3; j++)
		{
			if(p1[j] > BOGUS_RANGE || p1[j] < -BOGUS_RANGE)
				return false;
		}

		Uint32 j = (i + 1) == m_vertexArray.size() ? 0 : (i + 1);

		// Check if the vertex is off the plane
		Float dp = Math::DotProduct(p1, m_planeNormal) - m_planeDistance;
		if(dp < -ON_EPSILON || dp > ON_EPSILON)
			return false;

		Vector direction;
		Vector p2 = m_vertexArray[j];
		Math::VectorSubtract(p2, p1, direction);

		// Degenerate edge check
		if(direction.Length() < ON_EPSILON)
			return false;

		Vector edgenormal;
		Math::CrossProduct(m_planeNormal, direction, edgenormal);
		edgenormal.Normalize();

		Float edgedist = Math::DotProduct(p1, edgenormal);
		edgedist += ON_EPSILON;

		for(j = 0; j < m_vertexArray.size(); j++)
		{
			if(j == i)
				continue;

			// Check if non-convex
			dp = Math::DotProduct(m_vertexArray[j], edgenormal);
			if(dp > edgedist)
				return false;
		}
	}

	return true;
}

//===============================================
// @brief Tell what side of the plane the winding is on
//
//===============================================
planeside_t CWinding::OnPlaneSide( const Vector& normal, Float distance )
{
	bool front = false;
	bool back = false;

	for(Uint32 i = 0; i < m_vertexArray.size(); i++)
	{
		Float dp = Math::DotProduct(m_vertexArray[i], normal) - distance;
		if(dp < -ON_EPSILON)
		{
			if(front)
			{
				return SIDE_CROSS;
			}
			else
			{
				back = true;
				continue;
			}
		}
		else if(dp > ON_EPSILON)
		{
			if(back)
			{
				return SIDE_CROSS;
			}
			else
			{
				front = true;
				continue;
			}
		}
	}

	if(back)
		return SIDE_BACK;
	else if(front)
		return SIDE_FRONT;
	else
		return SIDE_ON;
}