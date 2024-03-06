/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef INPUT_H
#define INPUT_H

#include "usercmd.h"

// Max keys a command can be bound to
static const Uint32 MAX_INPUT_KEYS = 2;

enum key_states_t
{
	KS_UP		= 0, // Key is up
	KS_DOWN		= 1, // Key is down
	KS_IDOWN	= 2, // Impulse down
	KS_IUP		= 4, // Impulse up
};

#define KS_ALL_BITS (KS_UP|KS_DOWN|KS_IDOWN|KS_IUP)

// For client buttons
struct kbutton_t
{
	kbutton_t():
		state(0)
	{
		for(Uint32 i = 0; i < MAX_INPUT_KEYS; i++)
			buttons[i] = SDL_SCANCODE_UNKNOWN;
	}

	Int16 buttons[MAX_INPUT_KEYS];
	Int16 state;
};

struct m_filterframe_t
{
	m_filterframe_t():
		mousex(0),
		mousey(0)
		{}

	Int32 mousex;
	Int32 mousey;
};

struct m_filter_info_t
{
	m_filter_info_t():
		numframes(0)
		{}

	CArray<m_filterframe_t> filterframes;
	Uint32 numframes;
};

extern kbutton_t cmd_lean;
extern kbutton_t cmd_forward;
extern kbutton_t cmd_back;
extern kbutton_t cmd_moveleft;
extern kbutton_t cmd_moveright;
extern kbutton_t cmd_speed;
extern kbutton_t cmd_use;
extern kbutton_t cmd_jump;
extern kbutton_t cmd_attack;
extern kbutton_t cmd_attack2;
extern kbutton_t cmd_up;
extern kbutton_t cmd_down;
extern kbutton_t cmd_duck;
extern kbutton_t cmd_reload;
extern kbutton_t cmd_sprint;
extern kbutton_t cmd_walkmode;
extern kbutton_t cmd_heal;

// Functions for client input
extern void CL_KeyDown( kbutton_t& btn );
extern void CL_KeyUp( kbutton_t& btn, bool isReset = false );
extern Float CL_KeyState( kbutton_t& btn );

extern void CL_InitInput( void );
extern void CL_ShutdownInput( void );
extern void CL_CreateMove( usercmd_t& cmd );
extern void CL_AddMouseMove( usercmd_t& cmd );
extern void CL_ResetPressedInputs( void );
extern Int32 CL_GetButtonBits( bool resetstate );
extern void CL_ResetButtonBits( Int32 bits );
#endif