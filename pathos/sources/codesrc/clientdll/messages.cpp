/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "messages.h"
#include "clientdll.h"
#include "fontset.h"
#include "matrix.h"

// Class definition
CScreenMessages gMessages;

// Spacing of messages when bordering the screen
static const Int32 MSG_BORDER_INSET = 10;

// Filename of titles file
const Char CScreenMessages::TITLES_FILENAME[] = "titles.txt";
// Font set schema name for this clas
const Char CScreenMessages::MESSAGES_FONT_SCHEMA_FILENAME[] = "titles";

//====================================
//
//====================================
CScreenMessages::CScreenMessages( void ):
	m_screenWidth(0),
	m_screenHeight(0),
	m_pFontSet(nullptr)
{
}

//====================================
//
//====================================
CScreenMessages::~CScreenMessages( void )
{
	Shutdown();
}

//====================================
//
//====================================
bool CScreenMessages::Init( void )
{
	// Nothing for now
	return true;
}

//====================================
//
//====================================
void CScreenMessages::Shutdown( void )
{
	ClearGame();
}

//====================================
//
//====================================
bool CScreenMessages::InitGame( void )
{
	Uint32 screenWidth, screenHeight;
	cl_renderfuncs.pfnGetScreenSize(screenWidth, screenHeight);

	// Read titles file
	ReadTitlesFile();

	// Get screen width/height
	cl_renderfuncs.pfnGetScreenSize(m_screenWidth, m_screenHeight);

	return true;
}

//====================================
//
//====================================
void CScreenMessages::ClearGame( void )
{
	if(!m_drawnMessagesList.empty())
		m_drawnMessagesList.clear();

	if(!m_messageDefinitonsArray.empty())
	{
		for(Uint32 i = 0; i < m_messageDefinitonsArray.size(); i++)
			delete m_messageDefinitonsArray[i];

		m_messageDefinitonsArray.clear();
	}
}

//====================================
//
//====================================
bool CScreenMessages::InitGL( void )
{
	// Get screen width/height
	cl_renderfuncs.pfnGetScreenSize(m_screenWidth, m_screenHeight);

	m_pFontSet = cl_engfuncs.pfnGetResolutionSchemaFontSet(MESSAGES_FONT_SCHEMA_FILENAME, m_screenHeight);
	if(!m_pFontSet)
		m_pFontSet = cl_renderfuncs.pfnGetDefaultFontSet();

	// Re-adjust message definitions
	ReadjustMessageSizes();

	return true;
}

//====================================
//
//====================================
void CScreenMessages::ClearGL( void )
{
}

//====================================
//
//====================================
void CScreenMessages::ReadjustMessageSizes( void )
{
	if(m_messageDefinitonsArray.empty())
		return;

	for(Uint32 i = 0; i < m_messageDefinitonsArray.size(); i++)
	{
		scrmessage_t* pmsg = m_messageDefinitonsArray[i];
		ReadjustMessage(pmsg);
	}

	if(!m_customMessage.lines.empty())
		ReadjustMessage(&m_customMessage);
}

//====================================
//
//====================================
void CScreenMessages::ReadjustMessage( scrmessage_t* pmsg )
{
	// Reset this
	pmsg->pfontset = nullptr;

	if(!pmsg->textschemaname.empty())
		pmsg->pfontset = cl_engfuncs.pfnGetResolutionSchemaFontSet(pmsg->textschemaname.c_str(), m_screenHeight);

	if(!pmsg->pfontset)
		pmsg->pfontset = m_pFontSet;

	pmsg->width = 0;
	pmsg->height = 0;
			
	for(Uint32 j = 0; j < pmsg->lines.size(); j++)
	{
		msgline_t* pline = &pmsg->lines[j];

		pline->width = 0;
		pline->height = pmsg->pfontset->maxheight;
		pline->yoffset = pmsg->height;

		const Char* pstr = pline->text.c_str();
		while(*pstr)
		{
			Uint32 charidx = clamp(*pstr, 0, 255);
			pstr++;

			pline->width += pmsg->pfontset->glyphs[charidx].advancex;
		}

		if(pline->width > pmsg->width)
			pmsg->width = pline->width;

		pmsg->height += pline->height;
	}
}

//====================================
//
//====================================
void CScreenMessages::ReadTitlesFile( void )
{
	const Char* pfile = reinterpret_cast<const Char*>(cl_filefuncs.pfnLoadFile(TITLES_FILENAME, nullptr));
	if(!pfile)
	{
		cl_engfuncs.pfnCon_EPrintf("%s - Failed to load '%s'.\n", __FUNCTION__, TITLES_FILENAME);
		return;
	}

	Float positionx = 0;
	Float positiony = 0;
	color24_t color1;
	color24_t color2;
	Float fadein = 0;
	Float fadeout = 0;
	Float holdtime = 0;
	Float fxtime = 0;
	effects_t effect = EFFECT_UNDEFINED;
	CString schemaname;

	static Char name[MAX_PARSE_LENGTH];
	static Char line[MAX_LINE_LENGTH];

	const Char* pstr = pfile;
	while(pstr)
	{
		// Read the line
		pstr = Common::ReadLine(pstr, line);

		static Char token[MAX_PARSE_LENGTH];
		const Char* pchar = Common::Parse(line, token);
		if(!qstrlen(token))
			continue;

		// Manage special tokens
		if(!qstrcmp(token, "{"))
		{
			const Char* pstrend = qstrstr(pstr, "}");
			if(!pstrend)
			{
				cl_engfuncs.pfnCon_Printf("%s - Missing '}' bracket in '%s'.\n", __FUNCTION__, TITLES_FILENAME);
				cl_filefuncs.pfnFreeFile(pfile);
				return;
			}

			// Check for effect
			if(effect == EFFECT_UNDEFINED)
			{
				cl_engfuncs.pfnCon_Printf("%s - Message definition '%s' has no effect set.\n", __FUNCTION__, name);
				effect = EFFECT_FADEINOUT;
			}

			// Check holdtime
			if(holdtime <= 0)
			{
				cl_engfuncs.pfnCon_Printf("%s - Message definition '%s' has holdtime set.\n", __FUNCTION__, name);
				holdtime = 1.0;
			}

			// Make sure name is set
			if(!qstrlen(name))
			{
				cl_engfuncs.pfnCon_Printf("%s - Message with no name.\n", __FUNCTION__);
				pstr = pstrend+1;
				continue;
			}

			// Create the new entry
			scrmessage_t* pnew = new scrmessage_t;
			pnew->color1 = color1;
			pnew->color2 = color2;
			pnew->name = name;
			pnew->effect = effect;
			pnew->fadein = fadein;
			pnew->fadeout = fadeout;
			pnew->holdtime = holdtime;
			pnew->xposition = positionx;
			pnew->yposition = positiony;
			pnew->fxtime = fxtime;
			
			if(!schemaname.empty())
			{
				pnew->textschemaname = schemaname;
				pnew->pfontset = cl_engfuncs.pfnGetResolutionSchemaFontSet(pnew->textschemaname.c_str(), m_screenHeight);
				schemaname.clear();
			}

			if(!pnew->pfontset)
				pnew->pfontset = m_pFontSet;

			if(pnew->xposition != -1)
				pnew->xposition = clamp(pnew->xposition, 0.0, 1.0);
			if(pnew->xposition != -1)
				pnew->yposition = clamp(pnew->xposition, 0.0, 1.0);

			// Assing the message contents
			CString msgtext;
			Uint32 num = (pstrend - pstr);
			msgtext.assign(pstr, num);

			// Process the msg text
			ProcessMessageText(pnew->pfontset ? pnew->pfontset : m_pFontSet, *pnew, msgtext.c_str());

			// Add to the list
			m_messageDefinitonsArray.push_back(pnew);

			// Clear to catch errors
			name[0] = '\0';

			// Begin from start
			pstr = pstrend+1;
			continue;
		}
		else if(token[0] == '$')
		{
			// Check for errors
			if(!pchar)
			{
				cl_engfuncs.pfnCon_Printf("%s - Incomplete option '%s'.\n", __FUNCTION__, token);
				continue;
			}

			// Read in the first value
			static Char value[MAX_PARSE_LENGTH];
			pchar = Common::Parse(pchar, value);
			if(!qstrlen(value))
			{
				cl_engfuncs.pfnCon_Printf("%s - Incomplete option '%s'.\n", __FUNCTION__, token);
				continue;
			}

			if(!qstrcmp(token, "$position"))
			{
				if(!pchar)
				{
					cl_engfuncs.pfnCon_Printf("%s - Incomplete option '%s'.\n", __FUNCTION__, token);
					continue;
				}

				positionx = SDL_atof(value);

				// Read the Y coordinate
				pchar = Common::Parse(pchar, value);
				if(!qstrlen(value))
				{
					cl_engfuncs.pfnCon_Printf("%s - Incomplete option '%s'.\n", __FUNCTION__, token);
					continue;
				}

				positiony = SDL_atof(value);
			}
			else if(!qstrcmp(token, "$effect"))
			{
				// Get type
				Int32 type = SDL_atoi(value);
				switch(type)
				{
				case 0:
					effect = EFFECT_FADEINOUT;
					break;
				case 1:
					effect = EFFECT_FLICKERY;
					break;
				case 2:
					effect = EFFECT_WRITEOUT;
					break;
				default:
					cl_engfuncs.pfnCon_Printf("%s - Unknown value '%d' set for option '$s'.\n", __FUNCTION__, type, token);
					break;
				}
			}
			else if(!qstrcmp(token, "$color1") || !qstrcmp(token, "$color2"))
			{
				if(!pchar)
				{
					cl_engfuncs.pfnCon_Printf("%s - Incomplete option '%s'.\n", __FUNCTION__, token);
					continue;
				}

				// Set destination
				color24_t& dest = (!qstrcmp(token, "$color1")) ? color1 : color2;

				// Set R component
				dest.r = clamp(SDL_atoi(value), 0, 255);

				// Parse the G component
				pchar = Common::Parse(pchar, value);
				if(!pchar || !qstrlen(value))
				{
					cl_engfuncs.pfnCon_Printf("%s - Incomplete option '%s'.\n", __FUNCTION__, token);
					continue;
				}

				// Set G component
				dest.g = clamp(SDL_atoi(value), 0, 255);

				// Parse the B component
				pchar = Common::Parse(pchar, value);
				if(!qstrlen(value))
				{
					cl_engfuncs.pfnCon_Printf("%s - Incomplete option '%s'.\n", __FUNCTION__, token);
					continue;
				}

				// Set G component
				dest.b = clamp(SDL_atoi(value), 0, 255);
			}
			else if(!qstrcmp(token, "$fadein"))
			{
				// Set fade in
				fadein = SDL_atof(value);
			}
			else if(!qstrcmp(token, "$fadeout"))
			{
				// Set fade out
				fadeout = SDL_atof(value);
			}
			else if(!qstrcmp(token, "$holdtime"))
			{
				// Set hold time
				holdtime = SDL_atof(value);
			}
			else if(!qstrcmp(token, "$fxtime"))
			{
				// Set hold time
				fxtime = SDL_atof(value);
			}
			else if(!qstrcmp(token, "$schemaname"))
			{
				// Set the font set name
				schemaname = value;
			}
			else
			{
				cl_engfuncs.pfnCon_Printf("%s - Unknown option '%s'.\n", __FUNCTION__, token);
				continue;
			}
		}
		else
		{
			// Assume it's the message name
			qstrcpy(name, token);
		}
	}

	// Release the file
	cl_filefuncs.pfnFreeFile(pfile);

	// Print relevant info
	cl_engfuncs.pfnCon_Printf("%d message definitions read from '%s'.\n", m_messageDefinitonsArray.size(), TITLES_FILENAME);
}

//====================================
//
//====================================
void CScreenMessages::CalculateMessageLifetime( scrmessage_t& msg )
{
	// calculate lifetime
	switch(msg.effect)
	{
	case EFFECT_WRITEOUT:
		{
			// Calculate number of visible characters
			Uint32 numvisiblechars = 0;
			for(Uint32 i = 0; i < msg.lines.size(); i++)
			{
				const Char* pstr = msg.lines[i].text.c_str();
				while(*pstr)
				{
					if(!SDL_isspace(*pstr))
						numvisiblechars++;

					pstr++;
				}
			}

			// Lifetime is num chars*fadein plus holdtime and fadeout
			msg.lifetime = msg.holdtime + msg.fadeout;
		}
		break;
	case EFFECT_FADEINOUT:
	case EFFECT_FLICKERY:
	default:
		// Much simpler case, just combine fade in, holdtime and fadeout
		msg.lifetime = msg.fadein + msg.holdtime + msg.fadeout;
		break;
	}
}
//====================================
//
//====================================
void CScreenMessages::ProcessMessageText( const font_set_t* pset, scrmessage_t& msg, const Char* pstrMessageText )
{
	// Just to be safe
	if(!msg.lines.empty())
		msg.lines.clear();

	if(!qstrlen(pstrMessageText))
		return;

	// Count the newlines
	Uint32 nblines = 0;
	const Char* pstr = pstrMessageText;
	while(*pstr)
	{
		if(*pstr == '\n')
			nblines++;

		pstr++;
	}

	// Avoid zero size
	if(!nblines)
		nblines = 1;

	msg.width = 0;
	msg.height = 0;

	msg.lines.resize(nblines);
	nblines = 0;

	// Craft the line entries
	Uint32 nbchars = 0;
	static Char line[MAX_LINE_LENGTH];
	pstr = pstrMessageText;
	while(pstr)
	{
		// Read the line
		pstr = Common::ReadLine(pstr, line);

		if(nblines == msg.lines.size())
		{
			cl_engfuncs.pfnCon_Printf("%s - Mismatch in number of expected lines.\n", __FUNCTION__);
			break;
		}

		msgline_t& newline = msg.lines[nblines];
		nblines++;

		newline.text = line;
		newline.width = 0;
		newline.height = pset->maxheight;
		newline.charoffset = nbchars;
		newline.yoffset = msg.height;

		const Char* ppstr = line;
		while(*ppstr)
		{
			if(!SDL_isspace(*ppstr))
				nbchars++;

			Uint32 charidx = clamp(*ppstr, 0, 255);
			ppstr++;

			newline.width += pset->glyphs[charidx].advancex;
		}

		if(newline.width > msg.width)
			msg.width = newline.width;

		msg.height += newline.height;
	}

	// Make sure we don't screw up
	assert(nblines == msg.lines.size());

	// Calculate total lifetime
	CalculateMessageLifetime(msg);
}

//====================================
//
//====================================
void CScreenMessages::ShowMessage( const Char* pstrMessageName )
{
	// Make sure this is the only custom one drawn
	m_drawnMessagesList.begin();
	while(!m_drawnMessagesList.end())
	{
		displaymsg_t& msg = m_drawnMessagesList.get();
		if(!qstrcmp(msg.pmsg->name, pstrMessageName))
		{
			m_drawnMessagesList.remove(m_drawnMessagesList.get_link());
			break;
		}

		m_drawnMessagesList.next();
	}

	// Locate the message definition
	scrmessage_t* pmsg = nullptr;
	for(Uint32 i = 0; i < m_messageDefinitonsArray.size(); i++)
	{
		if(!qstrcmp(m_messageDefinitonsArray[i]->name, pstrMessageName))
		{
			pmsg = m_messageDefinitonsArray[i];
			break;
		}
	}

	if(!pmsg)
	{
		cl_engfuncs.pfnCon_Printf("%s - Could not find message definition '%s' in '%s'.\n", __FUNCTION__, pstrMessageName, TITLES_FILENAME);
		return;
	}

	// Add it to the drawn list
	displaymsg_t newmsg;
	newmsg.pmsg = pmsg;
	newmsg.time = -1;

	m_drawnMessagesList.add(newmsg);
}

//====================================
//
//====================================
void CScreenMessages::ShowMessage( const Char* pstrMessageText, Float fadein, Float fadeout, Float fxtime, Float holdtime, effects_t effect, Int32 channel, Float xposition, Float yposition, const color24_t& color1, const color24_t& color2 )
{
	// Make sure this is the only custom one drawn
	m_drawnMessagesList.begin();
	while(!m_drawnMessagesList.end())
	{
		const displaymsg_t& msg = m_drawnMessagesList.get();
		if(msg.pmsg->name.empty())
		{
			m_drawnMessagesList.remove(m_drawnMessagesList.get_link());
			break;
		}

		m_drawnMessagesList.next();
	}

	// Set contents of custom message
	m_customMessage.fadein = fadein;
	m_customMessage.fadeout = fadeout;
	m_customMessage.fxtime = fxtime;
	m_customMessage.holdtime = holdtime;
	m_customMessage.effect = effect;
	m_customMessage.xposition = xposition;
	m_customMessage.yposition = yposition;
	m_customMessage.color1 = color1;
	m_customMessage.color2 = color2;
	m_customMessage.name.clear();

	if(m_customMessage.xposition != -1)
		m_customMessage.xposition = clamp(m_customMessage.xposition, 0.0, 1.0);
	if(m_customMessage.xposition != -1)
		m_customMessage.yposition = clamp(m_customMessage.xposition, 0.0, 1.0);

	// Process the text
	ProcessMessageText(m_pFontSet, m_customMessage, pstrMessageText);

	// Add it to the drawn list
	displaymsg_t newmsg;
	newmsg.pmsg = &m_customMessage;
	newmsg.time = -1;

	m_drawnMessagesList.add(newmsg);
}

//====================================
//
//====================================
bool CScreenMessages::DrawMessages( void )
{
	if(m_drawnMessagesList.empty())
		return true;

	// Get current time
	Double time = cl_engfuncs.pfnGetClientTime();

	// Draw each message
	m_drawnMessagesList.begin();
	while(!m_drawnMessagesList.end())
	{
		displaymsg_t& msg = m_drawnMessagesList.get();
		if(msg.time == -1)
		{
			// Set initial time
			msg.time = time;
			msg.die = msg.time + msg.pmsg->lifetime;
		}

		if(msg.die <= time || !msg.pmsg)
		{
			m_drawnMessagesList.remove(m_drawnMessagesList.get_link());
			m_drawnMessagesList.next();
			continue;
		}

		// Draw it
		if(!DrawMessage(msg))
			return false;

		// Go to next
		m_drawnMessagesList.next();
	}

	return true;
}

//====================================
//
//====================================
void CScreenMessages::GetTextXPosition( Double time, const font_set_t* pset, displaymsg_t& msg, msgline_t& line, Int32& xcoord )
{
	// Non-centered position is simpler
	if(msg.pmsg->xposition != -1)
	{
		Int32 xpos;
		if(msg.pmsg->xposition < 0)
			xpos = m_screenWidth * (1.0 + msg.pmsg->xposition);
		else
			xpos = m_screenWidth * msg.pmsg->xposition;

		if(xpos + msg.pmsg->width > (Int32)m_screenWidth)
			xpos -= ((xpos + msg.pmsg->width) - (Int32)m_screenWidth) - MSG_BORDER_INSET;

		xcoord = xpos;
		return;
	}

	// Determine the width of the line
	Uint32 linewidth = 0;
	if(msg.pmsg->effect == EFFECT_WRITEOUT)
	{
		Uint32 charnum = 0;
		const Char* pstr = line.text.c_str();
		while(*pstr)
		{
			if(!SDL_isspace(*pstr))
			{
				Float chartime = msg.time + (line.charoffset + charnum)*msg.pmsg->fadein;
				if(chartime > time)
					break;

				charnum++;
			}

			Uint32 charidx = clamp(*pstr, 0, 255);
			linewidth += pset->glyphs[charidx].advancex;
			pstr++;
		}
	}
	else
	{
		// This is simple, just get the line width
		linewidth = line.width;
	}

	Int32 xposition = ((Int32)m_screenWidth)*0.5 - ((Int32)linewidth)*0.5;
	xcoord = xposition;
}

//====================================
//
//====================================
bool CScreenMessages::DrawMessage( displaymsg_t& msg )
{
	Double time = cl_engfuncs.pfnGetClientTime();

	// Get ptr to definition
	scrmessage_t* pmsgdef = msg.pmsg;

	// Calculate position
	Int32 xposition = 0;
	Int32 yposition = 0;

	Int32 charXPos = 0;
	Int32 charYPos = 0;

	if(msg.pmsg->yposition == -1)
		yposition = ((Int32)m_screenHeight)*0.5 - msg.pmsg->height*0.5;
	else
	{
		if(msg.pmsg->yposition > 0 && msg.pmsg->effect != EFFECT_WRITEOUT)
			yposition = m_screenHeight*(1.0 - msg.pmsg->yposition);
		else
			yposition = m_screenHeight*msg.pmsg->yposition;

		if(yposition+msg.pmsg->height > (Int32)m_screenHeight)
			yposition -= ((yposition+msg.pmsg->height) - (Int32)m_screenHeight) - MSG_BORDER_INSET;
	}

	// Get default font(only supported one rn)
	const font_set_t* pset = pmsgdef->pfontset;
	if(!pset)
		pset = m_pFontSet;

	// Begin text rendering
	if(!cl_renderfuncs.pfnBeginTextRendering(pset))
	{
		cl_engfuncs.pfnErrorPopup("Shader error: %s.", cl_renderfuncs.pfnGetStringDrawError());
		return false;
	}

	// Draw line by line
	for(Uint32 i = 0; i < pmsgdef->lines.size(); i++)
	{
		msgline_t &line = pmsgdef->lines[i];

		if(pmsgdef->effect == EFFECT_WRITEOUT)
		{
			Float linetime = msg.time + line.charoffset*pmsgdef->fadein;
			if(linetime > time)
				break;
		}

		// Get X Position for text
		GetTextXPosition(time, pset, msg, line, xposition);

		Uint32 charIndex = line.charoffset;
		charXPos = xposition;
		charYPos = yposition;

		Float alpha = 1.0;
		Float colorblend = 1.0;

		if(pmsgdef->effect != EFFECT_WRITEOUT)
		{
			if((msg.time + pmsgdef->fadein) > time)
				alpha = (time - msg.time) / pmsgdef->fadein;
			else if((msg.time + pmsgdef->fadein + pmsgdef->holdtime) <= time)
				alpha = 1.0 - (time - (msg.time + pmsgdef->fadein + pmsgdef->holdtime))/pmsgdef->fadeout;

			alpha = clamp(alpha, 0.0, 1.0);

			switch(pmsgdef->effect)
			{
			case EFFECT_FADEINOUT:
				colorblend = 1.0;
				break;
			case EFFECT_FLICKERY:
				if(Common::RandomLong(0, 255) > 25)
					colorblend = 1.0;
				else
					colorblend = 0.0;
				break;
			}
		}

		// Draw character by character
		const Char* pstr = line.text.c_str();
		while(*pstr)
		{
			if(pmsgdef->effect == EFFECT_WRITEOUT)
			{
				Float chartime = msg.time + charIndex*pmsgdef->fadein;
				if(chartime > time)
					break;

				if(chartime+pmsgdef->fadein > time)
					alpha = (time - chartime) / pmsgdef->fadein;
				else if((msg.die-pmsgdef->fadeout) > time)
					alpha = 1.0;
				else
					alpha = 1.0 - ((time-(msg.die-pmsgdef->fadeout))/pmsgdef->fadeout);

				alpha = clamp(alpha, 0.0, 1.0);

				if(pmsgdef->fxtime > 0)
				{
					Float delta = time-chartime;
					if(delta > pmsgdef->fxtime)
						colorblend = 1.0;
					else
						colorblend = 1.0 - (delta*(1.0/pmsgdef->fxtime));

						clamp(colorblend, 0.0, 1.0);
					}
					else
						colorblend = 1.0;
			}

			color24_t color;
			color.r = pmsgdef->color2.r * (1.0-colorblend) + pmsgdef->color1.r * colorblend;
			color.g = pmsgdef->color2.g * (1.0-colorblend) + pmsgdef->color1.g * colorblend;
			color.b = pmsgdef->color2.b * (1.0-colorblend) + pmsgdef->color1.b * colorblend;

			// Draw the character
			if(!cl_renderfuncs.pfnDrawCharacter(pset, charXPos, charYPos, *pstr, color.r, color.g, color.b, 255*alpha))
			{
				cl_engfuncs.pfnErrorPopup("Shader error: %s.", cl_renderfuncs.pfnGetStringDrawError());
				return false;
			}

			Uint32 charidx = clamp(*pstr, 0, 255);
			charXPos += pset->glyphs[charidx].advancex;

			if(!SDL_isspace(*pstr))
				charIndex++;

			pstr++;
		}

		yposition += line.height;
	}

	// Finish rendering
	cl_renderfuncs.pfnFinishTextRendering(pset);

	// Draw rectangle if type was WRITEOUT
	if(msg.pmsg->effect == EFFECT_WRITEOUT)
	{
		// Make the cursor blink
		if(((Int64)SDL_floor(time*2)) % 2 == 1)
			return true;

		Float fadetime = msg.die - msg.pmsg->fadeout;

		Float alpha = 1.0;
		if(fadetime < time)
		{
			Float fadedelta = time - fadetime;
			alpha = 1.0 - (fadedelta/msg.pmsg->fadeout);
		}

		if(!cl_renderfuncs.pfnEnableBasicDraw())
			return false;

		if(!cl_renderfuncs.pfnBasicDrawDisableTextures())
			return false;

		glDepthMask(GL_FALSE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		CMatrix projection;
		projection.LoadIdentity();

		CMatrix modelview;
		modelview.LoadIdentity();
		modelview.Ortho(GL_ZERO, GL_ONE, GL_ONE, GL_ZERO, 0.1, 100);

		cl_renderfuncs.pfnBasicDrawSetModelView(modelview.GetMatrix());
		cl_renderfuncs.pfnBasicDrawSetProjection(projection.GetMatrix());

		Float xsize = 6.0f/(Float)m_screenWidth;
		Float ysize = 6.0f/(Float)m_screenHeight;

		Float xpos = (Float)charXPos/(Float)m_screenWidth;
		Float ypos = (Float)charYPos/(Float)m_screenHeight;
		ypos -= ysize;

		Vector color((Float)msg.pmsg->color1.r/255.0f,
			(Float)msg.pmsg->color1.g/255.0f,
			(Float)msg.pmsg->color1.b/255.0f);

		cl_renderfuncs.pfnBasicDrawBegin(GL_TRIANGLES);

		// Draw first triangle
		cl_renderfuncs.pfnBasicDrawColor4f(color.x, color.y, color.z, alpha);
		cl_renderfuncs.pfnBasicDrawVertex3f(xpos, ypos+ysize, -1.0);

		cl_renderfuncs.pfnBasicDrawColor4f(color.x, color.y, color.z, alpha);
		cl_renderfuncs.pfnBasicDrawVertex3f(xpos, ypos, -1.0);

		cl_renderfuncs.pfnBasicDrawColor4f(color.x, color.y, color.z, alpha);
		cl_renderfuncs.pfnBasicDrawVertex3f(xpos+xsize, ypos, -1.0);

		// Draw second triangle
		cl_renderfuncs.pfnBasicDrawColor4f(color.x, color.y, color.z, alpha);
		cl_renderfuncs.pfnBasicDrawVertex3f(xpos, ypos+ysize, -1.0);

		cl_renderfuncs.pfnBasicDrawColor4f(color.x, color.y, color.z, alpha);
		cl_renderfuncs.pfnBasicDrawVertex3f(xpos+xsize, ypos, -1.0);

		cl_renderfuncs.pfnBasicDrawColor4f(color.x, color.y, color.z, alpha);
		cl_renderfuncs.pfnBasicDrawVertex3f(xpos+xsize, ypos+ysize, -1.0);
		cl_renderfuncs.pfnBasicDrawEnd();

		cl_renderfuncs.pfnDisableBasicDraw();

		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
	}

	return true;
}

