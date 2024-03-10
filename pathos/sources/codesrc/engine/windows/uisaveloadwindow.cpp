/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "file.h"
#include "input.h"
#include "enginestate.h"

#include "texturemanager.h"
#include "r_vbo.h"
#include "r_glsl.h"
#include "r_text.h"
#include "r_menu.h"

#include "uimanager.h"
#include "uielements.h"
#include "uisaveloadwindow.h"

#include "window.h"
#include "commands.h"

#include "r_basicdraw.h"
#include "r_main.h"
#include "system.h"
#include "saverestore.h"
#include "file.h"

// New save row index
static const Int32 NEW_SAVE_INDEX = -1;

// Current instance of the window
CUISaveLoadWindow* CUISaveLoadWindow::m_pInstance = nullptr;

// Window description file
const Char CUISaveLoadWindow::WINDOW_DESC_FILE[] = "saveloadwindow.txt";
// Window description file
const Char CUISaveLoadWindow::WINDOW_OBJ_NAME[] = "SaveLoadWindow";
// Save list object name
const Char CUISaveLoadWindow::SAVELIST_OBJ_NAME[] = "SaveList";
// Cancel button object name
const Char CUISaveLoadWindow::CANCEL_BUTTON_OBJ_NAME[] = "CancelButton";
// Load game button object name
const Char CUISaveLoadWindow::LOAD_GAME_BUTTON_OBJ_NAME[] = "LoadGameButton";
// Save game button object name
const Char CUISaveLoadWindow::SAVE_GAME_BUTTON_OBJ_NAME[] = "SaveGameButton";
// Delete save button object name
const Char CUISaveLoadWindow::DELETE_SAVE_BUTTON_OBJ_NAME[] = "DeleteSaveButton";

//=============================================
// @brief Comparator function for save file ordering
//
//=============================================
static Int32 SortSaves( const void* p1, const void* p2 )
{
	CUISaveLoadWindow::save_file_t *psave1 = (CUISaveLoadWindow::save_file_t *)p1;
	CUISaveLoadWindow::save_file_t *psave2 = (CUISaveLoadWindow::save_file_t *)p2;

	if(!qstrlen(psave1->filepath))
		return -1;

	return FL_CompareFileDates(psave1->date, psave2->date);
}

//=============================================
// @brief Constructor
//
//=============================================
CUISaveLoadWindow::CUISaveLoadWindow( Int32 flags, Uint32 width, Uint32 height, Int32 originx, Int32 originy ):
	CUIWindow(UIW_FL_MENUWINDOW, flags, width, height, originx, originy),
	m_pSaveList(nullptr),
	m_lastSelectedSave(-1),
	m_bRecheckSaves(false),
	m_rowTextInset(0),
	m_pListObjectInfo(nullptr),
	m_bIsIngame(false),
	m_pSaveButton(nullptr),
	m_pLoadButton(nullptr),
	m_pDeleteButton(nullptr)
{
}

//=============================================
// @brief Destructor
//
//=============================================
CUISaveLoadWindow::~CUISaveLoadWindow( void )
{
	// Probably destroyed by the UI manager
	if(m_pInstance)
		m_pInstance = nullptr;

	// Reset menu background
	gMenu.SetBlendTargetTexture(nullptr);
}

//=============================================
// @brief Creates an instance of the console window
//
//=============================================
CUISaveLoadWindow* CUISaveLoadWindow::CreateInstance( bool isIngame )
{
	if(m_pInstance)
		return m_pInstance;

	// Load the schema file
	ui_windowdescription_t* pWinDesc = gUIManager.LoadWindowDescriptionFile(WINDOW_OBJ_NAME, WINDOW_DESC_FILE);
	if(!pWinDesc)
	{
		Con_EPrintf("Failed to load window description '%s' for '%s'.\n", WINDOW_DESC_FILE, WINDOW_OBJ_NAME);
		return nullptr;
	}

	const ui_objectinfo_t* pWindowObject = pWinDesc->getObject(UI_OBJECT_WINDOW, WINDOW_OBJ_NAME);
	if(!pWindowObject)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, WINDOW_OBJ_NAME);
		return nullptr;
	}

	// Position to center of the screen
	Int32 xOrigin = gWindow.GetCenterX() - pWindowObject->getWidth()/2;
	Int32 yOrigin = gWindow.GetCenterY() - pWindowObject->getHeight()/3;

	// Create the main tab object
	CUISaveLoadWindow* pWindow = new CUISaveLoadWindow(UIEL_FL_NONE, pWindowObject->getWidth(), pWindowObject->getHeight(), xOrigin, yOrigin);
	if(!pWindow->init(pWinDesc, pWindowObject, isIngame))
	{
		Con_EPrintf("Failed to set schema '%s' for the exit window.\n", pWindowObject->getSchema().c_str());
		delete pWindow;
		return nullptr;
	}

	// Set pointer
	m_pInstance = pWindow;
	// Add to UI manager to handle
	gUIManager.AddWindow(pWindow);

	return m_pInstance;
}

//=============================================
// @brief Destroys the current instance of the console window
//
//=============================================
void CUISaveLoadWindow::DestroyInstance( void )
{
	if(!m_pInstance)
		return;

	m_pInstance->setWindowFlags(UIW_FL_KILLME);
	m_pInstance = nullptr;
}

//=============================================
// @brief Returns the current instance of the console window
//
//=============================================
CUISaveLoadWindow* CUISaveLoadWindow::GetInstance( void )
{
	return m_pInstance;
}

//=============================================
// @brief Returns the current instance of the console window
//
//=============================================
void CUISaveLoadWindow::AddSaveFileInfo( const CString& filePath, FILETIME& fileTime )
{
	// Load the file
	const byte* pdata = gSaveRestore.LoadSaveFile(filePath.c_str());
	if(!pdata)
		return;

	// Get header ptr
	const save_header_t* pheader = reinterpret_cast<const save_header_t*>(pdata);
	if(pheader->type != SAVE_REGULAR && pheader->type != SAVE_AUTO && pheader->type != SAVE_QUICK)
	{
		gSaveRestore.FreeSaveFile(pdata);
		return;
	}

	// Get file time
	SYSTEMTIME sysTime;
	if(!FileTimeToSystemTime(&fileTime, &sysTime))
	{
		Con_EPrintf("Failed to get file creation time for '%s'.\n", filePath.c_str());
		return;
	}

	TIME_ZONE_INFORMATION tzInfo;
	if(!GetTimeZoneInformation(&tzInfo))
	{
		Con_EPrintf("Failed to get time zone info for '%s'.\n", filePath.c_str());
		return;
	}

	SYSTEMTIME tzTime;
	if(!SystemTimeToTzSpecificLocalTime(&tzInfo, &sysTime, &tzTime))
	{
		Con_EPrintf("Failed to get time zone specific time information for '%s'.\n", filePath.c_str());
		return;
	}

	CString dateString;
	dateString << tzTime.wYear << "/";
	
	if(tzTime.wMonth < 10)
		dateString << '0';
	dateString << tzTime.wMonth << "/";

	if(tzTime.wDay < 10)
		dateString << '0';
	dateString << tzTime.wDay << " - ";
	
	if(tzTime.wHour < 10)
		dateString << '0';
	dateString << tzTime.wHour << ":";
	
	if(tzTime.wMinute < 10)
		dateString << '0';
	dateString << tzTime.wMinute;

	save_file_t saveFile;

	qstrcpy(saveFile.filepath, filePath.c_str());
	qstrcpy(saveFile.savetitle, pheader->saveheader);

	saveFile.gametime = pheader->gametime;
	saveFile.datestring = dateString;
	saveFile.type = pheader->type;

	saveFile.date.year = tzTime.wYear;
	saveFile.date.month = tzTime.wMonth;
	saveFile.date.day = tzTime.wDay;
	saveFile.date.hour = tzTime.wHour;
	saveFile.date.minute = tzTime.wMinute;
	saveFile.date.second = tzTime.wSecond;

	m_saveFilesArray.push_back(saveFile);

	// Release save file
	gSaveRestore.FreeSaveFile(pdata);
}

//=============================================
// @brief Returns the current instance of the console window
//
//=============================================
void CUISaveLoadWindow::CreateSaveFileRow( save_file_t& saveFile, Uint32 fileindex )
{
	Uint32 rowIndex = m_pSaveList->getNbRows();
	CUISaveLoadWindowRowEvent* pEvent = new CUISaveLoadWindowRowEvent(this, rowIndex, fileindex);

	// Add row
	CUIListRow* pRow = m_pSaveList->createNewRow(pEvent, m_rowTextInset);
	CUIText* pTextTitle = new CUIText((UIEL_FL_ALIGN_CV|UIEL_FL_ALIGN_CH), m_pListObjectInfo->getFont(), saveFile.savetitle, 0, 0, 0);

	pRow->setColumnContents(0, pTextTitle);

	CString type;
	switch(saveFile.type)
	{
	case SAVE_REGULAR:
		type = "Save file";
		break;
	case SAVE_AUTO:
		type = "Autosave";
		break;
	case SAVE_QUICK:
		type = "Quick save";
		break;
	}

	CUIText* pTextType = new CUIText((UIEL_FL_ALIGN_CV|UIEL_FL_ALIGN_CH), m_pListObjectInfo->getFont(), type.c_str(), 0, 0, 0);
	pRow->setColumnContents(1, pTextType);

	// Set game time
	CString gameTime;
	Int32 nbDays = (Int32)SDL_floor(saveFile.gametime / 86400.0f);
	Int32 nbHours = (Int32)SDL_floor(saveFile.gametime / 3600.0f);
	Int32 nbMinutes = (Int32)SDL_floor(saveFile.gametime / 60.0f);
	Int32 nbSeconds = (Int32)SDL_floor(saveFile.gametime);

	// Perform adjustments
	if(nbDays > 0)
	{
		nbHours -= nbDays*24;
		nbMinutes -= nbDays*24*60;
		nbSeconds -= nbDays*24*60*60;
	}
	
	if(nbHours > 0)
	{
		nbMinutes -= nbHours*60;
		nbSeconds -= nbHours*60*60;
	}

	if(nbMinutes > 0)
		nbSeconds -= nbMinutes*60;

	if(nbDays == 1)
		gameTime << nbDays << " day, ";
	else if(nbDays > 1)
		gameTime << nbDays << " days, ";

	if(nbHours == 1)
		gameTime << nbHours << " hour, ";
	else if(nbHours > 1)
		gameTime << nbHours << " hours, ";

	if(nbMinutes == 1)
		gameTime << nbMinutes << " minute, ";
	else if(nbMinutes > 1)
		gameTime << nbMinutes << " minutes, ";

	gameTime << nbSeconds << " seconds";

	CUIText* pTextGameTime = new CUIText((UIEL_FL_ALIGN_CV|UIEL_FL_ALIGN_CH), m_pListObjectInfo->getFont(), gameTime.c_str(), 0, 0, 0);
	pRow->setColumnContents(2, pTextGameTime);

	CUIText* pTextDate = new CUIText((UIEL_FL_ALIGN_CV|UIEL_FL_ALIGN_CH), m_pListObjectInfo->getFont(), saveFile.datestring.c_str(), 0, 0, 0);
	pRow->setColumnContents(3, pTextDate);
}

//=============================================
// @brief Initializes the tab element with a schema
//
//=============================================
bool CUISaveLoadWindow::init( const ui_windowdescription_t* pWinDesc, const ui_objectinfo_t* pWindowObject, bool isIngame )
{
	if(!CUIWindow::init(pWindowObject->getSchema().c_str()))
		return false;

	// Set this
	m_bIsIngame = isIngame;

	setTitle(pWindowObject->getTitle().c_str(), pWindowObject->getFont(), pWindowObject->getTitleXInset(), pWindowObject->getTitleYInset());
	setAlpha(pWindowObject->getAlpha());

	// Create the dragger
	if(pWindowObject->hasDragger())
	{
		CUIDragger* pDragger = new CUIDragger(UIEL_FL_FIXED_H, m_width-40, 30, 0, 0);
		pDragger->setParent(this);
	}

	// Create the list
	m_pListObjectInfo = pWinDesc->getObject(UI_OBJECT_LIST, SAVELIST_OBJ_NAME);
	if(!m_pListObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, SAVELIST_OBJ_NAME);
		return false;
	}

	m_pSaveList = new CUIList(m_pListObjectInfo->getFlags()|UIEL_FL_HOVER_HIGHLIGHT,
		m_pListObjectInfo->getFont(),
		m_pListObjectInfo->getListRowHeight(),
		4, 
		m_pListObjectInfo->getWidth(),
		m_pListObjectInfo->getHeight(), 
		pWindowObject->getXInset() + m_pListObjectInfo->getXOrigin(), 
		pWindowObject->getYInset() + m_pListObjectInfo->getYOrigin());

	m_pSaveList->setParent(this);
	if(!m_pSaveList->init(m_pListObjectInfo->getSchema().c_str()))
	{
		Con_EPrintf("Failed to initialize 'CUIScrollableSurface' for the save/load window.\n");
		return false;
	}

	m_pSaveList->setHeaderColumnName(0, "Title");
	m_pSaveList->setHeaderColumnName(1, "Type");
	m_pSaveList->setHeaderColumnName(2, "Elapsed Time");
	m_pSaveList->setHeaderColumnName(3, "Date");
	
	m_rowTextInset = m_pListObjectInfo->getTextInset();

	// Load save files
	LoadSaves(isIngame);

	// Add "Cancel" button
	const ui_objectinfo_t* pCancelButtonObjectInfo = pWinDesc->getObject(UI_OBJECT_BUTTON, CANCEL_BUTTON_OBJ_NAME);
	if(!pCancelButtonObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, CANCEL_BUTTON_OBJ_NAME);
		return false;
	}

	CUISaveLoadWindowButtonEvent* pEventCancel = new CUISaveLoadWindowButtonEvent(this, SL_BUTTON_CANCEL);
	CUIButton* pCancelButton = new CUIButton(pCancelButtonObjectInfo->getFlags(), 
		pCancelButtonObjectInfo->getText().c_str(), 
		pCancelButtonObjectInfo->getFont(), 
		pEventCancel, 
		pCancelButtonObjectInfo->getWidth(),
		pCancelButtonObjectInfo->getHeight(), 
		pWindowObject->getXInset() + pCancelButtonObjectInfo->getXOrigin(), 
		pWindowObject->getYInset() + pCancelButtonObjectInfo->getYOrigin());

	pCancelButton->setParent(this);

	if(!pCancelButton->init(pCancelButtonObjectInfo->getSchema().c_str()))
	{
		Con_EPrintf("Failed to initialize button object for save/load UI window.\n");
		return false;
	}

	// Add "Load game" button
	const ui_objectinfo_t* pLoadGameButtonObjectInfo = pWinDesc->getObject(UI_OBJECT_BUTTON, LOAD_GAME_BUTTON_OBJ_NAME);
	if(!pLoadGameButtonObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, LOAD_GAME_BUTTON_OBJ_NAME);
		return false;
	}

	CUISaveLoadWindowButtonEvent* pEventLoad = new CUISaveLoadWindowButtonEvent(this, SL_BUTTON_LOAD_GAME);
	m_pLoadButton = new CUIButton(pLoadGameButtonObjectInfo->getFlags(), 
		pLoadGameButtonObjectInfo->getText().c_str(), 
		pLoadGameButtonObjectInfo->getFont(), 
		pEventLoad, 
		pLoadGameButtonObjectInfo->getWidth(),
		pLoadGameButtonObjectInfo->getHeight(), 
		pWindowObject->getXInset() + pLoadGameButtonObjectInfo->getXOrigin(), 
		pWindowObject->getYInset() + pLoadGameButtonObjectInfo->getYOrigin());

	m_pLoadButton->setParent(this);

	if(!m_pLoadButton->init(pLoadGameButtonObjectInfo->getSchema().c_str()))
	{
		Con_EPrintf("Failed to initialize button object for save/load UI window.\n");
		return false;
	}

	// Add "Delete Save" button
	const ui_objectinfo_t* pDeleteSaveButtonObjectInfo = pWinDesc->getObject(UI_OBJECT_BUTTON, DELETE_SAVE_BUTTON_OBJ_NAME);
	if(!pDeleteSaveButtonObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, DELETE_SAVE_BUTTON_OBJ_NAME);
		return false;
	}

	CUISaveLoadWindowButtonEvent* pEventDelete = new CUISaveLoadWindowButtonEvent(this, SL_BUTTON_DELETE);
	m_pDeleteButton = new CUIButton(pDeleteSaveButtonObjectInfo->getFlags(), 
		pDeleteSaveButtonObjectInfo->getText().c_str(), 
		pDeleteSaveButtonObjectInfo->getFont(), 
		pEventDelete, 
		pDeleteSaveButtonObjectInfo->getWidth(),
		pDeleteSaveButtonObjectInfo->getHeight(), 
		pWindowObject->getXInset() + pDeleteSaveButtonObjectInfo->getXOrigin(), 
		pWindowObject->getYInset() + pDeleteSaveButtonObjectInfo->getYOrigin());

	m_pDeleteButton->setParent(this);

	if(!m_pDeleteButton->init(pDeleteSaveButtonObjectInfo->getSchema().c_str()))
	{
		Con_EPrintf("Failed to initialize button object for save/load UI window.\n");
		return false;
	}

	// Add "Save game" button
	const ui_objectinfo_t* pSaveGameButtonObjectInfo = pWinDesc->getObject(UI_OBJECT_BUTTON, SAVE_GAME_BUTTON_OBJ_NAME);
	if(!pSaveGameButtonObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, SAVE_GAME_BUTTON_OBJ_NAME);
		return false;
	}

	CUISaveLoadWindowButtonEvent* pEventSave = new CUISaveLoadWindowButtonEvent(this, SL_BUTTON_SAVE_GAME);
	m_pSaveButton = new CUIButton(pSaveGameButtonObjectInfo->getFlags(), 
		pSaveGameButtonObjectInfo->getText().c_str(), 
		pSaveGameButtonObjectInfo->getFont(), 
		pEventSave, 
		pSaveGameButtonObjectInfo->getWidth(),
		pSaveGameButtonObjectInfo->getHeight(), 
		pWindowObject->getXInset() + pSaveGameButtonObjectInfo->getXOrigin(), 
		pWindowObject->getYInset() + pSaveGameButtonObjectInfo->getYOrigin());

	m_pSaveButton->setParent(this);

	if(!m_pSaveButton->init(pSaveGameButtonObjectInfo->getSchema().c_str()))
	{
		Con_EPrintf("Failed to initialize button object for save/load UI window.\n");
		return false;
	}

	// Ensure these are updated
	UpdateButtons(false);

	return true;
}

//=============================================
// @brief Updates buttons
//
//=============================================
void CUISaveLoadWindow::UpdateButtons( bool selectFirst )
{
	if(m_bIsIngame)
	{
		// Set focus specifically on the new save
		SetFocusOnRow(NEW_SAVE_INDEX, NEW_SAVE_INDEX);
	}
	else if(selectFirst && !m_saveFilesArray.empty())
	{
		// Set focus on the first row
		SetFocusOnRow(0, 0);
	}
	else
	{
		m_pLoadButton->setDisabled(true);
		m_pSaveButton->setDisabled(true);
		m_pDeleteButton->setDisabled(true);
	}
}

//=============================================
// @brief Peforms the action of the button
//
//=============================================
void CUISaveLoadWindow::setFocusState( bool inFocus ) 
{ 
	if(inFocus && m_lastSelectedSave != -1)
	{
		save_file_t* psave = &m_saveFilesArray[m_lastSelectedSave];
		SetBackgroundTexture(psave);
	}
	else if(!inFocus && m_lastSelectedSave != -1)
	{
		m_lastSelectedSave = -1;
		gMenu.SetBlendTargetTexture(nullptr);
		m_pSaveList->clearHighlight();
	}

	// Call base class to do the rest
	CUIWindow::setFocusState( inFocus );
}

//=============================================
// @brief Peforms the action of the button
//
//=============================================
void CUISaveLoadWindow::LoadSaves( bool isIngame )
{
	if(!m_saveFilesArray.empty())
		m_saveFilesArray.clear();

	if(m_pSaveList->getNbRows() > 0)
		m_pSaveList->clearList();

	// Add save game element
	if(isIngame)
	{
		CUISaveLoadWindowRowEvent* pEvent = new CUISaveLoadWindowRowEvent(this, NEW_SAVE_INDEX, NEW_SAVE_INDEX);

		CUIListRow* pRow = m_pSaveList->createNewRow(pEvent, m_rowTextInset);

		CUIText* pTextTitle = new CUIText((UIEL_FL_ALIGN_CV|UIEL_FL_ALIGN_CH), m_pListObjectInfo->getFont(), "Create new save", 0, 0, 0);
		pRow->setColumnContents(0, pTextTitle);

		CUIText* pTextType = new CUIText((UIEL_FL_ALIGN_CV|UIEL_FL_ALIGN_CH), m_pListObjectInfo->getFont(), "New", 0, 0, 0);
		pRow->setColumnContents(1, pTextType);
	}

	// Load save files list
	CString searchPath;
	searchPath << ens.gamedir << PATH_SLASH_CHAR << CSaveRestore::SAVE_DIR_PATH << "*" << SAVE_FILE_EXTENSION;

	WIN32_FIND_DATAA findData;
	HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			CString filePath;
			filePath << CSaveRestore::SAVE_DIR_PATH << findData.cFileName;
			
			AddSaveFileInfo(filePath, findData.ftLastWriteTime);

		} while(FindNextFileA(hFind, &findData));
		
		FindClose(hFind);
	}

	// Re-organize save files based on date
	if(!m_saveFilesArray.empty())
	{
		qsort(&m_saveFilesArray[0], m_saveFilesArray.size(), sizeof(save_file_t), SortSaves);

		for(Uint32 i = 0; i < m_saveFilesArray.size(); i++)
			CreateSaveFileRow(m_saveFilesArray[i], i);
	}
}

//=============================================
// @brief Handles think functionalities
//
//=============================================
void CUISaveLoadWindow::postThink( void )
{
	// Perform regular think functions
	CUIWindow::postThink();

	// Reset if needed
	if(m_bRecheckSaves)
	{
		LoadSaves((ens.gamestate == GAME_RUNNING) ? true : false);
		m_bRecheckSaves = false;

		UpdateButtons(true);
	}
}

//=============================================
// @brief Peforms the action of the button
//
//=============================================
void CUISaveLoadWindow::SetBackgroundTexture( save_file_t* psave )
{
	if(!psave)
	{
		gMenu.SetBlendTargetTexture(nullptr);
		return;
	}

	CTextureManager* pTextureManager = CTextureManager::GetInstance();
	en_texture_t* ptexture = nullptr;
	bool isCurrentBgTexture = false;

	// Delete previous texture entry, if the save is a quick save, or autosave
	// as these get updated regularly
	CString filepath(psave->filepath);
	if(filepath.find(0, QUICKSAVE_FILE_NAME) != -1 
		|| filepath.find(0, AUTOSAVE_FILE_NAME) != -1)
	{
		// Remove
		ptexture = pTextureManager->FindTexture(filepath.c_str(), RS_WINDOW_LEVEL);
		if(ptexture)
		{
			if(gMenu.GetCurrentBgTexture() == ptexture)
				isCurrentBgTexture = true;

			gMenu.OnBgTextureDeleted(ptexture);
			pTextureManager->DeleteTexture(ptexture);
		}
	}
	else
	{
		// Try to find linked texture first
		ptexture = pTextureManager->FindTexture(psave->filepath, RS_WINDOW_LEVEL);
		if(ptexture)
		{
			gMenu.SetBlendTargetTexture(ptexture);
			return;
		}
	}

	// Retrieve the screenshot from the file
	const byte* pfile = gSaveRestore.LoadSaveFile(psave->filepath);
	if(!pfile)
		return;

	const save_header_t* pheader = reinterpret_cast<const save_header_t*>(pfile);
	if(!pheader->screenshotdatasize 
		|| !pheader->screenshotheight
		|| !pheader->screenshotwidth
		|| !pheader->screenshotoffset)
	{
		gSaveRestore.FreeSaveFile(pfile);
		return;
	}

	// Bind the texture to GL
	const byte* pimagedata = pfile + pheader->screenshotoffset;
	ptexture = pTextureManager->LoadFromMemory(psave->filepath, RS_WINDOW_LEVEL, (TX_FL_DXT1|TX_FL_NOMIPMAPS|TX_FL_CLAMP_S|TX_FL_CLAMP_T), pimagedata, pheader->screenshotwidth, pheader->screenshotheight, pheader->screenshotbpp, pheader->screenshotdatasize);
	if(!ptexture)
	{
		gSaveRestore.FreeSaveFile(pfile);
		return;
	}

	if(isCurrentBgTexture)
	{
		// Just restore
		gMenu.SetCurrentBgTexture(ptexture);
	}
	else
	{
		// Change menu bg texture to the save file
		gMenu.SetBlendTargetTexture(ptexture);
		gMenu.AddSaveBackgroundTexture(ptexture);
		gSaveRestore.FreeSaveFile(pfile);
	}
}

//=============================================
// @brief Peforms the action of the button
//
//=============================================
void CUISaveLoadWindow::SetFocusOnRow( Int32 rowIndex, Int32 fileIndex )
{
	if(rowIndex == NEW_SAVE_INDEX)
	{
		m_pSaveList->setHighlightOnRow(0, true);
		m_lastSelectedSave = NEW_SAVE_INDEX;
		m_pLoadButton->setDisabled(true);
		m_pSaveButton->setDisabled(false);
		m_pDeleteButton->setDisabled(true);
		SetBackgroundTexture(nullptr);
		return;
	}

	assert(rowIndex >= 0 && rowIndex < (Int32)m_pSaveList->getNbRows());
	m_pSaveList->setHighlightOnRow(rowIndex, true);
	
	// Retreive the image from the save file
	if(!m_saveFilesArray.empty())
	{
		assert(fileIndex >= 0 && fileIndex < (Int32)m_saveFilesArray.size());
		save_file_t* psave = &m_saveFilesArray[fileIndex];

		m_lastSelectedSave = fileIndex;
		if(!qstrlen(psave->filepath))
		{
			gMenu.SetBlendTargetTexture(nullptr);
			return;
		}

		// Set to save's screenshot
		SetBackgroundTexture(psave);
	}

	m_pLoadButton->setDisabled(false);
	m_pSaveButton->setDisabled(true);
	m_pDeleteButton->setDisabled(false);
}

//=============================================
// @brief Peforms the action of the button
//
//=============================================
void CUISaveLoadWindow::RowDoubleClickEvent( Int32 rowIndex )
{
	if(rowIndex == NEW_SAVE_INDEX)
	{
		gCommands.AddCommand("save");
		CUISaveLoadWindow::DestroyInstance();
		return;
	}

	// If it's a double click, then check what we clicked on
	assert(rowIndex >= 0 && rowIndex < (Int32)m_saveFilesArray.size());
	save_file_t* psave = &m_saveFilesArray[rowIndex];

	// Just load the selected file
	LoadSave(psave->filepath);
	CUISaveLoadWindow::DestroyInstance();
}

//=============================================
// @brief Loads a save
//
//=============================================
void CUISaveLoadWindow::LoadSave( const Char* pstrSavePath )
{
	// Load the mentioned save
	CString command;
	command << "load " << pstrSavePath;

	gCommands.AddCommand(command.c_str());
}

//=============================================
// @brief Deletes a save
//
//=============================================
void CUISaveLoadWindow::DeleteSave( const Char* pstrSavePath )
{
	// Load the mentioned save
	if(!FL_DeleteFile(pstrSavePath))
	{
		Con_EPrintf("%s - Could not delete save file '%s'.\n", __FUNCTION__, pstrSavePath);
		return;
	}

	// Force a re-check
	m_bRecheckSaves = true;
}

//=============================================
// @brief Peforms the action of the button
//
//=============================================
bool CUISaveLoadWindow::keyEvent( Int32 button, Int16 mod, bool keyDown )
{
	if(button == SDL_SCANCODE_RETURN && m_lastSelectedSave != -1)
	{
		// Just load the selected save, or save the new one
		RowDoubleClickEvent(m_lastSelectedSave);
		return true;
	}

	return CUIWindow::keyEvent(button, mod, keyDown);
}

//=============================================
// @brief Peforms the action of the button
//
//=============================================
void CUISaveLoadWindow::ButtonEvent( slwindow_btn_id btn )
{
	switch(btn)
	{
	case SL_BUTTON_LOAD_GAME:
		{
			if(m_lastSelectedSave == -1)
				return;

			RowDoubleClickEvent(m_lastSelectedSave);
		}
		break;
	case SL_BUTTON_SAVE_GAME:
		if(m_lastSelectedSave == -1)
		{
			gCommands.AddCommand("save");
			m_bRecheckSaves = true;
		}
		break;
	case SL_BUTTON_DELETE:
		{
			// If it's a double click, then check what we clicked on
			assert(m_lastSelectedSave >= 0 && m_lastSelectedSave < (Int32)m_saveFilesArray.size());
			save_file_t* psave = &m_saveFilesArray[m_lastSelectedSave];

			DeleteSave(psave->filepath);
		};
		break;
	case SL_BUTTON_CANCEL:
		CUISaveLoadWindow::DestroyInstance();
		break;
	}
}

//=============================================
// @brief Peforms the action of the button
//
//=============================================
bool CUISaveLoadWindowRowEvent::MouseButtonEvent( Int32 mouseX, Int32 mouseY, Int32 button, bool keyDown )
{
	// Only left mouse click affects draggers
	if(button != SDL_BUTTON_LEFT && !keyDown)
		return false;

	if(keyDown)
	{
		Float interval = (Float)ens.time - m_lastClickTime;
		if(interval < 0.5)
		{
			// Enter bind mode
			m_pWindow->RowDoubleClickEvent(m_fileIndex);
		}
		else
		{
			// Set focus to this row
			m_pWindow->SetFocusOnRow(m_rowIndex, m_fileIndex);
		}

		m_lastClickTime = ens.time;
	}

	return true;
}

//=============================================
// @brief Peforms the action of the button
//
//=============================================
void CUISaveLoadWindowButtonEvent::PerformAction( Float param )
{
	m_pWindow->ButtonEvent(m_buttonId);
}
