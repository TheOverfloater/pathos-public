//
//                 Half-Life Model Viewer (c) 1999 by Mete Ciragan
//
// file:           FileAssociation.h
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

#ifndef COMPILERWINDOW_H
#define COMPILERWINDOW_H

#ifndef INCLUDED_MXWINDOW
#include <mx/mxWindow.h>
#endif

#include "includes.h"

class mxChoice;
class mxRadioButton;
class mxLineEdit;
class mxButton;

/*
=================================
CCompilerWindow

=================================
*/
class CCompilerWindow : public mxWindow
{
private:
	// Last QC Path setting name
	static const Char LAST_QC_PATH_HEADER[];
	// Last Copy Path setting name
	static const Char LAST_COPY_PATH_HEADER[];
	// Compiler window title
	static const Char WINDOW_TITLE[];
	// Default program for editing
	static const Char DEFAULT_EDITOR[];
	// Max history of qc files
	static const Uint32 MAX_COMPILER_HISTORY;

	// Compiler window x origin
	static const Uint32 WINDOW_X_ORIGIN;
	// Compiler window y origin
	static const Uint32 WINDOW_Y_ORIGIN;
	// Compiler window width
	static const Uint32 WINDOW_WIDTH;
	// Compiler window height
	static const Uint32 WINDOW_HEIGHT;

private:
	enum
	{
		IDCW_COMPILE_BUTTON = 1001,
		IDCW_EXIT_BUTTON,
		IDCW_FILE_PATH,
		IDCW_FILE_BUTTON,
		IDCW_COPY_PATH,
		IDCW_COPY_PATH_BUTTON,
		IDCW_COPY_BUTTON,
		IDCW_EDIT_BUTTON,
	};

private:
	CCompilerWindow( void );
	virtual ~CCompilerWindow( void );

public:
	// Initializes QC history info
	void InitQcHistory( void );
	// Initializes copy path history info
	void InitCopyPathHistory( void );

	// Copies output files to the destination directory
	void CopyFiles( void );
	// Copies a single file to the destination directory
	bool CopyFile( const Char* pstrBaseName, const Char* pstrExtension );
	// Compiles the selected QC file
	void CompileQC( void );
	// Opens the editor for the selected QC file
	void EditQC( void );

	// Gets history info
	void GetHistory( void );
	// Saves history info
	void SaveHistory( void );

public:
	// Handles an mx event
	virtual Int32 handleEvent( mxEvent *pEvent ) override;

public:
	// Creates an instance of this class
	static CCompilerWindow* CreateInstance( void );
	// Returns the current instance of this class
	static CCompilerWindow* GetInstance( void );
	// Deletes the current instance of this class
	static void DeleteInstance( void );

private:
	mxChoice* m_pChoiceQcPaths;
	mxButton* m_pButtonQcPathButton;
	mxButton* m_pButtonQcCompileButton;
	mxButton* m_pButtonQcEditButton;

	mxChoice* m_pChoiceCopyPaths;
	mxButton* m_pButtonCopyPathButton;
	mxButton* m_pButtonCopyButton;

private:
	CArray<CString> m_qcFileHistoryArray;
	CArray<CString> m_copyPathHistoryArray;

private:
	// Current instance of this class
	static CCompilerWindow* g_pInstance;
};
#endif // COMPILERWINDOW_H