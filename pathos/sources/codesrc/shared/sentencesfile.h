/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef SENTENCESFILE_H
#define SENTENCESFILE_H

//=============================================
//
//=============================================
class CSentencesFile
{
public:
	typedef Int32 (*pfnPrecacheSoundFunctionPtr_t)( const Char* pstrFilepath );
	typedef Float (*pfnGetSoundDurationFunctionPtr_t)( const Char* pstrfilename, Uint32 pitch );

public:
	// Comma delay amount
	static const Float COMMA_DELAY;
	// Default folder for sentences
	static const Char DEFAULT_FOLDER[];

public:
	struct sentencegroup_t
	{
		sentencegroup_t():
			numsentences(0),
			startindex(0),
			duration(-1)
			{
			}

		CString groupname;
		Uint32 numsentences;
		Uint32 startindex;
		Float duration;
	};

	struct sent_chunk_t
	{
		sent_chunk_t():
			pitch(0),
			time(0),
			start(0),
			end(0),
			volume(0),
			delay(0),
			pnext(nullptr)
			{
			}

		CString soundname;
		Uint32 pitch;
		Uint32 time;
		Uint32 start;
		Uint32 end;
		Uint32 volume;
		Float delay;

		sent_chunk_t* pnext;
	};

	struct sentence_t
	{
		sentence_t():
			index(0),
			duration(-1)
		{
		}
		~sentence_t()
		{
			for(Uint32 i = 0; i < chunks.size(); i++)
				delete chunks[i];
		}

		CString name;
		CString entryname;
		Uint32 index;
		Float duration;

		CString folder;
		CArray<sent_chunk_t*> chunks;
	};

public:
	CSentencesFile( void );
	explicit CSentencesFile( pfnPrecacheSoundFunctionPtr_t pfnPrecacheSound, pfnGetSoundDurationFunctionPtr_t pfnGetSoundDuration );
	~CSentencesFile( void );

public:
	bool Init( const byte* pfile );
	void Clear( void );

	const Char* GetRandomSentence( const Char* pstrGroupName, Float* pDuration );
	const Char* GetSentence( const Char* pstrSentenceName, Float* pDuration );

	const sentence_t* GetSentenceDefinition( const Char* pstrSentenceName ) const;
	const sentence_t* GetSentenceDefinition( Int32 index ) const;
	Float GetSentenceDuration( const Char* pstrSentenceName );
	Float GetSentenceDuration( Int32 index );

	void PrecacheGroup( const Char* pstrGroupName );
	void PrecacheSentence( const Char* pstrSentenceName );
	void PrecacheSentence( sentence_t* psentence );

public:
	bool HasWarnings( void ) const;
	const Char* GetWarning( Uint32 index );
	Uint32 GetNbWarnings( void ) const;

	bool HasError( void ) const;
	const Char* GetError( void ) const;

private:
	bool ParseOptionToken( const Char** ppoutstr, const Char* pstroption, Uint32& pitch, Uint32& time, Uint32& start, Uint32& end, Uint32& volume );
	sentence_t* GetSentenceDefinition( const Char* pstrSentenceName );

private:
	// Array of sentence groups
	CArray<sentencegroup_t*> m_sentenceGroupsArray;
	// Array of sentences
	CArray<sentence_t*> m_sentencesArray;

	// Array of warning messages
	CArray<CString> m_warningMessagesArray;
	// Error message if any
	CString m_errorMessage;

private:
	// Precache sound function ptr
	pfnPrecacheSoundFunctionPtr_t m_pfnPrecacheSound;
	// Get sound duration function ptr
	pfnGetSoundDurationFunctionPtr_t m_pfnGetSoundDuration;
};
#endif //SENTENCESFILE_H