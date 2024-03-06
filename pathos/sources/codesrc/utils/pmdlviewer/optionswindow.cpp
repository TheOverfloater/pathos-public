//
//                 Half-Life Model Viewer (c) 1999 by Mete Ciragan
//
// file:           COptionsWindow.cpp
// last modified:  May 04 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
// version:        1.2
//
// email:          mete@swissquake.ch
// web:            http://www.swissquake.ch/chumbalum-soft/
//

#include <mx/mx.h>

#include "includes.h"
#include "optionswindow.h"
#include "viewerstate.h"
#include "config.h"

// Compiler path setting name
const Char COptionsWindow::OW_COMPILER_PATH[] = "COMPILER_PATH";
// Compiler args setting name
const Char COptionsWindow::OW_COMPILER_ARGS[] = "COMPILER_ARGS";
// Decompiler path setting name
const Char COptionsWindow::OW_DECOMPILER_PATH[] = "DECOMPILER_PATH";
// Mod folder path setting name
const Char COptionsWindow::OW_MOD_FOLDER_PATH[] = "MOD_FOLDER_PATH";
// Compiler window title
const Char COptionsWindow::WINDOW_TITLE[] = "Options Window";

// Compiler window x origin
const Uint32 COptionsWindow::WINDOW_X_ORIGIN = 100;
// Compiler window y origin
const Uint32 COptionsWindow::WINDOW_Y_ORIGIN = 100;
// Compiler window width
const Uint32 COptionsWindow::WINDOW_WIDTH = 400;
// Compiler window height
const Uint32 COptionsWindow::WINDOW_HEIGHT = 250;

// Current instance of this class
COptionsWindow* COptionsWindow::g_pInstance = nullptr;

//=============================================
// @brief Constructor
//
//=============================================
COptionsWindow::COptionsWindow(): 
	mxWindow(0, WINDOW_X_ORIGIN, WINDOW_Y_ORIGIN, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, mxWindow::Dialog),
	m_pButtonChooseCompiler(nullptr),
	m_pLineEditCompilerPath(nullptr),
	m_pLineEditCompilerArgs(nullptr),
	m_pButtonChooseDecompiler(nullptr),
	m_pLineEditDecompilerPath(nullptr),
	m_pButtonSetModFolderPath(nullptr),
	m_pLineEditModFolderPath(nullptr)
{
	new mxLabel (this, 5, 5, 150, 18, "Compiler path");
	m_pLineEditCompilerPath = new mxLineEdit(this, 5, 25, 320, 22, "", IDOW_COMPILER_PATH);
	m_pButtonChooseCompiler = new mxButton(this, 325, 25, 22, 22, "...", IDOW_COMPILER_BUTTON);

	new mxLabel(this, 5, 50, 150, 18, "Compiler arguments");
	m_pLineEditCompilerArgs = new mxLineEdit(this, 5, 70, 320, 22, "", IDOW_COMPILER_ARGS);

	new mxLabel(this, 5, 95, 150, 18, "Decompiler path");
	m_pLineEditDecompilerPath = new mxLineEdit(this, 5, 115, 320, 22, "", IDOW_DECOMPILER_PATH);
	m_pButtonChooseDecompiler = new mxButton(this, 325, 115, 22, 22, "...", IDOW_DECOMPILER_BUTTON);

	new mxLabel(this, 5, 140, 150, 18, "Mod folder path");
	m_pLineEditModFolderPath = new mxLineEdit(this, 5, 160, 320, 22, "", IDOW_MOD_FOLDER_PATH);
	m_pButtonSetModFolderPath = new mxButton(this, 325, 160, 22, 22, "...", IDOW_MOD_FOLDER_BUTTON);

	new mxButton(this, 5, 190, 75, 22, "Ok", IDOW_OK);
	new mxButton(this, 110, 190, 75, 22, "Cancel", IDOW_CANCEL);

	const Char* ptrValue = gConfig.GetOptionValue(OW_COMPILER_PATH);
	if(ptrValue)
		m_pLineEditCompilerPath->setLabel(ptrValue);

	ptrValue = gConfig.GetOptionValue(OW_COMPILER_ARGS);
	if(ptrValue)
		m_pLineEditCompilerArgs->setLabel(ptrValue);

	ptrValue = gConfig.GetOptionValue(OW_DECOMPILER_PATH);
	if(ptrValue)
		m_pLineEditDecompilerPath->setLabel(ptrValue);

	ptrValue = gConfig.GetOptionValue(OW_MOD_FOLDER_PATH);
	if(ptrValue)
		m_pLineEditModFolderPath->setLabel(ptrValue);
}

//=============================================
// @brief Destructor
//
//=============================================
COptionsWindow::~COptionsWindow( void )
{
	// mx deletes these windows before we 
	// get a chance to do it via DeleteInstance()
	g_pInstance = nullptr;
}

//=============================================
// @brief Handles an mx event
//
//=============================================
int COptionsWindow::handleEvent (mxEvent *pEvent)
{
	if (pEvent->event != mxEvent::Action)
		return 0;

	switch (pEvent->action)
	{
	case IDOW_COMPILER_BUTTON:
		{
			const Char *pstrFilePath = mxGetOpenFileName(this, nullptr, "*.exe");
			if(pstrFilePath)
			{
				m_pLineEditCompilerPath->setLabel(pstrFilePath);
				gConfig.SetOption(OW_COMPILER_PATH, pstrFilePath);
				redraw();
			}
		}
		break;
	case IDOW_DECOMPILER_BUTTON:
		{
			const Char *pstrFilePath = mxGetOpenFileName(this, nullptr, "*.exe");
			if(pstrFilePath)
			{
				m_pLineEditDecompilerPath->setLabel(pstrFilePath);
				gConfig.SetOption(OW_DECOMPILER_PATH, pstrFilePath);
				redraw();
			}
		}
		break;
	case IDOW_MOD_FOLDER_BUTTON:
		{
			const Char *pstrFolderPath = mxGetSelectFolder(this);
			if (pstrFolderPath)
			{
				m_pLineEditModFolderPath->setLabel(pstrFolderPath);
				gConfig.SetOption(OW_MOD_FOLDER_PATH, pstrFolderPath);
				redraw();

				vs.moddir_path = pstrFolderPath;
			}
		}
		break;
	case IDOW_DECOMPILER_PATH:
		{
			gConfig.SetOption(OW_DECOMPILER_PATH, m_pLineEditDecompilerPath->getLabel());
		}
		break;
	case IDOW_COMPILER_PATH:
		{
			gConfig.SetOption(OW_COMPILER_PATH, m_pLineEditCompilerPath->getLabel());
		}
		break;
	case IDOW_MOD_FOLDER_PATH:
		{
			const Char* pstrModFolderPath = m_pLineEditModFolderPath->getLabel();
			gConfig.SetOption(OW_MOD_FOLDER_PATH, pstrModFolderPath);
			vs.moddir_path = pstrModFolderPath;
		}
		break;
	case IDOW_COMPILER_ARGS:
		{
			gConfig.SetOption(OW_COMPILER_ARGS, m_pLineEditCompilerArgs->getLabel());
		}
		break;
	case IDOW_OK:
		{
			gConfig.SaveOptions();
			setVisible(false);
		}
		break;
	case IDOW_CANCEL:
		{
			setVisible (false);
		}
		break;
	}

	return 1;
}

//=============================================
// @brief Creates an instance of this class
//
// @return Created instance
//=============================================
COptionsWindow* COptionsWindow::CreateInstance( void )
{
	if(!g_pInstance)
		g_pInstance = new COptionsWindow();

	return g_pInstance;
}

//=============================================
// @brief Returns the current instance of this class
//
// @return Current instance, or nullptr
//=============================================
COptionsWindow* COptionsWindow::GetInstance( void )
{
	return g_pInstance;
}

//=============================================
// @brief Deletes the current instance of this class
//
//=============================================
void COptionsWindow::DeleteInstance( void )
{
	if(!g_pInstance)
		return;

	g_pInstance = nullptr;
}