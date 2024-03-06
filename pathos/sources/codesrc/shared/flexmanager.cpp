/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "flex_shared.h"
#include "vbmformat.h"
#include "flexmanager.h"
#include "common.h"

#pragma warning( disable: 4244 )

// Defines for blinking
#define BLINK_IN		0.1
#define BLINK_CLOSED	0.09
#define BLINK_OUT		0.085

// Default interpolation time
#define INTERP_TIME		0.5

// Script folder path for flexes
static const Char FLEX_SCRIPT_FOLDER_PATH[] = "scripts/expressions";
// Expression script format extension
static const Char FLEX_SCRIPT_FORMAT_EXTENSION[] = ".ccs";

//====================================
//
//====================================
CFlexManager::CFlexManager( const file_interface_t& fileInterface ):
	m_mouthValue(0),
	m_fileInterface(fileInterface)
{
}

//====================================
//
//====================================
CFlexManager::~CFlexManager()
{
}

//====================================
//
//====================================
void CFlexManager::Clear( void )
{
	if(!m_scriptCache.empty())
	{
		for(Uint32 i = 0; i < m_scriptCache.size(); i++)
			delete m_scriptCache[i];

		m_scriptCache.clear();
	}

	if(!m_sentenceScriptArray.empty())
		m_sentenceScriptArray.clear();

	for(Uint32 i = 0; i < NUM_FLEX_NPC_TYPES; i++)
	{
		for(Uint32 j = 0; j < NUM_FLEX_AISTATES; j++)
		{
			if(!m_flexAIStateScripts[i][j].empty())
				m_flexAIStateScripts[i][j].clear();
		}
	}

	if(!m_errorString.empty())
		m_errorString.clear();
}

//====================================
//
//====================================
Int32 CFlexManager::GetFixedFlexIndex( const Char *pstrname )
{
	if(!qstrcmp(pstrname, "mouth_open"))
		return FLEX_MOUTH_OPEN;
	else if(!qstrcmp(pstrname, "blink"))
		return FLEX_BLINK;
	else if(!qstrcmp(pstrname, "eyes_squint"))
		return FLEX_EYES_SQUINT;
	else if(!qstrcmp(pstrname, "eyes_wide"))
		return FLEX_EYES_WIDE;
	else
		return -1;
}

//====================================
//
//====================================
void CFlexManager::SetFlexMappings( const vbmheader_t* pvbm, flexstate_t* pstate )
{
	const vbmflexcontroller_t* pflexcontrollers = reinterpret_cast<const vbmflexcontroller_t*>(reinterpret_cast<const byte*>(pvbm) + pvbm->flexcontrolleroffset);
	for(Int32 i = 0; i < pvbm->numflexcontrollers; i++)
	{
		// Reset to default
		pstate->indexmap[i] = GetFixedFlexIndex(pflexcontrollers[i].name);
		if(pstate->indexmap[i] != -1)
		{
			pstate->r_indexmap[pstate->indexmap[i]] = i;
			continue;
		}

		if(!pstate->pscript)
			continue;

		for(Uint32 j = 0; j < pstate->pscript->controllers.size(); j++)
		{
			if(!qstrcmp(pflexcontrollers[i].name, pstate->pscript->controllers[j].name))
			{
				pstate->indexmap[i] = pstate->pscript->controllers[j].index;
				pstate->r_indexmap[pstate->indexmap[i]] = i;
				break;
			}
		}
	}
}

//====================================
//
//====================================
void CFlexManager::SetScript( const vbmheader_t* pvbm, flexstate_t* pstate, Float time, const Char* pscriptname )
{
	const flexscript_t* pfile = LoadScript( pscriptname );
	if( !pfile )
		return;

	pstate->pscript = pfile;
	pstate->time = time;

	// save previous values
	Float saved_values[MAX_VBM_FLEXES];
	for(Int32 i = 0; i < MAX_VBM_FLEXES; i++)
		saved_values[i] = 0;
	
	for(Int32 i = 0; i < pvbm->numflexcontrollers; i++)
	{
		if(pstate->r_indexmap[i] != -1 && pstate->indexmap[pstate->r_indexmap[i]] != -1)
			saved_values[pstate->r_indexmap[i]] = pstate->values[i];

		pstate->values[i] = 0;
	}
	
	// Reset the flex mappings
	SetFlexMappings(pvbm, pstate);

	// count unique flexes
	Int32 numuniqueflex = 0;
	for(Uint32 i = 0; i < pfile->controllers.size(); i++)
	{
		if(pfile->controllers[i].index >= NB_FIXED_FLEXES)
			numuniqueflex++;
	}

	// Create new entries for blending if needed
	for(Int32 i = 0; i < pvbm->numflexcontrollers; i++)
	{
		if(pstate->prev_values[i] && pstate->indexmap[i] == -1)
		{
			pstate->indexmap[i] = numuniqueflex + NB_FIXED_FLEXES;
			pstate->r_indexmap[pstate->indexmap[i]] = i;
			numuniqueflex++;
		}
	}
	
	// Copy previous values
	Int32 j = 0;
	for(; j < pvbm->numflexcontrollers; j++)
	{
		if(pstate->indexmap[j] != -1)
			pstate->prev_values[pstate->indexmap[j]] = saved_values[j];
	}
}

//====================================
//
//====================================
bool CFlexManager::SetControllerName( const flexscript_t* pscript, flexcontroller_t* pcontroller, const Char* pstrname )
{
	// Check for duplicacy
	for(Uint32 i = 0; i < pscript->controllers.size(); i++)
	{
		if(!qstrcmp(pscript->controllers[i].name, pstrname))
			return false;
	}

	// Set properties
	pcontroller->name = pstrname;
	pcontroller->index = GetFixedFlexIndex(pstrname);
	if(pcontroller->index == -1)
	{
		Uint32 index = 0;
		for(Uint32 i = 0; i < pscript->controllers.size(); i++)
		{
			if(&pscript->controllers[i] != pcontroller && pscript->controllers[i].index >= NB_FIXED_FLEXES)
				index++;
		}	

		pcontroller->index = NB_FIXED_FLEXES+index;
	}

	return true;
}

//====================================
//
//====================================
Int32 CFlexManager::GetControllerIndex( const flexscript_t* pscript, const Char* name )
{
	for(Uint32 i = 0; i < pscript->controllers.size(); i++)
	{
		if(!qstrcmp(pscript->controllers[i].name, name))
			return pscript->controllers[i].index;
	}

	return -1;
}

//====================================
//
//====================================
bool CFlexManager::LoadAssociationScript( flextypes_t npcType, const Char* pscriptname )
{
	// Make sure the type is valid
	if( npcType == FLEX_NPC_TYPE_NONE || npcType != FLEX_NPC_TYPE_HUMAN && npcType != FLEX_NPC_TYPE_UNUSED )
	{
		SetError("%s - Unknown NPC type '%d' in '%s'", __FUNCTION__, (Int32)npcType, pscriptname);
		return false;
	}

	CString filepath;
	filepath << FLEX_SCRIPT_FOLDER_PATH << PATH_SLASH_CHAR << pscriptname;
	const byte* pfile = m_fileInterface.pfnLoadFile(filepath.c_str(), nullptr);
	if(!pfile)
	{
		SetError("%s - Could not load script '%s'", __FUNCTION__, filepath.c_str());
		return false;
	}

	static Char line[MAX_LINE_LENGTH];
	static Char field[MAX_PARSE_LENGTH];

	const Char* pstr = reinterpret_cast<const Char*>(pfile);
	while(true)
	{
		if(!pstr)
			break;

		pstr = Common::ReadLine(pstr, line);
		if(!qstrlen(line))
			continue;

		const Char* pchar = Common::Parse(line, field);
		if(!pchar)
		{
			// Fail on missing definitions
			if(qstrlen(field))
			{
				SetError("%s - No scripts specified for association name '%s' in '%s'", __FUNCTION__, field, pscriptname);
				m_fileInterface.pfnFreeFile(pfile);
				return false;
			}

			// Empty line most likely
			continue;
		}

		if(!qstrncmp(field, "//", 2) || !qstrncmp(field, "#", 1))
		{
			// Skip comments
			continue;
		}

		if(!qstrcmp(field, "SCRIPT_EXPRESSION")) 
		{
			sent_assoc_t newassoc;

			// Read in the sentence name
			pchar = Common::Parse(pchar, field);
			if(!pchar)
			{
				SetError("%s - Empty sentence script '%s' in '%s'", __FUNCTION__, field, pscriptname);
				m_fileInterface.pfnFreeFile(pfile);
				return false;
			}

			// Copy the name and convert it to lowercase
			newassoc.sentence = field;
			newassoc.sentence.tolower();
			
			// Get the script file
			Common::Parse(pchar, field);
			newassoc.flexfile = field;
			newassoc.flexfile.tolower();

			m_sentenceScriptArray.push_back(newassoc);
		}
		else
		{
			Int32 aiState = FLEX_AISTATE_NONE;
			if(!qstrcmp(field, "IDLE_EXPRESSIONS")) 
				aiState = FLEX_AISTATE_IDLE;
			else if(!qstrcmp(field, "SCARED_EXPRESSIONS")) 
				aiState = FLEX_AISTATE_AFRAID;
			else if(!qstrcmp(field, "DEAD_EXPRESSIONS")) 
				aiState = FLEX_AISTATE_DEAD;
			else if(!qstrcmp(field, "COMBAT_EXPRESSIONS")) 
				aiState = FLEX_AISTATE_COMBAT;
			else
			{	
				SetError("%s - Unknown AI state '%s' in '%s'", __FUNCTION__, field, pscriptname);
				m_fileInterface.pfnFreeFile(pfile);
				return false;
			}

			// Link it to the association list
			CArray<CString>& scriptlist = m_flexAIStateScripts[npcType][aiState];

			while(pchar)
			{
				pchar = Common::Parse(pchar, field);
				scriptlist.push_back(field);
			}
		}
	}

	m_fileInterface.pfnFreeFile(pfile);

	// Precache these scripts
	for(Uint32 iNPC = FLEX_NPC_TYPE_NONE+1; iNPC < NUM_FLEX_NPC_TYPES; iNPC++)
	{
		for(Int32 iState = FLEX_AISTATE_NONE+1; iState < NUM_FLEX_AISTATES; iState++)
		{
			CArray<CString>& scriptlist = m_flexAIStateScripts[iNPC][iState];
			if(scriptlist.empty())
				continue;

			for(Uint32 i = 0; i < scriptlist.size(); i++)
				LoadScript(scriptlist[i].c_str());
		}
	}

	// Precache sentence scripts as well
	for(Uint32 i = 0; i < m_sentenceScriptArray.size(); i++)
		LoadScript(m_sentenceScriptArray[i].flexfile.c_str());

	return true;
}

//====================================
//
//====================================
const Char* CFlexManager::GetAIScript( flextypes_t npcType, flexaistates_t aiState )
{
	if( npcType == FLEX_NPC_TYPE_NONE || npcType != FLEX_NPC_TYPE_HUMAN && npcType != FLEX_NPC_TYPE_UNUSED )
	{
		SetError("%s - Unknown NPC type '%d' for state '%d'\n", __FUNCTION__, npcType, aiState );
		return nullptr;
	}

	if( aiState == FLEX_AISTATE_NONE || aiState != FLEX_AISTATE_IDLE && aiState != FLEX_AISTATE_AFRAID && aiState != FLEX_AISTATE_DEAD && aiState != FLEX_AISTATE_COMBAT && aiState != FLEX_AISTATE_SCRIPT )
	{
		SetError("%s - Unknown AI state '%d'\n", __FUNCTION__, aiState );
		return nullptr;
	}

	if(m_flexAIStateScripts[npcType][aiState].empty())
		return nullptr;

	CArray<CString>& scriptArray = m_flexAIStateScripts[npcType][aiState];
	Uint32 scriptIndex = Common::RandomLong( 0, (((Int32)scriptArray.size())-1) );

	return scriptArray[scriptIndex].c_str();
}

//====================================
//
//====================================
const Char* CFlexManager::GetSentenceScript( const Char* pstrSentence )
{
	CString sentence = pstrSentence;
	sentence.tolower();

	for(Uint32 i = 0; i < m_sentenceScriptArray.size(); i++)
	{
		if(!qstrcmp(m_sentenceScriptArray[i].sentence, sentence))
			return m_sentenceScriptArray[i].flexfile.c_str();
	}

	return nullptr;
}

//====================================
//
//====================================
const flexscript_t* CFlexManager::LoadScript( const Char* pscriptname )
{
	CString filepath;
	if(qstrstr(pscriptname, ":") == nullptr)
		filepath << FLEX_SCRIPT_FOLDER_PATH << PATH_SLASH_CHAR << pscriptname << FLEX_SCRIPT_FORMAT_EXTENSION;
	else
		filepath = pscriptname;

	for(Uint32 i = 0; i < m_scriptCache.size(); i++)
	{
		if(!qstrcmp(m_scriptCache[i]->filename, filepath))
			return m_scriptCache[i];
	}

	const byte* pfile = m_fileInterface.pfnLoadFile(filepath.c_str(), nullptr);
	if(!pfile)
	{
		SetError("%s - Could not load file '%s'", __FUNCTION__, pscriptname);
		return nullptr;
	}

	// Allocate new script
	flexscript_t* pscript = new flexscript_t;
	pscript->filename = pscriptname;

	CString field;

	// Read in the file contents
	const Char* pchar = reinterpret_cast<const Char*>(pfile);

	if(*pchar != '{')
	{
		m_fileInterface.pfnFreeFile(pfile);
		return nullptr;
	}
	
	// Skip opening bracket
	pchar++;

	CString szfield;
	CString szvalue;

	bool bError = false;
	Int32 controller_index = 0;

	// Parse the script
	while(*pchar)
	{
		// Skip whitespace
		while(SDL_isspace(*pchar)) 
			pchar++;

		// File close
		if(*pchar == '}' || !(*pchar) || bError)
			break;

		while(1)
		{
			if(bError)
				break;

			// Skip whitespace
			while(SDL_isspace(*pchar)) 
				pchar++;

			// Parse the entry
			if(*pchar == '{')
			{
				flexcontroller_t newcontroller;

				// Skip opening bracket
				pchar++;

				while(!bError)
				{
					// Skip whitespace
					while(SDL_isspace(*pchar)) 
						pchar++;

					// Read keyframe
					if( *pchar == '{' )
					{
						flexbind_t newbind;

						// skip bracket
						pchar++;

						while(!bError)
						{
							// Skip whitespace
							while(SDL_isspace(*pchar)) 
								pchar++;

							if( *pchar == '}' )
							{
								pchar++;
								break;
							}
						
							// Extract field name
							pchar = Common::Parse( pchar, szfield );

							// Skip whitespace
							while(isspace(*pchar)) 
								pchar++;

							// Extract field value
							pchar = Common::Parse( pchar, szvalue );

							if(!qstrcmp(szfield, "time")) 
								newbind.time = SDL_atof(szvalue.c_str());
							else if(!qstrcmp(szfield, "strength")) 
								newbind.strength = SDL_atof(szvalue.c_str());
							else
							{
								SetError(false, "Error reading %s: Unknown field: %s", pscriptname, szfield.c_str());
								bError = true;
								break;
							}
						}

						// Add the new bind
						newcontroller.binds.push_back(newbind);

						// Begin from start
						continue;
					}

					// Skip whitespace
					while(SDL_isspace(*pchar)) 
						pchar++;

					if(*pchar == '}')
					{
						pchar++;
						break;
					}

					// Extract field name
					pchar = Common::Parse(pchar, szfield);

					// Skip whitespace
					while(SDL_isspace(*pchar)) 
						pchar++;

					// Extract field value
					pchar = Common::Parse( pchar, szvalue );

					// Add the new controller
					if(!SetControllerName(pscript, &newcontroller, szvalue.c_str()))
					{
						SetError("Duplicate flex controller %s in %s\n", pscript->filename.c_str(), szvalue.c_str());
						bError = true;
						break;
					}
					else if(newcontroller.index >= NB_FIXED_FLEXES)
					{
						// Advance the index
						controller_index++;
					}


				}

				pscript->controllers.push_back(newcontroller);
			}

			// Skip whitespace
			while(SDL_isspace(*pchar)) 
				pchar++;

			if(*pchar == '}')
			{
				pchar++;
				break;
			}

			if(*pchar == '{')
				continue;

			// Extract field name
			pchar = Common::Parse( pchar, szfield, "\n" );
			if(!qstrcmp(szfield, "flags")) 
			{
				while(*pchar != '\n')
				{
					pchar = Common::Parse(pchar, szvalue, "\n");

					if(!qstrcmp(szvalue, "looped"))
						pscript->flags |= FLEX_FLAG_LOOP;
					else if(!qstrcmp(szvalue, "stay"))
						pscript->flags |= FLEX_FLAG_STAY;
					else
					{
						SetError(false, "Error reading %s: Unknown flag: %s", pscriptname, szvalue.c_str());
						bError = true;
					}
				}
			}
			else
			{
				SetError(false, "Error reading %s: Unknown field: %s", pscriptname, szfield.c_str());
				bError = true;
			}
		}
	}

	m_fileInterface.pfnFreeFile(pfile);

	if(bError)
	{
		delete pscript;
		return nullptr;
	}

	// Determine length
	for(Uint32 i = 0; i < pscript->controllers.size(); i++)
	{
		for(Uint32 j = 0; j < pscript->controllers[i].binds.size(); j++)
		{
			if(pscript->controllers[i].binds[j].time > pscript->duration)
				pscript->duration = pscript->controllers[i].binds[j].time;
		}
	}

	// Add to the list
	m_scriptCache.push_back(pscript);
	
	return pscript;
}

//====================================
//
//====================================
void CFlexManager::SetMouth( Int32 mouthopen, flexstate_t* pstate )
{
	Float value = (Float)mouthopen / 30.0f;
	value = clamp(value, 0.0, 1.0);

	Float outValue = pstate->values[FLEX_MOUTH_OPEN] += value;
	pstate->values[FLEX_MOUTH_OPEN] = clamp(outValue, 0.0, 1.0);
}

//====================================
//
//====================================
void CFlexManager::Blink( Float cur_time, Float health, flexstate_t* pstate )
{
	if(!cur_time)
		return;

	if(!pstate->nextblink)
	{
		pstate->nextblink = cur_time + Common::RandomFloat(0.5, 3);
		return;
	}

	if(pstate->nextblink > cur_time)
		return;

	Float blinkfull = pstate->nextblink + BLINK_IN;
	Float blinkout = blinkfull + BLINK_CLOSED;
	Float blinkdone = blinkout + BLINK_OUT;

	Float value = 0.0;
	if( cur_time < blinkfull && cur_time < blinkout )
	{
		Float local_time = cur_time - pstate->nextblink;
		value = Common::SplineFraction( local_time, 1/BLINK_IN );
	}
	else if( cur_time < blinkout && cur_time < blinkdone )
	{
		value = 1.0;
	}
	else if( cur_time < blinkdone && cur_time > blinkout )
	{
		Float local_time = cur_time - blinkout;
		value = 1.0 - Common::SplineFraction( local_time, 1/BLINK_OUT );
	}
	else if( cur_time > blinkdone )
	{
		pstate->nextblink = cur_time + Common::RandomFloat(0.5, 3);
		value = 0.0;
	}

	Float outValue = pstate->values[FLEX_BLINK] + value;
	pstate->values[FLEX_BLINK] = clamp(outValue, 0.0, 1.0);

	// Curb eyes wide and eyes squint by blink value
	Float eyeFlexesValue = 1.0 - pstate->values[FLEX_BLINK];
	pstate->values[FLEX_EYES_WIDE] *= eyeFlexesValue;
	pstate->values[FLEX_EYES_SQUINT] *= eyeFlexesValue;
}

//====================================
//
//====================================
void CFlexManager::UpdateValues( Float cur_time, Float health, Int32 mouthopen, flexstate_t* pstate, bool scripting )
{
	// Reset mouth and blink flexes
	pstate->values[FLEX_BLINK] = 0;
	pstate->values[FLEX_MOUTH_OPEN] = 0;

	// Temporary array of values
	Float flexValues[MAX_VBM_FLEXES];
	for(Int32 i = 0; i < MAX_VBM_FLEXES; i++)
		flexValues[i] = 0;

	// Add values from the script first, if present
	if(pstate->pscript)
	{
		const flexscript_t* pscript = pstate->pscript;

		for(Uint32 i = 0; i < pscript->controllers.size(); i++)
		{
			const flexcontroller_t* pcontroller = &pscript->controllers[i];
			for(Int32 j = 0; j < ((Int32)pcontroller->binds.size())-1; j++)
			{
				Double bindtime = pstate->time + pcontroller->binds[j].time;
				Double nextbindtime = pstate->time + pcontroller->binds[j+1].time;

				// Not there yet
				if(bindtime > cur_time)
				{
					// If it's the first, then set it's value
					if(j == 0)
						flexValues[pcontroller->index] = pcontroller->binds[j].strength;

					// Nothing to do here yet
					break;
				}

				// Already passed this segment
				if(bindtime < cur_time && nextbindtime < cur_time)
				{
					// If we're at the last one, just keep the last max value
					if(j == (((Int32)pcontroller->binds.size())-2))
						flexValues[pcontroller->index] = pcontroller->binds[j+1].strength;

					// Nothing to do on this one
					continue;
				}

				Double interval = nextbindtime - bindtime;
				Double cur_segment_time = cur_time - bindtime;
			
				Double curbindfraction = Common::SplineFraction(cur_segment_time, 1.0/interval);
				Double prevbindfraction = 1.0 - curbindfraction;

				Double prev_add = pcontroller->binds[j].strength*prevbindfraction;
				Double cur_add = pcontroller->binds[j+1].strength*curbindfraction;
				Double finalValue = prev_add + cur_add;

				flexValues[pcontroller->index] = finalValue;
			}
		}
	}

	// Interpolate if we're in time
	if((cur_time - pstate->time) < INTERP_TIME)
	{
		Float fullTime = cur_time - pstate->time;
		Float fullFrac = Common::SplineFraction(fullTime, 1.0/INTERP_TIME);

		for(Int32 i = 0; i < MAX_VBM_FLEXES; i++)
		{
			Float prevValue = pstate->prev_values[i]*(1.0-fullFrac);
			Float curValue = flexValues[i]*fullFrac;

			pstate->values[i] = prevValue + curValue;
		}
	}
	else
	{
		// Just straight copy
		for(Int32 i = 0; i < MAX_VBM_FLEXES; i++)
			pstate->values[i] = flexValues[i];
	}

	// Add in mouth movements
	SetMouth( mouthopen, pstate );

	// Only blink if we're allowed to
	if(!scripting && (!pstate->pscript || !(pstate->pscript->flags & FLEX_FLAG_NO_BLINK)))
		Blink( cur_time, health, pstate );
}

//====================================
//
//====================================
void CFlexManager::SetError( const Char *fmt, ... )
{
	va_list	vArgPtr;
	Char cMsg[MAX_PARSE_LENGTH];
	
	va_start(vArgPtr,fmt);
	vsprintf(cMsg, fmt, vArgPtr);
	va_end(vArgPtr);

	m_errorString = cMsg;
}
