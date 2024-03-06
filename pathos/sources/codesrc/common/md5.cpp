/*
===============================================
Pathos Engine - Created by Andrew Stephen "Overfloater" Lucas

Copyright 2016
All Rights Reserved.
===============================================
*/

#include "includes.h"
#include "md5.h"

// Constants for MD5Transform routine.
#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

//=============================================
// @brief
//
//=============================================
CMD5::CMD5()
{
	Init();
}

//=============================================
// @brief
//
//=============================================
CMD5::CMD5( const byte* pbuffer, Uint32 bufsize )
{
	Init();
	Update(pbuffer, bufsize);
	Finalize();
}

//=============================================
// @brief
//
//=============================================
void CMD5::Init( void )
{
	m_isFinalized=false;
 
	memset(m_buffer, 0, sizeof(m_buffer));
	memset(m_digest, 0, sizeof(m_digest));

	m_count[0] = 0;
	m_count[1] = 0;
 
	// load magic initialization constants.
	m_state[0] = 0x67452301;
	m_state[1] = 0xefcdab89;
	m_state[2] = 0x98badcfe;
	m_state[3] = 0x10325476;
}

//=============================================
// @brief
//
//=============================================
void CMD5::Decode( Uint32 *poutput, const byte *pinput, Uint32 length )
{
	for(Uint32 i = 0, j = 0; j < length; i++, j += 4)
		poutput[i] = ((Uint32)pinput[j]) | (((Uint32)pinput[j+1]) << 8) | (((Uint32)pinput[j+2]) << 16) | (((Uint32)pinput[j+3]) << 24);
}

//=============================================
// @brief
//
//=============================================
void CMD5::Encode( byte *poutput, const Uint32* pinput, Uint32 length )
{
	for(Uint32 i = 0, j = 0; j < length; i++, j += 4) 
	{
		poutput[j] = pinput[i] & 0xff;
		poutput[j+1] = (pinput[i] >> 8) & 0xff;
		poutput[j+2] = (pinput[i] >> 16) & 0xff;
		poutput[j+3] = (pinput[i] >> 24) & 0xff;
	}
}

//=============================================
// @brief
//
//=============================================
void CMD5::Transform( const byte* pblock )
{
	Uint32 a = m_state[0];
	Uint32 b = m_state[1];
	Uint32 c = m_state[2];
	Uint32 d = m_state[3];

	Uint32 x[16];
	Decode (x, pblock, MD5_BLOCKSIZE);
 
	// Round 1
	FF(a, b, c, d, x[0], S11, 0xd76aa478); // 1
	FF(d, a, b, c, x[1], S12, 0xe8c7b756); // 2
	FF(c, d, a, b, x[2], S13, 0x242070db); // 3
	FF(b, c, d, a, x[3], S14, 0xc1bdceee); // 4
	FF(a, b, c, d, x[4], S11, 0xf57c0faf); // 5
	FF(d, a, b, c, x[5], S12, 0x4787c62a); // 6
	FF(c, d, a, b, x[6], S13, 0xa8304613); // 7
	FF(b, c, d, a, x[7], S14, 0xfd469501); // 8
	FF(a, b, c, d, x[8], S11, 0x698098d8); // 9
	FF(d, a, b, c, x[9], S12, 0x8b44f7af); // 10
	FF(c, d, a, b, x[10], S13, 0xffff5bb1); // 11
	FF(b, c, d, a, x[11], S14, 0x895cd7be); // 12
	FF(a, b, c, d, x[12], S11, 0x6b901122); // 13
	FF(d, a, b, c, x[13], S12, 0xfd987193); // 14
	FF(c, d, a, b, x[14], S13, 0xa679438e); // 15
	FF(b, c, d, a, x[15], S14, 0x49b40821); // 16
 
	// Round 2
	GG(a, b, c, d, x[1], S21, 0xf61e2562); // 17
	GG(d, a, b, c, x[6], S22, 0xc040b340); // 18
	GG(c, d, a, b, x[11], S23, 0x265e5a51); // 19
	GG(b, c, d, a, x[0], S24, 0xe9b6c7aa); // 20
	GG(a, b, c, d, x[5], S21, 0xd62f105d); // 21
	GG(d, a, b, c, x[10], S22,  0x2441453); // 22
	GG(c, d, a, b, x[15], S23, 0xd8a1e681); // 23
	GG(b, c, d, a, x[4], S24, 0xe7d3fbc8); // 24
	GG(a, b, c, d, x[9], S21, 0x21e1cde6); // 25
	GG(d, a, b, c, x[14], S22, 0xc33707d6); // 26
	GG(c, d, a, b, x[3], S23, 0xf4d50d87); // 27
	GG(b, c, d, a, x[8], S24, 0x455a14ed); // 28
	GG(a, b, c, d, x[13], S21, 0xa9e3e905); // 29
	GG(d, a, b, c, x[2], S22, 0xfcefa3f8); // 30
	GG(c, d, a, b, x[7], S23, 0x676f02d9); // 31
	GG(b, c, d, a, x[12], S24, 0x8d2a4c8a); // 32
 
	// Round 3
	HH(a, b, c, d, x[5], S31, 0xfffa3942); // 33
	HH(d, a, b, c, x[8], S32, 0x8771f681); // 34
	HH(c, d, a, b, x[11], S33, 0x6d9d6122); // 35
	HH(b, c, d, a, x[14], S34, 0xfde5380c); // 36
	HH(a, b, c, d, x[1], S31, 0xa4beea44); // 37
	HH(d, a, b, c, x[4], S32, 0x4bdecfa9); // 38
	HH(c, d, a, b, x[7], S33, 0xf6bb4b60); // 39
	HH(b, c, d, a, x[10], S34, 0xbebfbc70); // 40
	HH(a, b, c, d, x[13], S31, 0x289b7ec6); // 41
	HH(d, a, b, c, x[0], S32, 0xeaa127fa); // 42
	HH(c, d, a, b, x[3], S33, 0xd4ef3085); // 43
	HH(b, c, d, a, x[6], S34,  0x4881d05); // 44
	HH(a, b, c, d, x[9], S31, 0xd9d4d039); // 45
	HH(d, a, b, c, x[12], S32, 0xe6db99e5); // 46
	HH(c, d, a, b, x[15], S33, 0x1fa27cf8); // 47
	HH(b, c, d, a, x[2], S34, 0xc4ac5665); // 48
 
	// Round 4
	II(a, b, c, d, x[0], S41, 0xf4292244); // 49
	II(d, a, b, c, x[7], S42, 0x432aff97); // 50
	II(c, d, a, b, x[14], S43, 0xab9423a7); // 51
	II(b, c, d, a, x[5], S44, 0xfc93a039); // 52
	II(a, b, c, d, x[12], S41, 0x655b59c3); // 53
	II(d, a, b, c, x[3], S42, 0x8f0ccc92); // 54
	II(c, d, a, b, x[10], S43, 0xffeff47d); // 55
	II(b, c, d, a, x[1], S44, 0x85845dd1); // 56
	II(a, b, c, d, x[8], S41, 0x6fa87e4f); // 57
	II(d, a, b, c, x[15], S42, 0xfe2ce6e0); // 58
	II(c, d, a, b, x[6], S43, 0xa3014314); // 59
	II(b, c, d, a, x[13], S44, 0x4e0811a1); // 60
	II(a, b, c, d, x[4], S41, 0xf7537e82); // 61
	II(d, a, b, c, x[11], S42, 0xbd3af235); // 62
	II(c, d, a, b, x[2], S43, 0x2ad7d2bb); // 63
	II(b, c, d, a, x[9], S44, 0xeb86d391); // 64
 
	m_state[0] += a;
	m_state[1] += b;
	m_state[2] += c;
	m_state[3] += d;
 
	// Zeroize sensitive information.
	memset(x, 0, sizeof(x));
}

//=============================================
// @brief
//
//=============================================
void CMD5::Update( const byte *pinput, Uint32 length )
{
	// compute number of bytes mod 64
	Uint32 index = m_count[0] / 8 % MD5_BLOCKSIZE;
 
	// Update number of bits
	if ((m_count[0] += (length << 3)) < (length << 3))
		m_count[1]++;

	m_count[1] += (length >> 29);
 
	// number of bytes we need to fill in m_buffer
	Uint32 firstpart = 64 - index;
	Uint32 i;
 
	// transform as many times as possible.
	if (length >= firstpart)
	{
		// fill m_buffer first, transform
		memcpy(&m_buffer[index], pinput, firstpart);
		Transform(m_buffer);
 
		// transform chunks of MD5_BLOCKSIZE (64 bytes)
		for (i = firstpart; i + MD5_BLOCKSIZE <= length; i += MD5_BLOCKSIZE)
			Transform(&pinput[i]);
 
		index = 0;
	}
	else
		i = 0;
 
	// m_buffer remaining input
	memcpy(&m_buffer[index], &pinput[i], length-i);
}

//=============================================
// @brief
//
//=============================================
CMD5& CMD5::Finalize()
{
	static unsigned char padding[64] = {
		0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};
 
	if (!m_isFinalized) 
	{
		// Save number of bits
		byte bits[8];
		Encode(bits, m_count, 8);
 
		// pad out to 56 mod 64.
		Uint32 index = m_count[0] / 8 % 64;
		Uint32 padLen = (index < 56) ? (56 - index) : (120 - index);
		Update(padding, padLen);
 
		// Append length (before padding)
		Update(bits, 8);
 
		// Store state in digest
		Encode(m_digest, m_state, 16);
 
		// Zeroize sensitive information.
		memset(m_buffer, 0, sizeof m_buffer);
		memset(m_count, 0, sizeof m_count);
 
		m_isFinalized = true;
	}
 
	return *this;
}

//=============================================
// @brief
//
//=============================================
CString CMD5::HexDigest( void ) const
{
	if(!m_isFinalized)
		return "";
 
	Char buffer[33];
	for(Uint32 i = 0; i < 16; i++)
		sprintf(&buffer[i*2], "%02x", m_digest[i]);

	// Terminate
	buffer[32]=0;
 
	return CString(buffer);
}