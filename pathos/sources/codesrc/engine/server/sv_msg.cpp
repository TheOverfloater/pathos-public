/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "com_math.h"
#include "edict.h"
#include "cvar.h"
#include "sv_main.h"
#include "sv_entities.h"
#include "system.h"
#include "sv_msg.h"
#include "file.h"
#include "enginestate.h"
#include "networking.h"
#include "modelcache.h"
#include "edictmanager.h"
#include "constants.h"
#include "snd_shared.h"
#include "md5.h"
#include "texturemanager.h"
#include "cl_snd.h"
#include "commands.h"
#include "console.h"

// Minimum chunk size is 56 kilobytes
static const Uint32 MIN_FILECHUNK_SIZE = 57344;
// Maximum chunk size is 4 megabytes
static const Uint32 MAX_FILECHUNK_SIZE = 4194304;

//=============================================
//
//=============================================
bool SV_SendClientData( sv_client_t* pclient )
{
	// Write the server info like time, etc before anything else is sent
	svs.netinfo.pnet->SVC_MessageBegin(MSG_ONE, svc_clientdata, pclient->pedict);
		svs.netinfo.pnet->WriteInt16(pclient->pedict->entindex);
		svs.netinfo.pnet->WriteInt16(pclient->index);
		svs.netinfo.pnet->WriteDouble(pclient->jointime);
		svs.netinfo.pnet->WriteUint64(pclient->lastusercmdidx); // write last acknowledged usercmd index
	svs.netinfo.pnet->SVC_MessageEnd();

	// Check for errors on client
	if(svs.netinfo.pnet->GetClientState(pclient->index) != NETCL_CONNECTED)
		return false;

	// Send packet entities to the client
	if(!SV_WriteEntitiesToClient(pclient))
		return false;

	// Check for errors on client
	if(svs.netinfo.pnet->GetClientState(pclient->index) != NETCL_CONNECTED)
		return false;

	// Send server data if we just spawned
	if(!pclient->initialized)
	{
		// Only skybox name for now
		const Char* pstrSkyname = g_psv_skyname->GetStrValue();
		svs.netinfo.pnet->SVC_MessageBegin(MSG_ONE, svc_serverinfo, pclient->pedict);
			// Skybox information
			svs.netinfo.pnet->WriteString(pstrSkyname);
			svs.netinfo.pnet->WriteFloat(g_psv_skyvec_x->GetValue());
			svs.netinfo.pnet->WriteFloat(g_psv_skyvec_y->GetValue());
			svs.netinfo.pnet->WriteFloat(g_psv_skyvec_z->GetValue());
			svs.netinfo.pnet->WriteFloat(g_psv_skycolor_r->GetValue());
			svs.netinfo.pnet->WriteFloat(g_psv_skycolor_g->GetValue());
			svs.netinfo.pnet->WriteFloat(g_psv_skycolor_b->GetValue());

			// Write hull sizes
			for(Uint32 i = 0; i < MAX_MAP_HULLS; i++)
			{
				for(Int32 j = 0; j < 3; j++)
					svs.netinfo.pnet->WriteFloat(svs.player_mins[i][j]);

				for(Int32 j = 0; j < 3; j++)
					svs.netinfo.pnet->WriteFloat(svs.player_maxs[i][j]);
			}
		svs.netinfo.pnet->SVC_MessageEnd();

		pclient->initialized = true;

		// Check for errors on client
		if(svs.netinfo.pnet->GetClientState(pclient->index) != NETCL_CONNECTED)
			return false;
	}

	// Set angles for the client
	if(pclient->pedict->state.fixangles)
	{
		svs.netinfo.pnet->SVC_MessageBegin(MSG_ONE, svc_setvangles, pclient->pedict);
			svs.netinfo.pnet->WriteFloat(pclient->pedict->state.angles[0]);
			svs.netinfo.pnet->WriteFloat(pclient->pedict->state.angles[1]);
			svs.netinfo.pnet->WriteFloat(pclient->pedict->state.angles[2]);
		svs.netinfo.pnet->SVC_MessageEnd();

		// Check for errors on client
		if(svs.netinfo.pnet->GetClientState(pclient->index) != NETCL_CONNECTED)
			return false;

		pclient->pedict->state.fixangles = false;
	}

	// Add avelocity if needed
	if(pclient->pedict->state.addavelocity)
	{
		svs.netinfo.pnet->SVC_MessageBegin(MSG_ONE, svc_addavelocity, pclient->pedict);
			svs.netinfo.pnet->WriteFloat(pclient->pedict->state.avelocity[0]);
			svs.netinfo.pnet->WriteFloat(pclient->pedict->state.avelocity[1]);
			svs.netinfo.pnet->WriteFloat(pclient->pedict->state.avelocity[2]);
		svs.netinfo.pnet->SVC_MessageEnd();

		// Reset this
		pclient->pedict->state.avelocity.Clear();

		// Check for errors on client
		if(svs.netinfo.pnet->GetClientState(pclient->index) != NETCL_CONNECTED)
			return false;

		pclient->pedict->state.addavelocity = false;
	}

	return true;
}

//=============================================
//
//=============================================
void SV_UpdateClientData( void )
{
	for(Uint32 i = 0; i < svs.maxclients; i++)
	{
		sv_client_t* pclient = &svs.clients[i];
		if(!pclient->connected)
			continue;

		netcl_state_t state = svs.netinfo.pnet->GetClientState(i);
		if(state == NETCL_LOST_CONNECTION)
			continue;

		// Clear up potentially dropped clients
		if(state != NETCL_CONNECTED)
		{
			SV_DropClient(*pclient, svs.netinfo.pnet->GetInfoString(pclient->index));
			continue;
		}

		// Check if connection failed at this time
		if(!SV_SendClientData(pclient))
		{
			SV_DropClient(*pclient, svs.netinfo.pnet->GetInfoString(pclient->index));
			continue;
		}

		// Send any user messages after entities have been sent
		if(pclient->spawned && pclient->originset)
		{
			if(!SV_SendUserMessages(svs.clients[i]))
			{
				SV_DropClient(*pclient, svs.netinfo.pnet->GetInfoString(pclient->index));
				continue;
			}
		}
	}
}

//=============================================
//
//=============================================
bool SV_WriteEntitiesToClient( sv_client_t* pclient )
{
	// Check for errors on client
	if(svs.netinfo.pnet->GetClientState(pclient->index) != NETCL_CONNECTED)
		return false;

	byte *pPVS = nullptr;
	byte *pPAS = nullptr;

	svs.dllfuncs.pfnSetupVisibility(pclient->pedict, pPVS, pPAS);

	// Clear packet entity count
	pclient->packet.numentities = 0;

	// Write other clients first
	for(Uint32 i = 0; i < svs.maxclients; i++)
	{
		sv_client_t* pcl = &svs.clients[i];
		if(!pcl->active || !pcl->spawned)
			continue;

		if(pclient->index == i)
			pclient->packet.cl_packetindex = pclient->packet.numentities;

		// Try adding it to the packet
		entity_state_t& packet_entity = pclient->packet.entities[pclient->packet.numentities];
		if(svs.dllfuncs.pfnAddPacketEntity(packet_entity, pcl->pedict->entindex, (*pcl->pedict), (*pclient->pedict), pPVS))
			pclient->packet.numentities++;
	}

	// Write all other entities
	for(Int32 i = svs.maxclients+1; i < (Int32)gEdicts.GetNbEdicts(); i++)
	{
		if(pclient->packet.numentities >= MAX_VISIBLE_ENTITIES)
		{
			Con_DPrintf("Warning: Exceeded MAX_PACKET_ENTITIES for client %d.\n", pclient->index);
			break;
		}

		edict_t* pedict = gEdicts.GetEdict(i);
		if(pedict->free)
			continue;

		// Try adding it to the packet
		entity_state_t& packet_entity = pclient->packet.entities[pclient->packet.numentities];
		if(svs.dllfuncs.pfnAddPacketEntity(packet_entity, pedict->entindex, (*pedict), (*pclient->pedict), pPVS))
			pclient->packet.numentities++;
	}

	// Write the entities to the client
	svs.netinfo.pnet->SVC_MessageBegin(MSG_ONE, svc_packetentities, pclient->pedict);
	svs.netinfo.pnet->WriteUint32(pclient->packet.numentities);

	for(Uint32 i = 0; i < pclient->packet.numentities; i++)
	{
		entity_state_t& curstate = pclient->packet.entities[i];
		assert(curstate.entindex < (Int32)pclient->packet.cl_entitystates.size());
		entity_state_t& clstate = pclient->packet.cl_entitystates[curstate.entindex];

		edict_t* pedict = gEdicts.GetEdict(curstate.entindex);
		
		// Write the entindex
		svs.netinfo.pnet->WriteInt16(curstate.entindex);
		svs.netinfo.pnet->WriteUint32(pedict->identifier);

		// Determine needed updates
		Uint32 updateMask = 0;
		if(!pclient->packet.cl_wasinpacket[curstate.entindex])
			updateMask |= U_NEW_TO_PACKET;

		if(curstate.origin != clstate.origin) 
			updateMask |= U_ORIGIN;

		if(curstate.angles != clstate.angles) 
			updateMask |= U_ANGLES;

		if(curstate.velocity != clstate.velocity
			|| curstate.fallvelocity != clstate.fallvelocity) 
			updateMask |= U_VELOCITY;

		if(curstate.avelocity != clstate.avelocity) 
			updateMask |= U_AVELOCITY;

		if(curstate.basevelocity != clstate.basevelocity)
			updateMask |= U_BASEVELOCITY;

		if(curstate.punchangles != clstate.punchangles
			|| curstate.punchamount != clstate.punchamount)
			updateMask |= U_PUNCHANGLES;

		if(curstate.viewangles != clstate.viewangles)
			updateMask |= U_VIEWANGLES;

		if(curstate.view_offset != clstate.view_offset)
			updateMask |= U_VIEWOFFSET;

		if(curstate.endpos != clstate.endpos
			|| curstate.startpos != clstate.startpos)
			updateMask |= U_POSITIONS;

		if(curstate.mins != clstate.mins
			|| curstate.maxs != clstate.maxs)
			updateMask |= U_MINSMAXS;

		if((curstate.modelindex != clstate.modelindex
			|| curstate.effects & EF_UPDATEMODEL)
			|| curstate.movetype != clstate.movetype
			|| curstate.solid != clstate.solid
			|| curstate.groupinfo != clstate.groupinfo)
			updateMask |= U_BASICS1;

		if(curstate.skin != clstate.skin
			|| curstate.body != clstate.body
			|| curstate.effects != clstate.effects)
			updateMask |= U_BASICS2;

		if(curstate.gravity != clstate.gravity
			|| curstate.friction != clstate.friction)
			updateMask |= U_BASICS3;

		if(curstate.sequence != clstate.sequence
			|| curstate.frame != clstate.frame
			|| curstate.animtime != clstate.animtime
			|| curstate.framerate != clstate.framerate)
			updateMask |= U_ANIMINFO;

		if(memcmp(curstate.controllers, clstate.controllers, sizeof(Float)*MAX_CONTROLLERS))
			updateMask |= U_CONTROLLERS;

		if(memcmp(curstate.blending, clstate.blending, sizeof(Float)*MAX_BLENDING))
			updateMask |= U_BLENDING;

		if(curstate.scale != clstate.scale
			|| curstate.rendertype != clstate.rendertype
			|| curstate.rendermode != clstate.rendermode
			|| curstate.renderamt != clstate.renderamt
			|| curstate.renderfx != clstate.renderfx
			|| curstate.numsegments != clstate.numsegments
			|| curstate.rendercolor != clstate.rendercolor
			|| curstate.lightorigin != clstate.lightorigin)
			updateMask |= U_RENDERINFO;

		if(curstate.health != clstate.health
			|| curstate.armorvalue != clstate.armorvalue
			|| curstate.frags != clstate.frags
			|| curstate.weapons != clstate.weapons
			|| curstate.buttons != clstate.buttons
			|| curstate.oldbuttons != clstate.oldbuttons
			|| curstate.flags != clstate.flags
			|| curstate.waterlevel != clstate.waterlevel
			|| curstate.fov != clstate.fov)
			updateMask |= U_BASICS4;

		if(curstate.aiment != clstate.aiment
			|| curstate.owner != clstate.owner
			|| curstate.groundent != clstate.groundent)
			updateMask |= U_ENTSINFO;

		if(curstate.stamina != clstate.stamina
			|| curstate.induck != clstate.induck
			|| curstate.ducktime != clstate.ducktime
			|| curstate.waterjumptime != clstate.waterjumptime
			|| curstate.swimtime != clstate.swimtime
			|| curstate.timestepsound != clstate.timestepsound
			|| curstate.stepleft != clstate.stepleft
			|| curstate.planezcap != clstate.planezcap
			|| curstate.weaponanim != clstate.weaponanim)
			updateMask |= U_PLAYERINFO;

		if(curstate.iuser1 != clstate.iuser1
			|| curstate.iuser2 != clstate.iuser2
			|| curstate.iuser3 != clstate.iuser3
			|| curstate.iuser4 != clstate.iuser4
			|| curstate.iuser5 != clstate.iuser5
			|| curstate.iuser6 != clstate.iuser6
			|| curstate.iuser7 != clstate.iuser7
			|| curstate.iuser8 != clstate.iuser8)
			updateMask |= U_IUSER;

		if(curstate.fuser1 != clstate.fuser1
			|| curstate.fuser2 != clstate.fuser2
			|| curstate.fuser3 != clstate.fuser3
			|| curstate.fuser4 != clstate.fuser4)
			updateMask |= U_FUSER;

		if(curstate.vuser1 != clstate.vuser1)
			updateMask |= U_VUSER1;

		if(curstate.vuser2 != clstate.vuser2)
			updateMask |= U_VUSER2;

		if(curstate.vuser3 != clstate.vuser3)
			updateMask |= U_VUSER3;

		if(curstate.vuser4 != clstate.vuser4)
			updateMask |= U_VUSER4;

		if(curstate.parent != clstate.parent
			|| curstate.parentoffset != clstate.parentoffset)
			updateMask |= U_PARENTING;

		// Write the update mask
		svs.netinfo.pnet->WriteUint32(updateMask);

		// Write origin and angles
		if(updateMask & U_ORIGIN)
		{
			for(Uint32 j = 0; j < 3; j++)
				svs.netinfo.pnet->WriteFloat(curstate.origin[j]);
		}

		if(updateMask & U_ANGLES)
		{
			for(Uint32 j = 0; j < 3; j++)
				svs.netinfo.pnet->WriteFloat(curstate.angles[j]);
		}

		// Write velocities
		if(updateMask & U_VELOCITY)
		{
			for(Uint32 j = 0; j < 3; j++)
				svs.netinfo.pnet->WriteFloat(curstate.velocity[j]);

			svs.netinfo.pnet->WriteFloat(curstate.fallvelocity);
		}

		if(updateMask & U_AVELOCITY)
		{
			for(Uint32 j = 0; j < 3; j++)
				svs.netinfo.pnet->WriteFloat(curstate.avelocity[j]);
		}

		if(updateMask & U_BASEVELOCITY)
		{
			for(Uint32 j = 0; j < 3; j++)
				svs.netinfo.pnet->WriteFloat(curstate.basevelocity[j]);
		}

		// Write view related things
		if(updateMask & U_PUNCHANGLES)
		{
			for(Uint32 j = 0; j < 3; j++)
				svs.netinfo.pnet->WriteFloat(curstate.punchangles[j]);

			for(Uint32 j = 0; j < 3; j++)
				svs.netinfo.pnet->WriteFloat(curstate.punchamount[j]);
		}

		if(updateMask & U_VIEWANGLES)
		{
			for(Uint32 j = 0; j < 3; j++)
				svs.netinfo.pnet->WriteFloat(curstate.viewangles[j]);
		}

		if(updateMask & U_VIEWOFFSET)
		{
			for(Uint32 j = 0; j < 3; j++)
				svs.netinfo.pnet->WriteFloat(curstate.view_offset[j]);
		}

		// Write endpos/startpos
		if(updateMask & U_POSITIONS)
		{
			for(Uint32 j = 0; j < 3; j++)
				svs.netinfo.pnet->WriteFloat(curstate.endpos[j]);
			for(Uint32 j = 0; j < 3; j++)
				svs.netinfo.pnet->WriteFloat(curstate.startpos[j]);
		}

		// Write mins/maxs
		if(updateMask & U_MINSMAXS)
		{
			for(Uint32 j = 0; j < 3; j++)
				svs.netinfo.pnet->WriteFloat(curstate.mins[j]);
			for(Uint32 j = 0; j < 3; j++)
				svs.netinfo.pnet->WriteFloat(curstate.maxs[j]);
		}

		if(updateMask & U_BASICS1)
		{
			svs.netinfo.pnet->WriteInt32(curstate.modelindex);
			svs.netinfo.pnet->WriteInt32(curstate.movetype);
			svs.netinfo.pnet->WriteInt32(curstate.solid);
			svs.netinfo.pnet->WriteInt32(curstate.groupinfo);
		}

		if(updateMask & U_BASICS2)
		{
			svs.netinfo.pnet->WriteInt32(curstate.skin);
			svs.netinfo.pnet->WriteInt64(curstate.body);
			svs.netinfo.pnet->WriteInt64(curstate.effects);
		}

		if(updateMask & U_BASICS3)
		{
			svs.netinfo.pnet->WriteFloat(curstate.gravity);
			svs.netinfo.pnet->WriteFloat(curstate.friction);
		}

		if(updateMask & U_ANIMINFO)
		{
			svs.netinfo.pnet->WriteInt32(curstate.sequence);
			svs.netinfo.pnet->WriteFloat(curstate.frame);
			svs.netinfo.pnet->WriteFloat(curstate.animtime);
			svs.netinfo.pnet->WriteFloat(curstate.framerate);
		}

		if(updateMask & U_CONTROLLERS)
		{
			for(Uint32 j = 0; j < MAX_CONTROLLERS; j++)
				svs.netinfo.pnet->WriteFloat(curstate.controllers[j]);
		}

		if(updateMask & U_BLENDING)
		{
			for(Uint32 j = 0; j < MAX_BLENDING; j++)
				svs.netinfo.pnet->WriteFloat(curstate.blending[j]);
		}
		
		if(updateMask & U_RENDERINFO)
		{
			svs.netinfo.pnet->WriteFloat(curstate.scale);
			svs.netinfo.pnet->WriteInt32(curstate.rendertype);
			svs.netinfo.pnet->WriteInt32(curstate.rendermode);
			svs.netinfo.pnet->WriteFloat(curstate.renderamt);
			svs.netinfo.pnet->WriteInt32(curstate.renderfx);
			svs.netinfo.pnet->WriteUint32(curstate.numsegments);

			for(Uint32 j = 0; j < 3; j++)
				svs.netinfo.pnet->WriteFloat(curstate.rendercolor[j]);

			for(Uint32 j = 0; j < 3; j++)
				svs.netinfo.pnet->WriteFloat(curstate.lightorigin[j]);
		}

		if(updateMask & U_BASICS4)
		{
			svs.netinfo.pnet->WriteFloat(curstate.health);
			svs.netinfo.pnet->WriteFloat(curstate.armorvalue);
			svs.netinfo.pnet->WriteInt32(curstate.frags);
			svs.netinfo.pnet->WriteInt64(curstate.weapons);
			svs.netinfo.pnet->WriteInt32(curstate.buttons);
			svs.netinfo.pnet->WriteInt32(curstate.oldbuttons);
			svs.netinfo.pnet->WriteUint64(curstate.flags);
			svs.netinfo.pnet->WriteInt32(curstate.waterlevel);
			svs.netinfo.pnet->WriteFloat(curstate.fov);
		}

		if(updateMask & U_ENTSINFO)
		{
			svs.netinfo.pnet->WriteInt16(curstate.aiment);
			svs.netinfo.pnet->WriteInt16(curstate.owner);
			svs.netinfo.pnet->WriteInt16(curstate.groundent);
		}

		if(updateMask & U_PLAYERINFO)
		{
			svs.netinfo.pnet->WriteFloat(curstate.stamina);
			svs.netinfo.pnet->WriteByte(curstate.induck);
			svs.netinfo.pnet->WriteFloat(curstate.ducktime);
			svs.netinfo.pnet->WriteFloat(curstate.waterjumptime);
			svs.netinfo.pnet->WriteFloat(curstate.swimtime);
			svs.netinfo.pnet->WriteFloat(curstate.timestepsound);
			svs.netinfo.pnet->WriteByte(curstate.stepleft);
			svs.netinfo.pnet->WriteFloat(curstate.planezcap);
			svs.netinfo.pnet->WriteByte(curstate.weaponanim);
		}

		if(updateMask & U_IUSER)
		{
			svs.netinfo.pnet->WriteInt32(curstate.iuser1);
			svs.netinfo.pnet->WriteInt32(curstate.iuser2);
			svs.netinfo.pnet->WriteInt32(curstate.iuser3);
			svs.netinfo.pnet->WriteInt32(curstate.iuser4);
			svs.netinfo.pnet->WriteInt32(curstate.iuser5);
			svs.netinfo.pnet->WriteInt32(curstate.iuser6);
			svs.netinfo.pnet->WriteInt32(curstate.iuser7);
			svs.netinfo.pnet->WriteInt32(curstate.iuser8);
		}

		if(updateMask & U_FUSER)
		{
			svs.netinfo.pnet->WriteFloat(curstate.fuser1);
			svs.netinfo.pnet->WriteFloat(curstate.fuser2);
			svs.netinfo.pnet->WriteFloat(curstate.fuser3);
			svs.netinfo.pnet->WriteFloat(curstate.fuser4);
		}

		if(updateMask & U_VUSER1)
		{
			for(Uint32 j = 0; j < 3; j++)
				svs.netinfo.pnet->WriteFloat(curstate.vuser1[j]);
		}

		if(updateMask & U_VUSER2)
		{
			for(Uint32 j = 0; j < 3; j++)
				svs.netinfo.pnet->WriteFloat(curstate.vuser2[j]);
		}

		if(updateMask & U_VUSER3)
		{
			for(Uint32 j = 0; j < 3; j++)
				svs.netinfo.pnet->WriteFloat(curstate.vuser3[j]);
		}

		if(updateMask & U_VUSER4)
		{
			for(Uint32 j = 0; j < 3; j++)
				svs.netinfo.pnet->WriteFloat(curstate.vuser4[j]);
		}

		if(updateMask & U_PARENTING)
		{
			svs.netinfo.pnet->WriteInt32(curstate.parent);

			for(Uint32 j = 0; j < 3; j++)
				svs.netinfo.pnet->WriteFloat(curstate.parentoffset[j]);
		}

		// Update required fields
		clstate = curstate;
	}

	svs.netinfo.pnet->SVC_MessageEnd();

	// First mark all as not sent
	for(Uint32 i = 0; i < gEdicts.GetNbEdicts(); i++)
		pclient->packet.cl_wasinpacket[i] = false;

	// Then mark ones we did send
	for(Uint32 i = 0; i < pclient->packet.numentities; i++)
	{
		const entity_state_t& curstate = pclient->packet.entities[i];
		pclient->packet.cl_wasinpacket[curstate.entindex] = true;
	}

	// Check for errors on client
	if(svs.netinfo.pnet->GetClientState(pclient->index) != NETCL_CONNECTED)
		return false;
	else
		return true;
}

//=============================================
//
//=============================================
void SV_WriteMoveVars( const sv_client_t& cl )
{
	if(svs.netinfo.pnet->GetClientState(cl.index) != NETCL_CONNECTED)
		return;

	svs.netinfo.pnet->SVC_MessageBegin(MSG_ONE, svc_movevars, cl.pedict);
	{
		svs.netinfo.pnet->WriteFloat(cl.pminfo.movevars.accelerate);
		svs.netinfo.pnet->WriteFloat(cl.pminfo.movevars.airaccelerate);
		svs.netinfo.pnet->WriteFloat(cl.pminfo.movevars.bounce);
		svs.netinfo.pnet->WriteFloat(cl.pminfo.movevars.edgefriction);
		svs.netinfo.pnet->WriteFloat(cl.pminfo.movevars.entgravity);
		svs.netinfo.pnet->WriteFloat(cl.pminfo.movevars.friction);
		svs.netinfo.pnet->WriteFloat(cl.pminfo.movevars.gravity);
		svs.netinfo.pnet->WriteFloat(cl.pminfo.movevars.maxspeed);
		svs.netinfo.pnet->WriteFloat(cl.pminfo.movevars.maxvelocity);
		svs.netinfo.pnet->WriteFloat(cl.pminfo.movevars.stepsize);
		svs.netinfo.pnet->WriteFloat(cl.pminfo.movevars.stopspeed);
		svs.netinfo.pnet->WriteFloat(cl.pminfo.movevars.wateraccelerate);
		svs.netinfo.pnet->WriteFloat(cl.pminfo.movevars.waterfriction);
		svs.netinfo.pnet->WriteFloat(cl.pminfo.movevars.waterdist);
		svs.netinfo.pnet->WriteByte(cl.pminfo.movevars.maxclients);
		svs.netinfo.pnet->WriteByte(cl.pminfo.movevars.holdtoduck);
	}
	svs.netinfo.pnet->SVC_MessageEnd();
}


//=============================================
//
//=============================================
bool SV_SendResourceLists( sv_client_t& cl )
{
	if(svs.phostclient == &cl)
		return true;

	assert(svs.netinfo.pnet != nullptr);
	// Check for connection errors on client
	if(svs.netinfo.pnet->GetClientState(cl.index) != NETCL_CONNECTED)
		return false;

	// Compile model cache list
	CArray<CString> modelsList;
	for(Uint32 i = 0; i < svs.modelcache.size(); i++)
	{
		sv_model_t* pmodel = &svs.modelcache[i];
		if(!pmodel)
			break;

		// Skip brushents
		if(pmodel->modelname[0] == '*')
			continue;

		modelsList.push_back(pmodel->modelname);
	}

	// Send it over
	svs.netinfo.pnet->SVC_MessageBegin(MSG_ONE, svc_resources, cl.pedict);
		svs.netinfo.pnet->WriteByte(CL_RESOURCE_LIST);
		svs.netinfo.pnet->WriteByte(RS_LIST_MODELS);
		svs.netinfo.pnet->WriteUint16(modelsList.size());
		for(Uint32 i = 0; i < modelsList.size(); i++)
			svs.netinfo.pnet->WriteString(modelsList[i].c_str());
	svs.netinfo.pnet->SVC_MessageEnd();

	// Check for connection errors on client
	if(svs.netinfo.pnet->GetClientState(cl.index) != NETCL_CONNECTED)
		return false;

	// Gather textures for the models
	CArray<CString> materialsArray;
	CArray<CString> texturesArray;

	// Gather WAD files
	CArray<CString> wadFilesArray;
	Common::GetWADList(ens.pworld->pentdata, wadFilesArray);

	for(Uint32 i = 0; i < wadFilesArray.size(); i++)
	{
		CString basename;
		Common::Basename(wadFilesArray[i].c_str(), basename);

		CString wadfilepath;
		wadfilepath << TEXTURE_BASE_DIRECTORY_PATH << WORLD_TEXTURES_PATH_BASE << basename << PATH_SLASH_CHAR << wadFilesArray[i];

		if(FL_FileExists(wadfilepath.c_str()))
			svs.genericsourcesarray.push_back(wadfilepath);
	}

	for(Uint32 i = 0; i < modelsList.size(); i++)
		gModelCache.GatherModelResources(modelsList[i].c_str(), svs.mapmaterialfiles, materialsArray, texturesArray);

	// Send material scripts
	svs.netinfo.pnet->SVC_MessageBegin(MSG_ONE, svc_resources, cl.pedict);
		svs.netinfo.pnet->WriteByte(CL_RESOURCE_LIST);
		svs.netinfo.pnet->WriteByte(RS_LIST_MATERIAL_SCRIPTS);
		svs.netinfo.pnet->WriteUint16(materialsArray.size());
		for(Uint32 i = 0; i < materialsArray.size(); i++)
			svs.netinfo.pnet->WriteString(materialsArray[i].c_str());
	svs.netinfo.pnet->SVC_MessageEnd();

	// Send texture scripts
	svs.netinfo.pnet->SVC_MessageBegin(MSG_ONE, svc_resources, cl.pedict);
		svs.netinfo.pnet->WriteByte(CL_RESOURCE_LIST);
		svs.netinfo.pnet->WriteByte(RS_LIST_TEXTURES);
		svs.netinfo.pnet->WriteUint16(texturesArray.size());
		for(Uint32 i = 0; i < texturesArray.size(); i++)
			svs.netinfo.pnet->WriteString(texturesArray[i].c_str());
	svs.netinfo.pnet->SVC_MessageEnd();

	// Send sounds
	svs.netinfo.pnet->SVC_MessageBegin(MSG_ONE, svc_resources, cl.pedict);
		svs.netinfo.pnet->WriteByte(CL_RESOURCE_LIST);
		svs.netinfo.pnet->WriteByte(RS_LIST_SOUNDS);
		svs.netinfo.pnet->WriteUint16(svs.sndcache.size());
		for(Uint32 i = 0; i < svs.sndcache.size(); i++)
		{
			svs.netinfo.pnet->WriteString(svs.sndcache[i].filepath.c_str());
			svs.netinfo.pnet->WriteInt16(svs.sndcache[i].sv_index);
		}
	svs.netinfo.pnet->SVC_MessageEnd();

	// Send generic files
	svs.netinfo.pnet->SVC_MessageBegin(MSG_ONE, svc_resources, cl.pedict);
		svs.netinfo.pnet->WriteByte(CL_RESOURCE_LIST);
		svs.netinfo.pnet->WriteByte(RS_LIST_GENERIC);
		svs.netinfo.pnet->WriteUint16(svs.genericsourcesarray.size());
		for(Uint32 i = 0; i < svs.genericsourcesarray.size(); i++)
			svs.netinfo.pnet->WriteString(svs.genericsourcesarray[i].c_str());
	svs.netinfo.pnet->SVC_MessageEnd();

	// Send particle scripts
	svs.netinfo.pnet->SVC_MessageBegin(MSG_ONE, svc_resources, cl.pedict);
		svs.netinfo.pnet->WriteByte(CL_RESOURCE_LIST);
		svs.netinfo.pnet->WriteByte(RS_LIST_PARTICLE_SCRIPTS);
		svs.netinfo.pnet->WriteUint16(svs.particlescache.size());
		for(Uint32 i = 0; i < svs.particlescache.size(); i++)
			svs.netinfo.pnet->WriteString(svs.particlescache[i].scriptpath.c_str());
	svs.netinfo.pnet->SVC_MessageEnd();

	// Send particle scripts
	svs.netinfo.pnet->SVC_MessageBegin(MSG_ONE, svc_resources, cl.pedict);
		svs.netinfo.pnet->WriteByte(CL_RESOURCE_LIST);
		svs.netinfo.pnet->WriteByte(RS_LIST_DECALS);
		svs.netinfo.pnet->WriteUint16(svs.decalcache.size());
		for(Uint32 i = 0; i < svs.decalcache.size(); i++)
		{
			svs.netinfo.pnet->WriteString(svs.decalcache[i].name.c_str());
			svs.netinfo.pnet->WriteByte(svs.decalcache[i].type);
		}
	svs.netinfo.pnet->SVC_MessageEnd();

	// Request consistency checks for non-hosts
	if(cl.index > 0)
	{
		// Skip worldmodel, it's already added
		for(Uint32 i = 1; i < modelsList.size(); i++)
			SV_AddClientConsistencyCheck(cl, RS_TYPE_MODEL, modelsList[i].c_str());

		for(Uint32 i = 0; i < texturesArray.size(); i++)
			SV_AddClientConsistencyCheck(cl, RS_TYPE_TEXTURE, texturesArray[i].c_str());

		for(Uint32 i = 0; i < materialsArray.size(); i++)
			SV_AddClientConsistencyCheck(cl, RS_TYPE_MATERIAL_SCRIPT, materialsArray[i].c_str());

		for(Uint32 i = 0; i < svs.sndcache.size(); i++)
			SV_AddClientConsistencyCheck(cl, RS_TYPE_SOUND, svs.sndcache[i].filepath.c_str());

		for(Uint32 i = 0; i < svs.genericsourcesarray.size(); i++)
			SV_AddClientConsistencyCheck(cl, RS_TYPE_GENERIC, svs.genericsourcesarray[i].c_str());

		for(Uint32 i = 0; i < svs.particlescache.size(); i++)
			SV_AddClientConsistencyCheck(cl, RS_TYPE_PARTICLE_SCRIPT, svs.particlescache[i].scriptpath.c_str());
	}

	// Close it off for the client
	svs.netinfo.pnet->SVC_MessageBegin(MSG_ONE, svc_resources, cl.pedict);
		svs.netinfo.pnet->WriteByte(CL_RESOURCE_LIST);
		svs.netinfo.pnet->WriteByte(RS_FINISHED);
	svs.netinfo.pnet->SVC_MessageEnd();

	// Check for connection errors on client
	if(svs.netinfo.pnet->GetClientState(cl.index) != NETCL_CONNECTED)
		return false;

	return true;
}

//=============================================
//
//=============================================
Int32 SV_PrecacheSound( const Char* pstrFilepath )
{
	// Do not allow precaching with an inactive game
	if(ens.gamestate == GAME_INACTIVE)
	{
		Con_Printf("%s - Game is not active.\n", __FUNCTION__);
		return NO_PRECACHE;
	}

	// See if it's already present
	for(Uint32 i = 0; i < svs.sndcache.size(); i++)
	{
		if(!qstrcmp(svs.sndcache[i].filepath, pstrFilepath))
			return i;
	}

	// Duration of file
	Float duration = 0;

	// Make sure the file exists
	if(pstrFilepath[0] != '!')
	{
		CString filepath;
		filepath << SOUND_FOLDER_BASE_PATH << pstrFilepath;
		if(!FL_FileExists(filepath.c_str()))
		{
			const byte* phashdata = reinterpret_cast<const byte*>(filepath.c_str());
			if(svs.promptshashlist.addhash(phashdata, filepath.length()))
				Con_Printf("%s - File '%s' does not exist.\n", __FUNCTION__, filepath.c_str());
			return NO_PRECACHE;
		}

		if(qstrstr(pstrFilepath, ".wav") || qstrstr(pstrFilepath, ".WAV"))
		{
			// Get length for WAV files
			duration = SV_GetWAVFileDuration(pstrFilepath);
		}
		else if(qstrstr(pstrFilepath, ".ogg") || qstrstr(pstrFilepath, ".OGG"))
		{
			// Get length for OGG files
			duration = SV_GetOGGFileDuration(pstrFilepath);
		}
		else
		{
			const byte* phashdata = reinterpret_cast<const byte*>(filepath.c_str());
			if(svs.promptshashlist.addhash(phashdata, filepath.length()))
				Con_Printf("%s - %s is not a valid sound file.\n", __FUNCTION__, filepath.c_str());
			return NO_PRECACHE;
		}
	}

	// Add it to the server precache list
	sv_sound_t newSound;
	newSound.filepath = pstrFilepath;
	newSound.sv_index = svs.sndcache.size();
	newSound.duration = duration;

	// Tell local player to precache it
	sv_client_t* pclient = SV_GetHostClient();

	// Tell all connected clients to precache this sound
	svs.netinfo.pnet->SVC_MessageBegin(MSG_ONE, svc_sndengine, pclient->pedict);
		svs.netinfo.pnet->WriteByte(MSG_SNDENGINE_PRECACHE);
		svs.netinfo.pnet->WriteString(pstrFilepath);
		svs.netinfo.pnet->WriteInt16(newSound.sv_index);
	svs.netinfo.pnet->SVC_MessageEnd();

	// Store it and return the index
	svs.sndcache.push_back(newSound);
	return newSound.sv_index;
}

//=============================================
//
//=============================================
bool SV_PrecacheParticleScript( const Char* pstrfilepath, part_script_type_t type )
{
	for(Uint32 i = 0; i < svs.particlescache.size(); i++)
	{
		if(!qstrcmp(pstrfilepath, svs.particlescache[i].scriptpath))
			return true;
	}

	// Check if the file exists
	CString filepath;
	filepath << PARTICLE_SCRIPT_PATH << pstrfilepath;
	if(!FL_FileExists(filepath.c_str()))
	{
		const byte* phashdata = reinterpret_cast<const byte*>(filepath.c_str());
		if(svs.promptshashlist.addhash(phashdata, filepath.length()))
			Con_Printf("%s - File '%s' does not exist.\n", __FUNCTION__, filepath.c_str());
		return false;
	}

	sv_particlecache_t newcache;
	newcache.scriptpath = pstrfilepath;
	newcache.type = type;

	svs.particlescache.push_back(newcache);

	// Tell local player to precache it
	sv_client_t* pclient = SV_GetHostClient();

	// Tell all connected clients to precache this sound
	svs.netinfo.pnet->SVC_MessageBegin(MSG_ONE, svc_precacheparticlescript, pclient->pedict);
		svs.netinfo.pnet->WriteString(pstrfilepath);
		svs.netinfo.pnet->WriteByte(type);
	svs.netinfo.pnet->SVC_MessageEnd();
	return true;
}

//=============================================
//
//=============================================
void SV_PrecacheDecal( const Char* pstrDecalName )
{
	for(Uint32 i = 0; i < svs.decalcache.size(); i++)
	{
		if(!qstrcmp(pstrDecalName, svs.decalcache[i].name)
			&& svs.decalcache[i].type == DECAL_CACHE_SINGLE)
			return;
	}

	decalcache_t newcache;
	newcache.name = pstrDecalName;
	newcache.type = DECAL_CACHE_SINGLE;
	svs.decalcache.push_back(newcache);

	// Tell local player to precache it
	sv_client_t* pclient = SV_GetHostClient();

	// Tell all connected clients to precache this sound
	svs.netinfo.pnet->SVC_MessageBegin(MSG_ONE, svc_precachedecal, pclient->pedict);
		svs.netinfo.pnet->WriteString(pstrDecalName);
		svs.netinfo.pnet->WriteByte(DECAL_CACHE_SINGLE);
	svs.netinfo.pnet->SVC_MessageEnd();
}

//=============================================
//
//=============================================
void SV_PrecacheDecalGroup( const Char* pstrDecalName )
{
	for(Uint32 i = 0; i < svs.decalcache.size(); i++)
	{
		if(!qstrcmp(pstrDecalName, svs.decalcache[i].name)
			&& svs.decalcache[i].type == DECAL_CACHE_GROUP)
			return;
	}

	decalcache_t newcache;
	newcache.name = pstrDecalName;
	newcache.type = DECAL_CACHE_GROUP;
	svs.decalcache.push_back(newcache);

	// Tell local player to precache it
	sv_client_t* pclient = SV_GetHostClient();

	// Tell all connected clients to precache this sound
	svs.netinfo.pnet->SVC_MessageBegin(MSG_ONE, svc_precachedecal, pclient->pedict);
		svs.netinfo.pnet->WriteString(pstrDecalName);
		svs.netinfo.pnet->WriteByte(DECAL_CACHE_GROUP);
	svs.netinfo.pnet->SVC_MessageEnd();
}

//=============================================
//
//=============================================
void SV_PlayEntitySound( entindex_t entindex, const Char* pstrPath, Int32 flags, Int32 channel, Float volume, Float attenuation, Int32 pitch, Float timeoffset, Int32 dest_player )
{
	// Do not allow server sound playback with an inactive game
	if(ens.gamestate == GAME_INACTIVE)
	{
		Con_Printf("%s - Game is not active.\n", __FUNCTION__);
		return;
	}

	// Make sure this is a number if it's a sentence
	if(pstrPath[0] == '!' && !Common::IsNumber(pstrPath + 1))
	{
		Con_EPrintf("%s - Sentence '%s' specified is not a valid sentence index.\n", __FUNCTION__, pstrPath);
		return;
	}

	// Send to one, or all clients, depending on dest_player
	edict_t* pplayer = nullptr;
	if(dest_player != NO_CLIENT_INDEX)
	{
		if(dest_player < 0 || dest_player >= (Int32)svs.maxclients)
		{
			Con_EPrintf("Invalid player entity index %d.\n", dest_player);
			return;
		}

		pplayer = gEdicts.GetEdict(dest_player+1);
		if(pplayer->free)
			return;
	}

	sv_sound_t* psnd = nullptr;
	for(Uint32 i = 0; i < svs.sndcache.size(); i++)
	{
		if(!qstrcmp(svs.sndcache[i].filepath, pstrPath))
		{
			psnd = &svs.sndcache[i];
			break;
		}
	}

	if(!psnd)
	{
		// Precache the sound
		Int32 index = SV_PrecacheSound(pstrPath);
		if(index == NO_PRECACHE)
			return;

		// Notify that this sound was precached late
		Con_Printf("%s - Late precache of '%s'.\n", __FUNCTION__, pstrPath);
		psnd = &svs.sndcache[index];
	}

	// Cap pitch between 0.5 and 5.0
	Int32 _pitch = clamp(pitch, MIN_PITCH, MAX_PITCH);

	// Account for sentences
	Int32 svindex;
	if(pstrPath[0] == '!')
		svindex = -(SDL_atoi(pstrPath+1) + 1);
	else
		svindex = psnd->sv_index;

	if(!pplayer)
		svs.netinfo.pnet->SVC_MessageBegin(MSG_ALL, svc_sndengine, nullptr);
	else
		svs.netinfo.pnet->SVC_MessageBegin(MSG_ONE, svc_sndengine, pplayer);

	svs.netinfo.pnet->WriteByte(MSG_SNDENGINE_EMITENTITYSOUND);
	svs.netinfo.pnet->WriteInt16(svindex);
	svs.netinfo.pnet->WriteInt16(entindex);
	svs.netinfo.pnet->WriteFloat(volume);
	svs.netinfo.pnet->WriteFloat(attenuation);
	svs.netinfo.pnet->WriteByte(channel);
	svs.netinfo.pnet->WriteInt32(flags);
	svs.netinfo.pnet->WriteInt16(_pitch);
	svs.netinfo.pnet->WriteSmallFloat(timeoffset);
	svs.netinfo.pnet->SVC_MessageEnd();
}

//=============================================
//
//=============================================
void SV_PlayAmbientSound( entindex_t entindex, const Char* pstrPath, const Vector& origin, Int32 flags, Float volume, Float attenuation, Int32 pitch, Float timeoffset, Int32 dest_player )
{
	// Do not allow server sound playback with an inactive game
	if(ens.gamestate == GAME_INACTIVE)
	{
		Con_Printf("%s - Game is not active.\n", __FUNCTION__);
		return;
	}

	// Make sure this is a number if it's a sentence
	if(pstrPath[0] == '!' && !Common::IsNumber(pstrPath + 1))
	{
		Con_EPrintf("%s - Sentence '%s' specified is not a valid sentence index.\n", __FUNCTION__, pstrPath);
		return;
	}

	// Send to one, or all clients, depending on dest_player
	edict_t* pplayer = nullptr;
	if(dest_player != NO_CLIENT_INDEX)
	{
		if(dest_player < 0 || dest_player >= (Int32)svs.maxclients)
		{
			Con_EPrintf("Invalid player entity index %d.\n", dest_player);
			return;
		}

		pplayer = gEdicts.GetEdict(dest_player+1);
		if(pplayer->free)
			return;
	}

	sv_sound_t* psnd = nullptr;
	for(Uint32 i = 0; i < svs.sndcache.size(); i++)
	{
		if(!qstrcmp(svs.sndcache[i].filepath, pstrPath))
		{
			psnd = &svs.sndcache[i];
			break;
		}
	}

	if(!psnd)
	{
		// Precache the sound
		Int32 index = SV_PrecacheSound(pstrPath);
		if(index == NO_PRECACHE)
			return;

		// Notify that this sound was precached late
		Con_Printf("%s - Late precache of '%s'.\n", __FUNCTION__, pstrPath);
		psnd = &svs.sndcache[index];
	}

	// Cap pitch between 0.5 and 5.0
	Int32 _pitch = clamp(pitch, MIN_PITCH, MAX_PITCH);

	// Account for sentences
	Int32 svindex;
	if(pstrPath[0] == '!')
		svindex = -(SDL_atoi(pstrPath+1) + 1);
	else
		svindex = psnd->sv_index;

	if(!pplayer)
		svs.netinfo.pnet->SVC_MessageBegin(MSG_ALL, svc_sndengine, nullptr);
	else
		svs.netinfo.pnet->SVC_MessageBegin(MSG_ONE, svc_sndengine, pplayer);

	svs.netinfo.pnet->WriteByte(MSG_SNDENGINE_EMITAMBIENTSOUND);
	svs.netinfo.pnet->WriteInt16(svindex);
	svs.netinfo.pnet->WriteInt16(entindex);
	svs.netinfo.pnet->WriteFloat(volume);
	svs.netinfo.pnet->WriteFloat(attenuation);
	svs.netinfo.pnet->WriteInt32(flags);
	svs.netinfo.pnet->WriteInt16(_pitch);
	svs.netinfo.pnet->WriteFloat(origin[0]);
	svs.netinfo.pnet->WriteFloat(origin[1]);
	svs.netinfo.pnet->WriteFloat(origin[2]);
	svs.netinfo.pnet->WriteSmallFloat(timeoffset);
	svs.netinfo.pnet->SVC_MessageEnd();
}

//=============================================
//
//=============================================
void SV_ApplySoundEffect( entindex_t entindex, const Char* pstrPath, Int32 channel, snd_effects_t effect, Float duration, Float targetvalue, Int32 dest_player )
{
	// Do not allow server sound playback with an inactive game
	if(ens.gamestate == GAME_INACTIVE)
	{
		Con_Printf("%s - Game is not active.\n", __FUNCTION__);
		return;
	}

	// Send to one, or all clients, depending on dest_player
	edict_t* pplayer = nullptr;
	if(dest_player != NO_CLIENT_INDEX)
	{
		if(dest_player < 0 || dest_player >= (Int32)svs.maxclients)
		{
			Con_EPrintf("Invalid player entity index %d.\n", dest_player);
			return;
		}

		pplayer = gEdicts.GetEdict(dest_player+1);
		if(pplayer->free)
			return;
	}

	sv_sound_t* psnd = nullptr;
	for(Uint32 i = 0; i < svs.sndcache.size(); i++)
	{
		if(!qstrcmp(svs.sndcache[i].filepath, pstrPath))
		{
			psnd = &svs.sndcache[i];
			break;
		}
	}

	if(!psnd)
	{
		Con_EPrintf("%s - File '%s' was not precached.\n", __FUNCTION__, pstrPath);
		return;
	}

	if(!pplayer)
		svs.netinfo.pnet->SVC_MessageBegin(MSG_ALL, svc_sndengine, nullptr);
	else
		svs.netinfo.pnet->SVC_MessageBegin(MSG_ONE, svc_sndengine, pplayer);

	svs.netinfo.pnet->WriteByte(MSG_SNDENGINE_APPLY_EFFECT);
	svs.netinfo.pnet->WriteInt16(psnd->sv_index);
	svs.netinfo.pnet->WriteInt16(entindex);
	svs.netinfo.pnet->WriteByte(channel);
	svs.netinfo.pnet->WriteByte(effect);
	svs.netinfo.pnet->WriteSmallFloat(duration);
	svs.netinfo.pnet->WriteFloat(targetvalue);
	svs.netinfo.pnet->SVC_MessageEnd();
}

//=============================================
//
//=============================================
void SV_StopEntitySounds( entindex_t entindex, Int32 channel, Int32 dest_player )
{
	// Do not allow server sound playback with an inactive game
	if(ens.gamestate == GAME_INACTIVE)
	{
		Con_Printf("%s - Game is not active.\n", __FUNCTION__);
		return;
	}

	// Send to one, or all clients, depending on dest_player
	edict_t* pplayer = nullptr;
	if(dest_player != NO_CLIENT_INDEX)
	{
		if(dest_player < 0 || dest_player >= (Int32)svs.maxclients)
		{
			Con_EPrintf("Invalid player entity index %d.\n", dest_player);
			return;
		}

		pplayer = gEdicts.GetEdict(dest_player+1);
		if(pplayer->free)
			return;
	}

	if(!pplayer)
		svs.netinfo.pnet->SVC_MessageBegin(MSG_ALL, svc_sndengine, nullptr);
	else
		svs.netinfo.pnet->SVC_MessageBegin(MSG_ONE, svc_sndengine, pplayer);

	svs.netinfo.pnet->WriteByte(MSG_SNDENGINE_KILL_ENTITY_SOUNDS);
	svs.netinfo.pnet->WriteInt16(entindex);
	svs.netinfo.pnet->WriteByte(channel);
	svs.netinfo.pnet->SVC_MessageEnd();
}

//=============================================
//
//=============================================
void SV_SetRoomType( Int32 roomtype )
{
	svs.netinfo.pnet->SVC_MessageBegin(MSG_ALL, svc_sndengine, nullptr);
	svs.netinfo.pnet->WriteByte(MSG_SNDENGINE_ROOMTYPE);
	svs.netinfo.pnet->WriteByte(roomtype);
	svs.netinfo.pnet->SVC_MessageEnd();
}

//=============================================
//
//=============================================
void SV_MuteAllSounds( bool mutesounds )
{
	svs.netinfo.pnet->SVC_MessageBegin(MSG_ALL, svc_sndengine, nullptr);
	svs.netinfo.pnet->WriteByte(MSG_SNDENGINE_KILL_ALL_SOUNDS);
	svs.netinfo.pnet->WriteByte(mutesounds ? 1 : 0);
	svs.netinfo.pnet->SVC_MessageEnd();
}

//=============================================
//
//=============================================
void SV_PlayMusic( const Char* pstrPath, Int32 channel, Int32 flags, Float timeOffset, Float fadeInTime, Int32 dest_player )
{
	if(channel != MUSIC_CHANNEL_ALL && channel < 0
		|| channel >= NB_MUSIC_CHANNELS && channel != MUSIC_CHANNEL_MENU)
	{
		Con_Printf("%s - Invalid music channel '%d' specified.\n", __FUNCTION__, channel);
		return;
	}
	
	if(channel == MUSIC_CHANNEL_MENU)
	{
		Con_Printf("%s - Music channel '%d' is reserved for menu music.\n", __FUNCTION__, channel);
		return;
	}

	// Do not allow server sound playback with an inactive game
	if(ens.gamestate == GAME_INACTIVE)
	{
		Con_Printf("%s - Game is not active.\n", __FUNCTION__);
		return;
	}

	// Send to one, or all clients, depending on dest_player
	edict_t* pplayer = nullptr;
	if(dest_player != NO_CLIENT_INDEX)
	{
		if(dest_player < 0 || dest_player >= (Int32)(svs.maxclients))
		{
			Con_EPrintf("Invalid player entity index %d.\n", dest_player);
			return;
		}

		pplayer = gEdicts.GetEdict(dest_player+1);
	}

	if(!pplayer)
		svs.netinfo.pnet->SVC_MessageBegin(MSG_ALL, svc_sndengine, nullptr);
	else
		svs.netinfo.pnet->SVC_MessageBegin(MSG_ONE, svc_sndengine, pplayer);

	Int32 _flags = flags;
	if(fadeInTime > 0)
		_flags |= OGG_FL_FADE_IN;

	svs.netinfo.pnet->WriteByte(MSG_SNDENGINE_OGG);
		svs.netinfo.pnet->WriteByte(_flags);
		svs.netinfo.pnet->WriteByte(channel);
		svs.netinfo.pnet->WriteString(pstrPath);
		if(_flags & OGG_FL_FADE_IN)
			svs.netinfo.pnet->WriteFloat(fadeInTime);

		svs.netinfo.pnet->WriteFloat(timeOffset);
	svs.netinfo.pnet->SVC_MessageEnd();
}

//=============================================
//
//=============================================
void SV_StopMusic( Int32 dest_player, const Char* pstrFilename, Int32 channel, Float fadeTime )
{
	if(channel == MUSIC_CHANNEL_MENU)
	{
		Con_Printf("%s - Music channel '%d' is reserved for menu music.\n", __FUNCTION__, channel);
		return;
	}

	if(fadeTime > 0 && (!pstrFilename || !qstrlen(pstrFilename)))
	{
		Con_Printf("%s - Fade time set without a file specified.\n", __FUNCTION__);
		return;
	}

	// Do not allow server sound playback with an inactive game
	if(ens.gamestate == GAME_INACTIVE)
	{
		Con_Printf("%s - Game is not active.\n", __FUNCTION__);
		return;
	}

	// Send to one, or all clients, depending on dest_player
	edict_t* pplayer = nullptr;
	if(dest_player != NO_CLIENT_INDEX)
	{
		if(dest_player < 0 || dest_player >= (Int32)svs.maxclients)
		{
			Con_EPrintf("Invalid player entity index %d.\n", dest_player);
			return;
		}

		pplayer = gEdicts.GetEdict(dest_player+1);
	}

	if(!pplayer)
		svs.netinfo.pnet->SVC_MessageBegin(MSG_ALL, svc_sndengine, nullptr);
	else
		svs.netinfo.pnet->SVC_MessageBegin(MSG_ONE, svc_sndengine, pplayer);

	svs.netinfo.pnet->WriteByte(MSG_SNDENGINE_OGG);
	if(fadeTime)
	{
		svs.netinfo.pnet->WriteByte(OGG_FL_STOP_FADE);
		svs.netinfo.pnet->WriteByte(channel);
		svs.netinfo.pnet->WriteString(pstrFilename);
		svs.netinfo.pnet->WriteFloat(fadeTime);
	}
	else
	{
		svs.netinfo.pnet->WriteByte(OGG_FL_STOP);
		svs.netinfo.pnet->WriteByte(channel);
	}
	svs.netinfo.pnet->SVC_MessageEnd();
}


//=============================================
//
//=============================================
bool SV_CheckFileConsistency( sv_client_t& cl, const Char* pstrFilename, const char* pstrHash )
{
	// Remove it from the list of files to be checked
	cl.consistencylist.remove(pstrFilename);

	// Load the file in and get the MD5 hash
	Uint32 filesize = 0;
	const byte* pfile = FL_LoadFile(pstrFilename, &filesize);
	if(!pfile)
	{
		Con_Printf("%s - File %s does not exist.\n", __FUNCTION__, pstrFilename);
		return false;
	}

	// Generate MD5 hash and compare
	CString hash = CMD5(pfile, filesize).HexDigest();
	FL_FreeFile(pfile);
	if(!qstrcmp(hash, pstrHash))
		return true;

#ifndef _DEBUG
	// See if it's in the enforced list
	for(Uint32 i = 0; i < svs.netinfo.enforcedfiles.size(); i++)
	{
		if(!qstrcmp(svs.netinfo.enforcedfiles[i], pstrFilename))
			return false;
	}
#endif

	// Let the server decide
	return svs.dllfuncs.pfnInconsistentFile(pstrFilename);
}

//=============================================
//
//=============================================
void SV_AddClientConsistencyCheck( sv_client_t& cl, rs_type_t type, const Char* pstrFilename )
{
	CString filepath;
	switch(type)
	{
	case RS_TYPE_MATERIAL_SCRIPT:
	case RS_TYPE_TEXTURE:
		filepath << TEXTURE_BASE_DIRECTORY_PATH << pstrFilename;
		break;
	case RS_TYPE_PARTICLE_SCRIPT:
		filepath << PARTICLE_SCRIPT_PATH << pstrFilename;
		break;
	case RS_TYPE_SOUND:
		filepath << SOUND_FOLDER_BASE_PATH << pstrFilename;
		break;
	case RS_TYPE_MODEL:
	case RS_TYPE_GENERIC:
	default:
		filepath = pstrFilename;
		break;
	}

	// Check if it's already present
	cl.consistencylist.begin();
	while(!cl.consistencylist.end())
	{
		if(cl.consistencylist.get() == filepath)
			return;

		cl.consistencylist.next();
	}

	// Add it to the list
	cl.consistencylist.add(filepath);
}

//=============================================
//
//=============================================
void SV_RequestClientConsistencyChecks( sv_client_t& cl )
{
	if(cl.consistencylist.empty())
	{
		Con_Printf("%s - No files to check.\n", __FUNCTION__);
		return;
	}

	cl.consistencylist.begin();
	while(!cl.consistencylist.end())
	{
		// Send the message
		CString& strFilename = cl.consistencylist.get();
		svs.netinfo.pnet->SVC_MessageBegin(MSG_ONE, svc_consistency, cl.pedict);
			svs.netinfo.pnet->WriteString(strFilename.c_str());
		svs.netinfo.pnet->SVC_MessageEnd();

		cl.consistencylist.next();
	}
}

//=============================================
//
//=============================================
void SV_ReadUserCmd( sv_client_t& cl )
{
	CMSGReader& reader = svs.netinfo.reader;

	// Increase buffer if needed
	if(cl.numusercmd == cl.usercmdarray.size())
		cl.usercmdarray.resize(cl.usercmdarray.size()+USERCMD_ALLOC_SIZE);

	usercmd_t& newcmd = cl.usercmdarray[cl.numusercmd];
	cl.numusercmd++;

	newcmd.cmdidx = reader.ReadUint64();

	newcmd.lerp_msec = reader.ReadInt32();
	newcmd.msec = reader.ReadByte();

	// Read view angles
	for(Uint32 i = 0; i < 3; i++)
		newcmd.viewangles[i] = reader.ReadFloat();

	newcmd.forwardmove = reader.ReadFloat();
	newcmd.sidemove = reader.ReadFloat();
	newcmd.upmove = reader.ReadFloat();
	newcmd.weaponselect = reader.ReadByte();
	newcmd.impulse = reader.ReadByte();
	newcmd.buttons = reader.ReadUint32();

	if(reader.HasError())
		Con_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
}

//=============================================
//
//=============================================
bool SV_ReadClientInfo( sv_client_t& cl )
{
	CMSGReader& reader = svs.netinfo.reader;

	// Only contains the client name for now
	const Char* pstrName = reader.ReadString();

	if(reader.HasError())
	{
		Con_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	// Give the server the chance to reject this client
	if(cl.index > 0 && !SV_VerifyClient(cl, pstrName))
		return false;

	if(svs.maxclients > 1)
	{
		CString message;
		message << pstrName << " is joining the game.\n";

		Uint32 sayMsgId = SV_FindUserMessageByName("SayText");
		if(sayMsgId != 0)
		{
			CString netname;
			netname << "Server notice: ";

			SV_UserMessageBegin(MSG_ALL, sayMsgId, nullptr, nullptr);
				SV_Msg_WriteString(netname.c_str());
				SV_Msg_WriteString(message.c_str());
			SV_UserMessageEnd();
		}

		SV_ClientPrintf(nullptr, message.c_str());
	}

	// Send the resource list
	if(!SV_SendResourceLists(cl))
		return false;

	if(cl.index > 0)
	{
		// Add the enforced files
		for(Uint32 i = 0; i < svs.netinfo.enforcedfiles.size(); i++)
			SV_AddClientConsistencyCheck(cl, RS_TYPE_GENERIC, svs.netinfo.enforcedfiles[i].c_str());

		// Send requests for consistency checks on non-hosts
		SV_RequestClientConsistencyChecks(cl);
		// Register usermsgs
		SV_ClientRegisterUserMessages(cl);
	}

	if(cl.active)
	{
		// A timed-out client might be trying to reconnect, so we need to clear this whenever sv_clientinfo is sent
		SV_ClearClient(cl);
		SV_PrepareClient(cl.index);
	}

	// Set player name after potential clear
	cl.pedict->fields.netname = SV_AllocString(pstrName);

	return true;
}

//=============================================
//
//=============================================
bool SV_ValidateClientCommand( const Char* pstrCmd )
{
	static Char token[MAX_PARSE_LENGTH];
	Common::Parse(pstrCmd, token);

	if(gCommands.GetCommandFlags(token) & CMD_FL_CL_RELEVANT)
		return true;
	else
		return false;
}

//=============================================
//
//=============================================
void SV_ProcessStringCommand( sv_client_t& cl, const Char* pstrCmd )
{
	if(!cl.connected)
		return;

	// Do not allow levelchange commands at this stage
	ens.isinprocesstringcommand = true;

	// Set the invoker's index
	gCommands.SetInvokerPlayerIndex(cl.index);

	// Check if it's a client-executed command
	if(SV_ValidateClientCommand(pstrCmd))
	{
		// This is a client-state setting command
		if(!gCommands.ExecuteCommand(pstrCmd, true))
			Con_Printf("Unknown command: '%s'.\n", pstrCmd);
	}
	else 
	{
		// Check if it can be executed as a server command
		if(!gCommands.ExecuteCommand(pstrCmd, true))
		{
			// See if game dll can execute
			if(!svs.dllfuncs.pfnClientCommand(cl.pedict))
				Con_Printf("Unknown command: '%s'.\n", pstrCmd);
		}
	}

	// Do not allow levelchange commands at this stage
	ens.isinprocesstringcommand = false;
}

//=============================================
//
//=============================================
void SV_ChunkSizeCvarCallBack( CCVar* pCVar )
{
	Int32 maxchunksize = pCVar->GetValue();
	if(maxchunksize < MIN_FILECHUNK_SIZE)
	{
		// Do not allow for bogus value
		Con_Printf("%s - The value of '%s'(%d) is too low, minimum is %d.\n", __FUNCTION__, pCVar->GetName(), maxchunksize, MIN_FILECHUNK_SIZE);
		gConsole.CVarSetFloatValue(pCVar->GetName(), MIN_FILECHUNK_SIZE);
	}
	else if(maxchunksize > MAX_FILECHUNK_SIZE)
	{
		// Do not allow for bogus value
		Con_Printf("%s - The value of '%s'(%d) is too high, maximum is %d.\n", __FUNCTION__, pCVar->GetName(), maxchunksize, MAX_FILECHUNK_SIZE);
		gConsole.CVarSetFloatValue(pCVar->GetName(), MAX_FILECHUNK_SIZE);
	}
}

//=============================================
//
//=============================================
void SV_ClearUpload( sv_client_t& cl )
{
	if(!cl.upload.chunkslist.empty())
	{
		cl.upload.chunkslist.begin();
		while(!cl.upload.chunkslist.end())
		{
			byte* pdata = reinterpret_cast<byte*>(cl.upload.chunkslist.get());
			delete[] pdata;

			cl.upload.chunkslist.next();
		}

		cl.upload.chunkslist.clear();
	}

	// Clear string too
	cl.upload.filepath.clear();
	cl.upload.fileid = 0;
}

//=============================================
//
//=============================================
bool SV_UploadNextChunk( sv_client_t& cl )
{
	// Check for errors
	if(cl.upload.chunkslist.empty())
	{
		Con_EPrintf("%s - Called with no chunks remaining to upload.\n", __FUNCTION__);
		return false;
	}

	// Retrieve the next chunk
	cl.upload.chunkslist.begin();
	CLinkedList<filechunk_t*>::link_t *plink = cl.upload.chunkslist.get_link();
	
	filechunk_t* pchunk = plink->_val;
	byte *pdatasrc = reinterpret_cast<byte*>(pchunk) + pchunk->dataoffset;

	// Send this chunk to the client
	svs.netinfo.pnet->SVC_MessageBegin(MSG_ONE, svc_resources, cl.pedict);
		svs.netinfo.pnet->WriteByte(CL_RESOURCE_FILECHUNK);
		svs.netinfo.pnet->WriteUint16(cl.upload.fileid);
		svs.netinfo.pnet->WriteUint32(pchunk->chunkindex);
		svs.netinfo.pnet->WriteUint32(pchunk->datasize);
		svs.netinfo.pnet->WriteBuffer(pdatasrc, pchunk->datasize);
	svs.netinfo.pnet->SVC_MessageEnd();

	// Remove chunk from list
	delete[] plink->_val;
	cl.upload.chunkslist.remove(plink);

	return true;
}

//=============================================
//
//=============================================
bool SV_PrepareUpload( sv_client_t& cl )
{
	CMSGReader& reader = svs.netinfo.reader;

	// Clear any previous uploads
	SV_ClearUpload(cl);

	if(g_psv_allowdownload->GetValue() < 1)
	{
		SV_DropClient(cl, "Server doesn't allow for downloads.\n");
		return false;
	}

	// Split it into parts
	Uint32 maxchunksize = g_psv_chunksize->GetValue();

	// Read in file path and file id
	const Char* pstrFilePath = reader.ReadString();
	Uint32 fileid = reader.ReadUint16();

	if(reader.HasError())
	{
		Con_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
		return false;
	}

	// Load the file in and split it into parts
	Uint32 filesize = 0;
	const byte* pfile = FL_LoadFile(pstrFilePath, &filesize);
	if(!pfile)
	{
		Con_EPrintf("%s - Failed to load file '%s'.\n", __FUNCTION__, pstrFilePath);
		return false;
	}

	// Set file info
	cl.upload.fileid = fileid;
	cl.upload.filepath = pstrFilePath;

	// Split into part, or send as one, depending on size
	if(filesize <= maxchunksize)
	{
		// It'll be a single chunk
		Uint32 allocsize = sizeof(filechunk_t)+filesize;
		byte* pchunk = new byte[allocsize];

		filechunk_t* pheader = reinterpret_cast<filechunk_t*>(pchunk);
		pheader->chunkindex = 0;
		pheader->fileid = fileid;
		pheader->dataoffset = sizeof(filechunk_t);
		pheader->datasize = filesize;

		byte* pdatadest = pchunk + sizeof(filechunk_t);
		memcpy(pdatadest, pfile, sizeof(byte)*filesize);

		// Add to the list
		cl.upload.chunkslist.radd(reinterpret_cast<filechunk_t*>(pchunk));
	}
	else
	{
		// Split it into many chunks
		Uint32 fileoffset = 0;
		while(fileoffset < filesize)
		{
			Uint32 chunksize = filesize - fileoffset;
			if(chunksize > maxchunksize)
				chunksize = maxchunksize;

			Uint32 allocsize = sizeof(filechunk_t)+chunksize;
			byte* pchunk = new byte[allocsize];

			filechunk_t* pheader = reinterpret_cast<filechunk_t*>(pchunk);
			pheader->chunkindex = cl.upload.chunkslist.size();
			pheader->fileid = fileid;
			pheader->dataoffset = sizeof(filechunk_t);
			pheader->datasize = chunksize;

			const byte *pdatasrc = pfile + fileoffset;
			byte* pdatadest = pchunk + sizeof(filechunk_t);
			memcpy(pdatadest, pdatasrc, sizeof(byte)*chunksize);

			// Add to the list
			cl.upload.chunkslist.radd(reinterpret_cast<filechunk_t*>(pchunk));
			fileoffset += chunksize;
		}
	}

	FL_FreeFile(pfile);

	Con_Printf("Uploading '%s' to client.\n", pstrFilePath);

	// Send file info to client
	svs.netinfo.pnet->SVC_MessageBegin(MSG_ONE, svc_resources, cl.pedict);
		svs.netinfo.pnet->WriteByte(CL_RESOURCE_FILEINFO);
		svs.netinfo.pnet->WriteString(pstrFilePath);
		svs.netinfo.pnet->WriteUint16(cl.upload.fileid);
		svs.netinfo.pnet->WriteInt32(cl.upload.chunkslist.size());
	svs.netinfo.pnet->SVC_MessageEnd();

	// Begin uploading chunks to client
	return SV_UploadNextChunk(cl);
}

//=============================================
//
//=============================================
bool SV_ReadResourceMessage( sv_client_t& cl )
{
	CMSGReader& reader = svs.netinfo.reader;

	Int32 type = reader.ReadByte();
	switch(type)
	{
	case SV_RESOURCES_OK:
		SV_ClearUpload(cl);
		break;
	case SV_RESOURCE_DOWNLOAD_BEGIN:
		return SV_PrepareUpload(cl);
		break;
	case SV_RESOURCE_CHUNK_RECEIVED:
		return SV_UploadNextChunk(cl);
		break;
	}

	return true;
}

//=============================================
//
//=============================================
sv_clstate_t SV_ReadClientMessages( sv_client_t& cl )
{
	netcl_state_t state = svs.netinfo.pnet->GetClientState(cl.index);
	switch(state)
	{
	case NETCL_LOST_CONNECTION:
		return SVCL_LOST_CONNECTION;
		break;
	case NETCL_NET_ERROR:
		return SVCL_NET_ERROR;
		break;
	case NETCL_DISCONNECTED:
		return SVCL_DISCONNECTED;
		break;
	}

	Uint32 msgsize = 0;
	byte* pmsgdata = nullptr;
	while(svs.netinfo.pnet->CLS_GetMessage(cl.index, pmsgdata, msgsize))
	{
		// Begin message reading
		CMSGReader& reader = svs.netinfo.reader;
		reader.BeginRead(pmsgdata, msgsize);

		byte type = reader.ReadByte();
		switch(type)
		{
		case cls_nop:
			break;
		case cls_cmdstring:
			SV_ProcessStringCommand(cl, reader.ReadString());
			break;
		case cls_disconnect:
			return SVCL_DISCONNECTED;
			break;
		case cls_cvarvalue:
			break;
		case cls_usercmd:
			SV_ReadUserCmd(cl);
			break;
		case cls_registerusermsg:
			SV_RegisterClientUserMessage();
			break;
		case cls_usermsg:
			SV_ReadClientUserMessage(cl);
			break;
		case cls_pausegame:
			SV_ReadPauseMessage(cl);
			break;
		case cls_clientinfo:
			{
				if(!SV_ReadClientInfo(cl))
					return SVCL_REJECTED;
			}
			break;
		case cls_heartbeat:
			{
				bool prompt = (reader.ReadByte() > 0) ? true : false;

				if(reader.HasError())
				{
					Con_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
					return SVCL_CLS_BAD;
				}

				// Tell the client we're still here
				svs.netinfo.pnet->SVC_MessageBegin(MSG_ONE, svc_heartbeat, cl.pedict);
				svs.netinfo.pnet->WriteByte(prompt);
				svs.netinfo.pnet->SVC_MessageEnd();
			}
			break;
		case cls_resources:
			{
				if(!SV_ReadResourceMessage(cl))
				{
					if(!cl.active)
					{
						// Fail if we're loading resources
						return SVCL_RESOURCES_ERROR;
					}
					else
					{
						// Tell the client the file can't be found
						svs.netinfo.pnet->SVC_MessageBegin(MSG_ONE, svc_resources, cl.pedict);
							svs.netinfo.pnet->WriteByte(CL_RESOURCE_UNAVAILABLE);
						svs.netinfo.pnet->SVC_MessageEnd();
					}
				}
			}
			break;
		case cls_clientready:
			{
				// Do not allow clients to spawn if they didn't pass consistency checks
				if(!cl.consistencylist.empty())
					return SVCL_NOT_CONSISTENT;

				// See if the client can spawn
				if(!SV_SpawnClient(cl))
					return SVCL_SPAWN_FAILED;

				// Reset this
				svs.saverestore = false;
			}
			break;
		case cls_consistency:
			{
				const Char* pstrFilename = reader.ReadString();
				const Char* pstrHash = reader.ReadString();

				if(reader.HasError())
				{
					Con_Printf("%s - Error reading message: %s.\n", __FUNCTION__, reader.GetError());
					return SVCL_CLS_BAD;
				}

				if(!SV_CheckFileConsistency(cl, pstrFilename, pstrHash))
				{
					SV_ClientPrintf(cl.pedict, "File '%s' differs from server's.\n", pstrFilename); 
					return SVCL_INCONSISTENT_FILE;
				}
			}
			break;
		case cls_bad:
		default:
			Con_EPrintf("%s: cls_bad.\n", __FUNCTION__);
			return SVCL_CLS_BAD;
			break;
		}
	}

	return SVCL_OK;
}

//=============================================
//
//=============================================
void SV_ClientPrintf( const edict_t* pclient, const Char *fmt, ... )
{
	// Write the message
	msgdest_t dest = (!pclient) ? MSG_ALL : MSG_ONE;
	if(pclient)
	{
		// Make sure it's valid
		if(!(pclient->state.flags & FL_CLIENT))
		{
			Con_Printf("%s - not a client.\n", __FUNCTION__);
			return;
		}

		if(svs.netinfo.pnet->GetClientState(pclient->entindex-1) != NETCL_CONNECTED)
			return;
	}

	va_list	vArgPtr;
	Char cMsg[MAX_PARSE_LENGTH];
	
	va_start(vArgPtr,fmt);
	vsprintf_s(cMsg, fmt, vArgPtr);
	va_end(vArgPtr);

	// Tell the client(s)
	svs.netinfo.pnet->SVC_MessageBegin(dest, svc_print, pclient);
		svs.netinfo.pnet->WriteString(cMsg);
	svs.netinfo.pnet->SVC_MessageEnd();
}

//=============================================
//
//=============================================
void SV_RegisterClientUserMessage( void )
{
	CMSGReader& reader = svs.netinfo.reader;

	// Read message info
	Uint32 msgid = reader.ReadByte();
	const Char* pstrMsgName = reader.ReadString();

	// Make sure we don't add it twice, but don't warn
	// Multiple clients connecting will send this msg
	for(Uint32 i = 0; i < svs.netinfo.usermsgfunctions.size(); i++)
	{
		if(!qstrcmp(svs.netinfo.usermsgfunctions[i].name, pstrMsgName))
			return;
	}

	// See if this message is already registered
	Int32 index = msgid - 1;
	if(index < (Int32)svs.netinfo.usermsgfunctions.size())
	{
		sv_usermsgfunction_t& msg = svs.netinfo.usermsgfunctions[index];
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
	pfnSVUserMsg_t pFunction = reinterpret_cast<pfnSVUserMsg_t>(SDL_LoadFunction(svs.pdllhandle, usermsgfuncname.c_str()));
	if(!pFunction)
	{
		// Add the entry anyway, but warn the client
		Con_EPrintf("%s - Function '%s' not found on server.\n", __FUNCTION__, usermsgfuncname.c_str());
	}

	// Create the entry
	svs.netinfo.usermsgfunctions.resize(index+1);
	sv_usermsgfunction_t& msg = svs.netinfo.usermsgfunctions[index];

	msg.id = msgid;
	msg.name = pstrMsgName;
	msg.pfnReadMsg = pFunction;
}

//=============================================
//
//=============================================
void SV_ReadPauseMessage( sv_client_t& cl )
{
	if(svs.maxclients > 1)
		return;

	CMSGReader& reader = svs.netinfo.reader;

	bool pauseOverride = false;
	bool isPaused = (reader.ReadByte() == 1) ? true : false;
	if(isPaused)
		pauseOverride = (reader.ReadByte() == 1);

	svs.pauseovveride = pauseOverride;
	Sys_SetPaused(isPaused, false);
}

//=============================================
//
//=============================================
void SV_ReadClientUserMessage( sv_client_t& cl )
{
	CMSGReader& reader = svs.netinfo.reader;

	// Read in the ID
	Int32 msgid = reader.ReadByte();
	Int32 msgindex = msgid - 1;

	// Look it up
	if((Int32)svs.netinfo.usermsgfunctions.size() <= msgindex)
	{
		Con_EPrintf("%s - Message with bogus id %d received.\n", __FUNCTION__, msgid);
		return;
	}

	sv_usermsgfunction_t& msg = svs.netinfo.usermsgfunctions[msgindex];
	if(!msg.pfnReadMsg)
	{
		Con_EPrintf("%s - Message '%s' function not found on client.\n", __FUNCTION__, msg.name.c_str());
		return;
	}

	Uint16 msgsize = reader.ReadUint16();
	const byte* pdata = reader.ReadBuffer(msgsize);

	// Call client to read the message contents
	if(!msg.pfnReadMsg(cl.pedict, msg.name.c_str(), pdata, msgsize))
		Con_Printf("%s - Client returned error on message '%s' read.\n", __FUNCTION__, msg.name.c_str());
}

//=============================================
//
//=============================================
Int32 SV_RegisterUserMessage( const Char* pstrMsgName, Int32 msgsize )
{
	return UserMSG_RegisterUserMessage(svs.netinfo.usermsgs, pstrMsgName, msgsize);
}

//=============================================
//
//=============================================
void SV_UserMessageBegin( msgdest_t dest, Int32 msgid, const Vector* porigin, const edict_t* pedict )
{
	if(svs.netinfo.pcurrentmsg)
	{
		delete svs.netinfo.pcurrentmsg;
		svs.netinfo.pcurrentmsg = nullptr;
	}

	if(dest == MSG_ONE || dest == MSG_ONE_UNRELIABLE)
	{
		if(!pedict)
		{
			Con_Printf("%s - Destination client was null.\n", __FUNCTION__);
			return;
		}

		if(pedict->clientindex == NO_CLIENT_INDEX || pedict->clientindex < 0 || pedict->clientindex >= (Int32)svs.maxclients)
		{
			Con_Printf("%s - Bogus destination client.\n", __FUNCTION__);
			return;
		}

		if(!(pedict->state.flags & FL_CLIENT))
		{
			Con_Printf("%s - Not a client.\n", __FUNCTION__);
			return;
		}
	}

	svs.netinfo.pcurrentmsg = new usermsgdata_t;
	UserMSG_UserMessageBegin(svs.netinfo.usermsgs, (*svs.netinfo.pcurrentmsg), dest, msgid, porigin, pedict);
}

//=============================================
//
//=============================================
void SV_UserMessageEnd( void )
{
	if(!svs.netinfo.pcurrentmsg || svs.netinfo.pcurrentmsg->pusermsg == nullptr)
	{
		if(svs.netinfo.pcurrentmsg)
		{
			delete svs.netinfo.pcurrentmsg;
			svs.netinfo.pcurrentmsg = nullptr;
		}

		Con_EPrintf("%s - Invalid message send attempt.\n", __FUNCTION__);
		return;
	}

	// get data ptr
	usermsgdata_t* pmsgdata = svs.netinfo.pcurrentmsg;
	svs.netinfo.pcurrentmsg = nullptr;

	if(pmsgdata->pusermsg->bufsize != -1 && pmsgdata->msgsize != (Int32)pmsgdata->bufsize)
	{
		// Notify client about buffer size mismatch
		Con_Printf("%s - User message '%s' mismatch with fixed size %d - %d bytes written.\n", 
			__FUNCTION__, pmsgdata->pusermsg->name.c_str(), pmsgdata->msgsize);
	}

	if(pmsgdata->msgsize <= 0)
	{
		Con_EPrintf("%s - Bogus message size %d for message '%s'.\n", __FUNCTION__, pmsgdata->msgsize, pmsgdata->pusermsg->name.c_str());
		delete pmsgdata;
		return;
	}

	// Retreive PVS data for this origin
	const byte* ppvsdata = nullptr;
	if(pmsgdata->originset)
		ppvsdata = SV_SetPVS(pmsgdata->pvsorigin);

	// Test for each client if they're in the PVS
	for(Uint32 i = 0; i < svs.maxclients; i++)
	{
		if(!svs.clients[i].connected)
			continue;

		edict_t* pedict = svs.clients[i].pedict;
		if(!pedict)
			continue;

		// Check visibility
		if(pmsgdata->originset && ppvsdata)
		{
			if(!Common::CheckVisibility(pedict->leafnums, ppvsdata))
				continue;
		}

		// Only send to destination client
		if((pmsgdata->destination == MSG_ONE || pmsgdata->destination == MSG_ONE_UNRELIABLE) && pmsgdata->pedict != pedict)
			continue;

		// Add to client's list of messages
		svs.clients[i].cachedmsglist.radd(new usermsgdata_t(*pmsgdata));
	}

	// Delete it
	delete pmsgdata;
}

//=============================================
//
//=============================================
bool SV_SendUserMessages( sv_client_t& cl )
{
	if(cl.cachedmsglist.empty())
		return true;

	// Send all the cached-up messages
	cl.cachedmsglist.begin();
	while(!cl.cachedmsglist.end())
	{
		usermsgdata_t* pmsgdata = cl.cachedmsglist.get();

		// write the data
		svs.netinfo.pnet->SVC_MessageBegin(MSG_ONE, svc_usermsg, cl.pedict);
			svs.netinfo.pnet->WriteByte(pmsgdata->pusermsg->id);
			svs.netinfo.pnet->WriteUint16(pmsgdata->msgsize);
			svs.netinfo.pnet->WriteBuffer(pmsgdata->pmsgbuffer, pmsgdata->msgsize);
		svs.netinfo.pnet->SVC_MessageEnd();

		// Delete and go to next
		delete pmsgdata;
		cl.cachedmsglist.next();
	}

	cl.cachedmsglist.clear();

	// Check for errors on client
	if(svs.netinfo.pnet->GetClientState(cl.index) != NETCL_CONNECTED)
		return false;

	return true;
}

//=============================================
//
//=============================================
void SV_ClientRegisterUserMessages( const sv_client_t& cl )
{
	// Make sure client is valid
	if(!cl.connected)
	{
		Con_EPrintf("%s - Called on inactive client.\n", __FUNCTION__);
		return;
	}

	for(Uint32 i = 0; i < svs.netinfo.usermsgs.size(); i++)
	{
		usermsg_t& msg = svs.netinfo.usermsgs[i];

		// write the data
		svs.netinfo.pnet->SVC_MessageBegin(MSG_ONE, svc_registerusermsg, cl.pedict);
			svs.netinfo.pnet->WriteByte(msg.id);
			svs.netinfo.pnet->WriteString(msg.name.c_str());
		svs.netinfo.pnet->SVC_MessageEnd();
	}
}

//=============================================
//
//=============================================
void SV_Msg_WriteByte( byte value )
{
	if(!svs.netinfo.pcurrentmsg)
		return;

	UserMSG_Msg_WriteByte((*svs.netinfo.pcurrentmsg), value);
}

//=============================================
//
//=============================================
void SV_Msg_WriteChar( Char value )
{
	if(!svs.netinfo.pcurrentmsg)
	{
		Con_Printf("%s - Called with no valid message defined.\n", __FUNCTION__);
		return;
	}

	UserMSG_Msg_WriteChar((*svs.netinfo.pcurrentmsg), value);
}

//=============================================
//
//=============================================
void SV_Msg_WriteInt16( Int16 value )
{
	if(!svs.netinfo.pcurrentmsg)
	{
		Con_Printf("%s - Called with no valid message defined.\n", __FUNCTION__);
		return;
	}

	UserMSG_Msg_WriteInt16((*svs.netinfo.pcurrentmsg), value);
}

//=============================================
//
//=============================================
void SV_Msg_WriteUint16( Uint16 value )
{
	if(!svs.netinfo.pcurrentmsg)
	{
		Con_Printf("%s - Called with no valid message defined.\n", __FUNCTION__);
		return;
	}

	UserMSG_Msg_WriteUint16((*svs.netinfo.pcurrentmsg), value);
}

//=============================================
//
//=============================================
void SV_Msg_WriteInt32( Int32 value )
{
	if(!svs.netinfo.pcurrentmsg)
	{
		Con_Printf("%s - Called with no valid message defined.\n", __FUNCTION__);
		return;
	}

	UserMSG_Msg_WriteInt32((*svs.netinfo.pcurrentmsg), value);
}

//=============================================
//
//=============================================
void SV_Msg_WriteUint32( Uint32 value )
{
	if(!svs.netinfo.pcurrentmsg)
	{
		Con_Printf("%s - Called with no valid message defined.\n", __FUNCTION__);
		return;
	}

	UserMSG_Msg_WriteUint32((*svs.netinfo.pcurrentmsg), value);
}

//=============================================
//
//=============================================
void SV_Msg_WriteInt64( Int64 value )
{
	if(!svs.netinfo.pcurrentmsg)
	{
		Con_Printf("%s - Called with no valid message defined.\n", __FUNCTION__);
		return;
	}

	UserMSG_Msg_WriteInt64((*svs.netinfo.pcurrentmsg), value);
}

//=============================================
//
//=============================================
void SV_Msg_WriteUint64( Uint64 value )
{
	if(!svs.netinfo.pcurrentmsg)
	{
		Con_Printf("%s - Called with no valid message defined.\n", __FUNCTION__);
		return;
	}

	UserMSG_Msg_WriteUint64((*svs.netinfo.pcurrentmsg), value);
}

//=============================================
//
//=============================================
void SV_Msg_WriteSmallFloat( Float value )
{
	if(!svs.netinfo.pcurrentmsg)
	{
		Con_Printf("%s - Called with no valid message defined.\n", __FUNCTION__);
		return;
	}

	UserMSG_Msg_WriteSmallFloat((*svs.netinfo.pcurrentmsg), value);
}

//=============================================
//
//=============================================
void SV_Msg_WriteFloat( Float value )
{
	if(!svs.netinfo.pcurrentmsg)
	{
		Con_Printf("%s - Called with no valid message defined.\n", __FUNCTION__);
		return;
	}

	UserMSG_Msg_WriteFloat((*svs.netinfo.pcurrentmsg), value);
}

//=============================================
//
//=============================================
void SV_Msg_WriteDouble( Double value )
{
	if(!svs.netinfo.pcurrentmsg)
	{
		Con_Printf("%s - Called with no valid message defined.\n", __FUNCTION__);
		return;
	}

	UserMSG_Msg_WriteDouble((*svs.netinfo.pcurrentmsg), value);
}

//=============================================
//
//=============================================
void SV_Msg_WriteBuffer( const byte* pdata, Uint32 size )
{
	if(!svs.netinfo.pcurrentmsg)
	{
		Con_Printf("%s - Called with no valid message defined.\n", __FUNCTION__);
		return;
	}

	UserMSG_Msg_WriteBuffer((*svs.netinfo.pcurrentmsg), pdata, size);
}

//=============================================
//
//=============================================
void SV_Msg_WriteString( const Char* pstring )
{
	if(!svs.netinfo.pcurrentmsg)
	{
		Con_Printf("%s - Called with no valid message defined.\n", __FUNCTION__);
		return;
	}

	UserMSG_Msg_WriteString((*svs.netinfo.pcurrentmsg), pstring);
}

//=============================================
//
//=============================================
void SV_Msg_WriteEntindex( entindex_t entindex )
{
	if(!svs.netinfo.pcurrentmsg)
	{
		Con_Printf("%s - Called with no valid message defined.\n", __FUNCTION__);
		return;
	}

	// Is this function even neeeded?
	UserMSG_Msg_WriteEntindex((*svs.netinfo.pcurrentmsg), entindex);
}

//=============================================
//
//=============================================
Uint32 SV_FindUserMessageByName( const Char* pstrName )
{
	for(Uint32 i = 0; i < svs.netinfo.usermsgs.size(); i++)
	{
		if(!qstrcmp(pstrName, svs.netinfo.usermsgs[i].name))
			return svs.netinfo.usermsgs[i].id;
	}
	
	return 0;
}