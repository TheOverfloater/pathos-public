/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "system.h"

#include "cl_entity.h"
#include "cl_main.h"
#include "cl_msg.h"
#include "networking.h"
#include "cl_utils.h"
#include "cl_snd.h"
#include "cl_resource.h"
#include "cl_predict.h"
#include "cl_efx.h"

#include "brushmodel.h"
#include "modelcache.h"
#include "com_math.h"
#include "enginestate.h"
#include "vid.h"
#include "file.h"
#include "md5.h"
#include "commands.h"
#include "msgreader.h"

#include "texturemanager.h"
#include "r_particles.h"
#include "r_decals.h"

//=============================================
//
//=============================================
bool CL_ReadMessages( void )
{
	// Get instance of networking object
	CNetworking* pnetworking = cls.netinfo.pnet;
	assert(pnetworking != nullptr);

	byte* pdata = nullptr;
	Uint32 msgsize = 0;
	while(pnetworking->SVC_GetMessage(pdata, msgsize))
	{
		// Begin reading the message
		CMSGReader& reader = cls.netinfo.reader;
		reader.BeginRead(pdata, msgsize);
		
		// Get the identifier
		Int32 cmd = reader.ReadByte();

		if(reader.HasError())
			Con_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());

		switch(cmd)
		{
		case svc_nop:
			break;

		case svc_clcommand:
			CL_ParseClientCommand();
			break;

		case svc_serverinfo:
			if(!CL_ParseServerInfo())
				return false;
			break;

		case svc_clientdata:
			if(!CL_ReadClientData())
				return false;
			break;

		case svc_movevars:
			CL_ReadMoveVars();
			break;

		case svc_sndengine:
			CL_ReadSoundEngineMessage();
			break;

		case svc_registerusermsg:
			CL_RegisterUserMessage();
			break;

		case svc_usermsg:
			CL_ReadUserMessage();
			break;

		case svc_heartbeat:
			CL_ParseHeartBeat();
			break;

		case svc_setvangles:
			CL_ParseViewSetAngles();
			break;

		case svc_addavelocity:
			CL_ParseAddAVelocity();
			break;

		case svc_packetentities:
			if(!CL_ReadPacketEntities())
				return false;
			break;

		case svc_print:
			Con_Printf(reader.ReadString());
			break;

		case svc_disconnect:
			CL_DisconnectMsg();
			return false;
			break;

		case svc_resources:
			if(!CL_ReadResourceMessage())
				return false;
			break;

		case svc_consistency:
			CL_FileConsistencyMsg();
			break;

		case svc_setpause:
			cls.paused = (reader.ReadByte() == 1) ? true : false;
			break;

		case svc_precacheparticlescript:
			CL_ReadParticlePrecacheMessage();
			break;

		case svc_precachedecal:
			CL_ReadDecalPrecacheMessage();
			break;

		case svc_bad:
		default:
			Con_Printf("%s - svc_bad.\n", __FUNCTION__);
			break;
		}
	}

	// Clear message buffer
	pnetworking->SVC_ClearMessages();
	return true;
}

//=============================================
//
//=============================================
void CL_ParseHeartBeat( void )
{
	CMSGReader& reader = cls.netinfo.reader;
	bool prompt = (reader.ReadByte() == 1) ? true : false;
	
	if(reader.HasError())
	{
		Con_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return;
	}

	if(prompt)
	{
		Double delta = cls.cl_time - cls.cl_pingtime;
		Int32 responsemstime = SDL_ceil(delta*SECONDS_TO_MILLISECONDS);
		Con_Printf("Sever response time: %d ms.\n", responsemstime);
	}
}

//=============================================
//
//=============================================
void CL_ParseAddAVelocity( void )
{
	CMSGReader& reader = cls.netinfo.reader;

	Vector avelocity;
	for(Uint32 i = 0; i < 3; i++)
		avelocity[i] = reader.ReadFloat();

	if(reader.HasError())
	{
		Con_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return;
	}

	cls.clinfo.viewangles += avelocity;
}

//=============================================
//
//=============================================
void CL_ParseViewSetAngles( void )
{
	CMSGReader& reader = cls.netinfo.reader;

	Vector vangles;
	for(Uint32 i = 0; i < 3; i++)
		vangles[i] = reader.ReadFloat();

	if(reader.HasError())
	{
		Con_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return;
	}

	CL_SetViewAngles(vangles);
}

//=============================================
//
//=============================================
void CL_ParseClientCommand( void )
{
	CMSGReader& reader = cls.netinfo.reader;

	CString cmd = reader.ReadString();
	if(cmd.empty())
		return;

	if(reader.HasError())
	{
		Con_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return;
	}

	gCommands.AddCommand(cmd.c_str());
}

//=============================================
//
//=============================================
bool CL_ParseServerInfo( void )
{
	CMSGReader& reader = cls.netinfo.reader;

	// Parse skybox name
	cls.skyname = reader.ReadString();
	cls.skyvec.x = reader.ReadFloat();
	cls.skyvec.y = reader.ReadFloat();
	cls.skyvec.z = reader.ReadFloat();
	cls.skycolor.x = reader.ReadFloat();
	cls.skycolor.y = reader.ReadFloat();
	cls.skycolor.z = reader.ReadFloat();

	for(Uint32 i = 0; i < MAX_MAP_HULLS; i++)
	{
		for(Uint32 j = 0; j < 3; j++)
			cls.pminfo.player_mins[i][j] = reader.ReadFloat();

		for(Uint32 j = 0; j < 3; j++)
			cls.pminfo.player_maxs[i][j] = reader.ReadFloat();
	}

	if(reader.HasError())
	{
		Con_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	// Set this and try to init
	if(!cls.hasserverdata)
	{
		cls.hasserverdata = true;
		if(!CL_CheckGameReady())
			return false;
	}

	return true;
}

//=============================================
//
//=============================================
void CL_ReadMoveVars( void )
{
	CMSGReader& reader = cls.netinfo.reader;

	cls.pminfo.movevars.accelerate = reader.ReadFloat();
	cls.pminfo.movevars.airaccelerate = reader.ReadFloat();
	cls.pminfo.movevars.bounce = reader.ReadFloat();
	cls.pminfo.movevars.edgefriction = reader.ReadFloat();
	cls.pminfo.movevars.entgravity = reader.ReadFloat();
	cls.pminfo.movevars.friction = reader.ReadFloat();
	cls.pminfo.movevars.gravity = reader.ReadFloat();
	cls.pminfo.movevars.maxspeed = reader.ReadFloat();
	cls.pminfo.movevars.maxvelocity = reader.ReadFloat();
	cls.pminfo.movevars.stepsize = reader.ReadFloat();
	cls.pminfo.movevars.stopspeed = reader.ReadFloat();
	cls.pminfo.movevars.wateraccelerate = reader.ReadFloat();
	cls.pminfo.movevars.waterfriction = reader.ReadFloat();
	cls.pminfo.movevars.waterdist = reader.ReadFloat();
	cls.pminfo.movevars.maxclients = reader.ReadByte();
	cls.pminfo.movevars.holdtoduck = (reader.ReadByte() ? true : false);

	if(reader.HasError())
	{
		Con_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return;
	}

	// Set in cls too
	cls.maxclients = cls.pminfo.movevars.maxclients;
}

//=============================================
//
//=============================================
bool CL_ReadClientData( void )
{
	CMSGReader& reader = cls.netinfo.reader;

	// Read the local time
	cls.clinfo.entindex = reader.ReadInt16();
	cls.clientindex = reader.ReadInt16();
	cls.cl_clsvtime = reader.ReadDouble();
	Uint32 lastsvusercmdidx = reader.ReadUint64();
	cls.parsecount++;

	if(reader.HasError())
	{
		Con_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	// Update usercmd history
	CL_CleanUserCmdHistory(lastsvusercmdidx);

	if(!cls.hasclientdata)
	{
		cls.hasclientdata = true;
		if(!CL_CheckGameReady())
			return false;
	}

	return true;
}

//=============================================
//
//=============================================
void CL_SendPlayerInfo( void )
{
	CNetworking* pnetworking = cls.netinfo.pnet;
	assert(pnetworking != nullptr);

	if(pnetworking->GetClientState(0) != NETCL_CONNECTED)
		return;

	const Char* pstrPlayerName = g_pCvarName->GetStrValue();
	pnetworking->CLS_MessageBegin(cls_clientinfo);
		pnetworking->WriteString(pstrPlayerName);
	pnetworking->CLS_MessageEnd();

	// Register client usermessages(this is the earliest we can do this)
	CL_ClientRegisterUserMessages();
}

//=============================================
//
//=============================================
void CL_DisconnectMsg( void )
{
	CMSGReader& reader = cls.netinfo.reader;

	// Determine if we got rejected
	bool rejected = reader.ReadByte() ? true : false;
	if(rejected)
	{
		const Char* pstrReason = reader.ReadString();
		Con_Printf("Rejected: %s.\n", pstrReason);
	}
	else
		Con_Printf("Dropped from server.\n");

	if(reader.HasError())
	{
		Con_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return;
	}

	// Disconnect us
	CL_Disconnect();
}

//=============================================
//
//=============================================
void CL_UpdateLatchedState( cl_entity_t* pentity )
{
	if(!pentity->pmodel || pentity->pmodel->type != MOD_VBM)
		return;

	if(pentity->prevstate.sequence != pentity->curstate.sequence
		&& !(pentity->curstate.effects & EF_NOLERP))
	{
		pentity->latched.sequence = pentity->prevstate.sequence;
		pentity->latched.sequencetime = pentity->curstate.animtime; // Otherwise take animtime

		for(Uint32 i = 0; i < MAX_BLENDING; i++)
			pentity->latched.prevseqblending[i] = pentity->prevstate.blending[i];
	}

	// Copy last predicted origin and angles
	pentity->latched.angles = pentity->prevstate.angles;
	pentity->latched.origin = pentity->prevstate.origin;
	pentity->latched.animtime = pentity->prevstate.animtime;

	for(Uint32 i = 0; i < MAX_CONTROLLERS; i++)
		pentity->latched.controllers[i] = pentity->prevstate.controllers[i];

	for(Uint32 i = 0; i < MAX_BLENDING; i++)
		pentity->latched.blending[i] = pentity->prevstate.blending[i];

	// Set attachments to their base values
	for(Uint32 i = 0; i < MAX_ATTACHMENTS; i++)
		pentity->attachments[i] = pentity->prevstate.origin;
}

//=============================================
//
//=============================================
void CL_ResetLatchedState( cl_entity_t* pentity )
{
	if(!pentity->pmodel || pentity->pmodel->type != MOD_VBM)
		return;

	pentity->latched.sequence = 0;
	pentity->latched.sequencetime = 0;

	for(Uint32 i = 0; i < MAX_BLENDING; i++)
		pentity->latched.prevseqblending[i] = pentity->curstate.blending[i];

	// Copy last predicted origin and angles
	pentity->latched.angles = pentity->curstate.angles;
	pentity->latched.origin = pentity->curstate.origin;
	pentity->latched.animtime = pentity->curstate.animtime;

	for(Uint32 i = 0; i < MAX_CONTROLLERS; i++)
		pentity->latched.controllers[i] = pentity->curstate.controllers[i];

	for(Uint32 i = 0; i < MAX_BLENDING; i++)
		pentity->latched.blending[i] = pentity->curstate.blending[i];

	// Set attachments to their base values
	for(Uint32 i = 0; i < MAX_ATTACHMENTS; i++)
		pentity->attachments[i] = pentity->curstate.origin;
}

//=============================================
//
//=============================================
bool CL_ReadPacketEntities( void )
{
	CMSGReader& reader = cls.netinfo.reader;

	// Get the number of entities
	Uint32 numentities = reader.ReadUint32();
	
	// Parse the entities
	for(Uint32 i = 0; i < numentities; i++)
	{
		// Get the entity index
		entindex_t entindex = reader.ReadInt16();
		// Get the identifier
		Uint32 identifier = reader.ReadUint32();

		// Filter for bad messages
		if(entindex >= (Int32)cls.entities.size())
		{
			Con_Printf("CL_ReadPacketEntities - svc_bad.\n", __FUNCTION__);
			return false;
		}

		if(entindex >= cls.numentities)
			cls.numentities = entindex+1;

		// Get entity and set the msg_num of the local client
		cl_entity_t* pentity = CL_GetEntityByIndex(entindex);

		// Set entity index always
		pentity->entindex = entindex;
		
		// Check if identifier was changed
		if(pentity->identifier != identifier)
		{
			CL_FreeEntityData(entindex, FREE_MSG_FL_NONE);
			pentity->identifier = identifier;
		}

		// Copy current state to prevstate
		pentity->prevstate = pentity->curstate;
		// Fill curstate
		entity_state_t& state = pentity->curstate;

		// Set entity index and parsecount
		state.entindex = entindex;
		state.msg_num = cls.parsecount;
		state.msg_time = cls.cl_time;

		// Read in the update mask
		Uint32 updateMask = reader.ReadUint32();

		// Nothing fancy until we have actual networking
		if(updateMask & U_ORIGIN)
		{
			for(Uint32 j = 0; j < 3; j++)
				state.origin[j] = reader.ReadFloat();
		}

		if(updateMask & U_ANGLES)
		{
			for(Uint32 j = 0; j < 3; j++)
				state.angles[j] = reader.ReadFloat();
		}

		// Read velocities
		if(updateMask & U_VELOCITY)
		{
			for(Uint32 j = 0; j < 3; j++)
				state.velocity[j] = reader.ReadFloat();

			state.fallvelocity = reader.ReadFloat();
		}

		if(updateMask & U_AVELOCITY)
		{
			for(Uint32 j = 0; j < 3; j++)
				state.avelocity[j] = reader.ReadFloat();
		}

		if(updateMask & U_BASEVELOCITY)
		{
			for(Uint32 j = 0; j < 3; j++)
				state.basevelocity[j] = reader.ReadFloat();
		}

		// Read view related things
		if(updateMask & U_PUNCHANGLES)
		{
			for(Uint32 j = 0; j < 3; j++)
				state.punchangles[j] = reader.ReadFloat();

			for(Uint32 j = 0; j < 3; j++)
				state.punchamount[j] = reader.ReadFloat();
		}

		if(updateMask & U_VIEWANGLES)
		{
			for(Uint32 j = 0; j < 3; j++)
				state.viewangles[j] = reader.ReadFloat();
		}

		if(updateMask & U_VIEWOFFSET)
		{
			for(Uint32 j = 0; j < 3; j++)
				state.view_offset[j] = reader.ReadFloat();
		}

		// Read endpos/startpos
		if(updateMask & U_POSITIONS)
		{
			for(Uint32 j = 0; j < 3; j++)
				state.endpos[j] = reader.ReadFloat();

			for(Uint32 j = 0; j < 3; j++)
				state.startpos[j] = reader.ReadFloat();
		}

		// Read mins/maxs
		if(updateMask & U_MINSMAXS)
		{
			for(Uint32 j = 0; j < 3; j++)
				state.mins[j] = reader.ReadFloat();
			for(Uint32 j = 0; j < 3; j++)
				state.maxs[j] = reader.ReadFloat();
		}

		if(updateMask & U_BASICS1)
		{
			state.modelindex = reader.ReadInt32();
			state.movetype = (movetype_t)reader.ReadInt32();
			state.solid = (solid_t)reader.ReadInt32();
			state.groupinfo = reader.ReadInt32();
		}

		if(updateMask & U_BASICS2)
		{
			state.skin = reader.ReadInt32();
			state.body = reader.ReadInt64();
			state.effects = reader.ReadInt64();
		}

		if(updateMask & U_BASICS3)
		{
			state.gravity = reader.ReadFloat();
			state.friction = reader.ReadFloat();
		}

		if(updateMask & U_ANIMINFO)
		{
			state.sequence = reader.ReadInt32();
			state.frame = reader.ReadFloat();
			state.animtime = reader.ReadFloat();
			state.framerate = reader.ReadFloat();
		}

		if(updateMask & U_CONTROLLERS)
		{
			for(Uint32 j = 0; j < MAX_CONTROLLERS; j++)
				state.controllers[j] = reader.ReadFloat();
		}

		if(updateMask & U_BLENDING)
		{
			for(Uint32 j = 0; j < MAX_BLENDING; j++)
				state.blending[j] = reader.ReadFloat();
		}

		if(updateMask & U_RENDERINFO)
		{
			state.scale = reader.ReadFloat();
			state.rendertype = (rendertype_t)reader.ReadInt32();
			state.rendermode = (rendermode_t)reader.ReadInt32();
			state.renderamt = reader.ReadFloat();
			state.renderfx = reader.ReadInt32();
			state.numsegments = reader.ReadUint32();

			for(Uint32 j = 0; j < 3; j++)
				state.rendercolor[j] = reader.ReadFloat();

			for(Uint32 j = 0; j < 3; j++)
				state.lightorigin[j] = reader.ReadFloat();
		}

		if(updateMask & U_BASICS4)
		{
			state.health = reader.ReadFloat();
			state.armorvalue = reader.ReadFloat();
			state.frags = reader.ReadInt32();
			state.weapons = reader.ReadInt64();
			state.buttons = reader.ReadInt32();
			state.oldbuttons = reader.ReadInt32();
			state.flags = reader.ReadUint64();
			state.waterlevel = (waterlevel_t)reader.ReadInt32();
			state.fov = reader.ReadFloat();
		}

		if(updateMask & U_ENTSINFO)
		{
			state.aiment = reader.ReadInt16();
			state.owner = reader.ReadInt16();
			state.groundent = reader.ReadInt16();
		}

		if(updateMask & U_PLAYERINFO)
		{
			state.stamina = reader.ReadFloat();
			state.induck = (reader.ReadByte() == 1) ? true : false;
			state.ducktime = reader.ReadFloat();
			state.waterjumptime = reader.ReadFloat();
			state.swimtime = reader.ReadFloat();
			state.timestepsound = reader.ReadFloat();
			state.stepleft = (reader.ReadByte() == 1) ? true : false;
			state.planezcap = reader.ReadFloat();
			state.weaponanim = reader.ReadByte();
		}

		if(updateMask & U_IUSER)
		{
			state.iuser1 = reader.ReadInt32();
			state.iuser2 = reader.ReadInt32();
			state.iuser3 = reader.ReadInt32();
			state.iuser4 = reader.ReadInt32();
			state.iuser5 = reader.ReadInt32();
			state.iuser6 = reader.ReadInt32();
			state.iuser7 = reader.ReadInt32();
			state.iuser8 = reader.ReadInt32();
		}

		if(updateMask & U_FUSER)
		{
			state.fuser1 = reader.ReadFloat();
			state.fuser2 = reader.ReadFloat();
			state.fuser3 = reader.ReadFloat();
			state.fuser4 = reader.ReadFloat();
		}

		if(updateMask & U_VUSER1)
		{
			for(Uint32 j = 0; j < 3; j++)
				state.vuser1[j] = reader.ReadFloat();
		}

		if(updateMask & U_VUSER2)
		{
			for(Uint32 j = 0; j < 3; j++)
				state.vuser2[j] = reader.ReadFloat();
		}

		if(updateMask & U_VUSER3)
		{
			for(Uint32 j = 0; j < 3; j++)
				state.vuser3[j] = reader.ReadFloat();
		}

		if(updateMask & U_VUSER4)
		{
			for(Uint32 j = 0; j < 3; j++)
				state.vuser4[j] = reader.ReadFloat();
		}

		if(updateMask & U_PARENTING)
		{
			state.parent = reader.ReadInt32();

			for(Uint32 j = 0; j < 3; j++)
				state.parentoffset[j] = reader.ReadFloat();
		}

		// Flag as client if needed
		if(state.flags & FL_CLIENT)
			pentity->player = true;

		// Set model
		if(state.modelindex)
		{
			cache_model_t* pmodel = gModelCache.GetModelByIndex(state.modelindex);
			pentity->pmodel = pmodel;
		}
		else
			pentity->pmodel = nullptr;

		// If entity wasn't rendered yet, paste current state to prevstate
		// to avoid interpolation-related problems
		bool isInitialInfoPacket = false;
		if(!pentity->prevstate.msg_time && !pentity->prevstate.msg_num)
		{
			pentity->prevstate = pentity->curstate;
			isInitialInfoPacket = true;
		}

		// Adjust timers on entity if this is not the host client
		// Host clients are synced 1:1 with the server timer
		if(!CL_IsHostClient() && (updateMask & U_ANIMINFO))
			cls.dllfuncs.pfnAdjustEntityTimers(&state, cls.cl_clsvtime);

		// This shouldn't possibly happen, but let's be sure
		if(state.animtime > cls.cl_time)
			state.animtime = cls.cl_time;

		// Update latched states
		if(updateMask & U_NEW_TO_PACKET)
		{
			// Entity hasn't been in the packet for a while, so reset
			// everything to curstate
			CL_ResetLatchedState(pentity);
		}
		else if((isInitialInfoPacket || pentity->curstate.animtime != pentity->prevstate.animtime))
		{
			// Animtime has changed, so update
			CL_UpdateLatchedState(pentity);
		}

		// Handle EF_MUZZLEFLASH for legacy support
		if(pentity->curstate.effects & EF_MUZZLEFLASH)
		{
			mlight_t* pmlight = CL_AllocEntityLight(pentity->curstate.entindex, 0.1f, 0);
			pmlight->origin = pentity->getAttachment(0);
			pmlight->color.x = 1.0;
			pmlight->color.y = 0.75;
			pmlight->color.z = 0.25;
			pmlight->radius = Common::RandomFloat(30, 40);
		}
	}

	if(reader.HasError())
	{
		Con_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	// Set the entity data flag
	if(!cls.hasentitydata)
	{
		cls.hasentitydata = true;
		if(!CL_CheckGameReady())
			return false;
	}

	// We need player data to start
	if(!cls.hasplayerentitydata && cls.clinfo.entindex)
	{
		cl_entity_t* pEntity = CL_GetEntityByIndex(cls.clinfo.entindex);
		if(pEntity && pEntity->curstate.msg_num > 0)
		{
			cls.hasplayerentitydata = true;
			if(!CL_CheckGameReady())
				return false;
		}
	}

	return true;
}

//=============================================
//
//=============================================
void CL_FileConsistencyMsg( void )
{
	CMSGReader& reader = cls.netinfo.reader;

	// Retrieve the file path string
	const Char* pstrFilename = reader.ReadString();

	if(reader.HasError())
	{
		Con_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return;
	}

	// Load the file in
	Uint32 filesize = 0;
	const byte* pfile = FL_LoadFile(pstrFilename, &filesize);
	if(!pfile)
	{
		Con_Printf("%s - Could not load '%s'.\n", __FUNCTION__, pstrFilename);
		return;
	}

	// Create the hash
	CString hash = CMD5(pfile, filesize).HexDigest();
	FL_FreeFile(pfile);

	// Send it to the server
	cls.netinfo.pnet->CLS_MessageBegin(cls_consistency);
		cls.netinfo.pnet->WriteString(pstrFilename);
		cls.netinfo.pnet->WriteString(hash.c_str());
	cls.netinfo.pnet->CLS_MessageEnd();
}

//=============================================
//
//=============================================
void CL_ReadUserMessage( void )
{
	CMSGReader& reader = cls.netinfo.reader;

	// Read in the ID
	Int32 msgid = reader.ReadByte();
	Int32 msgindex = msgid - 1;

	// Look it up
	if((Int32)cls.netinfo.usermsgfunctions.size() <= msgindex)
	{
		Con_EPrintf("%s - Message with bogus id %d received.\n", __FUNCTION__, msgid);
		return;
	}

	cl_usermsgfunction_t& msg = cls.netinfo.usermsgfunctions[msgindex];
	if(!msg.pfnReadMsg)
	{
		Con_EPrintf("%s - Message '%s' function not found on client.\n", __FUNCTION__, msg.name.c_str());
		return;
	}

	Uint16 msgsize = reader.ReadUint16();
	const byte* pdata = reader.ReadBuffer(msgsize);

	if(reader.HasError())
	{
		Con_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return;
	}

	// Call client to read the message contents
	if(!msg.pfnReadMsg(msg.name.c_str(), pdata, msgsize))
		Con_Printf("%s - Client returned error on message '%s' read.\n", __FUNCTION__, msg.name.c_str());
}

//=============================================
//
//=============================================
void CL_ReadParticlePrecacheMessage( void )
{
	CMSGReader& reader = cls.netinfo.reader;

	const Char* pstrfilepath = reader.ReadString();
	part_script_type_t type = (part_script_type_t)reader.ReadByte();

	if(reader.HasError())
	{
		Con_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return;
	}

	gParticleEngine.PrecacheScript(type, pstrfilepath, false);
}

//=============================================
//
//=============================================
void CL_ReadDecalPrecacheMessage( void )
{
	CMSGReader& reader = cls.netinfo.reader;

	CString name = reader.ReadString();
	decalcache_type_t type = (decalcache_type_t)reader.ReadByte();

	for(Uint32 i = 0; i < cls.netinfo.decalcache.size(); i++)
	{
		const decalcache_t& cache = cls.netinfo.decalcache[i];
		if(!qstrcmp(cache.name, name) && cache.type == type)
			return;
	}

	decalcache_t newcache;
	newcache.name = name;
	newcache.type = type;

	cls.netinfo.decalcache.push_back(newcache);

	if(reader.HasError())
	{
		Con_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return;
	}
}

//=============================================
//
//=============================================
void CL_RegisterUserMessage( void )
{
	CMSGReader& reader = cls.netinfo.reader;

	// Read message info
	Uint32 msgid = reader.ReadByte();
	const Char* pstrMsgName = reader.ReadString();

	if(reader.HasError())
	{
		Con_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return;
	}

	// Make sure we don't add it twice, not like it's likely to happen on client
	for(Uint32 i = 0; i < cls.netinfo.usermsgfunctions.size(); i++)
	{
		if(!qstrcmp(cls.netinfo.usermsgfunctions[i].name, pstrMsgName))
			return;
	}

	// See if this message is already registered
	Int32 index = msgid - 1;
	if(index < (Int32)cls.netinfo.usermsgfunctions.size())
	{
		cl_usermsgfunction_t& msg = cls.netinfo.usermsgfunctions[index];
		if(qstrcmp(msg.name, pstrMsgName) || msg.id != msgid)
		{
			Con_EPrintf("%s - Mismatch with user message '%s', client definition for id %d is '%s'.\n", __FUNCTION__, pstrMsgName, msgid, msg.name.c_str());
			return;
		}
		return;
	}

	CString usermsgfuncname;
	usermsgfuncname << "MsgFunc_" << pstrMsgName;

	// Load function pointer
	pfnCLUserMsg_t pFunction = reinterpret_cast<pfnCLUserMsg_t>(SDL_LoadFunction(cls.pdllhandle, usermsgfuncname.c_str()));
	if(!pFunction)
	{
		// Add the entry anyway, but warn the client
		Con_EPrintf("%s - Function '%s' not found on client.\n", __FUNCTION__, usermsgfuncname.c_str());
	}

	// Create the entry
	cls.netinfo.usermsgfunctions.resize(index+1);
	cl_usermsgfunction_t& msg = cls.netinfo.usermsgfunctions[index];

	msg.id = msgid;
	msg.name = pstrMsgName;
	msg.pfnReadMsg = pFunction;
}

//=============================================
// @brief
//
//=============================================
void CL_ReadSoundEngineMessage( void )
{
	CMSGReader& reader = cls.netinfo.reader;

	Int32 msgType = reader.ReadByte();
	switch(msgType)
	{
	case MSG_SNDENGINE_OGG:
		{
			Float fadetime = 0;
			Float timeoffset = 0;
			const Char* pstring = nullptr;

			Int32 flags = reader.ReadByte();
			Int32 channel = reader.ReadByte();
			if(!(flags & OGG_FL_STOP))
			{
				pstring = reader.ReadString();

				if(flags & OGG_FL_STOP_FADE)
				{
					// Grab fade time if stop fade flag is set
					fadetime = reader.ReadFloat();
				}
				else
				{
					// Grab fade in time if set
					if(flags & OGG_FL_FADE_IN)
						fadetime = reader.ReadFloat();

					// Grab any time offset
					timeoffset = reader.ReadFloat();
				}
			}

			if(reader.HasError())
			{
				Con_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
				return;
			}

			if(flags & OGG_FL_STOP)
			{
				// Stop playback instantly
				gSoundEngine.StopOgg(channel);
			}
			else if(flags & OGG_FL_STOP_FADE)
			{
				// Fade playback then stop
				gSoundEngine.StopOggFade(pstring, channel, fadetime);
			}
			else
			{
				// Stop any previous tracks, and play this one
				gSoundEngine.PlayOgg(pstring, channel, timeoffset, flags, fadetime);
			}
		}
		break;
	case MSG_SNDENGINE_ROOMTYPE:
		{
			Int32 reverb = reader.ReadByte();

			if(reader.HasError())
			{
				Con_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
				return;
			}

			gSoundEngine.SetActiveReverb(reverb);
		}
		break;
	case MSG_SNDENGINE_KILL_ALL_SOUNDS:
		{
			bool muted = (reader.ReadByte() == 1) ? true : false;

			if(reader.HasError())
			{
				Con_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
				return;
			}

			gSoundEngine.SetMuted(muted);
		}
		break;
	case MSG_SNDENGINE_PRECACHE:
		{
			const Char *sample = reader.ReadString();
			Int32 svindex = reader.ReadInt16();

			if(reader.HasError())
			{
				Con_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
				return;
			}

			gSoundEngine.PrecacheServerSound(sample, svindex);
		}
		break;
	case MSG_SNDENGINE_EMITENTITYSOUND:
		{
			Int32 svindex = reader.ReadInt16();
			entindex_t entindex = reader.ReadInt16();
			Float volume = reader.ReadFloat();
			Float atten = reader.ReadFloat();
			Int32 channel = reader.ReadByte();
			Int32 flags = reader.ReadInt32();
			Int32 pitch = reader.ReadInt16();
			Float timeoffset = reader.ReadSmallFloat();

			if(reader.HasError())
			{
				Con_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
				return;
			}

			if(volume < 0 || volume > VOL_NORM)
			{
				Con_Printf("%s - Invalid volume %f for ambient sound with server index %d.\n", __FUNCTION__, volume, svindex);
				return;
			}

			if(gSoundEngine.IsMuted() && !(flags & SND_FL_MUTEIGNORE))
				return;

			cl_entity_t *entity = CL_GetEntityByIndex(entindex);
			if(!entity || !entity->pmodel || !CL_IsGameActive())
			{	
				gSoundEngine.CacheMessage(nullptr, svindex, channel, flags, entindex, pitch, volume, atten, false, timeoffset);
				return;
			}

			if(flags & (SND_FL_STOP|SND_FL_CHANGE_PITCH|SND_FL_CHANGE_VOLUME))
				gSoundEngine.UpdateSound(nullptr, nullptr, flags, channel, volume, pitch, atten, entity, entindex, svindex, timeoffset);
			else
				gSoundEngine.PlaySound(nullptr, nullptr, flags, channel, volume, pitch, atten, entity, entindex, svindex, timeoffset);
		}
		break;
	case MSG_SNDENGINE_EMITAMBIENTSOUND:
		{
			Vector origin;
			Int32 svindex = reader.ReadInt16();
			entindex_t entindex = reader.ReadInt16();
			Float volume = reader.ReadFloat();
			Float atten = reader.ReadFloat();
			Int32 flags = reader.ReadInt32();
			Int32 pitch = reader.ReadInt16();
			origin[0] = reader.ReadFloat();
			origin[1] = reader.ReadFloat();
			origin[2] = reader.ReadFloat();
			Float timeoffs = reader.ReadSmallFloat();

			if(reader.HasError())
			{
				Con_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
				return;
			}

			if(volume < 0 || volume > VOL_NORM)
			{
				Con_Printf("%s - Invalid volume %f for ambient sound with server index %d.\n", __FUNCTION__, volume, svindex);
				return;
			}

			if(!CL_IsGameActive())
			{	
				gSoundEngine.CacheMessage(&origin, svindex, SND_CHAN_AUTO, flags, entindex, pitch, volume, atten, true, timeoffs);
				return;
			}

			if(gSoundEngine.IsMuted() && !(flags & SND_FL_MUTEIGNORE))
				return;

			if(flags & (SND_FL_STOP|SND_FL_CHANGE_PITCH|SND_FL_CHANGE_VOLUME))
				gSoundEngine.UpdateSound(nullptr, &origin, flags, SND_CHAN_AUTO, volume, pitch, atten, nullptr, entindex, svindex, timeoffs);
			else
				gSoundEngine.PlaySound(nullptr, &origin, flags, SND_CHAN_AUTO, volume, pitch, atten, nullptr, entindex, svindex, timeoffs);
		}
		break;
	case MSG_SNDENGINE_APPLY_EFFECT:
		{
			Int32 svindex = reader.ReadInt16();
			entindex_t entindex = reader.ReadInt16();
			byte channel = reader.ReadByte();
			snd_effects_t effect = (snd_effects_t)reader.ReadByte();
			Float duration = reader.ReadSmallFloat();
			Float targetvalue = reader.ReadFloat();

			if(reader.HasError())
			{
				Con_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
				return;
			}

			if(effect == SND_EF_CHANGE_VOLUME && (targetvalue < 0 || targetvalue > VOL_NORM))
			{
				Con_Printf("%s - Invalid volume %f for sound effect with server index %d.\n", __FUNCTION__, targetvalue, svindex);
				return;
			}

			if(!CL_IsGameActive() || !gSoundEngine.IsSoundPlaying(entindex, svindex, channel))
				gSoundEngine.CacheEffect(entindex, svindex, channel, effect, duration, targetvalue);
			else
				gSoundEngine.ApplySoundEffect(entindex, svindex, channel, effect, duration, targetvalue);
		}
		break;
	case MSG_SNDENGINE_KILL_ENTITY_SOUNDS:
		{
			entindex_t entindex = reader.ReadInt16();
			byte channel = reader.ReadByte();

			gSoundEngine.StopSound(entindex, channel);
		}
		break;
	default:
		{
			Con_EPrintf("%s - Unknown message type %d.\n", __FUNCTION__, msgType);
		}
		break;
	}
}

//=============================================
//
//=============================================
Int32 CL_RegisterClientUserMessage( const Char* pstrMsgName, Int32 msgsize )
{
	return UserMSG_RegisterUserMessage(cls.netinfo.usermsgs, pstrMsgName, msgsize);
}

//=============================================
//
//=============================================
void CL_ClientUserMessageBegin( Int32 msgid )
{
	UserMSG_UserMessageBegin(cls.netinfo.usermsgs, cls.netinfo.msgdata, MSG_ONE, msgid, nullptr, nullptr);
}

//=============================================
//
//=============================================
void CL_ClientUserMessageEnd( void )
{
	// Check for invalid calls
	if(cls.netinfo.msgdata.pusermsg == nullptr)
	{
		Con_EPrintf("%s - Invalid message send attempt.\n", __FUNCTION__);
		return;
	}

	if(cls.netinfo.msgdata.pusermsg->bufsize != -1 && cls.netinfo.msgdata.msgsize != cls.netinfo.msgdata.pusermsg->bufsize)
	{
		// Notify client about buffer size mismatch
		Con_Printf("%s - User message '%s' mismatch with fixed size %d - %d bytes written.\n", 
			__FUNCTION__, 
			cls.netinfo.msgdata.pusermsg->name.c_str(),
			cls.netinfo.msgdata.msgsize);
	}

	// get data ptr
	usermsgdata_t* pmsgdata = &cls.netinfo.msgdata;
	if(pmsgdata->msgsize <= 0)
	{
		Con_EPrintf("%s - Bogus message size %d for message '%s'.\n", __FUNCTION__, pmsgdata->msgsize, pmsgdata->pusermsg->name.c_str());
		return;
	}

	// write the data
	cls.netinfo.pnet->CLS_MessageBegin(cls_usermsg);
		cls.netinfo.pnet->WriteByte(pmsgdata->pusermsg->id);
		cls.netinfo.pnet->WriteUint16(pmsgdata->msgsize);
		cls.netinfo.pnet->WriteBuffer(pmsgdata->pmsgbuffer, pmsgdata->msgsize);
	cls.netinfo.pnet->CLS_MessageEnd();

	// Reset msg data
	pmsgdata->msgsize = 0;
	pmsgdata->pedict = nullptr;
	pmsgdata->pusermsg = nullptr;
}

//=============================================
//
//=============================================
void CL_ClientRegisterUserMessages( void )
{
	// Make sure client is valid
	if(cls.cl_state == CLIENT_INACTIVE)
	{
		Con_EPrintf("%s - Client is not connected to a server.\n", __FUNCTION__);
		return;
	}

	for(Uint32 i = 0; i < cls.netinfo.usermsgs.size(); i++)
	{
		usermsg_t& msg = cls.netinfo.usermsgs[i];

		// write the data
		cls.netinfo.pnet->CLS_MessageBegin(cls_registerusermsg);
			cls.netinfo.pnet->WriteByte(msg.id);
			cls.netinfo.pnet->WriteString(msg.name.c_str());
		cls.netinfo.pnet->CLS_MessageEnd();
	}
}

//=============================================
//
//=============================================
void CL_Msg_WriteByte( byte value )
{
	UserMSG_Msg_WriteByte(cls.netinfo.msgdata, value);
}

//=============================================
//
//=============================================
void CL_Msg_WriteChar( Char value )
{
	UserMSG_Msg_WriteChar(cls.netinfo.msgdata, value);
}

//=============================================
//
//=============================================
void CL_Msg_WriteInt16( Int16 value )
{
	UserMSG_Msg_WriteInt16(cls.netinfo.msgdata, value);
}

//=============================================
//
//=============================================
void CL_Msg_WriteUint16( Uint16 value )
{
	UserMSG_Msg_WriteUint16(cls.netinfo.msgdata, value);
}

//=============================================
//
//=============================================
void CL_Msg_WriteInt32( Int32 value )
{
	UserMSG_Msg_WriteInt32(cls.netinfo.msgdata, value);
}

//=============================================
//
//=============================================
void CL_Msg_WriteUint32( Uint32 value )
{
	UserMSG_Msg_WriteUint32(cls.netinfo.msgdata, value);
}

//=============================================
//
//=============================================
void CL_Msg_WriteInt64( Int64 value )
{
	UserMSG_Msg_WriteInt64(cls.netinfo.msgdata, value);
}

//=============================================
//
//=============================================
void CL_Msg_WriteUint64( Uint64 value )
{
	UserMSG_Msg_WriteUint64(cls.netinfo.msgdata, value);
}

//=============================================
//
//=============================================
void CL_Msg_WriteSmallFloat( Float value )
{
	UserMSG_Msg_WriteSmallFloat(cls.netinfo.msgdata, value);
}

//=============================================
//
//=============================================
void CL_Msg_WriteFloat( Float value )
{
	UserMSG_Msg_WriteFloat(cls.netinfo.msgdata, value);
}

//=============================================
//
//=============================================
void CL_Msg_WriteDouble( Double value )
{
	UserMSG_Msg_WriteDouble(cls.netinfo.msgdata, value);
}

//=============================================
//
//=============================================
void CL_Msg_WriteBuffer( const byte* pdata, Uint32 size )
{
	UserMSG_Msg_WriteBuffer(cls.netinfo.msgdata, pdata, size);
}

//=============================================
//
//=============================================
void CL_Msg_WriteString( const Char* pstring )
{
	UserMSG_Msg_WriteString(cls.netinfo.msgdata, pstring);
}

//=============================================
//
//=============================================
void CL_Msg_WriteEntindex( entindex_t entindex )
{
	// Is this function even neeeded?
	UserMSG_Msg_WriteEntindex(cls.netinfo.msgdata, entindex);
}