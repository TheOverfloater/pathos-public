/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifdef WIN32
#include <Windows.h>
#endif

#include <time.h>
#include <SDL.h>

#include "includes.h"
#include "logfile.h"
#include "entitydata.h"
#include "constants.h"
#include "wad3file.h"

namespace Common
{
	//=============================================
	// @brief Tells if a character is a break character
	//
	// @param character Character to check
	// @param pbreakchars Array of break characters
	// @return TRUE if break character, FALSE otherwise
	//=============================================
	bool IsBreakCharacter( Char character, const Char* pbreakchars )
	{
		if(!pbreakchars)
			return false;

		const Char* pstr = pbreakchars;
		while((*pstr))
		{
			if(character == (*pstr))
				return true;

			pstr++;
		}

		return false;
	}

	//=============================================
	// @brief Parses a token from an input string into another string
	//
	// @param pstr String to parse
	// @param pdest Destination string object
	// @param pstring Output string
	// @param pbreakchars Special characters to break on
	// @param ignoreComma Tells if commas should be ignored
	// @param checkCurlyBrackets If true, curly brackets are treated like quotes
	// @return Rest of the string or null if reached end
	//=============================================
	const Char* Parse( const Char *pstr, CString& str, const Char* pbreakchars, bool ignoreComma, bool checkCurlyBrackets )
	{
		static Char string[MAX_PARSE_LENGTH];
		const Char* preturn = Parse(pstr, string, pbreakchars, ignoreComma);

		str = string;
		return preturn;
	}

	//=============================================
	// @brief Parses a token from an input string into another string
	//
	// @param pstr String to parse
	// @param pdest Destination character array
	// @param pstring Output string
	// @param pbreakchars Special characters to break on
	// @param ignoreComma Tells if commas should be ignored
	// @param checkCurlyBrackets If true, curly brackets are treated like quotes
	// @return Rest of the string or null if reached end
	//=============================================
	const Char* Parse( const Char *pstr, Char* pdest, const Char* pbreakchars, bool ignoreComma, bool checkCurlyBrackets )
	{
		if (!pdest)
			return nullptr;

		if (!pstr)
			return nullptr;

		bool includeSpaces = false;
		Uint32 strLength = 0;
		const Char* ppstr = pstr;

		// skip whitespaces
		while(*ppstr && SDL_isspace(*ppstr) && !IsBreakCharacter((*ppstr), pbreakchars))
			ppstr++;

		while(*ppstr)
		{
			if(*ppstr == '/' && *(ppstr+1) == '/')
			{
				while(*ppstr != '\0' && *ppstr != '\n')
					ppstr++;

				if(*ppstr == '\0')
					break;

				ppstr++;
			}

			if(*ppstr == '/' && *(ppstr+1) == '*')
			{
				while(*ppstr != '*' && *(ppstr+1) != '/')
					ppstr++;

				ppstr += 2;
			}

			if(!includeSpaces && SDL_isspace(*ppstr) && !IsBreakCharacter((*ppstr), pbreakchars))
				break;

			if(IsBreakCharacter((*ppstr), pbreakchars))
				break;

			if(*ppstr != '\"')
			{
				if(strLength == MAX_PARSE_LENGTH)
				{
					// Fail if we reached the limit
					pdest[0] = '\0';
					return nullptr;
				}

				pdest[strLength] = *ppstr;
				strLength++; 
			}

			if(*ppstr == '\"' || checkCurlyBrackets && (*ppstr == '(' || *ppstr == ')'))
			{
				if(!includeSpaces)
					includeSpaces = true;
				else if(*ppstr == '\"')
				{
					ppstr++;
					break;
				}
				else
					includeSpaces = false;
			}

			if(!includeSpaces && *ppstr == ',' && !ignoreComma)
				break;

			ppstr++;
		}

		pdest[strLength] = '\0';

		// skip whitespaces
		while(*ppstr && SDL_isspace(*ppstr) && !IsBreakCharacter((*ppstr), pbreakchars))
			ppstr++;

		if(*ppstr == '\0')
			ppstr =  nullptr;

		return ppstr;
	}

	//=============================================
	// @brief Parses an entire line from an input string into another string
	//
	// @param pstr String to parse
	// @param pdest Destination string object
	//=============================================
	const Char* ReadLine( const Char* pstr, Char* pdest )
	{
		Char* ppdest = pdest;
		const Char* ppstr = pstr;

		while(*ppstr && *ppstr != '\n' && *ppstr != '\r')
		{
			if((ppdest - pdest) >= MAX_LINE_LENGTH)
			{
				pdest[0] = '\0';
				return nullptr;
			}

			*ppdest = *ppstr;
			ppdest++; ppstr++;
		}

		*ppdest = '\0';

		// skip whitespaces
		while(*ppstr && SDL_isspace(*ppstr) && *ppstr != '\n' && *ppstr != '\r')
			ppstr++;
	
		// Skip newline characters
		if(ppstr[0] == '\r' && ppstr[1] == '\n')
			ppstr += 2;
		else if(ppstr[0] == '\n')
			ppstr++;

		if(*ppstr == '\0')
			ppstr =  nullptr;

		return ppstr;
	}

	//=============================================
	// @brief Parses an entire line from an input string into another string
	//
	// @param pstr String to parse
	// @param str Destination string object
	//=============================================
	const Char* ReadLine( const Char* pstr, CString& str )
	{
		static Char buffer[1024];
		const Char *ppstr = ReadLine(pstr, buffer);

		str = buffer;
		return ppstr;
	}

	//=============================================
	// @brief Gets the current date as a string
	//
	// @return Current date in YYYYMMDD HHmmss format
	//=============================================
	CString GetDate( void )
	{
		const time_t curtime = time(0);
		struct tm tstruct;
		Char buffer[80];
		tstruct = *localtime(&curtime);

		// Get the date and time format
		strftime(buffer, sizeof(buffer), "%Y-%m-%d %X", &tstruct);
		return buffer;
	}

	//=============================================
	// @brief Takes a C type array and adds iCountNew element to it
	//
	// @param parray Pointer to original array
	// @param size Size of a single element in the array
	// @param count Current array element count
	// @param countNew Number of new elements to allocate
	// @return Pointer to newly allocated array
	//=============================================
	void *ResizeArray( void *parray, Uint64 size, Uint64 count, Uint64 countNew )
	{
		const Uint64 allocSize = size*(count+countNew);
		byte *pnew = new byte[allocSize]();

		if(parray && count)
		{
			memmove(pnew, parray, size*count);
			delete[] parray;
		}

		return pnew;
	}

	//=============================================
	// @brief Extracts the filename from a path string
	//
	// @param pstrin Input string's pointer
	// @param strOut Output string object
	//=============================================
	void Basename( const Char *pstrin, CString& strOut )
	{
		Char* pstrPath = new Char[qstrlen(pstrin)+1];
		Basename(pstrin, pstrPath);

		strOut = pstrPath;
		delete[] pstrPath;
	}

	//=============================================
	// @brief Extracts the filename from a path string
	//
	// @param pstrin Input string's pointer
	// @param pstrout Output string's pointer
	//=============================================
	void Basename( const Char *pstrin, Char *pstrout )
	{
		Uint32 lastdot = 0;
		Uint32 lastbar = 0;
		Uint32 pathlength = 0;
		
		for(Uint32 i = 0; i < static_cast<Uint32>(qstrlen(pstrin)); i++)
		{
			if(pstrin[i] == '/' || pstrin[i] == '\\')
				lastbar = i+1;

			if( pstrin[i] == '.' )
				lastdot = i;
		}

		if(!lastdot)
			lastdot = qstrlen(pstrin);

		for(Uint32 i = lastbar; i < qstrlen(pstrin); i++)
		{
			if(i == lastdot)
				break;

			pstrout[pathlength] = pstrin[i];
			pathlength++;
		}

		pstrout[pathlength] = 0;
	}

	//=============================================
	// @brief Tells if a string represents an integer number
	//
	// @param pstr Input string's pointer
	//=============================================
	bool IsNumber( const Char *pstr )
	{
		const Char* ppstr = pstr;
		while(*ppstr)
		{
			if(!SDL_isdigit(*ppstr) && *ppstr != '.' && *ppstr != '-')
				return false;

			ppstr++;
		}

		return true;
	}

	//=============================================
	// @brief Tells if a string represents an integer number
	//
	// @param pstr Reference to input string object
	//=============================================
	bool IsNumber( const CString& str )
	{
		return IsNumber(str.c_str());
	}

	//=============================================
	// @brief Converts a string to a vector
	//
	// @param pstrString Input string
	// @param pflVector Output vector
	//=============================================
	void StringToVector( const Char* pstrString, Vector& outVector )
	{
		static Char tmp[MAX_PARSE_LENGTH];

		Uint32 i = 0;
		const Char* pstr = pstrString;
		while(pstr)
		{
			// Skip any commas
			while(*pstr == ',')
				pstr++;

			pstr = Parse(pstr, tmp);
			outVector[i] = SDL_atof(tmp);
			i++;
		}

		if(i < 2)
		{
			for(Uint32 j = i+1; j < 3; j++)
				outVector[j] = 0;
		}
	}

	//=============================================
	// @brief Tells if the input is a power of two value
	//
	// @param size Size to check
	// @return TRUE if it's a power of two value, FALSE otherwise
	//=============================================
	bool IsPowerOfTwo( Uint32 size )
	{
		Uint32 _size = size;
		while(_size != 1)
		{
			if((_size % 2) != 0) 
				return false;

			_size /=2;
		}

		return true;
	}

	//=============================================
	// @brief Tells if a string represents an integer number
	//
	// @param pstr Reference to input string object
	//=============================================
	Char GetShiftedChar( Char c )
	{
		switch(c)
		{
		case '`': return '~';
		case '1': return '!';
		case '2': return '@';
		case '3': return '#';
		case '4': return '$';
		case '5': return '%';
		case '6': return '^';
		case '7': return '&';
		case '8': return '*';
		case '9': return '(';
		case '0': return ')';
		case '-': return '_';
		case '=': return '+';
		case '[': return '{';
		case ']': return '}';
		case ';': return ':';
		case '\'': return '\"';
		case '\\': return '|';
		case ',': return '<';
		case '.': return '>';
		case '/': return '?';
		default:
			return SDL_toupper(c);
		}
	}

	//=============================================
	// @brief Tells if the pitch value is reversed
	//
	// @param pitch Pitch value
	// @return true or false
	//=============================================
	Int32 IsPitchReversed( Float pitch )
	{
		const Int32 quadrant = static_cast<Int32>(pitch / 90) % 4;
		if ((quadrant == 1) || (quadrant == 2)) 
			return -1;
	
		return 1;
	}

	//=============================================
	// @brief Used when rendering dynamic lights
	//
	// @param pitch Pitch value
	// @return true or false
	//=============================================
	void FixVector( Float* pflVector )
	{
		Vector in(pflVector);
		Common::NormalizeAngles(in);

		for(Uint32 i = 0; i < 3; i++)
		{
			if(in[i] == 0) in[i] = 0.001;
			if(in[i] == 90) in[i] = 89.999;
			if(in[i] == 180) in[i] = 179.999;
			if(in[i] == 270) in[i] = 269.999;
			if(in[i] == 360) in[i] = 359.999;
			if(in[i] == -90) in[i] = -89.999;
			if(in[i] == -180) in[i] = -179.999;
			if(in[i] == -270) in[i] = -269.999;
			if(in[i] == -360) in[i] = -359.999;
		}

		for (Uint32 i = 0; i < 3; i++)
			pflVector[i] = in[i];
	}

	//=============================================
	// @brief Generates a random float value
	//
	// @param low Minimum value
	// @return high Maximum value
	// @return Random value
	//=============================================
	Float RandomFloat( Float low, Float high )
	{
		constexpr Int32 floatRandomMaxResolution = 1000;
		const Int32 randomvalue = rand() % floatRandomMaxResolution;

		// Constrict to 0-1 range first
		Float value = static_cast<Float>(randomvalue) / static_cast<Float>(floatRandomMaxResolution);
		// Then to the range of low-high
		value = value * (high - low) + low;

		return value;
	}

	//=============================================
	// @brief Generates a random long value
	//
	// @param low Minimum value
	// @return high Maximum value
	// @return Random value
	//=============================================
	Int64 RandomLong( Int64 low, Int64 high )
	{
		const Int64 valueRange = high - low;
		const Int64 randomvalue = rand() % (valueRange+1);

		return randomvalue + low;
	}

	//=============================================
	// @brief Makes slashes uniform in a path string
	//
	// @param pstring String to format
	// @return Formatted string
	//=============================================
	CString FixSlashes( const Char* pstring )
	{
		Char* pbuffer = new Char[qstrlen(pstring)+1];
		strcpy(pbuffer, pstring);

		Char* pstr = pbuffer;
		while(*pstr != '\0')
		{
			if(*pstr == '\\')
				(*pstr) = PATH_SLASH_CHAR;

			pstr++;
		}
	
		CString strout(pbuffer);
		delete[] pbuffer;
		return strout;
	}

	//=============================================
	// @brief Checks visibility on a set of leaf numbers
	//
	//=============================================
	bool CheckVisibility( const CArray<Uint32>& leafnums, const byte* pset )
	{
		if(leafnums.empty())
			return false;

		if(!pset)
			return true;

		for(Uint32 i = 0; i < leafnums.size(); i++)
		{
			if(pset[leafnums[i] >> 3] & (1 << (leafnums[i] & 7)))
				return true;
		}

		return false;
	}

	//=============================================
	// @brief Returns the number of lines in a text file
	//
	//=============================================
	Uint32 GetFileLineCount( const Char* pstrdata )
	{
		Uint32 linecount = 0;
		const Char* pstr = pstrdata;
		while(*pstr)
		{
			while(*pstr && pstr[0] != '\n')
				pstr++;

			linecount++;

			if(pstr[0] == '\r' && pstr[1] == '\n')
				pstr += 2;
			else if(pstr[0] == '\n')
				pstr++;
		}

		return linecount;
	}

	//=============================================
	// @brief Checks visibility on a set of leaf numbers
	//
	//=============================================
	bool GetWADList( const Char* pstrEntityData, CArray<CString>& outputArray )
	{
		// Retrieve the wad list from the entdata
		CString wadlist;
		Char token[MAX_PARSE_LENGTH];
		const Char* pscan = pstrEntityData;
		while(pscan && *pscan != '\0')
		{
			// Read first token
			pscan = Parse(pscan, token);
			if(!pscan || qstrcmp(token, "{"))
				return false;

			// temp struct to hold entdata
			entitydata_t entity;
			// Should suffice
			entity.values.reserve(64);

			while(true)
			{
				pscan = Parse(pscan, token);
				if(!qstrcmp(token, "}"))
					break;

				if(!pscan)
					return false;

				keyvalue_t *pkv = new keyvalue_t;
				qstrcpy(pkv->keyname, token);

				pscan = Parse(pscan, token);
				if(!pscan)
					return false;

				qstrcpy(pkv->value, token);

				entity.values.push_back(pkv);
			}

			// Check if it's the worldspawn entity
			const Char *pValue = ValueForKey(entity, "classname");
			if(pValue && !qstrcmp(pValue, "worldspawn"))
			{
				// Retrieve the wad list
				pValue = ValueForKey(entity, "wad");
				if(!pValue)
					break;

				// Set it
				wadlist = pValue;
				break;
			}
		}

		// If all textures are wadincludes, don't bother
		if(wadlist.empty())
			return true;

		const Char* pwadstr = wadlist.c_str();
		while(pwadstr)
		{
			pwadstr = Parse(pwadstr, token, ";");
			if(!qstrlen(token))
				break;

			if(pwadstr)
				pwadstr++;

			// Get basename
			CString basename;
			Basename(token, basename);
			basename << WAD_FILE_EXTENSION;

			outputArray.push_back(basename);
		}

		return true;
	}

	//====================================
	// 
	//
	//====================================
	void NormalizeAngles( Vector& angles )
	{
		for(Uint32 i = 0; i < 3; i++)
		{
			if(angles[i] < 0)
				angles[i] += SDL_ceil(SDL_fabs(angles[i])/360.0f)*360.0f;
			else if(angles[i] > 360)
				angles[i] -= SDL_floor(angles[i]/360.0f)*360.0f;
		}
	}

	//=============================================
	// @brief Converts the string to lowercase chars
	//
	//=============================================
	void ConvertStringToLowerCase( Char* pstring )
	{
		for (Char *temp = pstring; *temp; temp++) 
			*temp = ::tolower(*temp);
	}
};