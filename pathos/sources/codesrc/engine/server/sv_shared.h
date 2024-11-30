/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef SV_SHARED_H
#define SV_SHARED_H

#define MAX_CLIENTS				32		// Max connected clients
#define MAX_VISIBLE_ENTITIES	2048	// Max entities visible for a client
#define DEFAULT_MAX_EDICTS		2048	// Default max edict number

// Net update flags for specific entity property types
enum e_updflags_t
{
	NET_U_NONE			= 0,
	NET_U_ORG_ANG		= (1<<0),
	NET_U_VEL			= (1<<1),
	NET_U_VIEW			= (1<<2),
	NET_U_ENDSTART		= (1<<3),
	NET_U_MINSMAXS		= (1<<4),
	NET_U_SEQDATA		= (1<<5),
	NET_U_BASICS		= (1<<6),
	NET_U_MODELDATA		= (1<<7),
	NET_U_FLAGS			= (1<<8),
	NET_U_RENDERINFO	= (1<<9),
	NET_U_PLRINFO		= (1<<10),
	NET_U_ENTITIES		= (1<<11),
	NET_U_CONTROLLERS	= (1<<12)
};
#endif //SV_SHARED_H