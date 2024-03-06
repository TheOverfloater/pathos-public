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
#ifndef FILEASSOCIATION_H
#define FILEASSOCIATION_H

#include <mx/mxWindow.h>

#include "includes.h"

class mxChoice;
class mxRadioButton;
class mxLineEdit;
class mxButton;

/*
=================================
CFileAssociation

=================================
*/
class CFileAssociation : public mxWindow
{
public:
	// Window title
	static const Char WINDOW_TITLE[];
	// Compiler window x origin
	static const Uint32 WINDOW_X_ORIGIN;
	// Compiler window y origin
	static const Uint32 WINDOW_Y_ORIGIN;
	// Compiler window width
	static const Uint32 WINDOW_WIDTH;
	// Compiler window height
	static const Uint32 WINDOW_HEIGHT;
	// Setting prefix
	static const Char SETTING_PREFIX[];

public:
	enum
	{
		ACTION_BTN_NULL = -1,
		ACTION_BTN_PROGRAM = 0,
		ACTION_BTN_ASSOCIATED_PROGRAM,
		ACTION_BTN_DEFAULT,
		ACTION_BTN_NONE,

		NB_ACTION_BTNS
	};

	enum
	{
		IDC_EXTENSION = 1001,
		IDC_ADD,
		IDC_REMOVE,
		IDC_ACTION1,
		IDC_ACTION2,
		IDC_ACTION3,
		IDC_ACTION4,
		IDC_PROGRAM,
		IDC_CHOOSEPROGRAM,
		IDC_OK,
		IDC_CANCEL,
	};

public:
	struct association_t
	{
		association_t():
			association(0)
		{
		}

		CString extension;
		CString program;
		Int32 association;
	};

private:
	CFileAssociation( void );
	virtual ~CFileAssociation( void );

public:
	// Initializes assocations list
	void InitAssociations( void );
	// Saves associations to config
	void SaveAssociations( void );
	// Sets an association
	void SetAssociation( Int32 index );

	// Returns mode for an extension
	Int32 GetMode( const Char *pstrExtension );
	// Returns program for the extension
	const Char *GetProgram( const Char *pstrExtension );

public:
	// Handles an mx event
	virtual Int32 handleEvent( mxEvent *pEvent ) override;

public:
	// Creates an instance of this class
	static CFileAssociation* CreateInstance( void );
	// Returns the current instance of this class
	static CFileAssociation* GetInstance( void );
	// Deletes the current instance of this class
	static void DeleteInstance( void );

private:
	mxChoice *m_pChoiceExtension;
	mxRadioButton *m_pRadioButtonAction[NB_ACTION_BTNS];
	mxLineEdit *m_pLineEditProgram;
	mxButton *m_pButtonChooseProgram;

	CArray<association_t> m_associationsArray;

private:
	// Current instance of this class
	static CFileAssociation* g_pInstance;
};
#endif // FILEASSOCIATION_H