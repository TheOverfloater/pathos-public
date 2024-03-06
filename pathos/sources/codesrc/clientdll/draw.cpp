/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "draw.h"
#include "ref_params.h"
#include "messages.h"
#include "saytext.h"
#include "huddraw.h"
#include "clientdll.h"
#include "efxapi.h"
#include "hud.h"
#include "gameuimanager.h"
#include "ladder.h"
#include "motorbike.h"
#include "viewcontroller.h"
#include "viewmodel.h"
#include "dlight.h"
#include "nodedebug.h"
#include "screentext.h"

//=============================================
// @brief
//
//=============================================
void CL_SetupView( const ref_params_t& params )
{
}

//=============================================
// @brief
//
//=============================================
bool CL_DrawHUD( bool hudOnly )
{
	// Draw HUD
	if(!gHUD.Draw())
		return false;

	if(!hudOnly)
	{
		// Draw UI
		if(!gGameUIManager.Draw())
			return false;

		// Draw screen text
		if(!gScreenText.Draw())
			return false;

		// Draw text messages
		if(!gMessages.DrawMessages())
			return false;

		// Draw saytext
		if(!gSayText.DrawSayText())
			return false;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CL_DrawNormal( void )
{
	// Draw view controller objects
	if(!gViewController.Draw())
		return false;

	// Grab current modelview and proj
	if(!gHUD.DrawNormal())
		return false;

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CL_DrawTransparent( void )
{
	// Draw debug stuff here
	if(!gNodeDebug.Draw())
		return false;

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CL_DrawViewObjects( void )
{
	// Draw motorbike
	if(!gMotorBike.Draw())
		return false;

	// Draw viewmodel
	if(!gViewModel.Draw())
		return false;

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CL_DrawViewObjectsForVSM( cl_dlight_t* dl )
{
	// Draw shadows for view model if it's for the flashlight
	// shoulder flashlight is player entindex
	// glock flashlight is negative player entindex
	cl_entity_t* pplayer = cl_engfuncs.pfnGetLocalPlayer();
	if(!dl->isStatic() && SDL_abs(dl->key) == pplayer->entindex)
	{
		cl_entity_t* pviewmodel = gViewModel.GetViewModel();
		if(pviewmodel && pviewmodel->pmodel)
		{
			// Draw shadow for view model
			if(!cl_renderfuncs.pfnVBMPrepareVSMDraw(dl))
			{
				cl_engfuncs.pfnErrorPopup("Rendering error: %s.", cl_renderfuncs.pfnGetVBMShaderError());
				return false;
			}

			if(!cl_renderfuncs.pfnDrawVBMModelVSM(pviewmodel, dl))
			{
				cl_engfuncs.pfnErrorPopup("Rendering error: %s.", cl_renderfuncs.pfnGetVBMShaderError());
				cl_renderfuncs.pfnVBMEndDraw();
				return false;
			}

			cl_renderfuncs.pfnVBMEndDraw();
		}
	}

	// If motorbike is active
	if(gMotorBike.IsActive())
	{
		cl_entity_t* pmotorbike = gMotorBike.GetBikeEntity();
		if(pmotorbike && pmotorbike->pmodel)
		{
			// Draw shadow for motorbike
			if(!cl_renderfuncs.pfnVBMPrepareVSMDraw(dl))
			{
				cl_engfuncs.pfnErrorPopup("Rendering error: %s.", cl_renderfuncs.pfnGetVBMShaderError());
				return false;
			}

			if(!cl_renderfuncs.pfnDrawVBMModelVSM(pmotorbike, dl))
			{
				cl_engfuncs.pfnErrorPopup("Rendering error: %s.", cl_renderfuncs.pfnGetVBMShaderError());
				cl_renderfuncs.pfnVBMEndDraw();
				return false;
			}

			cl_renderfuncs.pfnVBMEndDraw();
		}
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CL_DrawLadders( void )
{
	if(!gLadder.Draw())
		return false;

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CL_DrawLaddersForVSM( cl_dlight_t* dl )
{
	if(!gLadder.DrawVSM(dl))
		return false;

	return true;
}