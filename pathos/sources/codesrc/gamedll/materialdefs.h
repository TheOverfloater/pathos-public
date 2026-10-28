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
			bullettypeindex(0),
			penetrationdepth(0),
			maxpenetration(0),
			penetrationchance(0),
			damagefalloff(0),
			chancedecrease(0)
			{
			}

		Int32 bullettypeindex;
		Float penetrationdepth;
		Uint32 maxpenetration;
		Uint32 penetrationchance;
		Float damagefalloff;
		Uint32 chancedecrease;
	};

	struct ricochetinfo_t
	{
		ricochetinfo_t():
			bullettypeindex(0),
			maxangle(0),
			maxdeviation(0),
			ricochetchance(0),
			maxricochets(0),
			ricochetdmgfalloff(0)
			{}

		Int32 bullettypeindex;
		Float maxangle;
		Float maxdeviation;
		Uint32 ricochetchance;
		Uint32 maxricochets;
		Float ricochetdmgfalloff;
	};

	struct streaksplashparams_t
	{
		streaksplashparams_t():
			alpha(0),
			width(0),
			length(0),
			count(0),
			speed(0),
			minlife(0),
			maxlife(0),
			minvelocity(0),
			maxvelocity(0)
		{}

		Vector color;
		Float alpha;
		Float width;
		Float length;
		Uint32 count;
		Float speed;
		Float minlife;
		Float maxlife;
		Float minvelocity;
		Float maxvelocity;
	};

	struct materialdefinition_t
	{
		materialdefinition_t():
			scripttype(PART_SCRIPT_SYSTEM)
			{
			}

		const penetration_t* getPenetrationInfo(bullet_types_t bulletType) const
		{
			for (Uint32 i = 0; i < penetrationinfos.size(); i++)
			{
				if (penetrationinfos[i].bullettypeindex == bulletType)
					return &penetrationinfos[i];
			}

			return nullptr;
		}

		const ricochetinfo_t* getRicochetInfo(bullet_types_t bulletType) const
		{
			for (Uint32 i = 0; i < ricochetinfos.size(); i++)
			{
				if (ricochetinfos[i].bullettypeindex == bulletType)
					return &ricochetinfos[i];
			}

			return nullptr;
		}

		CString materialname;
		CString decalgroup;
		CString particlescript;
		part_script_type_t scripttype;
		CArray<CString> sounds;
		CArray<penetration_t> penetrationinfos;
		CArray<ricochetinfo_t> ricochetinfos;
		CArray<streaksplashparams_t> streaksplashes;
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

	// Read integer field
	const Char* ReadInt32( const Char* pstrString, Int32& outValue, Int32 minValue = -1 );
	// Read float field
	const Char* ReadFloat( const Char* pstrString, Float& outValue, Float minValue = -1 );

private:
	// Array of impact effects for materials
	CArray<materialdefinition_t> m_materialDefinitionsArray;
	// Error string
	CString m_errorString;
};
extern CMaterialDefinitions gMaterialDefinitions;
#endif //IMPACTEFFECTS_H