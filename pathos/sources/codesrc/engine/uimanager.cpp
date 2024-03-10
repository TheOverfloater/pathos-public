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
#include "r_main.h"
#include "r_vbo.h"
#include "r_glsl.h"
#include "r_basicdraw.h"
#include "r_text.h"
#include "r_interface.h"
#include "r_interfacefuncs.h"
#include "cl_utils.h"
#include "cl_main.h"

#include "uielements.h"
#include "uimanager.h"
#include "uiconsolewindow.h"
#include "system.h"
#include "enginestate.h"
#include "enginefuncs.h"
#include "textschemas.h"

// Default font schema of the game UI
const Char CUIManager::DEFAULT_TEXT_SCHEMA[] = "uidefault";

// Class definition
CUIManager gUIManager;

//=============================================
// @brief Constructor
//
//=============================================
CUIManager::CUIManager( void ):
	m_pFocusWindow(nullptr),
	m_currentFocusIndex(0),
	m_windowFilterFlags(CUIWindow::UIW_FL_NONE),
	m_pFontSet(nullptr)
{
}

//=============================================
// @brief Destructor
//
//=============================================
CUIManager::~CUIManager( void )
{
	Shutdown();
}

//=============================================
// @brief Initializes interfaces
//
//=============================================
void CUIManager::Init( void )
{
	m_pFontSet = gTextSchemas.GetResolutionSchemaFontSet(DEFAULT_TEXT_SCHEMA, gWindow.GetHeight());
	if(!m_pFontSet)
		m_pFontSet = gText.GetDefaultFont();

	// TODO: Get rid of this
	ui_engine_interface_t uiFuncs;

	// Set the interface
	uiFuncs.pfnGetEngineTime = Engine_GetEngineTime;
	uiFuncs.pfnCon_Printf = Con_Printf;
	uiFuncs.pfnCon_DPrintf = Con_DPrintf;
	uiFuncs.pfnCon_VPrintf = Con_VPrintf;
	uiFuncs.pfnCon_EPrintf = Con_EPrintf;
	uiFuncs.pfnGetMousePosition = CL_GetMousePosition;
	uiFuncs.pfnUILoadSchemaFile = CL_UILoadSchemaFile;
	uiFuncs.pfnBasicDrawIsActive = R_BasicDrawIsActive;
	uiFuncs.pfnEnableBasicDraw = R_EnableBasicDraw,
	uiFuncs.pfnDisableBasicDraw = R_DisableBasicDraw;
	uiFuncs.pfnBasicDrawEnableTextures = R_BasicDrawEnableTextures;
	uiFuncs.pfnBasicDrawDisableTextures = R_BasicDrawDisableTextures;
	uiFuncs.pfnBasicDrawBegin = R_BasicDrawBegin;
	uiFuncs.pfnBasicDrawEnd = R_BasicDrawEnd;
	uiFuncs.pfnBasicDrawColor4f = R_BasicDrawColor4f;
	uiFuncs.pfnBasicDrawColor4fv = R_BasicDrawColor4fv;
	uiFuncs.pfnBasicDrawTexCoord2f = R_BasicDrawTexCoord2f;
	uiFuncs.pfnBasicDrawTexCoord2fv = R_BasicDrawTexCoord2fv;
	uiFuncs.pfnBasicDrawVertex3f = R_BasicDrawVertex3f;
	uiFuncs.pfnBasicDrawVertex3fv = R_BasicDrawVertex3fv;
	uiFuncs.pfnBasicDrawSetProjection = R_BasicDrawSetProjection;
	uiFuncs.pfnBasicDrawSetModelView = R_BasicDrawSetModelView;
	uiFuncs.pfnBind2DTexture = R_Bind2DTexture;
	uiFuncs.pfnGetDefaultFontSet = R_GetDefaultFontSet;
	uiFuncs.pfnLoadFontSet = R_LoadFontSet;
	uiFuncs.pfnDrawSimpleString = R_DrawString;
	uiFuncs.pfnDrawStringBox = R_DrawStringBox;
	uiFuncs.pfnBeginTextRendering = R_BeginTextRendering;
	uiFuncs.pfnFinishTextRendering = R_FinishTextRendering;
	uiFuncs.pfnSetStringRectangle = R_SetStringRectangle;
	uiFuncs.pfnDrawCharacter = R_DrawCharacter;
	uiFuncs.pfnGetStringSize = R_GetStringSize;
	uiFuncs.pfnEstimateStringHeight = R_EstimateStringHeight;
	uiFuncs.pfnGetProjectionMatrix = R_GetProjectionMatrix;
	uiFuncs.pfnGetModelViewMatrix = R_GetModelViewMatrix;
	uiFuncs.pfnGetWindowSize = R_GetScreenSize;
	uiFuncs.pfnValidateBasicDraw = R_IF_ValidateBasicDraw;

	CUIObject::SetRenderInterface(uiFuncs);
}

//=============================================
// @brief Initializes interfaces
//
//=============================================
void CUIManager::OnGLInitialization( void )
{
	m_windowList.begin();
	while(!m_windowList.end())
	{
		CUIWindow* pWindow = m_windowList.get();
		pWindow->onGLInitialization();

		m_windowList.next();
	}
}

//=============================================
// @brief Destroys all active windows
//
//=============================================
void CUIManager::Shutdown( void )
{
	if(!m_windowList.empty())
	{
		m_windowList.begin();
		while(!m_windowList.end())
		{
			CUIObject* pfree = m_windowList.get();
			delete pfree;

			m_windowList.next();
		}
	}

	if(!m_tabSchemeArray.empty())
	{
		for(Uint32 i = 0; i < m_tabSchemeArray.size(); i++)
			delete m_tabSchemeArray[i];

		m_tabSchemeArray.clear();
	}

	if(!m_windowDescriptionArray.empty())
	{
		for(Uint32 i = 0; i < m_windowDescriptionArray.size(); i++)
			delete m_windowDescriptionArray[i];

		m_windowDescriptionArray.clear();
	}
}

//=============================================
// @brief  Performs think functions
//
//=============================================
void CUIManager::Think( void )
{
	m_windowList.begin();
	while(!m_windowList.end())
	{
		CUIWindow* pWindow = m_windowList.get();
		pWindow->think();

		m_windowList.next();
	}
}

//=============================================
// @brief  Performs post-command think functions
//
//=============================================
void CUIManager::PostThink( void )
{
	m_windowList.begin();
	while(!m_windowList.end())
	{
		CUIWindow* pWindow = m_windowList.get();
		pWindow->postThink();

		// Destroy any flagged windows
		Int32 flags = pWindow->getWindowFlags();
		if(flags & CUIWindow::UIW_FL_KILLME)
			DestroyWindow(pWindow);

		m_windowList.next();
	}
}

//=============================================
// @brief Draws the active windows
//
//=============================================
bool CUIManager::Draw( void )
{
	// Set modelview
	rns.view.modelview.PushMatrix();
	rns.view.modelview.LoadIdentity();
	rns.view.modelview.Scale(1.0f/gWindow.GetWidth(), 1.0f/gWindow.GetHeight(), 1.0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Set it in the shader
	CBasicDraw* pDraw = CBasicDraw::GetInstance();
	pDraw->SetModelview(rns.view.modelview.GetMatrix());

	// Track render errors
	bool result = true;

	// Windows are sorted by focus index
	m_windowList.rbegin();
	while(!m_windowList.end())
	{
		CUIWindow* pWindow = m_windowList.get();

		if(m_windowFilterFlags && !(pWindow->getWindowFlags() & m_windowFilterFlags))
		{
			m_windowList.prev();
			continue;
		}

		if(pWindow->isVisible())
		{
			result = pWindow->draw();
			if(!result)
				break;
		}

		m_windowList.prev();
	}

	// Restore modelview
	rns.view.modelview.PopMatrix();

	glDisable(GL_BLEND);

	return result;
}

//=============================================
// @brief Draws the active windows
//
//=============================================
bool CUIManager::KeyEvent( Int32 button, Int16 mod, bool keyDown )
{
	if(!m_pFocusWindow)
		return false;

	if(m_pFocusWindow->keyEvent(button, mod, keyDown))
		return true;

	return false;
}

//=============================================
// @brief Draws the active windows
//
//=============================================
bool CUIManager::MouseButtonEvent( Int32 button, bool keyDown )
{
	Int32 cursorX, cursorY;
	gInput.GetMousePosition(cursorX, cursorY);

	m_windowList.begin();
	while(!m_windowList.end())
	{
		// See if we hit this window
		CUIWindow* pWindow = m_windowList.get();
		if(pWindow->isVisible()
			&& pWindow->isMouseOver(cursorX, cursorY) 
			&& pWindow->mouseButtonEvent(cursorX, cursorY, button, keyDown))
		{
			// Set as focus window if it's not already in focus
			if(keyDown && (!m_pFocusWindow || m_pFocusWindow != pWindow))
				SetFocusWindow(pWindow);

			// Release any press states
			if(!keyDown && m_pFocusWindow)
				m_pFocusWindow->releaseClickStates();

			return true;
		}

		m_windowList.next();
	}

	if(m_pFocusWindow)
	{
		// Release any clicked buttons
		if(button == SDL_BUTTON_LEFT && !keyDown)
			m_pFocusWindow->releaseClickStates();

		// We clicked away from any windows, so release focus
		if(button == SDL_BUTTON_LEFT && keyDown)
			SetFocusWindow(nullptr);
	}

	return false;
}

//=============================================
// @brief Draws the active windows
//
//=============================================
bool CUIManager::MouseWheelEvent( Int32 button, bool keyDown, Int32 scroll )
{
	Int32 cursorX, cursorY;
	gInput.GetMousePosition(cursorX, cursorY);

	m_windowList.begin();
	while(!m_windowList.end())
	{
		// See if we hit this window
		CUIWindow* pWindow = m_windowList.get();
		if(pWindow->isVisible()
			&& pWindow->isMouseOver(cursorX, cursorY) 
			&& pWindow->mouseWheelEvent(cursorX, cursorY, button, keyDown, scroll))
		{
			// Set as focus window if it's not already in focus
			if(keyDown && (!m_pFocusWindow || m_pFocusWindow != pWindow))
				SetFocusWindow(pWindow);

			// Release any press states
			if(!keyDown && m_pFocusWindow)
				m_pFocusWindow->releaseClickStates();

			return true;
		}

		m_windowList.next();
	}

	if(m_pFocusWindow)
	{
		// Release any clicked buttons
		if(button == SDL_BUTTON_LEFT && !keyDown)
			m_pFocusWindow->releaseClickStates();

		// We clicked away from any windows, so release focus
		if(button == SDL_BUTTON_LEFT && keyDown)
			SetFocusWindow(nullptr);
	}

	return false;
}


//=============================================
// @brief Sets the current focus window
//
//=============================================
void CUIManager::SetFocusWindow( CUIWindow* pWindow )
{
	if(m_pFocusWindow == pWindow)
		return;

	// Reset previous focus window
	if(m_pFocusWindow)
		m_pFocusWindow->setFocusState(false);

	// Raise focus index
	if(pWindow)
	{
		pWindow->setFocusIndex(m_currentFocusIndex);
		m_currentFocusIndex++;

		// Set window as current focus window
		m_pFocusWindow = pWindow;
		m_pFocusWindow->setFocusState(true);

		// Reorder the windows
		ReorderWindows();
	}
	else if(m_pFocusWindow)
	{
		// No focus on any window
		m_pFocusWindow = nullptr;
		m_currentFocusIndex++;
	}
}

//=============================================
// @brief Hides windows with a specific flag shows the rest
//
//=============================================
void CUIManager::HideWindows( Int32 windowFlags )
{
	if(m_windowList.empty())
		return;

	m_windowList.begin();
	while(!m_windowList.end())
	{
		// See if we hit this window
		CUIWindow* pWindow = m_windowList.get();
		if(pWindow->getWindowFlags() & windowFlags)
		{
			if(pWindow == m_pFocusWindow)
				SetFocusWindow(nullptr);

			// Hide the window
			pWindow->setVisible(false);
		}

		m_windowList.next();
	}
}

//=============================================
// @brief Show windows with a specific flag
//
//=============================================
void CUIManager::ShowWindows( Int32 windowFlags )
{
	if(m_windowList.empty())
		return;

	m_windowList.begin();
	while(!m_windowList.end())
	{
		// See if we hit this window
		CUIWindow* pWindow = m_windowList.get();
		if(pWindow->getWindowFlags() & windowFlags)
		{
			if(!m_pFocusWindow)
			{
				SetFocusWindow(pWindow);
				ShowWindows(windowFlags);
				return;
			}

			// Hide the window
			pWindow->setVisible(true);
		}

		m_windowList.next();
	}
}

//=============================================
// @brief Sorts the windows by focus indexes
//
//=============================================
void CUIManager::ReorderWindows( void )
{
	// Probably not the most efficient setup
	Uint32 currentFocusIdx = m_currentFocusIndex;

	// Temporary list to hold vars
	CLinkedList<CUIWindow*> tmpList;

	while(TRUE)
	{
		// Last nearest value to focus index
		Int32 lastNearest = -1;
		CUIWindow* pLastWindow = nullptr;

		m_windowList.begin();
		while(!m_windowList.end())
		{
			CUIWindow* pWindow = m_windowList.get();
			Uint32 focusIndex = pWindow->getFocusIndex();
			
			if(focusIndex < currentFocusIdx 
				&& lastNearest < (Int32)focusIndex)
			{
				pLastWindow = pWindow;
				lastNearest = (Int32)focusIndex;
			}

			m_windowList.next();
		}

		// Check if we reached the end
		if(lastNearest == -1)
			break;

		// Set current focus index
		currentFocusIdx = lastNearest;
		tmpList.add(pLastWindow);
	}

	// Shouldn't happen, but still
	assert(tmpList.size() == m_windowList.size());

	// Fill the list with the new values
	m_windowList.clear();

	tmpList.begin();
	while(!tmpList.end())
	{
		m_windowList.add(tmpList.get());
		tmpList.next();
	}
}

//=============================================
// @brief Sorts the windows by focus indexes
//
//=============================================
void CUIManager::RepositionWindows( void )
{
	if(m_windowList.empty())
		return;

	Uint32 winWidth = gWindow.GetWidth();
	Uint32 winHeight = gWindow.GetHeight();

	m_windowList.begin();
	while(!m_windowList.end())
	{
		CUIWindow* pWindow = m_windowList.get();

		Int32 originX, originY;
		pWindow->getAbsPosition(originX, originY);

		Uint32 width, height;
		pWindow->getSize(width, height);

		Int32 newOriginX = originX;
		if(originX + width > winWidth)
			newOriginX = winWidth - width;

		Int32 newOriginY = originY;
		if(originY + height > winHeight)
			newOriginY = winHeight - height;

		// Reposition to new location
		pWindow->setPosition(newOriginX, newOriginY);

		m_windowList.next();
	}

}

//=============================================
// @brief Adds a window to the list
//
//=============================================
void CUIManager::AddWindow( CUIWindow* pWindow )
{
	// Add it to the list
	m_windowList.add(pWindow);
	SetFocusWindow(pWindow);
}

//=============================================
// @brief Destroys a window and removes it from the list
//
//=============================================
void CUIManager::DestroyWindow( CUIWindow* pWindow )
{
	// Remove it from the list first
	m_windowList.remove(pWindow);

	// Remove it from focus
	if(m_pFocusWindow == pWindow)
	{
		// Find window with the highest focus
		Int32 lastHighest = -1;
		CUIWindow* pBestWindow = nullptr;

		m_windowList.begin();
		while(!m_windowList.end())
		{
			CUIWindow* pListWnd = m_windowList.get();
			if(pListWnd != pWindow)
			{
				Uint32 focusIndex = pWindow->getFocusIndex();
				if((Int32)focusIndex > lastHighest || lastHighest == -1)
				{
					pBestWindow = pListWnd;
					lastHighest = (Int32)focusIndex;
				}
			}

			m_windowList.next();
		}

		// Set this as the focus window
		if(pBestWindow)
			SetFocusWindow(pBestWindow);
		else
			m_pFocusWindow = nullptr;
	}

	// Delete this object
	delete pWindow;
}

//=============================================
// @brief
//
//=============================================
bool CUIManager::HasActiveWindows( void )
{
	if(m_windowList.empty())
		return false;

	m_windowList.begin();
	while(!m_windowList.end())
	{
		CUIWindow* pListWnd = m_windowList.get();
		if(pListWnd->isVisible())
			return true;

		m_windowList.next();
	}

	return false;
}

//=============================================
// @brief Loads in a schema file
//
// @param pstrFilename Name of the UI scheme file
// @return Pointer to scheme object
//=============================================
ui_schemeinfo_t* CUIManager::LoadSchemaFile( const Char* pstrFilename )
{
	// Try to find it in the cache first
	for(Uint32 i = 0; i < m_tabSchemeArray.size(); i++)
	{
		if(!qstrcmp(m_tabSchemeArray[i]->schemeName, pstrFilename))
			return m_tabSchemeArray[i];
	}

	// Load in the file
	CString scriptPath;
	scriptPath << "scripts/ui/schemas/" << pstrFilename;

	Uint32 fileSize = 0;
	const Char* pfile = reinterpret_cast<const Char*>(FL_LoadFile(scriptPath.c_str(), &fileSize));
	if(!pfile)
	{
		Con_EPrintf("Failed to load UI schema script %s.\n", scriptPath.c_str());
		return nullptr;
	}

	// Allocate new object
	ui_schemeinfo_t* pNew = new ui_schemeinfo_t;
	pNew->schemeName = pstrFilename;

	// Parse the contents
	CString token;
	CString line;

	CTextureManager* pTextureManager = CTextureManager::GetInstance();

	const Char* pstr = pfile;
	while(pstr && *pstr && (pstr - pfile) < fileSize)
	{
		// Read in the first token
		CString objName;
		pstr = Common::Parse(pstr, objName);
		if(!pstr || objName.empty())
			break;

		// Scheme object we'll be processing
		ui_schemeobject_t newObject;
		newObject.typeName = objName;

		// Next token should be an opening bracket
		pstr = Common::Parse(pstr, token);
		if(!pstr || token.empty())
		{
			Con_EPrintf("Unexpected EOF on %s.\n", scriptPath.c_str());
			FL_FreeFile(pfile);

			delete pNew;
			return nullptr;
		}

		// Make sure the script is valid
		if(qstrcmp(token, "{"))
		{
			Con_EPrintf("{ token expected %s, got %s.\n", scriptPath.c_str(), token.c_str());
			FL_FreeFile(pfile);

			delete pNew;
			return nullptr;
		}

		// Read in the fields, line by line
		while(pstr && *pstr && (pstr - pfile) < fileSize)
		{
			// Skip whitespaces
			while(*pstr && SDL_isspace(*pstr))
				pstr++;

			// Read in the entire line
			pstr = Common::ReadLine(pstr, line);
			if(line.empty())
				continue;

			// Read in the first token
			const Char* pstrl = Common::Parse(line.c_str(), token);
			if(token.empty())
			{
				Con_EPrintf("Unexpected EOF on %s.\n", scriptPath.c_str());
				FL_FreeFile(pfile);

				delete pNew;
				return nullptr;
			}

			// Exit the loop
			if(!qstrcmp(token, "}"))
				break;
			
			if(!pstrl)
			{
				Con_EPrintf("Unexpected EOF on %s.\n", scriptPath.c_str());
				FL_FreeFile(pfile);

				delete pNew;
				return nullptr;
			}

			// Read in the value
			CString value;
			pstrl = Common::Parse(pstrl, value);
			if(value.empty())
			{
				Con_EPrintf("Unexpected EOF on %s.\n", scriptPath.c_str());
				FL_FreeFile(pfile);

				delete pNew;
				return nullptr;
			}

			// Determine field type
			CString textureName;
			if(!qstrcmp(token, "$default") 
				|| !qstrcmp(token, "$focus")
				|| !qstrcmp(token, "$clicked")
				|| !qstrcmp(token, "$disabled"))
			{
				// If it's a texture resource, load it in
				CString texturePath;
				texturePath << "ui/" << value;

				// Load it in
				en_texture_t* ptexture = pTextureManager->LoadTexture(texturePath.c_str(), RS_WINDOW_LEVEL, TX_FL_NOMIPMAPS);
				if(!ptexture)
					ptexture = pTextureManager->GetDummyTexture();

				// Assign it to the right place
				if(!qstrcmp(token, "$default"))
					newObject.defaultTexture = ptexture;
				else if(!qstrcmp(token, "$focus"))
					newObject.focusTexture = ptexture;
				else if(!qstrcmp(token, "$clicked"))
					newObject.clickTexture = ptexture;
				else if(!qstrcmp(token, "$disabled"))
					newObject.disabledTexture = ptexture;

				if(!newObject.width)
					newObject.width = ptexture->width;
				if(!newObject.height)
					newObject.height = ptexture->height;
			}
			else if(!qstrcmp(token, "$width"))
				newObject.width = SDL_atoi(value.c_str());
			else if(!qstrcmp(token, "$height"))
				newObject.height = SDL_atoi(value.c_str());
			else
			{
				Con_Printf("Unknown field %s in %s.\n", token.c_str(), scriptPath.c_str());
				break;
			}
		}

		// Add it to the object
		pNew->tabObjects.push_back(newObject);
	}

	// Add this scheme object to the array
	m_tabSchemeArray.push_back(pNew);
	FL_FreeFile(pfile);

	return pNew;
}

//=============================================
// @brief Loads in a schema file
//
// @param pstrFilename Name of the UI scheme file
// @return Pointer to scheme object
//=============================================
ui_windowdescription_t* CUIManager::LoadWindowDescriptionFile( const Char* pstrWindowName, const Char* pstrFilename )
{
	// Try to find it in the cache first
	for(Uint32 i = 0; i < m_windowDescriptionArray.size(); i++)
	{
		if(!qstrcmp(m_windowDescriptionArray[i]->windowName, pstrWindowName))
			return m_windowDescriptionArray[i];
	}

	// Load in the file
	CString scriptPath;
	scriptPath << "scripts/ui/windows/" << pstrFilename;

	Uint32 fileSize = 0;
	const Char* pfile = reinterpret_cast<const Char*>(FL_LoadFile(scriptPath.c_str(), &fileSize));
	if(!pfile)
	{
		Con_EPrintf("Failed to load UI schema script %s.\n", scriptPath.c_str());
		return nullptr;
	}

	// Allocate new object
	ui_windowdescription_t* pNewWindowDesc = new ui_windowdescription_t;
	pNewWindowDesc->windowName = pstrWindowName;

	// Parse the contents
	CString token;
	CString line;

	const Char* pstr = pfile;
	while(pstr && *pstr && (pstr - pfile) < fileSize)
	{
		// Read in the object name
		CString objType;
		pstr = Common::Parse(pstr, objType);
		if(!pstr || objType.empty())
			break;

		ui_object_type_t type = UI_OBJECT_UNDEFINED;
		if(!qstrcmp(objType, "window"))
			type = UI_OBJECT_WINDOW;
		else if(!qstrcmp(objType, "button"))
			type = UI_OBJECT_BUTTON;
		else if(!qstrcmp(objType, "text"))
			type = UI_OBJECT_TEXT;
		else if(!qstrcmp(objType, "tab"))
			type = UI_OBJECT_TAB;
		else if(!qstrcmp(objType, "list"))
			type = UI_OBJECT_LIST;
		else if(!qstrcmp(objType, "tickbox"))
			type = UI_OBJECT_TICKBOX;
		else if(!qstrcmp(objType, "slider"))
			type = UI_OBJECT_SLIDER;
		else
		{
			Con_EPrintf("Unknown object type '%s' in '%s'.\n", objType.c_str(), scriptPath.c_str());
			FL_FreeFile(pfile);

			delete pNewWindowDesc;
			return nullptr;
		}

		// Read in the object name
		CString objName;
		pstr = Common::Parse(pstr, objName);
		if(!pstr || objName.empty())
			break;

		// Read the bracket token
		pstr = Common::Parse(pstr, token);
		if(!pstr || token.empty())
		{
			Con_EPrintf("Unexpected EOF on %s.\n", scriptPath.c_str());
			FL_FreeFile(pfile);

			delete pNewWindowDesc;
			return nullptr;
		}

		// Make sure the format is correct
		if(qstrcmp(token, "{"))
		{
			Con_EPrintf("Expected '{', got %s in %s.\n", token.c_str(), scriptPath.c_str());
			FL_FreeFile(pfile);

			delete pNewWindowDesc;
			return nullptr;
		}
		
		// Create the new object
		CString textschema;
		ui_objectinfo_t newObject;
		newObject.objectName = objName;
		newObject.type = type;

		// Read in the parameters
		while(true)
		{
			// Skip whitespaces
			while(*pstr && SDL_isspace(*pstr))
				pstr++;

			// Read in the entire line
			pstr = Common::ReadLine(pstr, line);
			if(line.empty())
				continue;

			// Read in the first token
			const Char* pstrl = Common::Parse(line.c_str(), token);
			if(token.empty())
			{
				Con_EPrintf("Unexpected EOF on %s.\n", scriptPath.c_str());
				FL_FreeFile(pfile);

				delete pNewWindowDesc;
				return nullptr;
			}

			// Exit the loop
			if(!qstrcmp(token, "}"))
				break;
			
			if(!pstrl)
			{
				Con_EPrintf("Unexpected EOF on %s.\n", scriptPath.c_str());
				FL_FreeFile(pfile);

				delete pNewWindowDesc;
				return nullptr;
			}

			// $flags is handled specially
			if(qstrcmp(token, "$flags"))
			{
				// Read in the value
				CString value;
				pstrl = Common::Parse(pstrl, value);
				if(value.empty())
				{
					Con_EPrintf("Unexpected EOF on %s.\n", scriptPath.c_str());
					FL_FreeFile(pfile);

					delete pNewWindowDesc;
					return nullptr;
				}

				if(!qstrcmp(token, "$width"))
					newObject.width = SDL_atoi(value.c_str());
				else if(!qstrcmp(token, "$height"))
					newObject.height = SDL_atoi(value.c_str());
				else if(!qstrcmp(token, "$text"))
					newObject.text = value;
				else if(!qstrcmp(token, "$schema"))
					newObject.schema = value;
				else if(!qstrcmp(token, "$alpha"))
					newObject.alpha = SDL_atof(value.c_str());
				else if(!qstrcmp(token, "$insetx"))
					newObject.insetx = SDL_atoi(value.c_str());
				else if(!qstrcmp(token, "$insety"))
					newObject.insety = SDL_atoi(value.c_str());
				else if(!qstrcmp(token, "$originx"))
					newObject.originx = SDL_atoi(value.c_str());
				else if(!qstrcmp(token, "$originy"))
					newObject.originy = SDL_atoi(value.c_str());
				else if(!qstrcmp(token, "$titleinsetx"))
					newObject.title_insetx = SDL_atoi(value.c_str());
				else if(!qstrcmp(token, "$titleinsety"))
					newObject.title_insety = SDL_atoi(value.c_str());
				else if(!qstrcmp(token, "$textinset"))
					newObject.text_inset = SDL_atoi(value.c_str());
				else if(!qstrcmp(token, "$rowheight"))
					newObject.listrowheight = SDL_atoi(value.c_str());
				else if(!qstrcmp(token, "$title"))
					newObject.title = value;
				else if(!qstrcmp(token, "$dragger"))
					newObject.dragger = (!qstrcmp(value.c_str(), "true") ? true : false);
				else if(!qstrcmp(token, "$resizable"))
					newObject.resizable = (!qstrcmp(value.c_str(), "true") ? true : false);
				else if(!qstrcmp(token, "$textschema"))
					textschema = value;
				else if(!qstrcmp(token, "$minvalue"))
					newObject.minvalue = SDL_atof(value.c_str());
				else if(!qstrcmp(token, "$maxvalue"))
					newObject.maxvalue = SDL_atof(value.c_str());
				else if(!qstrcmp(token, "$markerdistance"))
					newObject.markerdistance = SDL_atof(value.c_str());
				else
				{
					Con_Printf("Unknown field %s in %s.\n", token.c_str(), scriptPath.c_str());
					continue;
				}
			}
			else
			{
				newObject.flags = CUIObject::UIEL_FL_NONE;

				while(pstrl)
				{
					CString flag;
					pstrl = Common::Parse(pstrl, flag);
					
					if(!qstrcmp(flag, "fixed_width"))
						newObject.flags |= CUIObject::UIEL_FL_FIXED_W;
					else if(!qstrcmp(flag, "fixed_height"))
						newObject.flags |= CUIObject::UIEL_FL_FIXED_H;
					else if(!qstrcmp(flag, "align_left"))
						newObject.flags |= CUIObject::UIEL_FL_ALIGN_L;
					else if(!qstrcmp(flag, "align_right"))
						newObject.flags |= CUIObject::UIEL_FL_ALIGN_R;
					else if(!qstrcmp(flag, "align_top"))
						newObject.flags |= CUIObject::UIEL_FL_ALIGN_T;
					else if(!qstrcmp(flag, "align_bottom"))
						newObject.flags |= CUIObject::UIEL_FL_ALIGN_B;
					else if(!qstrcmp(flag, "align_center_horizontal"))
						newObject.flags |= CUIObject::UIEL_FL_ALIGN_CH;
					else if(!qstrcmp(flag, "align_center_vertical"))
						newObject.flags |= CUIObject::UIEL_FL_ALIGN_CV;
					else if(!qstrcmp(flag, "wrap_word"))
						newObject.flags |= CUIObject::UIEL_FL_WRAP_WORD;
					else if(!qstrcmp(flag, "fixed_xpos"))
						newObject.flags |= CUIObject::UIEL_FL_FIXED_XPOS;
					else if(!qstrcmp(flag, "fixed_ypos"))
						newObject.flags |= CUIObject::UIEL_FL_FIXED_YPOS;
					else if(!qstrcmp(flag, "ontop"))
						newObject.flags |= CUIObject::UIEL_FL_ONTOP;
					else if(!qstrcmp(flag, "scroller_reverse"))
						newObject.flags |= CUIObject::UIEL_FL_SCR_REVERSE;
					else if(!qstrcmp(flag, "expand_width"))
						newObject.flags |= CUIObject::UIEL_FL_EXPAND_W;
					else if(!qstrcmp(flag, "expand_height"))
						newObject.flags |= CUIObject::UIEL_FL_EXPAND_H;
					else if(!qstrcmp(flag, "noheader"))
						newObject.flags |= CUIObject::UIEL_FL_NO_HEADER;
					else if(!qstrcmp(flag, "hover_highlight"))
						newObject.flags |= CUIObject::UIEL_FL_HOVER_HIGHLIGHT;
					else
						Con_EPrintf("Unknown flag '%s' in '%s', discarding.\n", flag.c_str(), scriptPath.c_str());
				}
			}
		}

		// Verify that we have valid sizes for non-text objects
		if(newObject.type != UI_OBJECT_TEXT && newObject.type != UI_OBJECT_TICKBOX)
		{
			if(!newObject.width)
			{
				Con_EPrintf("Object '%s' in '%s' has no width, discarding.\n", newObject.objectName.c_str(), scriptPath.c_str());
				continue;
			}
			if(!newObject.height)
			{
				Con_EPrintf("Object '%s' in '%s' has no height, discarding.\n", newObject.objectName.c_str(), scriptPath.c_str());
				continue;
			}
		}

		// Make sure sliders have the necessary values set
		if(newObject.type == UI_OBJECT_SLIDER)
		{
			if(!newObject.maxvalue)
			{
				Con_EPrintf("Object '%s' in '%s' has no maxvalue, discarding.\n", newObject.objectName.c_str(), scriptPath.c_str());
				continue;
			}
			if(!newObject.markerdistance)
			{
				Con_EPrintf("Object '%s' in '%s' has no markerdistance, discarding.\n", newObject.objectName.c_str(), scriptPath.c_str());
				continue;
			}
		}

		// Check for valid row height
		if(newObject.type == UI_OBJECT_LIST)
		{
			if(!newObject.listrowheight)
			{
				Con_EPrintf("Object '%s' in '%s' has no row height set, discarding.\n", newObject.objectName.c_str(), scriptPath.c_str());
				continue;
			}
		}

		// Load any custom fonts
		if(!textschema.empty())
		{
			newObject.pfont = gTextSchemas.GetSchemaFontSet(textschema.c_str());
			if(!newObject.pfont)
			{
				Con_EPrintf("Object '%s' in '%s' - Schema '%s' not found.\n", newObject.objectName.c_str(), scriptPath.c_str(), textschema.c_str());
				newObject.pfont = m_pFontSet;
			}
		}
		else
		{
			// Set default
			newObject.pfont = m_pFontSet;
		}

		if(!newObject.alpha)
			newObject.alpha = 255;
		else if(newObject.alpha > 255)
			newObject.alpha = 255;

		pNewWindowDesc->objectsArray.push_back(newObject);
	}

	// Add it to the list
	m_windowDescriptionArray.push_back(pNewWindowDesc);

	FL_FreeFile(pfile);
	return pNewWindowDesc;
}

//=============================================
// @brief Sets a window flag for draw filtering
//
//=============================================
void CUIManager::SetDrawFilter( Int32 windowFlags )
{
	m_windowFilterFlags |= windowFlags;
}

//=============================================
// @brief Sets a window flag for draw filtering
//
//=============================================
void CUIManager::RemoveDrawFilter( Int32 windowFlags )
{
	m_windowFilterFlags &= ~windowFlags;
}

//=============================================
// @brief Tells if mouse is over any active window
//
//=============================================
bool CUIManager::IsMouseOverAnyWindow( void )
{
	if(m_windowList.empty())
		return false;

	Int32 cursorX, cursorY;
	gInput.GetMousePosition(cursorX, cursorY);

	m_windowList.begin();
	while(!m_windowList.end())
	{
		CUIWindow* pWindow = m_windowList.get();
		if(pWindow->isMouseOver(cursorX, cursorY))
			return true;

		m_windowList.next();
	}

	return false;
}