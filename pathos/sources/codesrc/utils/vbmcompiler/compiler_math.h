/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef COMPILER_MATH_H
#define COMPILER_MATH_H

#include "includes.h"
#include "compiler_types.h"

namespace CompilerMath
{
	// Takes the angles of an entity, and builds a rotation matrix from said angles
	void AngleMatrix( const Vector& angles, Float (*pmatrix)[4] );
	// Takes the angles of an entity, and builds an inverse rotation matrix from said angles
	void AngleInverseMatrix( const Vector& angles, Float (*pmatrix)[4] );
	// Inverts a matrix
	void InvertMatrix( const Float (*pin)[4], Float (*pout)[4] );
	// Takes an angle value in radians and moves them to the -M_PI ~ M_PI range
	Float AngleModRadians( Float angle );
	// Returns the best nearest power of 2 size for a value
	Int32 GetBestPowerOfTwo( Uint32 minimumSize, Int32 inputSize );
	// Set up matrices for a bone transform info object
	void SetupBoneTransform( Int32 boneindex, Int32 parentindex, const Vector& position, const Vector& rotation, CArray<smdl::bone_transforminfo_t>& bonetransforms );
};
#endif