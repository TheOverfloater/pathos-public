/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef WINDING_H
#define WINDING_H

#include "includes.h"
#include "constants.h"

/*
=======================
CWinding

=======================
*/
class CWinding
{
public:
	CWinding( void );
	CWinding( Uint32 nbpoints );
	CWinding( const Vector* ppoints, Uint32 nbpoints );
	CWinding( const Vector& normal, Float distance, Float planesize );
	CWinding( const CWinding& src );
	~CWinding( void );

public:
	// Assignment operator
	CWinding& operator=( const CWinding& src );
	// Assignment operator
	bool operator==( const CWinding& src );

public:
	// Tells if the winding is a valid object
	bool IsValid( void ) const { return m_vertexArray.empty() ? false : true; }
	// Removes colinear points from winding
	void RemoveColinearPoints( void );
	// Returns the area of the winding
	Float GetArea( void );
	// Returns the bounding box of the winding
	void GetBounds( Vector& mins, Vector& maxs );
	// Get winding center
	Vector GetCenter( void );
	// Check if winding is valid
	bool IsValid( void );
	// Tell what side of the plane the winding is on
	planeside_t OnPlaneSide( const Vector& normal, Float distance );

	// Clips winding
	void Clip( const Vector& normal, const Float& distance, CWinding*& ptrfront, CWinding*& ptrback );
	// Chop winding by plane, and return the front part
	CWinding* Chop( const Vector& normal, Float distance );

private:
	// Calculates plane of the winding
	void CalcPlane( void );
	// Returns the bounding box of the winding
	void CalcBounds( void );

private:
	// Mins
	Vector m_mins;
	// Maxs
	Vector m_maxs;
	// Normal of the plane
	Vector m_planeNormal;
	// Plane distance value
	Float m_planeDistance;
	// Array of vertices
	CArray<Vector> m_vertexArray;
};
#endif