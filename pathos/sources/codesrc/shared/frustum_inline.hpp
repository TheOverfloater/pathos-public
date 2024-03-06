/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef FRUSTUM_INLINE_HPP
#define FRUSTUM_INLINE_HPP

//=============================================
// @brief
//
//=============================================
inline Uint32 BoxOnPlaneSide( const Vector& mins, const Vector& maxs, const plane_t* pplane )
{
	Float dist1;
	Float dist2;

	switch(pplane->signbits)
	{
	case 0:
		dist1 = pplane->normal[0]*maxs[0] + pplane->normal[1]*maxs[1] + pplane->normal[2]*maxs[2];
		dist2 = pplane->normal[0]*mins[0] + pplane->normal[1]*mins[1] + pplane->normal[2]*mins[2];
		break;
	case 1:
		dist1 = pplane->normal[0]*mins[0] + pplane->normal[1]*maxs[1] + pplane->normal[2]*maxs[2];
		dist2 = pplane->normal[0]*maxs[0] + pplane->normal[1]*mins[1] + pplane->normal[2]*mins[2];
		break;
	case 2:
		dist1 = pplane->normal[0]*maxs[0] + pplane->normal[1]*mins[1] + pplane->normal[2]*maxs[2];
		dist2 = pplane->normal[0]*mins[0] + pplane->normal[1]*maxs[1] + pplane->normal[2]*mins[2];
		break;
	case 3:
		dist1 = pplane->normal[0]*mins[0] + pplane->normal[1]*mins[1] + pplane->normal[2]*maxs[2];
		dist2 = pplane->normal[0]*maxs[0] + pplane->normal[1]*maxs[1] + pplane->normal[2]*mins[2];
		break;
	case 4:
		dist1 = pplane->normal[0]*maxs[0] + pplane->normal[1]*maxs[1] + pplane->normal[2]*mins[2];
		dist2 = pplane->normal[0]*mins[0] + pplane->normal[1]*mins[1] + pplane->normal[2]*maxs[2];
		break;
	case 5:
		dist1 = pplane->normal[0]*mins[0] + pplane->normal[1]*maxs[1] + pplane->normal[2]*mins[2];
		dist2 = pplane->normal[0]*maxs[0] + pplane->normal[1]*mins[1] + pplane->normal[2]*maxs[2];
		break;
	case 6:
		dist1 = pplane->normal[0]*maxs[0] + pplane->normal[1]*mins[1] + pplane->normal[2]*mins[2];
		dist2 = pplane->normal[0]*mins[0] + pplane->normal[1]*maxs[1] + pplane->normal[2]*maxs[2];
		break;
	case 7:
		dist1 = pplane->normal[0]*mins[0] + pplane->normal[1]*mins[1] + pplane->normal[2]*mins[2];
		dist2 = pplane->normal[0]*maxs[0] + pplane->normal[1]*maxs[1] + pplane->normal[2]*maxs[2];
		break;
	default:
		dist1 = 0;
		dist2 = 0;
		break;
	}

	Uint32 sides = 0;
	if(dist1 >= pplane->dist)
		sides = 1;
	if(dist2 < pplane->dist)
		sides |= 2;

	return sides;
}

//=============================================
// @brief
//
//=============================================
inline void RotatePointAroundVector( const Vector& dir, const Vector& point, Float deg, Vector& dest )
{
	Float halfrad = DEG2RAD(deg)/2;
	Float sine = SDL_sin(halfrad);

	Vector q;
	Math::VectorScale(dir, sine, q);
	Float q3 = SDL_cos(halfrad);

	Vector t;
	Math::CrossProduct(q, point, t);
	Math::VectorMA(t, q3, point, t);
	Float t3 = Math::DotProduct(q, point);

	Math::CrossProduct(q, t, dest);
	Math::VectorMA(dest, t3, q, dest);
	Math::VectorMA(dest, q3, t, dest);
}

//=============================================
// @brief
//
//=============================================
Float GetXFOVFromY( Float fovY, Float ratio )
{
	Float halfradians = fovY * (0.5f * M_PI / 180.0f);
	Float t = SDL_tan(halfradians) * ratio;

	Float fovX = ((180.0f / M_PI) * SDL_atan(t)) * 2.0f;
	return fovX;
}
#endif //FRUSTUM_INLINE_HPP