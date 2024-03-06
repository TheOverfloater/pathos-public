/*
===============================================
Pathos Engine - Copyright Andrew Lucas

2016-2024
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "sentencesfile.h"
#include "constants.h"
#include "snd_shared.h"

// Comma delay amount
const Float CSentencesFile::COMMA_DELAY = 0.5;
// Default folder for sentences - Seems HL takes "vox" as the default dir
const Char CSentencesFile::DEFAULT_FOLDER[] = "vox";

//=============================================
// @brief
//
//=============================================
CSentencesFile::CSentencesFile( void ):
	m_pfnPrecacheSound(nullptr),
	m_pfnGetSoundDuration(nullptr)
{
}

//=============================================
// @brief
//
//=============================================
CSentencesFile::CSentencesFile( pfnPrecacheSoundFunctionPtr_t pfnPrecacheSound, pfnGetSoundDurationFunctionPtr_t pfnGetSoundDuration ):
	m_pfnPrecacheSound(pfnPrecacheSound),
	m_pfnGetSoundDuration(pfnGetSoundDuration)
{
}

//=============================================
// @brief
//
//=============================================
CSentencesFile::~CSentencesFile( void )
{
	Clear();
}

//=============================================
// @brief
//
//=============================================
bool CSentencesFile::Init( const byte* pfile )
{
	// Make sure the arrays are cleared
	Clear();

	Char lineBuffer[MAX_LINE_LENGTH];
	Char tokenBuffer[MAX_PARSE_LENGTH];
	Char chunkTokenBuffer[MAX_PARSE_LENGTH];

	Uint32 lineNb = 0;
	const Char* pstr = reinterpret_cast<const Char*>(pfile);
	while(pstr)
	{
		// Read in the line
		pstr = Common::ReadLine(pstr, lineBuffer);
		lineNb++;

		if(!qstrlen(lineBuffer))
			continue;

		// Get first token of the line
		const Char* plstr = Common::Parse(lineBuffer, tokenBuffer);
		if(!plstr && qstrncmp(lineBuffer, "//", 2))
		{
			CString warning;
			warning << __FUNCTION__ << " - Unexpected end of line " << lineNb;
			m_warningMessagesArray.push_back(warning);
			continue;
		}

		if(!qstrlen(tokenBuffer))
			continue;

		// Skip comments
		if(!qstrncmp(tokenBuffer, "//", 2))
			continue;

		// Add new sentence
		sentence_t* pNewSentence = new sentence_t;
		pNewSentence->name = tokenBuffer;
		pNewSentence->index = m_sentencesArray.size();

		// Build entry name
		CString entryname;
		entryname << "!" << (Int32)pNewSentence->index;

		pNewSentence->entryname = entryname;
		m_sentencesArray.push_back(pNewSentence);

		// Get the group's name from the entry
		CString groupName;
		Uint32 nameLength = qstrlen(tokenBuffer);
		if(SDL_isdigit(tokenBuffer[nameLength-1]))
		{
			Uint32 digitOffs = nameLength-1;
			while(SDL_isdigit(tokenBuffer[digitOffs]))
				digitOffs--;

			if(SDL_isdigit(tokenBuffer[digitOffs+1]))
				groupName.assign(tokenBuffer, digitOffs+1);
		}

		// If it doesn't have a number at the end
		if(!groupName.empty())
		{
			// Convert group name to lowercase
			groupName.tolower();

			// Try to find an existing group
			sentencegroup_t* pgroup = nullptr;
			for(Uint32 i = 0; i < m_sentenceGroupsArray.size(); i++)
			{
				if(!qstrcmp(m_sentenceGroupsArray[i]->groupname, groupName))
				{
					pgroup = m_sentenceGroupsArray[i];
					break;
				}
			}

			// If there's no group with this name, then create it
			if(!pgroup)
			{
				pgroup = new sentencegroup_t;
				m_sentenceGroupsArray.push_back(pgroup);

				// Set starting index
				pgroup->startindex = m_sentencesArray.size()-1;
				pgroup->groupname = groupName;
				pgroup->numsentences = 0;
			}

			pgroup->numsentences++;
		}

		// Set the defaults
		Uint32 defaultpitch = 100;
		Uint32 defaulttime = 0;
		Uint32 defaultstart = 0;
		Uint32 defaultend = 0;
		Uint32 defaultvolume = 100;

		// Set the chunk-specific ones
		Float chunkdelay = 0;
		Uint32 chunkpitch = PITCH_NORM;
		Uint32 chunktime = 0;
		Uint32 chunkstart = 0;
		Uint32 chunkend = 0;
		Uint32 chunkvolume = VOL_NORM*100;

		bool hasError = false;

		// Parse the line elements
		while(plstr)
		{
			// Parse in the token
			plstr = Common::Parse(plstr, tokenBuffer, "\\/", true, true);
			if(!qstrlen(tokenBuffer))
				break;

			if(plstr && ((*plstr) == '/' || (*plstr) == '\\'))
			{
				// It's a folder
				if(!pNewSentence->folder.empty())
					pNewSentence->folder << PATH_SLASH_CHAR;

				pNewSentence->folder << tokenBuffer;
				plstr++;
				continue;
			}
			else
			{
				// Get pointer to beginning
				const Char* ptstr = tokenBuffer;
				chunkTokenBuffer[0] = '\0';

				// Check for bracketed content first
				const Char* ptbstr = qstrstr(ptstr, "(");

				// Outer loop on chunk contents
				while(ptstr && (*ptstr) != '\0' && !hasError)
				{
					// If it's a bracket, then it's an option
					if(ptbstr != nullptr && (*ptbstr) != '\0' && (*ptbstr) != ',')
					{
						while(ptbstr && (*ptbstr) != '\0')
						{
							if((*ptbstr) == ')')
							{
								CString warning;
								warning << __FUNCTION__ << " - Error on line " << lineNb << ", unexpected ')' token";
								m_warningMessagesArray.push_back(warning);
								hasError = true;
								break;
							}

							if((*ptbstr) != '(')
								break;

							// Set default options
							Uint32 pitch = defaultpitch;
							Uint32 time = defaulttime;
							Uint32 start = defaultstart;
							Uint32 end = defaultend;
							Uint32 volume = defaultvolume;

							if(!ParseOptionToken(&ptbstr, ptbstr, pitch, time, start, end, volume))
							{
								CString warning;
								warning << __FUNCTION__ " - Error on line " << lineNb << " while parsing options";
								m_warningMessagesArray.push_back(warning);
								hasError = true;
								break;
							}

							// If we have a filename, then it's specific to the current chunk,
							// else it's the default for all chunks on this line
							if((*ptstr) == '(')
							{
								chunkpitch = defaultpitch = pitch;
								chunktime = defaulttime = time;
								chunkstart = defaultstart = start;
								chunkend = defaultend = end;
								chunkvolume = defaultvolume = volume;
							}
							else
							{
								chunkpitch = pitch;
								chunktime = time;
								chunkstart = start;
								chunkend = end;
								chunkvolume = volume;
							}
						}
					}
					else
					{
						// Make sure we don't enter an infinite loop
						if((*ptstr) == '(')
							ptstr = ptbstr;

						// Parse in the name token
						Uint32 tokenLength = 0;
						while(ptstr && (*ptstr) != '\0' && (*ptstr) != '(')
						{
							// Handle comma specially
							if((*ptstr) == ',')
							{
								chunkdelay = COMMA_DELAY;
								ptstr++;
								continue;
							}

							if(tokenLength == MAX_PARSE_LENGTH)
							{
								CString warning;
								warning << __FUNCTION__ << " - Token on line " << lineNb << " too long";
								m_warningMessagesArray.push_back(warning);
								hasError = true;
								break;
							}

							chunkTokenBuffer[tokenLength] = (*ptstr);
							tokenLength++; ptstr++;
						}

						if(hasError)
							break;

						// Skip whitespaces
						while(ptstr && (*ptstr) != '\0' && SDL_isspace(*ptstr))
							ptstr++;

						if(!tokenLength)
						{
							// Probably only a single comma
							continue;
						}

						// null terminate
						chunkTokenBuffer[tokenLength] = '\0';

						sent_chunk_t* pchunk = new sent_chunk_t;
						pchunk->pitch = chunkpitch;
						pchunk->time = chunktime;
						pchunk->start = chunkstart;
						pchunk->end = chunkend;
						pchunk->volume = chunkvolume;
						pchunk->delay = chunkdelay;
						chunkdelay = 0;

						pchunk->soundname = chunkTokenBuffer;
						if(!pNewSentence->chunks.empty())
							pchunk->pnext = pNewSentence->chunks[pNewSentence->chunks.size()-1];

						// Add it to the chunk array
						pNewSentence->chunks.push_back(pchunk);

						// Restore to defaults
						chunkpitch = defaultpitch;
						chunktime = defaulttime;
						chunkstart = defaultstart;
						chunkend = defaultend;
						chunkvolume = defaultvolume;

						// Reset to after-option portion
						ptstr = ptbstr;
					}
				}
			}
		}

		// Track errors
		if(hasError)
		{
			m_sentencesArray.erase(pNewSentence->index);
			delete pNewSentence;
			break;
		}

		// Check for default folder
		if(pNewSentence->folder.empty())
			pNewSentence->folder = DEFAULT_FOLDER;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
bool CSentencesFile::ParseOptionToken( const Char** ppoutstr, const Char* pstroption, Uint32& pitch, Uint32& time, Uint32& start, Uint32& end, Uint32& volume )
{
	Char optionchar = '\0';
	Char tokenbuffer[MAX_PARSE_LENGTH+1];

	const Char* pstr = pstroption;
	if((*pstr) == '(')
		pstr++;

	while((*pstr) != '\0')
	{
		while(SDL_isspace((*pstr)))
			pstr++;

		if((*pstr) == '(')
		{
			CString warning;
			warning << __FUNCTION__ << " - Error in option declaration, unexpected '('";
			m_warningMessagesArray.push_back(warning);
			return false;
		}

		if((*pstr) == ')')
		{
			pstr++;
			break;
		}

		if(!SDL_isdigit((*pstr)))
		{
			optionchar = (*pstr);
			pstr++;
			continue;
		}
		else
		{
			// Parse the value in
			Uint32 length = 0;
			while((*pstr) && SDL_isdigit((*pstr)) && length < MAX_PARSE_LENGTH)
			{
				tokenbuffer[length] = (*pstr);
				length++; pstr++;
			}

			// null terminate
			tokenbuffer[length] = '\0';

			Uint32 value = SDL_atoi(tokenbuffer);
			switch(optionchar)
			{
			case 'p':
				pitch = value;
				break;
			case 't':
				time = value;
				break;
			case 's':
				start = value;
				break;
			case 'e':
				end = value;
				break;
			case 'v':
				volume = value;
				break;
			default:
				{
					CString warning;
					warning << __FUNCTION__ << " - Unrecognized option " << optionchar;
					m_warningMessagesArray.push_back(warning);
				}
				break;
			}
		}

	}

	if(ppoutstr)
		(*ppoutstr) = pstr;

	return true;
}

//=============================================
// @brief
//
//=============================================
void CSentencesFile::Clear( void )
{
	if(!m_sentenceGroupsArray.empty())
	{
		for(Uint32 i = 0; i < m_sentenceGroupsArray.size(); i++)
			delete m_sentenceGroupsArray[i];

		m_sentenceGroupsArray.clear();
	}

	if(!m_sentencesArray.empty())
	{
		for(Uint32 i = 0; i < m_sentencesArray.size(); i++)
			delete m_sentencesArray[i];

		m_sentencesArray.clear();
	}

	if(!m_warningMessagesArray.empty())
		m_warningMessagesArray.clear();
}

//=============================================
// @brief
//
//=============================================
const Char* CSentencesFile::GetRandomSentence( const Char* pstrGroupName, Float* pDuration )
{
	if(!m_errorMessage.empty())
		m_errorMessage.clear();

	for(Uint32 i = 0; i < m_sentenceGroupsArray.size(); i++)
	{
		sentencegroup_t* pgrp = m_sentenceGroupsArray[i];
		if(!qstrcicmp(pgrp->groupname, pstrGroupName))
		{
			Uint32 endindex = pgrp->startindex + pgrp->numsentences - 1;
			Uint32 sentenceindex = Common::RandomLong(pgrp->startindex, endindex);
 			if(sentenceindex > m_sentencesArray.size())
			{
				m_errorMessage << __FUNCTION__ << " - Invalid sentence index '" << sentenceindex << "' in group '" << pgrp->groupname << "'";
				return nullptr;
			}

			sentence_t* psent = m_sentencesArray[sentenceindex];
			if(psent->duration == -1 && m_pfnGetSoundDuration)
				PrecacheSentence(psent);

			if(pDuration)
				(*pDuration) = psent->duration;

			return psent->entryname.c_str();
		}
	}

	m_errorMessage << __FUNCTION__ << " - Couldn't find sentence group '" << pstrGroupName << "'";
	return nullptr;
}

//=============================================
// @brief
//
//=============================================
const Char* CSentencesFile::GetSentence( const Char* pstrSentenceName, Float* pDuration )
{
	if(!m_errorMessage.empty())
		m_errorMessage.clear();

	if(!pstrSentenceName || !qstrlen(pstrSentenceName))
		return nullptr;

	const Char* pstrName;
	if(pstrSentenceName[0] == '!')
		pstrName = pstrSentenceName + 1;
	else
		pstrName = pstrSentenceName;

	for(Uint32 i = 0; i < m_sentencesArray.size(); i++)
	{
		sentence_t* psent = m_sentencesArray[i];
		if(!qstrcicmp(psent->name, pstrName))
		{
			if(psent->duration == -1 && m_pfnGetSoundDuration)
				PrecacheSentence(psent);

			if(pDuration)
				(*pDuration) = psent->duration;

			return psent->entryname.c_str();
		}
	}

	m_errorMessage << __FUNCTION__ << " - Couldn't find sentence '" << pstrSentenceName << "'";
	return nullptr;
}

//=============================================
// @brief
//
//=============================================
void CSentencesFile::PrecacheGroup( const Char* pstrGroupName )
{
	if(!pstrGroupName || !qstrlen(pstrGroupName) || !m_pfnPrecacheSound)
		return;

	sentencegroup_t* pgroup = nullptr;

	for(Uint32 i = 0; i < m_sentenceGroupsArray.size(); i++)
	{
		sentencegroup_t* pcheckgrp = m_sentenceGroupsArray[i];
		if(!qstrcicmp(pcheckgrp->groupname, pstrGroupName))
		{
			pgroup = pcheckgrp;
			break;
		}
	}
	
	if(!pgroup)
		return;

	for(Uint32 i = 0; i < pgroup->numsentences; i++)
	{
		CString sentname;
		sentname << pgroup->groupname << (Int32)i;

		sentence_t* psentence = GetSentenceDefinition(sentname.c_str());
		if(psentence)
		{
			Int32 sndIndex = m_pfnPrecacheSound(psentence->entryname.c_str());
			if(sndIndex == NO_PRECACHE)
				continue;
			
			if(m_pfnGetSoundDuration)
			{
				// Reset duration
				psentence->duration = 0;

				for(Uint32 j = 0; j < psentence->chunks.size(); j++)
				{
					sent_chunk_t* pchunk = psentence->chunks[j];
					if(!pchunk)
						continue;

					CString filepath;
					filepath << psentence->folder << PATH_SLASH_CHAR;
					filepath << pchunk->soundname << ".wav";

					Float duration = m_pfnGetSoundDuration(filepath.c_str(), pchunk->pitch);
					psentence->duration += duration;
				}
			}
		}
	}
}

//=============================================
// @brief
//
//=============================================
void CSentencesFile::PrecacheSentence( const Char* pstrSentenceName )
{
	if(!pstrSentenceName || !qstrlen(pstrSentenceName) || !m_pfnPrecacheSound)
		return;

	sentence_t* psentence = GetSentenceDefinition(pstrSentenceName);
	if(!psentence)
		return;

	Int32 sndIndex = m_pfnPrecacheSound(psentence->entryname.c_str());
	if(sndIndex == NO_PRECACHE)
		return;
		
	// Reset duration
	psentence->duration = 0;

	PrecacheSentence(psentence);
}

//=============================================
// @brief
//
//=============================================
void CSentencesFile::PrecacheSentence( sentence_t* psentence )
{
	if(!psentence || !m_pfnPrecacheSound)
		return;

	m_pfnPrecacheSound(psentence->entryname.c_str());

	// Reset this
	psentence->duration = 0;

	for(Uint32 i = 0; i < psentence->chunks.size(); i++)
	{
		sent_chunk_t* pchunk = psentence->chunks[i];
		if(!pchunk)
			continue;

		CString filepath;
		filepath << psentence->folder << PATH_SLASH_CHAR;
		filepath << pchunk->soundname << ".wav";

		Float duration = (*m_pfnGetSoundDuration)(filepath.c_str(), pchunk->pitch);
		psentence->duration += duration;
	}
}

//=============================================
// @brief
//
//=============================================
CSentencesFile::sentence_t* CSentencesFile::GetSentenceDefinition( const Char* pstrSentenceName )
{
	if(!pstrSentenceName || !qstrlen(pstrSentenceName))
		return nullptr;

	if(!m_errorMessage.empty())
		m_errorMessage.clear();

	if(pstrSentenceName[0] == '!')
	{
		Int32 index = SDL_atoi(pstrSentenceName+1);
		if(index < 0 || index >= (Int32)m_sentencesArray.size())
			return nullptr;

		return m_sentencesArray[index];
	}
	else
	{
		for(Uint32 i = 0; i < m_sentencesArray.size(); i++)
		{
			sentence_t* psent = m_sentencesArray[i];
			if(!qstrcicmp(psent->name, pstrSentenceName))
				return psent;
		}
	}

	m_errorMessage << __FUNCTION__ << " - Couldn't find sentence '" << pstrSentenceName << "'";
	return nullptr;
}

//=============================================
// @brief
//
//=============================================
const CSentencesFile::sentence_t* CSentencesFile::GetSentenceDefinition( Int32 index ) const
{
	if(index < 0 || index >= (Int32)m_sentencesArray.size())
		return nullptr;

	return m_sentencesArray[index];
}

//=============================================
// @brief
//
//=============================================
const CSentencesFile::sentence_t* CSentencesFile::GetSentenceDefinition( const Char* pstrSentenceName ) const
{
	if(!pstrSentenceName || !qstrlen(pstrSentenceName))
		return nullptr;

	if(pstrSentenceName[0] == '!')
	{
		Int32 index = SDL_atoi(pstrSentenceName+1);
		if(index < 0 || index >= (Int32)m_sentencesArray.size())
			return nullptr;

		return m_sentencesArray[index];
	}
	else
	{
		for(Uint32 i = 0; i < m_sentencesArray.size(); i++)
		{
			sentence_t* psent = m_sentencesArray[i];
			if(!qstrcicmp(psent->name, pstrSentenceName))
				return psent;
		}
	}
	return nullptr;
}

//=============================================
// @brief
//
//=============================================
Float CSentencesFile::GetSentenceDuration( Int32 index )
{
	if(index < 0 || index >= (Int32)m_sentencesArray.size())
		return 0;

	sentence_t* psentence = m_sentencesArray[index];

	if(!psentence)
		return 0;

	if(psentence->duration != -1)
		return psentence->duration;

	PrecacheSentence(psentence);
	return psentence->duration;
}

//=============================================
// @brief
//
//=============================================
Float CSentencesFile::GetSentenceDuration( const Char* pstrSentenceName )
{
	if(!pstrSentenceName || !qstrlen(pstrSentenceName))
		return 0;

	sentence_t* psentence = nullptr;
	if(pstrSentenceName[0] == '!')
	{
		Int32 index = SDL_atoi(pstrSentenceName+1);
		if(index < 0 || index >= (Int32)m_sentencesArray.size())
			return 0;

		psentence = m_sentencesArray[index];
	}
	else
	{
		psentence = GetSentenceDefinition(pstrSentenceName);
	}

	if(!psentence)
		return 0;

	if(psentence->duration != -1)
		return psentence->duration;

	PrecacheSentence(pstrSentenceName);
	return psentence->duration;
}

//=============================================
// @brief
//
//=============================================
bool CSentencesFile::HasWarnings( void ) const
{
	return (m_warningMessagesArray.empty()) ? false : true;
}

//=============================================
// @brief
//
//=============================================
const Char* CSentencesFile::GetWarning( Uint32 index )
{
	if(index >= m_warningMessagesArray.size())
		return nullptr;
	else
		return m_warningMessagesArray[index].c_str();
}

//=============================================
// @brief
//
//=============================================
Uint32 CSentencesFile::GetNbWarnings( void ) const
{
	return m_warningMessagesArray.size();
}

//=============================================
// @brief
//
//=============================================
bool CSentencesFile::HasError( void ) const
{
	return (m_errorMessage.empty()) ? false : true;
}

//=============================================
// @brief
//
//=============================================
const Char* CSentencesFile::GetError( void ) const
{
	if(!m_errorMessage.empty())
		return m_errorMessage.c_str();
	else
		return nullptr;
}