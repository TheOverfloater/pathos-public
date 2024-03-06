/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef NET_SHARED_H
#define NET_SHARED_H

//
// Messages destination types
// 
enum msgdest_t
{
	MSG_UNDEFINED = -1,
	MSG_ONE = 0,
	MSG_ALL,
	MSG_BROADCAST,
	MSG_ALL_UNRELIABLE,
	MSG_ONE_UNRELIABLE
};

//
// Message flags
//
enum msgflags_t
{
	 MSG_FL_NONE		= 0,
	 MSG_FL_COMPRESSED	= (1<<0)
};

#endif //NET_SHARED_H