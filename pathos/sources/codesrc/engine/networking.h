/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef NETWORKING_H
#define NETWORKING_H

#include <SDL.h>
#include <enet/enet.h>

#include "net_shared.h"

struct net_msgcache_t;

// Max size of a UDP packet, with the IPv4 header accounted for
static const Uint32 MAX_UDP_PACKET_SIZE = 548;
// Max timeout limit is 30 seconds
static const Float CLIENT_TIMEOUT_LIMIT = 30;

//
// Network client states
// 
enum netcl_state_t
{
	NETCL_STATE_NONE = 0,
	NETCL_CONNECTED,
	NETCL_DISCONNECTED,
	NETCL_NET_ERROR,
	NETCL_LOST_CONNECTION
};

//
// Messages types for server->client route
// 
enum svc_commands_t
{
	svc_bad,
	svc_nop,
	svc_disconnect,
	svc_sound,
	svc_print,
	svc_setvangles,
	svc_addavelocity,
	svc_serverinfo,
	svc_clientdata,
	svc_stopsound,
	svc_tempentity,
	svc_setpause,
	svc_usermsg,
	svc_registerusermsg,
	svc_packetentities,
	svc_resources,
	svc_movevars,
	svc_heartbeat,
	svc_sndengine,
	svc_consistency,
	svc_clcommand,
	svc_precacheparticlescript,
	svc_precachedecal
};

//
// Messages types for client->server route
// 
enum cls_commands_t
{
	cls_bad,
	cls_nop,
	cls_usercmd,
	cls_cvarvalue,
	cls_cmdstring,
	cls_disconnect,
	cls_clientinfo,
	cls_clientready,
	cls_heartbeat,
	cls_consistency,
	cls_resources,
	cls_registerusermsg,
	cls_usermsg,
	cls_pausegame
};

// header data for message sent to the destination
struct msgheader_t
{
	msgheader_t():
		msgsize(0),
		msgid(0),
		flags(0)
		{};

	Uint16 msgsize; // size of the message total
	Int32 msgid; // unique id, usually message number
	byte flags; // message flags
};

//
// Holds message data for writes
// 
struct net_msg_t
{
	net_msg_t():
		pmsg_base(nullptr),
		pmsg_bufsize(nullptr),
		msg_offset(0),
		msg_size(0),
		clientidx(0),
		dest(MSG_UNDEFINED),
		flags(MSG_FL_NONE)
		{}

	// message buffer pointer
	byte** pmsg_base;
	// message buffer size
	Uint32* pmsg_bufsize;

	// offset into the buffer
	Uint64 msg_offset;
	// message size
	Uint32 msg_size;

	// destination client index
	Uint32 clientidx;
	// destination type
	msgdest_t dest;
	// Flags for the message
	Int32 flags;
};

//
// A data cache that holds messages for a client
// 
struct net_msgcache_t
{
	net_msgcache_t( Uint32 arraysize, Uint32 buffsize ):
		num_msg(0),
		msg_index(0),
		pwritebuffer(nullptr),
		bufferdatasize(0),
		buffersize(buffsize)
		{
			msgarray.resize(arraysize);

			pwritebuffer = new byte[buffersize];
			memset(pwritebuffer, 0, sizeof(byte)*buffersize);
		}
	net_msgcache_t( const net_msgcache_t& src ):
		msgarray(src.msgarray),
		num_msg(src.num_msg),
		msg_index(src.msg_index),
		pwritebuffer(nullptr),
		bufferdatasize(src.bufferdatasize),
		buffersize(src.buffersize)
		{
			pwritebuffer = new byte[buffersize];
			memcpy(pwritebuffer, pwritebuffer, sizeof(byte)*buffersize);
		}
		~net_msgcache_t()
		{
			if(pwritebuffer)
				delete[] pwritebuffer;
		}
	net_msgcache_t& operator=( const net_msgcache_t& src )
		{
			if(&src == this)
				return *this;
			
			num_msg = src.num_msg;
			msg_index = src.msg_index;
			bufferdatasize = src.bufferdatasize;
			buffersize = src.buffersize;
			msgarray = src.msgarray;

			if(pwritebuffer)
				delete[] pwritebuffer;

			pwritebuffer = new byte[buffersize];
			memcpy(pwritebuffer, pwritebuffer, sizeof(byte)*buffersize);

			return *this;
		}

	// Message array
	CArray<net_msg_t> msgarray;
	// Number of messages cached
	Uint32 num_msg;
	// Current message's index
	Uint32 msg_index;

	// Write buffer
	byte* pwritebuffer;
	// Current data usage
	Uint32 bufferdatasize;
	// Buffer space available
	Uint32 buffersize;
};

//
// Definitions of a network client
// 
struct net_client_t
{
	net_client_t():
		index(0),
		ptrpeer(nullptr),
		psvc_cache(nullptr),
		pcls_cache(nullptr),
		nbmsgbrecieved(0),
		nbmsgsent(0),
		cl_state(NETCL_STATE_NONE),
		errorcode(0),
		timeoutbegintime(0),
		lastmsgtime(0)
		{
			ip_address.host = 0;
			ip_address.port = 0;
		}

	// client's index
	Uint32 index;

	// IP address of client
	ENetAddress ip_address;
	// Peer object
	ENetPeer* ptrpeer;

	// svc message array
	net_msgcache_t *psvc_cache;
	// cls message array
	net_msgcache_t *pcls_cache;

	// amount of messages recieved this frame
	Uint32 nbmsgbrecieved;
	// amount of messages sent this frame
	Uint32 nbmsgsent;

	// state of the client
	netcl_state_t cl_state;
	// Info string if client was lost
	CString infostring;
	// Failure code returned by send/recieve
	Int32 errorcode;

	// Last time we got anything from this client
	Double timeoutbegintime;
	// Last time we received any messages
	Double lastmsgtime;
};

/*
=======================
CNetworking

=======================
*/
class CNetworking
{
public:
	// Client to server message buffer allocation size
	static const Uint32 MSG_BUFFER_ALLOC_SIZE;
	// Client to server message buffer allocation size
	static const Uint32 MSG_DATA_BUFFER_ALLOC_SIZE;

public:
	CNetworking( void );
	virtual ~CNetworking( void );

public:
	static CNetworking* CreateInstance( bool ismultiplayer );
	static CNetworking* GetInstance( void );
	static void DeleteInstance( void );

public:
	// Initializes networking functions
	virtual bool Init( const Char* pstrhost );
	// Clears networking info
	virtual void ClearCache( net_msgcache_t *pcache );

	// Tries to reconnect to the host server
	virtual bool AttemptConnection( void ) = 0;

	// Begin writing a message
	void SVC_MessageBegin( msgdest_t dest, Uint32 type, const struct edict_t* pedict, Int32 flags = MSG_FL_NONE );
	// Close off and cache a message
	virtual void SVC_MessageEnd( void ) = 0;
	// Retrieve an SVC message
	bool SVC_GetMessage( byte*& pmsgdata, Uint32& msgsize );
	// Clear out SVC message arrays
	void SVC_ClearMessages( void );
	// Retrieves a cache pointer
	virtual net_msgcache_t* SVC_GetWriteCache( Uint32 clientidx ) = 0;

	// Begin writing a message
	void CLS_MessageBegin( Uint32 type, Int32 flags = MSG_FL_NONE );
	// Close off and cache a message
	virtual void CLS_MessageEnd( void ) = 0;
	// Retrieve a CLS message
	bool CLS_GetMessage( Uint32 cl_index, byte*& pmsgdata, Uint32& msgsize );
	// Clear out CLS message arrays
	void CLS_ClearMessages( Uint32 cl_index );
	// Retrieves a cache pointer
	virtual net_msgcache_t* CLS_GetWriteCache( void ) = 0;

	// Disconnects from the host server
	virtual void Disconnect( Uint32 clindex ) = 0;
	// Retrieve any packets
	virtual void Poll( void );

	// Gets the last time client received any messages
	Double GetLastMessageTime( Uint32 cl_index );

protected:
	// Close off and cache a message
	bool SVC_MessageEnd_Local( void );
	// Retrieves a cache pointer
	net_msgcache_t* SVC_GetWriteCache_Local( Uint32 clientidx );

	// Close off and cache a message
	bool CLS_MessageEnd_Local( void );
	// Retrieves a cache pointer
	net_msgcache_t* CLS_GetWriteCache_Local( void );

	// Allocates a client's data
	void InitClient( Uint32 index, net_client_t& cl, bool allocsvc );
	// Resets a client's states
	void ResetClient( net_client_t& cl );
	// Allocate a message
	net_msg_t* AllocMsg( net_msgcache_t* pcache );

public:
	// Tells if a client can recieve/read messages
	netcl_state_t GetClientState( Uint32 cl_index );
	// Retrieves an IP address
	CString GetIPAddress( Uint32 cl_index );
	// Retrieves the disconnect reason string
	const Char* GetInfoString( Uint32 cl_index );

public:
	// Writes a single byte
	void WriteByte( byte value );
	// Writes a single char
	void WriteChar( Char value );
	// Writes a signed short
	void WriteInt16( Int16 value );
	// Writes an unsigned short
	void WriteUint16( Uint16 value );
	// Writes a 32-bit integer
	void WriteInt32( Int32 value );
	// Writes an unsigned 32-bit integer
	void WriteUint32( Uint32 value );
	// Writes a 64-bit integer
	void WriteInt64( Int64 value );
	// Writes a 64-bit unsigned integer
	void WriteUint64( Uint64 value );
	// Writes a 2-byte float
	void WriteSmallFloat( Float value );
	// Writes a float
	void WriteFloat( Float value );
	// Writes a float
	void WriteDouble( Float value );
	// Writes a buffer of bytes
	void WriteBuffer( const byte* pdata, Uint32 size );
	// Writes a buffer of bytes
	void WriteString( const Char* pstring );

protected:
	// Checks if the buffer used by the message needs to be resized
	static void CheckBuffer( net_msg_t& msg, Uint32 size ); 

protected:
	// Local client
	net_client_t* m_pLocalClient;
	// Current message being written
	net_msg_t* m_pCurrentMessage;

	// For the message being read
	const byte* m_pReadBuffer;
	// Read buffer size
	Uint32 m_readBufferSize;
	// Current read position
	Uint32 m_readOffset;

	// Array of clients
	CArray<net_client_t> m_clientsArray;
	// Number of messages sent total
	Uint32 m_nbSVCMessagesSent;
	// Number of messages sent total
	Uint32 m_nbSVCMessagesRecieved;
	// Number of messages sent total
	Uint32 m_nbCLSMessagesSent;
	// Number of messages sent total
	Uint32 m_nbCLSMessagesRecieved;

	// Error string
	CString m_errorString;

private:
	// Networking class instance
	static CNetworking* g_pNetworking;
};
#endif //NETWORKING_H