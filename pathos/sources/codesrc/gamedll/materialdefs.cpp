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
					pstr = Common::Parse(pstr, token);
					if(!pstr || token.empty())
					{
						m_errorString << "Unexpected EOF";
						m_materialDefinitionsArray.clear();
						return false;
					}

					if(!qstrcmp(token, "}"))
					{
						m_errorString << "Unexpected '}' token when reading bullet type " << bullettypename << " definition for material " << newdef.materialname;
						m_materialDefinitionsArray.clear();
						return false;
					}

					if(!Common::IsNumber(token))
					{
						m_errorString << "Penetration depth " << token << " for bullet type " << bullettypename << " and material " << newdef.materialname << " is not a number";
						m_materialDefinitionsArray.clear();
						return false;
					}

					Float penetrationDepth = SDL_atof(token.c_str());
					if(penetrationDepth < 0)
					{
						m_errorString << "Invalid penetration depth " << penetrationDepth << " specified for bullet type " << bullettypename << " and material " << newdef.materialname << " specified";
						m_materialDefinitionsArray.clear();
						return false;
					}

					// Read penetration max count
					pstr = Common::Parse(pstr, token);
					if(!pstr || token.empty())
					{
						m_errorString << "Unexpected EOF while reading bullet type " << bullettypename << " for material " << newdef.materialname;
						m_materialDefinitionsArray.clear();
						return false;
					}

					if(!qstrcmp(token, "}"))
					{
						m_errorString << "Unexpected '}' token when reading bullet type " << bullettypename << " definition for material " << newdef.materialname;
						m_materialDefinitionsArray.clear();
						return false;
					}

					if(!Common::IsNumber(token))
					{
						m_errorString << "Penetration maximum " << token << " for bullet type " << bullettypename << " and material " << newdef.materialname << " is not a number";
						m_materialDefinitionsArray.clear();
						return false;
					}

					Uint32 maxPenetration = SDL_atoi(token.c_str());

					// Read penetration chance
					pstr = Common::Parse(pstr, token);
					if(!pstr || token.empty())
					{
						m_errorString << "Unexpected EOF while reading bullet type " << bullettypename << " for material " << newdef.materialname;
						m_materialDefinitionsArray.clear();
						return false;
					}

					if(!qstrcmp(token, "}"))
					{
						m_errorString << "Unexpected '}' token when reading bullet type " << bullettypename << " definition for material " << newdef.materialname;
						m_materialDefinitionsArray.clear();
						return false;
					}

					if(!Common::IsNumber(token))
					{
						m_errorString << "Penetration chance " << token << " for bullet type " << bullettypename << " and material " << newdef.materialname << " is not a number";
						m_materialDefinitionsArray.clear();
						return false;
					}

					Uint32 penetrationChance = SDL_atoi(token.c_str());

					// Read penetration damage falloff
					pstr = Common::Parse(pstr, token);
					if(!pstr || token.empty())
					{
						m_errorString << "Unexpected EOF while reading bullet type " << bullettypename << " for material " << newdef.materialname;
						m_materialDefinitionsArray.clear();
						return false;
					}

					if(!qstrcmp(token, "}"))
					{
						m_errorString << "Unexpected '}' token when reading bullet type " << bullettypename << " definition for material " << newdef.materialname;
						m_materialDefinitionsArray.clear();
						return false;
					}

					if(!Common::IsNumber(token))
					{
						m_errorString << "Penetration damage falloff " << token << " for bullet type " << bullettypename << " and material " << newdef.materialname << " is not a number";
						m_materialDefinitionsArray.clear();
						return false;
					}

					Float penetrationDmgFalloff = SDL_atof(token.c_str());
					if(penetrationDmgFalloff < 0)
					{
						m_errorString << "Invalid penetration damage falloff value " << penetrationDmgFalloff << " specified for bullet type " << bullettypename << " and material " << newdef.materialname << " specified";
						m_materialDefinitionsArray.clear();
						return false;
					}

					// Read penetration chance decrease
					pstr = Common::Parse(pstr, token);
					if(!pstr || token.empty())
					{
						m_errorString << "Unexpected EOF while reading bullet type " << bullettypename << " for material " << newdef.materialname;
						m_materialDefinitionsArray.clear();
						return false;
					}

					if(!qstrcmp(token, "}"))
					{
						m_errorString << "Unexpected '}' token when reading bullet type " << bullettypename << " definition for material " << newdef.materialname;
						m_materialDefinitionsArray.clear();
						return false;
					}

					if(!Common::IsNumber(token))
					{
						m_errorString << "Penetration damage falloff " << token << " for bullet type " << bullettypename << " and material " << newdef.materialname << " is not a number";
						m_materialDefinitionsArray.clear();
						return false;
					}

					Uint32 penetrationChanceDecrease = SDL_atoi(token.c_str());
					if(penetrationChanceDecrease < 0)
					{
						m_errorString << "Invalid penetration chance decrease value " << penetrationDmgFalloff << " specified for bullet type " << bullettypename << " and material " << newdef.materialname << " specified";
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
					pstr = Common::Parse(pstr, token);
					if(!pstr || token.empty())
					{
						m_errorString << "Unexpected EOF";
						m_materialDefinitionsArray.clear();
						return false;
					}

					if(!qstrcmp(token, "}"))
					{
						m_errorString << "Unexpected '}' token when reading bullet type ricochet " << bullettypename << " definition for material " << newdef.materialname;
						m_materialDefinitionsArray.clear();
						return false;
					}

					if(!Common::IsNumber(token))
					{
						m_errorString << "Ricochet max angle " << token << " for bullet type " << bullettypename << " and material " << newdef.materialname << " is not a number";
						m_materialDefinitionsArray.clear();
						return false;
					}

					Float maxAngle = SDL_atof(token.c_str());
					if(maxAngle < 0)
					{
						m_errorString << "Invalid ricochet max angle " << maxAngle << " specified for bullet type " << bullettypename << " and material " << newdef.materialname << " specified";
						m_materialDefinitionsArray.clear();
						return false;
					}

					// Read max ricochet angle variation
					pstr = Common::Parse(pstr, token);
					if(!pstr || token.empty())
					{
						m_errorString << "Unexpected EOF while reading bullet type ricochet " << bullettypename << " for material " << newdef.materialname;
						m_materialDefinitionsArray.clear();
						return false;
					}

					if(!qstrcmp(token, "}"))
					{
						m_errorString << "Unexpected '}' token when reading bullet type ricochet " << bullettypename << " definition for material " << newdef.materialname;
						m_materialDefinitionsArray.clear();
						return false;
					}

					if(!Common::IsNumber(token))
					{
						m_errorString << "Ricochet maximum angle variation  value" << token << " for bullet type " << bullettypename << " and material " << newdef.materialname << " is not a number";
						m_materialDefinitionsArray.clear();
						return false;
					}

					Float maxRicochetAngleVariation = SDL_atof(token.c_str());
					if(maxRicochetAngleVariation < 0)
					{
						m_errorString << "Invalid ricochet max angle variation value " << maxRicochetAngleVariation << " specified for bullet type " << bullettypename << " and material " << newdef.materialname << " specified";
						m_materialDefinitionsArray.clear();
						return false;
					}

					// Read ricochet chance
					pstr = Common::Parse(pstr, token);
					if(!pstr || token.empty())
					{
						m_errorString << "Unexpected EOF while reading bullet type ricochet " << bullettypename << " for material " << newdef.materialname;
						m_materialDefinitionsArray.clear();
						return false;
					}

					if(!qstrcmp(token, "}"))
					{
						m_errorString << "Unexpected '}' token when reading bullet type ricochet " << bullettypename << " definition for material " << newdef.materialname;
						m_materialDefinitionsArray.clear();
						return false;
					}

					if(!Common::IsNumber(token))
					{
						m_errorString << "Ricochet chance value " << token << " for bullet type ricochet " << bullettypename << " and material " << newdef.materialname << " is not a number";
						m_materialDefinitionsArray.clear();
						return false;
					}

					Uint32 ricochetChance = SDL_atoi(token.c_str());
					if(ricochetChance < 0)
					{
						m_errorString << "Invalid ricochet chance value " << ricochetChance << " specified for bullet type " << bullettypename << " and material " << newdef.materialname << " specified";
						m_materialDefinitionsArray.clear();
						return false;
					}

					// Read max ricochet nb
					pstr = Common::Parse(pstr, token);
					if(!pstr || token.empty())
					{
						m_errorString << "Unexpected EOF while reading bullet type ricochet " << bullettypename << " for material " << newdef.materialname;
						m_materialDefinitionsArray.clear();
						return false;
					}

					if(!qstrcmp(token, "}"))
					{
						m_errorString << "Unexpected '}' token when reading bullet type ricochet " << bullettypename << " definition for material " << newdef.materialname;
						m_materialDefinitionsArray.clear();
						return false;
					}

					if(!Common::IsNumber(token))
					{
						m_errorString << "Ricochet max nb value " << token << " for bullet type ricochet " << bullettypename << " and material " << newdef.materialname << " is not a number";
						m_materialDefinitionsArray.clear();
						return false;
					}

					Uint32 maxRicochetNb = SDL_atoi(token.c_str());
					if(maxRicochetNb < 0)
					{
						m_errorString << "Invalid ricochet max nb value " << maxRicochetNb << " specified for bullet type " << bullettypename << " and material " << newdef.materialname << " specified";
						m_materialDefinitionsArray.clear();
						return false;
					}

					// Read ricochet damage falloff
					pstr = Common::Parse(pstr, token);
					if(!pstr || token.empty())
					{
						m_errorString << "Unexpected EOF while reading bullet type ricochet " << bullettypename << " for material " << newdef.materialname;
						m_materialDefinitionsArray.clear();
						return false;
					}

					if(!qstrcmp(token, "}"))
					{
						m_errorString << "Unexpected '}' token when reading bullet type ricochet " << bullettypename << " definition for material " << newdef.materialname;
						m_materialDefinitionsArray.clear();
						return false;
					}

					if(!Common::IsNumber(token))
					{
						m_errorString << "Ricochet damage falloff value" << token << " for bullet type " << bullettypename << " and material " << newdef.materialname << " is not a number";
						m_materialDefinitionsArray.clear();
						return false;
					}

					Float ricochetDmgFalloff = SDL_atof(token.c_str());
					if(ricochetDmgFalloff < 0)
					{
						m_errorString << "Invalid ricochet damage falloff value " << maxRicochetAngleVariation << " specified for bullet type " << bullettypename << " and material " << newdef.materialname << " specified";
						m_materialDefinitionsArray.clear();
						return false;
					}

					// Otherwise it is a sound file
					Uint32 newindex = newdef.ricochetinfos.size();
					newdef.ricochetinfos.resize(newdef.ricochetinfos.size() + 1);
					ricochetinfo_t& info = newdef.ricochetinfos[newindex];

					info.bullettypeindex = BULLET_TYPE_MAP[j].type;
					info.maxangle = sin( maxAngle * (M_PI / 180.0));
					info.maxdeviation = sin( maxRicochetAngleVariation * (M_PI / 180.0));
					info.ricochetchance = ricochetChance;
					info.maxricochets = maxRicochetNb;
					info.ricochetdmgfalloff = ricochetDmgFalloff;
				}
			}
			else
			{
				// Read list of sounds
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