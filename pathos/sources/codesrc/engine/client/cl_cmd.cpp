/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "cl_entity.h"
#include "cl_main.h"
#include "system.h"
#include "cl_main.h"
#include "commands.h"
#include "networking.h"
#include "vid.h"
#include "sv_main.h"
#include "cl_resource.h"

//
// Client-side commands
//
extern void Cmd_Disconnect( void );
extern void Cmd_Connect( void );
extern void Cmd_DownloadFile( void );
extern void Cmd_Heartbeat( void );

//=============================================
// @brief
//
//=============================================
void CL_InitCommands( void )
{
	gCommands.CreateCommand("connect", Cmd_Connect, "Connect to a server at an address.\n");
	gCommands.CreateCommand("disconnect", Cmd_Disconnect, "Disconnect from the active server.\n");
	gCommands.CreateCommand("download", Cmd_DownloadFile, "Download a file from the server.\n");
	gCommands.CreateCommand("heartbeat", Cmd_Heartbeat, "Check server response status.\n");
}

//=============================================
// @brief
//
//=============================================
void Cmd_Disconnect( void )
{
	if(cls.netinfo.connecting)
	{
		cls.netinfo.connecting = false;
		cls.netinfo.nextretrytime = 0;
		cls.netinfo.numretries = 0;

		CNetworking::DeleteInstance();
		cls.netinfo.pnet = nullptr;

		Con_Printf("Connection attempt cancelled.\n");
		return;
	}

	CL_Disconnect();
}

//=============================================
// @brief
//
//=============================================
void Cmd_Connect( void )
{
	if(gCommands.Cmd_Argc() < 2)
	{
		Con_Printf("connect: <host ip>.\n");
		return;
	}

	// Disconnect from current game
	if(cls.cl_state != CLIENT_INACTIVE)
		CL_Disconnect();

	// Retrieve the IP address
	CString ipAddress = gCommands.Cmd_Argv(1);
	CL_EstablishConnection(ipAddress.c_str());
}

//=============================================
//
//=============================================
void Cmd_DownloadFile( void )
{
	if(!CL_IsGameActive())
	{
		Con_Printf("No active game.\n");
		return;
	}

	if(gCommands.Cmd_Argc() <= 1)
	{
		Con_Printf("download: <filename>.\n");
		return;
	}

	const Char* pstrFilename = gCommands.Cmd_Argv(1);
	if(!pstrFilename)
	{
		Con_Printf("download: no file specified.\n");
		return;
	}

	cl_resource_t newRes;
	newRes.fileid = cls.netinfo.resourcestlist.size()+1;
	newRes.filepath = pstrFilename;
	newRes.type = RS_TYPE_GENERIC;
	newRes.svindex = -1;
	newRes.missing = true; // Always true

	cls.netinfo.resourcestlist.radd(newRes);
	cls.netinfo.nummissingresources++;

	if(!CL_BeginFilesDownload())
	{
		Con_Printf("%s - Failed to download files.\n", __FUNCTION__);
		return;
	}
}

//=============================================
// @brief
//
//=============================================
void Cmd_Heartbeat( void )
{
	CL_SendHeartbeat(true);
}
