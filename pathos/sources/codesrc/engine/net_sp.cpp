/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "com_math.h"
#include "file.h"

#include "edict.h"
#include "networking.h"
#include "net_sp.h"
#include "enginestate.h"
#include "sv_main.h"
#include "system.h"

//=============================================
// @brief Default constructor
//
//=============================================
CSPNetworking::CSPNetworking( void )
{
}

//=============================================
// @brief Destructor
//
//=============================================
CSPNetworking::~CSPNetworking( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CSPNetworking::Init( const Char* pstrhost )
{
	// Call base class to initialize
	if(!CNetworking::Init(pstrhost))
		return false;

	return true;
}

//=============================================
// @brief
//
//=============================================
net_msgcache_t* CSPNetworking::CLS_GetWriteCache( void )
{
	return CNetworking::CLS_GetWriteCache_Local();
}

//=============================================
// @brief
//
//=============================================
void CSPNetworking::SVC_MessageEnd( void )
{
	CNetworking::SVC_MessageEnd_Local();
}

//=============================================
// @brief
//
//=============================================
net_msgcache_t* CSPNetworking::SVC_GetWriteCache( Uint32 clientidx )
{
	return CNetworking::SVC_GetWriteCache_Local(clientidx);
}

//=============================================
// @brief
//
//=============================================
void CSPNetworking::CLS_MessageEnd( void )
{
	CNetworking::CLS_MessageEnd_Local();
}
