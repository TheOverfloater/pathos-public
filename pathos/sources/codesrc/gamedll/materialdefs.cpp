/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "gd_includes.h"
#include "materialdefs.h"
#include "weapons_shared.h"

// Object definition
CMaterialDefinitions gMaterialDefinitions;

//=============================================
// @brief
//
//=============================================
CMaterialDefinitions::CMaterialDefinitions( void )
{
}

//=============================================
// @brief
//
//=============================================
CMaterialDefinitions::~CMaterialDefinitions( void )
{
}

//=============================================
// @brief
//
//=============================================
bool CMaterialDefinitions::Init( const Char* pscriptfile )
{
	if(!pscriptfile)
	{
		m_errorString << "Script data was null";
		return false;
	}

	CString token;
	const Char* pstr = pscriptfile;
	while(pstr && *pstr)
	{
		// Get first token
		pstr = Common::Parse(pstr, token);

		if(!pstr)
		{
			m_errorString << "Unexpected EOF";
			m_materialDefinitionsArray.clear();
			return false;
		}

		// Skip empty lines
		if(token.empty())
			continue;

		// Skip comments
		if(!qstrncmp(token, "//", 2))
		{
			while((*pstr) && (*pstr) != '\n')
				pstr++;

			while(SDL_isspace((*pstr)))
				pstr++;

			continue;
		}

		// First token should be the material name
		materialdefinition_t newdef;
		newdef.materialname = token;
		newdef.penetrationinfos.reserve(NB_BULLET_TYPES);
		newdef.ricochetinfos.reserve(NB_BULLET_TYPES);

		// Read bracket token
		pstr = Common::Parse(pstr, token);
		if(!pstr || token.empty())
		{
			m_errorString << "Unexpected EOF";
			m_materialDefinitionsArray.clear();
			return false;
		}

		if(qstrcmp(token, "{"))
		{
			m_errorString << "Expected '{', got '" << token << "' instead";
			m_materialDefinitionsArray.clear();
			return false;
		}

		// Read in contents
		while(true)
		{
			pstr = Common::Parse(pstr, token);
			if(token.empty())
			{
				m_errorString << "Unexpected EOF";
				m_materialDefinitionsArray.clear();
				return false;
			}

			if(!qstrcmp(token, "}"))
				break;

			if(!pstr)
			{
				m_errorString << "Unexpected EOF";
				m_materialDefinitionsArray.clear();
				return false;
			}

			if(!qstrcmp(token, "sounds"))
			{
				// Read bracket token
				pstr = Common::Parse(pstr, token);
				if(!pstr || token.empty())
				{
					m_errorString << "Unexpected EOF";
					m_materialDefinitionsArray.clear();
					return false;
				}

				if(qstrcmp(token, "{"))
				{
					m_errorString << "Expected '{', got '" << token << "' instead";
					m_materialDefinitionsArray.clear();
					return false;
				}

				while(true)
				{
					// Read list of sounds
					pstr = Common::Parse(pstr, token);
					if(!pstr || token.empty())
					{
						m_errorString << "Unexpected EOF";
						m_materialDefinitionsArray.clear();
						return false;
					}

					if(!qstrcmp(token, "}"))
						break;

					// Otherwise it is a sound file
					newdef.sounds.push_back(token);
				}
			}
			else if(!qstrcmp(token, "penetration"))
			{
				// Read bracket token
				pstr = Common::Parse(pstr, token);
				if(!pstr || token.empty())
				{
					m_errorString << "Unexpected EOF";
					m_materialDefinitionsArray.clear();
					return false;
				}

				if(qstrcmp(token, "{"))
				{
					m_errorString << "Expected '{', got '" << token << "' instead";
					m_materialDefinitionsArray.clear();
					return false;
				}

				while(true)
				{
					// Read bullet type
					pstr = Common::Parse(pstr, token);
					if(!pstr || token.empty())
					{
						m_errorString << "Unexpected EOF";
						m_materialDefinitionsArray.clear();
						return false;
					}

					if(!qstrcmp(token, "}"))
						break;

					CString bullettypename = token;
					Int32 i = 1, j = NO_POSITION; // Skip BULLET_NONE
					while(true)
					{
						if(!BULLET_TYPE_MAP[i].type)
							break;

						if(!qstrcicmp(BULLET_TYPE_MAP[i].name, bullettypename))
						{
							j = i;
							break;
						}

						i++;
					}

					if(j == NO_POSITION)
					{
						m_errorString << "Invalid bullet type " << bullettypename << " specified for material " << newdef.materialname;
						m_materialDefinitionsArray.clear();
						return false;
					}

					// Read bullet penetration depth
					Float penetrationDepth = 0;
					pstr = ReadFloat(pstr, penetrationDepth, 0);
					if(!pstr)
					{
						m_errorString << "Error encountered while reading penetration depth for bullet type " << bullettypename << " and material " << newdef.materialname << ".";
						m_materialDefinitionsArray.clear();
						return false;
					}

					// Read penetration max count
					Int32 maxPenetration = 0;
					pstr = ReadInt32(pstr, maxPenetration, 0);
					if(!pstr)
					{
						m_errorString << "Error encountered while reading max penetration count for bullet type " << bullettypename << " and material " << newdef.materialname << ".";
						m_materialDefinitionsArray.clear();
						return false;
					}

					// Read penetration chance
					Int32 penetrationChance = 0;
					pstr = ReadInt32(pstr, penetrationChance, 0);
					if(!pstr)
					{
						m_errorString << "Error encountered while reading penetration chance for bullet type " << bullettypename << " and material " << newdef.materialname << ".";
						m_materialDefinitionsArray.clear();
						return false;
					}

					// Read penetration damage falloff
					Float penetrationDmgFalloff = 0;
					pstr = ReadFloat(pstr, penetrationDmgFalloff, 0);
					if(!pstr)
					{
						m_errorString << "Error encountered while reading penetration chance for bullet type " << bullettypename << " and material " << newdef.materialname << ".";
						m_materialDefinitionsArray.clear();
						return false;
					}

					// Read penetration chance decrease
					Int32 penetrationChanceDecrease = 0;
					pstr = ReadInt32(pstr, penetrationChanceDecrease, 0);
					if(!pstr)
					{
						m_errorString << "Error encountered while reading penetration chance reduction for bullet type " << bullettypename << " and material " << newdef.materialname << ".";
						m_materialDefinitionsArray.clear();
						return false;
					}

					// Otherwise it is a sound file
					Uint32 newindex = newdef.penetrationinfos.size();
					newdef.penetrationinfos.resize(newdef.penetrationinfos.size() + 1);
					penetration_t& info = newdef.penetrationinfos[newindex];

					info.bullettypeindex = BULLET_TYPE_MAP[j].type;
					info.maxpenetration = maxPenetration;
					info.penetrationdepth = penetrationDepth;
					info.penetrationchance = penetrationChance;
					info.damagefalloff = penetrationDmgFalloff;
					info.chancedecrease = penetrationChanceDecrease;
				}
			}
			else if(!qstrcmp(token, "ricochet"))
			{
				// Read bracket token
				pstr = Common::Parse(pstr, token);
				if(!pstr || token.empty())
				{
					m_errorString << "Unexpected EOF";
					m_materialDefinitionsArray.clear();
					return false;
				}

				if(qstrcmp(token, "{"))
				{
					m_errorString << "Expected '{', got '" << token << "' instead";
					m_materialDefinitionsArray.clear();
					return false;
				}

				while(true)
				{
					// Read bullet type
					pstr = Common::Parse(pstr, token);
					if(!pstr || token.empty())
					{
						m_errorString << "Unexpected EOF";
						m_materialDefinitionsArray.clear();
						return false;
					}

					if(!qstrcmp(token, "}"))
						break;

					CString bullettypename = token;
					Int32 i = 1, j = NO_POSITION; // Skip BULLET_NONE
					while(true)
					{
						if(!BULLET_TYPE_MAP[i].type)
							break;

						if(!qstrcicmp(BULLET_TYPE_MAP[i].name, bullettypename))
						{
							j = i;
							break;
						}

						i++;
					}

					if(j == NO_POSITION)
					{
						m_errorString << "Invalid bullet type " << bullettypename << " specified for material " << newdef.materialname;
						m_materialDefinitionsArray.clear();
						return false;
					}

					// Read bullet ricochet max angle
					Float maxAngle = 0;
					pstr = ReadFloat(pstr, maxAngle, 0);
					if(!pstr)
					{
						m_errorString << "Error encountered while reading ricochet max angle for bullet type " << bullettypename << " and material " << newdef.materialname << ".";
						m_materialDefinitionsArray.clear();
						return false;
					}

					// Read bullet ricochet max angle variation
					Float maxRicochetAngleVariation = 0;
					pstr = ReadFloat(pstr, maxRicochetAngleVariation, 0);
					if(!pstr)
					{
						m_errorString << "Error encountered while reading ricochet max angle variation for bullet type " << bullettypename << " and material " << newdef.materialname << ".";
						m_materialDefinitionsArray.clear();
						return false;
					}

					// Read ricochet chance
					Int32 ricochetChance = 0;
					pstr = ReadInt32(pstr, ricochetChance, 0);
					if(!pstr)
					{
						m_errorString << "Error encountered while reading ricochet max chance for bullet type " << bullettypename << " and material " << newdef.materialname << ".";
						m_materialDefinitionsArray.clear();
						return false;
					}

					// Read max ricochet nb
					Int32 maxRicochetNb = 0;
					pstr = ReadInt32(pstr, maxRicochetNb, 0);
					if(!pstr)
					{
						m_errorString << "Error encountered while reading ricochet max angle for bullet type " << bullettypename << " and material " << newdef.materialname << ".";
						m_materialDefinitionsArray.clear();
						return false;
					}

					// Read ricochet damage falloff
					Float ricochetDmgFalloff = 0;
					pstr = ReadFloat(pstr, ricochetDmgFalloff, 0);
					if(!pstr)
					{
						m_errorString << "Error encountered while reading ricochet ricochet damage falloff for bullet type " << bullettypename << " and material " << newdef.materialname << ".";
						m_materialDefinitionsArray.clear();
						return false;
					}

					// Otherwise it is a sound file
					Uint32 newindex = newdef.ricochetinfos.size();
					newdef.ricochetinfos.resize(newdef.ricochetinfos.size() + 1);
					ricochetinfo_t& info = newdef.ricochetinfos[newindex];

					info.bullettypeindex = BULLET_TYPE_MAP[j].type;
					info.maxangle = sin(maxAngle * (M_PI / 180.0));
					info.maxdeviation = sin(maxRicochetAngleVariation * (M_PI / 180.0));
					info.ricochetchance = ricochetChance;
					info.maxricochets = maxRicochetNb;
					info.ricochetdmgfalloff = ricochetDmgFalloff;
				}
			}
			else if(!qstrcmp(token, "streaksplash"))
			{
				// Read color
				Vector color;
				for(Uint32 i = 0; i < 3; i++)
				{
					pstr = ReadFloat(pstr, color[i], 0);
					if(!pstr)
					{
						m_errorString << "Error encountered while reading streak splash color value for material " << newdef.materialname << ".";
						m_materialDefinitionsArray.clear();
						return false;
					}
				}

				// Read alpha value
				Float alpha = 0;
				pstr = ReadFloat(pstr, alpha, 0);
				if(!pstr)
				{
					m_errorString << "Error encountered while reading streak splash alpha value for material " << newdef.materialname << ".";
					m_materialDefinitionsArray.clear();
					return false;
				}

				// Read width
				Float width = 0;
				pstr = ReadFloat(pstr, width, 0);
				if(!pstr)
				{
					m_errorString << "Error encountered while reading streak splash width value for material " << newdef.materialname << ".";
					m_materialDefinitionsArray.clear();
					return false;
				}

				// Read length
				Float length = 0;
				pstr = ReadFloat(pstr, length, 0);
				if(!pstr)
				{
					m_errorString << "Error encountered while reading streak splash length value for material " << newdef.materialname << ".";
					m_materialDefinitionsArray.clear();
					return false;
				}

				// Read count
				Int32 count = 0;
				pstr = ReadInt32(pstr, count, 0);
				if(!pstr)
				{
					m_errorString << "Error encountered while reading streak splash count for material " << newdef.materialname << ".";
					m_materialDefinitionsArray.clear();
					return false;
				}

				// Read speed
				Float speed = 0;
				pstr = ReadFloat(pstr, speed, 0);
				if(!pstr)
				{
					m_errorString << "Error encountered while reading streak splash speed value for material " << newdef.materialname << ".";
					m_materialDefinitionsArray.clear();
					return false;
				}

				// Read minimum lifetime
				Float minLifetime = 0;
				pstr = ReadFloat(pstr, minLifetime, 0);
				if(!pstr)
				{
					m_errorString << "Error encountered while reading streak splash minimum life value for material " << newdef.materialname << ".";
					m_materialDefinitionsArray.clear();
					return false;
				}

				// Read maximum lifetime
				Float maxLifetime = 0;
				pstr = ReadFloat(pstr, maxLifetime, 0);
				if(!pstr)
				{
					m_errorString << "Error encountered while reading streak splash maximum life value for material " << newdef.materialname << ".";
					m_materialDefinitionsArray.clear();
					return false;
				}

				// Read minimum velocity
				Float minVelocity = 0;
				pstr = ReadFloat(pstr, minVelocity, 0);
				if(!pstr)
				{
					m_errorString << "Error encountered while reading streak splash minimum velocity value for material " << newdef.materialname << ".";
					m_materialDefinitionsArray.clear();
					return false;
				}

				// Read maximum velocity
				Float maxVelocity = 0;
				pstr = ReadFloat(pstr, maxVelocity, 0);
				if(!pstr)
				{
					m_errorString << "Error encountered while reading streak splash maximum velocity value for material " << newdef.materialname << ".";
					m_materialDefinitionsArray.clear();
					return false;
				}

				// Set params in definition
				streaksplashparams_t streaksplash;
				streaksplash.color = color;
				streaksplash.alpha = alpha;
				streaksplash.width = width;
				streaksplash.length = length;
				streaksplash.count = count;
				streaksplash.speed = speed;
				streaksplash.minlife = minLifetime;
				streaksplash.maxlife = maxLifetime;
				streaksplash.minvelocity = minVelocity;
				streaksplash.maxvelocity = maxVelocity;

				newdef.streaksplashes.push_back(streaksplash);
			}
			else
			{
				// Read field name
				CString value;
				pstr = Common::Parse(pstr, value);
				if(!pstr || value.empty())
				{
					m_errorString << "Unexpected EOF";
					m_materialDefinitionsArray.clear();
					return false;
				}
				
				if(!qstrcmp(token, "decalgroup"))
				{
					// Set decal group
					newdef.decalgroup = value;
				}
				else if(!qstrcmp(token, "scripttype"))
				{
					if(!qstrcmp(value, "cluster"))
						newdef.scripttype = PART_SCRIPT_CLUSTER;
					else if(!qstrcmp(value, "system"))
						newdef.scripttype = PART_SCRIPT_SYSTEM;
					else
					{
						m_errorString << "Unknown script type '" << value << "' specified for '" << newdef.materialname;
						m_materialDefinitionsArray.clear();
						return false;
					}
				}
				else if(!qstrcmp(token, "particlescript"))
				{
					// set material script
					newdef.particlescript = value;
				}
				else
				{
					m_errorString << "Unknown option '" << token << "' specified for '" << newdef.materialname;
					m_materialDefinitionsArray.clear();
					return false;
				}
			}
		}

		// Set final sizes
		newdef.penetrationinfos.resize(newdef.penetrationinfos.size());
		newdef.ricochetinfos.resize(newdef.ricochetinfos.size());

		// Add to the list
		m_materialDefinitionsArray.push_back(newdef);
	}

	// Make sure "default" is defined
	Uint32 i = 0;
	for(; i < m_materialDefinitionsArray.size(); i++)
	{
		if(!qstrcmp(m_materialDefinitionsArray[i].materialname, "default"))
			break;
	}

	if(i == m_materialDefinitionsArray.size())
	{
		m_errorString << "Material type 'default' is not defined in script file";
		return false;
	}

	return true;
}

//=============================================
// @brief
//
//=============================================
const Char* CMaterialDefinitions::ReadInt32( const Char* pstrString, Int32& outValue, Int32 minValue )
{
	// Read ricochet damage falloff
	CString token;
	const Char* pstr = Common::Parse(pstrString, token);
	if(!pstr || token.empty())
	{
		m_errorString << "Unexpected EOF while reading material script.";
		m_materialDefinitionsArray.clear();
		return nullptr;
	}

	if(!qstrcmp(token, "}"))
	{
		m_errorString << "Unexpected '}' token when reading material script.";
		m_materialDefinitionsArray.clear();
		return nullptr;
	}

	if(!Common::IsNumber(token))
	{
		m_errorString << "Value read is not a number.";
		m_materialDefinitionsArray.clear();
		return nullptr;
	}

	Int32 readValue = SDL_atoi(token.c_str());
	if(minValue != -1 && readValue < minValue)
	{
		m_errorString << "Value read is below minimum treshold " << minValue << ".";
		m_materialDefinitionsArray.clear();
		return nullptr;
	}
	else
	{
		outValue = readValue;
		return pstr;
	}
}

//=============================================
// @brief
//
//=============================================
const Char* CMaterialDefinitions::ReadFloat( const Char* pstrString, Float& outValue, Float minValue )
{
	// Read ricochet damage falloff
	CString token;
	const Char* pstr = Common::Parse(pstrString, token);
	if(!pstr || token.empty())
	{
		m_errorString << "Unexpected EOF while reading material script.";
		m_materialDefinitionsArray.clear();
		return nullptr;
	}

	if(!qstrcmp(token, "}"))
	{
		m_errorString << "Unexpected '}' token when reading material script.";
		m_materialDefinitionsArray.clear();
		return nullptr;
	}

	if(!Common::IsNumber(token))
	{
		m_errorString << "Value read is not a number.";
		m_materialDefinitionsArray.clear();
		return nullptr;
	}

	Float readValue = SDL_atof(token.c_str());
	if(minValue != -1 && readValue < minValue)
	{
		m_errorString << "Value read is below minimum treshold " << minValue << ".";
		m_materialDefinitionsArray.clear();
		return nullptr;
	}
	else
	{
		outValue = readValue;
		return pstr;
	}
}

//=============================================
// @brief
//
//=============================================
void CMaterialDefinitions::Clear( void )
{
	if(m_materialDefinitionsArray.empty())
		return;

	m_materialDefinitionsArray.clear();
}


//=============================================
// @brief
//
//=============================================
const CMaterialDefinitions::materialdefinition_t* CMaterialDefinitions::GetDefinitionForMaterial( const Char* pstrmaterialname ) const
{
	// Try to find the effect for this material type
	for(Uint32 i = 0; i < m_materialDefinitionsArray.size(); i++)
	{
		if(!qstrcmp(m_materialDefinitionsArray[i].materialname, pstrmaterialname))
			return &m_materialDefinitionsArray[i];
	}

	// Just use default then
	for(Uint32 i = 0; i < m_materialDefinitionsArray.size(); i++)
	{
		if(!qstrcmp(m_materialDefinitionsArray[i].materialname, "default"))
			return &m_materialDefinitionsArray[i];
	}

	return nullptr;
}

//=============================================
// @brief
//
//=============================================
Uint32 CMaterialDefinitions::GetNbDefinitions( void ) const
{
	return m_materialDefinitionsArray.size();
}

//=============================================
// @brief
//
//=============================================
const CMaterialDefinitions::materialdefinition_t* CMaterialDefinitions::GetDefinition( Uint32 index ) const
{
	assert(index < m_materialDefinitionsArray.size());
	return &m_materialDefinitionsArray[index];
}