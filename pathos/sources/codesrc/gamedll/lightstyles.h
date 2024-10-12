/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef LIGHTSTYLES_H
#define LIGHTSTYLES_H

//=============================================
//
//=============================================
class CLightStyles
{
public:
	CLightStyles( void );
	virtual ~CLightStyles( void );

public:
	// start lightstyle index for dynlights
	static const Uint32 CUSTOM_LIGHTSTYLE_START_INDEX;
	// Default framerate value
	static const Float DEFAULT_LIGHTSTYLE_FRAMERATE;

public:
	struct lightstyle_t
	{
		lightstyle_t():
			interpolate(false),
			styleindex(0),
			framerate(0)
			{
			}

		CString pattern;
		bool interpolate;
		Uint32 styleindex;
		Float framerate;
	};

public:
	void ResetStyles( void );
	void SendLightStylesToClient( edict_t* pPlayer );
	void Think( void );

	void SetLightStyle( const Char* pstrpattern, bool interpolate, Float framerate, Uint32 styleindex );
	Int32 AddCustomLightStyle( const Char* pstrpattern, bool interpolate, Float framerate );
	CArray<Float>* GetLightStyleValuesArray( void );

private:
	// Array of custom lightstyles
	CArray<lightstyle_t> m_lightStylesArray;
	// Next available lightstyle index
	Uint32 m_nextLightStyleIndex;

	// Lightstyle values array
	CArray<Float> m_lightStyleValuesArray;
};
extern CLightStyles gSVLightStyles;
#endif //LIGHTSTYLES_H