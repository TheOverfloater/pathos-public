/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef STEPSOUND_H
#define STEPSOUND_H

/*
=======================
CStepSound

=======================
*/
class CStepSound
{
public:
	enum foot_t
	{
		FOOT_RIGHT = 0,
		FOOT_LEFT
	};

private:
	struct footstepinfo_t
	{
		footstepinfo_t():
			type(DEFAULT_FOOTSTEP_TYPE)
			{
			}

		// Material this info is tied to
		CString materialname;
		// Type identifier
		CString type;

		// Right foot step sounds
		CArray<CString> rightarray;
		// Left foot step sounds
		CArray<CString> leftarray;
	};


public:
	// Default footstep type name
	static const Char DEFAULT_FOOTSTEP_TYPE[];
	// Default material type name
	static const Char DEFAULT_MATERIAL_TYPE[];
	// Concrete material type name
	static const Char CONCRETE_MATERIAL_TYPE[];

public:
	CStepSound( void );
	~CStepSound( void );

public:
	// Loads script for footsteps
	bool Init( const Char* pstrFile, Uint32 filesize );
	// Precaches the sounds used
	void GetPrecacheList( CArray<CString>& outList );
	// Retreives a footstep for a material type
	const CArray<CString>* GetFootSoundList( foot_t foot, const Char* pmaterialname, const Char* psteptype = nullptr );
	// Tells if the specified material type exists
	bool IsMaterialTypeValid( const Char* pmaterialname );

	// Returns the info string
	const bool IsInfoStringEmpty( void ) const { return m_strInfoString.empty(); }
	// Returns the info string
	const CString& GetInfoString( void ) const { return m_strInfoString; }

	// Returns material by index
	const footstepinfo_t* GetMaterialByIndex( Uint32 index );
	// Returns the number of materials
	Uint32 GetNbMaterials( void );

private:
	// Compares types
	static bool CheckType( const Char* pstrType, const Char* pstrCheckType );
	// Compares materials
	static bool CheckMaterial( const Char* pstrMaterial, const Char* pstrCheckMaterial );

private:
	// Array of footstep sounds
	CArray<footstepinfo_t> m_footStepInfoArray;
	// Defautl footstep type
	footstepinfo_t* m_pDefaultFootstepInfo;
	// Error string if any
	CString m_strInfoString;
};

#endif