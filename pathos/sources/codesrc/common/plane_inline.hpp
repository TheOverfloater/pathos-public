/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef PLANE_INLINE_HPP
#define PLANE_INLINE_HPP

//=============================================
// @brief
//
//=============================================
inline Int32 SignbitsForPlane( const plane_t& plane )
{
	Int32 bits = 0;
	for(Uint32 i = 0; i < 3; i++)
	{
		if(plane.normal[i] < 0)
			bits |= (1<<i);
	}

	return bits;
}
#endif //PLANE_INLINE_H