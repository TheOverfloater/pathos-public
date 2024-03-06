/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef USERMSG_H
#define USERMSG_H

#include "networking.h"

// Usermsg buffer allocation size
static const Uint32 USERMSG_ALLOC_SIZE = 512;

struct usermsg_t
{
	usermsg_t():
		id(0),
		bufsize(-1)
		{}

	// Usermsg ID
	Uint32 id;
	// Name of usermsg
	CString name;
	// Message size
	Int32 bufsize;
};

struct usermsgdata_t
{
	usermsgdata_t():
		pmsgbuffer(nullptr),
		bufsize(USERMSG_ALLOC_SIZE),
		msgsize(0),
		pusermsg(nullptr),
		destination(MSG_UNDEFINED),
		pedict(nullptr),
		originset(false)
	{
		pmsgbuffer = new byte[bufsize];
	}
	usermsgdata_t( const usermsgdata_t& src ):
		pmsgbuffer(nullptr),
		bufsize(src.msgsize),
		msgsize(src.msgsize),
		pusermsg(src.pusermsg),
		destination(src.destination),
		pedict(src.pedict),
		pvsorigin(src.pvsorigin),
		originset(src.originset)
	{
		pmsgbuffer = new byte[src.msgsize];
		memcpy(pmsgbuffer, src.pmsgbuffer, sizeof(byte)*src.msgsize);
	}
	~usermsgdata_t()
	{
		if(pmsgbuffer)
			delete[] pmsgbuffer;
	}
	usermsgdata_t& operator=( const usermsgdata_t& src )
	{
		if(&src == this)
			return *this;

		pmsgbuffer = new byte[src.msgsize];
		memcpy(pmsgbuffer, src.pmsgbuffer, sizeof(byte)*src.msgsize);

		bufsize = src.msgsize;
		msgsize = src.msgsize;
		pusermsg = src.pusermsg;
		destination = src.destination;
		pedict = src.pedict;
		pvsorigin = src.pvsorigin;
		originset = src.originset;

		return *this;
	}

	// Message data buffer
	byte* pmsgbuffer;
	// Buffer size
	Uint32 bufsize;
	// Message size
	Int32 msgsize;

	// Usermsg being written
	usermsg_t* pusermsg;
	// Message destination
	msgdest_t destination;
	// Client to send to
	const edict_t* pedict;
	// origin to test PVS with
	Vector pvsorigin;
	// TRUE if origin was set
	bool originset;
};

extern void UserMSG_Msg_WriteEntindex( usermsgdata_t& msgdata, entindex_t entindex );
extern void UserMSG_Msg_WriteBuffer( usermsgdata_t& msgdata, const byte* pdata, Uint32 size );
extern void UserMSG_Msg_WriteString( usermsgdata_t& msgdata, const Char* pstrstring );
extern void UserMSG_Msg_WriteFloat( usermsgdata_t& msgdata, Float value );
extern void UserMSG_Msg_WriteSmallFloat( usermsgdata_t& msgdata, Float value );
extern void UserMSG_Msg_WriteDouble( usermsgdata_t& msgdata, Double value );
extern void UserMSG_Msg_WriteUint64( usermsgdata_t& msgdata, Uint64 value );
extern void UserMSG_Msg_WriteInt64( usermsgdata_t& msgdata, Int64 value );
extern void UserMSG_Msg_WriteUint32( usermsgdata_t& msgdata, Uint32 value );
extern void UserMSG_Msg_WriteInt32( usermsgdata_t& msgdata, Int32 value );
extern void UserMSG_Msg_WriteUint16( usermsgdata_t& msgdata, Uint16 value );
extern void UserMSG_Msg_WriteInt16( usermsgdata_t& msgdata, Int16 value );
extern void UserMSG_Msg_WriteChar( usermsgdata_t& msgdata, Char value );
extern void UserMSG_Msg_WriteByte( usermsgdata_t& msgdata, byte value );
extern void UserMSG_Msg_CheckBuffer( usermsgdata_t& msgdata, Uint32 size );
extern void UserMSG_UserMessageBegin( CArray<usermsg_t>& usermsgarray, usermsgdata_t& msgdata, msgdest_t dest, Int32 msgid, const Vector* porigin, const edict_t* pedict );
extern Int32 UserMSG_RegisterUserMessage( CArray<usermsg_t>& usermsgarray, const Char* pstrMsgName, Int32 msgsize );
#endif //USERMSG_H