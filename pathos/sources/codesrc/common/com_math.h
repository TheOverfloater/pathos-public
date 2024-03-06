/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef COM_MATH_H
#define COM_MATH_H

#include <math.h>
namespace Math
{
	extern inline bool VectorCompare( const Vector& v1, const Vector& v2 );
	extern inline void VectorCopy( const Vector& src, Vector& dest );
	extern inline void VectorCopy( const Float* psrc, Vector& dest );
	extern inline void VectorCopy( const Float* psrc, Float* pdest );
	extern inline void VectorCopy( const Vector& src, Float* pdest );
	extern inline void VectorClear( Vector& dest );
	extern inline void VectorSubtract( const Vector& v1, const Vector& v2, Vector& dest );
	extern inline void VectorAdd( const Vector& v1, const Vector& v2, Vector& dest );
	extern inline void VectorMA( const Vector& v1, Float scale, const Vector& v2, Vector& dest );
	extern inline void VectorScale( const Vector& src, Float scale, Vector& dest );
	extern inline Float DotProduct( const Vector& v1, const Vector& v2 );
	extern inline Float DotProduct( const Vector& v1, const Float* pv2 );
	extern inline Float DotProduct4( const Float* pv1, const Float* pv2 );
	extern inline void CrossProduct( const Vector& v1, const Vector& v2, Vector& dest );
	extern inline void AngleVectors( const Vector& angles, Vector* pforward );
	extern inline void AngleVectors( const Vector& angles, Vector* pforward, Vector* pright );
	extern inline void AngleVectors( const Vector& angles, Vector* pforward, Vector* pright, Vector* pup );
	extern inline void AngleVectorsTranspose( const Vector& angles, Vector* pforward );
	extern inline void AngleVectorsTranspose( const Vector& angles, Vector* pforward, Vector* pright );
	extern inline void AngleVectorsTranspose( const Vector& angles, Vector* pforward, Vector* pright, Vector* pup );
	extern inline Float AngleMod( Float angle );
	extern inline Float AngleDiff( Float destangle, Float srcangle );
	extern inline Float VectorNormalize( Vector& v );
	extern inline bool IsVectorZero( const Vector& v );
	extern inline bool CheckMinsMaxs( const Vector& mins1, const Vector& maxs1, const Vector& mins2, const Vector& maxs2 );
	extern inline void RotateToEntitySpace( const Vector& angles, Vector& vec );
	extern inline void RotateFromEntitySpace( const Vector& angles, Vector& vec );
	extern inline bool PointInMinsMaxs( const Vector& point, const Vector& mins, const Vector& maxs );
	extern inline void AngleMatrix( const Vector& angles, Float (*pmatrix)[4] );
	extern inline void AngleInverseMatrix( const Vector& angles, Float (*pmatrix)[4] );
	extern inline void VectorRotate( const Vector& vec, const Float (*pmatrix)[4], Vector& out );
	extern inline void VectorInverseRotate( const Vector& vec, const Float (*pmatrix)[4], Vector& out );
	extern inline Vector VectorToAngles( const Vector& forward, const Vector& left );
	extern inline Vector VectorToAngles( const Vector& forward );
	extern inline void MatMultPosition( const Float *flmatrix, const Vector& vecin, Vector *vecout );
	extern inline void MatMult( const Float *flmatrix, const Vector& vecin, Vector *vecout );
	extern inline void MatMult4( const Float *flmatrix, const Float *vecin, Float *vecout );
	extern inline void GetUpRight( const Vector& forward, Vector &up, Vector &right );
	extern inline void VectorTransform( const Vector& in, Float (*pmatrix)[4], Vector& out );
	extern inline void QuaternionMatrix( const vec4_t& quaternion, Float (*pmatrix)[4] );
	extern inline void AngleQuaternion( const Vector& angles, vec4_t& quaternion );
	extern inline void QuaternionBlend( const vec4_t& q1, const vec4_t& q2, Float interp, vec4_t& outq );
	extern inline void ConcatTransforms( const Float (*pin1)[4], const Float (*pin2)[4], Float (*pout)[4] );
	extern inline void CopyMatrix( const Float (*pin)[4], Float (*pout)[4] );
	extern inline Vector AdjustAnglesToNormal( const Vector& normal, const Vector& angles );
};
#include "com_math_inline.hpp"
#endif //Common::MATH_H