/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "r_vbo.h"
#include "textschemas.h"
#include "file.h"
#include "fontset.h"
#include "r_text.h"
#include "system.h"
#include "r_main.h"
#include "window.h"

// Path to the folder containing the schemas
const Char CTextSchemas::SCHEMA_FOLDER_PATH[] = "/scripts/textschemas/";

CTextSchemas gTextSchemas;

//====================================
//
//====================================
CTextSchemas::CTextSchemas( void )
{
}

//====================================
//
//====================================
CTextSchemas::~CTextSchemas( void )
{
}

//====================================
//
//====================================
void CTextSchemas::Init( void )
{
}

//====================================
//
//====================================
void CTextSchemas::Clear( void )
{
	if(m_schemasList.empty())
		return;

	m_schemasList.clear();
}

//====================================
//
//====================================
const font_set_t* CTextSchemas::GetSchemaFontSet( const Char* schemaFileName )
{
	const textschema_t* pschema = nullptr;

	if(!m_schemasList.empty())
	{
		m_schemasList.begin();
		while(!m_schemasList.end())
		{
			textschema_t& schema = m_schemasList.get();
			if(!qstrcmp(schema.filename, schemaFileName))
			{
				pschema = &schema;
				break;
			}

			m_schemasList.next();
		}
	}

	if(!pschema)
	{
		pschema = LoadSchema(schemaFileName);
		if(!pschema)
			return nullptr;
	}

	const font_set_t* pset = gText.LoadFont(pschema->fontsetname.c_str(), pschema->fontsize, pschema->outlineradius > 0 ? true : false, &pschema->outlinecolor, pschema->outlineradius);
	if(!pset)
	{
		Con_EPrintf("%s - Unable to load font set '%s' for schema '%s'.\n", __FUNCTION__, pschema->fontsetname.c_str(), pschema->internalname.c_str());
		return nullptr;
	}

	return pset;
}

//====================================
//
//====================================
const font_set_t* CTextSchemas::GetResolutionSchemaFontSet( const Char* schemaFileName, Uint32 resolution )
{
	const textschema_t* pschema = nullptr;

	if(!m_schemasList.empty())
	{
		m_schemasList.begin();
		while(!m_schemasList.end())
		{
			textschema_t& schema = m_schemasList.get();
			if(!qstrcmp(schema.filename, schemaFileName))
			{
				pschema = &schema;
				break;
			}

			m_schemasList.next();
		}
	}

	if(!pschema)
	{
		pschema = LoadSchema(schemaFileName);
		if(!pschema)
			return nullptr;
	}

	text_reschema_t* pbestschema = nullptr;
	for(Uint32 i = 0; i < pschema->resolutions.size(); i++)
	{
		text_reschema_t* preschema = &pschema->resolutions[i];
		if(!pbestschema)
		{
			pbestschema = preschema;
			continue;
		}

		Int32 prevdiff = SDL_abs(pbestschema->screenheight - resolution);
		Int32 curdiff = SDL_abs(preschema->screenheight - resolution);
		if(prevdiff > curdiff)
			pbestschema = preschema;
	}

	const font_set_t* pset = nullptr;
	if(!pbestschema)
	{
		pset = gText.LoadFont(pschema->fontsetname.c_str(), pschema->fontsize, pschema->outlineradius > 0 ? true : false, &pschema->outlinecolor, pschema->outlineradius);
		if(!pset)
		{
			Con_EPrintf("%s - Unable to load font set '%s' for schema '%s'.\n", __FUNCTION__, pschema->fontsetname.c_str(), pschema->internalname.c_str());
			return nullptr;
		}
	}
	else
	{
		pset = gText.LoadFont(pschema->fontsetname.c_str(), pbestschema->fontsize, pbestschema->outlineradius > 0 ? true : false, &pschema->outlinecolor, pbestschema->outlineradius);
		if(!pset)
		{
			Con_EPrintf("%s - Unable to load font set '%s' for schema '%s'.\n", __FUNCTION__, pschema->fontsetname.c_str(), pschema->internalname.c_str());
			return nullptr;
		}
	}

	return pset;
}

//====================================
//
//====================================
const CTextSchemas::textschema_t* CTextSchemas::LoadSchema( const Char* schemaFileName )
{
	// Build the path and load the file
	CString path;
	path << SCHEMA_FOLDER_PATH << schemaFileName << ".txt";

	const byte* pfile = FL_LoadFile(path.c_str(), nullptr);
	if(!pfile)
		return nullptr;

	textschema_t schema;
	schema.filename = schemaFileName;

	CString token;

	// First token should be $schema
	const Char* pstr = reinterpret_cast<const Char*>(pfile);
	pstr = Common::Parse(pstr, token);
	if(qstrcmp(token, "$schema"))
	{
		Con_EPrintf("%s - Expected '$schema', got '%s' instead while reading '%s'.\n", __FUNCTION__, token.c_str(), path.c_str());
		FL_FreeFile(pfile);
		return nullptr;
	}

	if(!pstr)
	{
		Con_EPrintf("%s - Expected closing '}', got EOL instead while reading '%s'.\n", __FUNCTION__, path.c_str());
		FL_FreeFile(pfile);
		return nullptr;
	}

	// Next needs to be the schema name
	pstr = Common::Parse(pstr, token);
	if(!pstr)
	{
		Con_EPrintf("%s - Expected closing '}', got EOL instead while reading '%s'.\n", __FUNCTION__, path.c_str());
		FL_FreeFile(pfile);
		return nullptr;
	}

	schema.internalname = token;

	// Next needs to be an opening bracket
	pstr = Common::Parse(pstr, token);
	if(qstrcmp(token, "{"))
	{
		Con_EPrintf("%s - Expected '{', got '%s' instead while reading '%s'.\n", __FUNCTION__, token.c_str(), path.c_str());
		FL_FreeFile(pfile);
		return nullptr;
	}

	if(!pstr)
	{
		Con_EPrintf("%s - Expected closing '}', got EOL instead while reading '%s'.\n", __FUNCTION__, path.c_str());
		FL_FreeFile(pfile);
		return nullptr;
	}

	while(true)
	{
		// Grab the first token
		pstr = Common::Parse(pstr, token);
		if(!qstrcmp(token, "}"))
			break;

		// Make sure it's not EOL
		if(!pstr)
		{
			Con_EPrintf("%s - Expected closing '}', got EOL instead while reading '%s'.\n", __FUNCTION__, path.c_str());
			FL_FreeFile(pfile);
			return nullptr;
		}

		CString optionname = token;
		if(!qstrcmp(optionname, "$fontcolor")
			|| !qstrcmp(optionname, "$outlinecolor"))
		{
			// Parse the color component, it needs to be in quotes
			CString colortoken;
			pstr = Common::Parse(pstr, colortoken);

			// Color parsed
			color32_t color;
			// Parse the individual options
			const Char* ptokenstr = colortoken.c_str();

			Uint32 i = 0;
			while(ptokenstr)
			{
				ptokenstr = Common::Parse(ptokenstr, token);
				if(!ptokenstr && i < 3)
				{
					Con_EPrintf("%s - Only %d color values in '%s', 4 expected while reading '%s' from '%s'.\nMake sure the color values are encased in quotes.\n", 
						__FUNCTION__, (i+1), optionname.c_str(), path.c_str());
					break;
				}

				if(!Common::IsNumber(token.c_str()))
				{
					Con_EPrintf("%s - Color component '%s' is not a valid number in '%s' for option '%s'.\n", 
						__FUNCTION__, token.c_str(), path.c_str(), optionname.c_str());
					break;
				}

				Int32 value = SDL_atoi(token.c_str());
				if(value > 255 || value < 0)
				{
					Con_EPrintf("%s - Color component '%s' is not a valid number in '%s' for option '%s'.\n", 
						__FUNCTION__, token.c_str(), path.c_str(), optionname.c_str());
					break;
				}

				switch(i)
				{
				case 0: color.r = value; break;
				case 1: color.g = value; break;
				case 2: color.b = value; break;
				case 3: color.a = value; break;
				}

				i++;
			}

			if(i != 4)
			{
				Con_EPrintf("%s - Error parsing '%s' instead while reading '%s'.\n", __FUNCTION__, optionname.c_str(), path.c_str());
				FL_FreeFile(pfile);
				return nullptr;
			}

			if(!qstrcmp(optionname, "$fontcolor"))
				schema.color = color;
			else
				schema.outlinecolor = color;
		}
		else if(!qstrcmp(optionname, "$fontset"))
		{
			// Parse the name of the font set
			pstr = Common::Parse(pstr, token);
			if(!pstr)
			{
				Con_EPrintf("%s - Unexpected EOL instead while reading '%s'.\n", __FUNCTION__, path.c_str());
				FL_FreeFile(pfile);
				return nullptr;
			}

			schema.fontsetname = token;
		}
		else if(!qstrcmp(optionname, "$fontsize")
			|| !qstrcmp(optionname, "$outlineradius"))
		{
			// Parse the value token
			pstr = Common::Parse(pstr, token);
			if(!pstr)
			{
				Con_EPrintf("%s - Unexpected EOL instead while reading %s '%s'.\n", __FUNCTION__, optionname.c_str(), path.c_str());
				FL_FreeFile(pfile);
				return nullptr;
			}

			if(!Common::IsNumber(token.c_str()))
			{
				Con_EPrintf("%s - Option '%s' value '%s' is not a number in '%s'.\n", __FUNCTION__, optionname.c_str(), token.c_str(), path.c_str());
				FL_FreeFile(pfile);
				return nullptr;
			}

			Uint32 value = SDL_atoi(token.c_str());
			if(!qstrcmp(optionname, "$fontsize"))
			{
				if(value < 4 || value > 256) // Arbitrary
				{
					Con_EPrintf("%s - Resolution option '%s' value '%s' is not a valid value in '%s'.\n", __FUNCTION__, optionname.c_str(), token.c_str(), path.c_str());
					FL_FreeFile(pfile);
					return nullptr;
				}

				schema.fontsize = value;
			}
			else if(!qstrcmp(optionname, "$outlineradius"))
			{
				if(value < 0 || value > 4) // Arbitrary
				{
					Con_EPrintf("%s - Option '%s' value '%s' is not a valid value in '%s'.\n", __FUNCTION__, optionname.c_str(), token.c_str(), path.c_str());
					FL_FreeFile(pfile);
					return nullptr;
				}

				schema.outlineradius = value;
			}
		}
		else if(!qstrcmp(optionname, "$resolution"))
		{
			// Parse the value token
			pstr = Common::Parse(pstr, token);
			if(!pstr)
			{
				Con_EPrintf("%s - Unexpected EOL instead while reading '%s' '%s'.\n", __FUNCTION__, optionname.c_str(), path.c_str());
				FL_FreeFile(pfile);
				return nullptr;
			}

			if(!Common::IsNumber(token.c_str()))
			{
				Con_EPrintf("%s - Option '%s' resolution value '%s' is not a number in '%s'.\n", __FUNCTION__, optionname.c_str(), token.c_str(), path.c_str());
				FL_FreeFile(pfile);
				return nullptr;
			}

			Uint32 resolutionValue = SDL_atoi(token.c_str());
			if(resolutionValue < CWindow::MIN_SCREEN_HEIGHT)
			{
				Con_EPrintf("%s - Option '%s' resolution value '%s' is lower than the minimum resolution %d in '%s'.\n", __FUNCTION__, optionname.c_str(), token.c_str(), CWindow::MIN_SCREEN_HEIGHT, path.c_str());
				FL_FreeFile(pfile);
				return nullptr;
			}

			// Parse the next token, it needs to be a bracket
			pstr = Common::Parse(pstr, token);
			if(qstrcmp(token, "{"))
			{
				Con_EPrintf("%s - Expected '{', got '%s' instead while reading resolution %d in '%s'.\n", __FUNCTION__, token.c_str(), resolutionValue, path.c_str());
				FL_FreeFile(pfile);
				return nullptr;
			}

			if(!pstr)
			{
				Con_EPrintf("%s - Expected closing '}', got EOL instead while reading resolution %d in '%s'.\n", __FUNCTION__, resolutionValue, path.c_str());
				FL_FreeFile(pfile);
				return nullptr;
			}

			// Read the options in
			text_reschema_t resinfo;
			resinfo.screenheight = resolutionValue;

			while(true)
			{
				pstr = Common::Parse(pstr, token);
				if(!qstrcmp(token, "}"))
					break;

				// Make sure it's not EOL
				if(!pstr)
				{
					Con_EPrintf("%s - Expected closing '}', got EOL instead while reading resolution %d in '%s'.\n", __FUNCTION__, resolutionValue, path.c_str());
					FL_FreeFile(pfile);
					return nullptr;
				}

				CString resoptionname = token;
				if(!qstrcmp(resoptionname, "$fontsize")
					|| !qstrcmp(resoptionname, "$outlineradius"))
				{
					// Parse the value token
					pstr = Common::Parse(pstr, token);
					if(!pstr)
					{
						Con_EPrintf("%s - Unexpected EOL instead while reading %s '%s'.\n", __FUNCTION__, resoptionname.c_str(), path.c_str());
						FL_FreeFile(pfile);
						return nullptr;
					}

					if(!Common::IsNumber(token.c_str()))
					{
						Con_EPrintf("%s - Option '%s' value '%s' for resolution %d is not a number in '%s'.\n", __FUNCTION__, resoptionname.c_str(), token.c_str(), resolutionValue, path.c_str());
						FL_FreeFile(pfile);
						return nullptr;
					}

					Uint32 value = SDL_atoi(token.c_str());
					if(!qstrcmp(resoptionname, "$fontsize"))
					{
						if(value < 4 || value > 256) // Arbitrary
						{
							Con_EPrintf("%s - Resolution option '%s' value '%s' is not a valid value in '%s'.\n", __FUNCTION__, resoptionname.c_str(), token.c_str(), path.c_str());
							FL_FreeFile(pfile);
							return nullptr;
						}

						resinfo.fontsize = value;
					}
					else if(!qstrcmp(resoptionname, "$outlineradius"))
					{
						if(value < 0 || value > 4) // Arbitrary
						{
							Con_EPrintf("%s - Option '%s' value '%s' is not a valid value in '%s'.\n", __FUNCTION__, resoptionname.c_str(), token.c_str(), path.c_str());
							FL_FreeFile(pfile);
							return nullptr;
						}

						resinfo.outlineradius = value;
					}
				}
			}

			schema.resolutions.push_back(resinfo);
		}
		else
		{
			// Inform about broken crap
			Con_Printf("%s - Unknown option '%s' in '%s'.\n", __FUNCTION__, token.c_str(), path.c_str());
		}
	}

	// Release the file
	FL_FreeFile(pfile);

	// Return the element added
	return &m_schemasList.add(schema)->_val;
}