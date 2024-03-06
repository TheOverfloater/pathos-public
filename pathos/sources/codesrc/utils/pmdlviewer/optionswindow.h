//
//                 Half-Life Model Viewer (c) 1999 by Mete Ciragan
//
// file:           OptionsWindow.h
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

#ifndef OPTIONSWINDOW_H
#define OPTIONSWINDOW_H

#include <mx/mxWindow.h>

class mxChoice;
class mxRadioButton;
class mxLineEdit;
class mxButton;

/*
=================================
OptionsWindow

=================================
*/
class COptionsWindow : public mxWindow
{
public:
	enum
	{
		IDOW_COMPILER_BUTTON = 1001,
		IDOW_DECOMPILER_BUTTON,
		IDOW_COMPILER_PATH,
		IDOW_COMPILER_ARGS,
		IDOW_DECOMPILER_PATH,
		IDOW_OK,
		IDOW_CANCEL,
		IDOW_MOD_FOLDER_BUTTON,
		IDOW_MOD_FOLDER_PATH,
	};

public:
	// Compiler path setting name
	static const Char OW_COMPILER_PATH[];
	// Compiler args setting name
	static const Char OW_COMPILER_ARGS[];
	// Decompiler path setting name
	static const Char OW_DECOMPILER_PATH[];
	// Mod folder path setting name
	static const Char OW_MOD_FOLDER_PATH[];
	// Compiler window title
	static const Char WINDOW_TITLE[];

	// Compiler window x origin
	static const Uint32 WINDOW_X_ORIGIN;
	// Compiler window y origin
	static const Uint32 WINDOW_Y_ORIGIN;
	// Compiler window width
	static const Uint32 WINDOW_WIDTH;
	// Compiler window height
	static const Uint32 WINDOW_HEIGHT;

private:
	COptionsWindow( void );
	virtual ~COptionsWindow( void );

public:
	// Handles an mx event
	virtual Int32 handleEvent( mxEvent *pEvent ) override;

public:
	// Creates an instance of this class
	static COptionsWindow* CreateInstance( void );
	// Returns the current instance of this class
	static COptionsWindow* GetInstance( void );
	// Deletes the current instance of this class
	static void DeleteInstance( void );

private:
	mxButton* m_pButtonChooseCompiler;
	mxLineEdit* m_pLineEditCompilerPath;
	mxLineEdit* m_pLineEditCompilerArgs;

	mxButton* m_pButtonChooseDecompiler;
	mxLineEdit* m_pLineEditDecompilerPath;

	mxButton* m_pButtonSetModFolderPath;
	mxLineEdit* m_pLineEditModFolderPath;

private:
	// Current instance of this class
	static COptionsWindow* g_pInstance;
};
#endif // OPTIONSWINDOW_H