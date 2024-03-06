//
//                 Half-Life Model Viewer (c) 1999 by Mete Ciragan
//
// file:           folderviewer.h
// last modified:  Apr 28 1999, Mete Ciragan
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
#ifndef FOLDERVIEWER_H
#define FOLDERVIEWER_H

#include "includes.h"

#ifndef INCLUDED_MXWINDOW
#include <mx/mxWindow.h>
#endif

#define IDC_FOLDERVIEWER		1001

class mxTreeView;
class mxButton;
class mxPopupMenu;
class GlWindow;

/*
=================================
CFolderViewer

=================================
*/
class CFolderViewer : public mxWindow
{
public:
	// CREATORS
	CFolderViewer( mxWindow *pWindow );
	~CFolderViewer();

public:
	virtual Int32 handleEvent( mxEvent *pEvent ) override;
	Int32 OnFolderViewer( void );
	Int32 OnLoadModel( void );

	bool OpenFolder( const Char* pstrFolderPath );
	void CloseFolder( void );

public:
	// Creates an instance of this class
	static CFolderViewer* CreateInstance( mxWindow* pParent );
	// Returns the current instance of this class
	static CFolderViewer* GetInstance( void );
	// Deletes the current instance of this class
	static void DeleteInstance( void );

public:
	// Folder path
	CString m_folderPath;
	// Current model file path
	CString m_currentModelFilePath;
	// Folder tree view object
	mxTreeView* m_pFolderTreeView;
	// Popup menu
	mxPopupMenu* m_pPopupMenu;

private:
	// Current instance of this class
	static CFolderViewer* g_pInstance;
};
#endif // FOLDERVIEWER_H