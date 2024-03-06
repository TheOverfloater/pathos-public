/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "vector.h"
#include "cl_entity.h"
#include "input.h"
#include "usercmd.h"
#include "com_math.h"
#include "cvar.h"
#include "cldll_interface.h"
#include "movevars.h"
#include "buttonbits.h"
#include "clientdll.h"
#include "motorbike.h"
#include "ladder.h"
#include "viewcontroller.h"
#include "hud.h"
#include "view.h"

class Vector;

// Default number of filter frames
static const Char DEFAULT_NB_FILTER_FRAMES[] = "2";

// Mouse filter info structure
m_filter_info_t g_mouseFilterInfo;

// Impulse command value
Int32 g_impulseValue = 0;

kbutton_t	cmd_lean;
kbutton_t	cmd_forward;
kbutton_t	cmd_back;
kbutton_t	cmd_moveleft;
kbutton_t	cmd_moveright;
kbutton_t	cmd_speed;
kbutton_t	cmd_use;
kbutton_t	cmd_jump;
kbutton_t	cmd_attack;
kbutton_t	cmd_attack2;
kbutton_t	cmd_up;
kbutton_t	cmd_down;
kbutton_t	cmd_duck;
kbutton_t	cmd_reload;
kbutton_t	cmd_sprint;
kbutton_t	cmd_walkmode;
kbutton_t	cmd_heal;
kbutton_t	cmd_special;

CCVar* g_pCvarSideSpeed = nullptr;
CCVar* g_pCvarForwardSpeed = nullptr;
CCVar* g_pCvarBackSpeed = nullptr;
CCVar* g_pCvarSensitivity = nullptr;
CCVar* g_pCvarFilterMouse = nullptr;
CCVar* g_pCvarMouseFilterFrames = nullptr;
CCVar* g_pCvarReverseMouse = nullptr;
CCVar* g_pCvarMouseYaw = nullptr;
CCVar* g_pCvarMousePitch = nullptr;
CCVar* g_pCvarMousePitchUp = nullptr;
CCVar* g_pCvarMousePitchDown = nullptr;
CCVar* g_pCvarDefaultFOV = nullptr;
CCVar* g_pCvarReferenceFOV = nullptr;

//=============================================
// Functions for key commands
//
//=============================================
void Cmd_LeanDown( void )		{ CL_KeyDown(cmd_lean); }
void Cmd_LeanUp( void )			{ CL_KeyUp(cmd_lean); }
void Cmd_ForwardDown( void )	{ CL_KeyDown(cmd_forward); }
void Cmd_ForwardUp( void )		{ CL_KeyUp(cmd_forward); }
void Cmd_BackDown( void )		{ CL_KeyDown(cmd_back); }
void Cmd_BackUp( void )			{ CL_KeyUp(cmd_back); }
void Cmd_MoveLeftDown( void )	{ CL_KeyDown(cmd_moveleft); }
void Cmd_MoveLeftUp( void )		{ CL_KeyUp(cmd_moveleft); }
void Cmd_MoveRightDown( void )	{ CL_KeyDown(cmd_moveright); }
void Cmd_MoveRightUp( void )	{ CL_KeyUp(cmd_moveright); }
void Cmd_Attack2Down( void )	{ CL_KeyDown(cmd_attack2); }
void Cmd_Attack2Up( void )		{ CL_KeyUp(cmd_attack2); }
void Cmd_UseDown( void )		{ CL_KeyDown(cmd_use); }
void Cmd_UseUp( void )			{ CL_KeyUp(cmd_use); }
void Cmd_JumpDown( void )		{ CL_KeyDown(cmd_jump); }
void Cmd_JumpUp( void )			{ CL_KeyUp(cmd_jump); }
void Cmd_DuckDown( void )		{ CL_KeyDown(cmd_duck); }
void Cmd_DuckUp( void )			{ CL_KeyUp(cmd_duck); }
void Cmd_ReloadDown( void )		{ CL_KeyDown(cmd_reload); }
void Cmd_ReloadUp( void )		{ CL_KeyUp(cmd_reload); }
void Cmd_SprintDown( void )		{ CL_KeyDown(cmd_sprint); }
void Cmd_SprintUp( void )		{ CL_KeyUp(cmd_sprint); }
void Cmd_WalkModeDown( void )	{ CL_KeyDown(cmd_walkmode);}
void Cmd_WalkModeUp( void )		{ CL_KeyUp(cmd_walkmode); }
void Cmd_AttackDown( void )		{ CL_KeyDown(cmd_attack); }
void Cmd_AttackUp( void )		{ CL_KeyUp(cmd_attack); }
void Cmd_HealDown( void )		{ CL_KeyDown(cmd_heal); }
void Cmd_HealUp( void )			{ CL_KeyUp(cmd_heal); }
void Cmd_SpecialDown( void )	{ CL_KeyDown(cmd_special); }
void Cmd_SpecialUp( void )		{ CL_KeyUp(cmd_special); }

//=============================================
//
//=============================================
void Cmd_Impulse( void )			
{ 
	if(cl_engfuncs.pfnCmd_Argc() <= 1)
		return;

	g_impulseValue = SDL_atoi(cl_engfuncs.pfnCmd_Argv(1));
}

//=============================================
//
//=============================================
void CL_KeyDown( kbutton_t& btn )
{
	Int32 keyCode = SDL_SCANCODE_UNKNOWN;
	const Char* pstrKey = cl_engfuncs.pfnCmd_Argv(1);

	// Typed manually, continous down
	if(pstrKey[0] == '\0')
		keyCode = -1;
	else
		keyCode = SDL_atoi(pstrKey);

	// If it's already down
	Uint32 i = 0;
	for(; i < MAX_INPUT_KEYS; i++)
	{
		if(btn.buttons[i] == keyCode)
			return; // Repeating key

		if(btn.buttons[i] == SDL_SCANCODE_UNKNOWN)
		{
			btn.buttons[i] = keyCode;
			break;
		}
	}

	if(i == MAX_INPUT_KEYS)
	{
		const Char* pstrBind = cl_engfuncs.pfnCmd_Argv(0);
		Char c1 = (Char)SDL_GetKeyFromScancode((SDL_Scancode)btn.buttons[0]);
		Char c2 = (Char)SDL_GetKeyFromScancode((SDL_Scancode)btn.buttons[1]);
		Char c3 = (Char)SDL_GetKeyFromScancode((SDL_Scancode)keyCode);

		cl_engfuncs.pfnCon_DPrintf("Three keys pressed simultaneously for bind '%s': '%c', '%c', '%c'.\n", pstrBind, c1, c2, c3);
	}

	if(btn.state & KS_DOWN)
		return; // Still down

	// Down + impulse down
	btn.state |= (KS_DOWN | KS_IDOWN);
}

//=============================================
//
//=============================================
void CL_KeyUp( kbutton_t& btn, bool isReset )
{
	Int32 keyCode = SDL_SCANCODE_UNKNOWN;
	const Char* pstrKey = nullptr;
	if(!isReset)
		pstrKey = cl_engfuncs.pfnCmd_Argv(1);

	// Typed manually, unstick the key
	if(!pstrKey || pstrKey[0] == '\0')
	{
		// Unstick all keys
		for(Uint32 i = 0; i < MAX_INPUT_KEYS; i++)
			btn.buttons[i] = SDL_SCANCODE_UNKNOWN;

		btn.state = KS_IUP;
		return;
	}

	// Get key from the second parameter
	keyCode = SDL_atoi(pstrKey);

	Uint32 i = 0;
	for(; i < MAX_INPUT_KEYS; i++)
	{
		if(btn.buttons[i] == keyCode)
		{
			btn.buttons[i] = SDL_SCANCODE_UNKNOWN;
			break;
		}
	}

	if(i == MAX_INPUT_KEYS)
		return;

	// See if any of the buttons are down
	for(i = 0; i < MAX_INPUT_KEYS; i++)
	{
		if(btn.buttons[i] != SDL_SCANCODE_UNKNOWN)
			return;
	}

	if(!(btn.state & KS_DOWN))
		return; // Still up

	btn.state &= ~KS_DOWN;
	btn.state |= KS_IUP;
}

//=============================================
//
//=============================================
Float CL_KeyState( kbutton_t& btn )
{
	// Value to return
	Float value = 0;

	bool impDown = (btn.state & KS_IDOWN) ? true : false;
	bool impUp = (btn.state & KS_IUP) ? true : false;
	bool down = (btn.state & KS_DOWN) ? true : false;

	if( impDown && !impUp )
		value = down ? 0.5 : 0.0; // Pressed and held this frame ?

	if( impUp && !impDown )
		value = 0.0;

	if( !impDown && !impUp )
		value = down ? 1.0 : 0.0; // Held entire frame?

	if( impDown && impUp )
		value = down ? 0.75 : 0.25;

	// Clear impulses
	btn.state &= KS_DOWN;
	return value;
}

//=============================================
//
//=============================================
void CL_MouseFilterFramesCvarCallback( CCVar* pCVar )
{
	Int32 numFilterFrames = pCVar->GetValue();
	if(numFilterFrames < MOUSE_FILTER_MIN_FRAMES)
	{
		const char* pstrCvarName = pCVar->GetName();
		cl_engfuncs.pfnCon_Printf("Invalid setting '%d' specified for cvar '%s'.\n", numFilterFrames, pstrCvarName);
		cl_engfuncs.pfnSetCVarFloat(pstrCvarName, (Float)MOUSE_FILTER_MIN_FRAMES);
		numFilterFrames = MOUSE_FILTER_MIN_FRAMES;
	}
	else if(numFilterFrames > MOUSE_FILTER_MAX_FRAMES)
	{
		const char* pstrCvarName = pCVar->GetName();
		cl_engfuncs.pfnCon_Printf("Invalid setting '%d' specified for cvar '%s'.\n", numFilterFrames, pstrCvarName);
		cl_engfuncs.pfnSetCVarFloat(pstrCvarName, (Float)MOUSE_FILTER_MAX_FRAMES);
		numFilterFrames = MOUSE_FILTER_MAX_FRAMES;
	}

	// Reset the mouse filter
	g_mouseFilterInfo.numframes = 0;
	g_mouseFilterInfo.filterframes.resize(numFilterFrames);
}

//=============================================
//
//=============================================
void CL_InitInput( void )
{
	// Create the input commands
	cl_engfuncs.pfnCreateCommand("+forward",Cmd_ForwardDown, nullptr);
	cl_engfuncs.pfnCreateCommand("-forward",Cmd_ForwardUp, nullptr);
	cl_engfuncs.pfnCreateCommand("+back",Cmd_BackDown, nullptr);
	cl_engfuncs.pfnCreateCommand("-back",Cmd_BackUp, nullptr);
	cl_engfuncs.pfnCreateCommand("+moveleft", Cmd_MoveLeftDown, nullptr);
	cl_engfuncs.pfnCreateCommand("-moveleft", Cmd_MoveLeftUp, nullptr);
	cl_engfuncs.pfnCreateCommand("+moveright", Cmd_MoveRightDown, nullptr);
	cl_engfuncs.pfnCreateCommand("-moveright", Cmd_MoveRightUp, nullptr);
	cl_engfuncs.pfnCreateCommand("+attack", Cmd_AttackDown, nullptr);
	cl_engfuncs.pfnCreateCommand("-attack", Cmd_AttackUp, nullptr);
	cl_engfuncs.pfnCreateCommand("+attack2", Cmd_Attack2Down, nullptr);
	cl_engfuncs.pfnCreateCommand("-attack2", Cmd_Attack2Up, nullptr);
	cl_engfuncs.pfnCreateCommand("+use", Cmd_UseDown, nullptr);
	cl_engfuncs.pfnCreateCommand("-use", Cmd_UseUp, nullptr);
	cl_engfuncs.pfnCreateCommand("+jump", Cmd_JumpDown, nullptr);
	cl_engfuncs.pfnCreateCommand("-jump", Cmd_JumpUp, nullptr);
	cl_engfuncs.pfnCreateCommand("+duck", Cmd_DuckDown, nullptr);
	cl_engfuncs.pfnCreateCommand("-duck", Cmd_DuckUp, nullptr);
	cl_engfuncs.pfnCreateCommand("+reload", Cmd_ReloadDown, nullptr);
	cl_engfuncs.pfnCreateCommand("-reload", Cmd_ReloadUp, nullptr);
	cl_engfuncs.pfnCreateCommand("+lean",Cmd_LeanDown, nullptr);
	cl_engfuncs.pfnCreateCommand("-lean",Cmd_LeanUp, nullptr);
	cl_engfuncs.pfnCreateCommand("+sprint", Cmd_SprintDown, nullptr);
	cl_engfuncs.pfnCreateCommand("-sprint", Cmd_SprintUp, nullptr);
	cl_engfuncs.pfnCreateCommand("+walkmode",Cmd_WalkModeDown, nullptr);
	cl_engfuncs.pfnCreateCommand("-walkmode",Cmd_WalkModeUp, nullptr);
	cl_engfuncs.pfnCreateCommand("+heal", Cmd_HealDown, nullptr);
	cl_engfuncs.pfnCreateCommand("-heal", Cmd_HealUp, nullptr);
	cl_engfuncs.pfnCreateCommand("+special", Cmd_SpecialDown, nullptr);
	cl_engfuncs.pfnCreateCommand("-special", Cmd_SpecialUp, nullptr);
	cl_engfuncs.pfnCreateCommand("impulse", Cmd_Impulse, nullptr);

	// Player movement related cvars
	g_pCvarSideSpeed = cl_engfuncs.pfnCreateCVar(CVAR_FLOAT, FL_CV_CLIENT, "cl_sidespeed", "300", "Sideways movement speed for players.");
	g_pCvarForwardSpeed = cl_engfuncs.pfnCreateCVar(CVAR_FLOAT, FL_CV_CLIENT, "cl_forwardspeed", "300", "Forward movement speed for players.");
	g_pCvarBackSpeed = cl_engfuncs.pfnCreateCVar(CVAR_FLOAT, FL_CV_CLIENT, "cl_backspeed", "300", "Backwards movement speed for players.");

	// Mouse input related cvars
	g_pCvarSensitivity = cl_engfuncs.pfnCreateCVar(CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), MOUSE_SENSITIVITY_CVAR_NAME, "4", "Mouse sensitivity.");
	g_pCvarFilterMouse = cl_engfuncs.pfnCreateCVar(CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), MOUSE_FILTER_CVAR_NAME, "1", "Mouse movement filtering.");
	g_pCvarMouseFilterFrames = cl_engfuncs.pfnCreateCVarCallback(CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), MOUSE_FILTER_FRAMES_CVAR_NAME, DEFAULT_NB_FILTER_FRAMES, "Number of frames to blend mouse movements over.", CL_MouseFilterFramesCvarCallback);
	g_pCvarReverseMouse = cl_engfuncs.pfnCreateCVar(CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), MOUSE_REVERSE_CVAR_NAME, "1", "Reverse mouse Y axis.");
	g_pCvarMouseYaw = cl_engfuncs.pfnCreateCVar(CVAR_FLOAT, FL_CV_CLIENT, "m_yaw", "1", "Mouse yaw turn speed.");
	g_pCvarMousePitch = cl_engfuncs.pfnCreateCVar(CVAR_FLOAT, FL_CV_CLIENT, "m_pitch", "1", "Mouse pitch turn speed.");
	g_pCvarMousePitchUp = cl_engfuncs.pfnCreateCVar(CVAR_FLOAT, FL_CV_CLIENT, "m_pitchup", "80", "Mouse pitch turn min limit.");
	g_pCvarMousePitchDown = cl_engfuncs.pfnCreateCVar(CVAR_FLOAT, FL_CV_CLIENT, "m_pitchdown", "80", "Mouse pitch turn max limit.");
	g_pCvarDefaultFOV = cl_engfuncs.pfnGetCVarPointer(DEFAULT_FOV_CVAR_NAME);
	g_pCvarReferenceFOV = cl_engfuncs.pfnGetCVarPointer(REFERENCE_FOV_CVAR_NAME);
}

//=============================================
//
//=============================================
void CL_ShutdownInput( void )
{
}

//=============================================
//
//=============================================
void CL_ResetPressedInputs( void )
{
	Int32 bits = CL_GetButtonBits(false);
	if(!bits)
		return;

	if(bits & IN_LEAN)
		CL_KeyUp(cmd_lean, true);

	if(bits & IN_FORWARD)
		CL_KeyUp(cmd_forward, true);

	if(bits & IN_BACK)
		CL_KeyUp(cmd_back, true);

	if(bits & IN_MOVELEFT)
		CL_KeyUp(cmd_moveleft, true);

	if(bits & IN_MOVERIGHT)
		CL_KeyUp(cmd_moveright, true);

	if(bits & IN_USE)
		CL_KeyUp(cmd_use, true);

	if(bits & IN_JUMP)
		CL_KeyUp(cmd_jump, true);

	if(bits & IN_ATTACK)
		CL_KeyUp(cmd_attack, true);

	if(bits & IN_ATTACK2)
		CL_KeyUp(cmd_attack2, true);

	if(bits & IN_DUCK)
		CL_KeyUp(cmd_duck, true);

	if(bits & IN_RELOAD)
		CL_KeyUp(cmd_reload, true);

	if(bits & IN_SPRINT)
		CL_KeyUp(cmd_sprint, true);

	if(bits & IN_WALKMODE)
		CL_KeyUp(cmd_walkmode, true);

	if(bits & IN_HEAL)
		CL_KeyUp(cmd_heal, true);

	if(bits & IN_SPECIAL)
		CL_KeyUp(cmd_special, true);
}

//=============================================
//
//=============================================
Int32 CL_GetButtonBits( bool resetstate )
{
	Int32 bits = 0;

	if(cmd_lean.state & (KS_IDOWN|KS_DOWN))
		bits |= IN_LEAN;

	if(cmd_forward.state & (KS_IDOWN|KS_DOWN))
		bits |= IN_FORWARD;

	if(cmd_back.state & (KS_IDOWN|KS_DOWN))
		bits |= IN_BACK;

	if(cmd_moveleft.state & (KS_IDOWN|KS_DOWN))
		bits |= IN_MOVELEFT;

	if(cmd_moveright.state & (KS_IDOWN|KS_DOWN))
		bits |= IN_MOVERIGHT;

	if(cmd_use.state & (KS_IDOWN|KS_DOWN))
		bits |= IN_USE;

	if(cmd_jump.state & (KS_IDOWN|KS_DOWN))
		bits |= IN_JUMP;

	if(cmd_attack.state & (KS_IDOWN|KS_DOWN))
		bits |= IN_ATTACK;

	if(cmd_attack2.state & (KS_IDOWN|KS_DOWN))
		bits |= IN_ATTACK2;

	if(cmd_up.state & (KS_IDOWN|KS_DOWN))
		bits |= IN_UP;

	if(cmd_down.state & (KS_IDOWN|KS_DOWN))
		bits |= IN_DOWN;

	if(cmd_duck.state & (KS_IDOWN|KS_DOWN))
		bits |= IN_DUCK;

	if(cmd_reload.state & (KS_IDOWN|KS_DOWN))
		bits |= IN_RELOAD;

	if(cmd_sprint.state & (KS_IDOWN|KS_DOWN))
		bits |= IN_SPRINT;

	if(cmd_walkmode.state & (KS_IDOWN|KS_DOWN))
		bits |= IN_WALKMODE;

	if(cmd_heal.state & (KS_IDOWN|KS_DOWN))
		bits |= IN_HEAL;

	if(cmd_special.state & (KS_IDOWN|KS_DOWN))
		bits |= IN_SPECIAL;

	if(resetstate)
	{
		cmd_lean.state &= ~KS_IDOWN;
		cmd_forward.state &= ~KS_IDOWN;
		cmd_back.state &= ~KS_IDOWN;
		cmd_moveleft.state &= ~KS_IDOWN;
		cmd_moveright.state &= ~KS_IDOWN;
		cmd_speed.state &= ~KS_IDOWN;
		cmd_use.state &= ~KS_IDOWN;
		cmd_jump.state &= ~KS_IDOWN;
		cmd_attack.state &= ~KS_IDOWN;
		cmd_attack2.state &= ~KS_IDOWN;
		cmd_up.state &= ~KS_IDOWN;
		cmd_down.state &= ~KS_IDOWN;
		cmd_duck.state &= ~KS_IDOWN;
		cmd_reload.state &= ~KS_IDOWN;
		cmd_sprint.state &= ~KS_IDOWN;
		cmd_walkmode.state &= ~KS_IDOWN;
		cmd_heal.state &= ~KS_IDOWN;
		cmd_special.state &= ~KS_IDOWN;
	}

	return bits;
}

//=============================================
//
//=============================================
bool CL_IsNoClipping( void )
{
	cl_entity_t* pplayer = cl_engfuncs.pfnGetLocalPlayer();
	if(!pplayer || pplayer->curstate.movetype != MOVETYPE_NOCLIP)
		return false;
	else
		return true;
}

//=============================================
//
//=============================================
bool CL_IsParalyzed( void )
{
	cl_entity_t* pplayer = cl_engfuncs.pfnGetLocalPlayer();
	if(pplayer && (pplayer->curstate.flags & FL_PARALYZED))
		return true;
	else
		return false;
}

//=============================================
//
//=============================================
Float CL_GetSideSpeed( bool isNoClipping )
{
	if(isNoClipping)
		return PLAYER_NOCLIP_SPEED;
	else
		return g_pCvarSideSpeed->GetValue();
}

//=============================================
//
//=============================================
Float CL_GetForwardSpeed( bool isNoClipping )
{
	if(isNoClipping)
		return PLAYER_NOCLIP_SPEED;
	else
		return g_pCvarForwardSpeed->GetValue();
}

//=============================================
//
//=============================================
Float CL_GetBackSpeed( bool isNoClipping )
{
	if(isNoClipping)
		return PLAYER_NOCLIP_SPEED;
	else
		return g_pCvarBackSpeed->GetValue();
}

//=============================================
//
//=============================================
void CL_CreateMove( usercmd_t& cmd )
{
	// Set view angles
	cmd.viewangles = cl_engfuncs.pfnGetViewAngles();
	// Check for noclip
	bool isNoClipping = CL_IsNoClipping();

	Float maxspeed = 0;
	if(!CL_IsParalyzed())
	{
		if(gMotorBike.IsActive())
		{
			// With motorbike, just add turn amount
			cmd.viewangles[YAW] += gMotorBike.GetTurnAmount();

			// Set forwardmove to acceleration
			cmd.forwardmove = gMotorBike.GetAcceleration();

			// Cap at motorbike max speed
			maxspeed = MOTORBIKE_MAX_SPEED;
		}
		else
		{
			// Add in movement to the side
			cmd.sidemove += CL_GetSideSpeed(isNoClipping) * CL_KeyState(cmd_moveright);
			cmd.sidemove -= CL_GetSideSpeed(isNoClipping) * CL_KeyState(cmd_moveleft);

			// Add in movement to front/back
			cmd.forwardmove += CL_GetForwardSpeed(isNoClipping) * CL_KeyState(cmd_forward);
			cmd.forwardmove -= CL_GetBackSpeed(isNoClipping) * CL_KeyState(cmd_back);

			// Get movevars
			const movevars_t* pmovevars = cl_engfuncs.pfnGetMoveVars();

			// Set max speed at one from server
			maxspeed = isNoClipping ? PLAYER_NOCLIP_SPEED : pmovevars->maxspeed;
		}
	}

	// Cap at maxspeed
	if(maxspeed != 0)
	{
		Float fmov = SDL_sqrt((cmd.forwardmove*cmd.forwardmove)
			+ (cmd.sidemove*cmd.sidemove)
			+ (cmd.upmove*cmd.upmove));

		if(fmov > maxspeed)
		{
			Float ratio = maxspeed/fmov;
			cmd.forwardmove *= ratio;
			cmd.sidemove *= ratio;
			cmd.upmove *= ratio;
		}
	}

	// Set weapon selection
	cmd.weaponselect = gHUD.GetWeaponSelect();
	gHUD.SetWeaponSelect(0);

	// Set impulse
	cmd.impulse = g_impulseValue;
	g_impulseValue = 0;

	// Set button bits
	cmd.buttons = CL_GetButtonBits(true);
}

//=============================================
//
//=============================================
void CL_ResetButtonBits( Int32 bits )
{
	Int32 bitsnew = CL_GetButtonBits( false ) ^ bits;
	
	if(bitsnew & IN_ATTACK)
	{
		if(bits & IN_ATTACK)
			CL_KeyDown(cmd_attack);
		else
			cmd_attack.state &= ~KS_ALL_BITS;
	}
}

//=============================================
//
//=============================================
void CL_AddMouseMove( usercmd_t& cmd )
{
	Int32 deltaX, deltaY;
	cl_engfuncs.pfnGetMouseDelta(deltaX, deltaY);

	// Shift filter frames
	if(g_mouseFilterInfo.numframes > 0)
	{
		Uint32 count = g_mouseFilterInfo.filterframes.size() - 1;

		for(Uint32 i = count; i > 0; i--)
			g_mouseFilterInfo.filterframes[i] = g_mouseFilterInfo.filterframes[i - 1];
	}

	// Save current filter frame
	g_mouseFilterInfo.filterframes[0].mousex = deltaX;
	g_mouseFilterInfo.filterframes[0].mousey = deltaY;

	if(g_mouseFilterInfo.numframes < g_mouseFilterInfo.filterframes.size())
		g_mouseFilterInfo.numframes++;

	// Determine mouse movement
	Float mouseX, mouseY;
	if(g_pCvarFilterMouse->GetValue() > 0)
	{
		mouseX = 0;
		mouseY = 0;
		
		for(Uint32 i = 0; i < g_mouseFilterInfo.numframes; i++)
		{
			mouseX += (Float)g_mouseFilterInfo.filterframes[i].mousex;
			mouseY += (Float)g_mouseFilterInfo.filterframes[i].mousey;
		}

		mouseX = mouseX / (Float)g_mouseFilterInfo.numframes;
		mouseY = mouseY / (Float)g_mouseFilterInfo.numframes;
	}
	else
	{
		mouseX = deltaX;
		mouseY = deltaY;
	}

	// Adjust for zooming
	if(gDefaultView.GetFOV() != g_pCvarReferenceFOV->GetValue())
	{
		Float adjScale = gDefaultView.GetFOV()/g_pCvarReferenceFOV->GetValue();
		mouseX *= adjScale;
		mouseY *= adjScale;
	}

	Float sensitivity = g_pCvarSensitivity->GetValue() * 0.1;
	if(sensitivity > 0)
	{
		mouseX *= sensitivity;
		mouseY *= sensitivity;
	}

	if(g_pCvarReverseMouse->GetValue() > 0)
		mouseY = -mouseY;

	// Calculate final movement
	Float mouseMoveYaw = g_pCvarMouseYaw->GetValue() * mouseX;
	Float mouseMovePitch = g_pCvarMousePitch->GetValue() * mouseY;
	if(!CL_IsParalyzed())
	{
		if(gLadder.IsActive())
		{
			// Move ladder view by mouse movement
			gLadder.MouseMove(mouseMoveYaw, mouseMovePitch);
		}
		else if(gMotorBike.IsActive())
		{
			// Move motorbike view by mouse movement
			gMotorBike.MouseMove(mouseMoveYaw, mouseMovePitch);
		}
		else if(!gViewController.IsActive())
		{
			cmd.viewangles[YAW] -= mouseMoveYaw;
			cmd.viewangles[PITCH] += mouseMovePitch;

			Float pitchdown = g_pCvarMousePitchDown->GetValue();
			if(cmd.viewangles[PITCH] > pitchdown)
				cmd.viewangles[PITCH] = pitchdown;

			Float pitchup = g_pCvarMousePitchUp->GetValue();
			if(cmd.viewangles[PITCH] < -pitchup)
				cmd.viewangles[PITCH] = -pitchup;
		}
	}

	// Set view angles
	cmd.viewangles[YAW] = Math::AngleMod(cmd.viewangles[YAW]);

	// Set view angles for client
	cl_engfuncs.pfnSetViewAngles(cmd.viewangles);
}
