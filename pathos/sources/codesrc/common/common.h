/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#ifndef COMMON_H
#define COMMON_H

class Vector;
template <typename T> class CArray;

#ifndef MAX_PATH
#define MAX_PATH			260
#endif //MAX_PATH

#ifndef TRUE
#define TRUE				1
#endif //TRUE

#ifndef FALSE
#define FALSE				0
#endif //FALSE

#ifndef M_PI
#define M_PI		3.14159265358979323846	// matches value in gcc v2 math.h
#endif //M_PI

#ifndef DEG2RAD
#define DEG2RAD( a ) ( a * M_PI ) / 180.0F
#endif //DEG2RAD

#ifndef RAD2DEG
#define RAD2DEG( a ) ( a * 180.0F ) / M_PI
#endif //RAD2DEG

#ifndef clamp
#define clamp( val, min, max ) ( ((val) > (max)) ? (max) : ( ((val) < (min)) ? (min) : (val) ) )
#endif //clamp

#ifndef _max
#define _max( max, val ) ( ((val) > (max)) ? (max) : (val) )
#endif //max

#ifndef _min
#define _min( min, val ) ( ((val) < (min)) ? (min) : (val) )
#endif //min

#ifndef DIST_EPSILON
#define DIST_EPSILON 0.03125f
#endif //DIST_EPSILON

// For SDL_GetTicks
#ifndef SECONDS_TO_MILLISECONDS
#define SECONDS_TO_MILLISECONDS 1000
#endif //SECONDS_TO_MILLISECONDS

// For converting seconds to milliseconds
#ifndef MILLISECONDS_TO_SECONDS
#define MILLISECONDS_TO_SECONDS 1.0f/1000.0f
#endif //MILLISECONDS_TO_SECONDS

#ifndef force_assert
#define force_assert(_Expression) (void)( (!!(_Expression)) || (_wassert(_CRT_WIDE(#_Expression), _CRT_WIDE(__FILE__), __LINE__), 0) )
#endif

// NAN mask
static constexpr Int32 NANMASK = 0x7F800000;

// Valid slash used in paths
static constexpr Char PATH_SLASH_CHAR = '/';

// Maximum line length
static constexpr Uint32 MAX_LINE_LENGTH = 4096;
// Maximum parse length
static constexpr Uint32 MAX_PARSE_LENGTH = 256;

// Resource levels
enum rs_level_t
{
	RS_LEVEL_UNDEFINED = 0,
	RS_GAME_LEVEL,				// Resources that get freed whenever a game reloads
	RS_WINDOW_LEVEL,			// Resources that get freed only when the window is destroyed
	RS_APP_LEVEL				// Resources that get freed only when the application exits
};

// Resource message types
enum rs_msg_type_t
{
	RS_FINISHED = 0,
	RS_LIST_MODELS,
	RS_LIST_SOUNDS,
	RS_LIST_TEXTURES,
	RS_LIST_MATERIAL_SCRIPTS,
	RS_LIST_GENERIC,
	RS_LIST_PARTICLE_SCRIPTS,
	RS_LIST_DECALS
};

// Resource type
enum rs_type_t
{
	RS_TYPE_UNDEFINED = -1,
	RS_TYPE_MODEL = 0,
	RS_TYPE_SOUND,
	RS_TYPE_TEXTURE,
	RS_TYPE_MATERIAL_SCRIPT,
	RS_TYPE_GENERIC,
	RS_TYPE_PARTICLE_SCRIPT,
	RS_TYPE_DECAL
};

// Resources cls msg types
enum rs_clmsg_t
{
	CL_RESOURCE_LIST = 0,
	CL_RESOURCE_FILEINFO,
	CL_RESOURCE_FILECHUNK,
	CL_RESOURCE_UNAVAILABLE
};

enum rs_svmsg_t
{
	SV_RESOURCES_OK = 0,
	SV_RESOURCE_DOWNLOAD_BEGIN,
	SV_RESOURCE_CHUNK_RECEIVED
};

enum wheel_type_t
{
	MOUSE_WHEEL_UP = 0,
	MOUSE_WHEEL_DOWN,
	MOUSE_WHEEL_RIGHT,
	MOUSE_WHEEL_LEFT
};

struct file_dateinfo_t
{
	file_dateinfo_t():
		year(0),
		month(0),
		day(0),
		hour(0),
		minute(0),
		second(0)
		{
		}

	Int32 year;
	Int32 month;
	Int32 day;
	Int32 hour;
	Int32 minute;
	Int32 second;
};

// up / down
#ifndef PITCH
#define	PITCH	0
#endif // PITCH

// left / right
#ifndef YAW
#define	YAW		1
#endif // YAW

// fall over
#ifndef ROLL
#define	ROLL	2 
#endif // ROLL

#ifndef PT_ARRAYSIZE
#define PT_ARRAYSIZE(ptr) (sizeof(ptr)/sizeof(ptr[0]))
#endif

class CString;

extern inline Uint32 qstrlen( const Char* pstr );
extern inline Char* qstrcpy( Char* pdest, const Char *psrc );
extern inline Char* qstrncpy( Char* pdest, const Char *psrc, Uint32 size );
extern Int32 qstrcmp( const Char* pstr1, const Char* pstr2 );
extern Int32 qstrcmp( const CString& str1, const CString& str2 );
extern Int32 qstrcmp( const Char* pstr1, const CString& str2 );
extern Int32 qstrcmp( const CString& str1, const Char* pstr2 );
extern Int32 qstrcicmp( const Char* pstr1, const Char* pstr2 );
extern Int32 qstrcicmp( const CString& str1, const CString& str2 );
extern Int32 qstrcicmp( const Char* pstr1, const CString& str2 );
extern Int32 qstrcicmp( const CString& str1, const Char* pstr2 );
extern Int32 qstrncmp( const Char* pstr1, const Char* pstr2, Uint32 n );
extern Int32 qstrncmp( const CString& str1, const CString& str2, Uint32 n );
extern Int32 qstrncmp( const Char* pstr1, const CString& str2, Uint32 n );
extern Int32 qstrncmp( const CString& str1, const Char* pstr2, Uint32 n );
extern void qstrins( const Char* psrc, Char *pdest, Uint32 offset );
extern inline const Char* qstrstr( const Char* pstr, const Char *psubstr );

extern inline Float qrsqrt( Float value );
extern inline Float sgn(Float a);

namespace Common
{
	extern const Char* Parse( const Char *pstr, Char* pdest, const Char* pbreakchars = nullptr, bool ignoreComma = false, bool checkCurlyBrackets = false );
	extern const Char* Parse( const Char *pstr, CString& str, const Char* pbreakchars = nullptr, bool ignoreComma = false, bool checkCurlyBrackets = false );
	extern const Char* ReadLine( const Char* pstr, Char* pdest );
	extern const Char* ReadLine( const Char* pstr, CString& str );
	extern void *ResizeArray( void *parray, Uint64 size, Uint64 count, Uint64 countNew = 1 );
	extern void Basename( const Char *pstrin, Char *pstrout );
	extern void Basename( const Char *pstrin, CString& strOut );
	extern bool IsNumber( const Char *pstr );
	extern bool IsNumber( const CString& str );
	extern Char GetShiftedChar( Char c );
	extern void StringToVector( const Char* pstrString, Vector& outVector );
	extern void NormalizeAngles( Vector& angles );
	extern void ConvertStringToLowerCase( Char* pstring );
	extern Uint32 GetStringCRC32Hash( const Char* pstrString );

	extern inline Int16 ByteToInt16( const byte *pdata );
	extern inline Uint16 ByteToUint16( const byte *pdata );
	extern inline Int32 ByteToInt32( const byte *pdata );
	extern inline Uint32 ByteToUint32( const byte *pdata );
	extern inline Float ByteToFloat( const byte *pdata );
	extern inline Int64 ByteToInt64( const byte *pdata );
	extern inline Uint64 ByteToUint64( const byte *pdata );
	extern inline Double ByteToDouble( const byte *pdata );

	extern bool IsPowerOfTwo( Uint32 size );
	extern inline CString GetDate( void );
	extern inline Int32 IsPitchReversed( Float pitch );
	extern void FixVector( Float* pflVector );
	extern inline Float RandomFloat( Float low, Float high );
	extern inline Int64 RandomLong( Int64 low, Int64 high );
	extern inline bool IsNAN( Float value );
	extern inline Float SplineFraction( Float value, Float scale );
	extern inline void ScaleByte( DWORD *nInput );
	extern inline CString FixSlashes( const Char* pstring );
	extern inline bool CheckVisibility( const CArray<Uint32>& leafnums, const byte* pset );
	extern bool GetWADList( const Char* pstrEntityData, CArray<CString>& outputArray );
	extern Uint32 GetFileLineCount( const Char* pstrdata );

	extern inline void ParseColor( Float* pout, const color24_t *plightmap );
	extern inline void ParseVectorColor (Float* pout, const color24_t *plightmap);
	extern inline Float RemapValue( Float value, Float a, Float b, Float c, Float d );

	extern inline bool ValueInRange( Float comparisonValue, Float referenceValue, Float rangeMax );
};

#include "common_inline.hpp"

#endif