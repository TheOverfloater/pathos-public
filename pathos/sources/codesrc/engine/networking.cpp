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
#include "net_mp.h"
#include "enginestate.h"
#include "sv_main.h"
#include "system.h"
#include "cl_main.h"
#include "console.h"
#include "cvar.h"

// Client to server message buffer allocation size
const Uint32 CNetworking::MSG_BUFFER_ALLOC_SIZE = 256;
// Message data buffer allocation size
const Uint32 CNetworking::MSG_DATA_BUFFER_ALLOC_SIZE = 2048;

// Pointer to networking class
CNetworking* CNetworking::g_pNetworking = nullptr;

//=============================================
// @brief
//
//=============================================
CNetworking* CNetworking::CreateInstance( bool ismultiplayer )
{
	if(!g_pNetworking)
	{
		if(!ismultiplayer)
			g_pNetworking = new CSPNetworking();
		else
			g_pNetworking = new CMPNetworking();
	}

	return g_pNetworking;
}

//=============================================
// @brief
//
//=============================================
CNetworking* CNetworking::GetInstance( void )
{
	return g_pNetworking;
}

//=============================================
// @brief
//
//=============================================
void CNetworking::DeleteInstance( void )
{
	if(g_pNetworking)
	{
		delete g_pNetworking;
		g_pNetworking = nullptr;
	}
}

//=============================================
// @brief Default constructor
//
//=============================================
CNetworking::CNetworking( void ):
	m_pLocalClient(nullptr),
	m_pCurrentMessage(nullptr),
	m_pReadBuffer(nullptr),
	m_readBufferSize(0),
	m_readOffset(0),
	m_nbSVCMessagesSent(0),
	m_nbSVCMessagesRecieved(0),
	m_nbCLSMessagesSent(0),
	m_nbCLSMessagesRecieved(0)
{
}

//=============================================
// @brief Destructor
//
//=============================================
CNetworking::~CNetworking( void )
{
	if(m_clientsArray.empty())
		return;

	for(Uint32 i = 0; i < m_clientsArray.size(); i++)
	{
		if(m_clientsArray[i].pcls_cache)
			delete m_clientsArray[i].pcls_cache;

		if(m_clientsArray[i].psvc_cache)
			delete m_clientsArray[i].psvc_cache;
	}

	m_clientsArray.clear();

	// Clear this
	m_nbSVCMessagesSent = 0;
	m_nbCLSMessagesSent = 0;
}

//=============================================
// @brief
//
//=============================================
bool CNetworking::Init( const Char* pstrhost )
{
	// Check if we're initializing the localhost
	if(qstrcmp(pstrhost, "localhost"))
	{
		// Not localhost, so only alloc one
		m_clientsArray.resize(1);
		InitClient(0, m_clientsArray[0], true);
	}
	else
	{
		// Allocate all clients for server
		m_clientsArray.resize(svs.maxclients);
		for(Uint32 i = 0; i < svs.maxclients; i++)
			InitClient(i, m_clientsArray[i], (i == 0) ? true : false);
	}

	// Set local client
	m_pLocalClient = &m_clientsArray[0];
	m_pLocalClient->cl_state = NETCL_CONNECTED;
	return true;
}

//=============================================
// @brief
//
//=============================================
net_msg_t* CNetworking::AllocMsg( net_msgcache_t* pcache )
{
	// See if the buffer's about to overflow, and resize if needed
	if(pcache->num_msg == pcache->msgarray.size())
		pcache->msgarray.resize(pcache->msgarray.size()+MSG_BUFFER_ALLOC_SIZE);

	return &pcache->msgarray[pcache->num_msg];
}

//=============================================
// @brief
//
//=============================================
void CNetworking::SVC_MessageBegin( msgdest_t dest, Uint32 type, const edict_t* pedict, Int32 flags )
{
	Uint32 cl_index = 0;

	// Only seek cl index if it's not MSG_ALL
	if(dest != MSG_ALL)
	{
		if(!pedict)
		{
			Con_EPrintf("%s called with type %d and nullptr edict.\n", __FUNCTION__, dest);
			return;
		}

		// Make sure it's a valid client, if we have an edict
		if(pedict->entindex < 1 || pedict->entindex > (Int32)svs.maxclients)
		{
			Con_EPrintf("%s: Not a client.\n", __FUNCTION__);
			return;
		}

		cl_index = pedict->entindex-1;
	}

	// Warn about any screwups with msg calls
	if(m_pCurrentMessage != nullptr)
	{
		Con_EPrintf("%s called with an unfinished message.\n", __FUNCTION__);

		// Reset the pointer and the data
		byte *pdata = (*m_pCurrentMessage->pmsg_base) + m_pCurrentMessage->msg_offset;
		memset(pdata, 0, sizeof(byte)*m_pCurrentMessage->msg_size);
		m_pCurrentMessage = nullptr;
	}

	// For MSG_ALL, we'll be using index 0
	net_msgcache_t* pcache = SVC_GetWriteCache(cl_index);

	// Grab last message and clear the struct
	m_pCurrentMessage = AllocMsg(pcache);
	memset(m_pCurrentMessage, 0, sizeof(net_msg_t));

	// Set the basic data
	m_pCurrentMessage->pmsg_bufsize = &pcache->buffersize;
	m_pCurrentMessage->pmsg_base = &pcache->pwritebuffer;
	m_pCurrentMessage->msg_offset = pcache->bufferdatasize;

	m_pCurrentMessage->clientidx = cl_index;
	m_pCurrentMessage->flags = flags;
	m_pCurrentMessage->dest = dest;

	m_nbSVCMessagesSent++;

	// Reserve data for header and set the header data
	CheckBuffer((*m_pCurrentMessage), sizeof(msgheader_t));
	m_pCurrentMessage->msg_size = sizeof(msgheader_t);

	// Set basics
	msgheader_t* phdr = (msgheader_t*)((*m_pCurrentMessage->pmsg_base) + m_pCurrentMessage->msg_offset);
	phdr->msgid = m_nbSVCMessagesSent;
	phdr->msgsize = 0;
	phdr->flags = flags;

	// Write the message type identifier
	WriteByte(type);
}

//=============================================
// @brief
//
//=============================================
bool CNetworking::SVC_MessageEnd_Local( void )
{
	if(!m_pCurrentMessage)
		return false;

	net_client_t& cl = m_clientsArray[m_pCurrentMessage->clientidx];
	net_msgcache_t* pcache = cl.psvc_cache;

	// Advance pointer's offset
	pcache->bufferdatasize += m_pCurrentMessage->msg_size;
	pcache->num_msg++;

	// Set the header data
	msgheader_t* phdr = (msgheader_t*)((*m_pCurrentMessage->pmsg_base) + m_pCurrentMessage->msg_offset);
	phdr->msgsize = m_pCurrentMessage->msg_size;

	m_nbSVCMessagesRecieved++;

	// Clear the message
	m_pCurrentMessage = nullptr;
	return true;
}

//=============================================
// @brief
//
//=============================================
net_msgcache_t* CNetworking::SVC_GetWriteCache_Local( Uint32 clientidx )
{
	const net_client_t& cl = m_clientsArray[clientidx];
	net_msgcache_t* pcache = cl.psvc_cache;

	return pcache;
}

//=============================================
// @brief
//
//=============================================
bool CNetworking::SVC_GetMessage( byte*& pmsgdata, Uint32& msgsize )
{
	// SVC will always arrive on client 0 as local client is on index 0
	assert(m_pLocalClient != nullptr);
	net_msgcache_t* pcache = m_pLocalClient->psvc_cache;
	if(!pcache->num_msg)
		return false;

	if(pcache->msg_index == pcache->num_msg)
		return false;

	net_msg_t* pmsg = &pcache->msgarray[pcache->msg_index];
	pcache->msg_index++;

	pmsgdata = (*pmsg->pmsg_base) + pmsg->msg_offset + sizeof(msgheader_t);
	msgsize = pmsg->msg_size;
	return true;
}

//=============================================
// @brief
//
//=============================================
void CNetworking::SVC_ClearMessages( void )
{
	assert(m_pLocalClient != nullptr);
	net_msgcache_t* pcache = m_pLocalClient->psvc_cache;
	if(!pcache->num_msg)
		return;

	// Clear the array
	memset(&pcache->msgarray[0], 0, sizeof(net_msg_t)*pcache->num_msg);

	// Reset these
	pcache->num_msg = 0;
	pcache->bufferdatasize = 0;
	pcache->msg_index = 0;
}

//=============================================
// @brief
//
//=============================================
void CNetworking::CLS_MessageBegin( Uint32 type, Int32 flags )
{
	if(m_pCurrentMessage != nullptr)
	{
		Con_EPrintf("%s called with an unfinished message.\n", __FUNCTION__);

		// Reset the pointer and the data
		byte *pdata = (*m_pCurrentMessage->pmsg_base) + m_pCurrentMessage->msg_offset;
		memset(pdata, 0, sizeof(byte)*m_pCurrentMessage->msg_size);
		m_pCurrentMessage = nullptr;
	}

	net_msgcache_t* pcache = CLS_GetWriteCache();

	// Grab last message and clear the struct
	m_pCurrentMessage = AllocMsg(pcache);
	memset(m_pCurrentMessage, 0, sizeof(net_msg_t));

	m_pCurrentMessage->pmsg_bufsize = &pcache->buffersize;
	m_pCurrentMessage->pmsg_base = &pcache->pwritebuffer;
	m_pCurrentMessage->msg_offset = pcache->bufferdatasize;

	m_pCurrentMessage->clientidx = 0;
	m_pCurrentMessage->dest = MSG_ONE;
	m_pCurrentMessage->flags = flags;

	m_nbCLSMessagesSent++;

	// Reserve 2 bytes for the message size
	CheckBuffer((*m_pCurrentMessage), sizeof(msgheader_t));
	m_pCurrentMessage->msg_size += sizeof(msgheader_t);

	// Set the header info
	msgheader_t* phdr = (msgheader_t*)((*m_pCurrentMessage->pmsg_base) + m_pCurrentMessage->msg_offset);
	phdr->msgid = m_nbCLSMessagesSent;
	phdr->msgsize = 0;
	phdr->flags = flags;

	// Write the type
	WriteByte(type);
}

//=============================================
// @brief
//
//=============================================
net_msgcache_t* CNetworking::CLS_GetWriteCache_Local( void )
{
	return m_pLocalClient->pcls_cache;
}

//=============================================
// @brief
//
//=============================================
bool CNetworking::CLS_MessageEnd_Local( void )
{
	if(!m_pCurrentMessage)
		return false;

	// Advance pointer's offset
	net_msgcache_t* pcache = m_pLocalClient->pcls_cache;
	pcache->bufferdatasize += m_pCurrentMessage->msg_size;

	// Set the header data
	msgheader_t* phdr = (msgheader_t*)((*m_pCurrentMessage->pmsg_base) + m_pCurrentMessage->msg_offset);
	phdr->msgsize = m_pCurrentMessage->msg_size;

	m_nbCLSMessagesRecieved++;

	// Clear the message
	m_pCurrentMessage = nullptr;
	pcache->num_msg++;
	return true;
}

//=============================================
// @brief
//
//=============================================
bool CNetworking::CLS_GetMessage( Uint32 cl_index, byte*& pmsgdata, Uint32& msgsize )
{
	assert(cl_index < m_clientsArray.size());
	net_client_t& cl = m_clientsArray[cl_index];

	if(cl.pcls_cache->msg_index == cl.pcls_cache->num_msg)
		return false;

	net_msg_t* pmsg = &cl.pcls_cache->msgarray[cl.pcls_cache->msg_index];
	cl.pcls_cache->msg_index++;

	pmsgdata = (*pmsg->pmsg_base) + pmsg->msg_offset + sizeof(msgheader_t);
	msgsize = pmsg->msg_size;
	return true;
}

//=============================================
// @brief
//
//=============================================
void CNetworking::CLS_ClearMessages( Uint32 cl_index )
{
	assert(cl_index < m_clientsArray.size());
	net_client_t& cl = m_clientsArray[cl_index];
	if(!cl.pcls_cache->num_msg)
		return;

	// Clear the array
	memset(&cl.pcls_cache->msgarray[0], 0, sizeof(net_msg_t)*cl.pcls_cache->num_msg);

	// Reset these
	cl.pcls_cache->num_msg = 0;
	cl.pcls_cache->msg_index = 0;
	cl.pcls_cache->bufferdatasize = 0;
}

//=============================================
// @brief
//
//=============================================
void CNetworking::ClearCache( net_msgcache_t *pcache )
{
	pcache->bufferdatasize = 0;
	pcache->num_msg = 0;
	pcache->msg_index = 0;
}

//=============================================
// @brief
//
//=============================================
void CNetworking::ResetClient( net_client_t& cl )
{
	cl.errorcode = 0;
	cl.nbmsgbrecieved = 0;
	cl.nbmsgsent = 0;
	cl.cl_state = NETCL_STATE_NONE;
	cl.ip_address.host = 0;
	cl.ip_address.port = 0;
	cl.nbmsgbrecieved = 0;
	cl.nbmsgsent = 0;

	cl.infostring.clear();

	if(cl.pcls_cache)
		ClearCache(cl.pcls_cache);

	if(cl.psvc_cache)
		ClearCache(cl.psvc_cache);
}

//=============================================
// @brief
//
//=============================================
void CNetworking::WriteByte( byte value )
{
	if(!m_pCurrentMessage)
		return;

	CheckBuffer((*m_pCurrentMessage), sizeof(byte));

	byte* pdata = (*m_pCurrentMessage->pmsg_base) 
		+ m_pCurrentMessage->msg_offset
		+ m_pCurrentMessage->msg_size;

	*pdata = value;
	m_pCurrentMessage->msg_size += sizeof(byte);
}

//=============================================
// @brief
//
//=============================================
void CNetworking::WriteChar( Char value )
{
	if(!m_pCurrentMessage)
		return;

	CheckBuffer((*m_pCurrentMessage), sizeof(value));

	Char* pdata = (Char*)(*m_pCurrentMessage->pmsg_base) 
		+ m_pCurrentMessage->msg_offset
		+ m_pCurrentMessage->msg_size;

	*pdata = value;
	m_pCurrentMessage->msg_size += sizeof(Char);
}

//=============================================
// @brief
//
//=============================================
void CNetworking::WriteInt16( Int16 value )
{
	if(!m_pCurrentMessage)
		return;

	CheckBuffer((*m_pCurrentMessage), sizeof(Int16));

	byte* pdata = (*m_pCurrentMessage->pmsg_base) 
		+ m_pCurrentMessage->msg_offset
		+ m_pCurrentMessage->msg_size;

	memcpy(pdata, &value, sizeof(Int16));
	m_pCurrentMessage->msg_size += sizeof(Int16);
}

//=============================================
// @brief
//
//=============================================
void CNetworking::WriteUint16( Uint16 value )
{
	if(!m_pCurrentMessage)
		return;

	CheckBuffer((*m_pCurrentMessage), sizeof(Uint16));

	byte* pdata = (*m_pCurrentMessage->pmsg_base) 
		+ m_pCurrentMessage->msg_offset
		+ m_pCurrentMessage->msg_size;

	memcpy(pdata, &value, sizeof(Uint16));
	m_pCurrentMessage->msg_size += sizeof(Uint16);
}

//=============================================
// @brief
//
//=============================================
void CNetworking::WriteInt32( Int32 value )
{
	if(!m_pCurrentMessage)
		return;

	CheckBuffer((*m_pCurrentMessage), sizeof(Int32));

	byte* pdata = (*m_pCurrentMessage->pmsg_base) 
		+ m_pCurrentMessage->msg_offset
		+ m_pCurrentMessage->msg_size;

	memcpy(pdata, &value, sizeof(Int32));
	m_pCurrentMessage->msg_size += sizeof(Int32);
}

//=============================================
// @brief
//
//=============================================
void CNetworking::WriteUint32( Uint32 value )
{
	if(!m_pCurrentMessage)
		return;

	CheckBuffer((*m_pCurrentMessage), sizeof(Uint32));

	byte* pdata = (*m_pCurrentMessage->pmsg_base) 
		+ m_pCurrentMessage->msg_offset
		+ m_pCurrentMessage->msg_size;

	memcpy(pdata, &value, sizeof(Uint32));
	m_pCurrentMessage->msg_size += sizeof(Uint32);
}

//=============================================
// @brief
//
//=============================================
void CNetworking::WriteInt64( Int64 value )
{
	if(!m_pCurrentMessage)
		return;

	CheckBuffer((*m_pCurrentMessage), sizeof(Int64));

	byte* pdata = (*m_pCurrentMessage->pmsg_base) 
		+ m_pCurrentMessage->msg_offset
		+ m_pCurrentMessage->msg_size;

	memcpy(pdata, &value, sizeof(Int64));
	m_pCurrentMessage->msg_size += sizeof(Int64);
}

//=============================================
// @brief
//
//=============================================
void CNetworking::WriteUint64( Uint64 value )
{
	if(!m_pCurrentMessage)
		return;
	
	CheckBuffer((*m_pCurrentMessage), sizeof(Uint64));

	byte* pdata = (*m_pCurrentMessage->pmsg_base) 
		+ m_pCurrentMessage->msg_offset
		+ m_pCurrentMessage->msg_size;

	memcpy(pdata, &value, sizeof(Uint64));
	m_pCurrentMessage->msg_size += sizeof(Uint64);
}

//=============================================
// @brief
//
//=============================================
void CNetworking::WriteSmallFloat( Float value )
{
	if(!m_pCurrentMessage)
		return;

	Int16 intvalue = (Int16)SDL_floor(value * 8);
	WriteInt16(intvalue);
}

//=============================================
// @brief
//
//=============================================
void CNetworking::WriteFloat( Float value )
{
	if(!m_pCurrentMessage)
		return;

	CheckBuffer((*m_pCurrentMessage), sizeof(Float));

	byte* pdata = (*m_pCurrentMessage->pmsg_base) 
		+ m_pCurrentMessage->msg_offset
		+ m_pCurrentMessage->msg_size;

	memcpy(pdata, &value, sizeof(Float));

	m_pCurrentMessage->msg_size += sizeof(Float);
}

//=============================================
// @brief
//
//=============================================
void CNetworking::WriteDouble( Float value )
{
	if(!m_pCurrentMessage)
		return;

	CheckBuffer((*m_pCurrentMessage), sizeof(Double));

	byte* pdata = (*m_pCurrentMessage->pmsg_base) 
		+ m_pCurrentMessage->msg_offset
		+ m_pCurrentMessage->msg_size;

	memcpy(pdata, &value, sizeof(Double));

	m_pCurrentMessage->msg_size += sizeof(Double);
}

//=============================================
// @brief
//
//=============================================
void CNetworking::WriteBuffer( const byte* pdata, Uint32 size )
{
	if(!m_pCurrentMessage)
		return;

	CheckBuffer((*m_pCurrentMessage), size);

	byte* pdest = (*m_pCurrentMessage->pmsg_base) 
		+ m_pCurrentMessage->msg_offset
		+ m_pCurrentMessage->msg_size;

	for(Uint32 i = 0; i < size; i++)
		pdest[i] = pdata[i];
		
	m_pCurrentMessage->msg_size += size;
}

//=============================================
// @brief
//
//=============================================
void CNetworking::WriteString( const Char* pstring )
{
	if(!m_pCurrentMessage)
		return;

	if(!pstring)
	{
		CheckBuffer((*m_pCurrentMessage), 1);
		byte* pdest = (*m_pCurrentMessage->pmsg_base) 
			+ m_pCurrentMessage->msg_offset
			+ m_pCurrentMessage->msg_size;

		*pdest = '\0';
		m_pCurrentMessage->msg_size++;
		return;
	}

	CString str(pstring);
	Uint32 stringlength = qstrlen(pstring)+1;
	WriteUint16(stringlength);

	CheckBuffer((*m_pCurrentMessage), stringlength);

	byte* pdest = (*m_pCurrentMessage->pmsg_base) 
		+ m_pCurrentMessage->msg_offset
		+ m_pCurrentMessage->msg_size;

	for(Uint32 i = 0; i < stringlength; i++)
		pdest[i] = (byte)pstring[i];
		
	m_pCurrentMessage->msg_size += stringlength;
}

//=============================================
// @brief
//
//=============================================
void CNetworking::CheckBuffer( net_msg_t& msg, Uint32 size )
{
	Uint32 finalSize = msg.msg_offset + msg.msg_size + size;
	if(finalSize < (*msg.pmsg_bufsize))
		return;

	Int32 multiplier = 1;
	Uint32 memNeeded = finalSize - ((*msg.pmsg_bufsize) - msg.msg_size);
	if(memNeeded > MSG_DATA_BUFFER_ALLOC_SIZE)
	{
		Float nbTimes = (Float)((Float)memNeeded/(Float)MSG_DATA_BUFFER_ALLOC_SIZE);
		multiplier = (Int32)ceil(nbTimes);
	}

	// Resize the message data buffer
	void* pmsgbuffer = Common::ResizeArray((*msg.pmsg_base), sizeof(byte), (*msg.pmsg_bufsize), MSG_DATA_BUFFER_ALLOC_SIZE*multiplier);
	(*msg.pmsg_base) = reinterpret_cast<byte*>(pmsgbuffer);
	(*msg.pmsg_bufsize) = (*msg.pmsg_bufsize) + MSG_DATA_BUFFER_ALLOC_SIZE*multiplier;
}

//=============================================
// @brief
//
//=============================================
void CNetworking::InitClient( Uint32 index, net_client_t& cl, bool allocsvc )
{
	if(allocsvc)
		cl.psvc_cache = new net_msgcache_t(MSG_BUFFER_ALLOC_SIZE, MSG_DATA_BUFFER_ALLOC_SIZE);

	cl.pcls_cache = new net_msgcache_t(MSG_BUFFER_ALLOC_SIZE, MSG_DATA_BUFFER_ALLOC_SIZE);

	// Reset client info
	ResetClient(cl);
	// Make sure this is set
	cl.index = index;
}

//=============================================
// @brief
//
//=============================================
CString CNetworking::GetIPAddress( Uint32 cl_index )
{
	assert(cl_index < m_clientsArray.size());
	const net_client_t& cl = m_clientsArray[cl_index];

	if(cl.cl_state != NETCL_CONNECTED)
		return CString();

	// Localhost only for now
	CString sIp("127.0.0.1");
	return sIp;
}

//=============================================
// @brief
//
//=============================================
const Char* CNetworking::GetInfoString( Uint32 cl_index )
{
	assert(cl_index < m_clientsArray.size());
	net_client_t& cl = m_clientsArray[cl_index];

	return cl.infostring.c_str();
}

//=============================================
// @brief
//
//=============================================
netcl_state_t CNetworking::GetClientState( Uint32 cl_index )
{
	assert(cl_index < m_clientsArray.size());
	const net_client_t& cl = m_clientsArray[cl_index];

	return cl.cl_state;
}

//=============================================
// @brief
//
//=============================================
void CNetworking::Poll( void )
{
	if(g_psv_netdebug->GetValue() < 1)
		return;

	// Write information
	Con_VPrintf("SVC Messages sent: %d, recieved: %d.\n", m_nbCLSMessagesSent, m_nbCLSMessagesRecieved);
	m_nbCLSMessagesSent = 0;
	m_nbCLSMessagesRecieved = 0;

	Con_VPrintf("CLS Messages sent: %d, recieved: %d.\n", m_nbSVCMessagesSent, m_nbSVCMessagesRecieved);
	m_nbSVCMessagesSent = 0;
	m_nbCLSMessagesRecieved = 0;
}

//=============================================
// @brief
//
//=============================================
Double CNetworking::GetLastMessageTime( Uint32 cl_index )
{
	assert(cl_index < m_clientsArray.size());
	const net_client_t& cl = m_clientsArray[cl_index];

	return cl.lastmsgtime;
}