//
//                 Half-Life Model Viewer (c) 1999 by Mete Ciragan
//
// file:           pakviewer.cpp
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mx/mx.h>
#include "folderviewer.h"
#include "mdlviewer.h"
#include "GlWindow.h"
#include "ControlPanel.h"
#include "FileAssociation.h"

// Current instance of this class
CFolderViewer* CFolderViewer::g_pInstance = nullptr;

//=============================================
// @brief Constructor
//
//=============================================
CFolderViewer::CFolderViewer( mxWindow *pWindow ): 
	mxWindow(pWindow, 0, 0, 0, 0, "", mxWindow::Normal),
	m_pFolderTreeView(nullptr),
	m_pPopupMenu(nullptr)
{
	// Create tree view
	m_pFolderTreeView = new mxTreeView(this, 0, 0, 0, 0, IDC_FOLDERVIEWER);

	// Create popup menu
	m_pPopupMenu = new mxPopupMenu();
	m_pPopupMenu->add("Load Model", 1);

	// Make invisible at first
	setVisible(false);
}

//=============================================
// @brief Destructor
//
//=============================================
CFolderViewer::~CFolderViewer()
{
	// mx deletes these windows before we 
	// get a chance to do it via DeleteInstance()
	g_pInstance = nullptr;
}

//=============================================
// @brief handleEvent
//
//=============================================
Int32 CFolderViewer::handleEvent ( mxEvent *pEvent )
{
	switch (pEvent->event)
	{
		case mxEvent::Action:
		{
			switch (pEvent->action)
			{
				case IDC_FOLDERVIEWER:
				{
					if (pEvent->flags & mxEvent::RightClicked)
					{
						bool enable = qstrcicmp(m_currentModelFilePath, ".vbm") != 0 ? true : false;
						m_pPopupMenu->setEnabled(1, enable);

						Int32 ret = m_pPopupMenu->popup(m_pFolderTreeView, pEvent->x, pEvent->y);
						switch(ret)
						{
						case 1:
							OnLoadModel();
							break;
						}
					}
					else if (pEvent->flags & mxEvent::DoubleClicked)
					{
						OnLoadModel();
					}
			
					return OnFolderViewer();
				}
			}
		}
		break;
		case mxEvent::Size:
		{
			m_pFolderTreeView->setBounds (0, 0, pEvent->width, pEvent->height);
		}
		break;
	}

	return 1;
}

//=============================================
// @brief OnFolderViewer
//
//=============================================
Int32 CFolderViewer::OnFolderViewer( void )
{
	mxTreeViewItem *pItem = m_pFolderTreeView->getSelectedItem ();
	if (pItem)
	{
		CString modelname = m_pFolderTreeView->getLabel(pItem);

		// find the full path
		CString path;
		mxTreeViewItem *pParentItem = m_pFolderTreeView->getParent(pItem);
		while(pParentItem)
		{
			CString prevStr = path;
			CString currentItemStr = m_pFolderTreeView->getLabel(pParentItem);
			path << currentItemStr << "/" << prevStr;

			pParentItem = m_pFolderTreeView->getParent(pParentItem);
		}
		
		m_currentModelFilePath = m_folderPath;
		if(m_folderPath[m_folderPath.length()-1] != '/' 
			&& m_folderPath[m_folderPath.length()-1] != '\\')
			m_currentModelFilePath << PATH_SLASH_CHAR;

		m_currentModelFilePath << path << modelname;
	}

	return 1;
}

//=============================================
// @brief OnLoadModel
//
//=============================================
Int32 CFolderViewer::OnLoadModel( void )
{
	if(m_currentModelFilePath.empty())
		return 1;

	CControlPanel::GetInstance()->LoadModel(m_currentModelFilePath.c_str());
	return 1;
}

//=============================================
// @brief OnLoadModel
//
//=============================================
bool CFolderViewer::OpenFolder( const Char *pstrFolderPath ) 
{
	// Set current folder path
	m_folderPath = pstrFolderPath;

	// Clear previous list
	m_pFolderTreeView->removeAll();

	CString searchPath;
	searchPath << pstrFolderPath << PATH_SLASH_CHAR << "*.vbm";

	// Parse directory for files
	HANDLE dir;
	WIN32_FIND_DATA file_data;
	if ((dir = FindFirstFile(searchPath.c_str(), &file_data)) == INVALID_HANDLE_VALUE)
		return false;

	// Add the VBM files to the list
	while (TRUE) 
	{
		m_pFolderTreeView->add(nullptr, file_data.cFileName);

		if(!FindNextFile(dir, &file_data))
			break;
	}

	// Show folder viewer
	setVisible (true);

	return true;
}

//=============================================
// @brief OnLoadModel
//
//=============================================
void CFolderViewer::CloseFolder( void )
{
	// Clear list
	m_pFolderTreeView->removeAll();

	setVisible(false);
}

//=============================================
// @brief Creates an instance of this class
//
// @return Created instance
//=============================================
CFolderViewer* CFolderViewer::CreateInstance( mxWindow* pParent )
{
	if(!g_pInstance)
		g_pInstance = new CFolderViewer(pParent);

	return g_pInstance;
}

//=============================================
// @brief Returns the current instance of this class
//
// @return Current instance, or nullptr
//=============================================
CFolderViewer* CFolderViewer::GetInstance( void )
{
	return g_pInstance;
}

//=============================================
// @brief Deletes the current instance of this class
//
//=============================================
void CFolderViewer::DeleteInstance( void )
{
	if(!g_pInstance)
		return;

	// TODO
	//delete g_pInstance;
	g_pInstance = nullptr;
}