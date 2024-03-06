/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "input.h"
#include "commands.h"
#include "window.h"
#include "file.h"

#include "uimanager.h"

#include "texturemanager.h"
#include "r_menu.h"
#include "console.h"
#include "config.h"
#include "system.h"
#include "enginestate.h"
#include "cl_main.h"

// Number of additional keys besides scancodes
#define NUM_EXTRA_KEYS 32
// Number of keys for mouse wheel events
#define NUM_WHEEL_KEYS 4
// Number of mouse buttons
#define NB_MOUSE_BTN 5

//
// Console commands
//
void Cmd_BindKey( void ) { gInput.CmdBindKey(); }
void Cmd_UnbindKey( void ) { gInput.CmdUnbindKey(); }
void Cmd_UnbindAll( void ) { gInput.CmdUnbindAll(); }

// Class definition
CInput gInput;

const CString CInput::MOUSE_WHEEL_NAMES[] = {
	"MWHEELUP",
	"MWHEELDOWN",
	"MWHEELRIGHT",
	"MWHEELLEFT"
};

// Path to keynames file
const Char CInput::KEYNAMES_FILE_PATH[] = "scripts/keys/keynames.txt";

//=============================================
// @brief Constructor
//
//=============================================
CInput::CInput( void ):
	m_numKeyInputs(0),
	m_isCursorVisible(true),
	m_isShiftDown(false),
	m_resetMouse(false)
{
	m_oldMousePosition[0] = m_mousePosition[0] = 0;
	m_oldMousePosition[1] = m_mousePosition[1] = 0;
}


//=============================================
// @brief Destructor
//
//=============================================
CInput::~CInput( void )
{
}

//=============================================
// @brief Initializes the class
//
//=============================================
bool CInput::Init( void )
{
	// Create the commands
	gCommands.CreateCommand("bind", Cmd_BindKey, "Binds a command to a key");
	gCommands.CreateCommand("unbind", Cmd_UnbindKey, "Unbinds commands from a key");
	gCommands.CreateCommand("unbindall", Cmd_UnbindAll, "Unbinds commands from all keys");

	// Allocate keyinfos
	m_keyInfoArray.resize(SDL_NUM_SCANCODES+NUM_EXTRA_KEYS);
	// Allocate key buffer
	m_keyInputBuffer.resize(SDL_NUM_SCANCODES+NUM_EXTRA_KEYS);

	// Load key names
	if(!LoadKeyNames())
		return false;

	return true;
}

//=============================================
// @brief Loads the key names list
//
//=============================================
bool CInput::LoadKeyNames( void )
{
	CString filename = KEYNAMES_FILE_PATH;
	const Char* pfile = reinterpret_cast<const Char*>(FL_LoadFile(filename.c_str()));
	if(!pfile)
	{
		Con_EPrintf("Failed to load key names file.\n");
		return false;
	}

	// Process each line
	Int32 lineNum = 0;
	const Char *pstr = pfile;
	while(pstr)
	{
		CString line;
		pstr = Common::ReadLine(pstr, line);

		// Skip empty lines
		if(line.empty())
		{
			lineNum++;
			continue;
		}

		// Skip comments
		if(!qstrncmp(line.c_str(), "//", 2))
		{
			lineNum++;
			continue;
		}

		// Read in the key name
		CString btnName;
		const Char* ppstr = Common::Parse(line.c_str(), btnName);
		if(!ppstr || btnName.empty())
		{
			Con_EPrintf("Line %d in %s is incomplete.\n", lineNum, filename.c_str());
			FL_FreeFile(pfile);
			return false;
		}

		// Read in the key name
		CString sdlScancode;
		ppstr = Common::Parse(ppstr, sdlScancode);
		if(!ppstr || sdlScancode.empty())
		{
			Con_EPrintf("Line %d in %s is incomplete.\n", lineNum, filename.c_str());
			FL_FreeFile(pfile);
			return false;
		}

		// Make sure the value is valid
		if(!Common::IsNumber(sdlScancode.c_str()))
		{
			Con_EPrintf("Line %d in %s has an invalid scancode value.\n", lineNum, filename.c_str());
			FL_FreeFile(pfile);
			return false;
		}

		// Read in the forst color field
		CString keyColor;
		ppstr = Common::Parse(ppstr, keyColor);
		if(keyColor.empty())
		{
			Con_EPrintf("Line %d in %s is incomplete.\n", lineNum, filename.c_str());
			FL_FreeFile(pfile);
			return false;
		}

		// Will hold the color value
		color32_t color;
		if(!qstrcicmp(keyColor, "DEFAULT"))
		{
			// Set default color
			color = color32_t(255, 255, 255, 255);
		}
		else
		{
			if(!ppstr)
			{
				Con_EPrintf("Line %d in %s is incomplete.\n", lineNum, filename.c_str());
				FL_FreeFile(pfile);
				return false;
			}

			// Read in the color's second component
			CString keyColorG;
			ppstr = Common::Parse(ppstr, keyColorG);
			if(!ppstr || keyColorG.empty())
			{
				Con_EPrintf("Line %d in %s is incomplete.\n", lineNum, filename.c_str());
				FL_FreeFile(pfile);
				return false;
			}

			// Read in the color's third component
			CString keyColorB;
			ppstr = Common::Parse(ppstr, keyColorB);
			if(keyColorB.empty())
			{
				Con_EPrintf("Line %d in %s is incomplete.\n", lineNum, filename.c_str());
				FL_FreeFile(pfile);
				return false;
			}

			// Verify that the values are correct
			if(!Common::IsNumber(keyColor.c_str()) 
				|| !Common::IsNumber(keyColorG.c_str()) 
				|| !Common::IsNumber(keyColorB.c_str()))
			{
				Con_EPrintf("Line %d has invalid color values in %s.\n", lineNum, filename.c_str());
				FL_FreeFile(pfile);
				return false;
			}

			// Set the values
			color.r = clamp(SDL_atoi(keyColor.c_str()), 0, 255);
			color.g = clamp(SDL_atoi(keyColorG.c_str()), 0, 255);
			color.b = clamp(SDL_atoi(keyColorB.c_str()), 0, 255);
		}
		
		// Get the scancode from the file
		Int32 keyIndex = SDL_atoi(sdlScancode.c_str());
		if(keyIndex < 0 || (Uint32)keyIndex >= m_keyInfoArray.size())
		{
			Con_EPrintf("Line %d in %s has an invalid scancode value.\n", lineNum, filename.c_str());
			FL_FreeFile(pfile);
			return false;
		}

		// Get the keyinfo
		keyinfo_t& keyInfo = m_keyInfoArray[keyIndex];
		keyInfo.color = color;
		keyInfo.name = btnName;

		lineNum++;
	}

	FL_FreeFile(pfile);

	// For any missing keys, just set the SDL default
	for(Uint32 i = 0; i < m_keyInfoArray.size(); i++)
	{
		keyinfo_t& keyInfo = m_keyInfoArray[i];

		if(keyInfo.name.empty())
		{
			SDL_Keycode sdlKeycode = SDL_GetKeyFromScancode((SDL_Scancode)i);
			keyInfo.name = SDL_GetKeyName(sdlKeycode);
			keyInfo.color = color32_t(255, 255, 255, 255);
		}
	}

	// For mouse too
	for(Uint32 i = 0; i < NB_MOUSE_BTN; i++)
	{
		Uint32 idx = SDL_NUM_SCANCODES + (i + 1);
		keyinfo_t& keyInfo = m_keyInfoArray[idx];

		if(keyInfo.name.empty())
		{
			CString sBtnName;
			sBtnName << "MOUSE" << (Int32)(i+1);

			keyInfo.name = sBtnName;
			keyInfo.color = color32_t(255, 255, 255, 255);
		}
	}

	// For mouse wheel events too
	for(Uint32 i = 0; i < NUM_WHEEL_KEYS; i++)
	{
		Uint32 idx = SDL_NUM_SCANCODES + NB_MOUSE_BTN + (i + 1);
		keyinfo_t& keyInfo = m_keyInfoArray[idx];

		if(keyInfo.name.empty())
		{
			CString sBtnName;
			sBtnName = MOUSE_WHEEL_NAMES[i];

			keyInfo.name = sBtnName;
			keyInfo.color = color32_t(255, 255, 255, 255);
		}
	}

	return true;
}

//=============================================
// @brief Shows the mouse cursor
//
//=============================================
void CInput::ShowMouse( void )
{
	if(!m_isCursorVisible)
	{
		SDL_ShowCursor(SDL_ENABLE);
		m_isCursorVisible = true;
	}
}

//=============================================
// @brief Hides the mouse cursor
//
//=============================================
void CInput::HideMouse( void )
{
	if(m_isCursorVisible)
	{
		SDL_ShowCursor(SDL_DISABLE);
		m_isCursorVisible = false;
	}
}

//=============================================
// @brief Handles an SDL event
//
//=============================================
void CInput::HandleSDLEvent( const SDL_Event& sdlEvent )
{
	switch(sdlEvent.type)
	{
	case SDL_KEYDOWN:
	case SDL_KEYUP:
		{
			if(m_numKeyInputs == m_keyInputBuffer.size())
				return;

			// Add a key event
			keyevent_t& keyEvent = m_keyInputBuffer[m_numKeyInputs];
			m_numKeyInputs++;

			keyEvent.mod = sdlEvent.key.keysym.mod;
			keyEvent.button = sdlEvent.key.keysym.scancode;
			keyEvent.isDown = (sdlEvent.type == SDL_KEYDOWN) ? true : false;
			keyEvent.type = EVENT_KEYBOARD_KEY;
		}
		break;
	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
		{
			if(m_numKeyInputs == m_keyInputBuffer.size())
				return;

			// Add a key event
			keyevent_t& keyEvent = m_keyInputBuffer[m_numKeyInputs];
			m_numKeyInputs++;
			
			keyEvent.mod = KMOD_NONE;
			keyEvent.button = sdlEvent.button.button;
			keyEvent.isDown = (sdlEvent.type == SDL_MOUSEBUTTONDOWN) ? true : false;
			keyEvent.type = EVENT_MOUSE_BUTTON;
		}
		break;
	case SDL_MOUSEWHEEL:
		{
			if(m_numKeyInputs == m_keyInputBuffer.size())
				return;

			// Add a key event
			keyevent_t& keyEvent = m_keyInputBuffer[m_numKeyInputs];
			m_numKeyInputs++;

			keyEvent.mod = KMOD_NONE;
			keyEvent.type = EVENT_MOUSE_WHEEL;

			if(sdlEvent.wheel.x != 0)
			{
				if(sdlEvent.wheel.x > 0)
					keyEvent.button = MOUSE_WHEEL_RIGHT;
				else
					keyEvent.button = MOUSE_WHEEL_LEFT;

				keyEvent.scroll = abs(sdlEvent.wheel.x);
			}
			else if(sdlEvent.wheel.y != 0)
			{
				if(sdlEvent.wheel.y > 0)
					keyEvent.button = MOUSE_WHEEL_UP;
				else
					keyEvent.button = MOUSE_WHEEL_DOWN;

				keyEvent.scroll = abs(sdlEvent.wheel.y);
			}
		}
		break;
	case SDL_MOUSEMOTION:
	default:
		// Not an event we want to handle
		break;
	}
}

//=============================================
// @brief Handles all inputs
//
//=============================================
void CInput::HandleInput( void )
{
	// Parse through all the key events
	for(Uint32 i = 0; i < m_numKeyInputs; i++)
	{
		const keyevent_t& keyEvent = m_keyInputBuffer[i];

		// Send the key event
		switch(keyEvent.type)
		{
		case EVENT_KEYBOARD_KEY:
			KeyEvent(keyEvent.button, keyEvent.mod, keyEvent.isDown);
			break;
		case EVENT_MOUSE_BUTTON:
			MouseButtonEvent(keyEvent.button, keyEvent.isDown);
			break;
		case EVENT_MOUSE_WHEEL:
			// Send two events to handle input correctly
			MouseWheelEvent(keyEvent.button, true, keyEvent.scroll);
			MouseWheelEvent(keyEvent.button, false, keyEvent.scroll);
			break;
		}
	}

	// Reset this
	m_numKeyInputs = 0;
}

//=============================================
// @brief Handles a key event
//
//=============================================
void CInput::KeyEvent( Int32 button, Int16 mod, bool keyDown )
{
	keyinfo_t& keyInfo = m_keyInfoArray[button];
	// Set the key down state
	keyInfo.isDown = keyDown;

	if(keyDown)
		keyInfo.nbRepeats++;
	else
		keyInfo.nbRepeats = 0;

	// Toggleconsole key is handled specially
	if(keyDown && !qstrcmp(keyInfo.binding, CConsole::TOGGLECONSOLE_CMD_NAME))
	{
		gConsole.ToggleConsole();
		return;
	}

	// Menu is handled specially
	if(gMenu.IsActive())
	{
		// Don't allow repeats on the escape key
		if(button == SDL_SCANCODE_ESCAPE && keyInfo.nbRepeats > 1)
			return;

		// Handle escape specially
		if(button == SDL_SCANCODE_ESCAPE && keyDown)
		{
			gMenu.HideMenu();
			return;
		}

		// First try to manage any UI windows
		if(gUIManager.KeyEvent(button, mod, keyDown))
			return;

		// Now send it to the menu
		gMenu.KeyEvent(button, mod, keyDown);
		return;
	}
	else if(button == SDL_SCANCODE_ESCAPE && !cls.dllfuncs.pfnIsEscapeKeyOverridden())
	{
		if(!keyDown)
			return;

		if(!gMenu.IsActive())
		{
			// Show the menu
			gMenu.ShowMenu();
			return;
		}
	}

	// Allow client to steal inputs
	if(cls.dllfuncs.pfnIsInputOverridden())
	{
		cls.dllfuncs.pfnKeyEvent(button, mod, keyDown);
		return;
	}

	// See if the UI can manage it
	if(gUIManager.KeyEvent(button, mod, keyDown))
		return;

	// Get SDL Keycode
	SDL_Keycode sdlKeycode = SDL_GetKeyFromScancode((SDL_Scancode)button);

	// Reset this
	if(keyDown)
	{
		// Ignore most autorepeats(as in Quake)
		if(sdlKeycode != SDLK_BACKSPACE && sdlKeycode != SDLK_PAUSE && keyInfo.nbRepeats > 1)
			return;

		// Warn about special buttons being unbound
		if(button >= SDL_SCANCODE_RETURN && keyInfo.binding.empty())
			Con_Printf("'%s' is unbound.\n", keyInfo.name.c_str());
	}

	// Handle shift key
	if(sdlKeycode == SDLK_RSHIFT || sdlKeycode == SDLK_LSHIFT)
		m_isShiftDown = keyDown;

	if(!keyDown && !keyInfo.binding.empty())
	{
		CString& keyBinding = keyInfo.binding;
		if(keyBinding[0] == '+')
		{
			CString keyCommand;
			keyCommand << '-' << keyBinding.c_str()+1 << " " << (Int32)button;
			
			// Add it to the buffer
			gCommands.AddCommand(keyCommand.c_str());
		}
	}

	if(keyDown && !keyInfo.binding.empty())
	{
		CString& keyBinding = keyInfo.binding;
		if(keyBinding[0] == '+')
		{
			CString keyCommand;
			keyCommand << keyBinding.c_str() << " " << (Int32)button;
			
			// Add it to the buffer
			gCommands.AddCommand(keyCommand.c_str());
		}
		else
		{
			// Add it to the buffer
			gCommands.AddCommand(keyBinding.c_str());
		}
	}
}

//=============================================
// @brief Handles a mouse button event
//
//=============================================
void CInput::MouseButtonEvent( Int32 button, bool keyDown )
{
	// Handle UI events
	if(gUIManager.MouseButtonEvent(button, keyDown))
		return;

	// Handle menu/UI specially
	if(gMenu.IsActive())
	{
		gMenu.MouseButtonEvent(button, keyDown);
		return;
	}

	// Allow client to override
	if(cls.dllfuncs.pfnIsInputOverridden())
	{
		cls.dllfuncs.pfnMouseButtonEvent(button, keyDown);
		return;
	}

	Int32 keyinfoIdx = SDL_NUM_SCANCODES+button;
	keyinfo_t& keyInfo = m_keyInfoArray[keyinfoIdx];

	// Set the key down state
	keyInfo.isDown = keyDown;

	// Reset this
	if(keyDown)
	{
		keyInfo.nbRepeats++;
		if(keyInfo.nbRepeats > 1)
			return;
	}
	else
		keyInfo.nbRepeats = 0;

	// Print if it's empty
	if(keyInfo.binding.empty())
	{
		if(keyDown)
			Con_Printf("'%s' is unbound.\n", keyInfo.name.c_str());

		return;
	}

	CString& keyBinding = keyInfo.binding;
	if(!keyDown && keyBinding[0] == '+')
	{
		CString keyCommand;
		keyCommand << '-' << keyBinding.c_str()+1 << " " << (Int32)keyinfoIdx;
			
		// Add it to the buffer
		gCommands.AddCommand(keyCommand.c_str());
	}
	else
	{
		if(keyBinding[0] == '+')
		{
			CString keyCommand;
			keyCommand << keyBinding.c_str() << " " << (Int32)keyinfoIdx;
			
			// Add it to the buffer
			gCommands.AddCommand(keyCommand.c_str());
		}
		else
		{
			// Add it to the buffer
			gCommands.AddCommand(keyBinding.c_str());
		}
	}
}

//=============================================
// @brief Handles a mouse wheel event
//
//=============================================
void CInput::MouseWheelEvent( Int32 button, bool keyDown, Int32 scroll )
{
	// Handle UI events
	if(gUIManager.MouseWheelEvent(button, keyDown, scroll))
		return;

	// Handle menu/UI specially
	if(gMenu.IsActive())
	{
		gMenu.MouseWheelEvent(button, keyDown, scroll);
		return;
	}

	// Allow client to override
	if(cls.dllfuncs.pfnIsInputOverridden())
	{
		cls.dllfuncs.pfnMouseWheelEvent(button, keyDown, scroll);
		return;
	}

	Int32 keyinfoIdx = SDL_NUM_SCANCODES+NB_MOUSE_BTN+(button+1);
	keyinfo_t& keyInfo = m_keyInfoArray[keyinfoIdx];

	// Set the key down state
	keyInfo.isDown = keyDown;

	// Reset this
	if(keyDown)
	{
		keyInfo.nbRepeats++;
		if(keyInfo.nbRepeats > 1)
			return;
	}
	else
		keyInfo.nbRepeats = 0;

	// Print if it's empty
	if(keyInfo.binding.empty())
	{
		if(keyDown)
			Con_Printf("'%s' is unbound.\n", keyInfo.name.c_str());

		return;
	}

	CString& keyBinding = keyInfo.binding;
	if(!keyDown && keyBinding[0] == '+')
	{
		CString keyCommand;
		keyCommand << '-' << keyBinding.c_str()+1 << " " << (Int32)keyinfoIdx;
			
		// Add it to the buffer
		gCommands.AddCommand(keyCommand.c_str());
	}
	else
	{
		if(keyBinding[0] == '+')
		{
			CString keyCommand;
			keyCommand << keyBinding.c_str() << " " << (Int32)keyinfoIdx;
			
			// Add it to the buffer
			gCommands.AddCommand(keyCommand.c_str());
		}
		else
		{
			// Add it to the buffer
			gCommands.AddCommand(keyBinding.c_str());
		}
	}
}

//=============================================
// @brief Returns the distance the mouse traveled on x and y axes
//
//=============================================
void CInput::GetMouseDelta( Int32 &deltaX, Int32 &deltaY )
{
	deltaX = 0;
	deltaY = 0;

	if(!gWindow.IsActive())
		return;

	SDL_Window* pWindow = gWindow.GetWindow();
	if(!pWindow)
		return;

	// For cursor offset
	Int32 x, y;
	SDL_GetWindowPosition(pWindow, &x, &y);

	// Store old mouse position
	for(Uint32 i = 0; i < 2; i++)
		m_oldMousePosition[i] = m_mousePosition[i];
	
	Int32 centerX = gWindow.GetCenterX();
	Int32 centerY = gWindow.GetCenterY();

	// Get relative positions
	if(m_resetMouse)
	{
		m_resetMouse = false;
	}
	else
	{
		POINT p;
		GetCursorPos(&p);

		deltaX = (p.x-x) - centerX;
		deltaY = (p.y-y) - centerY;
	}

	m_mousePosition[0] += deltaX;
	m_mousePosition[1] += deltaY;

	// Cap at limits
	Uint32 winWidth = gWindow.GetWidth();
	if(m_mousePosition[0] < 0)
		m_mousePosition[0] = 0;
	else if(m_mousePosition[0] > (Int32)winWidth)
		m_mousePosition[0] = winWidth;

	Uint32 winHeight = gWindow.GetHeight();
	if(m_mousePosition[1] < 0)
		m_mousePosition[1] = 0;
	else if(m_mousePosition[1] > (Int32)winHeight)
		m_mousePosition[1] = winHeight;

	// Reposition mouse on center
	SetCursorPos(x + centerX, y + centerY);
}

//=============================================
// @brief Updates mouse position
//
//=============================================
void CInput::UpdateMousePositions( bool clearReset )
{
	if(!gWindow.IsActive())
		return;

	SDL_Window* pWindow = gWindow.GetWindow();
	if(!pWindow)
		return;

	// For cursor offset
	Int32 x, y;
	SDL_GetWindowPosition(pWindow, &x, &y);

	// Store old mouse position
	for(Uint32 i = 0; i < 2; i++)
		m_oldMousePosition[i] = m_mousePosition[i];
	
	Int32 centerX = gWindow.GetCenterX();
	Int32 centerY = gWindow.GetCenterY();

	Int32 deltaX = 0;
	Int32 deltaY = 0;

	// Get relative positions
	if(m_resetMouse)
	{
		if(clearReset)
			m_resetMouse = false;

		m_oldMousePosition[0] = centerX;
		m_oldMousePosition[1] = centerY;
	}
	else
	{
		POINT p;
		GetCursorPos(&p);

		deltaX = (p.x-x) - centerX;
		deltaY = (p.y-y) - centerY;
	}

	m_mousePosition[0] += deltaX;
	m_mousePosition[1] += deltaY;

	// Cap at limits
	Uint32 winWidth = gWindow.GetWidth();
	if(m_mousePosition[0] < 0)
		m_mousePosition[0] = 0;
	else if(m_mousePosition[0] > (Int32)winWidth)
		m_mousePosition[0] = winWidth;

	Uint32 winHeight = gWindow.GetHeight();
	if(m_mousePosition[1] < 0)
		m_mousePosition[1] = 0;
	else if(m_mousePosition[1] > (Int32)winHeight)
		m_mousePosition[1] = winHeight;

	// Reposition mouse on center
	SetCursorPos(x + centerX, y + centerY);
}

//=============================================
// @brief Resets the mouse
//
//=============================================
void CInput::ResetMouse( void )
{
	if(!gWindow.IsActive())
		return;
	
	Int32 centerX = gWindow.GetCenterX();
	Int32 centerY = gWindow.GetCenterY();

	m_oldMousePosition[0] = m_mousePosition[0] = centerX;
	m_oldMousePosition[1] = m_mousePosition[1] = centerY;

	m_resetMouse = true;

	SDL_Window* pWindow = gWindow.GetWindow();
	if(!pWindow)
		return;

	SDL_WarpMouseInWindow(pWindow, centerX, centerY);
}

//=============================================
// @brief Resets theinput class
//
//=============================================
void CInput::GetMousePosition( Int32& x, Int32& y )
{
	SDL_GetMouseState(&x, &y);
}

//=============================================
// Class: CConfig
// Function: GetKeynameForBind
//=============================================
const Char* CInput::GetKeynameForBind( const Char* pstrBind )
{
	for(Uint32 i = 0; i < m_keyInfoArray.size(); i++)
	{
		keyinfo_t& keyInfo = m_keyInfoArray[i];
		if(keyInfo.binding.empty())
			continue;

		if(!qstrcmp(keyInfo.binding, pstrBind))
			return keyInfo.name.c_str();
	}

	return nullptr;
}

//=============================================
// Class: CConfig
// Function: GetKeynameForScancode
//=============================================
const Char* CInput::GetKeynameForScancode( Int32 scancodeIdx )
{
	assert((Uint32)scancodeIdx < m_keyInfoArray.size());
	return m_keyInfoArray[scancodeIdx].name.c_str();
}

//=============================================
// Class: CConfig
// Function: GetMouseButtonName
//=============================================
const Char* CInput::GetMouseButtonName( Int32 button )
{
	assert(button < NB_MOUSE_BTN);
	Uint32 scancodeIdx = SDL_NUM_SCANCODES + button;
	return m_keyInfoArray[scancodeIdx].name.c_str();
}

//=============================================
// Class: CConfig
// Function: CmdBindKey
//=============================================
void CInput::CmdBindKey( void )
{
	// Make sure the params are ok
	if(gCommands.Cmd_Argc() != 3)
	{
		Con_Printf("'bind' usage: bind <key name> <command>\n");
		return;
	}

	// Get the parameters
	const Char* pstrKeyName = gCommands.Cmd_Argv(1);
	const Char* pstrCmdName = gCommands.Cmd_Argv(2);

	// Clear key from other binds
	for(Uint32 i = 0; i < m_keyInfoArray.size(); i++)
	{
		if(!qstrcmp(m_keyInfoArray[i].binding, pstrCmdName))
		{
			gConfig.DeleteField(CConfig::USER_CONFIG_GRP_NAME, m_keyInfoArray[i].name.c_str());
			m_keyInfoArray[i].binding.clear();
		}
	}

	// Try to bind it
	Uint32 i = 0;
	for(; i < m_keyInfoArray.size(); i++)
	{
		keyinfo_t& keyInfo = m_keyInfoArray[i];
		if(!qstrcicmp(keyInfo.name, pstrKeyName))
		{
			// Never allow binding over the grave key
			if(i == SDL_SCANCODE_GRAVE && qstrcmp(pstrCmdName, CConsole::TOGGLECONSOLE_CMD_NAME))
			{
				Con_EPrintf("Binding over this key is not allowed.\n");
				return;
			}

			keyInfo.binding = pstrCmdName;
			break;
		}
	}

	if(i == m_keyInfoArray.size())
	{
		Con_EPrintf("%s is not a valid key.\n", pstrKeyName);
		return;
	}

	// Set it in the config too
	gConfig.SetValue(CConfig::USER_CONFIG_GRP_NAME, pstrKeyName, pstrCmdName, true, CONF_FIELD_KEYBIND);
}

//=============================================
// Class: CConfig
// Function: CmdUnbindKey
//=============================================
void CInput::CmdUnbindKey( void )
{
	// Make sure the params are ok
	if(gCommands.Cmd_Argc() != 2)
	{
		Con_Printf("'unbind' usage: unbind <key name>\n");
		return;
	}

	const Char* pstrKeyName = gCommands.Cmd_Argv(1);

	// Find the key
	Uint32 i = 0;
	for(; i < m_keyInfoArray.size(); i++)
	{
		keyinfo_t& keyInfo = m_keyInfoArray[i];
		if(!qstrcicmp(keyInfo.name, pstrKeyName))
		{
			// Never allow unbinding for this key
			if(i == SDL_SCANCODE_GRAVE && !qstrcmp(keyInfo.binding.c_str(), CConsole::TOGGLECONSOLE_CMD_NAME))
			{
				Con_EPrintf("Unbinding this key is not allowed.\n");
				return;
			}

			keyInfo.binding.clear();
			break;
		}
	}

	if(i == m_keyInfoArray.size())
	{
		Con_EPrintf("%s is not a valid key.\n", pstrKeyName);
		return;
	}

	// Set it in the config too
	gConfig.DeleteField(CConfig::USER_CONFIG_GRP_NAME, pstrKeyName);
}

//=============================================
// Class: CConfig
// Function: CmdUnbindAll
//=============================================
void CInput::CmdUnbindAll( void )
{
	// Clear all the binds
	for(Uint32 i = 0; i < m_keyInfoArray.size(); i++)
	{
		keyinfo_t& keyInfo = m_keyInfoArray[i];
		if(keyInfo.binding.empty())
			continue;

		// Never unbind the console key
		if(i == SDL_SCANCODE_GRAVE && !qstrcmp(keyInfo.binding.c_str(), CConsole::TOGGLECONSOLE_CMD_NAME))
			continue;

		// Clear it out
		gConfig.DeleteField(CConfig::USER_CONFIG_GRP_NAME, keyInfo.name.c_str());
		keyInfo.binding.clear();
	}
}
