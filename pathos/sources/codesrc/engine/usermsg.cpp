/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.

===============================================
*/

#include "includes.h"
#include "usermsg.h"
#include "enginestate.h"
#include "system.h"
#include "networking.h"

//=============================================
//
//=============================================
Int32 UserMSG_RegisterUserMessage( CArray<usermsg_t>& usermsgarray, const Char* pstrMsgName, Int32 msgsize )
{
	// Make sure it's not called on an active game
	if(ens.gamestate == GAME_RUNNING)
	{
		Con_Printf("%s - This function can only be called during game library load.\n", __FUNCTION__);
		return 0;
	}

	// See if it's already registered
	for(Uint32 i = 0; i < usermsgarray.size(); i++)
	{
		if(!qstrcmp(usermsgarray[i].name, pstrMsgName))
		{
			Con_Printf("%s - User message %s already registered.\n", __FUNCTION__, pstrMsgName);
			return i+1;
		}
	}

	// Index we're adding in at
	Int32 addindex = usermsgarray.size();

	// Create the new user message
	usermsg_t newmsg;
	newmsg.name = pstrMsgName;
	newmsg.bufsize = msgsize;
	newmsg.id = addindex+1;

	// Add it to the list and give back the id
	usermsgarray.push_back(newmsg);
	return addindex+1;
}

//=============================================
//
//=============================================
void UserMSG_UserMessageBegin( CArray<usermsg_t>& usermsgarray, usermsgdata_t& msgdata, msgdest_t dest, Int32 msgid, const Vector* porigin, const edict_t* pedict )
{
	// Make sure the message is valid
	if(msgid <= 0 || msgid > (Int32)usermsgarray.size())
	{
		Con_EPrintf("%s - Bogus message id %d.\n", __FUNCTION__, msgid);
		return;
	}

	// Get message index
	Int32 msgidx = msgid - 1;
	usermsg_t& msg = usermsgarray[msgidx];

	msgdata.pusermsg = &msg;
	msgdata.pedict = pedict;
	msgdata.destination = dest;
	msgdata.msgsize = 0;

	if(porigin)
	{
		msgdata.pvsorigin = (*porigin);
		msgdata.originset = true;
	}
	else
	{
		// Origin was not set
		msgdata.originset = false;
	}
}

//=============================================
//
//=============================================
void UserMSG_Msg_CheckBuffer( usermsgdata_t& msgdata, Uint32 size )
{
	Uint32 finalSize = msgdata.msgsize + size;
	if(finalSize < msgdata.bufsize)
		return;

	// Determine alloc size needed
	Int32 multiplier = 1;
	Uint32 memNeeded = finalSize - (msgdata.bufsize - msgdata.msgsize);
	if(memNeeded > USERMSG_ALLOC_SIZE)
	{
		Float nbTimes = (Float)((Float)memNeeded/(Float)USERMSG_ALLOC_SIZE);
		multiplier = (Int32)ceil(nbTimes);
	}

	// Resize the message data buffer
	void* pnewbuffer = Common::ResizeArray(msgdata.pmsgbuffer, sizeof(byte), msgdata.bufsize, USERMSG_ALLOC_SIZE*multiplier);
	msgdata.pmsgbuffer = reinterpret_cast<byte*>(pnewbuffer);
	msgdata.bufsize = msgdata.bufsize + USERMSG_ALLOC_SIZE*multiplier;
}

//=============================================
//
//=============================================
void UserMSG_Msg_WriteByte( usermsgdata_t& msgdata, byte value )
{
	if(msgdata.pusermsg == nullptr)
	{
		Con_EPrintf("%s - No message begin called.\n", __FUNCTION__);
		return;
	}

	UserMSG_Msg_CheckBuffer(msgdata, sizeof(byte));

	byte* pdata = msgdata.pmsgbuffer 
		+ msgdata.msgsize;

	*pdata = value;
	msgdata.msgsize += sizeof(byte);
}

//=============================================
//
//=============================================
void UserMSG_Msg_WriteChar( usermsgdata_t& msgdata, Char value )
{
	if(msgdata.pusermsg == nullptr)
	{
		Con_EPrintf("%s - No message begin called.\n", __FUNCTION__);
		return;
	}

	UserMSG_Msg_CheckBuffer(msgdata, sizeof(value));

	Char* pdata = reinterpret_cast<Char*>(msgdata.pmsgbuffer)
		+ msgdata.msgsize;

	*pdata = value;
	msgdata.msgsize += sizeof(byte);
}

//=============================================
//
//=============================================
void UserMSG_Msg_WriteInt16( usermsgdata_t& msgdata, Int16 value )
{
	if(msgdata.pusermsg == nullptr)
	{
		Con_EPrintf("%s - No message begin called.\n", __FUNCTION__);
		return;
	}

	UserMSG_Msg_CheckBuffer(msgdata, sizeof(Int16));

	byte* pdata = msgdata.pmsgbuffer 
		+ msgdata.msgsize;

	memcpy(pdata, &value, sizeof(Int16));
	msgdata.msgsize += sizeof(Int16);
}

//=============================================
//
//=============================================
void UserMSG_Msg_WriteUint16( usermsgdata_t& msgdata, Uint16 value )
{
	if(msgdata.pusermsg == nullptr)
	{
		Con_EPrintf("%s - No message begin called.\n", __FUNCTION__);
		return;
	}

	UserMSG_Msg_CheckBuffer(msgdata, sizeof(Uint16));

	byte* pdata = msgdata.pmsgbuffer 
		+ msgdata.msgsize;

	memcpy(pdata, &value, sizeof(Uint16));
	msgdata.msgsize += sizeof(Uint16);
}

//=============================================
//
//=============================================
void UserMSG_Msg_WriteInt32( usermsgdata_t& msgdata, Int32 value )
{
	if(msgdata.pusermsg == nullptr)
	{
		Con_EPrintf("%s - No message begin called.\n", __FUNCTION__);
		return;
	}

	UserMSG_Msg_CheckBuffer(msgdata, sizeof(Int32));

	byte* pdata = msgdata.pmsgbuffer 
		+ msgdata.msgsize;

	memcpy(pdata, &value, sizeof(Int32));
	msgdata.msgsize += sizeof(Int32);
}

//=============================================
//
//=============================================
void UserMSG_Msg_WriteUint32( usermsgdata_t& msgdata, Uint32 value )
{
	if(msgdata.pusermsg == nullptr)
	{
		Con_EPrintf("%s - No message begin called.\n", __FUNCTION__);
		return;
	}

	UserMSG_Msg_CheckBuffer(msgdata, sizeof(Uint32));

	byte* pdata = msgdata.pmsgbuffer 
		+ msgdata.msgsize;

	memcpy(pdata, &value, sizeof(Uint32));
	msgdata.msgsize += sizeof(Uint32);
}

//=============================================
//
//=============================================
void UserMSG_Msg_WriteInt64( usermsgdata_t& msgdata, Int64 value )
{
	if(msgdata.pusermsg == nullptr)
	{
		Con_EPrintf("%s - No message begin called.\n", __FUNCTION__);
		return;
	}

	UserMSG_Msg_CheckBuffer(msgdata, sizeof(Int64));

	byte* pdata = msgdata.pmsgbuffer 
		+ msgdata.msgsize;

	memcpy(pdata, &value, sizeof(Int64));
	msgdata.msgsize += sizeof(Int64);
}

//=============================================
//
//=============================================
void UserMSG_Msg_WriteUint64( usermsgdata_t& msgdata, Uint64 value )
{
	if(msgdata.pusermsg == nullptr)
	{
		Con_EPrintf("%s - No message begin called.\n", __FUNCTION__);
		return;
	}

	UserMSG_Msg_CheckBuffer(msgdata, sizeof(Uint64));

	byte* pdata = msgdata.pmsgbuffer 
		+ msgdata.msgsize;

	memcpy(pdata, &value, sizeof(Uint64));
	msgdata.msgsize += sizeof(Uint64);
}

//=============================================
//
//=============================================
void UserMSG_Msg_WriteSmallFloat( usermsgdata_t& msgdata, Float value )
{
	Int16 intvalue = (Int16)SDL_floor(value * 8);
	UserMSG_Msg_WriteInt16(msgdata, intvalue);
}

//=============================================
//
//=============================================
void UserMSG_Msg_WriteFloat( usermsgdata_t& msgdata, Float value )
{
	if(msgdata.pusermsg == nullptr)
	{
		Con_EPrintf("%s - No message begin called.\n", __FUNCTION__);
		return;
	}

	UserMSG_Msg_CheckBuffer(msgdata, sizeof(Float));

	byte* pdata = msgdata.pmsgbuffer 
		+ msgdata.msgsize;

	memcpy(pdata, &value, sizeof(Float));

	msgdata.msgsize += sizeof(Float);
}

//=============================================
//
//=============================================
void UserMSG_Msg_WriteDouble( usermsgdata_t& msgdata, Double value )
{
	if(msgdata.pusermsg == nullptr)
	{
		Con_EPrintf("%s - No message begin called.\n", __FUNCTION__);
		return;
	}

	UserMSG_Msg_CheckBuffer(msgdata, sizeof(Double));

	byte* pdata = msgdata.pmsgbuffer 
		+ msgdata.msgsize;

	memcpy(pdata, &value, sizeof(Double));

	msgdata.msgsize += sizeof(Double);
}

//=============================================
//
//=============================================
void UserMSG_Msg_WriteBuffer( usermsgdata_t& msgdata, const byte* pdata, Uint32 size )
{
	if(msgdata.pusermsg == nullptr)
	{
		Con_EPrintf("%s - No message begin called.\n", __FUNCTION__);
		return;
	}

	UserMSG_Msg_CheckBuffer(msgdata, size);

	byte* pdest = msgdata.pmsgbuffer 
		+ msgdata.msgsize;

	for(Uint32 i = 0; i < size; i++)
		pdest[i] = pdata[i];
		
	msgdata.msgsize += size;
}

//=============================================
//
//=============================================
void UserMSG_Msg_WriteString( usermsgdata_t& msgdata, const Char* pstrstring )
{
	if(msgdata.pusermsg == nullptr)
	{
		Con_EPrintf("%s - No message begin called.\n", __FUNCTION__);
		return;
	}

	if(!pstrstring)
	{
		// Write the size
		UserMSG_Msg_WriteUint16(msgdata, 1);

		// Check for size
		UserMSG_Msg_CheckBuffer(msgdata, 1);
		byte* pdest = msgdata.pmsgbuffer 
			+ msgdata.msgsize;

		*pdest = '\0';
		msgdata.msgsize++;
		return;
	}

	// Write string length
	Uint32 strlength = qstrlen(pstrstring)+1;
	UserMSG_Msg_WriteUint16(msgdata, strlength);

	// Check buffer and write
	UserMSG_Msg_CheckBuffer(msgdata, strlength);

	byte* pdest = msgdata.pmsgbuffer 
		+ msgdata.msgsize;

	for(Uint32 i = 0; i < strlength; i++)
		pdest[i] = (byte)pstrstring[i];
		
	msgdata.msgsize += strlength;
}

//=============================================
//
//=============================================
void UserMSG_Msg_WriteEntindex( usermsgdata_t& msgdata, entindex_t entindex )
{
	// Is this function even neeeded?
	UserMSG_Msg_WriteInt32(msgdata, entindex);
}
