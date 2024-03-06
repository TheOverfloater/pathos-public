/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#ifndef CPLANE_H
#define CPLANE_H

//=============================================
//
//=============================================
class CPlane
{
public:
	CPlane( const Vector& normal, const Vector& point );
	~CPlane( void );

public:
	// Tells if the point is in front of the plane
	bool IsPointInFront( const Vector& point );

private:
	// Normal of the plane
	Vector m_planeNormal;
	// Plane distance
	Float m_planeDistance;
};
#endif //CPLANE_H