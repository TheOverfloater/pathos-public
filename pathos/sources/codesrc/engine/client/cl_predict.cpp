/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "cl_predict.h"
#include "cl_main.h"
#include "pm_shared.h"
#include "cl_utils.h"
#include "com_math.h"
#include "system.h"

#include "r_main.h"
#include "cl_tempentities.h"
#include "modelcache.h"

//=============================================
//
//=============================================
void CL_PredictPlayer( usercmd_t& cmd, pm_info_t& pminfo, bool playSounds, cl_entity_t* pplayer )
{
	// Reset player curstate
	cls.pminfo.playerstate = pplayer->curstate;

	// Predict player
	cls.dllfuncs.pfnRunPlayermove(cmd, &pminfo, playSounds);

	// Remember old buttons
	pplayer->curstate.oldbuttons = cmd.buttons;
}

//=============================================
//
//=============================================
void CL_RunPrediction( void )
{
	if(cls.paused)
		return;

	// Reset these
	cls.pminfo.clientindex = 0;

	// Retreive local player
	cl_entity_t *pplayer = CL_GetLocalPlayer();
	if(!pplayer)
		return;

	if(g_pCvarPredictiton->GetValue() < 1)
	{
		if(cls.usercmdhistorynum > 0)
		{
			// Only predict on last player cmd
			Uint32 lastcmdidx = cls.usercmdhistorynum-1;
			usercmd_t& latestcmd = cls.usercmdhistory[lastcmdidx];

			CL_PredictPlayer(latestcmd, cls.pminfo, true, pplayer);
		}

		// Do no further prediction
		return;
	}

	// Run each saved usercmd to predict everything
	for(Uint32 i = 0; i < cls.usercmdhistorynum; i++)
	{
		usercmd_t& cmd = cls.usercmdhistory[i];

		// Only play sounds on the latest cmd
		bool playSounds = (i == (cls.usercmdhistorynum-1)) ? true : false;

		// Predict the player
		CL_PredictPlayer(cmd, cls.pminfo, playSounds, pplayer);

		// Copy curstate to prevstate, and predicted player state to curstate
		pplayer->prevstate = pplayer->curstate;
		pplayer->curstate = cls.pminfo.playerstate;
	}
}