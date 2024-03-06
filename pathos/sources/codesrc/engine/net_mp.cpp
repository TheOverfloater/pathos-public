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
#include "net_mp.h"
#include "enginestate.h"
#include "sv_main.h"
#include "system.h"
#include "cl_main.h"

// Amount of time without messages until we try to reconnect
static const Float MSG_TIMEOUT_DELAY = 5.0f;
// Reconnect attmept delay time
static const Float RECONNECT_ATTEMPT_DELAY_TIME = 5.0f;

//=============================================
// @brief Default constructor
//
//=============================================
CMPNetworking::CMPNetworking( void ):
	m_pWriteCache(nullptr),
	m_pHost(nullptr),
	m_hostPort(0),
	m_lastReconnectAttemptTime(0)
{
}

//=============================================
// @brief Destructor
//
//=============================================
CMPNetworking::~CMPNetworking( void )
{
	if(m_pHost)
	{
		enet_host_destroy(m_pHost);
		m_pHost = nullptr;
	}

	if(m_pWriteCache)
	{
		delete m_pWriteCache;
		m_pWriteCache = nullptr;
	}

	// Clear enet
	enet_deinitialize();
}

//=============================================
// @brief
//
//=============================================
bool CMPNetworking::Init( const Char* pstrhost )
{
	// Call base class to initialize
	if(!CNetworking::Init(pstrhost))
		return false;

	if(enet_initialize() != 0)
	{
		Con_EPrintf("Failed to initialize networking.\n");
		return false;
	}
	
	// Get port number
	Uint32 port = g_pCVarPort->GetValue();

	// Create the write cache
	m_pWriteCache = new net_msgcache_t(MSG_BUFFER_ALLOC_SIZE, MSG_DATA_BUFFER_ALLOC_SIZE);

	// Initialize networking based on host type
	if(!qstrcmp(pstrhost, "localhost"))
	{
		if(!InitServerNetworking(port))
		{
			Con_EPrintf("Failed to initialize host.\n");
			return false;
		}
	}
	else
	{
		// Connect to a remote host
		if(!InitClientNetworking(port, pstrhost))
			return false;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CMPNetworking::InitServerNetworking( Uint32 port )
{
	// Get local client
	net_client_t& cl = m_clientsArray[0];

	// Set localhost's address
	cl.ip_address.host = ENET_HOST_ANY;
	cl.ip_address.port = port;

	m_pHost = enet_host_create(&cl.ip_address, svs.maxclients-1, 2, 0, 0);
	if(!m_pHost)
	{
		Con_DPrintf("Failed to initialize localhost.\n");
		return false;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CMPNetworking::InitClientNetworking( Uint32 port, const Char* pstrhost )
{
	// Set these
	m_hostAddress = pstrhost;
	m_hostPort = port;

	// Set up the local client
	return AttemptConnection();
}

//=============================================
// @brief
//
//=============================================
bool CMPNetworking::AttemptConnection( void )
{
	// Set up the local client
	net_client_t& cl = m_clientsArray[0];

	// Clear client if he's connected
	if(cl.ptrpeer)
	{
		cl.ptrpeer->data = nullptr;
		enet_peer_reset(cl.ptrpeer);
		cl.ptrpeer = nullptr;
	}

	if(m_pHost)
	{
		enet_host_destroy(m_pHost);
		m_pHost = nullptr;
	}

	// Use only one connection with clients
	m_pHost = enet_host_create(nullptr, 1, 2, 0, 0);
	if(!m_pHost)
	{
		Con_DPrintf("Failed to initialize localhost.\n");
		return false;
	}

	// Try connecting to the remote host
	enet_address_set_host(&cl.ip_address, m_hostAddress.c_str());
	cl.ip_address.port = m_hostPort;

	// Try connecting to the host
	cl.ptrpeer = enet_host_connect(m_pHost, &cl.ip_address, 2, 0);
	if(!cl.ptrpeer)
	{
		Con_DPrintf("No available peers for initiating network connection.\n");
		enet_host_destroy(m_pHost);
		m_pHost = nullptr;
		return false;
	}

	// Flush the current datagram
	enet_host_flush(m_pHost);

	// See if we've connected, wait five seconds before failing
	ENetEvent netEvent;
	if(enet_host_service(m_pHost, &netEvent, 1000) > 0 && netEvent.type == ENET_EVENT_TYPE_CONNECT)
	{
		Con_Printf("Connected to %s:%d.\n", m_hostAddress.c_str(), m_hostPort);
	}
	else
	{
		Con_Printf("Couldn't connect to %s:%d.\n", m_hostAddress.c_str(), m_hostPort);

		if(cl.ptrpeer)
		{
			cl.ptrpeer->data = nullptr;
			enet_peer_reset(cl.ptrpeer);
			cl.ptrpeer = nullptr;
		}

		enet_host_destroy(m_pHost);
		m_pHost = nullptr;
		return false;
	}
	
	// Reset this
	cl.timeoutbegintime = ens.time;
	cl.lastmsgtime = ens.time;
	cl.cl_state = NETCL_CONNECTED;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CMPNetworking::Poll( void )
{
	if(svs.serverstate == SV_ACTIVE)
		Poll_Host();
	else
		Poll_Client();

	CNetworking::Poll();
}

//=============================================
// @brief
//
//=============================================
void CMPNetworking::Disconnect( Uint32 clindex )
{
	net_client_t& cl = m_clientsArray[clindex];
	if(cl.ptrpeer)
	{
		// Forcefully disconnect peer
		cl.ptrpeer->data = nullptr;
		enet_peer_reset(cl.ptrpeer);
		cl.ptrpeer = nullptr;
	}

	// Reset client info
	ResetClient(cl);
}

//=============================================
// @brief
//
//=============================================
void CMPNetworking::ConnectionLost( Uint32 clindex )
{
	assert(clindex < m_clientsArray.size());
	net_client_t& cl = m_clientsArray[clindex];
	if(cl.timeoutbegintime != 0 && cl.cl_state == NETCL_LOST_CONNECTION)
		return;

	cl.cl_state = NETCL_LOST_CONNECTION;
	cl.timeoutbegintime = ens.time;
}

//=============================================
// @brief
//
//=============================================
void CMPNetworking::Poll_Host( void )
{
	ENetEvent netEvent;
	while(enet_host_service(m_pHost, &netEvent, 0) > 0)
	{
		switch(netEvent.type)
		{
		case ENET_EVENT_TYPE_CONNECT:
			ClientConnect(&netEvent);
			break;

		case ENET_EVENT_TYPE_RECEIVE:
			{
				Uint32 clientindex = *reinterpret_cast<Uint32*>(netEvent.peer->data);
				assert(clientindex > 0);

				net_client_t& cl = m_clientsArray[clientindex];
				ReadUDPMessage(cl.pcls_cache, &netEvent);
				enet_packet_destroy(netEvent.packet);

				cl.lastmsgtime = ens.time;
				m_nbCLSMessagesRecieved++;
			}
			break;

		case ENET_EVENT_TYPE_DISCONNECT:
			{
				Uint32 clientindex = *reinterpret_cast<Uint32*>(netEvent.peer->data);
				assert(clientindex > 0);

				ConnectionLost(clientindex);
			}
			break;

		default:
			Con_Printf("%s - Unhandled enet event %d.\n", __FUNCTION__, netEvent.type);
			break;
		}
	}

	// Keep updating connected clients
	for(Uint32 i = 1; i < m_clientsArray.size(); i++)
	{
		net_client_t& client = m_clientsArray[i];
		if(client.cl_state == NETCL_CONNECTED || client.cl_state == NETCL_DISCONNECTED || client.cl_state == NETCL_STATE_NONE)
			continue;

		if(client.lastmsgtime && client.lastmsgtime + MSG_TIMEOUT_DELAY < ens.time)
		{
			// We haven't received messages from the client in a while, so mark him as having lost connection
			client.cl_state = NETCL_LOST_CONNECTION;
			m_lastReconnectAttemptTime = ens.time;
		}

		// Handle any networking errors
		if(client.cl_state == NETCL_NET_ERROR)
		{
			Disconnect(i);
			continue;
		}

		// Disconnect client if connection was lost for too long
		if(client.cl_state == NETCL_LOST_CONNECTION)
		{
			Float timeouttime = ens.time - client.timeoutbegintime;
			if(timeouttime >= CLIENT_TIMEOUT_LIMIT)
			{
				Con_Printf("Connection timed out for client.\n");
				Disconnect(i);
				continue;
			}
		}
	}
}

//=============================================
// @brief
//
//=============================================
void CMPNetworking::Poll_Client( void )
{
	// Get local client
	net_client_t& client = m_clientsArray[0];

	// If connection was lost, try to re-establish it
	if(client.cl_state == NETCL_LOST_CONNECTION)
	{
		Float timeouttime = ens.time - client.timeoutbegintime;
		if(timeouttime >= CLIENT_TIMEOUT_LIMIT)
		{
			Con_Printf("Connection timed out.\n");
			CL_Disconnect();
			return;
		}

		// Try reconnecting periodically
		if(m_lastReconnectAttemptTime + RECONNECT_ATTEMPT_DELAY_TIME < ens.time)
		{
			m_lastReconnectAttemptTime = ens.time;

			Con_Printf("Attempting to reconnect to '%s'.\n", m_hostAddress.c_str());
			if(AttemptConnection())
				Con_Printf("Successfully restored connection to host.\n");
		}

		// Do nothing else until we get a connection back
		return;
	}

	if(!m_pHost)
		return;

	// Just poll messages
	ENetEvent netEvent;
	while(enet_host_service(m_pHost, &netEvent, 0) > 0)
	{
		switch(netEvent.type)
		{
		case ENET_EVENT_TYPE_RECEIVE:
			ReadUDPMessage(client.psvc_cache, &netEvent);
			enet_packet_destroy(netEvent.packet);

			client.lastmsgtime = ens.time;
			m_nbSVCMessagesRecieved++;
			break;

		case ENET_EVENT_TYPE_DISCONNECT:
			ConnectionLost(client.index);
			break;

		default:
			Con_Printf("%s - Unhandled enet event %d.\n", __FUNCTION__, netEvent.type);
			break;
		}
	}

	if(client.lastmsgtime && client.lastmsgtime + MSG_TIMEOUT_DELAY < ens.time)
	{
		// We haven't received messages from the server in a while, so try and reconnect
		client.cl_state = NETCL_LOST_CONNECTION;
		m_lastReconnectAttemptTime = ens.time;
	}
}

//=============================================
// @brief
//
//=============================================
void CMPNetworking::ClientConnect( ENetEvent* pevent )
{
	// See if it's an already connected client that's timed out
	for(Uint32 i = 0; i < m_clientsArray.size(); i++)
	{
		net_client_t& cl = m_clientsArray[i];
		if(cl.cl_state != NETCL_LOST_CONNECTION)
			continue;

		if(pevent->peer->address.host == cl.ip_address.host)
		{
			// Set as new peer and return
			cl.ptrpeer = pevent->peer;

			// Set address too
			cl.ip_address.host = pevent->peer->address.host;
			cl.ip_address.port = pevent->peer->address.port;

			// Reset these
			cl.timeoutbegintime = 0;
			cl.cl_state = NETCL_CONNECTED;
			return;
		}
	}

	// If not, find an empty slot for him
	Uint32 i = 1;
	for(; i < m_clientsArray.size(); i++)
	{
		const net_client_t& cl = m_clientsArray[i];
		if(cl.cl_state == NETCL_STATE_NONE || cl.cl_state == NETCL_DISCONNECTED)
			break;
	}

	if(i == m_clientsArray.size())
	{
		// No free slots found
		RejectClient(pevent->peer, "No free slots available");
		return;
	}

	// Get client
	net_client_t& cl = m_clientsArray[i];

	// Set the peer's index
	pevent->peer->data = &cl.index;
	cl.cl_state = NETCL_CONNECTED;
	cl.ptrpeer = pevent->peer;

	cl.ip_address.host = pevent->peer->address.host;
	cl.ip_address.port = pevent->peer->address.port;

	// Call server to handle basics
	SV_EstablishedClientConnection(i);
}

//=============================================
// @brief
//
//=============================================
void CMPNetworking::RejectClient( ENetPeer* ptrpeer, const Char* pstrreason )
{
	// Create a temporary buffer
	Uint32 stringlength = qstrlen(pstrreason)+1;
	Uint32 msgsize = sizeof(byte) + sizeof(byte) + stringlength;
	byte* pmsg = new byte[msgsize];
	memset(pmsg, 0, sizeof(byte)*msgsize);
	
	Uint32 msgofs = 0;
	(pmsg[msgofs++]) = svc_disconnect;
	(pmsg[msgofs++]) = 1;

	Char *pdest = reinterpret_cast<Char*>(pmsg+msgofs);
	qstrcpy(pdest, pstrreason);

	// Send it over to the client
	SendUDPMessage(MSG_ONE, pmsg, msgsize, ptrpeer);
	enet_host_flush(m_pHost);
	delete[] pmsg;

	// Reset the peer
	enet_peer_reset(ptrpeer);
}

//=============================================
// @brief
//
//=============================================
bool CMPNetworking::SendUDPMessage( msgdest_t dest, byte* pdata, Uint32 msgsize, ENetPeer* ptrpeer )
{
	Uint32 packetflags = 0;
	if( dest == MSG_ONE_UNRELIABLE || dest == MSG_ALL_UNRELIABLE )
		packetflags |= ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT;
	else
		packetflags |= ENET_PACKET_FLAG_RELIABLE;

	ENetPacket* ppacket = enet_packet_create(pdata, msgsize, packetflags);

	if(dest == MSG_BROADCAST)
	{
		// Broadcast to all clients
		enet_host_broadcast(m_pHost, 0, ppacket);
	}
	else if(enet_peer_send(ptrpeer, 0, ppacket))
	{
		Con_Printf("Send failed on message.\n");
		return false;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
void CMPNetworking::ReadUDPMessage( net_msgcache_t* pcache, ENetEvent* pevent )
{
	byte* psrcdata = pevent->packet->data;
	Uint32 msgsize = (Uint32)pevent->packet->dataLength;

	// Create new message object
	m_pCurrentMessage = AllocMsg(pcache);
	memset(m_pCurrentMessage, 0, sizeof(net_msg_t));
	pcache->num_msg++;

	m_pCurrentMessage->clientidx = 0;
	m_pCurrentMessage->dest = MSG_ONE;
	m_pCurrentMessage->flags = MSG_FL_NONE;

	m_pCurrentMessage->msg_offset = pcache->bufferdatasize;
	m_pCurrentMessage->msg_size = 0;

	m_pCurrentMessage->pmsg_base = &pcache->pwritebuffer;
	m_pCurrentMessage->pmsg_bufsize = &pcache->buffersize;

	// Write to the message
	WriteBuffer(psrcdata, msgsize);

	m_pCurrentMessage = nullptr;

	pcache->bufferdatasize += msgsize;
}

//=============================================
// @brief
//
//=============================================
net_msgcache_t* CMPNetworking::CLS_GetWriteCache( void )
{
	if(CL_IsHostClient())
		return CNetworking::CLS_GetWriteCache_Local();
	else
		return m_pWriteCache;
}

//=============================================
// @brief
//
//=============================================
void CMPNetworking::SVC_MessageEnd( void )
{
	if(m_pCurrentMessage->dest == MSG_ONE || m_pCurrentMessage->dest == MSG_ONE_UNRELIABLE)
	{
		if(m_pCurrentMessage->clientidx == 0)
		{
			// Local messages aren't sent through enet
			CNetworking::SVC_MessageEnd_Local();
			return;
		}

		// Send the data through enet to the target
		net_client_t& cl = m_clientsArray[m_pCurrentMessage->clientidx];
		if(cl.cl_state != NETCL_CONNECTED)
		{
			m_pCurrentMessage = nullptr;
			return;
		}

		// Set the header data
		msgheader_t* phdr = (msgheader_t*)((*m_pCurrentMessage->pmsg_base) + m_pCurrentMessage->msg_offset);
		phdr->msgsize = m_pCurrentMessage->msg_size;

		byte* pdata = (*m_pCurrentMessage->pmsg_base) + m_pCurrentMessage->msg_offset;
		SendUDPMessage(m_pCurrentMessage->dest, pdata, m_pCurrentMessage->msg_size, cl.ptrpeer);

		m_pCurrentMessage = nullptr;
	}
	else
	{
		msgheader_t* phdr = (msgheader_t*)((*m_pCurrentMessage->pmsg_base) + m_pCurrentMessage->msg_offset);
		phdr->msgsize = m_pCurrentMessage->msg_size;

		byte* pdata = (*m_pCurrentMessage->pmsg_base) + m_pCurrentMessage->msg_offset;

		// Send to other players
		for(Uint32 i = 1; i < m_clientsArray.size(); i++)
		{
			net_client_t& cl = m_clientsArray[i];
			if(cl.cl_state != NETCL_CONNECTED)
				continue;

			// Set the header data
			SendUDPMessage(m_pCurrentMessage->dest, pdata, m_pCurrentMessage->msg_size, cl.ptrpeer);
		}

		// Send to local player
		CNetworking::SVC_MessageEnd_Local();
	}

	enet_host_flush(m_pHost);
}

//=============================================
// @brief
//
//=============================================
net_msgcache_t* CMPNetworking::SVC_GetWriteCache( Uint32 clientidx )
{
	if(clientidx == 0)
		return CNetworking::SVC_GetWriteCache_Local(clientidx);
	else
		return m_pWriteCache;
}

//=============================================
// @brief
//
//=============================================
void CMPNetworking::CLS_MessageEnd( void )
{
	if(CL_IsHostClient())
	{
		// Store it in the local msg cache
		CNetworking::CLS_MessageEnd_Local();
		return;
	}

	// Send the data through enet to the target
	net_client_t& cl = m_clientsArray[0];

	// Set the header data
	msgheader_t* phdr = (msgheader_t*)((*m_pCurrentMessage->pmsg_base) + m_pCurrentMessage->msg_offset);
	phdr->msgsize = m_pCurrentMessage->msg_size;

	byte* pdata = (*m_pCurrentMessage->pmsg_base) + m_pCurrentMessage->msg_offset;
	SendUDPMessage(m_pCurrentMessage->dest, pdata, m_pCurrentMessage->msg_size, cl.ptrpeer);

	m_pCurrentMessage = nullptr;
	enet_host_flush(m_pHost);
}
