/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef MESSAGES_H
#define MESSAGES_H

struct font_set_t;

/*
=================================
-Class: CScreenMessages
-Description:

=================================
*/
class CScreenMessages
{
public:
	// Filename of titles file
	static const Char TITLES_FILENAME[];
	// Font set schema name for this clas
	static const Char MESSAGES_FONT_SCHEMA_FILENAME[];

public:
	enum effects_t
	{
		EFFECT_UNDEFINED = -1,
		EFFECT_FADEINOUT = 0,
		EFFECT_FLICKERY,
		EFFECT_WRITEOUT
	};

	struct msgline_t
	{
		msgline_t():
			width(0),
			height(0),
			charoffset(0),
			yoffset(0)
			{
			}

		CString text;

		Uint32 width;
		Uint32 height;
		Uint32 charoffset;
		Int32 yoffset;
	};

	struct scrmessage_t
	{
		scrmessage_t():
			xposition(-1),
			yposition(-1),
			effect(EFFECT_UNDEFINED),
			fadein(0),
			fadeout(0),
			holdtime(0),
			fxtime(0),
			width(0),
			height(0),
			lifetime(0),
			pfontset(nullptr)
			{
			}

		CString name;

		Float xposition;
		Float yposition;

		effects_t effect;
		color24_t color1;
		color24_t color2;

		Float fadein;
		Float fadeout;
		Float holdtime;
		Float fxtime;

		// Values set by code
		Uint32 width;
		Uint32 height;
		Float lifetime;

		// Text broken down into lines
		CArray<msgline_t> lines;

		// Font set used
		const font_set_t* pfontset;
		// Name of text schema
		CString textschemaname;
	};

	struct displaymsg_t
	{
		displaymsg_t():
			time(0),
			die(0),
			pmsg(nullptr)
			{}

		// Time this message was called to display
		Float time;
		// Total lifetime of the message
		Float die;

		// Pointer to message contents
		scrmessage_t* pmsg;
	};

public:
	CScreenMessages( void );
	~CScreenMessages( void );

public:
	// Initializes the class
	bool Init( void );
	// Shuts down the class
	void Shutdown( void );

	// Initializes game objects
	bool InitGame( void );
	// Clears game objects
	void ClearGame( void );

	// Initializes OpenGL data
	bool InitGL( void );
	// Clear OGL data
	void ClearGL( void );

public:
	// Shows a message on-screen
	void ShowMessage( const Char* pstrMessageName );
	// Shows a custom message on-screen
	void ShowMessage( const Char* pstrMessageText, Float fadein, Float fadeout, Float fxtime, Float holdtime, effects_t effect, Int32 channel, Float xposition, Float yposition, const color24_t& color1, const color24_t& color2 );

	// Draws messages
	bool DrawMessages( void );

private:
	// Reads the title file
	void ReadTitlesFile( void );
	// Re-adjusts message definitions after GL reload
	void ReadjustMessageSizes( void );
	// Re-adjusts message after GL reload
	void ReadjustMessage( scrmessage_t* pmsg );

	// Draws a single message
	bool DrawMessage( displaymsg_t& msg );
	// Calculates message x and y position
	void GetTextXPosition( Double time, const font_set_t* pset, displaymsg_t& msg, msgline_t& line, Int32& xcoord );

	// Calculates message size
	void CalculateMessageSizes( scrmessage_t& msg );
	// Calculates the lifetime of a message
	static void CalculateMessageLifetime( scrmessage_t& msg );
	// Processes message text
	static void ProcessMessageText( const font_set_t* pset, scrmessage_t& msg, const Char* pstrMessageText );

private:
	// Array of messages
	CArray<scrmessage_t*> m_messageDefinitonsArray;
	// Linked list of messages being renderer
	CLinkedList<displaymsg_t> m_drawnMessagesList;

	// For custom messages
	scrmessage_t m_customMessage;

	// Screen width
	Uint32 m_screenWidth;
	// Screen height
	Uint32 m_screenHeight;

	// Font set used by engine
	const font_set_t* m_pFontSet;
};
extern CScreenMessages gMessages;
#endif //MESSAGES_H