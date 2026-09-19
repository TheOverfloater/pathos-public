/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/
// Code by valina354

#ifndef CREDITS_H
#define CREDITS_H

struct font_set_t;

/*
====================
CCredits

====================
*/
class CCredits
{
public:
	// Default path to credits file
	static const Char CREDITS_FILE_PATH[];
	// Credits font schema name
	static const Char CREDITS_TEXTSCHEMA_NAME[];
	// Default color for text
	static const color32_t CREDITS_TEXT_COLOR;

public:
	struct creditline_t
	{
		creditline_t():
			width(0),
			height(0)
		{
		}

		CString text;
		Uint32 width;
		Uint32 height;
	};

public:
	CCredits( void );
	~CCredits( void );

public:
	bool Init( void );
	void Shutdown( void );

	bool InitGL( void );
	void ClearGL( void );

	bool InitGame( void );
	void ClearGame( void );

	void Think( void );
	bool Draw( void );

public:
	void StartCredits( Float scrollSpeed );
	bool LoadCreditsFile( void );
	void RecomputeDimensions( void );

private:
	bool m_isActive;
	Float m_scrollSpeed;
	Double m_currentY;
	Double m_lastTime;
	Double m_totalHeight;

	Uint32 m_screenWidth;
	Uint32 m_screenHeight;

	CArray<creditline_t> m_creditsLines;
	const font_set_t* m_pFontSet;
};

extern CCredits gCredits;
#endif //CREDITS_H