/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "cplane.h"
#include "com_math.h"

//=============================================
// @brief Constructor
//
//=============================================
CPlane::CPlane( const Vector& normal, const Vector& point ):
	m_planeNormal(normal),
	m_planeDistance(0)
{
	m_planeDistance = Math::DotProduct(m_planeNormal, point);
}

//=============================================
// @brief Destructor
//
//=============================================
CPlane::~CPlane( void )
{
}

//=============================================
// @brief Tests if a point is in front of the plane
//
//=============================================
bool CPlane::IsPointInFront( const Vector& point )
{
	Float dp = Math::DotProduct(m_planeNormal, point) - m_planeDistance;
	return (dp >= 0) ? true : false;
}