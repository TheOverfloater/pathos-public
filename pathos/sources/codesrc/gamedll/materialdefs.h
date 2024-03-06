/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef IMPACTEFFECTS_H
#define IMPACTEFFECTS_H

#include "constants.h"

/*
====================
CMaterialDefinitions

====================
*/
class CMaterialDefinitions
{
public:
	struct penetration_t
	{
		penetration_t():
			penetrationdepth(0),
			maxpenetration(0),
			penetrationchance(0),
			damagefalloff(0),
			chancedecrease(0)
			{
			}

		Float penetrationdepth;
		Uint32 maxpenetration;
		Uint32 penetrationchance;
		Float damagefalloff;
		Uint32 chancedecrease;
	};

	struct ricochetinfo_t
	{
		ricochetinfo_t():
			maxangle(0),
			maxdeviation(0),
			ricochetchance(0),
			maxricochets(0),
			ricochetdmgfalloff(0)
			{}

		Float maxangle;
		Float maxdeviation;
		Uint32 ricochetchance;
		Uint32 maxricochets;
		Float ricochetdmgfalloff;
	};

	struct materialdefinition_t
	{
		materialdefinition_t():
			scripttype(PART_SCRIPT_SYSTEM)
			{
			}

		CString materialname;
		CString decalgroup;
		CString particlescript;
		part_script_type_t scripttype;
		CArray<CString> sounds;
		CArray<penetration_t> penetrationinfos;
		CArray<ricochetinfo_t> ricochetinfos;
	};

public:
	CMaterialDefinitions( void );
	~CMaterialDefinitions( void );

public:
	// Initializes the class
	bool Init( const Char* pscriptfile );
	// Clears the class
	void Clear( void );

	// Gets the impact effect info for a material type
	const materialdefinition_t* GetDefinitionForMaterial( const Char* pstrmaterialname ) const;
	// Returns the error string
	const Char* GetError( void ) const { return m_errorString.c_str(); }

	// Returns the number of effect definitions
	Uint32 GetNbDefinitions( void ) const;
	// Returns an effect definition
	const materialdefinition_t* GetDefinition( Uint32 index ) const;

private:
	// Array of impact effects for materials
	CArray<materialdefinition_t> m_materialDefinitionsArray;
	// Error string
	CString m_errorString;
};
extern CMaterialDefinitions gMaterialDefinitions;
#endif //IMPACTEFFECTS_H