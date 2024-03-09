/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef SCREENTEXT_H
#define SCREENTEXT_H

/*
====================
CScreenText

====================
*/
class CScreenText
{
public:
	// Screen text color
	static const color32_t SCREENTEXT_COLOR;
	// Screen text text schema name
	static const Char SCREENTEXT_TEXTSCHEME_FILENAME[];

public:
	struct textmsg_t
	{
		textmsg_t():
			xcoord(0),
			ycoord(0),
			life(0)
			{}

		Int32 xcoord;
		Int32 ycoord;

		CString text;

		Double life;
	};

public:
	CScreenText( void );
	~CScreenText( void );

public:
	bool Init( void );
	void Shutdown( void );

	bool InitGL( void );
	void ClearGL( void );

	bool InitGame( void );
	void ClearGame( void );

	bool Draw( void );

public:
	void AddText( const Char* pstrText, Int32 xcoord, Int32 ycoord, Float lifetime );

private:
	// List of texts
	CLinkedList<textmsg_t> m_messagesList;
	// Font set used
	const font_set_t* m_pFontSet;
};
extern CScreenText gScreenText;
#endif //SCREENTEXT_H