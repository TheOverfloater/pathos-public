/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include <SDL.h>
#ifdef WIN32
#include <Windows.h>
#endif

#include "compiler_math.h"
#include "com_math.h"
#include "compiler_types.h"

namespace CompilerMath
{
	//=============================================
	// @brief Takes the angles of an entity, and builds
	// a rotation matrix from said angles. This function
	// is specific to the compiler, because it uses different
	// axes to compute the matrix.
	//
	// @param angles Angles of entity
	// @param pmatrix Output matrix of 3x4 floats to hold the
	// rotation matrix
	//=============================================
	void AngleMatrix( const Vector& angles, Float (*pmatrix)[4] )
	{
		Float angle = angles[ROLL]*(M_PI*2/360);
		Float sy = SDL_sin(angle);
		Float cy = SDL_cos(angle);

		angle = angles[YAW]*(M_PI*2/360);
		Float sp = SDL_sin(angle);
		Float cp = SDL_cos(angle);

		angle = angles[PITCH]*(M_PI*2/360);
		Float sr = SDL_sin(angle);
		Float cr = SDL_cos(angle);

		pmatrix[0][0] = cp*cy;
		pmatrix[1][0] = cp*sy;
		pmatrix[2][0] = -sp;

		pmatrix[0][1] = sr*sp*cy+cr*-sy;
		pmatrix[1][1] = sr*sp*sy+cr*cy;
		pmatrix[2][1] = sr*cp;

		pmatrix[0][2] = (cr*sp*cy+-sr*-sy);
		pmatrix[1][2] = (cr*sp*sy+-sr*cy);
		pmatrix[2][2] = cr*cp;

		pmatrix[0][3] = 0;
		pmatrix[1][3] = 0;
		pmatrix[2][3] = 0;
	}

	//=============================================
	// @brief Takes the angles of an entity, and builds
	// a rotation matrix from said angles. This function
	// is specific to the compiler, because it uses different
	// axes to compute the matrix.
	//
	// @param angles Angles of entity
	// @param pmatrix Output matrix of 3x4 floats to hold the
	// inverse rotation matrix
	//=============================================
	void AngleInverseMatrix( const Vector& angles, Float (*pmatrix)[4] )
	{
		Float angle = angles[ROLL] * (M_PI*2 / 360);
		Float sy = sin(angle);
		Float cy = cos(angle);
		angle = angles[YAW] * (M_PI*2 / 360);
		Float sp = sin(angle);
		Float cp = cos(angle);
		angle = angles[PITCH] * (M_PI*2 / 360);
		Float sr = sin(angle);
		Float cr = cos(angle);

		pmatrix[0][0] = cp*cy;
		pmatrix[0][1] = cp*sy;
		pmatrix[0][2] = -sp;

		pmatrix[1][0] = sr*sp*cy+cr*-sy;
		pmatrix[1][1] = sr*sp*sy+cr*cy;
		pmatrix[1][2] = sr*cp;

		pmatrix[2][0] = (cr*sp*cy+-sr*-sy);
		pmatrix[2][1] = (cr*sp*sy+-sr*cy);
		pmatrix[2][2] = cr*cp;

		pmatrix[0][3] = 0;
		pmatrix[1][3] = 0;
		pmatrix[2][3] = 0;
	}

	//=============================================
	// @brief Inverts a matrix
	//
	// @param pin Matrix to calculate inverse of
	// @param pout Matrix to output inverse matrix into
	//=============================================
	void InvertMatrix( const Float (*pin)[4], Float (*pout)[4] )
	{
		for(Uint32 i = 0; i < 3; i++)
		{
			for(Uint32 j = 0; j < 3; j++)
				pout[i][j] = pin[j][i];
		}

		Vector tmp(pin[0][3], pin[1][3], pin[2][3]);
		for(Uint32 i = 0; i < 3; i++)
			pout[i][3] = -Math::DotProduct(tmp, pout[i]);
	}
 
	//=============================================
	// @brief Takes Takes an angle value in radians 
	// and moves them to the -M_PI ~ M_PI range
	//
	// @param angle Angle value to modulate
	// @return Modulated angle value
	//=============================================
	Float AngleModRadians( Float angle )
	{
		Float _angle = angle;
		if(_angle >= M_PI)
			_angle -= M_PI * 2;
		if(_angle < -M_PI)
			_angle += M_PI * 2;

		return _angle;
	}

	//=============================================
	// @brief Returns the best nearest power of 2 
	// size for a value
	//
	// @param minimumSize Minimum size a texture can have
	// @param inputSize Original texture size
	// @return Nearest power of two value higher than inputSize
	//=============================================
	Int32 GetBestPowerOfTwo( Uint32 minimumSize, Int32 inputSize )
	{
		Uint32 resolution = minimumSize;
		
		Uint32 i = minimumSize;
		while(true)
		{
			if(i >= inputSize)
				break;

			i *= 2;
		}

		return i;
	}

	//=============================================
	// Set up matrices for a bone transform info object
	//
	// @param destnode Node to set up transforms for
	// @param destbone Bone to set up transforms for
	// @param desttransform Destination of transform data we're setting up
	// @param bonetransforms Bone transforms array
	//=============================================
	void SetupBoneTransforms( const smdl::bone_node_t& destnode, const smdl::bone_t& destbone, smdl::bone_transforminfo_t& desttransform, CArray<smdl::bone_transforminfo_t>& bonetransforms )
	{
		// Convert from radians to degrees
		Vector angles;
		for(Uint32 j = 0; j < 3; j++)
			angles[j] = destbone.rotation[j] * (180.0 / M_PI);

		// Set up transform matrices based on whether we have a parent or not
		if(destnode.parentindex == NO_POSITION)
		{
			// Note: Use custom math functions here, as AngleMatrix 
			// and AngleInverseMatrix need to use different axes
			CompilerMath::AngleMatrix(angles, desttransform.matrix);

			for(Uint32 i = 0; i < 3; i++)
				desttransform.matrix[i][3] = destbone.position[i];
		}
		else
		{
			// Set up rotation matrix
			Float matrix[3][4];

			// Note: Use custom math functions here, as AngleMatrix 
			// and AngleInverseMatrix need to use different axes
			CompilerMath::AngleMatrix(angles, matrix);

			// Set position
			for(Uint32 i = 0; i < 3; i++)
				matrix[i][3] = destbone.position[i];

			// Transform by parent
			smdl::bone_transforminfo_t parentTransformInfo = bonetransforms[destnode.parentindex];
			Math::ConcatTransforms(parentTransformInfo.matrix, matrix, desttransform.matrix);
		}
	}
};