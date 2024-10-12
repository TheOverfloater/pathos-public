/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "file.h"
#include "window.h"
#include "input.h"

#include "texturemanager.h"
#include "r_vbo.h"
#include "r_glsl.h"
#include "r_basicdraw.h"
#include "r_text.h"
#include "r_menu.h"
#include "r_common.h"

#include "uielements.h"
#include "uimanager.h"
#include "uiconsolewindow.h"
#include "console.h"
#include "commands.h"
#include "config.h"
#include "enginestate.h"
#include "logfile.h"
#include "system.h"
#include "cl_main.h"
#include "r_main.h"
#include "filewriterthread.h"
#include "textschemas.h"

// Log file pointer
extern CLogFile* g_pLogFile;

extern void SV_ClientPrintf( const struct edict_t* pclient, const Char *fmt, ... );

// How much to increase the buffer's size by each time
const Uint32 CConsole::CONSOLE_BUFFER_CHUNKSIZE = 1024*1024*8;
// Name of the toggleconsole command
const Char CConsole::TOGGLECONSOLE_CMD_NAME[] = "toggleconsole";

// Max remembered last prints
const Uint32 CConsole::MAX_DEBUG_PRINTS = 10;
// Print lifetime
const Float CConsole::DEBUG_PRINT_LIFETIME = 15;

// Print box reference width
const Uint32 CConsole::PRINT_BOX_REF_X_OFFSET = 20;
// Print box reference width
const Uint32 CConsole::PRINT_BOX_REF_Y_OFFSET = 60;
// Print box text inset
const Uint32 CConsole::PRINT_BOX_INSET = 5;

// Console reference width
const Uint32 CConsole::CONSOLE_REF_RESOLUTION_WIDTH = 1920;
// Print box reference height
const Uint32 CConsole::CONSOLE_REF_RESOLUTION_HEIGHT = 1080;

// Print box reference width
const Uint32 CConsole::PRINT_BOX_REF_WIDTH = 800;
// Print box reference width
const Uint32 CConsole::PRINT_BOX_REF_HEIGHT = 200;
// Print box reference bar thickness
const Uint32 CConsole::PRINT_BOX_REF_BAR_THICKNESS = 5;

// Print box bar color
const color32_t CConsole::PRINT_BOX_BAR_COLOR = color32_t(150, 0, 0, 100);
// Print box background color
const color32_t CConsole::PRINT_BOX_BACKGROUND_COLOR = color32_t(0, 0, 0, 50);

// Text schema name for console text box
const Char CConsole::TEXTBOX_TEXTSCHEMA_NAME[] = "consolebox";

// Class definition
CConsole gConsole;

//
// Commands used by the console
//
void Cmd_ToggleConsole( void ) { gConsole.ToggleConsole(); }
void Cmd_ClearHistory( void ) { gConsole.CmdClearHistory(); }
void Cmd_CVarList( void ) { gConsole.CmdInputList(INPUT_CVAR); }
void Cmd_CmdList( void ) { gConsole.CmdInputList(INPUT_CMD); }
void Cmd_ConDump( void ) { gConsole.CmdConDump(); }

//=============================================
// @brief Constructor
//
//=============================================
CConsole::CConsole( void ):
	m_pConsoleBuffer(nullptr),
	m_bufferSize(0),
	m_bufferPosition(0),
	m_reachedHistoryEnd(false),
	m_isHistoryUpdated(true),
	m_pCvarDrawHistoryBox(nullptr),
	m_pFontSet(nullptr)
{
	// Allocate the buffer
	m_pConsoleBuffer = new Char[CONSOLE_BUFFER_CHUNKSIZE];
	memset(m_pConsoleBuffer, 0, sizeof(Char)*CONSOLE_BUFFER_CHUNKSIZE);
	m_bufferSize = CONSOLE_BUFFER_CHUNKSIZE;
}

//=============================================
// @brief Constructor
//
//=============================================
CConsole::CConsole( CConsole& src ):
	m_pConsoleBuffer(nullptr),
	m_bufferSize(src.m_bufferSize),
	m_bufferPosition(src.m_bufferPosition),
	m_inputHistoryList(src.m_inputHistoryList),
	m_reachedHistoryEnd(src.m_reachedHistoryEnd),
	m_isHistoryUpdated(src.m_isHistoryUpdated),
	m_cvarList(src.m_cvarList),
	m_inputList(src.m_inputList),
	m_listFilterString(src.m_listFilterString),
	m_debugPrintList(src.m_debugPrintList),
	m_pCvarDrawHistoryBox(src.m_pCvarDrawHistoryBox)
{
	// Allocate the buffer
	m_pConsoleBuffer = new Char[m_bufferSize];
	memcpy(m_pConsoleBuffer, src.m_pConsoleBuffer, sizeof(Char)*m_bufferSize);
}

//=============================================
// @brief Destructor
//
//=============================================
CConsole::~CConsole( void )
{
	if(m_pConsoleBuffer)
		delete m_pConsoleBuffer;

	if(!m_inputHistoryList.empty())
	{
		m_inputHistoryList.begin();
		while(!m_inputHistoryList.end())
			m_inputHistoryList.next();

		m_inputHistoryList.clear();
	}

	if(!m_cvarList.empty())
	{
		m_cvarList.begin();
		while(!m_cvarList.end())
		{
			delete m_cvarList.get();
			m_cvarList.next();
		}
	}
}

//=============================================
// @brief Constructor
//
//=============================================
CConsole& CConsole::operator=( CConsole& src )
{
	if(&src == this)
		return *this;

	if(m_pConsoleBuffer)
		delete[] m_pConsoleBuffer;

	m_bufferSize = src.m_bufferSize;
	m_bufferPosition = src.m_bufferPosition;
	m_inputHistoryList = src.m_inputHistoryList;
	m_inputList = src.m_inputList;
	m_cvarList = src.m_cvarList;
	m_listFilterString = src.m_listFilterString;
	m_debugPrintList = src.m_debugPrintList;
	m_reachedHistoryEnd = src.m_reachedHistoryEnd;
	m_isHistoryUpdated = src.m_isHistoryUpdated;
	m_pCvarDrawHistoryBox = src.m_pCvarDrawHistoryBox;

	// Allocate the buffer
	m_pConsoleBuffer = new Char[m_bufferSize];
	memcpy(m_pConsoleBuffer, src.m_pConsoleBuffer, sizeof(Char)*m_bufferSize);
	return *this;
}

//=============================================
// @brief Initializes the console
//
//=============================================
void CConsole::Init( void )
{
	// Create the commands relevant to the console
	gCommands.CreateCommand(TOGGLECONSOLE_CMD_NAME, Cmd_ToggleConsole, "Toggles the console");
	gCommands.CreateCommand("clear", Cmd_ClearHistory, "Clears the console history");
	gCommands.CreateCommand("cvarlist", Cmd_CVarList, "Prints the list of cvars to the console, or to a file");
	gCommands.CreateCommand("cmdlist", Cmd_CmdList, "Prints a list of commands to the console, or to a file");
	gCommands.CreateCommand("condump", Cmd_ConDump, "Dumps the console contents to a file");

	m_pCvarDrawHistoryBox = CreateCVar(CVAR_FLOAT, (FL_CV_CLIENT|FL_CV_SAVE), "con_historybox", "0", "Draws recent console history box");
}

//=============================================
// @brief Initializes OpenGL stuff
//
//=============================================
bool CConsole::InitGL( void )
{
	m_pFontSet = gTextSchemas.GetResolutionSchemaFontSet(TEXTBOX_TEXTSCHEMA_NAME, gWindow.GetHeight());
	if(!m_pFontSet)
		m_pFontSet = gText.GetDefaultFont();

	return true;
}

//=============================================
// @brief Adds text to the history buffer
//
//=============================================
void CConsole::AddTextHistory( const Char* pstrText )
{
	if(!pstrText)
		return;

	// Resize the array if needed
	Uint32 stringLength = qstrlen(pstrText);
	if(m_bufferPosition+stringLength+1 > m_bufferSize)
	{
		void* pnew = Common::ResizeArray(m_pConsoleBuffer, sizeof(Char), m_bufferSize, CONSOLE_BUFFER_CHUNKSIZE);
		m_pConsoleBuffer = static_cast<Char*>(pnew);
		m_bufferSize += CONSOLE_BUFFER_CHUNKSIZE;
	}

	// Add it to the buffer
	memcpy(m_pConsoleBuffer + m_bufferPosition, pstrText, sizeof(Char)*stringLength);
	m_bufferPosition += stringLength;

	// Set terminator
	m_pConsoleBuffer[m_bufferPosition] = '\0';

	// Update the console window, if it's active at all
	CUIConsoleWindow* pWindow = CUIConsoleWindow::GetInstance();
	if(pWindow && pWindow->isVisible())
	{
		// Needs to be called each time to adjust the scroller
		pWindow->SetHistoryText(m_pConsoleBuffer);
		m_isHistoryUpdated = true;
	}
	else
	{
		// Needs to be updated menu return
		m_isHistoryUpdated = false;
	}

	if(m_pCvarDrawHistoryBox && m_pCvarDrawHistoryBox->GetValue() >= 1)
	{
		// Add debug prints
		if(m_debugPrintList.size() == MAX_DEBUG_PRINTS)
		{
			m_debugPrintList.rbegin();
			m_debugPrintList.remove(m_debugPrintList.get_link());
		}

		debug_print_t newprint;
		newprint.text = pstrText;
		newprint.time = ens.time;

		m_debugPrintList.add(newprint);
	}
}

//=============================================
// @brief Processes a command input
//
//=============================================
void CConsole::ProcessInput( const Char* pstrText )
{
	// Add it to the text history
	CString historyText;
	historyText << '\\' << pstrText << '\n';

	AddTextHistory(historyText.c_str());

	if(pstrText[0] != '\0')
	{
		// Add it to the input history
		m_inputHistoryList.add(pstrText);
		m_inputHistoryList.begin(); // Reset iterator
		m_reachedHistoryEnd = false;

		// Send it to the buffer
		gCommands.AddCommand(pstrText);

		// Clear tabbing
		ResetTabbing();
	}
}

//=============================================
// @brief Gets the currently iterated input from the history
//
//=============================================
const Char* CConsole::GetCurrentInputHistory( void )
{
	if(m_inputHistoryList.empty()
		|| m_inputHistoryList.end()
		|| m_reachedHistoryEnd)
		return nullptr;

	return m_inputHistoryList.get().c_str();
}

//=============================================
// @brief Advances along the text history
//
//=============================================
void CConsole::HistoryStepForward( void )
{
	if(m_inputHistoryList.empty())
		return;

	if(m_inputHistoryList.end())
		m_inputHistoryList.begin();

	m_reachedHistoryEnd = false;

	// Only advance if this isn't the last element
	CLinkedList<CString>::link_t* link = m_inputHistoryList.get_link();
	if(!link->last())
		m_inputHistoryList.next();
}

//=============================================
// @brief Backtraces along the text history
//
//=============================================
void CConsole::HistoryStepBack( void )
{
	if(m_inputHistoryList.empty())
		return;

	if(!m_inputHistoryList.end())
		m_inputHistoryList.prev();
	else
		m_reachedHistoryEnd = true;
}

//=============================================
// @brief Performs think functions
//
//=============================================
void CConsole::Think( void )
{
	if(!m_debugPrintList.empty())
	{
		m_debugPrintList.begin();
		while(!m_debugPrintList.end())
		{
			const debug_print_t& print = m_debugPrintList.get();
			if(print.time + DEBUG_PRINT_LIFETIME < ens.time)
				m_debugPrintList.remove(m_debugPrintList.get_link());

			m_debugPrintList.next();
		}
	}

	if(!m_glDependentCVarCommandsArray.empty())
	{
		if(gWindow.IsInitialized())
		{
			for(Uint32 i = 0; i < m_glDependentCVarCommandsArray.size(); i++)
			{
				CString& cmd = m_glDependentCVarCommandsArray[i];
				gCommands.ExecuteCommand(cmd.c_str(), false);
			}

			m_glDependentCVarCommandsArray.clear();
		}
	}

	// Get any prints from file writer thread
	CArray<CString> writerThreadPrintsArray;
	FWT_GetConsolePrints(writerThreadPrintsArray);

	if(!writerThreadPrintsArray.empty())
	{
		for(Uint32 i = 0; i < writerThreadPrintsArray.size(); i++)
			Con_Printf(writerThreadPrintsArray[i].c_str());
	}
}

//=============================================
// @brief Performs think functions
//
//=============================================
bool CConsole::Draw( void )
{
	if(m_pCvarDrawHistoryBox->GetValue() < 1)
		return true;

	if(m_debugPrintList.empty())
		return true;

	// Get box offsets
	Float boxXOrigin = R_GetRelativeX(PRINT_BOX_REF_X_OFFSET, CONSOLE_REF_RESOLUTION_WIDTH, rns.screenwidth);
	Float boxYOrigin = R_GetRelativeY(PRINT_BOX_REF_Y_OFFSET, CONSOLE_REF_RESOLUTION_HEIGHT, rns.screenheight);

	// Get relative sizes
	Float boxWidth = R_GetRelativeX(PRINT_BOX_REF_WIDTH, CONSOLE_REF_RESOLUTION_WIDTH, rns.screenwidth);
	Float boxHeight = R_GetRelativeY(PRINT_BOX_REF_HEIGHT, CONSOLE_REF_RESOLUTION_HEIGHT, rns.screenheight);
	Float barThickness = R_GetRelativeY(PRINT_BOX_REF_BAR_THICKNESS, CONSOLE_REF_RESOLUTION_HEIGHT, rns.screenheight);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	// Draw the box
	CBasicDraw* pDraw = CBasicDraw::GetInstance();
	if(!pDraw)
		return false;

	if(!pDraw->Enable() || !pDraw->DisableTexture())
		return false;

	// Set matrices
	rns.view.modelview.LoadIdentity();
	rns.view.modelview.Scale(1.0f/rns.screenwidth, 1.0f/rns.screenheight, 1.0);

	rns.view.projection.LoadIdentity();
	rns.view.projection.Ortho(GL_ZERO, GL_ONE, GL_ONE, GL_ZERO, static_cast<Float>(0.1), 100);

	pDraw->SetModelview(rns.view.modelview.GetMatrix());
	pDraw->SetProjection(rns.view.projection.GetMatrix());

	pDraw->Color4f(static_cast<Float>(PRINT_BOX_BAR_COLOR.r)/255.0f,
		static_cast<Float>(PRINT_BOX_BAR_COLOR.g)/255.0f,
		static_cast<Float>(PRINT_BOX_BAR_COLOR.b)/255.0f,
		static_cast<Float>(PRINT_BOX_BAR_COLOR.a)/255.0f);

	// Draw top bar
	Float xCoord = boxXOrigin;
	Float yCoord = boxYOrigin;
	Float objWidth = boxWidth;
	Float objHeight = barThickness;

	pDraw->Begin(CBasicDraw::DRAW_TRIANGLES);
	pDraw->Vertex3f(xCoord, yCoord, -1.0);
	pDraw->Vertex3f(xCoord+objWidth, yCoord, -1.0);
	pDraw->Vertex3f(xCoord, yCoord+objHeight, -1.0);

	pDraw->Vertex3f(xCoord, yCoord+objHeight, -1.0);
	pDraw->Vertex3f(xCoord+objWidth, yCoord, -1.0);
	pDraw->Vertex3f(xCoord+objWidth, yCoord+objHeight, -1.0);
	pDraw->End();

	// Draw left bar
	xCoord = boxXOrigin;
	yCoord = boxYOrigin + barThickness;

	objWidth = barThickness;
	objHeight = boxHeight - barThickness*3;

	pDraw->Begin(CBasicDraw::DRAW_TRIANGLES);
	pDraw->Vertex3f(xCoord, yCoord, -1.0);
	pDraw->Vertex3f(xCoord+objWidth, yCoord, -1.0);
	pDraw->Vertex3f(xCoord, yCoord+objHeight, -1.0);

	pDraw->Vertex3f(xCoord, yCoord+objHeight, -1.0);
	pDraw->Vertex3f(xCoord+objWidth, yCoord, -1.0);
	pDraw->Vertex3f(xCoord+objWidth, yCoord+objHeight, -1.0);
	pDraw->End();

	// Draw bottom bar
	xCoord = boxXOrigin;
	yCoord = boxYOrigin + boxHeight - barThickness*2;

	objWidth = boxWidth;
	objHeight = barThickness;

	pDraw->Begin(CBasicDraw::DRAW_TRIANGLES);
	pDraw->Vertex3f(xCoord, yCoord, -1.0);
	pDraw->Vertex3f(xCoord+objWidth, yCoord, -1.0);
	pDraw->Vertex3f(xCoord, yCoord+objHeight, -1.0);

	pDraw->Vertex3f(xCoord, yCoord+objHeight, -1.0);
	pDraw->Vertex3f(xCoord+objWidth, yCoord, -1.0);
	pDraw->Vertex3f(xCoord+objWidth, yCoord+objHeight, -1.0);
	pDraw->End();

	// Draw right bar
	xCoord = boxXOrigin + boxWidth - barThickness;
	yCoord = boxYOrigin + barThickness;

	objWidth = barThickness;
	objHeight = boxHeight - barThickness*3;

	pDraw->Begin(CBasicDraw::DRAW_TRIANGLES);
	pDraw->Vertex3f(xCoord, yCoord, -1.0);
	pDraw->Vertex3f(xCoord+objWidth, yCoord, -1.0);
	pDraw->Vertex3f(xCoord, yCoord+objHeight, -1.0);

	pDraw->Vertex3f(xCoord, yCoord+objHeight, -1.0);
	pDraw->Vertex3f(xCoord+objWidth, yCoord, -1.0);
	pDraw->Vertex3f(xCoord+objWidth, yCoord+objHeight, -1.0);
	pDraw->End();

	// Draw background
	pDraw->Color4f(static_cast<Float>(PRINT_BOX_BACKGROUND_COLOR.r)/255.0f,
		static_cast<Float>(PRINT_BOX_BACKGROUND_COLOR.g)/255.0f,
		static_cast<Float>(PRINT_BOX_BACKGROUND_COLOR.b)/255.0f,
		static_cast<Float>(PRINT_BOX_BACKGROUND_COLOR.a)/255.0f);

	xCoord = boxXOrigin + barThickness;
	yCoord = boxYOrigin + barThickness;
	objHeight = boxHeight - barThickness*3;
	objWidth = boxWidth - barThickness*2;
	
	pDraw->Begin(CBasicDraw::DRAW_TRIANGLES);
	pDraw->Vertex3f(xCoord, yCoord, -1.0);
	pDraw->Vertex3f(xCoord+objWidth, yCoord, -1.0);
	pDraw->Vertex3f(xCoord, yCoord+objHeight, -1.0);

	pDraw->Vertex3f(xCoord, yCoord+objHeight, -1.0);
	pDraw->Vertex3f(xCoord+objWidth, yCoord, -1.0);
	pDraw->Vertex3f(xCoord+objWidth, yCoord+objHeight, -1.0);
	pDraw->End();

	pDraw->Disable();

	glDisable(GL_BLEND);

	// Get default font set
	if(!gText.Prepare())
		return false;

	if(!gText.BindSet(m_pFontSet))
		return false;

	Int32 stringBoxXPos = boxXOrigin + barThickness;
	Int32 stringBoxYPos = boxYOrigin + barThickness;

	gText.SetRectangle(0, 0, objWidth, objHeight, PRINT_BOX_INSET, 0);

	Int32 yoffset = objHeight - PRINT_BOX_INSET;
	bool result = true;

	m_debugPrintList.begin();
	while(!m_debugPrintList.end())
	{
		debug_print_t& dp = m_debugPrintList.get();
		Int32 height = gText.EstimateHeight(m_pFontSet, dp.text.c_str(), m_pFontSet->fontsize);

		yoffset -= height;
		if(yoffset < 0)
			break;

		result = gText.DrawString(m_pFontSet, dp.text.c_str(), stringBoxXPos, stringBoxYPos+yoffset);
		if(!result)
			return false;

		m_debugPrintList.next();
	}

	gText.SetRectangle(0, 0, 0, 0, 0, 0);

	gText.UnBind(m_pFontSet);
	gText.Reset();

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);

	return result;
}

//=============================================
// @brief Shows the console
//
//=============================================
void CConsole::ShowConsole( void )
{
	CUIConsoleWindow* pConsole = CUIConsoleWindow::GetInstance();
	if(!pConsole)
	{
		// Create the console window
		pConsole = CUIConsoleWindow::CreateInstance();
		if(!pConsole)
			Con_EPrintf("Failed to create console window.\n");
	}
	else
	{
		if(!pConsole->isVisible())
			pConsole->setVisible(true);

		// Change focus to this window
		gUIManager.SetFocusWindow(pConsole);
	}
}

//=============================================
// @brief Hides the console
//
//=============================================
void CConsole::HideConsole( void )
{
	CUIConsoleWindow* pConsole = CUIConsoleWindow::GetInstance();
	if(!pConsole)
		return;

	pConsole->setWindowFlags(CUIWindow::UIW_FL_KILLME);
}

//=============================================
// @brief Tells if the console is visible
//
//=============================================
bool CConsole::IsConsoleVisible( void )
{
	CUIConsoleWindow* pConsole = CUIConsoleWindow::GetInstance();
	if(!pConsole)
		return false;

	if(!pConsole->isVisible())
		return false;

	return true;
}

//=============================================
// @brief Tells if the console is visible
//
//=============================================
void CConsole::ToggleConsole( void )
{
	// If menu is not visible, open it!
	if(!gMenu.IsActive())
	{
		gMenu.ShowMenu();

		if(!IsConsoleVisible())
			ShowConsole();

		CUIConsoleWindow* pConsole = CUIConsoleWindow::GetInstance();
		if(pConsole)
			gUIManager.SetFocusWindow(pConsole);

		return;
	}

	if(IsConsoleVisible())
		HideConsole();
	else
		ShowConsole();
}

//=============================================
// @brief Tells if the console is visible
//
//=============================================
CCVar* CConsole::CreateCVar( cvar_type_t type, Int32 flags, const Char* pstrName, const Char* pstrValue, const Char* pstrDescription, pfnCVarCallback_t pfnCallback )
{
	// See if it's already added
	CCVar* pCVar = GetCVar(pstrName);
	if(pCVar)
	{
		Con_EPrintf("CVar %s is already added.\n", pstrName);
		return pCVar;
	}

	// Create the cvar
	if(type == CVAR_STRING)
		pCVar = new CStringCVar(flags, pstrName, pfnCallback, Con_EPrintf);
	else if(type == CVAR_FLOAT)
		pCVar = new CFloatCVar(flags, pstrName, pfnCallback, Con_EPrintf);

	if(!pCVar)
	{
		Con_EPrintf("Unknown cvar type specified for %s.\n", pstrName);
		return nullptr;
	}

	// Add to the input list
	AddInput(INPUT_CVAR, pstrName, pstrDescription);

	// Add it to the list
	m_cvarList.add(pCVar);

	// Set default value
	if(type == CVAR_STRING)
	{
		pCVar->SetValue(pstrValue);

		// Save to config if needed
		if(flags & FL_CV_SAVE)
		{
			gConfig.SetValue(CConfig::USER_CONFIG_GRP_NAME, pstrName, pstrValue, true);
			if(gConfig.GetStatus() != CONF_ERR_NONE)
				Con_EPrintf("Failed to set userconfig data for cvar '%s'.\n", pstrName);
		}
	}
	else
	{
		// Make sure the string is valid
		if(Common::IsNumber(pstrValue))
		{
			Float value = SDL_atof(pstrValue);
			pCVar->SetValue(value);

			// Save to config if needed
			if(flags & FL_CV_SAVE)
			{
				gConfig.SetValue(CConfig::USER_CONFIG_GRP_NAME, pstrName, value, true);
				if(gConfig.GetStatus() != CONF_ERR_NONE)
					Con_EPrintf("Failed to set userconfig data for cvar '%s'.\n", pstrName);
			}
		}
		else
			Con_EPrintf("Invalid value '%s' set for float cvar '%s'.\n", pstrValue, pstrName);
	}

	return pCVar;
}

//=============================================
// @brief Tells if the console is visible
//
//=============================================
CCVar* CConsole::GetCVar( const Char* pstrName )
{
	if(m_cvarList.empty())
		return nullptr;

	m_cvarList.begin();
	while(!m_cvarList.end())
	{
		CCVar* pCVar = m_cvarList.get();
		if(!qstrcmp(pCVar->GetName(), pstrName))
			return pCVar;

		m_cvarList.next();
	}

	return nullptr;
}

//=============================================
// @brief Adds an input option to the ordered list
//
//=============================================
void CConsole::AddInput( input_type_t type, const Char* name, const Char* description )
{
	// Check if it's already present
	m_inputList.begin();
	while(!m_inputList.end())
	{
		con_input_t& input = m_inputList.get();
		if(!qstrcmp(input.name, name))
		{
			Con_EPrintf("Duplicate entry for %s in console input list.\n", name);
			return;
		}

		m_inputList.next();
	}

	// Create the new entry
	con_input_t newInput;
	newInput.name = name;
	newInput.type = type;
	newInput.description = description;

	// Check if we have anything
	if(m_inputList.empty())
	{	
		m_inputList.add(newInput);
		return;
	}

	// See where we should put it
	m_inputList.begin();
	while(!m_inputList.end())
	{
		con_input_t& input = m_inputList.get();
		if(qstrcmp(input.name, name) > 0)
		{
			m_inputList.insert_before(m_inputList.get_link(), newInput);
			return;
		}

		m_inputList.next();
	}

	// Just add it at the end
	m_inputList.radd(newInput);
}

//=============================================
// @brief Attemps to process a command from the cmd buffer
//
//=============================================
bool CConsole::ProcessCommand( void )
{
	if(!gCommands.Cmd_Argc())
		return false;

	// Find the cvar
	const Char* pstrName = gCommands.Cmd_Argv(0);
	CCVar* pCVar = GetCVar(pstrName);
	if(!pCVar)
		return false;

	// Manage GL-dependent cvars
	if((pCVar->GetFlags() & FL_CV_GL_DEPENDENT) && !gWindow.IsInitialized())
	{
		CString fullCmd;
		for(Uint32 i = 0; i < gCommands.Cmd_Argc(); i++)
			fullCmd << " " << gCommands.Cmd_Argv(i);

		m_glDependentCVarCommandsArray.push_back(fullCmd);
		return true;
	}

	// Print status of the cvar to the console
	if(gCommands.Cmd_Argc() == 1)
	{
		CString printStr;
		printStr << "The value of '" << pstrName << "' is '";

		if(pCVar->GetType() == CVAR_STRING)
			printStr << pCVar->GetStrValue();
		else
			printStr << pCVar->GetValue();

		printStr << "'\n";

		// Print to the console
		Con_Printf(printStr.c_str());
	}
	else if(gCommands.Cmd_Argc() > 1)
	{
		// Make sure no clients try to modify it
		if(pCVar->GetFlags() & FL_CV_SV_ONLY && (CL_IsGameActive() && !CL_IsHostClient()))
		{
			Con_Printf("%s can only be modified by the server.\n", pCVar->GetName());
			return true;
		}

		const Char* pstrValue = gCommands.Cmd_Argv(1);
		if(pCVar->GetType() == CVAR_FLOAT)
		{
			Int32 sgn;
			if(pstrValue[0] == '-')
			{
				pstrValue++;
				sgn = -1;
			}
			else 
			{
				if(pstrValue[0] == '+')
					pstrValue++;

				sgn = 1;
			}

			// Make sure the string is valid
			if(!Common::IsNumber(pstrValue))
			{
				Con_EPrintf("Invalid value '%s' set for float cvar '%s'.\n", pstrValue, pstrName);
				return true;
			}

			Float value = atof(pstrValue)*sgn;
			if(!CVarSetFloatValue(pstrName, value))
				return false;
		}
		else if(!CVarSetStringValue(pstrName, pstrValue))
			return false;
	}

	return true;
}

//=============================================
// @brief Sets the value of a float-type convar
//
//=============================================
bool CConsole::CVarSetFloatValue( const Char* pstrName, Float value )
{
	// Find the cvar
	CCVar* pCVar = GetCVar(pstrName);
	if(!pCVar)
		return false;

	// Only change if needed
	if(pCVar->GetValue() == value)
		return true;

	// Convert and set
	if(!pCVar->SetValue(value))
	{
		Con_DPrintf("Failed to set cvar value.\n");
	}
	else if(pCVar->GetFlags() & FL_CV_NOTIFY)
	{
		if(CL_IsGameActive() && CL_IsHostClient() && (pCVar->GetFlags() & FL_CV_SV_ONLY))
			SV_ClientPrintf(nullptr, "'%s' set to '%f'.\n", pstrName, value);
		else
			Con_Printf("'%s' set to '%f'.\n", pstrName, value);
	}

	// Save to config if needed
	if(pCVar->GetFlags() & FL_CV_SAVE)
	{
		gConfig.SetValue(CConfig::USER_CONFIG_GRP_NAME, pstrName, value, true);
		if(gConfig.GetStatus() != CONF_ERR_NONE)
			Con_EPrintf("Failed to set userconfig data for cvar '%s'.\n", pstrName);
	}

	return true;
}

//=============================================
// @brief Sets the value of a string-type convar
//
//=============================================
bool CConsole::CVarSetStringValue( const Char* pstrName, const Char* pstrValue )
{
	// Find the cvar
	CCVar* pCVar = GetCVar(pstrName);
	if(!pCVar)
		return false;

	// Only set if needed
	if(!qstrcmp(pCVar->GetStrValue(), pstrValue))
		return true;

	// Set the value
	if(!pCVar->SetValue(pstrValue))
		Con_DPrintf("Failed to set cvar value.\n");
	else if(pCVar->GetFlags() & FL_CV_NOTIFY)
	{
		if(CL_IsGameActive() && CL_IsHostClient() && (pCVar->GetFlags() & FL_CV_SV_ONLY))
			SV_ClientPrintf(nullptr, "'%s' set to '%s'.\n", pstrName, pstrValue);
		else
			Con_Printf("'%s' set to '%s'.\n", pstrName, pstrValue);
	}

	// Save to config if needed
	if(pCVar->GetFlags() & FL_CV_SAVE)
	{
		gConfig.SetValue(CConfig::USER_CONFIG_GRP_NAME, pCVar->GetName(), pstrValue, true, CONF_FIELD_STRING);
		if(gConfig.GetStatus() != CONF_ERR_NONE)
			Con_EPrintf("Failed to set userconfig for cvar '%s'.\n", pstrName);
	}

	return true;
}

//=============================================
// @brief Returns the best input choice for a given string
//
//=============================================
const Char* CConsole::GetBestInputChoice( const Char* pstrFilterString, bool retry )
{
	if(m_inputList.empty() || !pstrFilterString)
		return nullptr;

	if(qstrcmp(m_listFilterString, pstrFilterString))
	{
		// Reset this
		m_inputList.begin();
		m_listFilterString = pstrFilterString;
	}

	// Start from the beginning
	if(m_inputList.end())
		m_inputList.begin();

	// Try to find the next best choice
	const Char* pstrBestOption = nullptr;
	while(!m_inputList.end())
	{
		con_input_t& conInput = m_inputList.get();
		if(!m_listFilterString.empty())
		{
			// Skip ahead until we find a match
			if(qstrncmp(m_listFilterString, conInput.name, m_listFilterString.length()))
			{
				m_inputList.next();
				continue;
			}
		}

		pstrBestOption = conInput.name.c_str();
		m_inputList.next();
		break;
	}

	// If we found anything
	if(pstrBestOption)
		return pstrBestOption;

	// Try again from the start
	if(retry)
		return GetBestInputChoice(pstrFilterString, false);

	return nullptr;
}

//=============================================
// @brief Returns the best input choice for a given string
//
//=============================================
void CConsole::ResetTabbing( void )
{
	// Go back to the beginning
	if(!m_inputList.empty())
		m_inputList.begin();

	if(!m_listFilterString.empty())
		m_listFilterString.clear();
}

//=============================================
// @brief Clears the console history
//
//=============================================
void CConsole::CmdClearHistory( void )
{
	m_pConsoleBuffer[0] = '\0';
	m_bufferPosition = 0;

	// Update the console window, if it's active at all
	CUIConsoleWindow* pWindow = CUIConsoleWindow::GetInstance();
	if(pWindow)
		pWindow->SetHistoryText(m_pConsoleBuffer);

	ResetTabbing();
}

//=============================================
// @brief Dumps a list of either input types to the console or to a file
//
//=============================================
void CConsole::CmdInputList( input_type_t type )
{
	// Optional filter argument
	CString strFilter;
	// TRUE if we need to write to file
	bool bLog = false;

	CString inputName;
	CString cmdName;
	if(type == INPUT_CVAR)
	{
		cmdName = "cvarlist";
		inputName = "commands";
	}
	else
	{
		cmdName = "cmdlist";
		inputName = "console variables";
	}

	// Get the options
	Uint32 i = 1;
	while(i < gCommands.Cmd_Argc())
	{
		const Char* pstrArg = gCommands.Cmd_Argv(i);
		if(!qstrcmp(pstrArg, "?"))
		{
			Con_Printf("%s - Prints a list of %s.\n", cmdName.c_str(), inputName.c_str());
			Con_Printf("Usage:");
			Con_Printf("\t%s [partial] - Lists all %s beginning with [partial].\n", cmdName.c_str(), inputName.c_str());
			Con_Printf("\t%s [*partial] - Lists all %s containing [partial] in their names.\n", cmdName.c_str(), inputName.c_str());
			Con_Printf("\t%s log [partial] - Writes all %s beginning with [partial] to %s.log in the game folder.\n", cmdName.c_str(), inputName.c_str(), cmdName.c_str());
			Con_Printf("\t%s log [*partial] - Writes all %s names containing [partial] in their names to %s.log in the game folder.\n", cmdName.c_str(), inputName.c_str(), cmdName.c_str());
			Con_Printf("%s ? for syntax.\n", cmdName.c_str());
			return;
		}
		else if(!qstrcmp(pstrArg, "log"))
			bLog = true;
		else if(strFilter.empty())
			strFilter = pstrArg;
		else
			break;

		i++;
	}

	// File buffer string
	CString* pfileBuffer = nullptr;
	if(bLog)
	{
		pfileBuffer = new CString();

		// Add the header
		if(type == INPUT_CVAR)
			(*pfileBuffer) << "Console variables list." << NEWLINE;
		else
			(*pfileBuffer) << "Command list." << NEWLINE;

		(*pfileBuffer) << "---------------" << NEWLINE;
	}
	else
	{
		if(type == INPUT_CVAR)
			Con_Printf("Console variables list.\n");
		else
			Con_Printf("Command list.\n");

		Con_Printf("---------------\n");
	}

	// List all the commands
	Uint32 nbPrinted = 0;
	m_inputList.begin();
	while(!m_inputList.end())
	{
		con_input_t& conInput = m_inputList.get();
		if(conInput.type != type)
		{
			m_inputList.next();
			continue;
		}

		if(!strFilter.empty())
		{
			const Char* pstr = strFilter.c_str();
			if(*pstr == '*')
			{
				if(!qstrstr(conInput.name.c_str(), pstr+1))
				{
					m_inputList.next();
					continue;
				}
			}
			else if(qstrncmp(pstr, conInput.name.c_str(), strFilter.length()))
			{
				m_inputList.next();
				continue;
			}
		}

		CString strOut;
		strOut << conInput.name;
		if(!conInput.description.empty())
			strOut << " - " << conInput.description;

		if(pfileBuffer)
			(*pfileBuffer) << strOut << NEWLINE;
		else
			Con_Printf("%s\n", strOut.c_str());

		nbPrinted++;

		m_inputList.next();
	}


	if(pfileBuffer)
	{
		(*pfileBuffer) << "---------------" << NEWLINE;
		(*pfileBuffer) << static_cast<Int32>(nbPrinted);
		
		CString filename;
		if(type == INPUT_CVAR)
		{
			(*pfileBuffer)<< " console variables" << NEWLINE;
			filename = "cvarlist.log";
		}
		else
		{
			(*pfileBuffer)<< " commands" << NEWLINE;
			filename = "cmdlist.log";
		}

		const byte* pdata = reinterpret_cast<const byte*>(pfileBuffer->c_str());
		FL_WriteFile(pdata, pfileBuffer->length(), filename.c_str());

		delete pfileBuffer;
	}
	else
	{
		Con_Printf("---------------\n");

		if(type == INPUT_CVAR)
			Con_Printf("%d console variables.\n", static_cast<Int32>(nbPrinted));
		else
			Con_Printf("%d commands.\n", static_cast<Int32>(nbPrinted));
	}

	Con_Printf("%s ? for syntax.\n", cmdName.c_str());
}

//=============================================
// @brief Dumps the contents of the console to a file
//
//=============================================
void CConsole::CmdConDump( void ) const
{
	if(m_bufferPosition <= 0)
		return;

	// Convert to windows line endings
	CString fileBuffer;
	const Char* pstr = m_pConsoleBuffer;
	while(pstr)
	{
		CString line;
		pstr = Common::ReadLine(pstr, line);
		if(line.empty())
			continue;

		fileBuffer << line << NEWLINE;
	}

	// Determine index for the file
	Uint32 fileIdx = 0;
	CString filePath;
	while(TRUE)
	{
		filePath.clear();
		filePath << "condump" << static_cast<Int32>(fileIdx) << ".txt";

		if(!FL_FileExists(filePath.c_str()))
			break;

		fileIdx++;
	}

	// Write the file
	const byte* pdata = reinterpret_cast<const byte*>(fileBuffer.c_str());
	FL_WriteFile(pdata, fileBuffer.length(), filePath.c_str());
}

//=============================================
// @brief Updates text history, if it wasn't updated already
//
//=============================================
void CConsole::UpdateTextHistory( void )
{
	if(m_isHistoryUpdated)
		return;

	if(m_pConsoleBuffer && qstrlen(m_pConsoleBuffer))
	{
		CUIConsoleWindow* pWindow = CUIConsoleWindow::GetInstance();
		if(pWindow && pWindow->isVisible())
			pWindow->SetHistoryText(m_pConsoleBuffer);
	}

	m_isHistoryUpdated = true;
}