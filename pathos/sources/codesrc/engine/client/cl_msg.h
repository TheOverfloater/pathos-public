/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef CL_MSG_H
#define CL_MSG_H

class CNetworking;

extern bool CL_ReadMessages( void );
extern bool CL_ReadClientData( void );
extern bool CL_ReadPacketEntities( void );
extern void CL_SendPlayerInfo( void );
extern void CL_DisconnectMsg( void );
extern bool CL_ReadResourceList( void );
extern void CL_ReadMoveVars( void );
extern void CL_FileConsistencyMsg( void );
extern void CL_RegisterUserMessage( void );
extern void CL_ReadUserMessage( void );
extern bool CL_ParseServerInfo( void );
extern void CL_ParseClientCommand( void );
extern void CL_ParseViewSetAngles( void );
extern void CL_ParseAddAVelocity( void );
extern void CL_ParseHeartBeat( void );
extern void CL_ReadSoundEngineMessage( void );
extern void CL_ReadParticlePrecacheMessage( void );
extern void CL_ReadDecalPrecacheMessage( void );
extern void CL_ClientUserMessageEnd( void );
extern void CL_ClientUserMessageBegin( Int32 msgid );
extern Int32 CL_RegisterClientUserMessage( const Char* pstrMsgName, Int32 msgsize );
extern void CL_ClientRegisterUserMessages( void );
extern void CL_Msg_WriteByte( byte value );
extern void CL_Msg_WriteChar( Char value );
extern void CL_Msg_WriteInt16( Int16 value );
extern void CL_Msg_WriteUint16( Uint16 value );
extern void CL_Msg_WriteInt32( Int32 value );
extern void CL_Msg_WriteUint32( Uint32 value );
extern void CL_Msg_WriteInt64( Int64 value );
extern void CL_Msg_WriteUint64( Uint64 value );
extern void CL_Msg_WriteSmallFloat( Float value );
extern void CL_Msg_WriteFloat( Float value );
extern void CL_Msg_WriteDouble( Double value );
extern void CL_Msg_WriteBuffer( const byte* pdata, Uint32 size );
extern void CL_Msg_WriteString( const Char* pstring );
extern void CL_Msg_WriteEntindex( entindex_t entindex );
#endif //CL_MSG_H