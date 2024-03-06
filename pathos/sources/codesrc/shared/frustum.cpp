/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.

===============================================
*/

#include "includes.h"
#include "com_math.h"

#include "r_main.h"
#include "frustum.h"

//=============================================
// @brief Default constructor
//
//=============================================
CFrustum::CFrustum( void ):
	m_farClipDistance(0),
	m_useCullBox(false),
	m_useExtraCullBox(false)
{
}

//=============================================
// @brief Destructor
//
//=============================================
CFrustum::~CFrustum()
{
}

//=============================================
// @brief
//
//=============================================
void CFrustum::SetFrustum( const Vector& angles, const Vector& origin, Float fov, Float farplanedist )
{
	Vector forward, right, up;
	Math::AngleVectors(angles, &forward, &right, &up);

	// Set up plane normals;
	RotatePointAroundVector(up, forward, -(90 - (fov * 0.5)), m_frustumPlanes[0].normal);
	RotatePointAroundVector(up, forward, 90 - (fov * 0.5), m_frustumPlanes[1].normal);
	RotatePointAroundVector(right, forward, 90 - (fov * 0.5), m_frustumPlanes[2].normal);
	RotatePointAroundVector(right, forward, -(90 - (fov * 0.5)), m_frustumPlanes[3].normal);

	// Set other params
	for(Uint32 i = 0; i < NUM_FRUSTUM_PLANES; i++)
	{
		plane_t& plane = m_frustumPlanes[i];

		plane.type = PLANE_AZ;
		plane.dist = Math::DotProduct(origin, plane.normal);
		plane.signbits = SignbitsForPlane(plane);
	}

	// Set bounding box if needed
	if(farplanedist > 0)
	{
		for(Uint32 i = 0; i < 3; i++)
		{
			m_cullBoxMins[i] = origin[i] - farplanedist;
			m_cullBoxMaxs[i] = origin[i] + farplanedist;
		}

		m_useCullBox = true;
	}
	else
	{
		// cull box not used
		m_useCullBox = false;
	}
}

//=============================================
// @brief
//
//=============================================
void CFrustum::SetFrustum( const Vector& angles, const Vector& origin, Float fovx, Float fovy, Float farplanedist )
{
	Vector forward, right, up;
	Math::AngleVectors(angles, &forward, &right, &up);

	// Set up plane normals;
	RotatePointAroundVector(up, forward, -(90 - (fovx * 0.5)), m_frustumPlanes[0].normal);
	RotatePointAroundVector(up, forward, 90 - (fovx * 0.5), m_frustumPlanes[1].normal);
	RotatePointAroundVector(right, forward, 90 - (fovy * 0.5), m_frustumPlanes[2].normal);
	RotatePointAroundVector(right, forward, -(90 - (fovy * 0.5)), m_frustumPlanes[3].normal);

	// Set other params
	for(Uint32 i = 0; i < NUM_FRUSTUM_PLANES; i++)
	{
		plane_t& plane = m_frustumPlanes[i];

		plane.type = PLANE_AZ;
		plane.dist = Math::DotProduct(origin, plane.normal);
		plane.signbits = SignbitsForPlane(plane);
	}

	// Set bounding box if needed
	if(farplanedist > 0)
	{
		for(Uint32 i = 0; i < 3; i++)
		{
			m_cullBoxMins[i] = origin[i] - farplanedist;
			m_cullBoxMaxs[i] = origin[i] + farplanedist;
		}

		m_useCullBox = true;
	}
	else
	{
		// cull box not used
		m_useCullBox = false;
	}
}

//=============================================
// @brief
//
//=============================================
bool CFrustum::CullBBox( const Vector& mins, const Vector& maxs ) const
{
	if(m_useCullBox)
	{
		if(CheckCullBox(mins, maxs))
			return true;
	}

	if(m_useExtraCullBox)
	{
		if(CheckExtraCullBox(mins, maxs))
			return true;
	}

	for(Uint32 i = 0; i < NUM_FRUSTUM_PLANES; i++)
	{
		if(BoxOnPlaneSide(mins, maxs, &m_frustumPlanes[i]) == 2)
			return true;
	}

	return false;
}

//=============================================
// @brief
//
//=============================================
bool CFrustum::CheckCullBox( const Vector& mins, const Vector& maxs ) const
{
	for(Uint32 i = 0; i < 3; i++)
	{
		if(m_cullBoxMins[i] > maxs[i])
			return true;

		if(m_cullBoxMaxs[i] < mins[i])
			return true;
	}

	return false;
}

//=============================================
// @brief
//
//=============================================
void CFrustum::SetExtraCullBox( const Vector& vMins, const Vector& vMaxs )
{
	for(Uint32 i = 0; i < 3; i++)
		m_extraCullBoxMins[i] = vMins[i];

	for(Uint32 i = 0; i < 3; i++)
		m_extraCullBoxMaxs[i] = vMaxs[i];

	m_useExtraCullBox = true;
}

//=============================================
// @brief
//
//=============================================
bool CFrustum::CheckExtraCullBox( const Vector& mins, const Vector& maxs ) const
{
	for(Uint32 i = 0; i < 3; i++)
	{
		if(m_extraCullBoxMins[i] > maxs[i])
			return true;

		if(m_extraCullBoxMaxs[i] < mins[i])
			return true;
	}

	return false;
}