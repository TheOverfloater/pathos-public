/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef SAYTEXT_H
#define SAYTEXT_H

struct font_set_t;

/*
====================
CGameTextWindow

====================
*/
class CSayText
{
private:
	// Maximum text lifetime
	static const Float SAYTEXT_LIFETIME;
	// Fade duration for each line
	static const Float SAYTEXT_FADETIME;
	// Screen text text schema name
	static const Char TEXTSCHEME_FILENAME[];

private:
	struct saytextinfo_t
	{
		Float die;
		CString playername;
		CString message;
	};

public:
	CSayText( void );
	~CSayText();

public:
	// Initializes the class
	bool Init( void );
	// Resets the class
	void Shutdown( void );

	// Initializes GL objects
	bool InitGL( void );
	// Resets GL objects
	void ClearGL( void );

	// Initializes game objects
	bool InitGame( void );
	// Resets game objects
	void ClearGame( void );

	// Draws the text
	bool DrawSayText( void );

public:
	// Takes input control
	void BeginInputMode( bool teamonly );
	// Tells if input is overridden
	bool IsInInputMode( void ) const;
	// Manages key inputs
	void KeyEvent( Int32 button, Int16 mod, bool keyDown );

public:
	// Adds a text to the list
	void AddText( const Char* pstrPlayerName, const Char* pstrText );

public:
	// Linked list of texts to render
	CLinkedList<saytextinfo_t> m_sayTextList;
	// Font set used
	const font_set_t* m_pFont;
	// TRUE if in input mode
	bool m_isInInputMode;
	// TRUE if team only
	bool m_isTeamOnly;

	// Text being written
	CString m_sayText;
};
extern CSayText gSayText;
#endif //SAYTEXT_H