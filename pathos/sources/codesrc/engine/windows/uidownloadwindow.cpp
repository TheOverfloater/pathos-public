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

#include "uimanager.h"
#include "uielements.h"
#include "uidownloadwindow.h"

#include "window.h"
#include "commands.h"

#include "r_basicdraw.h"
#include "r_main.h"
#include "system.h"
#include "cl_resource.h"

// Current instance of the window
CUIDownloadWindow* CUIDownloadWindow::m_pInstance = nullptr;

// Window description file
const Char CUIDownloadWindow::WINDOW_DESC_FILE[] = "downloadwindow.txt";
// Window description file
const Char CUIDownloadWindow::WINDOW_OBJ_NAME[] = "DownloadWindow";
// Cancel button object name
const Char CUIDownloadWindow::CANCEL_BUTTON_OBJ_NAME[] = "CancelButton";
// Save and Quit button object name
const Char CUIDownloadWindow::FILE_DOWNLOAD_LABEL_OBJ_NAME[] = "FileDownloadLabel";
// Quit button object name
const Char CUIDownloadWindow::FILE_DOWNLOAD_PROGRESSBAR_OBJ_NAME[] = "FileDownloadProgressBar";
// Save and Quit button object name
const Char CUIDownloadWindow::TOTAL_PROGRESS_LABEL_OBJ_NAME[] = "TotalProgressLabel";
// Quit button object name
const Char CUIDownloadWindow::TOTAL_PROGRESS_PROGRESSBAR_OBJ_NAME[] = "TotalProgressBar";

//=============================================
// @brief Constructor
//
//=============================================
CUIDownloadWindow::CUIDownloadWindow( Int32 flags, Uint32 width, Uint32 height, Int32 originx, Int32 originy ):
	CUIWindow(UIW_FL_MENUWINDOW|UIW_FL_DOWNLOADWINDOW, flags, width, height, originx, originy),
	m_pDownloadFileText(nullptr),
	m_pFileProgressBar(nullptr),
	m_pTotalProgressBar(nullptr)
{
}

//=============================================
// @brief Destructor
//
//=============================================
CUIDownloadWindow::~CUIDownloadWindow( void )
{
	// Probably destroyed by the UI manager
	if(m_pInstance)
		m_pInstance = nullptr;
}

//=============================================
// @brief Creates an instance of the console window
//
//=============================================
CUIDownloadWindow* CUIDownloadWindow::CreateInstance( void )
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
	CUIDownloadWindow* pWindow = new CUIDownloadWindow(pWindowObject->getFlags(), pWindowObject->getWidth(), pWindowObject->getHeight(), xOrigin, yOrigin);
	if(!pWindow->init(pWinDesc, pWindowObject))
	{
		Con_EPrintf("Failed to initialize download window.\n");
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
void CUIDownloadWindow::DestroyInstance( void )
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
CUIDownloadWindow* CUIDownloadWindow::GetInstance( void )
{
	return m_pInstance;
}

//=============================================
// @brief Initializes the tab element with a schema
//
//=============================================
bool CUIDownloadWindow::init( const ui_windowdescription_t* pWinDesc, const ui_objectinfo_t* pWindowObject )
{
	// Retreive schema for the window object
	const Char* pstrSchemaName = pWindowObject->getSchema().c_str();
	if(!CUIWindow::init(pstrSchemaName))
	{
		Con_EPrintf("Failed to initialize schema '%s' for console window.\n", pstrSchemaName);
		return false;
	}

	// Create the dragger
	if(pWindowObject->dragger)
	{
		CUIDragger* pDragger = new CUIDragger(UIEL_FL_FIXED_H, m_width-40, 30, 0, 0);
		pDragger->setParent(this);
	}

	// Set the window's properties
	setAlpha(pWindowObject->getAlpha());
	setTitle(pWindowObject->getTitle().c_str(), pWindowObject->getFont(), pWindowObject->getXInset(), pWindowObject->getYInset());

	// Create cancel button
	const ui_objectinfo_t* pCancelButtonObjectInfo = pWinDesc->getObject(UI_OBJECT_BUTTON, CANCEL_BUTTON_OBJ_NAME);
	if(!pCancelButtonObjectInfo)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, CANCEL_BUTTON_OBJ_NAME);
		return false;
	}

	CUIDownloadWindowCancelEvent* pEvent = new CUIDownloadWindowCancelEvent(this);
	CUIButton* pCancelButton = new CUIButton(pCancelButtonObjectInfo->getFlags(), 
		pCancelButtonObjectInfo->getText().c_str(), 
		pCancelButtonObjectInfo->getFont(), pEvent, 
		pCancelButtonObjectInfo->getWidth(), 
		pCancelButtonObjectInfo->getHeight(), 
		pWindowObject->getXInset() + pCancelButtonObjectInfo->getXOrigin(), 
		pWindowObject->getYInset() + pCancelButtonObjectInfo->getYOrigin());

	pCancelButton->setParent(this);
	
	if(!pCancelButton->init(pCancelButtonObjectInfo->getSchema().c_str()))
	{
		Con_EPrintf("Failed to initialize button object for download UI window.\n");
		return false;
	}

	// Get text object
	const ui_objectinfo_t* pFileDownloadTextObjectName = pWinDesc->getObject(UI_OBJECT_TEXT, FILE_DOWNLOAD_LABEL_OBJ_NAME);
	if(!pFileDownloadTextObjectName)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, FILE_DOWNLOAD_LABEL_OBJ_NAME);
		return false;
	}

	// Set appropriate text
	m_pDownloadFileText = new CUIText(pFileDownloadTextObjectName->getFlags(), 
		pFileDownloadTextObjectName->getFont(), 
		pFileDownloadTextObjectName->getText().c_str(), 
		pWindowObject->getXInset() + pFileDownloadTextObjectName->getXOrigin(), 
		pWindowObject->getYInset() + pFileDownloadTextObjectName->getYOrigin());
	m_pDownloadFileText->setParent(this);

	// Save this
	m_downloadBarTextBase = pFileDownloadTextObjectName->getText();

	// Create progress bar object
	const ui_objectinfo_t* pFileDownloadBarObjectName = pWinDesc->getObject(UI_OBJECT_TAB, FILE_DOWNLOAD_PROGRESSBAR_OBJ_NAME);
	if(!pFileDownloadBarObjectName)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, FILE_DOWNLOAD_PROGRESSBAR_OBJ_NAME);
		return false;
	}

	// Set appropriate text
	m_pFileProgressBar = new CUIProgressBar(pFileDownloadBarObjectName->getFlags(), 
		pFileDownloadBarObjectName->getWidth(),
		pFileDownloadBarObjectName->getHeight(),
		pWindowObject->getXInset() + pFileDownloadBarObjectName->getXOrigin(), 
		pWindowObject->getYInset() + pFileDownloadBarObjectName->getYOrigin());
	m_pFileProgressBar->setParent(this);

	if(!m_pFileProgressBar->init(pFileDownloadBarObjectName->getSchema().c_str()))
	{
		Con_EPrintf("Failed to initialize progress bar object for download UI window.\n");
		return false;
	}

	// Get text object
	const ui_objectinfo_t* pTotalProgressTextObjectName = pWinDesc->getObject(UI_OBJECT_TEXT, TOTAL_PROGRESS_LABEL_OBJ_NAME);
	if(!pTotalProgressTextObjectName)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, TOTAL_PROGRESS_LABEL_OBJ_NAME);
		return false;
	}

	// Set appropriate text
	CUIText* pTotalProgressText = new CUIText(pTotalProgressTextObjectName->getFlags(), 
		pTotalProgressTextObjectName->getFont(), 
		pTotalProgressTextObjectName->getText().c_str(), 
		pWindowObject->getXInset() + pTotalProgressTextObjectName->getXOrigin(), 
		pWindowObject->getYInset() + pTotalProgressTextObjectName->getYOrigin());
	pTotalProgressText->setParent(this);

	// Create progress bar object
	const ui_objectinfo_t* pFileTotalProgressBarObjectName = pWinDesc->getObject(UI_OBJECT_TAB, TOTAL_PROGRESS_PROGRESSBAR_OBJ_NAME);
	if(!pFileTotalProgressBarObjectName)
	{
		Con_EPrintf("Window description file '%s' has no definition for '%s'.\n", WINDOW_DESC_FILE, TOTAL_PROGRESS_PROGRESSBAR_OBJ_NAME);
		return false;
	}

	// Set appropriate text
	m_pTotalProgressBar = new CUIProgressBar(pFileTotalProgressBarObjectName->getFlags(), 
		pFileTotalProgressBarObjectName->getWidth(),
		pFileTotalProgressBarObjectName->getHeight(),
		pWindowObject->getXInset() + pFileTotalProgressBarObjectName->getXOrigin(), 
		pWindowObject->getYInset() + pFileTotalProgressBarObjectName->getYOrigin());
	m_pTotalProgressBar->setParent(this);

	if(!m_pTotalProgressBar->init(pFileTotalProgressBarObjectName->getSchema().c_str()))
	{
		Con_EPrintf("Failed to initialize progress bar object for download UI window.\n");
		return false;
	}

	return true;
}

//=============================================
// @brief Manages a button event
//
//=============================================
void CUIDownloadWindow::SetFileProgressBar( Float value )
{
	if(!m_pFileProgressBar)
		return;

	m_pFileProgressBar->setValue(value);
}

//=============================================
// @brief Manages a button event
//
//=============================================
void CUIDownloadWindow::SetTotalProgressBar( Float value )
{
	if(!m_pTotalProgressBar)
		return;

	m_pTotalProgressBar->setValue(value);
}

//=============================================
// @brief Manages a button event
//
//=============================================
void CUIDownloadWindow::SetDownloadFileName( const Char* pstr )
{
	if(!m_pDownloadFileText)
		return;

	CString text;
	text << m_downloadBarTextBase << pstr;

	m_pDownloadFileText->setText(text.c_str());
}


//=============================================
// @brief Manages a button event
//
//=============================================
void CUIDownloadWindow::CancelDownload( void )
{
	// Disconnect from server
	CL_CancelDownload();

	// Delete window
	CUIDownloadWindow::DestroyInstance();
}